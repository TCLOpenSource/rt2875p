#ifndef __RTK_BB_INTERFACE_H__
#define __RTK_BB_INTERFACE_H__
//----------------------------------------------------------------------------------------------------
// ID Code      : BillboardInterface.h
// Update Note  :
//----------------------------------------------------------------------------------------------------

//****************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************

#if(_TYPE_C_PORT_CTRL_SUPPORT == _ON)
#define GET_USB_HUB_MUX_SEL_STATUS()                    (g_ucUsbBillboardMuxSelPort)
#define SET_USB_HUB_MUX_SEL_STATUS(x)                   (g_ucUsbBillboardMuxSelPort = (x))
#endif

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)

#define _USB_VENDOR_DUAL_BANK_PROCESS                   0x45

#define GET_USB_BB_ISP_READY_FLAG()                     (g_bUsbBillboardIspInfoReadyFlag)
#define SET_USB_BB_ISP_READY_FLAG(x)                    (g_bUsbBillboardIspInfoReadyFlag = (x))
#define CLR_USB_BB_ISP_READY_FLAG()                     (g_bUsbBillboardIspInfoReadyFlag = _FALSE)

#define GET_USB_BB_ISP_SUB_OPCODE()                     (g_stUsbBillboardIspCommandInfo.ucSubOpCode)
#define SET_USB_BB_ISP_SUB_OPCODE(x)                    (g_stUsbBillboardIspCommandInfo.ucSubOpCode = (x))
#define CLR_USB_BB_ISP_SUB_OPCODE()                     (g_stUsbBillboardIspCommandInfo.ucSubOpCode = _FALSE)

#define GET_USB_BB_ISP_OPCODE()                         (g_stUsbBillboardIspCommandInfo.ucOpCode)
#define SET_USB_BB_ISP_OPCODE(x)                        (g_stUsbBillboardIspCommandInfo.ucOpCode = (x))
#define CLR_USB_BB_ISP_OPCODE()                         (g_stUsbBillboardIspCommandInfo.ucOpCode = _FALSE)


#define GET_USB_BB_ISP_REV_CMD_STATUS()                 (g_stUsbBillboardIspCommandInfo.ucRevCmd)
#define SET_USB_BB_ISP_REV_CMD_STATUS()                 (g_stUsbBillboardIspCommandInfo.ucRevCmd = _TRUE)
#define CLR_USB_BB_ISP_REV_CMD_STATUS()                 (g_stUsbBillboardIspCommandInfo.ucRevCmd = _FALSE)

//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************

//****************************************************************************
// VARIABLE EXTERN
//****************************************************************************
extern StructUsbBillboardIspCommandInfo g_stUsbBillboardIspCommandInfo;

//****************************************************************************
// FUNCTION EXTERN
//****************************************************************************
#if(_DUAL_BANK_DEBUG_SUPPORT == _ON)
extern void UsbBillboardCopyISPInfo(unsigned char *pucSrcAddr, unsigned char ucLength, unsigned char ucReadyFlag);
#endif

extern void UsbBillboardIspFlagInitial(void);
#endif
//extern void UsbBillboardHandler(void);
extern void UsbBillboardPllPower(EnumBillboardCtrl enumAtion);
extern unsigned char g_ucUsbBillboardMuxSelPort;

#endif
