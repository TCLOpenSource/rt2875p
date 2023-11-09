#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <rtk_kdriver/io.h>
#include <linux/mutex.h>
#include "rtk_otp_debug.h"
#include "rtk_otp_reg.h"
#include "rtk_otp_region.h"
#include "rtk_otp_hw.h"

#define WRITE_REG32(addr, val)  rtd_outl((addr), val)
#define READ_REG32(addr)          rtd_inl((addr))

static DEFINE_MUTEX(otp_w_mutex);

/*-----------------------------------------------------------------------------
* Func : rtk_otp_is_readable
*
* Desc : Test if the OTP Reg you want to read is readable
*
* Parm : ofst : byte offsets, it should be 4 bytes alignments
*
* Retn : 0: can read;1 can't read.
*-----------------------------------------------------------------------------*/
static int rtk_otp_is_readable(unsigned int ofst)
{
    /*if(ofst <= 0x34 || (ofst <= 0x130 && ofst >= 0xF4)
        || (ofst <= 0x29C && ofst >= 0x27C) || ofst >= 0x600)
            return -1;*/
    return 0;
}


/*-----------------------------------------------------------------------------
* Func : rtk_otp_hw_read
*
* Desc : read OTP word
*
* Parm : ofst : byte offsets, it should be 4 bytes alignments
*
* Retn : OTP words
*-----------------------------------------------------------------------------*/
static int rtk_otp_hw_read(unsigned int ofst, unsigned int *pval)
{
    if (ofst & 0x3) {
        OTP_WARNING("read otp failed, ofst 0x%08x is not 4 bytes alignments\n", ofst);
        return -1;
    }
    if (rtk_otp_is_readable(ofst) == -1) {
        *pval = 0xdeaddead;
    } else {
        
        *pval = READ_REG32(OTP_DDR_MAP_BASE + ofst);
    }
    return 0;
}


/*-----------------------------------------------------------------------------
 * Func : rtk_otp_hw_crc
 *
 * Desc : rtk_otp_hw_crc
 *
 * Parm : start_ofst:the offset of start reg you want to verify.
 *      end_ofst:the offset of end reg you want to verify.
 *      pval:the address of the crc value
 * Retn : OTP words
 *-----------------------------------------------------------------------------*/
static int rtk_otp_hw_crc(unsigned int start_ofst, unsigned int len, unsigned int *pval)
{
    int ret = -1;
    int times = 200;
    if (start_ofst >= MAX_OTP_OFST || (start_ofst + len) > MAX_OTP_OFST || pval == NULL) {
        OTP_WARNING("params are input error\n");
        return ret;
    }
    mutex_lock(&otp_w_mutex);
    WRITE_REG32(OTP_CRC, OTP_CRC_CRC_INI);
    OTP_DBG("[%s,%d,%s]0x%08x = 0x%08x\n", __FILE__, __LINE__, __FUNCTION__, OTP_CRC, READ_REG32(OTP_CRC));

    WRITE_REG32(OTP_AUTO_RD_REGION, OTP_AUTO_RD_REGION_START_ADDR(start_ofst)
                | OTP_AUTO_RD_REGION_END_ADDR(start_ofst + len - 1));
    OTP_DBG("[%s,%d,%s]0x%08x = 0x%08x\n", __FILE__, __LINE__, __FUNCTION__, OTP_AUTO_RD_REGION, READ_REG32(OTP_AUTO_RD_REGION));

    WRITE_REG32(OTP_CTRL_CRC_EN, OTP_CRC_EN(1));
    OTP_DBG("[%s,%d,%s]0x%08x = 0x%08x\n", __FILE__, __LINE__, __FUNCTION__, OTP_CTRL_CRC_EN, READ_REG32(OTP_CTRL_CRC_EN));

    WRITE_REG32(OTP_CTRL, (OTP_CTRL_AUTO_RD_REQ | OTP_CTRL_WRITE_EN0));
    OTP_DBG("[%s,%d,%s]0x%08x = 0x%08x\n", __FILE__, __LINE__, __FUNCTION__, OTP_CTRL, READ_REG32(OTP_CTRL));

    while (times --) {
        unsigned int value = READ_REG32(OTP_CTRL);
        if (!(value & OTP_CTRL_AUTO_RD_REQ)) {
            ret = 0;
            break;
        }
        udelay(1000);
    }
    if (ret == 0)
        *pval = GET_OTP_CRC_CRC_OUT(READ_REG32(OTP_CRC));
    WRITE_REG32(OTP_CTRL_CRC_EN, OTP_CRC_EN(0));
    mutex_unlock(&otp_w_mutex);
    return ret;
}

RTK_OTP_HW_OPS  g_rtk_otp_hw_ops =
{
    .otp_hw_read = rtk_otp_hw_read,
    .otp_hw_crc = rtk_otp_hw_crc,
};