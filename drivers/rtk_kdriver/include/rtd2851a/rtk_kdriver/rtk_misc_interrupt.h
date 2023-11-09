#ifndef __RTK_MISC_INTERRUPT_H__
#define __RTK_MISC_INTERRUPT_H__


typedef enum
{
    ISO_EMPTY  = 0 ,
    ISO_UART   = 0x1 ,
    ISO_I2C    = 0x1 << 1,
    ISO_GPIO   = 0x1 << 2,
} ISO_IP;


typedef enum
{
    MISC_EMPTY  = 0 ,
    MISC_UART   = 0x01 ,
    MISC_I2C    = 0x01 << 1 ,
    MISC_GPIO   = 0x01 << 2 ,
} MISC_IP;


#define MISC_INTERRUPT_WARN(fmt, args...)	rtd_pr_misc_interrupt_warn("[Warn] " fmt, ## args)


int setIsoInterrupt(int isoIp , int enable );
int setMiscInterrupt(int isoIp , int enable );

#endif