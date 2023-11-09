#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include "rtk_otp_debug.h"
#include "rtk_otp_reg.h"
#include "rtk_otp_hw.h"
#include "rtk_otp_util.h"
extern RTK_OTP_HW_OPS  g_rtk_otp_hw_ops;

#define OTP_BIT_MASK(i)          (0xFFFFFFFF >> (32-(i)))


/*-----------------------------------------------------------------------------
 * Func : _otp_get_bits
 *
 * Desc : get specified bits
 *
 * Parm :
 *
 * Retn :
 *-----------------------------------------------------------------------------*/
static int _otp_get_bits(unsigned int *pval, unsigned char msb, unsigned char lsb)
{
    if (msb < lsb || lsb > 31 || msb > 31)
        return -1;
    *pval >>= lsb;
    *pval &= OTP_BIT_MASK(msb - lsb + 1);
    return 0;
}


/*-----------------------------------------------------------------------------
* Func : otp_read
*
* Desc : read OTP word
*
* Parm : ofst : byte offsets, it should be 4 bytes alignments
*
* Retn : OTP words
*-----------------------------------------------------------------------------*/
static int otp_read(unsigned int ofst, unsigned int *pval)
{
    return g_rtk_otp_hw_ops.otp_hw_read(ofst, pval);
}


/*-----------------------------------------------------------------------------
 * Func : otp_read_region
 *
 * Desc : read OTP region
 *
 * Parm : ofst : byte offsets, it should be 4 bytes alignments
 *
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/
static int otp_read_region(unsigned int ofst, unsigned int *data, int len)
{
    int i = 0;
    if (ofst & 0x3) {
        OTP_WARNING("read otp region failed, ofst 0x%08x is not 4 bytes alignments\n", ofst);
        return -1;
    }
    if (data == NULL || len < 0)
        return -1;
    for (i = 0; i < len; i++) {
        if (otp_read(ofst, (unsigned int *)data) != 0) {
            return -1;
        }
        ofst += sizeof(unsigned int);
        data++;
    }
    return 0;
}

/*-----------------------------------------------------------------------------
 * Func : otp_read_region_byte
 *
 * Desc : read OTP region
 *
 * Parm : ofst : byte offsets, it should be 4 bytes alignments
 *
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/
static int otp_read_region_byte(unsigned int ofst,
                         unsigned char *data, int len)
{
    unsigned char *tmp = data;
    if (tmp == NULL || len < 0) {
        OTP_WARNING("otp program region failed, params are input error\n");
        return -1;
    }
    /*if ofst is not align by 4 bytes,should seperate read the ualigned parts*/
    if ((ofst & 0x3) && len > 0) {
        unsigned int val = 0;
        unsigned char start_byte = (ofst & 0x3);
        unsigned char read_len = ((len >= sizeof(unsigned int) - start_byte)) ? (sizeof(unsigned int) - start_byte) : len;
        otp_read(ofst - start_byte, &val);
        ofst += read_len;
        len -= read_len;
        val = val >> (start_byte * 8);
        while (read_len > 0) {
            *tmp = val & 0xFF;
            val >>= 8;
            tmp++;
            read_len--;
        }
        if (len <= 0)
            return 0;

    }

    if (len / sizeof(int) > 0) {
        int read_len = len / sizeof(int);
        unsigned int val = 0;
        while (read_len > 0) {
            unsigned char times = 4;
            otp_read(ofst, &val);
            while (times > 0) {
                *tmp = val & 0xFF;
                tmp++;
                val >>= 8;
                times--;
            }
            ofst += sizeof(unsigned int);
            read_len--;
        }
        len = len - (len / sizeof(int)) * sizeof(int);
    }
    if (len > 0) {
        unsigned int val = 0;
        otp_read(ofst, &val);
        while (len > 0) {
            *tmp = val & 0xFF;
            val >>= 8;
            tmp++;
            ofst += 1;
            len--;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------------
 * Func : otp_read_bits
 *
 * Desc : read OTP bits
 *
 * Parm : ofst : byte offsets, it should be 4 bytes alignments
 *
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/
static int otp_read_bits(unsigned int ofst, unsigned char msb,
                  unsigned char lsb, unsigned int *pval)
{
    int ret = otp_read(ofst, pval);

    if (ret == 0 && *pval != 0xdeaddead)    /* do not mask bits if it is non readable*/
        ret = _otp_get_bits(pval, msb, lsb);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Func : otp_crc_auto_region
 *
 * Desc : opt crc auto region
 *
 * Parm : pval:pval:the address of the crc value
 *
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/

static int otp_crc_auto_region(unsigned int *pval)
{
    return g_rtk_otp_hw_ops.otp_hw_crc(0x0, MAX_OTP_OFST, pval);
}


/*-----------------------------------------------------------------------------
 * Func : otp_crc_partial_region
 *
 * Desc : otp_crc_partial_region
 *
 * Parm : start_ofst:the offset of start reg you want to verify.
 *      end_ofst:the offset of end reg you want to verify.
 *      pval:the address of the crc value
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/
static int otp_crc_partial_region(unsigned int start_ofst, unsigned int len, unsigned int *pval)
{
    return g_rtk_otp_hw_ops.otp_hw_crc(start_ofst, len, pval);
}


RTK_OTP_OPS g_rtk_otp_ops =
{
    .otp_read = otp_read,
    .otp_read_bits = otp_read_bits,
    .otp_read_region = otp_read_region,
    .otp_read_region_byte = otp_read_region_byte,
    .otp_crc_auto_region = otp_crc_auto_region,
    .otp_crc_partial_region = otp_crc_partial_region,  
};

