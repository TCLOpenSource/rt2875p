// $Header: /project/nabik/cvsroot/nabik/FEUSB3/src/feusb3/feusb.h,v 1.4 2006/10/06 10:44:37 yonaga Exp $

#ifndef FEUSB_H
#define FEUSB_H
//-----------------------------------------------------------------------------
//   File    Name:  feusb.h
//   Project Name:  feusb
//   Contents    :  General define Header
//   Used Platform : Firmware / Kernel mode driver / User mode driver
//
//  $Date: 2013-11-08 02:12:20 +0900 (2013/11/08 (Èáë)) $
//
//   Copyright (c) 2003 Argohytec Inc. All rights reserved
//-----------------------------------------------------------------------------

// F/WÇ≈ÉRÉìÉpÉCÉãÇ∑ÇÈÇ∆Ç´ÇÕÅAÇ±ÇÃÉwÉbÉ_ÉtÉ@ÉCÉãÇÉCÉìÉNÉãÅ[ÉhÇ∑ÇÈëOÇ…ÅuDEF_FEUSB_FWÅv
// Ç#defineÇ∑ÇÈÇ±Ç∆
#ifndef DEF_FEUSB_FW
//////////////////////////////////
// DeviceIoControl Define Table //
//////////////////////////////////
#define FEUSB_IOCTL_INDEX  0x0000


#define IOCTL_FEUSB_GET_CONFIG_DESCRIPTOR     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_RESET_DEVICE   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_RESET_PIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_VENDORREQUEST_IN  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+3,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_VENDORREQUEST_OUT  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+4,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_GETDESCRIPTOR  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+5,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_FEUSB_CHECK  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+6,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_FEUSB_GETSTATUS  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+8,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_ABORT_PIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+9,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_FEUSB_GET_DRIVER_VERSION  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+7,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_FEUSB_GET_KERNEL_BUFFER_STATUS  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+10,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
/*
FunctionÇÃíl

URB_FUNCTION_VENDOR_DEVICE
    Indicates the URB is a vendor-defined request for a USB device.
URB_FUNCTION_VENDOR_INTERFACE
    Indicates the URB is a vendor-defined request for an interface on a USB device.
URB_FUNCTION_VENDOR_ENDPOINT
    Indicates the URB is a vendor-defined request for an endpoint, in an interface, on a USB device.
URB_FUNCTION_VENDOR_OTHER
    Indicates the URB is a vendor-defined request for a device-defined target.
URB_FUNCTION_CLASS_DEVICE
    Indicates the URB is a USB-defined class request for a USB device.
URB_FUNCTION_CLASS_INTERFACE
    Indicates the URB is a USB-defined class request for an interface on a USB device.
URB_FUNCTION_CLASS_ENDPOINT
    Indicates the URB is a USB-defined class request for an endpoint, in an interface, on a USB device.
URB_FUNCTION_CLASS_OTHER
    Indicates the URB is a USB-defined class request for a device-defined target.
*/

/////////////////////////////////////////////////////////////////
// DeviceIoControlÇÃInputBufferÇ≈égópÇ∑ÇÈGetDescriptorÇÃç\ë¢ëÃ //
/////////////////////////////////////////////////////////////////
typedef struct tagPARAM_PARAM_GET_DESCRIPTOR {
    UCHAR   DescriptorType;     // éüÇÃÇ¢Ç∏ÇÍÇ©
                                // USB_DEVICE_DESCRIPTOR_TYPE
                                // USB_CONFIGURATION_DESCRIPTOR_TYPE
                                // USB_STRING_DESCRIPTOR_TYPE

    UCHAR   Index;              // Index
    USHORT  LanguageId;         // DescriptorTypeÇ™USB_STRING_DESCRIPTOR_TYPE
                                // ÇÃéûÇÃÇ›óLå¯
} PARAM_GET_DESCRIPTOR;

#ifndef DEF_FEUSB_KERNELMODE
typedef struct _USB_DEVICE_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    USHORT bcdUSB;
    UCHAR bDeviceClass;
    UCHAR bDeviceSubClass;
    UCHAR bDeviceProtocol;
    UCHAR bMaxPacketSize0;
    USHORT idVendor;
    USHORT idProduct;
    USHORT bcdDevice;
    UCHAR iManufacturer;
    UCHAR iProduct;
    UCHAR iSerialNumber;
    UCHAR bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

#define USB_DEVICE_DESCRIPTOR_TYPE                0x01
#endif

typedef struct tagPARAM_GET_STATUS {
    USHORT  type;
    USHORT  Index;
} PARAM_GET_STATUS;

/////////////////////////////////////////////////////////////////
// DeviceIoControlÇÃInputBufferÇ≈égópÇ∑ÇÈVendorRequestÇÃç\ë¢ëÃ //
/////////////////////////////////////////////////////////////////
typedef struct tagPARAM_VENDOR_REQUEST {
    IN USHORT Function;
    IN ULONG TransferFlags;     // éwíËí∑ÇÊÇËé¿ç€ÇÃì]ëóí∑Ç™íZÇ≠ÇƒÇ‡ÉGÉâÅ[
                                // Ç∆ÇµÇ»Ç¢èÍçáUSBD_SHORT_TRANSFER_OKÇéwíË
    IN UCHAR ReservedBits;
    IN UCHAR Request;
    IN USHORT Value;
    IN USHORT Index;
} PARAM_VENDOR_REQUEST;

#define GET_STATUS_FROM_DEVICE      0
#define GET_STATUS_FROM_INTERFACE   1
#define GET_STATUS_FROM_ENDPOINT    2
#define GET_STATUS_FROM_OTHER       3

/////////////////////////////////////////////////////////////////
// DeviceIoControlÇÃInputBufferÇ≈égópÇ∑ÇÈ                      //
// VendorRequestÉRÉ}ÉìÉhÇÃRQCTL_FEUSB_GET_VERSION_INFOÇÃç\ë¢ëÃ //
/////////////////////////////////////////////////////////////////
typedef struct _FEUSB_VERSION_INFO {
    SHORT wFirmwareRevision;
    ULONG dwBoardSerialNumber;
} FEUSB_VERSION_INFO;

///////////////////////////////////////////////////////////////
// DeviceIoControlÇÃIOCTL_FEUSB_GET_DRIVER_VERSIONÇ≈égópÇ∑ÇÈ //
// OutBufferÇÃç\ë¢ëÃ                                         //
///////////////////////////////////////////////////////////////
typedef struct _FEUSB_DRIVER_VERSION {
    SHORT   MajorVersion;
    SHORT   MinorVersion;
    SHORT   BuildVersion;
} FEUSB_DRIVER_VERSION, *PFEUSB_DRIVER_VERSION;

/////////////////////////////////////////////////////////////////////
// DeviceIoControlÇÃIOCTL_FEUSB_GET_KERNEL_BUFFER_STATUSÇ≈égópÇ∑ÇÈ //
// OutBufferÇÃç\ë¢ëÃ                                               //
/////////////////////////////////////////////////////////////////////
typedef struct _FEUSB_KERNEL_BUFFER_STATUS {
    ULONG   ulBufferingLength;
    ULONG   ulMaxBufferSize;
} FEUSB_KERNEL_BUFFER_STATUS, *PFEUSB_KERNEL_BUFFER_STATUS;

////////////////////////////////////////////////////////////////
// ÉGÉìÉhÉ|ÉCÉìÉg0ÇÃì]ëóÇ≈Vendor Command Ç≈ÉfÉoÉCÉXÇ…ëóÇÈà∂êÊ //
// ãyÇ—Shortì]ëóÉtÉâÉO                                        //
////////////////////////////////////////////////////////////////
#define URB_FUNCTION_VENDOR_DEVICE                   0x0017
#define URB_FUNCTION_VENDOR_INTERFACE                0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT                 0x0019
#define URB_FUNCTION_VENDOR_OTHER                    0x0020

#define USBD_SHORT_TRANSFER_OK_BIT            1
#define USBD_SHORT_TRANSFER_OK                (1<<USBD_SHORT_TRANSFER_OK_BIT)

#endif /* ifndef DEF_FEUSB_FW end */

//////////////////////////////////////////////////////
// DeviceIoControl / FW Vendor Request Define Table //
//////////////////////////////////////////////////////
#define RQCTL_FEUSB_WRITE                         0x20
#define RQCTL_FEUSB_READ                          0x21
#define RQCTL_FEUSB_READWOSUBADR                  0x2A
#define RQCTL_FEUSB_3WIRE_WR                      0x22
#define RQCTL_FEUSB_GET_SERIAL_TRANSFER_RESPONSE  0x25
#define RQCTL_FEUSB_GET_IF_MODE                   0x24
#define RQCTL_FEUSB_SET_IF_MODE                   0x23
#define RQCTL_FEUSB_GET_VERSION_INFO              0x2F
#define RQCTL_FEUSB_GET_TSP_BUFFER                0x26
#define RQCTL_FEUSB_FIFO_RESET                    0x27
#define RQCTL_FEUSB_SET_POWER_PROFILE             0x29
#define RQCTL_FEUSB_READ2                         0x28

////////////////////////////////////
// ALPHA-USB Power Profile Define //
////////////////////////////////////
#define POWER_PROFILE_SUSPEND                     0x00
#define POWER_PROFILE_WAKEUP                      0xFF

///////////////////////////////
// Transfer Interface Define //
///////////////////////////////
#define IF_4WIRE    0          // 4WIRE - 3WIRE
#define IF_I2C      1          // I2C   - 3WIRE
#define IF_PORT     2          // General PORT Access
#define IF_FX2I2C   1          // FX2 I2C Interface Only
// ä˘ë∂ÇÃGUIÇ∆å›ä∑ê´ÇéùÇΩÇπÇÈÇΩÇﬂÇ…ÅuIF_FX2I2CÅvÇÕÅuIF_I2CÅvÇ∆ìØÇ∂ílÇÇ∆ÇÈ

////////////////////////////
// IF_PORT Address Define //
////////////////////////////
#define ACS_PORT_A  0
#define ACS_PORT_B  1
#define ACS_PORT_C  2
#define ACS_PORT_D  3
#define ACS_PORT_E  4

////////////////////////////////////////////////////////////////////////////////////
// Transfer Response Code Define ( RQCTL_FEUSB_GET_SERIAL_TRANSFER_RESPONSE Res ) //
////////////////////////////////////////////////////////////////////////////////////
#define ERR_IF_OK   0
#define ERR_ACK     1
#define ERR_NACK    2
#define ERR_IF_NG   0xFF
#define ERR_POWER   3

////////////////////////////////////
// 3WIRE Transfer Protocol Define //
////////////////////////////////////
#define DATA_ENABLE  0x01
#define LATCH_ENABLE 0x02

#endif /* FEUSB_H */
