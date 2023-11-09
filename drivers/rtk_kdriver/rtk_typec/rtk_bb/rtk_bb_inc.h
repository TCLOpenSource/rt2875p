#ifndef __RTK_BB_INC_H__
#define __RTK_BB_INC_H__
//----------------------------------------------------------------------------------------------------
// ID Code      : BillboardInclude.h
// Update Note  :
//----------------------------------------------------------------------------------------------------

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
//****************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************
#define GET_USB_BB_FLASH_WRITE_FLAG()                     (g_bUsbBillboardFlashWriteFlag)
#define SET_USB_BB_FLASH_WRITE_FLAG()                     (g_bUsbBillboardFlashWriteFlag = _TRUE)
#define CLR_USB_BB_FLASH_WRITE_FLAG()                     (g_bUsbBillboardFlashWriteFlag = _FALSE)

//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************


//****************************************************************************
// VARIABLE EXTERN
//****************************************************************************
extern bool g_bUsbBillboardFlashWriteFlag;

//****************************************************************************
// FUNCTION EXTERN
//****************************************************************************
#endif
#endif