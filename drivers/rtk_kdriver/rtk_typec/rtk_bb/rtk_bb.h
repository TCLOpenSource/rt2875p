//----------------------------------------------------------------------------------------------------
// ID Code      : Billboard.h
// Update Note  :
//----------------------------------------------------------------------------------------------------

//****************************************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************************************

#ifndef __RTK_USB_BILLBOARD__
#define __RTK_USB_BILLBOARD__

#include <rtk_kdriver/io.h>
#include <mach/rtk_platform.h>
#include <mach/pcbMgr.h>
#include <mach/rtk_log.h>
#include <linux/time.h>
#include <base_types.h>

#define _ON                                     1
#define _OFF                                    0
#define _FALSE                                  0
#define _NO_TYPE_C_PORT                         0
#define _USB_TYPE_C_BB_PROGRAM_SUPPORT          1
#define _FLASH_WRITE_FUNCTION_SUPPORT           1
#define _CPU_ID_DW8051                          0
#define _CPU_TYPE_ID                            0
#define _USB_VENDOR_ID_L                        0
#define _USB_VENDOR_ID_H                        0
#define _USB_PRODUCT_ID_L                       0
#define _USB_PRODUCT_ID_H                       0
#define _TYPE_C_VENDOR_ALT_MODE                 0
#define _TYPE_C_LENOVO_ALT_MODE                 0
#define _DX_DP_TYPE_C_EMB_DP_ALT_MODE_SUPPORT   0
#define _DUAL_BANK_DEBUG_SUPPORT                0
#define _TYPE_C_PORT_CTRL_SUPPORT               1
#define _EEPROM_WP_DISABLE                      0
#define _SW_IIC_PIN_GPIO                        0
#define _PCB_SYS_EEPROM_IIC                     0
#define _TYPE_C_BILLBOARD_URL_INDEX_LENOVO      1
#define _TYPE_C_BILLBOARD_URL_INDEX_DP          2

/*TODO*/
#define _D0_TYPE_C_PORT               4
#define _D1_TYPE_C_PORT               5

/*TODO*/
#define FlashRead(x)        0
#define _EEPROM_TYPE        0
#define _EEPROM_24LC16      0
#define _NO_IIC_PIN         0

/*TODO*/
#define __BOOT_GET_ISP_START_ADDR_OP_CODE         0
#define __BOOT_GET_ISP_USER_KEY_ADDR_OP_CODE      1
#define __BOOT_GET_ISP_USER_FLAG_ADDR_OP_CODE     2
#define __BOOT_GET_ISP_USER_SIGN_ADDR_OP_CODE     3
#define __BOOT_GET_VERIFY_COPY_RESULT_OP_CODE     4
#define __BOOT_GET_ISP_CURRENT_KEY_ADDR_OP_CODE   5
#define __BOOT_GET_DUAL_BANK_INFO_OP_CODE         6
#define __BOOT_SWITCH_CODE_OP_CODE                7

/*TODO*/
//#define P1D_14_BANDGAP                  0
#define P64_00_HS_USP_CTL               0xb805D000
#define P64_04_HS_USP_STS               0xb805D010
#define P64_20_HS_USP_IRQ               0xb805D080
#define P64_C6_CMU480_PLL_DIV_CFG0      0
#define P64_74_HS_USB2_RESET            0xb805D1D0
#define P64_52_HS_DEV_G_STA             0xb805D148
#define P64_53_HS_UP_TEST               0xb805D14C
#define P64_54_HS_DEV_ADDR              0xb805D150
#define P64_56_HS_DEV_G_CFG             0xb805D158
//#define P80_0E_XTAL_CTRL0      0
//#define P80_0F_XTAL_CTRL1      0
#define P98_00_HS_EP0_IRQ               0xb805D300
#define P98_01_HS_EP0_IRQ_EN            0xb805D304
#define P98_04_HS_EP0_CTRL              0xb805D310
#define P98_06_HS_EP0_TX_BC             0xb805D318
#define P98_08_HS_EP0_SETUP0            0xb805D320
#define P98_09_HS_EP0_SETUP1            0xb805D324
#define P98_0A_HS_EP0_SETUP2            0xb805D328
#define P98_0B_HS_EP0_SETUP3            0xb805D32C
#define P98_0C_HS_EP0_SETUP4            0xb805D330
#define P98_0D_HS_EP0_SETUP5            0xb805D334
#define P98_0E_HS_EP0_SETUP6            0xb805D338
#define P98_0F_HS_EP0_SETUP7            0xb805D33C
#define P9A_00_BB_BUF0_WR_ADDR          0xb805D400
#define P9A_01_BB_BUF0_WR_DATA          0xb805D404
#define P9A_02_BB_BUF0_RD_ADDR          0xb805D408
#define P9A_03_BB_BUF0_RD_DATA          0xb805D40C
#define PFF_E2_PORT1_PIN_REG            0
#define PFF_E3_PORT3_PIN_REG            0
#define _HW_XRAM_ISP_MAPPING_ADDR_START   0


#if(_CPU_TYPE_ID == _CPU_ID_DW8051)
#define BB_SET_P1_VALUE(x)                               (P1 = (x))
#define BB_SET_P3_VALUE(x)                               (P3 = (x))
#endif

#define _USB_EP0_DATA_BUF_SIZE                           0x40

#define _USB_31_SS_SUPPORT                               _OFF
#define _USB_HUB_SS_PID_H_                               0x04
#define _USB_HUB_SS_PID_L_                               0x00
#define _USB_HUB_HS_PID_H_                               0x54
#define _USB_HUB_HS_PID_L_                               0x00

#define _USB_FW_MAIN_VER_                                0x01
#define _USB_FW_SUB_VER_                                 0x00
#define _USB_DESC_SIZE_DEV                               0x12
#define _USB_DESC_SIZE_CONF                              0x09
#define _USB_DESC_SIZE_INTF                              0x09
#define _USB_DESC_SIZE_EP                                0x07
#define _USB_DESC_SIZE_EP_COMPANION                      0x06
#define _USB_DESC_SIZE_CPLS                              0x04
#define _USB_DESC_SIZE_BOS                               0x05
#define _USB_DESC_SIZE_USB2_EXTENSION                    0x07

#if(_USB_31_SS_SUPPORT == _ON)
#define _USB_DESC_SIZE_SUPERSPEED_USB                    0x0A
#define _USB_BOS_DEVICEACPS_NUM                          0x04
#define _USB_BB_CAP_LEN_OFFSEET_IN_BOS                   0x2A
#define _USB_BB_CAP_ALTMODE_NUM_OFFSET_IN_BOS            0x2E
#define _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_1_4     0x32
#define _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_5_8     0x33
#else
#define _USB_DESC_SIZE_SUPERSPEED_USB                    0x00
#define _USB_BOS_DEVICEACPS_NUM                          0x03
#define _USB_BB_CAP_LEN_OFFSEET_IN_BOS                   0x20 // 0x2A - 0x0A(_USB_DESC_SIZE_SUPERSPEED_USB)
#define _USB_BB_CAP_ALTMODE_NUM_OFFSET_IN_BOS            0x24 // 0x2e - 0x0A(_USB_DESC_SIZE_SUPERSPEED_USB)
#define _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_1_4     0x28 // 0x32 - 0x0A(_USB_DESC_SIZE_SUPERSPEED_USB)
#define _USB_BB_CAP_ALTMODE_STATUS_OFFSET_IN_BOS_5_8     0x29 // 0x33 - 0x0A(_USB_DESC_SIZE_SUPERSPEED_USB)
#endif

#define _USB_DESC_SIZE_CONTAINER_ID                      0x14
#define _USB_DESC_SIZE_BILLBOARD_CAP                     0x2C
#define _USB_DESC_SIZE_ALTERNATE_MODE                    0x04
#define _USB_DESC_SIZE_BILLBOARD_ALTMODE_CAP             0x08

#define _USB_DESC_SIZE_CONF_TOTAL                        (_USB_DESC_SIZE_CONF + _USB_DESC_SIZE_INTF)

// Could Support (Dp AltMode + Lenovo AltMode) so far
#define _USB_MAX_ALTERNATE_MODE_NUM                      ((_DX_DP_TYPE_C_EMB_DP_ALT_MODE_SUPPORT == _ON) +\
                                                          (_TYPE_C_VENDOR_ALT_MODE == _TYPE_C_LENOVO_ALT_MODE))

#define _USB_BB_BOS_DESC_FIRST_PART_LEN                  (_USB_DESC_SIZE_BOS + _USB_DESC_SIZE_USB2_EXTENSION + _USB_DESC_SIZE_SUPERSPEED_USB + _USB_DESC_SIZE_CONTAINER_ID + _USB_DESC_SIZE_BILLBOARD_CAP)
#define _USB_BB_BOS_DESC_TOTAL_LEN_INIT                  (_USB_BB_BOS_DESC_FIRST_PART_LEN + _USB_MAX_ALTERNATE_MODE_NUM * (_USB_DESC_SIZE_ALTERNATE_MODE + _USB_DESC_SIZE_BILLBOARD_ALTMODE_CAP))

#define _USB_CTRL_XFER_NO_DATA                           0x00
#define _USB_CTRL_XFER_READ                              0x01
#define _USB_CTRL_XFER_WRITE_DATA_STAGE                  0x02
#define _USB_CTRL_XFER_WRITE_STATUS_STAGE                0x03

// Standard Request based on the USB 1.0 specification
#define _USB_REQUEST_GET_STATUS                          0x00
#define _USB_REQUEST_CLEAR_FEATURE                       0x01
#define _USB_REQUEST_SET_FEATURE                         0x03
#define _USB_REQUEST_SET_ADDRESS                         0x05
#define _USB_REQUEST_GET_DESCRIPTOR                      0x06
#define _USB_REQUEST_SET_DESCRIPTOR                      0x07
#define _USB_REQUEST_GET_CONFIGURATION                   0x08
#define _USB_REQUEST_SET_CONFIGURATION                   0x09
#define _USB_REQUEST_GET_INTERFACE                       0x0A
#define _USB_REQUEST_SET_INTERFACE                       0x0B
#define _USB_REQUEST_SYNC_FRAME                          0x0C
#define _USB_REQUEST_SET_SEL                             0x30
#define _USB_REQUEST_ISOCH_DELAY                         0x31

// Define the 8-bytes Data location of Setup Packet
#define _USB_REQUEST_TYPE_MASK                           0x60
#define _USB_STANDARD_REQUEST                            0x00
#define _USB_CLASS_REQUEST                               0x20
#define _USB_VENDOR_REQUEST                              0x40
#define _USB_REQUEST_RECIPIENT_MASK                      0x1F
#define _USB_REQUEST_RECIPIENT_DEVICE                    0x00
#define _USB_REQUEST_RECIPIENT_INTERFACE                 0x01
#define _USB_REQUEST_RECIPIENT_ENDPOINT                  0x02
#define _USB_REQUEST_RECIPIENT_PORT                      0x03

#define _USB_REQUESET_RECIPIENT_ENDPOINT_ZERO_IN         0x80
#define _USB_REQUESET_RECIPIENT_ENDPOINT_ZERO_OUT        0x00
#define _USB_REQUESET_RECIPIENT_ENDPOINT_ONE_IN          0x81

// standard feature selectors
#define _USB_ENDPOINT_HALT                               0x00   // endpoint
#define _USB_DEVICE_REMOTE_WAKEUP                        0x01   // device
#define _USB_TEST_STATE                                  0x02   // device

// HS_EP0_IRQ           --  0x9800
#define _HS_EP0_SETUP_PKT_RECEIVE_INT                    0x01
#define _HS_EP0_VENDOR_CMD_PKT_RECEIVE_INT               0x02
#define _HS_EP0_GET_DESCRI_PKT_RECEIVE_INT               0x04
#define _HS_EP0_SEND_PKT_DONE_INT                        0x08
#define _HS_EP0_RECEIVE_PKT_DONE_INT                     0x10

// HS_EP0_IRQ_EN        --  0x9801
#define _HS_EP0_SETUP_PKT_RECEIVE_INT_EN                 0x01
#define _HS_EP0_VENDOR_CMD_PKT_RECEIVE_INT_EN            0x02
#define _HS_EP0_GET_DESCRI_PKT_RECEIVE_INT_EN            0x04
#define _HS_EP0_SEND_PKT_DONE_INT_EN                     0x08
#define _HS_EP0_RECEIVE_PKT_DONE_INT_EN                  0x10

// HS_EP0_CTRL          --  0x9804
#define _USB_EP0_AUTO_TX_STATUS_HS                       0x10
#define _USB_EP0_REC_PKT_EN                              0x08
#define _USB_EP0_SEND_PKT_EN                             0x04
#define _USB_EP0_STALL                                   0x02
#define _USB_EP0_STATUS_HS_EN                            0x01

// HS_USP_STS           --  0x6404
#define _USB_HS_USP_SPEED                                0x01
#define _USB_HS_USP_SUSPEND                              0x02
#define _USB_HS_USP_RESUME                               0x04
#define _USB_HS_USP_CONFIG                               0x08

// HS_DEV_STATUS        --  0x6452
#define _USB_DEV_REMOTE_WAKEUP_EN                        0x01
#define _USB_DEV_EPB_STALL                               0x02
#define _USB_DEV_SELF_POWER                              0x04

// HS_USP_TEST          --  0x6453
#define _USB_TEST_J                                      0x01
#define _USB_TEST_K                                      0x02
#define _USB_TEST_PACKET                                 0x04
#define _USB_TEST_SE0_NAK                                0x08
#define _USB_TEST_MODE                                   0x10

// HS_DEV_ADDR          --  0x6454
#define _USB_DEV_ADDR_VALID                              0x80
#define _USB_DEV_ADDR_MASK                               0x7F

// Standard Descriptor Type (Table 9-5)
#define _USB_DEVICE_DESCRIPTOR_TYPE                      0x01
#define _USB_CONFIGURATION_DESCRIPTOR_TYPE               0x02
#define _USB_STRING_DESCRIPTOR_TYPE                      0x03
#define _USB_INTERFACE_DESCRIPTOR_TYPE                   0x04
#define _USB_ENDPOINT_DESCRIPTOR_TYPE                    0x05
#define _USB_INTERFACE_POWER_DESCRIPTOR_TYPE             0x08
#define _USB_OTG_DESCRIPTOR_TYPE                         0x09
#define _USB_DEBUG_DESCRIPTOR_TYPE                       0x0A
#define _USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE       0x0B
#define _USB_BOS_DESCRIPTOR_TYPE                         0x0F
#define _USB_DEVICE_CAPABILITY_DESCRIPTOR_TYPE           0x10
#define _USB_EP_COMPANION_DESCRIPTOR_TYPE                0x30


#define _USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE            0x06
#define _USB_OTHER_SPEED_CONFIGURATION_TYPE              0x07
#define _USB_POWER_DESCRIPTOR_TYPE                       0x08

// Device Capability Type defintion for BOS Descriptor
#define _USB_EXTENSION_TYPE                              0x02
#define _USB_SUPERSPEED_USB_TYPE                         0x03
#define _USB_CONTAINER_ID_TYPE                           0x04
#define _USB_BILLBOARD_TYPE                              0x0D
#define _USB_BILLBOARD_ALTMODE_TYPE                      0x0F

// string descriptor index
#define _USB_INDEX0                                      0x00

#define _USB_INDEX1                                      0x21
#define _USB_INDEX2                                      0x22
#define _USB_INDEX3                                      0x23
#define _USB_INDEX4                                      0x24
#define _USB_INDEX5                                      0x25

#define _USB_BB_INDEX0                                   0x10
#define _USB_BB_INDEX1                                   0x11
#define _USB_BB_INDEX2                                   0x12
#define _USB_BB_INDEX3                                   0x13

#define _USB_RT_HUB_STRING_INDEX0                        0x34   // "R"
#define _USB_RT_HUB_STRING_INDEX1                        0x36   // "T"

// BB Capability Descriptor
#define _TYPE_C_BILLBOARD_URL_INDEX_IADDTIONALINFO       0x40

// Device status
#define _USB_DEV_STATUS_BUS_POWERED                      0x00
#define _USB_DEV_STATUS_SELF_POWERED                     0x01
#define _USB_DEV_STATUS_REMOTE_WAKEUP                    0x02
#define _USB_DEV_STATUS_U1_EN                            0x04
#define _USB_DEV_STATUS_U2_EN                            0x08
#define _USB_DEV_STATUS_LTM_EN                           0x10


/////////////////////////////
////////Vendor Request///////
/////////////////////////////

// Debug
#define _USB_VENDOR_SET_REGISTER_VALUE                   0x01
#define _USB_VENDOR_GET_REGISTER_VALUE                   0x02
#define _USB_VENDOR_WRITE_SYSEEPROM                      0x03
#define _USB_VENDOR_READ_SYSEEPROM                       0x04

// Debug Message
#define _USB_VENDOR_DEBUGMSG_EVENT_START                 0x30
#define _USB_VENDOR_DEBUGMSG_EVENT_FINISH                0x31
#define _USB_VENDOR_DEBUGMSG_START                       0x32
#define _USB_VENDOR_DEBUGMSG_END                         0x33
#define _USB_VENDOR_DEBUGMSG_GET_VALUE                   0x34
#define _USB_VENDOR_DEBUGMSG_GET_STRING                  0x35

//ISP
#define _USB_VENDOR_WRITE_TO_FLASH                       0x40
#define _USB_VENDOR_READ_FROM_FLASH                      0x41
#define _USB_VENDOR_FLASH_SECTOR_ERASE                   0x42
#define _USB_VENDOR_FLASH_BANK_ERASE                     0x43
#define _USB_VENDOR_ISP_ENABLE                           0x44
// #define _USB_VENDOR_DUAL_BANK_PROCESS                 0x45

//Relatek Handshake
#define _USB_VENDOR_REALTEK_BB_HANDSHAKE                 0x50

///////////////////////////////////////
////////DUAL BANK PROCESS OPCODE///////
///////////////////////////////////////

#define _USB_SVID_MAX_NUM                                12   // Now default support 12 SVID number
#define _USB_VENDOR_CMD_MAX_LEN                          256  // Now default support 12 SVID number
#define _USB_MCM_FLASH_USB_PIDVID                        0xD000
#define _USB_MCM_FLASH_USB_MANUFACTORY                   0xD008
#define _USB_MCM_FLASH_USB_PRODUCT                       0xD028


#define _IMANUFACTUR_SIZE                                62 // total 31 char
#define _IPRODUCT_SIZE                                   62 // total 31 char

//****************************************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************************************
// These descriptors should follow the Billboard Capability Descriptor,
// Adding the N descriptors increases the BOS Descriptor wTotalLength by N*8 and bNumDeviceCaps by N.
typedef struct
{
    unsigned char ucLength;                  // bLength
    unsigned char ucDescriptorType;          // bDescriptorType
    unsigned char ucDevCapabilityType;       // bDevCapabilityType
    unsigned char ucIndex;                   // bIndex
    unsigned char pucAlternateModeVdo[4];    // bAlternateModeVdo
} StructUsbBBAltModeCapabilityDescriptor;

typedef struct
{
    unsigned char ucAltModeStauts;
    unsigned char ucSVID_LO;
    unsigned char ucSVID_HI;
    unsigned char ucMode;
    unsigned char ucStringIndex;
    unsigned char ucdwAlternateModeVdo_Byte0;
    unsigned char ucdwAlternateModeVdo_Byte1;
    unsigned char ucdwAlternateModeVdo_Byte2;
    unsigned char ucdwAlternateModeVdo_Byte3;
    unsigned char ucCurAltModeActive;
} StructTypeCBillboardInfo;

//****************************************************************************************************
// VARIABLE EXTERN
//****************************************************************************************************


//****************************************************************************************************
// FUNCTION EXTERN
//****************************************************************************************************

extern void UsbBillboardIntHandler_EXINT0(void);
extern void UsbBillboardStandardRequest(void);
extern void UsbBillboardVendorSetRegisterByte(void);
extern void UsbBillboardVendorGetRegisterByte(void);
extern void UsbBillboardVendorReadFlash(void);

#if(_FLASH_WRITE_FUNCTION_SUPPORT == _ON)
#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
extern void UsbBillboardVendorWriteFlash(void);
#endif
extern void UsbBillboardVendorSectorErase(void);
#endif
extern void UsbBillboardVendorBankErase(void);
extern void UsbBillboardVendorIspEnable(void);
extern void UsbBillboardVendorHandshake(void);

#if(_USB_TYPE_C_BB_PROGRAM_SUPPORT == _ON)
extern void UsbBillboardVendorDualBankProcess1(void);
extern void UsbBillboardVendorDualBankProcess2(void);

extern void UsbBillboardVendorGetIspStartBank(void);
extern void UsbBillboardVendorGetIspKeyAddr(void);
extern void UsbBillboardVendorGetIspFalgAddr(void);
extern void UsbBillboardVendorGetIspSignatureAddr(void);
extern void UsbBillboardVendorGetVerifyCopyResult(void);
extern void UsbBillboardVendorGetCurrentKeyAddr(void);
extern void UsbBillboardVendorGetFWInfo(void);
extern void UsbBillboardVendorSwitchCode(void);
#endif

extern void UsbBillboardVendorDebugMsgEventStart(void);
extern void UsbBillboardVendorDebugMsgEventFinish(void);
extern void UsbBillboardVendorDebugMsgStart(void);
extern void UsbBillboardVendorDebugMsgEnd(void);
extern void UsbBillboardVendorDebugMsgGetValue(void);
extern void UsbBillboardVendorDebugMsgGetString(void);

extern void UsbBillboardVendorWriteSyseeprom(void);
extern void UsbBillboardVendorReadSyseeprom(void);
extern void UsbBillboardTransmtDataToHost(unsigned int usLength, unsigned char *pucDataBuf);
extern void UsbBillboardVendorRequest(void);

extern void UsbBillboardEepromWrite(bool bSysDefualtIICPin, unsigned char ucSlaveAddr, unsigned char ucSubAddrLength, unsigned int usSubAddr, unsigned int usLength, unsigned char *pucWriteArray);
extern void UsbBillboardEepromRead(bool bSysDefualtIICPin, unsigned char ucSlaveAddr, unsigned char ucSubAddrLength, unsigned int usSubAddr, unsigned int usLength, unsigned char *pucWriteArray);
extern void UsbBillboardGetDescriptor(void);
extern void UsbBillboardSetAddr(void);
extern void UsbBillboardSetConf(void);
extern void UsbBillboardGetInterface(void);
extern void UsbBillboardSetInterface(void);
extern void UsbBillboardGetStatus(void);
extern void UsbBillboardGetConf(void);
extern void UsbBillboardSetFeature(void);
extern void UsbBillboardClearFeature(void);
extern void UsbBillboardInitCtrlVariable(void);
extern void UsbBillboardGetAltModeInfo(void);
extern bool UsbBillboardCtrlXfer(unsigned char ucXferType, unsigned char ucIsLastPacket);
extern bool UsbBillboardXferForGetDescriptor(unsigned char ucXferType, unsigned char ucIsLastPacket);


#endif //__RTK_USB_BILLBOARD__