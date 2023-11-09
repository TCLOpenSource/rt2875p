#ifndef __RTK_OTP_HW_H__
#define __RTK_OTP_HW_H__


typedef struct 
{
    int (*otp_hw_read)(unsigned int ofst, unsigned int *pval); 
    int (*otp_hw_crc)(unsigned int start_ofst, unsigned int len, unsigned int *pval); 
    
}RTK_OTP_HW_OPS;
#endif