#ifndef __RTK_OTP_UTIL_H__
#define __RTK_OTP_UTIL_H__

typedef struct 
{
    int (*otp_read)(unsigned int ofst, unsigned int *pval);
    int (*otp_read_bits)(unsigned int ofst, unsigned char msb, unsigned char lsb, unsigned int *pval);
    int (*otp_read_region)(unsigned int ofst, unsigned int *data, int len);
    int (*otp_read_region_byte)(unsigned int ofst, unsigned char *data, int len);
    int (*otp_crc_auto_region)(unsigned int *pval);
    int (*otp_crc_partial_region)(unsigned int start_ofst, unsigned int len, unsigned int *pval);
    
}RTK_OTP_OPS;

#endif
