#if 1
/**
 * @file regio.c
 * @brief This file implements all memc exported api.
 * @date August.29.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Kiwi-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

/*! include register db define */

#ifdef WIN32
#define DLL_IMPLEMENT
#endif

#include "memc_isr/Driver/regio.h"
#include "memc_isr/Common/kw_common.h"		///< common stdio define such as kw_printf etc
#include "memc_isr/Common/kw_debug.h"		///< debug support
#include "memc_isr/Driver/reg_offset.h"		///< register offset for different customer
//#include "drv_i2c.h"

//#include "rtk_vgip_isr.h"
#ifdef CONFIG_ARM64 //ARM32 compatible
//#include "io.h"

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
#else
//#include "io.h"
#include <tvscalercontrol/io/ioregdrv.h>
#endif

#ifdef WIN32

static ReadRegCallback gReadCallbackFunc = NULL;
static WriteRegCallback gWriteCallbackFunc = NULL;

#endif

static unsigned int gu32BaseOffset = BASE_OFFSET;

/**
 * @brief This function set Base Offset of Register R/W
 * @param [in] u32Offset
 * @retval FALSE
*/
VOID RegSetBaseOffset(unsigned int u32Offset)
{
	gu32BaseOffset = u32Offset;
}

/*! define register access api for use */

/* Invoke Sample:
Read named register: base: FRC_TOP Page: kmc_top Named register reg_top_patt_htotal

BOOL bRet = FASLE;
unsigned int uiVal = 0;
bRet = ReadRegister(KMC_TOP_kmc_top_00_reg,0,15, &uiVal);

Write name register: base: FRC_TOP Page: kmc_top Named register reg_top_patt_htotal
BOOL bRet = FASLE;
unsigned int uiVal = 0x100;
bRet = WriteRegister(KMC_TOP_kmc_top_00_reg,0,15, uiVal);
*/

/**
 * @brief This function read specified register and returns named register value, the value has been processed for specified named register.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL ReadRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int *puiVal)
{
	BOOL bRet = TRUE;
	unsigned int u32Mask = 0xffffffff;

	if (ibitEnd < ibitStart)
	{
		return FALSE;
	}

	if((uiAddr % 4 == 0) && (ibitEnd - ibitStart == 31))
	{
		#ifdef WIN32
		if(gReadCallbackFunc != NULL)
		{
			bRet = gReadCallbackFunc(uiAddr, puiVal);
		}
		#else
			//uiAddr += gu32BaseOffset;

			*puiVal = rtd_inl(uiAddr);
		#endif
		return bRet;
	}


#ifdef WIN32
	if(gReadCallbackFunc != NULL)
	{
		bRet = gReadCallbackFunc(uiAddr, puiVal);
	}
	//bRet = Drv_IIC_ReadDword(uiAddr, puiVal);
#else
	//uiAddr += gu32BaseOffset;
	*puiVal = rtd_inl(uiAddr);
#endif
	// get mask
	u32Mask = u32Mask << (31 - ibitEnd);
	u32Mask = u32Mask >> (31 - ibitEnd);
	u32Mask = u32Mask >> ibitStart;
	u32Mask = u32Mask << ibitStart;

	// set mask
	*puiVal &= u32Mask;
	*puiVal = *puiVal >> ibitStart;

	return bRet;
}

/**
 * @brief This function write specified register of named register value. The value must do not move bits.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param uiVal [in]		Named Register value want to write
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL WriteRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int uiVal)
{
	BOOL bRet = TRUE;
	unsigned int u32Val = 0, u32Mask = 0xffffffff, u32SetValue = 0;

	if (ibitEnd < ibitStart)
	{
		return FALSE;
	}

	if((uiAddr % 4 == 0) && (ibitEnd - ibitStart == 31))
	{
		#ifdef WIN32
		if(gWriteCallbackFunc != NULL)
		{
			bRet = gWriteCallbackFunc(uiAddr, uiVal);
		}
		#else
			//uiAddr += gu32BaseOffset;
			rtd_outl(uiAddr, uiVal);
		#endif

		return bRet;
	}

#ifdef WIN32
	if(gReadCallbackFunc != NULL)
	{
		bRet = gReadCallbackFunc(uiAddr, &u32Val);
	}
	//bRet = Drv_IIC_ReadDword(uiAddr, &u32Val);
#else
	//uiAddr += gu32BaseOffset;
	//u32Val = *(unsigned int*)uiAddr;//need to modify CSFC,20160906
	u32Val = rtd_inl(uiAddr);
#endif
	u32SetValue = uiVal;

	// get mask
	u32Mask = u32Mask << (31 - ibitEnd);
	u32Mask = u32Mask >> (31 - ibitEnd);
	u32Mask = u32Mask >> ibitStart;

	// process uiset value to avoid uiset value has too many bits
	u32SetValue &= u32Mask;

	u32Mask = u32Mask << ibitStart;

	u32Mask = ~u32Mask;
	u32SetValue = u32SetValue << ibitStart;

	// set mask
	u32Val &= u32Mask;
	u32Val |= u32SetValue;

#ifdef WIN32
	if(gWriteCallbackFunc != NULL)
	{
		bRet = gWriteCallbackFunc(uiAddr, u32Val);
	}
	//bRet = Drv_IIC_WriteDword(uiAddr, u32Val);
#else
	rtd_outl(uiAddr, u32Val);
#endif
	return bRet;
}

/**
 * @brief rtk register read
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL RTKReadRegister(unsigned int uiAddr, unsigned int *puiVal)
{
	*puiVal = rtd_inl(uiAddr);

	return TRUE;

}

/**
 * @brief rtk register write
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL RTKWriteRegister(unsigned int uiAddr, unsigned int uiVal)
{
	rtd_outl(uiAddr, uiVal);

	return TRUE;
}

#ifdef WIN32
BOOL RegIOReadRegSetCallback(ReadRegCallback cb)
{
	gReadCallbackFunc = cb;
	return TRUE;
}

BOOL RegIOWriteRegSetCallback(WriteRegCallback cb)
{
	gWriteCallbackFunc = cb;
	return TRUE;
}

#endif


#else
/**
 * @file regio.c
 * @brief This file implements all memc exported api.
 * @date August.29.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par MEMC Library V0.0.1 - Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

/*! include register db define */

#ifdef WIN32
#define DLL_IMPLEMENT
#endif

#include "memc_isr/Driver/regio.h"
#include "memc_isr/Common/kw_common.h"		///< common stdio define such as kw_printf etc
#include "memc_isr/Common/kw_debug.h"		///< debug support
#include "memc_isr/Driver/reg_offset.h"		///< register offset for different customer
//#include "drv_i2c.h"

#include "rtk_vgip_isr.h"
#include "io.h"

#ifdef WIN32
static ReadRegCallback gReadCallbackFunc = NULL;
static WriteRegCallback gWriteCallbackFunc = NULL;
#endif

static unsigned int gu32BaseOffset = BASE_OFFSET;

/**
 * @brief This function set Base Offset of Register R/W
 * @param [in] u32Offset
 * @retval FALSE
*/
VOID RegSetBaseOffset(unsigned int u32Offset)
{
	gu32BaseOffset = u32Offset;
}

/*! define register access api for use */

/* Invoke Sample:
Read named register: base: FRC_TOP Page: kmc_top Named register reg_top_patt_htotal

BOOL bRet = FASLE;
unsigned int uiVal = 0;
bRet = ReadRegister(KMC_TOP_kmc_top_00_reg,0,15, &uiVal);

Write name register: base: FRC_TOP Page: kmc_top Named register reg_top_patt_htotal
BOOL bRet = FASLE;
unsigned int uiVal = 0x100;
bRet = WriteRegister(KMC_TOP_kmc_top_00_reg,0,15, uiVal);
*/

/**
 * @brief This function read specified register and returns named register value, the value has been processed for specified named register.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL ReadRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int *puiVal)
{
	BOOL bRet = TRUE;
	unsigned int u32Mask = 0xffffffff;

	if (ibitEnd < ibitStart)
	{
		return FALSE;
	}

	if((uiAddr % 4 == 0) && (ibitEnd - ibitStart == 31))
	{
		#ifdef WIN32
		if(gReadCallbackFunc != NULL)
		{
			bRet = gReadCallbackFunc(uiAddr, puiVal);
		}
		#else
			uiAddr += gu32BaseOffset;

			*puiVal = rtd_inl(uiAddr);
		#endif
		return bRet;
	}


#ifdef WIN32
	if(gReadCallbackFunc != NULL)
	{
		bRet = gReadCallbackFunc(uiAddr, puiVal);
	}
	//bRet = Drv_IIC_ReadDword(uiAddr, puiVal);
#else
	uiAddr += gu32BaseOffset;
	*puiVal = rtd_inl(uiAddr);
#endif
	// get mask
	u32Mask = u32Mask << (31 - ibitEnd);
	u32Mask = u32Mask >> (31 - ibitEnd);
	u32Mask = u32Mask >> ibitStart;
	u32Mask = u32Mask << ibitStart;

	// set mask
	*puiVal &= u32Mask;
	*puiVal = *puiVal >> ibitStart;

	return bRet;
}

/**
 * @brief This function write specified register of named register value. The value must do not move bits.
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param ibitStart [in]	Named Register start bit in register address
 * @param ibitEnd [in]		Named Register end bit in register address
 * @param uiVal [in]		Named Register value want to write
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL WriteRegister(unsigned int uiAddr, int ibitStart, int ibitEnd, unsigned int uiVal)
{
	BOOL bRet = TRUE;
	unsigned int u32Val = 0, u32Mask = 0xffffffff, u32SetValue = 0;

	if (ibitEnd < ibitStart)
	{
		return FALSE;
	}

	if((uiAddr % 4 == 0) && (ibitEnd - ibitStart == 31))
	{
		#ifdef WIN32
		if(gWriteCallbackFunc != NULL)
		{
			bRet = gWriteCallbackFunc(uiAddr, uiVal);
		}
		#else
			uiAddr += gu32BaseOffset;
			rtd_outl(uiAddr, uiVal);
		#endif

		return bRet;
	}

#ifdef WIN32
	if(gReadCallbackFunc != NULL)
	{
		bRet = gReadCallbackFunc(uiAddr, &u32Val);
	}
	//bRet = Drv_IIC_ReadDword(uiAddr, &u32Val);
#else
	uiAddr += gu32BaseOffset;
	//u32Val = *(unsigned int*)uiAddr;//need to modify CSFC,20160906
	u32Val = rtd_inl(uiAddr);
#endif
	u32SetValue = uiVal;

	// get mask
	u32Mask = u32Mask << (31 - ibitEnd);
	u32Mask = u32Mask >> (31 - ibitEnd);
	u32Mask = u32Mask >> ibitStart;

	// process uiset value to avoid uiset value has too many bits
	u32SetValue &= u32Mask;

	u32Mask = u32Mask << ibitStart;

	u32Mask = ~u32Mask;
	u32SetValue = u32SetValue << ibitStart;

	// set mask
	u32Val &= u32Mask;
	u32Val |= u32SetValue;

#ifdef WIN32
	if(gWriteCallbackFunc != NULL)
	{
		bRet = gWriteCallbackFunc(uiAddr, u32Val);
	}
	//bRet = Drv_IIC_WriteDword(uiAddr, u32Val);
#else
	rtd_outl(uiAddr, u32Val);
#endif
	return bRet;
}

/**
 * @brief rtk register read
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL RTKReadRegister(unsigned int uiAddr, unsigned int *puiVal)
{
	*puiVal = rtd_inl(uiAddr);

	return TRUE;

}

/**
 * @brief rtk register write
 * @param uiAddr [in]		Named Register address, 32 bits
 * @param puiVal [in/out]	Named Register value
 * @retval TRUE 			indicates success
 * @retval FALSE			indicates fail.
*/
BOOL RTKWriteRegister(unsigned int uiAddr, unsigned int uiVal)
{
	rtd_outl(uiAddr, uiVal);

	return TRUE;
}

#ifdef WIN32
BOOL RegIOReadRegSetCallback(ReadRegCallback cb)
{
	gReadCallbackFunc = cb;
	return TRUE;
}

BOOL RegIOWriteRegSetCallback(WriteRegCallback cb)
{
	gWriteCallbackFunc = cb;
	return TRUE;
}

#endif


#endif
