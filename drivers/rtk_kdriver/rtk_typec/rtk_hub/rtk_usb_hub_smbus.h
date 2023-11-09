#ifndef __RTK_USB_HUB_SMBUS_INIT__
#define __RTK_USB_HUB_SMBUS_INIT__

#define RTK_SMBUS_TEST_READ            0x30
#define RTK_SMBUS_READ                 0x21
#define RTK_SMBUS_BLOCK_READ           0x80

#define RTK_SMBUS_BLOCK_WRITE_TYPE        0
#define RTK_SMBUS_BLOCK_READ_TYPE         1
#define RTK_SMBUS_NON_BLOCK_READ_TYPE     2

bool rtk_smbus_read_data_transfer(unsigned char address_H, 
				unsigned char address_L, unsigned int *pVal);
#endif	/* __RTK_USB_HUB_SMBUS_INIT__ */
