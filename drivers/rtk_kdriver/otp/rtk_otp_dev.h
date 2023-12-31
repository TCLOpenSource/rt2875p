#ifndef __OTP_DEV_H__
#define __OTP_DEV_H__

#define OTP_CMD_READ                          0x1101
#define OTP_CMD_PARTIAL_CRC             0x1103
#define OTP_CMD_AUTO_CRC                  0x1104
#define OTP_CMD_READ_REGION               0x1105
#define OTP_CMD_READ_NAME                 0x1106

#ifdef CONFIG_COMPAT
#define COMPAT_OTP_CMD_READ             0x1101
#endif

typedef struct {
    unsigned int addr;
    unsigned int len;
    unsigned char *data;
} RTK_OTP_RW;

typedef struct {
    const char *name;
    unsigned int name_len;
    unsigned int len;
    unsigned char *data;
} RTK_OTP_R_NAME;

typedef struct {
    unsigned int addr;
    unsigned int len;
    unsigned int crc_value;
} RTK_OTP_PARTIAL_CRC;

#ifdef CONFIG_COMPAT
typedef struct {
    unsigned int addr;
    unsigned int len;
    unsigned int data;
} COMPAT_RTK_OTP_RW;

typedef struct {
    unsigned int name;
    unsigned int name_len;
    unsigned int len;
    unsigned int data;
} COMPAT_RTK_OTP_R_NAME;
#endif

#endif
