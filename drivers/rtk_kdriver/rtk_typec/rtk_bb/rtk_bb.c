//----------------------------------------------------------------------------------------------------
// ID Code      : Billboard.c
// Update Note  :
//----------------------------------------------------------------------------------------------------
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <mach/rtk_platform.h>
#include <mach/pcbMgr.h>
#include <rbus/usb_bb_reg.h>
#include <rbus/sys_reg_reg.h>
#include <linux/spinlock.h>
#include "rtk_bb.h"
#include "rtk_bb_interface_user.h"
#include "rtk_bb_interface.h"
#include "rtk_bb_inc.h"
#include "rtk_bb_wrapper.h"
#include <rtd_log/rtd_module_log.h>


#define DRIVER_DESC "RTK BillBoard generic platform driver"

static DEFINE_SPINLOCK(g_rtk_bb_lock);
static struct completion g_usb_bb_check_thread_exited;
static struct task_struct * g_usb_bb_task = NULL;
static int g_usb_bb_check_thread_exited_flag = 0;

//****************************************************************************
// CODE TABLES
//****************************************************************************
unsigned char  tBillboardUrl_DP[] =
{
    72, 3,
    'h',0,'t',0,'t',0,'p',0,
    ':',0,'/',0,'/',0,'h',0,
    'e',0,'l',0,'p',0,'.',0,
    'v',0,'e',0,'s',0,'a',0,
    '.',0,'o',0,'r',0,'g',0,
    '/',0,'d',0,'p',0,'-',0,
    'u',0,'s',0,'b',0,'-',0,
    't',0,'y',0,'p',0,'e',0,
    '-',0,'c',0,'/',0
};

unsigned char  tBillboardUrl_Lenovo[] =
{
    50, 3,
    'h',0,'t',0,'t',0,'p',0,
    ':',0,'/',0,'/',0,'w',0,
    'w',0,'w',0,'.',0,'l',0,
    'e',0,'n',0,'o',0,'v',0,
    'o',0,'.',0,'c',0,'o',0,
    'm',0,'.',0,'c',0,'n',0
};

unsigned char  tBillboardUrl_iAddtionalInfo[] =
{
    46, 3,
    'h',0,'t',0,'t',0,'p',0,
    ':',0,'/',0,'/',0,'w',0,
    'w',0,'w',0,'.',0,'r',0,
    'e',0,'a',0,'l',0,'t',0,
    'e',0,'k',0,'.',0,'c',0,
    'o',0,'m',0
};

unsigned char  tUsbBillboardUsbDeviceDescriptor[_USB_DESC_SIZE_DEV] =
{
    _USB_DESC_SIZE_DEV,                                  // bLength
    _USB_DEVICE_DESCRIPTOR_TYPE,                         // bDescriptorType (Device Desc 0x01)
    0x01, 0x02,                                          // bcdUSB (LSB, MSB), USB3.0
    0x11,                                                // bDeviceClass ( Billboard Class 0x11)
    0x00,                                                // bDeviceSubClass
    0x00,                                                // bDeviceProtocol
    0x40,                                                // bMaxPacketSize0, 64 bytes
    _USB_VENDOR_ID_L,                                    // Vendor ID
    _USB_VENDOR_ID_H,
    _USB_PRODUCT_ID_L,                                   // PID
    _USB_PRODUCT_ID_H,
    _USB_FW_SUB_VER_,                                    // bcdDevice
    _USB_FW_MAIN_VER_,
    _USB_BB_INDEX1,                                      // iManufacturer - idx of Manf Str Desc.
    _USB_BB_INDEX2,                                      // iProduct - idx of Prod String Descriptor
    _USB_BB_INDEX3,                                      // iSerialNumber - Idx of Serial Num String
    0x01                                                 // bNumConfigurations
};

unsigned char  tUsbBillboardUsbConfigDescriptor[_USB_DESC_SIZE_CONF_TOTAL] =
{
    // --------- Configuration Descriptor Type ---------
    _USB_DESC_SIZE_CONF,                             // bLength
    _USB_CONFIGURATION_DESCRIPTOR_TYPE,              // bDescriptorType
    ((_USB_DESC_SIZE_CONF_TOTAL) % 256),
    ((_USB_DESC_SIZE_CONF_TOTAL) / 256),             // wTotalLength
    0x01,                                            // bNumInterfaces
    0x01,                                            // bConfigurationValue
    _USB_BB_INDEX2,                                  // iConfiguration
    0xC0,                                            // bmAttributes,
    0x00,                                            // bMaxPower

    // --------- Interface Descriptor Type ------------
    _USB_DESC_SIZE_INTF,                             // bLength
    _USB_INTERFACE_DESCRIPTOR_TYPE,                  // bDescriptorType
    0x00,                                            // bInterfaceNumber
    0x00,                                            // bAlternateSetting
    0x00,                                            // bNumEndpoints (the status change endpoint: interrupt endpoint)
    0x11,                                            // bInterfaceClass(Hub Class == 0x09)
    0x00,                                            // bInterfaceSubClass
    0x00,                                            // bInterfaceProtocol, single-TT
    _USB_BB_INDEX2,                                  // ilInterface (Idx of this intf str desc.)
};


unsigned char  tUsbBillboardUsbStringDescriptor[_USB_DESC_SIZE_CPLS] =
{
    _USB_DESC_SIZE_CPLS,                                 // bLength
    _USB_STRING_DESCRIPTOR_TYPE,                         // bDescriptorType
    0x09, 0x04
};

unsigned char  tUsbBillboardBOSDeviceCapabilityDescriptor[_USB_BB_BOS_DESC_FIRST_PART_LEN] =
{
    // ----------- BOS Descriptor Type ---------------
    _USB_DESC_SIZE_BOS,                                 // bLength
    _USB_BOS_DESCRIPTOR_TYPE,                           // bDescriptorType
    // lint -e572 :Excessive shift value (precision 7 shifted right by 8)
    (_USB_BB_BOS_DESC_TOTAL_LEN_INIT % 256),            // wTotalLength(LSB),need to be reconfirmed in FW
    (_USB_BB_BOS_DESC_TOTAL_LEN_INIT / 256),            // wTotalLength(MSB),need to be reconfirmed in FW
    // lint +e572 :Excessive shift value (precision 7 shifted right by 8)
    _USB_BOS_DEVICEACPS_NUM,                            // bNumDeviceCaps,need to be reconfirmed in FW(_USB_BOS_DEVICEACPS_NUM + x BBAltModeCaps)

    // ----- Device Capability Descriptor Type - USB2.0 Extension
    _USB_DESC_SIZE_USB2_EXTENSION,                      // bLength
    _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE,             // bDescriptorType
    _USB_EXTENSION_TYPE,                                // bDevCapabilityType
#ifdef _WIN8P1_LPM_L1_
    0x1E, 0xF4, 0x00, 0x00,                             // bmAttributes, SS device shall support LPM (LSB~MSB)
#else
    0x00, 0x00, 0x00, 0x00,                             // bmAttributes, SS device shall support LPM (LSB~MSB)
#endif

#if(_USB_31_SS_SUPPORT == _ON)
    // ---- Device Capability Descriptor - SuperSpeed USB
    _USB_DESC_SIZE_SUPERSPEED_USB,                      // bLength
    _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE,             // bDescriptorType
    _USB_SUPERSPEED_USB_TYPE,                           // bDevCapabilityType
    0x00,                                               // bmAttributes, Do not support LTM
    0x0E, 0x00,                                         // wSpeedsSupported, Support HS/FS, (LSB,MSB)
    0x01,                                               // bFunctionalitySupport, Lowest Speed of function is FS
    0x0A,                                               // bU1DevExitLat, Less than 10us for U1->U0
    0xFF, 0x03,                                         // wU2DevExitLat, less than 1024us for U2->U0, (LSB,MSB)
#endif

    // ---- Device ContainID descriptor
    _USB_DESC_SIZE_CONTAINER_ID,                        // bLength
    _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE,             // bDescriptorType
    _USB_CONTAINER_ID_TYPE,                             // bDevCapabilityType
    0x00,
    0xAF, 0x4F, 0xAC, 0x0D,                             // ContainID
    0x00, 0x87, 0x40, 0xEE,                             // This is a 128-bit number that is unique to a device instance
    0xB7, 0x58, 0x26, 0xA1,                             // that is used to uniquely identify the device instance accross
    0x07, 0xD7, 0x6C, 0x6B,                             // all mode of operation.

    // ---- BillBoard Capability descriptor
    _USB_DESC_SIZE_BILLBOARD_CAP + _USB_DESC_SIZE_ALTERNATE_MODE * _USB_MAX_ALTERNATE_MODE_NUM,  // bLength,need to be reconfirmed in FW
    _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE,             // bDescriptorType
    _USB_BILLBOARD_TYPE,                                // bDevCapabilityType
    _TYPE_C_BILLBOARD_URL_INDEX_IADDTIONALINFO,         // iAdditionalInfoURL(TBD)
    _USB_MAX_ALTERNATE_MODE_NUM,                        // bNumberOfAlternateModes
    0x00,                                               // bPreferredAlternateModes
    0x00,                                               // bVconnPower(LSB) bit 0~2: Vconn Power  000b=1W, 001b=1.5W, 010b=2W, 011b=3W
    // 100b=4W, 101b=5W, 110b=6W, 111b=reserved
    0x00,
    0x03,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,                 // bmConfigured
                                                        // total 32 byte(256 bit) for 128 altenate modes(spec 0x34)
                                                        // 00b:Unspecified Error, 01b:Alternate Mode configuration
                                                        // 10b:Alternate Mode configuration attempted but unsuccessful
                                                        // 11b:Alternate Mode configuration successful
    0x21, 0x01,                                         // bcdVersion
    0x00,                                               // bAdditonalFailureInfo
    0x00                                                // bReserved

    // Concatenate USB_DEVCAP_BILLBOARD_CAP_DESCRIPTOR_ALTERNATE_MODE in FW
};

//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************
StructUsbBBAltModeCapabilityDescriptor  g_stUsbBillboardAltModeCapabilityDescriptor =
{
    _USB_DESC_SIZE_BILLBOARD_ALTMODE_CAP,                // bLength
    _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE,              // bDescriptorType
    _USB_BILLBOARD_ALTMODE_TYPE,                         // bDevCapabilityType
    0x00,                                                // need to be reconfirmed in FW, refer g_pstUsbBillboardAltModeInfo ucStringIndex
    {
        0x00,0x00,0x00,0x00                              // need to be reconfirmed in FW, refer g_pstUsbBillboardAltModeInfo ucdwAlternateModeVdo_Byte0-4
    }
};

//****************************************************************************
// CODE TABLES
//****************************************************************************
unsigned char  tManufacturer[] =
{
    // MID = 1111 "Generic"
    16,  3, 'G', 0, 'e',  0, 'n', 0,
    'e', 0, 'r', 0, 'i',  0, 'c', 0
};

unsigned char  tProduct[] =
{
    28,  3, 'U', 0, 'S',  0, 'B', 0,
    '-', 0, 'C', 0, ' ',  0, 'A', 0,
    'D', 0, 'A', 0, 'P',  0, 'T', 0,
    'O', 0, 'R', 0
};

unsigned char  tSerialNumber[] =
{
    36, 3, '2', 0, '0', 0, '0', 0, '9', 0,
    '0', 0, '7', 0, '0', 0, '3', 0, '8', 0,
    '1', 0, '9', 0, '9', 0, '0', 0, '0', 0,
    '0', 0, '0', 0, '0', 0
};

unsigned char  tUsbBillboardManufacturer_Custom[_IMANUFACTUR_SIZE] = {0};

unsigned char  tUsbBillboardManufacturer[] =
{
    // MID = 1111 "Generic"
    16,  3, 'R', 0, 'e',  0, 'a', 0,
    'l', 0, 't', 0, 'e',  0, 'k', 0
};

unsigned char  tUsbBillboardProduct_Custom[_IPRODUCT_SIZE] = {0};

unsigned char  tUsbBillboardProduct[] =
{
    34,  3, 'B', 0, 'i',  0, 'l', 0,
    'l', 0, 'B', 0, 'o',  0, 'a', 0,
    'r', 0, 'd', 0, ' ',  0, 'D', 0,
    'e', 0, 'v', 0, 'i',  0, 'c', 0,
    'e', 0
};

unsigned char  tBBSerialNumber[] =
{
    36, 3, '0', 0, '0', 0, '0', 0, '0', 0,
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
    '0', 0, '0', 0, '0', 0
};

unsigned char  tRTString[] =
{
    0x0E, 0x03, 'R', 0x00, 'T', 0x00, ' ', 0x00,
    'H',  0x00, 'u', 0x00, 'b', 0x00
};

unsigned char  tBBDeviceQualifier[] =
{
    0x0A,   // bLength
    0x06,   // bDescriptorType
    0x10,   // bcdUSB (LSB)
    0x02,   // bcdUSB (MSB)
    0x11,   // bDeviceClass
    0x00,   // bDeviceSubClass
    0x00,   // bDeviceProtocol
    0x40,   // bMaxPacketSixe0
    0x01,   // bNumberConfigurations
    0x00    // bReserved
};

unsigned char g_pucUsbBillboardRTHandshake[] =
{
    0xA, 'R', 'E', 'A', 'L', 'T', 'E',
    'K',  'L', 'C', 'D'
};

//****************************************************************************
// VARIABLE DECLARATIONS
//****************************************************************************

unsigned char g_ucUsbBillboardUsbHostEnableRemoteWakeup;
unsigned char g_ucUsbBillboardUsbConfigurationValue;
unsigned char g_ucUsbBillboardUsbVendorComEnb;
int PDATA_WORD = 0;
//TODO
unsigned int P1 = 1;
unsigned int P2 = 2;
unsigned int P3 = 3;

bool g_bUsbBillboardLoadCustomManufactory = _FALSE;
bool g_bUsbBillboardLoadCustomProduct = _FALSE;
bool g_bUsbBillboardInitial = _FALSE;
bool g_bUsbBillboardIspEn = _FALSE;
//bool g_bBBDebugMessageEven;
//unsigned int g_usBBFilterPro;
//bool g_bBBDebugMessageStart;
unsigned char g_ucUsbBillboardMuxSelPort = _NO_TYPE_C_PORT;
int BB_event_flag = 0;

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
unsigned char g_pucUsbBillboardGetISPinfo[16];
bool g_bUsbBillboardIspInfoReadyFlag = _FALSE;
bool g_bUsbBillboardFlashWriteFlag = _FALSE;

StructUsbBillboardIspCommandInfo g_stUsbBillboardIspCommandInfo;
#endif

StructTypeCBillboardInfo g_pstUsbBillboardAltModeInfo[_USB_MAX_ALTERNATE_MODE_NUM];

//****************************************************************************
// FUNCTION DECLARATIONS
//****************************************************************************


//****************************************************************************
// FUNCTION DEFINITIONS
//****************************************************************************
//--------------------------------------------------
// Description  : Bill Board Device Handler Process
// Input Value  : None
// Output Value : None
//--------------------------------------------------
static irqreturn_t UsbBillboardHandler(int irq, void *dev_id)
{
    // Judge USB Setup Packet Received or not
    irqreturn_t ret = IRQ_NONE;
    spin_lock(&g_rtk_bb_lock);
    if(!is_UsbBillboard_CRT_ON()) {
		spin_unlock(&g_rtk_bb_lock);
		return ret;
    }

    if((rtd_inl(P98_00_HS_EP0_IRQ) & _BIT0) == _BIT0)
    {
        // Clear Buf0 for EP0 RX/TX Enable

        rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT3 | _BIT2), 0x00);

        // Clear USB Setup Packet Received Flag
        rtd_maskl(P98_00_HS_EP0_IRQ, ~(_BIT4 | _BIT3 | _BIT2 | _BIT1 | _BIT0), _BIT0);

        rtd_pr_usb_bb_debug("U2 Req Type = %x\n", rtd_inl(P98_08_HS_EP0_SETUP0));

        switch(rtd_inl(P98_08_HS_EP0_SETUP0) & _USB_REQUEST_TYPE_MASK)
        {
            case _USB_STANDARD_REQUEST:
                UsbBillboardStandardRequest();
                break;

            case _USB_VENDOR_REQUEST:
                UsbBillboardVendorRequest();
                break;

            default:
                rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
                break;
        }
	 ret = IRQ_HANDLED;
     } else if((rtd_inl(P98_00_HS_EP0_IRQ) & (_BIT4 | _BIT3 | _BIT2 | _BIT1 )) 
     		== (_BIT4 | _BIT3 | _BIT2 | _BIT1 )) {
			rtd_maskl(P98_00_HS_EP0_IRQ, ~(_BIT4 | _BIT3 | _BIT2 | _BIT1 | _BIT0), (_BIT4 | _BIT3 | _BIT2 | _BIT1 ));
			ret = IRQ_HANDLED;
	 }

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
    // BB Reply Info to Tool (Info From UserCommonDualBankHandler)
    if(GET_USB_BB_ISP_READY_FLAG() == _TRUE)
    {
        if(GET_USB_BB_ISP_OPCODE() == _USB_VENDOR_DUAL_BANK_PROCESS)
        {
            UsbBillboardVendorDualBankProcess2();
        }
        else
        {
            rtd_pr_usb_bb_debug("Command error = %x\n", GET_USB_BB_ISP_OPCODE());
        }
	 ret = IRQ_HANDLED;
    }
#endif
    spin_unlock(&g_rtk_bb_lock);
    return ret;
}

//--------------------------------------------------
// Description  : Standard Request handler
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardStandardRequest(void)
{
    rtd_pr_usb_bb_debug("U2 Std Request = %x\n", rtd_inl(P98_09_HS_EP0_SETUP1));
    rtd_pr_usb_bb_debug("U2 Std wVALL = %x\n", rtd_inl(P98_0A_HS_EP0_SETUP2));
    rtd_pr_usb_bb_debug("U2 Std wVALH = %x\n", rtd_inl(P98_0B_HS_EP0_SETUP3));
    rtd_pr_usb_bb_debug("U2 Std wIDXL = %x\n", rtd_inl(P98_0C_HS_EP0_SETUP4));
    rtd_pr_usb_bb_debug("U2 Std wIDXH = %x\n", rtd_inl(P98_0D_HS_EP0_SETUP5));
    rtd_pr_usb_bb_debug("U2 Std wLENL = %x\n", rtd_inl(P98_0E_HS_EP0_SETUP6));
    rtd_pr_usb_bb_debug("U2 Std wLENH = %x\n", rtd_inl(P98_0F_HS_EP0_SETUP7));

    switch(rtd_inl(P98_09_HS_EP0_SETUP1))
    {
        case _USB_REQUEST_GET_DESCRIPTOR:
            UsbBillboardGetDescriptor();
            break;

        case _USB_REQUEST_SET_ADDRESS:
            UsbBillboardSetAddr();
            break;

        case _USB_REQUEST_SET_CONFIGURATION:
            UsbBillboardSetConf();
            break;

        case _USB_REQUEST_GET_INTERFACE:
            // Undefined request for hubs
            // Hubs are allowed to support only one interface.
            UsbBillboardGetInterface();

            break;

        case _USB_REQUEST_SET_INTERFACE:
            // Undefined request for hubs
            // Hubs are allowed to support only one interface.
            UsbBillboardSetInterface();

            break;

        case _USB_REQUEST_GET_STATUS:
            UsbBillboardGetStatus();
            break;

        case _USB_REQUEST_GET_CONFIGURATION:
            UsbBillboardGetConf();
            break;

        case _USB_REQUEST_SET_FEATURE:
            UsbBillboardSetFeature();
            break;

        case _USB_REQUEST_CLEAR_FEATURE:
            UsbBillboardClearFeature();
            break;    // end of USB_REQUEST_CLEAR_FEATURE

        case _USB_REQUEST_SYNC_FRAME:
            // Undefined request for hubs
            // Hubs are not allowed to have isochronous endpoints,
            // thus this request should return STALL
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;

        case _USB_REQUEST_SET_DESCRIPTOR:
            // Optional requests that are not implemented shall return
            // STALL in the Data stage or Status stage of the request.
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;

        default:
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;
    }
}

//-------------------------------------------------------
// Description  : USB Vendor Request - Set Register Byte
// Input Value  : None
// Output Value : None
//-------------------------------------------------------
void UsbBillboardVendorSetRegisterByte(void)
{
    unsigned int usLength = 0;
    unsigned char ucIdx = 0;
    unsigned int usRegisterAdd = 0x0000;
    // unsigned char xdata *pucFlashData = 0x0000;
    unsigned int usFlag = 0x0000;
    unsigned char ucPageIdx = 0;
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    usRegisterAdd = (rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) + rtd_inl(P98_0A_HS_EP0_SETUP2);
    usFlag = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);
    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;

    // temp buffer for U2 RX test
    for(ucPageIdx = 0; ucPageIdx < usPageNum; ucPageIdx++)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            for(ucIdx = 0; ucIdx < _USB_EP0_DATA_BUF_SIZE; ucIdx++)
            {
                if((usFlag & _BIT0) == _BIT0)
                {
                    // pucFlashData[usRegisterAdd++] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
#if(_CPU_TYPE_ID == _CPU_ID_DW8051)
                    if(usRegisterAdd == PFF_E2_PORT1_PIN_REG)
                    {
                        //BB_SET_P1_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P1 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                        usRegisterAdd++;
                    }
                    else if(usRegisterAdd == PFF_E3_PORT3_PIN_REG)
                    {
                        //BB_SET_P3_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P3 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                        usRegisterAdd++;
                    }
                    else
#endif
                    {
                        rtd_outl(usRegisterAdd++, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                    }
                }
                else
                {
                    // pucFlashData[usRegisterAdd] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
#if(_CPU_TYPE_ID == _CPU_ID_DW8051)
                    if(usRegisterAdd == PFF_E2_PORT1_PIN_REG)
                    {
                        //BB_SET_P1_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P1 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                    }
                    else if(usRegisterAdd == PFF_E3_PORT3_PIN_REG)
                    {
                        //BB_SET_P3_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P3 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                    }
                    else
#endif
                    {
                        rtd_outl(usRegisterAdd, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                    }
                }
            }
            // [TBD]: Write flash...
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }

    // [TBD]: Read the remaining
    if(usRemainNum != 0)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            for(ucIdx = 0; ucIdx < usRemainNum; ucIdx++)
            {
                if((usFlag & _BIT0) == _BIT0)
                {
                    // pucFlashData[usRegisterAdd++] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
#if(_CPU_TYPE_ID == _CPU_ID_DW8051)
                    if(usRegisterAdd == PFF_E2_PORT1_PIN_REG)
                    {
                        //BB_SET_P1_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P1 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                        usRegisterAdd++;
                    }
                    else if(usRegisterAdd == PFF_E3_PORT3_PIN_REG)
                    {
                        //BB_SET_P3_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P3 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                        usRegisterAdd++;
                    }
                    else
#endif
                    {
                        rtd_outl(usRegisterAdd++, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                    }
                }
                else
                {
                    // pucFlashData[usRegisterAdd] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
#if(_CPU_TYPE_ID == _CPU_ID_DW8051)
                    if(usRegisterAdd == PFF_E2_PORT1_PIN_REG)
                    {
                        //BB_SET_P1_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P1 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                    }
                    else if(usRegisterAdd == PFF_E3_PORT3_PIN_REG)
                    {
                        //BB_SET_P3_VALUE(rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                        P3 = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                    }
                    else
#endif
                    {
                        rtd_outl(usRegisterAdd, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                    }
                }
            }
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }
    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, _FALSE);
}

//-------------------------------------------------------
// Description  : USB Vendor Request - Get Register Byte
// Input Value  : None
// Output Value : None
//-------------------------------------------------------
void UsbBillboardVendorGetRegisterByte(void)
{
    unsigned int usLength = 0;
    unsigned char ucIdx = 0;
    unsigned int usRegisterAdd = 0x0000;
    unsigned int usFlag = 0x0000;
    unsigned char ucPageIdx = 0;
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    usRegisterAdd = (rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) + rtd_inl(P98_0A_HS_EP0_SETUP2);
    usFlag = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);
    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;

    // [TBD]: limit to one Byte set atm.
    // if(usLength > 1)
    // usLength = 1;
    for(ucPageIdx = 0; ucPageIdx < usPageNum; ucPageIdx++)
    {
        // [TBD]:Read data from flash...
        // Set write fifo to transmit data to host
        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
        for(ucIdx = 0; ucIdx < _USB_EP0_DATA_BUF_SIZE; ucIdx++)
        {
            if((usFlag & _BIT0) == _BIT0)
            {
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, rtd_inl(usRegisterAdd++));
            }
            else
            {
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, rtd_inl(usRegisterAdd));
            }
        }
        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
    }

    if(usRemainNum == 0)
    {
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
    }
    else
    {
        // [TBD]:Read remaining data from flash...

        // Set write fifo to transmit data to host
        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
        for(ucIdx = 0; ucIdx < usRemainNum; ucIdx++)
        {
            if((usFlag & _BIT0) == _BIT0)
            {
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, rtd_inl(usRegisterAdd++));
            }
            else
            {
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, rtd_inl(usRegisterAdd));
            }
        }

        rtd_outl(P98_06_HS_EP0_TX_BC, usRemainNum);
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
    }
    rtd_pr_usb_bb_debug("U2 Check Get Add = %x\n", usRegisterAdd);
    rtd_pr_usb_bb_debug("U2 Check Get Value = %x\n", rtd_inl(usRegisterAdd));
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Read Flash (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorReadFlash(void)
{
    unsigned int usLength = 0;
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;
    unsigned int usBankID = 0;
    unsigned int usAddr = 0;

    unsigned char pucUsbBillboardVendorDataBuf[_USB_VENDOR_CMD_MAX_LEN];
    memset(pucUsbBillboardVendorDataBuf, 0, _USB_VENDOR_CMD_MAX_LEN);
    // temp buffer for U2 TX test

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;
    usBankID = (rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) + rtd_inl(P98_0A_HS_EP0_SETUP2);
    usAddr = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);

    memset(pucUsbBillboardVendorDataBuf, 0, _USB_VENDOR_CMD_MAX_LEN);
    // TODO
    //FlashRead(usBankID, (unsigned char *)usAddr, usLength, pucUsbBillboardVendorDataBuf);
    UsbBillboardTransmtDataToHost(usLength, pucUsbBillboardVendorDataBuf);
}

#if(_FLASH_WRITE_FUNCTION_SUPPORT == _ON)
#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
//----------------------------------------------------------------
// Description  : USB Vendor Request - Write Flash (Host->Deivce)
// Input Value  : None
// Output Value : None
//----------------------------------------------------------------
void UsbBillboardVendorWriteFlash(void)
{
    unsigned int usLength = 0;
    unsigned char ucIdx = 0;
    unsigned int usRemainIdx = 0;
    unsigned char ucPageIdx = 0;
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;
    unsigned int usBankID = 0;
    unsigned int usAddr = 0;

    // temp buffer for U2 RX test
    unsigned int usPageTmp = 0;

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;
    usBankID = (rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) + rtd_inl(P98_0A_HS_EP0_SETUP2);
    usAddr = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);

    rtd_pr_usb_bb_debug("U2-WT-usLength = %x\n", usLength);
    rtd_pr_usb_bb_debug("U2-WT-usPageNum = %x\n", usPageNum);
    rtd_pr_usb_bb_debug("U2-WT-usRemainNum = %x\n", usRemainNum);


    // [TBD]: Read the complete 64Byte
    for(ucPageIdx = 0; ucPageIdx < usPageNum; ucPageIdx++)
    {
        usPageTmp = ucPageIdx * _USB_EP0_DATA_BUF_SIZE;
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            for(ucIdx = 0; ucIdx < _USB_EP0_DATA_BUF_SIZE; ucIdx++)
            {
                // ucFlashData[ucIdx + ucPageIdx*_USB_EP0_DATA_BUF_SIZE] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                // rtd_outl(_XRAM_ADDR_START + (ucIdx + usPageTmp), rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                rtd_outl(_HW_XRAM_ISP_MAPPING_ADDR_START + (ucIdx + usPageTmp), rtd_inl(P9A_03_BB_BUF0_RD_DATA));
            }
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }

    // [TBD]: Read the remaining
    if(usRemainNum != 0)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            usRemainIdx = usPageNum * _USB_EP0_DATA_BUF_SIZE;
            for(ucIdx = 0; ucIdx < usRemainNum; ucIdx++)
            {
                rtd_outl(_HW_XRAM_ISP_MAPPING_ADDR_START + usRemainIdx, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                // ucFlashData[usRemainIdx] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                // rtd_outl(_XRAM_ADDR_START + usRemainIdx, rtd_inl(P9A_03_BB_BUF0_RD_DATA));
                // rtd_outl(_XRAM_ADDR_START + usRemainIdx, ucFlashData[usRemainIdx]);
                usRemainIdx++;
            }
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }
    // TODO
    //FlashWritePageProgram(usBankID, usAddr, usLength);
    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, _FALSE);
}
#endif

//-----------------------------------------------------------------
// Description  : USBVendor Request - Flash sector erase (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorSectorErase(void)
{
    unsigned char ucBankID = 0;
    unsigned char ucSectorID = 0;

    ucBankID = rtd_inl(P98_0A_HS_EP0_SETUP2);
    ucSectorID = rtd_inl(P98_0C_HS_EP0_SETUP4);

    // TODO
    //FlashErasePage(ucBankID, ucSectorID);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT0), (_BIT0));
}
#endif

//-----------------------------------------------------------------
// Description  : USBVendor Request - Flash sector erase (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorBankErase(void)
{
/*
    // FlashEraseBank(ucBankID) needs to be implemented.
    unsigned char ucBankID = 0;

    ucBankID = rtd_inl(P98_0A_HS_EP0_SETUP2);
    FlashEraseBank(ucBankID);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT0), (_BIT0));
*/
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Isp Enable (Host -> Device)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorIspEnable(void)
{
    g_bUsbBillboardIspEn = (rtd_inl(P98_0A_HS_EP0_SETUP2) & _BIT0);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT0), (_BIT0));
}

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
//-----------------------------------------------------------------
// Description  : USBVendor Request - Dual Bank Process1 (record steup token packet info)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDualBankProcess1(void)
{
    // Will Be Cleared By UserCommonDualBankUserHandler
    SET_USB_BB_ISP_REV_CMD_STATUS();
    // Will Be Cleared By When Complete a Transfer
    SET_USB_BB_ISP_OPCODE(_USB_VENDOR_DUAL_BANK_PROCESS);

    // Means That Isp Info from UserCommondualBankHandler has not been update During a Given Command
    if(GET_USB_BB_ISP_READY_FLAG() == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
    }

    switch(rtd_inl(P98_0A_HS_EP0_SETUP2))
    {
        case __BOOT_GET_ISP_START_ADDR_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_ISP_START_ADDR_OP_CODE);
            break;

        case __BOOT_GET_ISP_USER_KEY_ADDR_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_ISP_USER_KEY_ADDR_OP_CODE);
            break;

        case  __BOOT_GET_ISP_USER_FLAG_ADDR_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_ISP_USER_FLAG_ADDR_OP_CODE);
            break;

        case __BOOT_GET_ISP_USER_SIGN_ADDR_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_ISP_USER_SIGN_ADDR_OP_CODE);
            break;

        case __BOOT_GET_VERIFY_COPY_RESULT_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_VERIFY_COPY_RESULT_OP_CODE);
            break;

        case __BOOT_GET_ISP_CURRENT_KEY_ADDR_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_ISP_CURRENT_KEY_ADDR_OP_CODE);
            break;

        case __BOOT_GET_DUAL_BANK_INFO_OP_CODE:
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_GET_DUAL_BANK_INFO_OP_CODE);
            break;

        case __BOOT_SWITCH_CODE_OP_CODE:
            UsbBillboardVendorSwitchCode();
            break;
        default:
            break;
    }
}

//-----------------------------------------------------------------
// Description  : USBVendor Request - Dual Bank Process2 (reply isp info to host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDualBankProcess2(void)
{
    switch(rtd_inl(P98_0A_HS_EP0_SETUP2))
    {
        case __BOOT_GET_ISP_START_ADDR_OP_CODE:
            UsbBillboardVendorGetIspStartBank();
            break;

        case __BOOT_GET_ISP_USER_KEY_ADDR_OP_CODE:
            UsbBillboardVendorGetIspKeyAddr();
            break;

        case  __BOOT_GET_ISP_USER_FLAG_ADDR_OP_CODE:
            UsbBillboardVendorGetIspFalgAddr();
            break;

        case __BOOT_GET_ISP_USER_SIGN_ADDR_OP_CODE:
            UsbBillboardVendorGetIspSignatureAddr();
            break;

        case __BOOT_GET_VERIFY_COPY_RESULT_OP_CODE:
            UsbBillboardVendorGetVerifyCopyResult();
            break;

        case __BOOT_GET_ISP_CURRENT_KEY_ADDR_OP_CODE:
            UsbBillboardVendorGetCurrentKeyAddr();
            break;

        case __BOOT_GET_DUAL_BANK_INFO_OP_CODE:
            UsbBillboardVendorGetFWInfo();
            break;

        case __BOOT_SWITCH_CODE_OP_CODE:
            UsbBillboardVendorSwitchCode();
            break;

        default:
            break;
    }
}
//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x01 GetIspStartBank (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetIspStartBank(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x02 GetIspKeyAddr (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetIspKeyAddr(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);


    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x03 GetIspFalgAddr (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetIspFalgAddr(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x04 GetIspSignatureAddr (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetIspSignatureAddr(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x05 GetVerifyCopyResult (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetVerifyCopyResult(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x06 GetCurrentKeyAddr (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetCurrentKeyAddr(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);


    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - Dual Bank Process:0x07 GetFWInfo (Device -> Host)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorGetFWInfo(void)
{
    unsigned char ucLength = 0;
    unsigned char ucIdx = 0;

    // In dual bank process, tool set (P98_0F_HS_EP0_SETUP7) fixed 0,so ucLength = (P98_0E_HS_EP0_SETUP6).
    // also tool setting ucLength will lower than _USB_EP0_DATA_BUF_SIZE(64)
    ucLength = rtd_inl(P98_0E_HS_EP0_SETUP6);

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

    for(ucIdx = 0; ucIdx < ucLength; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardGetISPinfo[ucIdx]);
    }

    // In dual bank process,tool setting ucLength will lower than  _USB_EP0_DATA_BUF_SIZE(64)
    rtd_outl(P98_06_HS_EP0_TX_BC, ucLength);

    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    // BB Send Data Success
    if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1) == _TRUE)
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
    else
    {
        rtd_pr_usb_bb_debug("BB SEND DATA FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
    }
}

//-----------------------------------------------------------------
// Description  : USB Vendor Request - BootCode Use Only. Dual Bank Process:0x08 SwitchCOde (Host -> Device)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorSwitchCode(void)
{
    if(GET_USB_BB_ISP_READY_FLAG() == _FALSE)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0) == _TRUE)
        {
            SET_USB_BB_ISP_SUB_OPCODE(__BOOT_SWITCH_CODE_OP_CODE);
        }
        else
        {
            rtd_pr_usb_bb_debug("Complete Command FAIL = %x\n", GET_USB_BB_ISP_SUB_OPCODE());
        }
    }
    else
    {
        CLR_USB_BB_ISP_READY_FLAG();
        CLR_USB_BB_ISP_OPCODE();
        CLR_USB_BB_ISP_SUB_OPCODE();
    }
}
#if(_DUAL_BANK_DEBUG_SUPPORT == _ON)
//-----------------------------------------------------------------
// Description  : USB BB Get ISP/DualBank Related Info (Host -> Device)
// Input Value  : Soure Info,Info Length,ReadyFlag
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardCopyISPInfo(unsigned char *pucSrcAddr, unsigned char ucLength, unsigned char ucReadyFlag)
{
    // pulSrcAddr data(from UserCommonDualBankUserHandler) copy to g_pucUsbBillboardGetISPinfo
    memcpy(g_pucUsbBillboardGetISPinfo, pucSrcAddr, (unsigned int)ucLength);

    SET_USB_BB_ISP_READY_FLAG(ucReadyFlag);
}
#endif
//--------------------------------------------------
// Description  : Bill Board ISP FLAG INITIAL
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardIspFlagInitial(void)
{
    CLR_USB_BB_ISP_REV_CMD_STATUS();
    CLR_USB_BB_ISP_READY_FLAG();
    CLR_USB_BB_ISP_OPCODE();
    CLR_USB_BB_ISP_SUB_OPCODE();
    CLR_USB_BB_FLASH_WRITE_FLAG();
    memset(g_pucUsbBillboardGetISPinfo, 0x00, sizeof(g_pucUsbBillboardGetISPinfo));
}

#endif

//-----------------------------------------------------------------
// Description  : USBVendor Request - Handshake (Host <-> Device)
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorHandshake(void)
{
    unsigned int usLength = 0;
    unsigned char ucIdx = 0;

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    if(usLength != g_pucUsbBillboardRTHandshake[0])
    {
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
        return;
    }
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    for(ucIdx = 0; ucIdx < g_pucUsbBillboardRTHandshake[0]; ucIdx++)
    {
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pucUsbBillboardRTHandshake[ucIdx + 1]);
    }
    rtd_outl(P98_06_HS_EP0_TX_BC, g_pucUsbBillboardRTHandshake[0]);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgEventStart(void)
{
// TBD for BB Debug.
/*
    g_bBBDebugMessageEven = _TRUE;
    g_usBBFilterPro = (rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) + rtd_inl(P98_0A_HS_EP0_SETUP2);
    rtd_outl(0x00007, rtd_inl(P98_0B_HS_EP0_SETUP3));
    rtd_outl(0x00008, rtd_inl(P98_0A_HS_EP0_SETUP2));
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0xF1);
    rtd_outl(P98_06_HS_EP0_TX_BC, 1);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
*/
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgEventFinish(void)
{
// TBD for BB Debug.
/*
    g_bBBDebugMessageEven = _FALSE;
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0xF5);
    rtd_outl(P98_06_HS_EP0_TX_BC, 1);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
*/
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgStart(void)
{
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P98_06_HS_EP0_TX_BC, 2);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}

//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgEnd(void)
{
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P98_06_HS_EP0_TX_BC, 1);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}

//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgGetValue(void)
{
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P98_06_HS_EP0_TX_BC, 2);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Debug Msg
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorDebugMsgGetString(void)
{
    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
    rtd_outl(P98_06_HS_EP0_TX_BC, 2);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Eeprom Write
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardEepromWrite(bool bSysDefualtIICPin, unsigned char ucSlaveAddr, unsigned char ucSubAddrLength, unsigned int usSubAddr, unsigned int usLength, unsigned char *pucWriteArray)
{
    // Disable EEPROM Write Protect
    // TODO
    //PCB_EEPROM_WRITE_PROTECT(_EEPROM_WP_DISABLE);

    if(!bSysDefualtIICPin)
    {
#if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)

        // TODO
        //SET_SW_IIC_STATUS(_IIC_SYS_EEPROM);

#if(_EEPROM_TYPE == _EEPROM_24LC16)
        // TODO
        //McuIICWrite(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray);
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
        // TODO
        //McuIICWrite(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray);
#endif

#elif(_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN)  // Else of #if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)

#if(_EEPROM_TYPE == _EEPROM_24LC16)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Write(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICWrite(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _PCB_SYS_EEPROM_IIC);
// #endif
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Write(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray _VGA_I2C);
// #else
        McuHwIICWrite(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _PCB_SYS_EEPROM_IIC);
// #endif
#endif
#endif  // End of #if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)
    }
    else
    {
#if((_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN) && (_PCB_SYS_EEPROM_IIC != _SW_IIC_PIN_GPIO))

#if(_EEPROM_TYPE == _EEPROM_24LC16)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Write(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICWrite(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _NO_IIC_PIN);
// #endif
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Write(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICWrite(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _NO_IIC_PIN);
// #endif
#endif

#endif // End of #if(_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN) && (_PCB_SYS_EEPROM_IIC != _SW_IIC_PIN_GPIO)

    }

    // Enable EEPROM Write Protect
    // TODO
    //PCB_EEPROM_WRITE_PROTECT(_EEPROM_WP_ENABLE);
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Eeprom Read
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardEepromRead(bool bSysDefualtIICPin, unsigned char ucSlaveAddr, unsigned char ucSubAddrLength, unsigned int usSubAddr, unsigned int usLength, unsigned char *pucWriteArray)
{

    if(bSysDefualtIICPin)
    {
#if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)

        // TODO
        //SET_SW_IIC_STATUS(_IIC_SYS_EEPROM);

#if(_EEPROM_TYPE == _EEPROM_24LC16)
        // TODO
        //McuIICRead(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray);
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
        // TODO
        //McuIICRead(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray);
#endif

#elif(_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN)  // Else of #if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)

#if(_EEPROM_TYPE == _EEPROM_24LC16)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Read(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICRead(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _PCB_SYS_EEPROM_IIC);
// #endif
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Read(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray _VGA_I2C);
// #else
        McuHwIICRead(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _PCB_SYS_EEPROM_IIC);
// #endif
#endif
#endif  // End of #if(_PCB_SYS_EEPROM_IIC == _SW_IIC_PIN_GPIO)
    }
    else
    {
#if((_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN) && (_PCB_SYS_EEPROM_IIC != _SW_IIC_PIN_GPIO))

#if(_EEPROM_TYPE == _EEPROM_24LC16)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Read(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICRead(ucSlaveAddr, 1, usSubAddr, usLength, pucWriteArray, _NO_IIC_PIN);
// #endif
#elif(_EEPROM_TYPE == _EEPROM_24WC64)
// #if(_HW_TRANSLATOR_IIC_SUPPORT == _ON)
        // McuHwIIC0Read(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _VGA_I2C);
// #else
        McuHwIICRead(ucSlaveAddr, 2, usSubAddr, usLength, pucWriteArray, _NO_IIC_PIN);
// #endif
#endif

#endif // End of #if(_PCB_SYS_EEPROM_IIC != _NO_IIC_PIN) && (_PCB_SYS_EEPROM_IIC != _SW_IIC_PIN_GPIO)
    }
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Write Sys Eeprom
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorWriteSyseeprom(void)
{
    unsigned int usLength = 0;
    unsigned char ucIdx = 0;
    unsigned char ucPageIdx = 0;
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;
    unsigned char ucSlaveAddr = 0;
    bool bOterIICPin = 0;
    unsigned char ucSubAddeLen = 0;
    unsigned int usSubAddr = 0;
    unsigned int usRemainIdx = 0;

    unsigned char pucUsbBillboardVendorDataBuf[_USB_VENDOR_CMD_MAX_LEN];
    memset(pucUsbBillboardVendorDataBuf, 0, _USB_VENDOR_CMD_MAX_LEN);

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);
    ucSlaveAddr = rtd_inl(P98_0A_HS_EP0_SETUP2);

    bOterIICPin = rtd_inl(P98_0B_HS_EP0_SETUP3) & _BIT2;
    ucSubAddeLen = rtd_inl(P98_0B_HS_EP0_SETUP3) & (_BIT1 | _BIT0);
    usSubAddr = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);

    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;

    // temp buffer for U2 RX test
    for(ucPageIdx = 0; ucPageIdx < usPageNum; ucPageIdx++)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            for(ucIdx = 0; ucIdx < _USB_EP0_DATA_BUF_SIZE; ucIdx++)
            {
                pucUsbBillboardVendorDataBuf[ucIdx + ucPageIdx * _USB_EP0_DATA_BUF_SIZE] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
            }
            // [TBD]: Write flash...
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }

    // [TBD]: Read the remaining
    if(usRemainNum != 0)
    {
        if(UsbBillboardCtrlXfer(_USB_CTRL_XFER_WRITE_DATA_STAGE, _FALSE))
        {
            // Set read fifo to receive host data
            rtd_outl(P9A_02_BB_BUF0_RD_ADDR, 0x00);
            usRemainIdx = usPageNum * _USB_EP0_DATA_BUF_SIZE;
            for(ucIdx = 0; ucIdx < usRemainNum; ucIdx++)
            {
                pucUsbBillboardVendorDataBuf[usRemainIdx] = rtd_inl(P9A_03_BB_BUF0_RD_DATA);
                usRemainIdx++;
            }
        }
        else
        {
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            return;
        }
    }
    // start write eeprom
    UsbBillboardEepromWrite(bOterIICPin, ucSlaveAddr, ucSubAddeLen, usSubAddr, usLength, pucUsbBillboardVendorDataBuf);
    // end write eeprom
    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, _FALSE);
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Read Sys Eeprom
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorReadSyseeprom(void)
{
    unsigned int usLength = 0;
    unsigned char ucSlaveAddr = 0;
    bool bOterIICPin = 0;
    unsigned char ucSubAddeLen = 0;
    unsigned int usSubAddr = 0;

    unsigned char pucUsbBillboardVendorDataBuf[_USB_VENDOR_CMD_MAX_LEN];

    memset(pucUsbBillboardVendorDataBuf, 0, _USB_VENDOR_CMD_MAX_LEN);

    // temp buffer for U2 TX test
    ucSlaveAddr = rtd_inl(P98_0A_HS_EP0_SETUP2);
    bOterIICPin = rtd_inl(P98_0B_HS_EP0_SETUP3) & _BIT2;
    ucSubAddeLen = rtd_inl(P98_0B_HS_EP0_SETUP3) & (_BIT1 | _BIT0);
    usSubAddr = (rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) + rtd_inl(P98_0C_HS_EP0_SETUP4);

    usLength = (rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) + rtd_inl(P98_0E_HS_EP0_SETUP6);

    UsbBillboardEepromRead(bOterIICPin, ucSlaveAddr, ucSubAddeLen, usSubAddr, usLength, pucUsbBillboardVendorDataBuf);
    UsbBillboardTransmtDataToHost(usLength, pucUsbBillboardVendorDataBuf);
}


//-----------------------------------------------------------------
// Description  : USBVendor Request - Transfer Data to Host
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardTransmtDataToHost(unsigned int usLength, unsigned char *pucDataBuf)
{
    unsigned int usPageNum = 0;
    unsigned int usRemainNum = 0;
    unsigned char ucIdx = 0;
    unsigned char ucPageIdx = 0;
    unsigned int usRemainIdx = 0;

    usPageNum = usLength / _USB_EP0_DATA_BUF_SIZE;
    usRemainNum = usLength % _USB_EP0_DATA_BUF_SIZE;

    for(ucPageIdx = 0; ucPageIdx < usPageNum; ucPageIdx++)
    {
        // [TBD]:Read data from flash...

        // Set write fifo to transmit data to host
        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
        for(ucIdx = 0; ucIdx < _USB_EP0_DATA_BUF_SIZE; ucIdx++)
        {
            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucDataBuf[ucIdx + ucPageIdx * _USB_EP0_DATA_BUF_SIZE]);
        }
        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
    }

    if(usRemainNum == 0)
    {
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
    }
    else
    {
        // [TBD]:Read remaining data from flash...

        usRemainIdx = usPageNum * _USB_EP0_DATA_BUF_SIZE;
        // Set write fifo to transmit data to host
        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
        for(ucIdx = 0; ucIdx < usRemainNum; ucIdx++)
        {
            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucDataBuf[usRemainIdx]);
            usRemainIdx++;
        }
        rtd_outl(P98_06_HS_EP0_TX_BC, usRemainNum);
        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
    }
}



//-----------------------------------------------------------------
// Description  : USBVendor Request
// Input Value  : None
// Output Value : None
//-----------------------------------------------------------------
void UsbBillboardVendorRequest(void)
{
    rtd_pr_usb_bb_debug("U2 Ven Request = %x\n", rtd_inl(P98_09_HS_EP0_SETUP1));
    rtd_pr_usb_bb_debug("U2 Ven wVALL = %x\n", rtd_inl(P98_0A_HS_EP0_SETUP2));
    rtd_pr_usb_bb_debug("U2 Ven wVALH = %x\n", rtd_inl(P98_0B_HS_EP0_SETUP3));
    rtd_pr_usb_bb_debug("U2 Ven wIDXL = %x\n", rtd_inl(P98_0C_HS_EP0_SETUP4));
    rtd_pr_usb_bb_debug("U2 Ven wIDXH = %x\n", rtd_inl(P98_0D_HS_EP0_SETUP5));
    rtd_pr_usb_bb_debug("U2 Ven wLENL = %x\n", rtd_inl(P98_0E_HS_EP0_SETUP6));
    rtd_pr_usb_bb_debug("U2 Ven wLENH = %x\n", rtd_inl(P98_0F_HS_EP0_SETUP7));

    switch(rtd_inl(P98_09_HS_EP0_SETUP1))
    {
        case _USB_VENDOR_SET_REGISTER_VALUE:
            UsbBillboardVendorSetRegisterByte();
            break;

        case _USB_VENDOR_GET_REGISTER_VALUE:
            UsbBillboardVendorGetRegisterByte();
            break;

        case _USB_VENDOR_WRITE_SYSEEPROM:
            UsbBillboardVendorWriteSyseeprom();
            break;

        case _USB_VENDOR_READ_SYSEEPROM:
            UsbBillboardVendorReadSyseeprom();
            break;

        case _USB_VENDOR_WRITE_TO_FLASH:
#if((_FLASH_WRITE_FUNCTION_SUPPORT == _ON) && (_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON))
            SET_USB_BB_FLASH_WRITE_FLAG();
            UsbBillboardVendorWriteFlash();
            CLR_USB_BB_FLASH_WRITE_FLAG();
#endif
            break;

        case _USB_VENDOR_READ_FROM_FLASH:
            UsbBillboardVendorReadFlash();
            break;

        case _USB_VENDOR_FLASH_SECTOR_ERASE:
#if(_FLASH_WRITE_FUNCTION_SUPPORT == _ON)
            UsbBillboardVendorSectorErase();
#endif
            break;

        case _USB_VENDOR_FLASH_BANK_ERASE:
            UsbBillboardVendorBankErase();
            break;
        case _USB_VENDOR_ISP_ENABLE:
            UsbBillboardVendorIspEnable();
            break;

        case _USB_VENDOR_DEBUGMSG_EVENT_START:
            UsbBillboardVendorDebugMsgEventStart();
            break;

        case _USB_VENDOR_DEBUGMSG_EVENT_FINISH:
            UsbBillboardVendorDebugMsgEventFinish();
            break;

        case _USB_VENDOR_DEBUGMSG_START:
            UsbBillboardVendorDebugMsgStart();
            break;

        case _USB_VENDOR_DEBUGMSG_END:
            UsbBillboardVendorDebugMsgEnd();
            break;

        case _USB_VENDOR_DEBUGMSG_GET_VALUE:
            UsbBillboardVendorDebugMsgGetValue();
            break;

        case _USB_VENDOR_DEBUGMSG_GET_STRING:
            UsbBillboardVendorDebugMsgGetString();
            break;

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
        case _USB_VENDOR_DUAL_BANK_PROCESS:
            UsbBillboardVendorDualBankProcess1();
            break;
#endif

        case _USB_VENDOR_REALTEK_BB_HANDSHAKE:
            UsbBillboardVendorHandshake();
            break;

        default:
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;
    }
}


//--------------------------------------------------
// Description  : Bill Board PLL Power
// Input Value  : Action
// Output Value : None
//--------------------------------------------------
void UsbBillboardPllPower(EnumBillboardCtrl enumAtion)
{
    if(enumAtion == _USB_BB_ENABLE)
    {
        // Xtal has power down
        /*if((rtd_inl(P80_0E_XTAL_CTRL0) & _BIT7) == 0x00)
        {
            // Bit4~3 : 00 --> XI/XO Pad Driving to Strong
            rtd_maskl(P80_0F_XTAL_CTRL1, ~(_BIT4 | _BIT3), 0x00);

            // Power On External Xtal
            rtd_maskl(P80_0E_XTAL_CTRL0, ~_BIT7, _BIT7);

            // Waiting for xtal stable.
            mdelay(6);
        }*/

        // Bit4~3 : 00 --> XI/XO Pad Driving to weak
        //rtd_maskl(P80_0F_XTAL_CTRL1, ~(_BIT4 | _BIT3), (_BIT4 | _BIT3));

        // Enable BB PLL Power
        //rtd_maskl(P64_C6_CMU480_PLL_DIV_CFG0, ~_BIT6, _BIT6);

        // Delay Time us [300,x] Waiting for BB PLL Stable
        rtd_outl(P64_00_HS_USP_CTL, 0x00);
	udelay(10);
	rtd_outl(P64_74_HS_USB2_RESET, 0x00);
	udelay(10);
	 // Bill Board Connect Enable
         rtd_outl(P64_00_HS_USP_CTL, 0x01);
	udelay(10);
	rtd_outl(P64_74_HS_USB2_RESET, 0x03);
	udelay(10);
        
    }
    else if(enumAtion == _USB_BB_DISABLE)
    {
        // Disable BB PLL Power
        //rtd_maskl(P64_C6_CMU480_PLL_DIV_CFG0, ~_BIT6, 0x00);

        /*if((rtd_inl(P1D_14_BANDGAP) & _BIT7) == 0x00)
        {
            // Power down External Xtal
            rtd_maskl(P80_0E_XTAL_CTRL0, ~_BIT7, 0x00);

            // Bit4~3 : 00 --> XI/XO Pad Driving to weak
            rtd_maskl(P80_0F_XTAL_CTRL1, ~(_BIT4 | _BIT3), (_BIT4 | _BIT3));
        }*/

        rtd_outl(P64_00_HS_USP_CTL, 0x00);
	udelay(10);
	rtd_outl(P64_74_HS_USB2_RESET, 0x00);
	udelay(10);
    }
}

//--------------------------------------------------
// Description  : USB Standard Request Get Descriptor
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardGetDescriptor(void)
{
    unsigned int usCount = 0;
    unsigned int usLoop = 0;
    unsigned int usData = 0;
    unsigned char *pucIndex = NULL;
    unsigned char ucCount = 0;
    unsigned char ucCurAltModeNum = 0;
    unsigned char ucAltModeNumLoop = 0;
    unsigned char ucLoop = 0;
    unsigned char ucBmConfigured = 0;
    unsigned int usBOSCurLength = 0;
    unsigned char ucBOSCurNumDeviceCaps = 0;

    usCount = rtd_inl(P98_0E_HS_EP0_SETUP6);

    // Judge Descriptor Type in the high byte
    switch(rtd_inl(P98_0B_HS_EP0_SETUP3))
    {
        //=========================================
        case _USB_DEVICE_DESCRIPTOR_TYPE:

            // wLength = valid device descriptor bLength
            if(usCount > 18)
            {
                usCount = 18;
            }

            pucIndex = &tUsbBillboardUsbDeviceDescriptor[0];

            rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
            for(PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
            {
                // return device descriptor
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[PDATA_WORD]);
            }

            // When a hub works as full-speed,
            // bDeviceProtocol should be reported as zero
            if((rtd_inl(P64_04_HS_USP_STS) & _BIT0) == 0x00)
            {
                rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x06);
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
            }

            rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
            UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
            // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
            break;   // end of USB_DEVICE_DESCRIPTOR_TYPE

        //=========================================
        case _USB_CONFIGURATION_DESCRIPTOR_TYPE:
        case _USB_OTHER_SPEED_CONFIGURATION_TYPE:

            // descriptor index (wValue Low byte) must be checked
            if(usCount > _USB_DESC_SIZE_CONF_TOTAL)
            {
                usCount = _USB_DESC_SIZE_CONF_TOTAL;
            }

            pucIndex = &tUsbBillboardUsbConfigDescriptor[0];

            for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
            {
                // for USB_OTHER_SPEED_CONFIGURATION_TYPE Descriptor
                if((PDATA_WORD == 1) && (rtd_inl(P98_0B_HS_EP0_SETUP3) == _USB_OTHER_SPEED_CONFIGURATION_TYPE))
                {
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, PDATA_WORD);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, _USB_OTHER_SPEED_CONFIGURATION_TYPE);
                    continue;
                }

                if((((rtd_inl(P64_04_HS_USP_STS) & _BIT0) == 0x00) && (rtd_inl(P98_0B_HS_EP0_SETUP3) == _USB_CONFIGURATION_DESCRIPTOR_TYPE)) ||
                   (((rtd_inl(P64_04_HS_USP_STS) & _BIT0) == _BIT0) && (rtd_inl(P98_0B_HS_EP0_SETUP3) == _USB_OTHER_SPEED_CONFIGURATION_TYPE)))
                {
                    // bInterval of the first endpoint descriptor
                    if(PDATA_WORD == 0x18)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, PDATA_WORD);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0xFF);
                        continue;
                    }
                }

                rtd_outl(P9A_00_BB_BUF0_WR_ADDR, PDATA_WORD);
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[PDATA_WORD]);
            }

            rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
            UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
            // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
            break;// end of USB_CONFIGURATION_DESCRIPTOR_TYPE

        //=========================================
        case _USB_STRING_DESCRIPTOR_TYPE:

            switch(rtd_inl(P98_0A_HS_EP0_SETUP2))    // descriptor index
            {
                case _USB_INDEX0:    // language ID

                    // if(usCount > sizeof(g_stUsbBillboardUsbStringDescriptor))
                    // {
                    // usCount = sizeof(g_stUsbBillboardUsbStringDescriptor);
                    // }

                    // Force to reply 4Byte for [usb cv]Billboard test Fail
                    usCount = _USB_DESC_SIZE_CPLS;

                    pucIndex = &tUsbBillboardUsbStringDescriptor[0];

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for(PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
                    break;

                case _TYPE_C_BILLBOARD_URL_INDEX_DP:

                    usData = 0;

                    if((unsigned int)tBillboardUrl_DP[0] < usCount)
                    {
                        usCount = (unsigned int)tBillboardUrl_DP[0];
                    }

                    for(usLoop = 0; usLoop < usCount; usLoop++)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tBillboardUrl_DP[usLoop]);
                        if(usData == (_USB_EP0_DATA_BUF_SIZE - 1))
                        {
                            rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
                            if(usLoop == (usCount - 1))
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                            }
                            else
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                            }
                            usData = 0;
                        }
                        else
                        {
                            usData++;
                        }
                    }

                    if(usData > 0)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, usData);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                    }
                    break;

                case _TYPE_C_BILLBOARD_URL_INDEX_LENOVO:

                    usData = 0;
                    if((unsigned int)tBillboardUrl_Lenovo[0] < usCount)
                    {
                        usCount = (unsigned int)tBillboardUrl_Lenovo[0];
                    }

                    for(usLoop = 0; usLoop < usCount; usLoop++)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tBillboardUrl_Lenovo[usLoop]);
                        if(usData == (_USB_EP0_DATA_BUF_SIZE - 1))
                        {
                            rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
                            if(usLoop == (usCount - 1))
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                            }
                            else
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                            }
                            usData = 0;
                        }
                        else
                        {
                            usData++;
                        }
                    }

                    if(usData > 0)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, usData);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                    }

                    break;

                case _TYPE_C_BILLBOARD_URL_INDEX_IADDTIONALINFO:    // BB Capability Descriptor iAddtionalInfoURL

                    usData = 0;
                    if((unsigned int)tBillboardUrl_iAddtionalInfo[0] < usCount)
                    {
                        usCount = (unsigned int)tBillboardUrl_iAddtionalInfo[0];
                    }

                    for(usLoop = 0; usLoop < usCount; usLoop++)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tBillboardUrl_iAddtionalInfo[usLoop]);
                        if(usData == (_USB_EP0_DATA_BUF_SIZE - 1))
                        {
                            rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
                            if(usLoop == (usCount - 1))
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                            }
                            else
                            {
                                UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                            }
                            usData = 0;
                        }
                        else
                        {
                            usData++;
                        }
                    }

                    if(usData > 0)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, usData);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                    }

                    break;

                case _USB_INDEX1:    // iManufacturer

                    if((unsigned int)tManufacturer[0] < usCount)
                    {
                        usCount = (unsigned int)tManufacturer[0];
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for(PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tManufacturer[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
                    break;

                case _USB_INDEX2:    // iProduct

                    if((unsigned int)tProduct[0] < usCount)
                    {
                        usCount = (unsigned int)tProduct[0];
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tProduct[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

                    break;

                case _USB_INDEX3:

                    if((unsigned int)tSerialNumber[0] < usCount)
                    {
                        usCount = (unsigned int)tSerialNumber[0];
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tSerialNumber[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

                    break;

                case _USB_BB_INDEX1:    // iManufacturer

                    if(g_bUsbBillboardLoadCustomManufactory == _TRUE)
                    {
                        if((unsigned int)tUsbBillboardManufacturer_Custom[0] < usCount)
                        {
                            usCount = (unsigned int)tUsbBillboardManufacturer_Custom[0];
                        }

                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                        for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, tUsbBillboardManufacturer_Custom[PDATA_WORD]);
                        }
                    }
                    else
                    {
                        if((unsigned int)tUsbBillboardManufacturer[0] < usCount)
                        {
                            usCount = (unsigned int)tUsbBillboardManufacturer[0];
                        }

                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                        for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, tUsbBillboardManufacturer[PDATA_WORD]);
                        }
                    }


                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

                    break;

                case _USB_BB_INDEX2:    // iProduct

                    if(g_bUsbBillboardLoadCustomProduct == _TRUE)
                    {
                        if((unsigned int)tUsbBillboardProduct_Custom[0] < usCount)
                        {
                            usCount = (unsigned int)tUsbBillboardProduct_Custom[0];
                        }

                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                        for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, tUsbBillboardProduct_Custom[PDATA_WORD]);
                        }
                    }
                    else
                    {
                        if((unsigned int)tUsbBillboardProduct[0] < usCount)
                        {
                            usCount = (unsigned int)tUsbBillboardProduct[0];
                        }

                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                        for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, tUsbBillboardProduct[PDATA_WORD]);
                        }
                    }
                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
                    break;

                case _USB_BB_INDEX3:

                    if((unsigned int)tBBSerialNumber[0] < usCount)
                    {
                        usCount = (unsigned int)tBBSerialNumber[0];
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tBBSerialNumber[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

                    break;

                case _USB_RT_HUB_STRING_INDEX0:

                    if((rtd_inl(P98_0C_HS_EP0_SETUP4) != 0) || (rtd_inl(P98_0D_HS_EP0_SETUP5) != 0))
                    {
                        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
                        break;
                    }

                    if((unsigned int)tRTString[0] < usCount)
                    {
                        usCount = (unsigned int)tRTString[0];
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    for (PDATA_WORD = 0; PDATA_WORD < usCount; PDATA_WORD++)
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, tRTString[PDATA_WORD]);
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                    // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

                    break;

                default:   // undefined string index
                    // NAK if set STALL only. Force to enable handshake.
                    rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL | _USB_EP0_STATUS_HS_EN);
                    mdelay(5);
                    rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                    break;
            }

            break;   // end of USB_STRING_DESCRIPTOR_TYPE


        //=========================================
        case _USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE:

            if(usCount > 0x0A)
            {
                usCount = 0x0A;
            }

            rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
            for (usLoop = 0; usLoop < usCount; usLoop++)
            {
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, tBBDeviceQualifier[usLoop]);
            }

            rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
            UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
            // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));

            break;   // end of USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE

        //===========================================
        case _USB_BOS_DESCRIPTOR_TYPE:
            // The BCD of High/Full Speed of a supporting super-speed device shall set to "0210".
            // If the BCD of device descriptor is "0210" or higher, the BOS Descriptor need to be replied.
            // So I set the BCD as "0210" and will return the  BOS Descriptor request.

            // Get svid, altmode, altmodestring, Vdo info
            UsbBillboardGetAltModeInfo();

            // Get current alt mode num
            for(ucCount = 0; ucCount < _USB_MAX_ALTERNATE_MODE_NUM; ucCount++)
            {
                ucCurAltModeNum += g_pstUsbBillboardAltModeInfo[ucCount].ucCurAltModeActive;
            }

            // Indpendent of TypeC to Make/Check BB expose Normally. ex.BB Debug use / Verify / Program...
            if(ucCurAltModeNum == 0)
            {
                ucCurAltModeNum = _USB_MAX_ALTERNATE_MODE_NUM;
            }

            // Update current NumDeviceCaps
            ucBOSCurNumDeviceCaps = tUsbBillboardBOSDeviceCapabilityDescriptor[4] + ucCurAltModeNum;
            rtd_pr_usb_bb_debug("ucBOSCurNumDeviceCaps = %x\n", ucBOSCurNumDeviceCaps);

            // Get current BOS total Length
            usBOSCurLength = (_USB_BB_BOS_DESC_FIRST_PART_LEN + (_USB_DESC_SIZE_ALTERNATE_MODE + _USB_DESC_SIZE_BILLBOARD_ALTMODE_CAP) * ucCurAltModeNum);

            // wLength = valid device descriptor bLength
            if(usCount > usBOSCurLength)
            {
                usCount = usBOSCurLength;
            }

            pucIndex = &tUsbBillboardBOSDeviceCapabilityDescriptor[0];

            // Get current BOS total Length
            usBOSCurLength = (_USB_BB_BOS_DESC_FIRST_PART_LEN + (_USB_DESC_SIZE_ALTERNATE_MODE + _USB_DESC_SIZE_BILLBOARD_ALTMODE_CAP) * ucCurAltModeNum);

            if(usCount == 0x05)
            {
                rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                for(usLoop = 0; usLoop < usCount; usLoop++)
                {
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop]);
                }

                rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                UsbBillboardXferForGetDescriptor(_USB_CTRL_XFER_READ, _TRUE);
                // rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
            }
            else if(usCount >= usBOSCurLength)
            {
                usData = 0;

                // Step1 : Reply BOS + USB2.0 Extension + SuperSpeed USB + ContainID + BillBoard Capability descriptor(offset0-43)
                for (usLoop = 0; usLoop < _USB_BB_BOS_DESC_FIRST_PART_LEN; usLoop++)
                {
                    // Configure legacy BOS descriptors (USB2 EXT Cap, Superspeed Cap, Container ID) and Billboard Capability Descriptor (Byte 0~43)
                    // We could modify bmConfigured in Billboard Capability Descriptor
                    // i = 50~81 for 128 Alternate modes
                    // usloop = 0x02/0x03:update BOS curruent total length.
                    // usloop = 0x04:update BOS curruent total bNumDeviceCaps.
                    // usloop = 0x2A: update BB CAP CurLength.
                    // usloop = 0x2E: update Support altmode NUM.
                    // usloop = 0x32/0x33: update altmode status
                    // Note:BB can send up to Max 8 valid altmode info if as below(process loop 0x32/0x33)(Cai)
                    if(usLoop == 0x02)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength % 256));
                    }
                    else if(usLoop == 0x03)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength / 256));
                    }
                    else if(usLoop == 0x04)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBOSCurNumDeviceCaps);
                    }
                    else if(usLoop == _USB_BB_CAP_LEN_OFFSEET_IN_BOS)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, (_USB_DESC_SIZE_BILLBOARD_CAP + _USB_DESC_SIZE_ALTERNATE_MODE * ucCurAltModeNum));
                    }
                    else if(usLoop == _USB_BB_CAP_ALTMODE_NUM_OFFSET_IN_BOS)
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucCurAltModeNum);
                    }
                    else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_1_4)
                    {
                        // Reply first 4 bit pair (first Byte) indicating the state of the specified alt mode.
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);

                        ucAltModeNumLoop = ((ucCurAltModeNum > 0x04) ? 0x04 : ucCurAltModeNum);
                        for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                        {
                            ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[ucCount].ucAltModeStauts) & 0x03) << (ucCount * 2));
                        }
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                        ucBmConfigured = 0;
                    }
                    else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_5_8)
                    {
                        // Reply second 4 bit pair (second Byte) indicating the state of the specified alt mode.
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        if(ucCurAltModeNum > 0x04)
                        {
                            ucAltModeNumLoop = ucCurAltModeNum - 4;
                            for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                            {
                                // ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCount + 4)].ucAltModeStauts) & 0x03) << (ucCount * 2));
                                ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCurAltModeNum - (ucAltModeNumLoop - ucCount))].ucAltModeStauts) & 0x03) << (ucCount * 2));
                            }
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                            ucBmConfigured = 0;
                        }
                        else
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
                        }
                    }
                    else
                    {
                        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop]);
                    }

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }
                }

                // Step2 : Reply BillBoard Capability descriptor[offset44 -> (44+n*4), n = ucCurAltModeNum]
                for (usLoop = 0; usLoop < ucCurAltModeNum; usLoop++)
                {
                    // Configure Billboard Capability Descriptor (Byte 44~47*(_USB_NUM_ALTERNATE_MODE*4))
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucSVID_LO);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucSVID_HI);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucMode);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucStringIndex);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }
                }

                // Step3:Reply BillBoard AltMode Capability descriptor(Describe additional properties for a given alternate mode)
                for (usLoop = 0; usLoop < ucCurAltModeNum; usLoop++)
                {
                    // -------- BB AltModeCapabilityDescriptor ucLength(Constant) ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucLength);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- BB AltModeCapabilityDescriptor ucDescriptorType(Constant) ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucDescriptorType);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- BB AltModeCapabilityDescriptor ucDevCapabilityType(Constant) ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucDevCapabilityType);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- BB AltModeCapabilityDescriptor ucIndex ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, (g_stUsbBillboardAltModeCapabilityDescriptor.ucIndex + usLoop));

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- AlternateModeVdo Byte0 ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucdwAlternateModeVdo_Byte0);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- AlternateModeVdo Byte1 ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucdwAlternateModeVdo_Byte1);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- AlternateModeVdo Byte2 ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucdwAlternateModeVdo_Byte2);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }

                    // -------- AlternateModeVdo Byte3 ---------
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, usData);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[usLoop].ucdwAlternateModeVdo_Byte3);

                    if(usData == _USB_EP0_DATA_BUF_SIZE - 1)
                    {
                        rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);

                        if(usLoop == ucCurAltModeNum - 1)
                        {
                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                        }
                        else
                        {
                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                        }
                        usData = 0;
                    }
                    else
                    {
                        usData++;
                    }
                }

                // Reply last packet(length = usdata) if 0 < remaing data(usdata) < _USB_EP0_DATA_BUF_SIZE Bytes
                if(usData > 0)
                {
                    rtd_outl(P98_06_HS_EP0_TX_BC, usData);

                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                }
            }
            else
            {
                rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

                // _USB_EP0_DATA_BUF_SIZE = 64 is BB Endpoint Buffer, Reply BOS(5)+U2EXTENSION(7)+CONTAINID(20)+32 of BBCAP(44+4x) at Most
                if(usCount <= _USB_EP0_DATA_BUF_SIZE)
                {
                    for(usLoop = 0; usLoop < usCount; usLoop++)
                    {
                        if(usLoop == 0x02)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength % 256));
                        }
                        else if(usLoop == 0x03)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength / 256));
                        }
                        else if(usLoop == 0x04)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBOSCurNumDeviceCaps);
                        }
                        else if(usLoop == _USB_BB_CAP_LEN_OFFSEET_IN_BOS)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (_USB_DESC_SIZE_BILLBOARD_CAP + _USB_DESC_SIZE_ALTERNATE_MODE * ucCurAltModeNum));
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_NUM_OFFSET_IN_BOS)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucCurAltModeNum);
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_1_4)
                        {
                            // Reply first 4 bit pair (first Byte) indicating the state of the specified alt mode.
                            ucAltModeNumLoop = ((ucCurAltModeNum > 0x04) ? 0x04 : ucCurAltModeNum);
                            for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                            {
                                ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[ucCount].ucAltModeStauts) & 0x03) << (ucCount * 2));
                            }
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                            ucBmConfigured = 0;
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_5_8)
                        {
                            // Reply second 4 bit pair (second Byte) indicating the state of the specified alt mode.
                            if(ucCurAltModeNum > 0x04)
                            {
                                ucAltModeNumLoop = ucCurAltModeNum - 4;
                                for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                                {
                                    // ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCount + 4)].ucAltModeStauts) & 0x03) << (ucCount * 2));
                                    ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCurAltModeNum - (ucAltModeNumLoop - ucCount))].ucAltModeStauts) & 0x03) << (ucCount * 2));
                                }
                                rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                                ucBmConfigured = 0;
                            }
                            else
                            {
                                rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
                            }
                        }
                        else
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop]);
                        }
                    }

                    // For reply 64B Case,I dont know why Normal Host will indicate stall info if not add the 2nd/3rd line. But below will make CTS Host abnormally
                    // rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
                    // UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);
                    // rtd_outl(P98_06_HS_EP0_TX_BC, 0);
                    // UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);

                    rtd_outl(P98_06_HS_EP0_TX_BC, usCount);
                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                }
                else // support Max 4 Altmode all ON
                {
                    for(usLoop = 0; usLoop < _USB_EP0_DATA_BUF_SIZE; usLoop++)
                    {
                        if(usLoop == 0x02)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength % 256));
                        }
                        else if(usLoop == 0x03)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (usBOSCurLength / 256));
                        }
                        else if(usLoop == 0x04)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBOSCurNumDeviceCaps);
                        }
                        else if(usLoop == _USB_BB_CAP_LEN_OFFSEET_IN_BOS)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (_USB_DESC_SIZE_BILLBOARD_CAP + _USB_DESC_SIZE_ALTERNATE_MODE * ucCurAltModeNum));
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_NUM_OFFSET_IN_BOS)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucCurAltModeNum);
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_1_4)
                        {
                            // Reply first 4 bit pair (first Byte) indicating the state of the specified alt mode.
                            ucAltModeNumLoop = ((ucCurAltModeNum > 0x04) ? 0x04 : ucCurAltModeNum);
                            for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                            {
                                ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[ucCount].ucAltModeStauts) & 0x03) << (ucCount * 2));
                            }
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                            ucBmConfigured = 0;
                        }
                        else if(usLoop == _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_5_8)
                        {
                            // Reply second 4 bit pair (second Byte) indicating the state of the specified alt mode.
                            if(ucCurAltModeNum > 0x04)
                            {
                                ucAltModeNumLoop = ucCurAltModeNum - 4;
                                for(ucCount = 0; ucCount < ucAltModeNumLoop; ucCount++)
                                {
                                    // ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCount + 4)].ucAltModeStauts) & 0x03) << (ucCount * 2));
                                    ucBmConfigured += (((g_pstUsbBillboardAltModeInfo[(ucCurAltModeNum - (ucAltModeNumLoop - ucCount))].ucAltModeStauts) & 0x03) << (ucCount * 2));
                                }
                                rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucBmConfigured);
                                ucBmConfigured = 0;
                            }
                            else
                            {
                                rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
                            }
                        }
                        else
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop]);
                        }
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, _USB_EP0_DATA_BUF_SIZE);
                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 0);

                    // Reset BB Buf start addr
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);

                    // Reply info which exceed _USB_EP0_DATA_BUF_SIZE length _ fixed Part
                    if((usCount - _USB_EP0_DATA_BUF_SIZE) <= (_USB_BB_BOS_DESC_FIRST_PART_LEN - _USB_EP0_DATA_BUF_SIZE))
                    {
                        for(usLoop = 0; usLoop < (usCount - _USB_EP0_DATA_BUF_SIZE); usLoop++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop + _USB_EP0_DATA_BUF_SIZE]);
                        }
                    }
                    else // Reply info which exceed _USB_EP0_DATA_BUF_SIZE length _ fixed Part + Variable Part
                    {
                        for(usLoop = 0; usLoop < (_USB_BB_BOS_DESC_FIRST_PART_LEN - _USB_EP0_DATA_BUF_SIZE); usLoop++)
                        {
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, pucIndex[usLoop + _USB_EP0_DATA_BUF_SIZE]);
                        }

                        for(ucLoop = 0; ucLoop < ucCurAltModeNum; ucLoop++)
                        {
                            // Reply BillBoard Capability descriptor[offset44 -> (44+n*4)  n = ucCurAltModeNum
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucSVID_LO);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucSVID_HI);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucMode);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucStringIndex);
                        }

                        for(ucLoop = 0; ucLoop < ucCurAltModeNum; ucLoop++)
                        {
                            // Reply BillBoard AltMode Capability descriptor(Describe additional properties for a given alternate mode
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucLength);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucDescriptorType);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_stUsbBillboardAltModeCapabilityDescriptor.ucDevCapabilityType);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, (g_stUsbBillboardAltModeCapabilityDescriptor.ucIndex + ucLoop));
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucdwAlternateModeVdo_Byte0);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucdwAlternateModeVdo_Byte1);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucdwAlternateModeVdo_Byte2);
                            rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_pstUsbBillboardAltModeInfo[ucLoop].ucdwAlternateModeVdo_Byte3);
                        }
                    }

                    rtd_outl(P98_06_HS_EP0_TX_BC, (usCount - _USB_EP0_DATA_BUF_SIZE));
                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_READ, 1);
                }
            }

            break;// end of USB_BOS_DESCRIPTOR_TYPE

        default:// invalid command, Request Error

            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

            break;
    }
}

//--------------------------------------------------
// Description  : USB Standard Request - Set Address
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardSetAddr(void)
{
    unsigned char ucAddr = 0;

    ucAddr = rtd_inl(P98_0A_HS_EP0_SETUP2);
    ucAddr &= _USB_DEV_ADDR_MASK;
    ucAddr |= _USB_DEV_ADDR_VALID;

    // HW doesn't change the device address immediatelly;
    // It will update the device address after the status stage has finished;
    rtd_outl(P64_54_HS_DEV_ADDR, ucAddr);
    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
}

//--------------------------------------------------
// Description  : USB Standard Request - Set Configureation
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardSetConf(void)
{
    switch (rtd_inl(P98_0A_HS_EP0_SETUP2))    // lower byte of wValue specifies the desired configuration
    {
        case 0x00: // Un-configured

            UsbBillboardInitCtrlVariable();

            rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT1, 0x00);
            rtd_maskl(P64_56_HS_DEV_G_CFG, ~(_BIT1 | _BIT0), _BIT1);

            // Finish Control Transfer
            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

            break;

        case 0x01: // using current configuration, do nothing

            // initialization of interrupt endpoint 1
            g_ucUsbBillboardUsbConfigurationValue = 0x01;

            rtd_maskl(P64_52_HS_DEV_G_STA, ~(_BIT1 | _BIT0), 0x00);

            // Clear interrupt flag for configuration
            rtd_maskl(P64_20_HS_USP_IRQ, ~(_BIT3 | _BIT2 | _BIT1 | _BIT0), _BIT3);

            rtd_maskl(P64_56_HS_DEV_G_CFG, ~(_BIT1 | _BIT0), (_BIT1 | _BIT0));

            // Finish Control Transfer
            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

            break;

        default: // return a Request Error

            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;
    }
}

//--------------------------------------------------
// Description  : USB Standard Request - Get Interface
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardGetInterface(void)
{
    // Undefined Request
    // Hubs are allowed to support only one interface
    // alternate setting not supported
    // return 0 to host in the data packet
    unsigned int usCount = 0;

    // If the interface specified does not exist, then the device responds with a Request Error.
    usCount = ((rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) | rtd_inl(P98_0C_HS_EP0_SETUP4));

    if(usCount != 0)
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
    }

    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);

    rtd_outl(P98_06_HS_EP0_TX_BC, 0x01);
    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
}

//--------------------------------------------------
// Description  : USB Standard Request - Set Interface
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardSetInterface(void)
{
    // Undefined Request
    // Hubs are allowd to support only one interface
    unsigned int usCount = 0;

    // If the interface(wIndex) does not exist, then the device responds with a Request Error
    usCount = ((rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) | rtd_inl(P98_0C_HS_EP0_SETUP4));

    if(usCount != 0)
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
    }

    // If the alternate setting(wValue) does not exist, then the device responds with a Request Error
    usCount = ((rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) | rtd_inl(P98_0A_HS_EP0_SETUP2));

    // alternate setting not supported
    if (usCount == 0)  // interface 0, do nothing
    {
        // The Halt feature is reset to zero after either a SetConfiguration() or SetInterface()
        // request even if the requested configuration or interface is the same as the current
        // configuration or interface.
        rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT1, 0x00);

        UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
    }
    else
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
    }
}

//--------------------------------------------------
// Description  : USB Standard Request - Get Status
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardGetStatus(void)
{
    unsigned int usCount = 0;
    unsigned char ucTemp = 0;

    // If wValue should be 0 and wLength should be two.
    usCount = ((rtd_inl(P98_0B_HS_EP0_SETUP3) << 8) | rtd_inl(P98_0A_HS_EP0_SETUP2));

    if(usCount != 0)
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
        return;
    }

    usCount = ((rtd_inl(P98_0F_HS_EP0_SETUP7) << 8) | rtd_inl(P98_0E_HS_EP0_SETUP6));

    if(usCount != 2)
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
        return;
    }

    switch(rtd_inl(P98_08_HS_EP0_SETUP0) & _USB_REQUEST_RECIPIENT_MASK)
    {
        //=========================================
        case _USB_REQUEST_RECIPIENT_DEVICE: // Device

            usCount = ((rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) | rtd_inl(P98_0C_HS_EP0_SETUP4));

            // wINDEX must be 0 when for a device status request
            if (!usCount)
            {
                // HighSpeed HUB device
                // Byte0:
                // bit0: 1b, self powered
                // bit1: 0b, Remote Wakeup
                // bit2~bit7: Reserved
                ucTemp = 0;

                // HUB device is self powered
                if((rtd_inl(P64_52_HS_DEV_G_STA) & _BIT2) == _BIT2)
                {
                    ucTemp |= _USB_DEV_STATUS_SELF_POWERED;
                }

                rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, ucTemp);
                rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);   // Reserved to 0

                rtd_outl(P98_06_HS_EP0_TX_BC, 0x02);

                rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
            }
            else
            {
                // return a STALL handshake
                rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            }
            break;

        //=========================================
        case _USB_REQUEST_RECIPIENT_INTERFACE: // Interface

            usCount = ((rtd_inl(P98_0D_HS_EP0_SETUP5) << 8) | rtd_inl(P98_0C_HS_EP0_SETUP4));

            // A GetStatus() request to any interface except interface0 in a function shall return all zeros.
            rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
            rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
            rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);

            rtd_outl(P98_06_HS_EP0_TX_BC, 0x02);

            rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
            break;

        //=========================================
        case _USB_REQUEST_RECIPIENT_ENDPOINT: // Endpoint
            switch (rtd_inl(P98_0C_HS_EP0_SETUP4))
            {
                // In USB2 or USB3 Spec 9.3.4, 'Direction' of wIndex Format for control pipe can be '0' or '1'
                case _USB_REQUESET_RECIPIENT_ENDPOINT_ZERO_IN:
                case _USB_REQUESET_RECIPIENT_ENDPOINT_ZERO_OUT:

                    // HighSpeed devices do not support function stall all the time on control endpoints.
                    rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);

                    rtd_outl(P98_06_HS_EP0_TX_BC, 0x02);

                    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
                    break;

                case _USB_REQUESET_RECIPIENT_ENDPOINT_ONE_IN: // Interrtup Endpoint 1


                    if ((rtd_inl(P64_52_HS_DEV_G_STA) & _BIT1) == _BIT1) // endpoint Halt
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x01);
                    }
                    else
                    {
                        rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);
                    }

                    rtd_outl(P9A_01_BB_BUF0_WR_DATA, 0x00);

                    rtd_outl(P98_06_HS_EP0_TX_BC, 0x02);

                    rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
                    break;

                default:
                    // return a STALL handshake
                    rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
                    break;
            }
            break;// end of switch for endpoint

        //=========================================
        default:    // invalid request type
            // return a STALL handshake
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
            break;

    } // end of switch
}

//--------------------------------------------------
// Description  : USB Standard Request - Get Configuration
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardGetConf(void)
{
    // only two possible configuration values: 0 and 1
    if ((g_ucUsbBillboardUsbConfigurationValue == 0x00) || (g_ucUsbBillboardUsbConfigurationValue == 0x01))
    {
        rtd_outl(P9A_00_BB_BUF0_WR_ADDR, 0x00);
        rtd_outl(P9A_01_BB_BUF0_WR_DATA, g_ucUsbBillboardUsbConfigurationValue);

        rtd_outl(P98_06_HS_EP0_TX_BC, 0x01);

        rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
    }
    else  // undefined Configuration Value
    {
        // return a STALL handshake
        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);
    }
}

//--------------------------------------------------
// Description  : USB Standard Request - Set Feature
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardSetFeature(void)
{
    switch (rtd_inl(P98_08_HS_EP0_SETUP0) & _USB_REQUEST_RECIPIENT_MASK)
    {
        case _USB_REQUEST_RECIPIENT_DEVICE: // Device
            switch (rtd_inl(P98_0A_HS_EP0_SETUP2))
            {
                case _USB_DEVICE_REMOTE_WAKEUP: // Remote Wakeup

                    g_ucUsbBillboardUsbHostEnableRemoteWakeup = 0;

                    // Disable HW Remote Wakeup Function
                    rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT0, 0x00);

                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

                    break;

                case _USB_TEST_STATE: // Test Mode

                    switch (rtd_inl(P98_0D_HS_EP0_SETUP5))
                    {
                        case 0x01: // Test_J

                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
                            rtd_outl(P64_53_HS_UP_TEST, (_USB_TEST_MODE | _USB_TEST_J));
                            while(1) {};

                            break;

                        case 0x02: // Test_K

                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
                            rtd_outl(P64_53_HS_UP_TEST, (_USB_TEST_MODE | _USB_TEST_K));
                            while(1) {};

                            break;

                        case 0x03: // Test_SE0_NAK

                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
                            rtd_outl(P64_53_HS_UP_TEST, (_USB_TEST_MODE | _USB_TEST_SE0_NAK));
                            while(1) {};

                            break;

                        case 0x04: // Test_Packet

                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
                            rtd_outl(P64_53_HS_UP_TEST, (_USB_TEST_MODE | _USB_TEST_PACKET));
                            while(1) {};

                            break;

                        case 0x05: // Test_Force_Enable

                            UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);
                            while(1) {};

                            break;

                        default:

                            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                            break;
                    }

                    break;

                default:

                    break;
            } // end of switch HS_SETUP_PKT_wVALUE_L
            break;    // Break of case 0x00

        case _USB_REQUEST_RECIPIENT_INTERFACE: // Interface

            // SetFeature() for Interface is not valid in HS
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

            break;

        case _USB_REQUEST_RECIPIENT_ENDPOINT: // Endpoints

            switch (rtd_inl(P98_0C_HS_EP0_SETUP4))
            {
                case _USB_REQUESET_RECIPIENT_ENDPOINT_ONE_IN: // INT Endpoint 1

                    // Endpoint B (INT endpoint) Halt
                    rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT1, _BIT1);
                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

                    break;

                default: // invalid command

                    rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                    break;
            } // end of switch SETUP_PKT_wINDEX_L

            break;

        default: // invalid command

            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

            break;

    } // end of HS_SETUP_PKT_bmREQUST_TYPE
}

//--------------------------------------------------
// Description  : USB Standard Request - Clear Feature
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardClearFeature(void)
{
    switch(rtd_inl(P98_08_HS_EP0_SETUP0) & _USB_REQUEST_RECIPIENT_MASK)
    {
        case _USB_REQUEST_RECIPIENT_DEVICE: // Device

            switch (rtd_inl(P98_0A_HS_EP0_SETUP2))
            {
                case _USB_DEVICE_REMOTE_WAKEUP: // Remote Wakeup for USB2.0

                    g_ucUsbBillboardUsbHostEnableRemoteWakeup = 0;

                    // Disable HW Remote Wakeup Function
                    rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT0, 0x00);

                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

                    break;

                case _USB_TEST_STATE: // Test mode

                    // Test_Mode feature cannot be cleared by the ClearFeature() request
                    UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

                    break;

                default:

                    rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                    break;
            }
            break;

        case _USB_REQUEST_RECIPIENT_INTERFACE: // Interface

            // ClearFeature() for Interface is not valid in HS
            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

            break;


        case _USB_REQUEST_RECIPIENT_ENDPOINT: // Endpoint

            if (rtd_inl(P98_0A_HS_EP0_SETUP2) == _USB_ENDPOINT_HALT)
            {
                switch (rtd_inl(P98_0C_HS_EP0_SETUP4))
                {
                    // Clear Endpoint Stall feature;
                    case _USB_REQUESET_RECIPIENT_ENDPOINT_ONE_IN: // INT Endpoint 1

                        rtd_maskl(P64_52_HS_DEV_G_STA, ~_BIT1, 0x00);

                        UsbBillboardCtrlXfer(_USB_CTRL_XFER_NO_DATA, 0);

                        break;

                    default:

                        rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                        break;

                }    // end of switch SETUP_PKT_wINDEX_L

                break;
            }
            else
            {
                rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

                break;
            }

        default:

            rtd_outl(P98_04_HS_EP0_CTRL, _USB_EP0_STALL);

            break;
    } // end of switch HS_SETUP_PKT_bmREQUST_TYPE
}

//--------------------------------------------------
// Description  : Billboard get altmode info from PD
// Input Value  : None
// Output Value : None
//--------------------------------------------------
void UsbBillboardGetAltModeInfo(void)
{
    memset(g_pstUsbBillboardAltModeInfo, 0x00, _USB_MAX_ALTERNATE_MODE_NUM * sizeof(StructTypeCBillboardInfo));

    if(GET_USB_HUB_MUX_SEL_STATUS() == _D0_TYPE_C_PORT)
    {
        // TODO
        //TypeCGetAltModeBillboardInfo(_D0_TYPE_C_PORT, (unsigned char *)(&g_pstUsbBillboardAltModeInfo));
    }
    else if(GET_USB_HUB_MUX_SEL_STATUS() == _D1_TYPE_C_PORT)
    {
        // TODO
        //TypeCGetAltModeBillboardInfo(_D1_TYPE_C_PORT, (unsigned char *)(&g_pstUsbBillboardAltModeInfo));
    }
}

//--------------------------------------------------
// Description  : Initial USB Variable
// Input Value  : None
// Output Value : _TRUE or _FALSE
//--------------------------------------------------
void UsbBillboardInitCtrlVariable(void)
{
    g_ucUsbBillboardUsbConfigurationValue = 0;
    g_ucUsbBillboardUsbHostEnableRemoteWakeup = 0;
    g_ucUsbBillboardUsbVendorComEnb = 0;
}

//--------------------------------------------------
// Description  : USB Control Transfer Sequence
// Input Value  : None
// Output Value : _TRUE or _FALSE
//--------------------------------------------------
bool UsbBillboardCtrlXfer(unsigned char ucXferType, unsigned char ucIsLastPacket)
{
    // Update by Frank@2017/07/13
    unsigned int ulCounter = 200000;

    // Validate data in FIFO.
    switch(ucXferType) {
	    case _USB_CTRL_XFER_READ:
	    {
	        rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT2, _BIT2);

	        // In EP0 read data transfer, return 0 if the data stage doesn't finished in 1 seconds
	        // Howard temp removed if( !SetupISR_WaitTimeOut(HS_EP0_CTRL, HS_HUB_EP0_SEND_PKT_EN, 0x00, 100, ALL_MASK_BIT_MEET))
	        // Howard temp removed {
	        // Howard temp removed     // Disable BUF0 for EP0 TX Enable
	        // Howard temp removed     rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT2, 0x00);

	        // Howard temp removed     // Timeout occurred, do not STALL and just return 0, let caller determine STALL or not
	        // Howard temp removed     return 0;
	        // Howard temp removed }

	        // TX polling & wait
	        while(((rtd_inl(P98_04_HS_EP0_CTRL) & _BIT2) != 0x00) && (ulCounter-- > 0))
	        {
	            udelay(5);
	        }

	        // 1SEC Time-out
	        if(ulCounter == 0)
	        {
	            rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT2, 0x00);
	            return _FALSE;
	        }

	        if(ucIsLastPacket == 1)
	        {
	            // Clear CONTROL_STATUS_HANDSHAKE
	            rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT0, _BIT0);
	        }

	        return _TRUE;
	    }
	    case _USB_CTRL_XFER_WRITE_DATA_STAGE:
	    {
	        // Enable EP0 for FIFO
	        rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT3, _BIT3);

	        // In EP0 write data transfer, wait all data already received in the Buf0
	        // Howard temp removed if( !SetupISR_WaitTimeOut(HS_EP0_CTRL, HS_HUB_EP0_REC_PKT_EN, 0x00, 100, ALL_MASK_BIT_MEET))
	        // Howard temp removed {
	        // Howard temp removed     // Disable BUF0 for EP0 RX
	        // Howard temp removed     rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT3, 0x00);

	        // Howard temp removed     // Timeout occurred, do not STALL and just return 0, let caller determine STALL or not
	        // Howard temp removed     return 0;
	        // Howard temp removed }

	        // RX polling & wait
	        while(((rtd_inl(P98_04_HS_EP0_CTRL) & _BIT3) != 0x00) && (ulCounter-- > 0))
	        {
	            udelay(5);
	        }

	        // 1SEC Time-out
	        if(ulCounter == 0)
	        {
	            rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT3, 0x00);
	            return _FALSE;
	        }

	        return _TRUE;
	    }
	    case _USB_CTRL_XFER_WRITE_STATUS_STAGE:
	    {
	        return _FALSE;
	    }
	    default: // _USB_CTRL_XFER_NO_DATA
	    {
	        rtd_maskl(P98_04_HS_EP0_CTRL, ~_BIT0, _BIT0);

	        return _TRUE;
	    }
    }
}


//--------------------------------------------------
// Description  : USB Control Transfer for GetDescriptor
// Input Value  : None
// Output Value : _TRUE or _FALSE
//--------------------------------------------------
bool UsbBillboardXferForGetDescriptor(unsigned char ucXferType, unsigned char ucIsLastPacket)
{
    // Validate data in FIFO.
    if(ucXferType == _USB_CTRL_XFER_READ)
    {
        if(ucIsLastPacket == 1)
        {
            rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT4 | _BIT2), (_BIT4 | _BIT2));
        }
        else
        {
            rtd_maskl(P98_04_HS_EP0_CTRL, ~(_BIT2), (_BIT2));
        }

        return _TRUE;
    }

    return _FALSE;
}

void rtk_billboard_enable_connect(bool connect)
{
	unsigned long flags;
	spin_lock_irqsave(&g_rtk_bb_lock, flags);
	if(connect) {
		UsbBillboard_CRT_ON_OFF(true);
		UsbBillboardPllPower(_USB_BB_ENABLE);
		rtd_outl(P98_01_HS_EP0_IRQ_EN, 0x01);
	} else {
		if(is_UsbBillboard_CRT_ON()) {
			rtd_outl(P98_01_HS_EP0_IRQ_EN, 0x00);
			UsbBillboardPllPower(_USB_BB_DISABLE);
			UsbBillboard_CRT_ON_OFF(false);
		}
	}
	spin_unlock_irqrestore(&g_rtk_bb_lock, flags);
}
EXPORT_SYMBOL(rtk_billboard_enable_connect);

static ssize_t get_billboard_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t set_billboard_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        int val = 0;

        if (sscanf(buf, "%u", &val) == 1) {
		  if((!!val))
		  	rtd_outl(0xb80644EC, rtd_inl(0xb80644EC) | _BIT7);
		  else
		  	rtd_outl(0xb80644EC, rtd_inl(0xb80644EC) & (~_BIT7));
                //rtk_billboard_enable_connect(!!val);
        }
        return count;
}

static DEVICE_ATTR(billboard_state, S_IRUGO | S_IWUSR,
                   get_billboard_state, set_billboard_state);

const struct attribute *rtk_bb_platform_dev_attrs[] =
{
        &dev_attr_billboard_state.attr,
	 NULL,
};

static const struct attribute_group rtk_bb_platform_dev_attr_grp =
{
        .attrs = (struct attribute **)rtk_bb_platform_dev_attrs,
};

int _usb_bb_detect_process(void *arg)
{
    while(!g_usb_bb_check_thread_exited_flag) {
	 set_freezable();
        if(((rtd_inl(0xb80644EC) & _BIT7) == _BIT7) && (!BB_event_flag)) {
            BB_event_flag = 1;
            rtd_pr_usb_bb_debug("_usb_bb_detect_process: rtk_billboard_enable_connect\n");
            rtk_billboard_enable_connect(true);
        }
        else if((rtd_inl(0xb80644EC) & _BIT7) == 0x0) {
            BB_event_flag = 0;
            rtk_billboard_enable_connect(false);
        }
        msleep(100);
    }
    complete_and_exit(&g_usb_bb_check_thread_exited, 1);
    return 0;
}

static int __init rtk_bb_platform_probe(struct platform_device *pdev)
{
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
                rtd_pr_usb_bb_err("Get irq number failed(ret:%x)\n", irq);
                return irq;
	}
	if(request_irq(irq, UsbBillboardHandler,
				IRQF_SHARED, "rtk_bb", UsbBillboardHandler)) {
        	rtd_pr_usb_bb_err("request irq failed\n");
        	return -EIO;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &rtk_bb_platform_dev_attr_grp)) {
		rtd_pr_usb_bb_warn("Create self-defined sysfs attributes fail \n");
	}

	// Polling Hub change status
	g_usb_bb_check_thread_exited_flag = 0;
	g_usb_bb_task = kthread_create(_usb_bb_detect_process, NULL, "_usb_bb_detect_process");
	if(IS_ERR(g_usb_bb_task)) {
		rtd_pr_usb_bb_err("Create USB BB Thread fail \n");
		return -1;
	}
	init_completion(&g_usb_bb_check_thread_exited);
	wake_up_process(g_usb_bb_task);

	return 0;
}

static int rtk_bb_platform_remove(struct platform_device *pdev)
{
	int irq;
	if(g_usb_bb_task) {
		g_usb_bb_check_thread_exited_flag = 1;
		wait_for_completion(&g_usb_bb_check_thread_exited);
		g_usb_bb_task = NULL;
	}	
	sysfs_remove_group(&pdev->dev.kobj, &rtk_bb_platform_dev_attr_grp);
	
	irq = platform_get_irq(pdev, 0);
	free_irq(irq, UsbBillboardHandler);
	rtk_billboard_enable_connect(false);
	return 0;
}

static int rtk_bb_platform_resume(struct device *dev)
{
    //no need do anything
    return 0;
}


static int rtk_bb_platform_suspend(struct device *dev)
{
    rtd_outl(0xb80644EC, rtd_inl(0xb80644EC) & (~_BIT7));
    BB_event_flag = 0;
    rtk_billboard_enable_connect(false);
    return 0;
}


static const struct dev_pm_ops rtk_bb_platform_pm_ops = {
    .resume = rtk_bb_platform_resume,
    .suspend = rtk_bb_platform_suspend,
};


static const struct of_device_id rtk_bb_of_match[] = {
    { .compatible = "rtk,rtk-bb-platform", },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_bb_of_match);


static struct platform_driver rtk_bb_platform_driver = {
    .probe      = rtk_bb_platform_probe,
    .remove     = rtk_bb_platform_remove,
    .driver     = {
        .name = "rtk-bb-platform",
        .pm = &rtk_bb_platform_pm_ops,
        .of_match_table = rtk_bb_of_match,
    }
};


static int __init rtk_bb_platform_init(void)
{
    int ret = 0;

    ret = platform_driver_register(&rtk_bb_platform_driver);
    if(ret != 0)
        rtd_pr_usb_bb_err("Realtek BillBoard Platorm driver init failed\n");
    return ret;
}
late_initcall(rtk_bb_platform_init);


static void __exit rtk_bb_platform_cleanup(void)
{
    platform_driver_unregister(&rtk_bb_platform_driver);
}
module_exit(rtk_bb_platform_cleanup);


MODULE_DESCRIPTION(DRIVER_DESC);

