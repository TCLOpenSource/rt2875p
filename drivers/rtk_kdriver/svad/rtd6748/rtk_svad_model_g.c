/******************************************************************************
 *
 *   Copyright(c) 2021. Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author timo.wu@realtek.com
 *
 *****************************************************************************/
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/string.h>
#include <linux/freezer.h>

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/platform.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_svad_def.h>
#include <rtk_kdriver/rtk_svad_api.h>

#include "rbus/MIO/mio_sys_reg_reg.h"
#include "rbus/MIO/smart_vad_reg.h"


/******************************************************************************
                     SVAD only works on WOV DMIC path
*****************************************************************************/

extern int rtk_svad_4bytes_i2c_wr(unsigned int addr, unsigned int data);
extern int rtk_svad_4bytes_i2c_rd(unsigned int addr, unsigned int* data);
extern int rtk_svad_4bytes_i2c_mask(unsigned int addr, unsigned int mask, unsigned int data);
extern int rtk_svad_crt_clk_onoff(enum CRT_CLK clken);

void rtk_svad_set_model_g(unsigned int scale, unsigned int dnn_th, unsigned int kw_weight)
{
#ifdef MIO_I2C_MODE
    int ret = 0;
    unsigned int SVAD_tcon_1_reg = 0;
    unsigned int reg_value_sys_srst = 0;
    unsigned int reg_value_sys_clken = 0;
#endif

    SVAD_INFO("\033[1;32;33m" "rtk_svad_set_model_g: scale=%u, dnn_th=%u, kw_weight=%u" "\033[m", scale, dnn_th, kw_weight);

#ifndef MIO_I2C_MODE
    if(((rtd_inl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
    {
        SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
        rtk_svad_crt_clk_onoff(CLK_ON);
    }

    //Mfcc parameters
    rtd_outl(SMART_VAD_svad_coef_sram_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    
    //Line 1
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0cf210fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x11f5f5f6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1111f6fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f40b06);
    udelay(10);
    //Line 2
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010ffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fcf9f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0905fdfb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff50205);
    udelay(10);
    //Line 3
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050209fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040002fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f800fd);
    udelay(10);
    //Line 4
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff06fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f9fcfa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa0201);
    udelay(10);
    //Line 5
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb06f802);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010108);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd00ff);
    udelay(10);
    //Line 6
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04faf303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000209);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    udelay(10);
    //Line 7
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301f503);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fd04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    udelay(10);
    //Line 8
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fffc03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0005fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfafb07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0406fe03);
    udelay(10);
    //Line 9
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90405fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf7fc08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0604fefe);
    udelay(10);
    //Line 10
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80706ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2f1fb07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000104);
    udelay(10);
    //Line 11
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a09f80d);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010a0909);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0008);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0605fefe);
    udelay(10);
    //Line 12
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d02fa00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05030100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0001);
    udelay(10);
    //Line 13
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601fcfb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbffff);
    udelay(10);
    //Line 14
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010306);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe060400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fefd);
    udelay(10);
    //Line 15
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08040301);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020202fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030000);
    udelay(10);
    //Line 16
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b030101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf901ff00);
    udelay(10);
    //Line 17
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02070203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9040204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00fbfc);
    udelay(10);
    //Line 18
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010bfb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fa02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003fff8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800fdfc);
    udelay(10);
    //Line 19
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0204fa00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9090000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fdfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fafd);
    udelay(10);
    //Line 20
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffaf9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfff901);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0202f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf701fcfd);
    udelay(10);
    //Line 21
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1103fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f4f8ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fa090c);
    udelay(10);
    //Line 22
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfd0200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fbef01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ee0509);
    udelay(10);
    //Line 23
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fbe600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f40203);
    udelay(10);
    //Line 24
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcfffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fc00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdf2fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f70003);
    udelay(10);
    //Line 25
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0004f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f300fe);
    udelay(10);
    //Line 26
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefa01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0001);
    udelay(10);
    //Line 27
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0004fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fffa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc04fffb);
    udelay(10);
    //Line 28
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe01f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffcfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0900fc);
    udelay(10);
    //Line 29
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf905fdfb);
    udelay(10);
    //Line 30
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf50605fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fbf9);
    udelay(10);
    //Line 31
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0df313);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa090e11);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf900030e);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040ffdfd);
    udelay(10);
    //Line 32
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000603);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f7fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040101);
    udelay(10);
    //Line 33
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0f06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06030400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fdf805);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010601ff);
    udelay(10);
    //Line 34
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030603);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfaf004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020102);
    udelay(10);
    //Line 35
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf700fb04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000402);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fbef02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    udelay(10);
    //Line 36
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf401fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefafdf9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefef6fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f90001);
    udelay(10);
    //Line 37
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffe80201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff4fdf0);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffa04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff01);
    udelay(10);
    //Line 38
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f00002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf900fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff00);
    udelay(10);
    //Line 39
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf5ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf90000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    udelay(10);
    //Line 40
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f7fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0205fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040103);
    udelay(10);
    //Line 41
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000702);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfd03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffe03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa0100);
    udelay(10);
    //Line 42
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0500);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f8f803);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefffb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    udelay(10);
    //Line 43
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd06f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f6f8fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    udelay(10);
    //Line 44
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff605fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff7f8fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffb0204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010000);
    udelay(10);
    //Line 45
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa0500);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf9fa05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfc0205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    udelay(10);
    //Line 46
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010804);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefe05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafc0204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04ffff);
    udelay(10);
    //Line 47
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0701);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfc04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefa0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030101);
    udelay(10);
    //Line 48
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000f02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfb04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f90301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010102);
    udelay(10);
    //Line 49
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0c08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9f10207);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010203);
    udelay(10);
    //Line 50
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0c13);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe090e07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3e6fd20);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07030304);
    udelay(10);
    //Line 51
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0707020a);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040b0907);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fd0a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fa0100);
    udelay(10);
    //Line 52
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0cfd0600);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffbff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f20304);
    udelay(10);
    //Line 53
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080305ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ee0000);
    udelay(10);
    //Line 54
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060605fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fff9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff50101);
    udelay(10);
    //Line 55
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050404fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fff8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f800ff);
    udelay(10);
    //Line 56
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0506fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0402fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffbfe03);
    udelay(10);
    //Line 57
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a01fafa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fdfa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe03fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0203);
    udelay(10);
    //Line 58
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502fbfc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd03fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff90100);
    udelay(10);
    //Line 59
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a02feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0004ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff8ff00);
    udelay(10);
    //Line 60
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0f0100fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0405fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff80000);
    udelay(10);
    //Line 61
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0060208);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc050308);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101f9f7);
    udelay(10);
    //Line 62
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1030202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffdfd);
    udelay(10);
    //Line 63
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5020202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffefd);
    udelay(10);
    //Line 64
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe01);
    udelay(10);
    //Line 65
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3ff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fdfd);
    udelay(10);
    //Line 66
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fe0803);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001feff);
    udelay(10);
    //Line 67
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0ff0401);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    udelay(10);
    //Line 68
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeefc0406);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffe);
    udelay(10);
    //Line 69
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fffb);
    udelay(10);
    //Line 70
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2fc0806);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x070307fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fcfd12);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00fb);
    udelay(10);
    //Line 71
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02cbff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70600fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xed0104fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0205fdfc);
    udelay(10);
    //Line 72
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcffcfff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0001f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeefd02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ffff);
    udelay(10);
    //Line 73
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefce2fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0100f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4fc02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03ffff);
    udelay(10);
    //Line 74
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffbe0fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0004fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fc01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010400fe);
    udelay(10);
    //Line 75
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00e301);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030104fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03faff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    udelay(10);
    //Line 76
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fef402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000302fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0200);
    udelay(10);
    //Line 77
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff06fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb0003);
    udelay(10);
    //Line 78
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0400);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000005fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0000);
    udelay(10);
    //Line 79
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0601);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020409ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040101);
    udelay(10);
    //Line 80
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02060704);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01050501);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03050204);
    udelay(10);
    //Line 81
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0ef10601);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f402f1);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6040302);
    udelay(10);
    //Line 82
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fdfc00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd03fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf902fdfe);
    udelay(10);
    //Line 83
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f6fffa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcfbf3);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fbfc);
    udelay(10);
    //Line 84
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf7fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf900fff6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030304);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002f9fc);
    udelay(10);
    //Line 85
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb0402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdfd04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fbfe);
    udelay(10);
    //Line 86
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000e06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020403);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403fe00);
    udelay(10);
    //Line 87
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d040b03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010105);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00070203);
    udelay(10);
    //Line 88
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07010902);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07020105);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030202);
    udelay(10);
    //Line 89
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0504ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01060002);
    udelay(10);
    //Line 90
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f903fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f8fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000a09f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000a0101);
    udelay(10);
    //Line 91
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe3fdfdf7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafff8f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0506fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf603f4f0);
    udelay(10);
    //Line 92
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe0fcfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0003fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030404);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd05f5f6);
    udelay(10);
    //Line 93
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe2fd0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003f4f5);
    udelay(10);
    //Line 94
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xea000207);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb020401);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0504f7f9);
    udelay(10);
    //Line 95
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xef030902);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fefe);
    udelay(10);
    //Line 96
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2010804);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfb0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010100);
    udelay(10);
    //Line 97
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0010203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010103);
    udelay(10);
    //Line 98
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000504);
    udelay(10);
    //Line 99
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf501ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef6fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000502);
    udelay(10);
    //Line 100
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020506fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff5fa12);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fe0704);
    udelay(10);
    //Line 101
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe05f306);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8050006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3ff0d05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe15ffff);
    udelay(10);
    //Line 102
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffff3fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0203ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    udelay(10);
    //Line 103
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefa02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ffff);
    udelay(10);
    //Line 104
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefaf701);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030000);
    udelay(10);
    //Line 105
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd0302);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020202);
    udelay(10);
    //Line 106
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0602);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    udelay(10);
    //Line 107
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040402);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010101);
    udelay(10);
    //Line 108
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fbfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    udelay(10);
    //Line 109
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefeffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000201);
    udelay(10);
    //Line 110
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfcff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08ff0804);
    udelay(10);
    //Line 111
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf60af70c);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf40a0804);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00040b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f8f8);
    udelay(10);
    //Line 112
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000501);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa030103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbfbfa);
    udelay(10);
    //Line 113
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa050405);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcfcfd);
    udelay(10);
    //Line 114
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafefe);
    udelay(10);
    //Line 115
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfe00);
    udelay(10);
    //Line 116
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0202);
    udelay(10);
    //Line 117
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0301);
    udelay(10);
    //Line 118
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffbff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020401fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfafefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0201);
    udelay(10);
    //Line 119
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff3fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf8fcfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000301);
    udelay(10);
    //Line 120
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf7fa02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fe0405);
    udelay(10);
    //Line 121
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc08e812);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80c0b0b);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1fc040c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050e0101);
    udelay(10);
    //Line 122
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fceefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000201);
    udelay(10);
    //Line 123
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef9fdf9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00edeff4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff050200);
    udelay(10);
    //Line 124
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f80101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01e9f1ed);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff020a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff060202);
    udelay(10);
    //Line 125
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f8000c);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe03fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020201);
    udelay(10);
    //Line 126
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafefc04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000302fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    udelay(10);
    //Line 127
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00e802);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0303fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe9fb0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff00);
    udelay(10);
    //Line 128
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfde500);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3f90000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020101);
    udelay(10);
    //Line 129
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafee700);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 130
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fdf2ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbfffb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0102);
    udelay(10);
    //Line 131
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70aec0c);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf408060c);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0f20106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f8fdfe);
    udelay(10);
    //Line 132
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0008fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf900fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f80101);
    udelay(10);
    //Line 133
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb0202);
    udelay(10);
    //Line 134
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0105);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 135
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0204ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0106);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0006);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030001);
    udelay(10);
    //Line 136
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fc0305);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020301);
    udelay(10);
    //Line 137
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feffff);
    udelay(10);
    //Line 138
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    udelay(10);
    //Line 139
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff01);
    udelay(10);
    //Line 140
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020405);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030001);
    udelay(10);
    //Line 141
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff001ec);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08f0f7ee);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502fbf5);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f20604);
    udelay(10);
    //Line 142
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f8fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fbfff8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa0503);
    udelay(10);
    //Line 143
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfa0bfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc0302);
    udelay(10);
    //Line 144
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030204);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000108);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0201);
    udelay(10);
    //Line 145
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc041103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010105);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04050600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdff00);
    udelay(10);
    //Line 146
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000000);
    udelay(10);
    //Line 147
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa030b01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01050601);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0200fe);
    udelay(10);
    //Line 148
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9040202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdfefd);
    udelay(10);
    //Line 149
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0607fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0203fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00fdfc);
    udelay(10);
    //Line 150
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90502f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefa03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0504fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fffcf9);
    udelay(10);
    //Line 151
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f8f7ea);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f5f7e6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0900ffe9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010604);
    udelay(10);
    //Line 152
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f9fae6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fbfee3);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffffec);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010102);
    udelay(10);
    //Line 153
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fafae2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f8fce6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffe9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    udelay(10);
    //Line 154
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfcd9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff8f7e0);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000df);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002ff);
    udelay(10);
    //Line 155
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f7fed4);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f2f0d9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff00dd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010001);
    udelay(10);
    //Line 156
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fbfde2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdf7ea);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0101ff);
    udelay(10);
    //Line 157
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fef8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fef4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    udelay(10);
    //Line 158
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060302fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fefe);
    udelay(10);
    //Line 159
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0504fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    udelay(10);
    //Line 160
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040902ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04fe07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02100500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fefe);
    udelay(10);
    //Line 161
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00d60fef);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04e5f4e2);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0804fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f80101);
    udelay(10);
    //Line 162
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ef0507);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff020e);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    udelay(10);
    //Line 163
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf70b03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030505);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    udelay(10);
    //Line 164
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafc0200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040100);
    udelay(10);
    //Line 165
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01050000);
    udelay(10);
    //Line 166
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfa0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020203fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020000);
    udelay(10);
    //Line 167
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000104fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030000);
    udelay(10);
    //Line 168
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    udelay(10);
    //Line 169
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0300ff);
    udelay(10);
    //Line 170
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020602fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdfb05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    udelay(10);
    //Line 171
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb030804);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f4f7fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf902f300);
    udelay(10);
    //Line 172
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf601ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0900);
    udelay(10);
    //Line 173
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa01f905);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd02ff);
    udelay(10);
    //Line 174
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0102);
    udelay(10);
    //Line 175
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010006fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010102fe);
    udelay(10);
    //Line 176
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff05fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020201);
    udelay(10);
    //Line 177
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90119fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fefaee);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fd0afe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8feff00);
    udelay(10);
    //Line 178
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0101fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0005fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd04fefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0500);
    udelay(10);
    //Line 179
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe040504);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0000);
    udelay(10);
    //Line 180
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010601);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe05fe);
    udelay(10);
    //Line 181
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000401ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff03ff);
    udelay(10);
    //Line 182
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0106ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010505);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffdf8);
    udelay(10);
    //Line 183
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf604f708);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcf605);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefcfcff);
    udelay(10);
    //Line 184
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f9f5f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030206);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303f9f0);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd05fe);
    udelay(10);
    //Line 185
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020502);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfdfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    udelay(10);
    //Line 186
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc01f5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd03ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00fffe);
    udelay(10);
    //Line 187
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0001fd);
    udelay(10);
    //Line 188
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030202);
    udelay(10);
    //Line 189
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf00dfe07);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fafa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f00c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcee01);
    udelay(10);
    //Line 190
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fb01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb03fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fafa00);
    udelay(10);
    //Line 191
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000207);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfa01);
    udelay(10);
    //Line 192
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fc01);
    udelay(10);
    //Line 193
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010300fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fc01);
    udelay(10);
    //Line 194
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff060000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdff0b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fdff);
    udelay(10);
    //Line 195
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f9f4f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb080907);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0306fef9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fefd00);
    udelay(10);
    //Line 196
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf803fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcf901);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fff6fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fa01);
    udelay(10);
    //Line 197
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003ff);
    udelay(10);
    //Line 198
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffffd);
    udelay(10);
    //Line 199
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000301);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff01ff);
    udelay(10);
    //Line 200
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000101);
    udelay(10);
    //Line 201
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fef9fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020403);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020809);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0504fefb);
    udelay(10);
    //Line 202
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402f8fb);
    udelay(10);
    //Line 203
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2040103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fb02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03f800);
    udelay(10);
    //Line 204
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010602);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000fc);
    udelay(10);
    //Line 205
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff07fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fbf1);
    udelay(10);
    //Line 206
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf60404fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfff9fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff6050e);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0800f4eb);
    udelay(10);
    //Line 207
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0007fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fdfa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fc01);
    udelay(10);
    //Line 208
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03fe03);
    udelay(10);
    //Line 209
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0103ff);
    udelay(10);
    //Line 210
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe02);
    udelay(10);
    //Line 211
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    udelay(10);
    //Line 212
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010204);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff08feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010104);
    udelay(10);
    //Line 213
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fd04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf50200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbf700);
    udelay(10);
    //Line 214
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000207);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb00fe);
    udelay(10);
    //Line 215
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa00ff);
    udelay(10);
    //Line 216
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0202fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f9fefe);
    udelay(10);
    //Line 217
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00fc);
    udelay(10);
    //Line 218
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff8ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0c0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff05fff9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd00);
    udelay(10);
    //Line 219
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf016f5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030002f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09010501);
    udelay(10);
    //Line 220
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef403ef);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    udelay(10);
    //Line 221
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff800ea);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc01f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fd01);
    udelay(10);
    //Line 222
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff02f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfffa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe04);
    udelay(10);
    //Line 223
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefcfefb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000401);
    udelay(10);
    //Line 224
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0101);
    udelay(10);
    //Line 225
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f7f801);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0203f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfafefe);
    udelay(10);
    //Line 226
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f7fafa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000300fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0204fe);
    udelay(10);
    //Line 227
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdff);
    udelay(10);
    //Line 228
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    udelay(10);
    //Line 229
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfefa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01ff);
    udelay(10);
    //Line 230
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdfa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0501ff);
    udelay(10);
    //Line 231
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00faf4ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08080203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fa07ff);
    udelay(10);
    //Line 232
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfafd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    udelay(10);
    //Line 233
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020000);
    udelay(10);
    //Line 234
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101ff);
    udelay(10);
    //Line 235
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    udelay(10);
    //Line 236
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fb03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe06fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    udelay(10);
    //Line 237
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf502fb04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fa0d);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdf90b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa04f702);
    udelay(10);
    //Line 238
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01faf802);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0301);
    udelay(10);
    //Line 239
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdfdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000505ff);
    udelay(10);
    //Line 240
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    udelay(10);
    //Line 241
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000603);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fc00);
    udelay(10);
    //Line 242
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000602);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd05f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd05fdfe);
    udelay(10);
    //Line 243
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcf901);
    udelay(10);
    //Line 244
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbff01);
    udelay(10);
    //Line 245
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0100);
    udelay(10);
    //Line 246
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000101);
    udelay(10);
    //Line 247
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa000000);
    udelay(10);
    //Line 248
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffb05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0702fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf901fc01);
    udelay(10);
    //Line 249
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010403);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf902fc05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfdfa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0bfd02);
    udelay(10);
    //Line 250
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe050205);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf7f9ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fbf8fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfbfe01);
    udelay(10);
    //Line 251
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01050201);
    udelay(10);
    //Line 252
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fb05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff02);
    udelay(10);
    //Line 253
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fd01);
    udelay(10);
    //Line 254
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc04fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfdfbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0604fdfe);
    udelay(10);
    //Line 255
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02faf600);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000507);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fdfb01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9ffffff);
    udelay(10);
    //Line 256
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f9ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe040100);
    udelay(10);
    //Line 257
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f80203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030303);
    udelay(10);
    //Line 258
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f6fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd060102);
    udelay(10);
    //Line 259
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f7fafd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0afa0403);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0303);
    udelay(10);
    //Line 260
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff8fa00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05080702);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f805fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfb0707);
    udelay(10);
    //Line 261
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f208f5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x090004ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0105fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ee00ff);
    udelay(10);
    //Line 262
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f701f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fff6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f401fe);
    udelay(10);
    //Line 263
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fafbf7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0103fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefb0002);
    udelay(10);
    //Line 264
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301f9fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfeff01);
    udelay(10);
    //Line 265
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefc00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefdff);
    udelay(10);
    //Line 266
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffef5);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0101);
    udelay(10);
    //Line 267
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0501fe);
    udelay(10);
    //Line 268
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffeff);
    udelay(10);
    //Line 269
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff00);
    udelay(10);
    //Line 270
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    udelay(10);
    //Line 271
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020000);
    udelay(10);
    //Line 272
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    udelay(10);
    //Line 273
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x011208f2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf50b08);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0610feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fced0b);
    udelay(10);
    //Line 274
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050101fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030707);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0afc02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffe0b);
    udelay(10);
    //Line 275
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90901fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0a08);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0105fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5f7f804);
    udelay(10);
    //Line 276
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0702fef5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000004ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb05000a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f9fd08);
    udelay(10);
    //Line 277
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0406fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff08ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03ff0b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdff03);
    udelay(10);
    //Line 278
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf902fcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5000506);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030506);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fcff);
    udelay(10);
    //Line 279
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0006fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfaff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff06fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    udelay(10);
    //Line 280
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    udelay(10);
    //Line 281
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    udelay(10);
    //Line 282
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 283
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 284
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    udelay(10);
    //Line 285
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fff8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1fc0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d02f9fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf4effd);
    udelay(10);
    //Line 286
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc05fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fb03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000302);
    udelay(10);
    //Line 287
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb030002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffa00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfcff);
    udelay(10);
    //Line 288
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffd02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101ff);
    udelay(10);
    //Line 289
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff05fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    udelay(10);
    //Line 290
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd03fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff05fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    udelay(10);
    //Line 291
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fdff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd03fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030000);
    udelay(10);
    //Line 292
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe00);
    udelay(10);
    //Line 293
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ffff);
    udelay(10);
    //Line 294
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    udelay(10);
    //Line 295
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    udelay(10);
    //Line 296
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    udelay(10);
    //Line 297
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe00fe);
    udelay(10);
    //Line 298
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020004fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000001);
    udelay(10);
    //Line 299
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    udelay(10);
    //Line 300
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    udelay(10);
    //Line 301
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    udelay(10);
    //Line 302
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    udelay(10);
    //Line 303
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffefd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbfe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe0202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d030205);
    udelay(10);
    //Line 304
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    udelay(10);
    //Line 305
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020100);
    udelay(10);
    //Line 306
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd00);
    udelay(10);
    //Line 307
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00feff);
    udelay(10);
    //Line 308
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ffff);
    udelay(10);
    //Line 309
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060706f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeaf9fd0b);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08ff06f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f70301);
    udelay(10);
    //Line 310
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000306);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1010102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fb01fe);
    udelay(10);
    //Line 311
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf50402fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf70200);
    udelay(10);
    //Line 312
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05020603);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafe00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fafffe);
    udelay(10);
    //Line 313
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0afd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0501fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc04fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f9f9ff);
    udelay(10);
    //Line 314
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bf91b06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b0601fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f702fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f2fafb);
    udelay(10);
    //Line 315
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0101f5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfc10);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0b04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000105);
    udelay(10);
    //Line 316
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fdfc00);
    udelay(10);
    //Line 317
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040304fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0404);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100f806);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fb0403);
    udelay(10);
    //Line 318
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04030504);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff01fe);
    udelay(10);
    //Line 319
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0306010a);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020105fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fc01fd);
    udelay(10);
    //Line 320
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa02feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040106);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020cf7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffffd);
    udelay(10);
    //Line 321
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff040304);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc050400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf8fbfd);
    udelay(10);
    //Line 322
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb02fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0100);
    udelay(10);
    //Line 323
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0006fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010202fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcfdfe);
    udelay(10);
    //Line 324
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc03fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000004fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    udelay(10);
    //Line 325
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000200);
    udelay(10);
    //Line 326
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0204fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000301);
    udelay(10);
    //Line 327
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05050500);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1802eefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5ff07f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fbf8);
    udelay(10);
    //Line 328
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b01f904);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0007ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbfff9);
    udelay(10);
    //Line 329
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030102fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfffa02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe04f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfefdf4);
    udelay(10);
    //Line 330
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040402fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a00fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe05f4);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc01fb);
    udelay(10);
    //Line 331
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010402fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fcfdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfcfff4);
    udelay(10);
    //Line 332
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0303fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b04f903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa06f3);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfa02fb);
    udelay(10);
    //Line 333
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff00);
    udelay(10);
    //Line 334
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    udelay(10);
    //Line 335
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001ff);
    udelay(10);
    //Line 336
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0100);
    udelay(10);
    //Line 337
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010101);
    udelay(10);
    //Line 338
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0902);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0100);
    udelay(10);
    //Line 339
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010801);
    udelay(10);
    //Line 340
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0000);
    udelay(10);
    //Line 341
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010301);
    udelay(10);
    //Line 342
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0200);
    udelay(10);
    //Line 343
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000200);
    udelay(10);
    //Line 344
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0100);
    udelay(10);
    //Line 345
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc05ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2fa050a);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfeea06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f80100);
    udelay(10);
    //Line 346
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfc0307);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010b0904);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fef001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040408fc);
    udelay(10);
    //Line 347
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc050505);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0704f4fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd02fe);
    udelay(10);
    //Line 348
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf60501fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb05f602);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106fbfb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    udelay(10);
    //Line 349
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010303fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fd00);
    udelay(10);
    //Line 350
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff0203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fbf801);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc06f5fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0407fc00);
    udelay(10);
    //Line 351
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f30804);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404f507);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9060c0b);
    udelay(10);
    //Line 352
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffb02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa04f4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303f504);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb09ff01);
    udelay(10);
    //Line 353
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd04fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f201fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02f902);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6090405);
    udelay(10);
    //Line 354
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd04fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf501fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0109faff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9120205);
    udelay(10);
    //Line 355
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f70202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0700fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000cfd06);
    udelay(10);
    //Line 356
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf509fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfafe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc18f5f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd16fb16);
    udelay(10);
    //Line 357
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ff02);
    udelay(10);
    //Line 358
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe00);
    udelay(10);
    //Line 359
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    udelay(10);
    //Line 360
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    udelay(10);
    //Line 361
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 362
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    udelay(10);
    //Line 363
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffff9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefbff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060002fe);
    udelay(10);
    //Line 364
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0702ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0af600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000c03);
    udelay(10);
    //Line 365
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0301fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc090501);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07ff0402);
    udelay(10);
    //Line 366
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fc0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500f800);
    udelay(10);
    //Line 367
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe03);
    udelay(10);
    //Line 368
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffff9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fefa00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000200);
    udelay(10);
    //Line 369
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000501);
    udelay(10);
    //Line 370
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0009);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f70300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010401);
    udelay(10);
    //Line 371
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 372
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000400);
    udelay(10);
    //Line 373
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0101);
    udelay(10);
    //Line 374
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe00);
    udelay(10);
    //Line 375
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    udelay(10);
    //Line 376
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfcfe06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdf9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0502fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff05ff);
    udelay(10);
    //Line 377
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffbff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf10bf601);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08ff05fc);
    udelay(10);
    //Line 378
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfd06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f80500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7000602);
    udelay(10);
    //Line 379
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010107);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000801);
    udelay(10);
    //Line 380
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000400);
    udelay(10);
    //Line 381
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000201);
    udelay(10);
    //Line 382
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fefe);
    udelay(10);
    //Line 383
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff01);
    udelay(10);
    //Line 384
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    udelay(10);
    //Line 385
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fe01);
    udelay(10);
    //Line 386
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    udelay(10);
    //Line 387
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    udelay(10);
    //Line 388
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe00);
    udelay(10);
    //Line 389
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0cfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe30ef301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010002);
    udelay(10);
    //Line 390
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80200f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe080101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fff500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fc00);
    udelay(10);
    //Line 391
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101f5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc050008);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203f900);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefbfd);
    udelay(10);
    //Line 392
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff01f4);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0001fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf703fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff0108);
    udelay(10);
    //Line 393
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfb00f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf00bf700);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0bff);
    udelay(10);
    //Line 394
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbf604);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0bfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffb01);
    udelay(10);
    //Line 395
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000406ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffff4ff);
    udelay(10);
    //Line 396
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f90000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafffe00);
    udelay(10);
    //Line 397
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0703);
    udelay(10);
    //Line 398
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f5fcfc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f80200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf900fc01);
    udelay(10);
    //Line 399
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f5fbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fc0401);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0200);
    udelay(10);
    //Line 400
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf802fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010604);
    udelay(10);
    //Line 401
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000503);
    udelay(10);
    //Line 402
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ffff0f);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf8fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6ff0301);
    udelay(10);
    //Line 403
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fd0d);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9000801);
    udelay(10);
    //Line 404
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010005ff);
    udelay(10);
    //Line 405
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01faff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef801ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000a01);
    udelay(10);
    //Line 406
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc05ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010501);
    udelay(10);
    //Line 407
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0105);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb04feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    udelay(10);
    //Line 408
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00050100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fffe);
    udelay(10);
    //Line 409
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe00);
    udelay(10);
    //Line 410
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000301);
    udelay(10);
    //Line 411
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fc00);
    udelay(10);
    //Line 412
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fb00);
    udelay(10);
    //Line 413
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fe00);
    udelay(10);
    //Line 414
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000101);
    udelay(10);
    //Line 415
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fb04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010302fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafef603);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfd00);
    udelay(10);
    //Line 416
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf3f6fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000f3fb);
    udelay(10);
    //Line 417
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020302ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fd00);
    udelay(10);
    //Line 418
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0301fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500fa01);
    udelay(10);
    //Line 419
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020502fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    udelay(10);
    //Line 420
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 421
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 422
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000201);
    udelay(10);
    //Line 423
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0402);
    udelay(10);
    //Line 424
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000300);
    udelay(10);
    //Line 425
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    udelay(10);
    //Line 426
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000301);
    udelay(10);
    //Line 427
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    udelay(10);
    //Line 428
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdff0a);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fa0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffeff);
    udelay(10);
    //Line 429
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fb0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010701);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffd01);
    udelay(10);
    //Line 430
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010700);
    udelay(10);
    //Line 431
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb050000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020103fe);
    udelay(10);
    //Line 432
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004fe0b);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000500);
    udelay(10);
    //Line 433
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fa00);
    udelay(10);
    //Line 434
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fd0202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050001ff);
    udelay(10);
    //Line 435
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050305ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000401);
    udelay(10);
    //Line 436
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe01);
    udelay(10);
    //Line 437
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fcf9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfdff04);
    udelay(10);
    //Line 438
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    udelay(10);
    //Line 439
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0200);
    udelay(10);
    //Line 440
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 441
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0df801fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf30003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3f30902);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0405ea);
    udelay(10);
    //Line 442
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4f80004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9020500);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03e60301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf60015f5);
    udelay(10);
    //Line 443
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf901f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff200f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfb0400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0aff);
    udelay(10);
    //Line 444
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7f90102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb010204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05020bfc);
    udelay(10);
    //Line 445
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0ff7ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd04fd);
    udelay(10);
    //Line 446
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000203fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0007);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefd03);
    udelay(10);
    //Line 447
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0107);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef8ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff02ff);
    udelay(10);
    //Line 448
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01070102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fc);
    udelay(10);
    //Line 449
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010109);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000101);
    udelay(10);
    //Line 450
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030005fe);
    udelay(10);
    //Line 451
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe050102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010107fe);
    udelay(10);
    //Line 452
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fe01);
    udelay(10);
    //Line 453
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0108fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fd01);
    udelay(10);
    //Line 454
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xef020106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f8020a);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d03ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x120104fe);
    udelay(10);
    //Line 455
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fdf3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fdf000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90402f5);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f7080c);
    udelay(10);
    //Line 456
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf601ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303f20a);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080105fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08000304);
    udelay(10);
    //Line 457
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0040203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f3d6fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050b06eb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302f401);
    udelay(10);
    //Line 458
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf801ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03f9fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffefd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe01);
    udelay(10);
    //Line 459
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8000104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfcfff6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff9fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fb01);
    udelay(10);
    //Line 460
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001f408);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000402);
    udelay(10);
    //Line 461
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fffffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000006fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfbfc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800fc01);
    udelay(10);
    //Line 462
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0605fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010100);
    udelay(10);
    //Line 463
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0304fa02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07050000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000003);
    udelay(10);
    //Line 464
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfef8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff0300);
    udelay(10);
    //Line 465
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd0302);
    udelay(10);
    //Line 466
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdf9fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff8fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffb03);
    udelay(10);
    //Line 467
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfb03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff08ff);
    udelay(10);
    //Line 468
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02050202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010a0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0400);
    udelay(10);
    //Line 469
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fd05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050007ff);
    udelay(10);
    //Line 470
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08040302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0603);
    udelay(10);
    //Line 471
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0302);
    udelay(10);
    //Line 472
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafcfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0003ff);
    udelay(10);
    //Line 473
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd05ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010000);
    udelay(10);
    //Line 474
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 475
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010102);
    udelay(10);
    //Line 476
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000501);
    udelay(10);
    //Line 477
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    udelay(10);
    //Line 478
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    udelay(10);
    //Line 479
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefa02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0003fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    udelay(10);
    //Line 480
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020008);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000401);
    udelay(10);
    //Line 481
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07010006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefa08);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6ff01fe);
    udelay(10);
    //Line 482
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0404);
    udelay(10);
    //Line 483
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02fff8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040402ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000c04);
    udelay(10);
    //Line 484
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffa00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9ff0304);
    udelay(10);
    //Line 485
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    udelay(10);
    //Line 486
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcfc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fcfe);
    udelay(10);
    //Line 487
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040002ff);
    udelay(10);
    //Line 488
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fd00);
    udelay(10);
    //Line 489
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fa00);
    udelay(10);
    //Line 490
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe01);
    udelay(10);
    //Line 491
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ffff);
    udelay(10);
    //Line 492
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fcff);
    udelay(10);
    //Line 493
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010300ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601050d);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020b0304);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030500);
    udelay(10);
    //Line 494
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9000009);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0707fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08f60003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000900);
    udelay(10);
    //Line 495
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf9fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f7fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fff900);
    udelay(10);
    //Line 496
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0008);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff05fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeefff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fefe);
    udelay(10);
    //Line 497
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fef9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020305fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd040502);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fe02);
    udelay(10);
    //Line 498
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70102fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020802fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fafd);
    udelay(10);
    //Line 499
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcfef2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000301f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00f802);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffcfe);
    udelay(10);
    //Line 500
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf901fff7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020403);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    udelay(10);
    //Line 501
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fcfc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0100);
    udelay(10);
    //Line 502
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f90200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffff901);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060003ff);
    udelay(10);
    //Line 503
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fafd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fafefd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fefc);
    udelay(10);
    //Line 504
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020003ff);
    udelay(10);
    //Line 505
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000105);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000300);
    udelay(10);
    //Line 506
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf00100db);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030802fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf006f3fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeb00fe08);
    udelay(10);
    //Line 507
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80101f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f4fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xedfff806);
    udelay(10);
    //Line 508
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fcfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0fefb05);
    udelay(10);
    //Line 509
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010403f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb06f802);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff02fd);
    udelay(10);
    //Line 510
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400f4fa);
    udelay(10);
    //Line 511
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000501);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00f8fe);
    udelay(10);
    //Line 512
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0401);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0201);
    udelay(10);
    //Line 513
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    udelay(10);
    //Line 514
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300f901);
    udelay(10);
    //Line 515
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fefd);
    udelay(10);
    //Line 516
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000200);
    udelay(10);
    //Line 517
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fcfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000302ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000101);
    udelay(10);
    //Line 518
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fa01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7ff0202);
    udelay(10);
    //Line 519
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500000b);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b06f800);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ee01);
    udelay(10);
    //Line 520
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe06ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0207f701);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a00f4fe);
    udelay(10);
    //Line 521
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020207);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f90002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fdfe);
    udelay(10);
    //Line 522
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd030d);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ec0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800f7fd);
    udelay(10);
    //Line 523
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101fd);
    udelay(10);
    //Line 524
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    udelay(10);
    //Line 525
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefefc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0102);
    udelay(10);
    //Line 526
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f70100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    udelay(10);
    //Line 527
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000400);
    udelay(10);
    //Line 528
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000303fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    udelay(10);
    //Line 529
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000101);
    udelay(10);
    //Line 530
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000500);
    udelay(10);
    //Line 531
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    udelay(10);
    //Line 532
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf50000ef);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404000c);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6ed0701);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601e50d);
    udelay(10);
    //Line 533
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xecfd00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f201ef);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fb0a02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf001060c);
    udelay(10);
    //Line 534
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f801f1);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fd0308);
    udelay(10);
    //Line 535
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03ffef);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff040200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffccfc);
    udelay(10);
    //Line 536
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003ff09);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020202fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fff102);
    udelay(10);
    //Line 537
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f4fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0402ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0503);
    udelay(10);
    //Line 538
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfc0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000102);
    udelay(10);
    //Line 539
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500faff);
    udelay(10);
    //Line 540
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000f903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    udelay(10);
    //Line 541
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf801ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fc00);
    udelay(10);
    //Line 542
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fe01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    udelay(10);
    //Line 543
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010103);
    udelay(10);
    //Line 544
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe04);
    udelay(10);
    //Line 545
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa03fff1);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403fff6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xef07f400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf402fa08);
    udelay(10);
    //Line 546
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0b03fddd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0500f5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf8ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xedf8f8fc);
    udelay(10);
    //Line 547
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfdfdf4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fdf202);
    udelay(10);
    //Line 548
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffef8f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf50d0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000206);
    udelay(10);
    //Line 549
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0400ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffa00);
    udelay(10);
    //Line 550
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fa00);
    udelay(10);
    //Line 551
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010202fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0502);
    udelay(10);
    //Line 552
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff05ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 553
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffd00);
    udelay(10);
    //Line 554
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fefe);
    udelay(10);
    //Line 555
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000102);
    udelay(10);
    //Line 556
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000100);
    udelay(10);
    //Line 557
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    udelay(10);
    //Line 558
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfffd03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff03f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffaf501);
    udelay(10);
    //Line 559
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbfc03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fff2ff);
    udelay(10);
    //Line 560
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefb02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04050001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fffe);
    udelay(10);
    //Line 561
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfa00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500fbff);
    udelay(10);
    //Line 562
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060102fe);
    udelay(10);
    //Line 563
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000801);
    udelay(10);
    //Line 564
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    udelay(10);
    //Line 565
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000200);
    udelay(10);
    //Line 566
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000400);
    udelay(10);
    //Line 567
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000303);
    udelay(10);
    //Line 568
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    udelay(10);
    //Line 569
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000200);
    udelay(10);
    //Line 570
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000100);
    udelay(10);
    //Line 571
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060200fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc050005);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00f0ff);
    udelay(10);
    //Line 572
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03040001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030410);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080c02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100f400);
    udelay(10);
    //Line 573
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffcff);
    udelay(10);
    //Line 574
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fff4ff);
    udelay(10);
    //Line 575
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ffff02);
    udelay(10);
    //Line 576
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc04ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000400);
    udelay(10);
    //Line 577
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010005);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000500);
    udelay(10);
    //Line 578
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020303);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000302);
    udelay(10);
    //Line 579
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000306);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010003fe);
    udelay(10);
    //Line 580
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000201);
    udelay(10);
    //Line 581
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0102);
    udelay(10);
    //Line 582
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    udelay(10);
    //Line 583
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001ff);
    udelay(10);
    //Line 584
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303ff0b);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020bff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8010902);
    udelay(10);
    //Line 585
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd04fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f4fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffa01);
    udelay(10);
    //Line 586
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020109);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc01);
    udelay(10);
    //Line 587
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf30c0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0703);
    udelay(10);
    //Line 588
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x070000f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf704fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fff9fd);
    udelay(10);
    //Line 589
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a01fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd070700);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fef200);
    udelay(10);
    //Line 590
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0001fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfefb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf80200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff0402);
    udelay(10);
    //Line 591
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f9fdf7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0302);
    udelay(10);
    //Line 592
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fafaf9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fefb01);
    udelay(10);
    //Line 593
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0105);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f1f5f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefa0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fb00);
    udelay(10);
    //Line 594
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fefa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf501fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    udelay(10);
    //Line 595
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0001);
    udelay(10);
    //Line 596
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffd01);
    udelay(10);
    //Line 597
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600f9fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe040402);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f801fd);
    udelay(10);
    //Line 598
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfef6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f503fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a000401);
    udelay(10);
    //Line 599
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0305fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09000800);
    udelay(10);
    //Line 600
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fe0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f904fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffeff);
    udelay(10);
    //Line 601
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ffff);
    udelay(10);
    //Line 602
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0000f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0304fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0602);
    udelay(10);
    //Line 603
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0402fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000102);
    udelay(10);
    //Line 604
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe01);
    udelay(10);
    //Line 605
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000300f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030103fe);
    udelay(10);
    //Line 606
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefa00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040001ff);
    udelay(10);
    //Line 607
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100feff);
    udelay(10);
    //Line 608
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000402);
    udelay(10);
    //Line 609
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fefe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    udelay(10);
    //Line 610
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffe09);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x12fc00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefa01);
    udelay(10);
    //Line 611
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04030502);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060e0500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefa02);
    udelay(10);
    //Line 612
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fe0008);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000008);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x070004fd);
    udelay(10);
    //Line 613
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06feff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000afb02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000101);
    udelay(10);
    //Line 614
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fef602);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff04fe);
    udelay(10);
    //Line 615
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0001f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00050201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0904);
    udelay(10);
    //Line 616
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0000);
    udelay(10);
    //Line 617
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000102);
    udelay(10);
    //Line 618
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030003ff);
    udelay(10);
    //Line 619
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030504);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000201);
    udelay(10);
    //Line 620
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fa02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fdff);
    udelay(10);
    //Line 621
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000301);
    udelay(10);
    //Line 622
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 623
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf00000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0406fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020107);
    udelay(10);
    //Line 624
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05eb0104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfefd02);
    udelay(10);
    //Line 625
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050100f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f7fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fde600);
    udelay(10);
    //Line 626
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa04fbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0800fef7);
    udelay(10);
    //Line 627
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfb0d);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0104ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fa00);
    udelay(10);
    //Line 628
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfc00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f40600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fe0500);
    udelay(10);
    //Line 629
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f8fff7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f00101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000100);
    udelay(10);
    //Line 630
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fd00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf905ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fffd00);
    udelay(10);
    //Line 631
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff6fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f30301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00faff);
    udelay(10);
    //Line 632
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fc05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fafd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101fe);
    udelay(10);
    //Line 633
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0006fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000102);
    udelay(10);
    //Line 634
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f9fbfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9ffff00);
    udelay(10);
    //Line 635
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffb02);
    udelay(10);
    //Line 636
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa02ffed);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90700fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe609f100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xef000a06);
    udelay(10);
    //Line 637
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x070100f2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90600fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800f101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600eafa);
    udelay(10);
    //Line 638
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100f0);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0500fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf803f3ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200f1ff);
    udelay(10);
    //Line 639
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa04fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8020303);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000c06);
    udelay(10);
    //Line 640
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefef8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff07ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000303);
    udelay(10);
    //Line 641
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04090100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fff101);
    udelay(10);
    //Line 642
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff01);
    udelay(10);
    //Line 643
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9ff0401);
    udelay(10);
    //Line 644
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff00);
    udelay(10);
    //Line 645
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe01);
    udelay(10);
    //Line 646
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000200);
    udelay(10);
    //Line 647
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffffff);
    udelay(10);
    //Line 648
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd01);
    udelay(10);
    //Line 649
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fdf4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff07fcfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf700fd08);
    udelay(10);
    //Line 650
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3020000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fcf0);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f9f8f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbffe808);
    udelay(10);
    //Line 651
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0102f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00ee05);
    udelay(10);
    //Line 652
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa06fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000304);
    udelay(10);
    //Line 653
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe06ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000fe);
    udelay(10);
    //Line 654
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020500ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff901);
    udelay(10);
    //Line 655
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ffff);
    udelay(10);
    //Line 656
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fb0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102ff);
    udelay(10);
    //Line 657
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06f80300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    udelay(10);
    //Line 658
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    udelay(10);
    //Line 659
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fffe);
    udelay(10);
    //Line 660
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000100);
    udelay(10);
    //Line 661
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020101ff);
    udelay(10);
    //Line 662
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfdff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fef702);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcedfe);
    udelay(10);
    //Line 663
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fd0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0604ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700f8fe);
    udelay(10);
    //Line 664
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff02ff);
    udelay(10);
    //Line 665
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fb02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0202);
    udelay(10);
    //Line 666
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00050400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff03);
    udelay(10);
    //Line 667
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000402ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000500);
    udelay(10);
    //Line 668
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010402);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000401);
    udelay(10);
    //Line 669
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000005ff);
    udelay(10);
    //Line 670
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    udelay(10);
    //Line 671
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000500);
    udelay(10);
    //Line 672
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000200);
    udelay(10);
    //Line 673
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    udelay(10);
    //Line 674
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000200);
    udelay(10);
    //Line 675
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f50401);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010005fc);
    udelay(10);
    //Line 676
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01ff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0802);
    udelay(10);
    //Line 677
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000404);
    udelay(10);
    //Line 678
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fd01);
    udelay(10);
    //Line 679
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfdfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010306ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd07fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a010102);
    udelay(10);
    //Line 680
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000108);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfe07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f901fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fb00);
    udelay(10);
    //Line 681
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    udelay(10);
    //Line 682
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbfffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000301);
    udelay(10);
    //Line 683
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefeff05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdfe);
    udelay(10);
    //Line 684
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffefb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000500fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    udelay(10);
    //Line 685
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03feffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000101);
    udelay(10);
    //Line 686
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200feff);
    udelay(10);
    //Line 687
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fd00);
    udelay(10);
    //Line 688
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffff9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9020202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010c02);
    udelay(10);
    //Line 689
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfaf8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf60400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000800);
    udelay(10);
    //Line 690
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0402);
    udelay(10);
    //Line 691
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfb03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0402);
    udelay(10);
    //Line 692
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0302);
    udelay(10);
    //Line 693
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fd01);
    udelay(10);
    //Line 694
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000102);
    udelay(10);
    //Line 695
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0400fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    udelay(10);
    //Line 696
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fe00);
    udelay(10);
    //Line 697
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fffa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd00);
    udelay(10);
    //Line 698
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    udelay(10);
    //Line 699
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff01);
    udelay(10);
    //Line 700
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    udelay(10);
    //Line 701
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fb02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000dff);
    udelay(10);
    //Line 702
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfcff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff06ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000c00);
    udelay(10);
    //Line 703
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000d01);
    udelay(10);
    //Line 704
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030005fe);
    udelay(10);
    //Line 705
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020103ff);
    udelay(10);
    //Line 706
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 707
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000202);
    udelay(10);
    //Line 708
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fff901);
    udelay(10);
    //Line 709
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fd00);
    udelay(10);
    //Line 710
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000201);
    udelay(10);
    //Line 711
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffd01);
    udelay(10);
    //Line 712
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    udelay(10);
    //Line 713
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    udelay(10);
    //Line 714
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500fefd);
    udelay(10);
    //Line 715
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04fc06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0007fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050101fd);
    udelay(10);
    //Line 716
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcfd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0300);
    udelay(10);
    //Line 717
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0005);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06020103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0601);
    udelay(10);
    //Line 718
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f90400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0302);
    udelay(10);
    //Line 719
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fc0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000601);
    udelay(10);
    //Line 720
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0301fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010301);
    udelay(10);
    //Line 721
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    udelay(10);
    //Line 722
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030402);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 723
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffd01);
    udelay(10);
    //Line 724
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0401);
    udelay(10);
    //Line 725
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200feff);
    udelay(10);
    //Line 726
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 727
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f707fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcfcfc);
    udelay(10);
    //Line 728
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020006);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    udelay(10);
    //Line 729
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001ff);
    udelay(10);
    //Line 730
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010005);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f8ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0001fe);
    udelay(10);
    //Line 731
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000401);
    udelay(10);
    //Line 732
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0204fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fc00);
    udelay(10);
    //Line 733
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    udelay(10);
    //Line 734
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0104fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000100);
    udelay(10);
    //Line 735
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    udelay(10);
    //Line 736
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 737
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    udelay(10);
    //Line 738
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000300);
    udelay(10);
    //Line 739
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000201);
    udelay(10);
    //Line 740
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fd05ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000afb);
    udelay(10);
    //Line 741
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000f3);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90a03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000cfe);
    udelay(10);
    //Line 742
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000001);
    udelay(10);
    //Line 743
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff090400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000301);
    udelay(10);
    //Line 744
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010008);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00f5fe);
    udelay(10);
    //Line 745
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000903);
    udelay(10);
    //Line 746
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc040100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000301);
    udelay(10);
    //Line 747
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    udelay(10);
    //Line 748
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    udelay(10);
    //Line 749
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbffff02);
    udelay(10);
    //Line 750
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    udelay(10);
    //Line 751
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 752
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fc00);
    udelay(10);
    //Line 753
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fc0105);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0802ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffef8);
    udelay(10);
    //Line 754
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff080200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff05fb);
    udelay(10);
    //Line 755
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00070405);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0c02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    udelay(10);
    //Line 756
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020300fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe050106);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00f9fe);
    udelay(10);
    //Line 757
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefefb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f802ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffe);
    udelay(10);
    //Line 758
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfffb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0105ff);
    udelay(10);
    //Line 759
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002fd);
    udelay(10);
    //Line 760
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000600);
    udelay(10);
    //Line 761
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0001ff);
    udelay(10);
    //Line 762
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001ff);
    udelay(10);
    //Line 763
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    udelay(10);
    //Line 764
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000feff);
    udelay(10);
    //Line 765
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000200);
    udelay(10);
    //Line 766
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf200ffee);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060201f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf900f801);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01f105);
    udelay(10);
    //Line 767
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0100f2);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff80101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ef0202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2ff040c);
    udelay(10);
    //Line 768
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf60201f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf800fd06);
    udelay(10);
    //Line 769
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a0300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fa00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ffe4fb);
    udelay(10);
    //Line 770
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe05fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf700f700);
    udelay(10);
    //Line 771
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb0205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9f70002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0ff0502);
    udelay(10);
    //Line 772
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc01);
    udelay(10);
    //Line 773
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300f900);
    udelay(10);
    //Line 774
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd03fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff02);
    udelay(10);
    //Line 775
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef80001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00feff);
    udelay(10);
    //Line 776
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafefd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    udelay(10);
    //Line 777
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000103);
    udelay(10);
    //Line 778
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000300ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff03);
    udelay(10);
    //Line 779
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x15e0e111);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x181c0ee8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x20180ce8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x141ae418);
    udelay(10);
    //Line 780
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x201d18de);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x181e2716);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe4dfe719);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x16e81317);
    udelay(10);
    //Line 781
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xed2220ef);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe3e5f319);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xdee8f215);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xefe61ce6);
    udelay(10);
    //Line 782
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xdfe1ea1e);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeae3ddeb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1b2217e4);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe61befe9);
    udelay(10);
    //Line 783
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf104f800);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff8ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ef03ff);
    udelay(10);
    //Line 784
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafe0ffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030604fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff07e7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000f8fa);
    udelay(10);
    //Line 785
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0602f906);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0308ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fb0503);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02070703);
    udelay(10);
    //Line 786
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103f9fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70312fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030505);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef10208);
    udelay(10);
    //Line 787
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080210fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xed06f406);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5f4fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01070001);
    udelay(10);
    //Line 788
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fb0504);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fbfdf5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00eaf3);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0503f4);
    udelay(10);
    //Line 789
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fb0000);
    udelay(10);
    
    rtd_outl(SMART_VAD_svad_coef_sram_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);

    //mean, std settings
    rtd_outl(SMART_VAD_svad_dmic_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f0f4ff);
    rtd_outl(SMART_VAD_svad_dmic_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5f4f9ee);
    rtd_outl(SMART_VAD_svad_dmic_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f5faf4);
    rtd_outl(SMART_VAD_svad_dmic_6_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000018fa);
    rtd_outl(SMART_VAD_svad_dmic_7_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x13141317);
    rtd_outl(SMART_VAD_svad_dmic_8_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x13121313);
    rtd_outl(SMART_VAD_svad_dmic_9_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x16151314);

    //TH settings
    //scale[25:22], dnn_th[15:8], kw_weight[4:0]
    rtd_maskl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0x03c00000, (scale<<22));
    rtd_maskl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0x0000ff00, (dnn_th<<8));
    rtd_maskl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0x0000001f, kw_weight);
    
    SVAD_INFO("\033[1;32;33m" "TH settings: 0x%08x" "\033[m", rtd_inl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT));
#else
    ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_SRST_reg, &reg_value_sys_srst);
    ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_CLKEN_reg, &reg_value_sys_clken);

    if(((reg_value_sys_srst & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((reg_value_sys_clken & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
    {
        SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
        rtk_svad_crt_clk_onoff(CLK_ON);
    }

    //Mfcc parameters
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_1_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_1_reg, 0x00000001);
    
    //Line 1
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0cf210fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x11f5f5f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x1111f6fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8f40b06);
    udelay(10);
    //Line 2
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010ffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x09fcf9f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0905fdfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff50205);
    udelay(10);
    //Line 3
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050209fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb01fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040002fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f800fd);
    udelay(10);
    //Line 4
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff06fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f9fcfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fa0201);
    udelay(10);
    //Line 5
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb06f802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010108);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fbfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fd00ff);
    udelay(10);
    //Line 6
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04faf303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000209);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfcfc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0101);
    udelay(10);
    //Line 7
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301f503);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01fd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010101);
    udelay(10);
    //Line 8
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf5fffc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0005fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfafb07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0406fe03);
    udelay(10);
    //Line 9
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08fe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf90405fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfaf7fc08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0604fefe);
    udelay(10);
    //Line 10
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe030303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf80706ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf2f1fb07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06000104);
    udelay(10);
    //Line 11
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a09f80d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010a0909);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd0008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0605fefe);
    udelay(10);
    //Line 12
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0d02fa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fc0001);
    udelay(10);
    //Line 13
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0601fcfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfbffff);
    udelay(10);
    //Line 14
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04010306);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe060400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fefd);
    udelay(10);
    //Line 15
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08040301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020202fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00030000);
    udelay(10);
    //Line 16
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0b030101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf901ff00);
    udelay(10);
    //Line 17
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02070203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9040204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfd00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00fbfc);
    udelay(10);
    //Line 18
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010bfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0003fff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf800fdfc);
    udelay(10);
    //Line 19
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0204fa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9090000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fdfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02fafd);
    udelay(10);
    //Line 20
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fffaf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfff901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0202f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf701fcfd);
    udelay(10);
    //Line 21
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x1103fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0a010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05f4f8ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fa090c);
    udelay(10);
    //Line 22
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0bfd0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fbef01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04ee0509);
    udelay(10);
    //Line 23
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fbe600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02f40203);
    udelay(10);
    //Line 24
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fcfffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x09fc00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffdf2fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f70003);
    udelay(10);
    //Line 25
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0004f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0601fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f300fe);
    udelay(10);
    //Line 26
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefa01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fe00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0001);
    udelay(10);
    //Line 27
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0004fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301fffa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc04fffb);
    udelay(10);
    //Line 28
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfe01f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffcfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa0900fc);
    udelay(10);
    //Line 29
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf905fdfb);
    udelay(10);
    //Line 30
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf50605fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0004ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x080101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb01fbf9);
    udelay(10);
    //Line 31
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0df313);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa090e11);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf900030e);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040ffdfd);
    udelay(10);
    //Line 32
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04000603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04f7fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00040101);
    udelay(10);
    //Line 33
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fc0f06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06030400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fdf805);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010601ff);
    udelay(10);
    //Line 34
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfdfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe030603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfaf004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02020102);
    udelay(10);
    //Line 35
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf700fb04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd000402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fbef02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe00ff);
    udelay(10);
    //Line 36
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbf401fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefafdf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefef6fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03f90001);
    udelay(10);
    //Line 37
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffe80201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff4fdf0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffa04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201ff01);
    udelay(10);
    //Line 38
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02f00002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcf900fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfbfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ffff00);
    udelay(10);
    //Line 39
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdf5ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbf90000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0000);
    udelay(10);
    //Line 40
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02f7fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfc01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0205fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01040103);
    udelay(10);
    //Line 41
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000702);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfbfd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfffe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fa0100);
    udelay(10);
    //Line 42
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f8f803);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fefffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0000);
    udelay(10);
    //Line 43
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd06f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f6f8fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000000);
    udelay(10);
    //Line 44
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff605fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff7f8fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffb0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010000);
    udelay(10);
    //Line 45
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fa0500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdf9fa05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfc0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0100ff);
    udelay(10);
    //Line 46
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfefe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafc0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff04ffff);
    udelay(10);
    //Line 47
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fe0701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefa0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00030101);
    udelay(10);
    //Line 48
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000f02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfb04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f90301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010102);
    udelay(10);
    //Line 49
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0c08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff030301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9f10207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010203);
    udelay(10);
    //Line 50
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05ff0c13);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe090e07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf3e6fd20);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07030304);
    udelay(10);
    //Line 51
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0707020a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x040b0907);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301fd0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fa0100);
    udelay(10);
    //Line 52
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0cfd0600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0601fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fffbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f20304);
    udelay(10);
    //Line 53
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x080305ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fefffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ee0000);
    udelay(10);
    //Line 54
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x060605fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001fff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff50101);
    udelay(10);
    //Line 55
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050404fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f800ff);
    udelay(10);
    //Line 56
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0506fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0402fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0000fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffbfe03);
    udelay(10);
    //Line 57
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a01fafa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fdfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfe03fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffc0203);
    udelay(10);
    //Line 58
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0502fbfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfd03fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff90100);
    udelay(10);
    //Line 59
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a02feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0004ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff8ff00);
    udelay(10);
    //Line 60
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0f0100fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0405fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff80000);
    udelay(10);
    //Line 61
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf0060208);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc050308);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101f9f7);
    udelay(10);
    //Line 62
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf1030202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffdfd);
    udelay(10);
    //Line 63
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf5020202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fffefd);
    udelay(10);
    //Line 64
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf8fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fc0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fe01);
    udelay(10);
    //Line 65
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf3ff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fdfd);
    udelay(10);
    //Line 66
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf7fe0803);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0001feff);
    udelay(10);
    //Line 67
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf0ff0401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000fe);
    udelay(10);
    //Line 68
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xeefc0406);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fffe);
    udelay(10);
    //Line 69
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf0ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe01fffb);
    udelay(10);
    //Line 70
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf2fc0806);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x070307fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x09fcfd12);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff00fb);
    udelay(10);
    //Line 71
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd02cbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf70600fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xed0104fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0205fdfc);
    udelay(10);
    //Line 72
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcffcfff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0001f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xeefd02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0301ffff);
    udelay(10);
    //Line 73
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefce2fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb0100f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf4fc02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff03ffff);
    udelay(10);
    //Line 74
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffbe0fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0004fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fc01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010400fe);
    udelay(10);
    //Line 75
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00e301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030104fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03faff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ffff01);
    udelay(10);
    //Line 76
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fef402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000302fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fcfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fc0200);
    udelay(10);
    //Line 77
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff06fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fb0003);
    udelay(10);
    //Line 78
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000005fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fd0000);
    udelay(10);
    //Line 79
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020409ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02040101);
    udelay(10);
    //Line 80
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02060704);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01050501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03050204);
    udelay(10);
    //Line 81
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0ef10601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x07f402f1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6040302);
    udelay(10);
    //Line 82
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fdfc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefe01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd03fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf902fdfe);
    udelay(10);
    //Line 83
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01f6fffa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffcfbf3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc01fbfc);
    udelay(10);
    //Line 84
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdf7fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf900fff6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd030304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002f9fc);
    udelay(10);
    //Line 85
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fb0402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfdfd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fbfe);
    udelay(10);
    //Line 86
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06000e06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04010205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0403fe00);
    udelay(10);
    //Line 87
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0d040b03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06000204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05010105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00070203);
    udelay(10);
    //Line 88
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07010902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x07020105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04ff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff030202);
    udelay(10);
    //Line 89
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0504ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01060002);
    udelay(10);
    //Line 90
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f903fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02f8fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000a09f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000a0101);
    udelay(10);
    //Line 91
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xe3fdfdf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafff8f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0506fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf603f4f0);
    udelay(10);
    //Line 92
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xe0fcfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa0003fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff030404);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd05f5f6);
    udelay(10);
    //Line 93
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xe2fd0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe030202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0003f4f5);
    udelay(10);
    //Line 94
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xea000207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb020401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0504f7f9);
    udelay(10);
    //Line 95
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xef030902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0202fefe);
    udelay(10);
    //Line 96
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf2010804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfb0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04010100);
    udelay(10);
    //Line 97
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf0010203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04010103);
    udelay(10);
    //Line 98
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf2010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfbfe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000504);
    udelay(10);
    //Line 99
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf501ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef6fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000502);
    udelay(10);
    //Line 100
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa00ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020506fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfff5fa12);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fe0704);
    udelay(10);
    //Line 101
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe05f306);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8050006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf3ff0d05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe15ffff);
    udelay(10);
    //Line 102
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffff3fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfffdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa0203ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020000);
    udelay(10);
    //Line 103
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefefa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002ffff);
    udelay(10);
    //Line 104
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefaf701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd030000);
    udelay(10);
    //Line 105
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfd0302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefd0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020202);
    udelay(10);
    //Line 106
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000001);
    udelay(10);
    //Line 107
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03010101);
    udelay(10);
    //Line 108
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200fbfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfe01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000100);
    udelay(10);
    //Line 109
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefeffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000201);
    udelay(10);
    //Line 110
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fcfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfcff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08ff0804);
    udelay(10);
    //Line 111
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf60af70c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf40a0804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb00040b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01f8f8);
    udelay(10);
    //Line 112
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa030103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff03ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefbfbfa);
    udelay(10);
    //Line 113
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff030303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa050405);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fcfcfd);
    udelay(10);
    //Line 114
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfe0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010201fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffafefe);
    udelay(10);
    //Line 115
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0202ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fdfe00);
    udelay(10);
    //Line 116
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0500ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0202);
    udelay(10);
    //Line 117
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0400fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fd0301);
    udelay(10);
    //Line 118
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fffbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020401fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfafefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0201);
    udelay(10);
    //Line 119
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fff3fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfaf8fcfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000301);
    udelay(10);
    //Line 120
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0400fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdf7fa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fe0405);
    udelay(10);
    //Line 121
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc08e812);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf80c0b0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf1fc040c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050e0101);
    udelay(10);
    //Line 122
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fceefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000201);
    udelay(10);
    //Line 123
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfef9fdf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00edeff4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff050200);
    udelay(10);
    //Line 124
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f80101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01e9f1ed);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff020a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff060202);
    udelay(10);
    //Line 125
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04f8000c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe03fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04000204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020201);
    udelay(10);
    //Line 126
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfafefc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000302fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0000);
    udelay(10);
    //Line 127
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa00e802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0303fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xe9fb0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201ff00);
    udelay(10);
    //Line 128
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfde500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf3f90000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020101);
    udelay(10);
    //Line 129
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfafee700);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fcffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 130
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9fdf2ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefbfffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe0102);
    udelay(10);
    //Line 131
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf70aec0c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf408060c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf0f20106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f8fdfe);
    udelay(10);
    //Line 132
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0008fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb0100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcf900fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f80101);
    udelay(10);
    //Line 133
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fb0202);
    udelay(10);
    //Line 134
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 135
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0204ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff030001);
    udelay(10);
    //Line 136
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fc0305);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020301);
    udelay(10);
    //Line 137
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01feffff);
    udelay(10);
    //Line 138
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000103);
    udelay(10);
    //Line 139
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefd0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201ff01);
    udelay(10);
    //Line 140
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefd0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020405);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01030001);
    udelay(10);
    //Line 141
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff001ec);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x08f0f7ee);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0502fbf5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f20604);
    udelay(10);
    //Line 142
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01f8fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fbfff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fa0503);
    udelay(10);
    //Line 143
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfa0bfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fcfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefc0302);
    udelay(10);
    //Line 144
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd030204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000108);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffd0201);
    udelay(10);
    //Line 145
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc041103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04050600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffdff00);
    udelay(10);
    //Line 146
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd02fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0300ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000000);
    udelay(10);
    //Line 147
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa030b01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01050601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0200fe);
    udelay(10);
    //Line 148
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9040202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfdfefd);
    udelay(10);
    //Line 149
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0607fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0203fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00fdfc);
    udelay(10);
    //Line 150
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf90502f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefefa03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0504fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9fffcf9);
    udelay(10);
    //Line 151
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07f8f7ea);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05f5f7e6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0900ffe9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010604);
    udelay(10);
    //Line 152
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f9fae6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fbfee3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ffffec);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010102);
    udelay(10);
    //Line 153
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fafae2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f8fce6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101ffe9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000002);
    udelay(10);
    //Line 154
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfcd9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff8f7e0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0000df);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0002ff);
    udelay(10);
    //Line 155
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04f7fed4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03f2f0d9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff00dd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010001);
    udelay(10);
    //Line 156
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fbfde2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdf7ea);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0200f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0101ff);
    udelay(10);
    //Line 157
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0400fef8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fef4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ffff);
    udelay(10);
    //Line 158
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x060302fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefefefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fefe);
    udelay(10);
    //Line 159
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0504fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fffe);
    udelay(10);
    //Line 160
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040902ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff04fe07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02100500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe04fefe);
    udelay(10);
    //Line 161
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00d60fef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04e5f4e2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0804fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f80101);
    udelay(10);
    //Line 162
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ef0507);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff020e);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0000);
    udelay(10);
    //Line 163
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdf70b03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030505);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000101);
    udelay(10);
    //Line 164
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfafc0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01040100);
    udelay(10);
    //Line 165
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01050000);
    udelay(10);
    //Line 166
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfa0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020203fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff04ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020000);
    udelay(10);
    //Line 167
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfcfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000104fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02030000);
    udelay(10);
    //Line 168
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfbfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0302ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0000);
    udelay(10);
    //Line 169
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fdfe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0300ff);
    udelay(10);
    //Line 170
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfcfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020602fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fdfb05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0000ff);
    udelay(10);
    //Line 171
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb030804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02f4f7fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf902f300);
    udelay(10);
    //Line 172
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf601ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fd00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0900);
    udelay(10);
    //Line 173
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa01f905);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fd02ff);
    udelay(10);
    //Line 174
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fd0102);
    udelay(10);
    //Line 175
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010006fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010102fe);
    udelay(10);
    //Line 176
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff05fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020201);
    udelay(10);
    //Line 177
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf90119fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06fefaee);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fd0afe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8feff00);
    udelay(10);
    //Line 178
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0101fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0005fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd04fefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0500);
    udelay(10);
    //Line 179
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe040504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0000);
    udelay(10);
    //Line 180
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe05fe);
    udelay(10);
    //Line 181
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000401ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff03ff);
    udelay(10);
    //Line 182
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0106ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc00fdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010505);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fffdf8);
    udelay(10);
    //Line 183
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf604f708);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfcf605);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefcfcff);
    udelay(10);
    //Line 184
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f9f5f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc030206);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303f9f0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fd05fe);
    udelay(10);
    //Line 185
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfdfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000fe);
    udelay(10);
    //Line 186
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fc01f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd03ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00fffe);
    udelay(10);
    //Line 187
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0001fd);
    udelay(10);
    //Line 188
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd030202);
    udelay(10);
    //Line 189
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf00dfe07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc00fafa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01f00c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fcee01);
    udelay(10);
    //Line 190
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fb01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb03fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fafa00);
    udelay(10);
    //Line 191
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0002fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fcfa01);
    udelay(10);
    //Line 192
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fc01);
    udelay(10);
    //Line 193
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc020201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010300fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc00fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0401fc01);
    udelay(10);
    //Line 194
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe04ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff060000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefdff0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600fdff);
    udelay(10);
    //Line 195
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07f9f4f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb080907);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0306fef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fefd00);
    udelay(10);
    //Line 196
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf803fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fcf901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fff6fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fa01);
    udelay(10);
    //Line 197
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fefefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000003ff);
    udelay(10);
    //Line 198
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefffffd);
    udelay(10);
    //Line 199
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff01ff);
    udelay(10);
    //Line 200
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000101);
    udelay(10);
    //Line 201
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fef9fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd020403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03020809);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0504fefb);
    udelay(10);
    //Line 202
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0301fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402f8fb);
    udelay(10);
    //Line 203
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf2040103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe02fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff03f800);
    udelay(10);
    //Line 204
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020000fc);
    udelay(10);
    //Line 205
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff04fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff07fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fbf1);
    udelay(10);
    //Line 206
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf60404fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfff9fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfff6050e);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0800f4eb);
    udelay(10);
    //Line 207
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0007fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fdfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fc01);
    udelay(10);
    //Line 208
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe03fe03);
    udelay(10);
    //Line 209
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0103ff);
    udelay(10);
    //Line 210
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fe02);
    udelay(10);
    //Line 211
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fe00);
    udelay(10);
    //Line 212
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff08feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010104);
    udelay(10);
    //Line 213
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb00fd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0401feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf50200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefbf700);
    udelay(10);
    //Line 214
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fb00fe);
    udelay(10);
    //Line 215
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fa00ff);
    udelay(10);
    //Line 216
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0202fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f9fefe);
    udelay(10);
    //Line 217
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000001fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fe00fc);
    udelay(10);
    //Line 218
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fff8ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0c0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff05fff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fd00);
    udelay(10);
    //Line 219
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdf016f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0201ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030002f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x09010501);
    udelay(10);
    //Line 220
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfef403ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000101);
    udelay(10);
    //Line 221
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff800ea);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc01f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fd01);
    udelay(10);
    //Line 222
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff02f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fbfffa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fe04);
    udelay(10);
    //Line 223
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefcfefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fc01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000401);
    udelay(10);
    //Line 224
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06ff0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0101);
    udelay(10);
    //Line 225
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07f7f801);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0203f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfafefe);
    udelay(10);
    //Line 226
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02f7fafa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000300fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0204fe);
    udelay(10);
    //Line 227
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fdff);
    udelay(10);
    //Line 228
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03020402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fd01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010100);
    udelay(10);
    //Line 229
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfefa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe01ff);
    udelay(10);
    //Line 230
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fdfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0501ff);
    udelay(10);
    //Line 231
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00faf4ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x08080203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fa07ff);
    udelay(10);
    //Line 232
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fbfafd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010101);
    udelay(10);
    //Line 233
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefffd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020000);
    udelay(10);
    //Line 234
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010101ff);
    udelay(10);
    //Line 235
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010100);
    udelay(10);
    //Line 236
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fb03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe06fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000002);
    udelay(10);
    //Line 237
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf502fb04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fa0d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fdf90b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa04f702);
    udelay(10);
    //Line 238
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01faf802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fe0301);
    udelay(10);
    //Line 239
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fdfdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefe01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000505ff);
    udelay(10);
    //Line 240
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fe0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefdfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000000ff);
    udelay(10);
    //Line 241
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbfc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff03fc00);
    udelay(10);
    //Line 242
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fd05f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd05fdfe);
    udelay(10);
    //Line 243
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fcf901);
    udelay(10);
    //Line 244
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0106fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfbff01);
    udelay(10);
    //Line 245
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefe0100);
    udelay(10);
    //Line 246
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fffffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000101);
    udelay(10);
    //Line 247
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa000000);
    udelay(10);
    //Line 248
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffb05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0702fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf901fc01);
    udelay(10);
    //Line 249
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf902fc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fcfdfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0bfd02);
    udelay(10);
    //Line 250
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe050205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcf7f9ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fbf8fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfbfe01);
    udelay(10);
    //Line 251
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc00fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000003fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01050201);
    udelay(10);
    //Line 252
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fb05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0002fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0103ff02);
    udelay(10);
    //Line 253
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0001fd01);
    udelay(10);
    //Line 254
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc04fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfdfbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff03fefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0604fdfe);
    udelay(10);
    //Line 255
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02faf600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04000507);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08fdfb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9ffffff);
    udelay(10);
    //Line 256
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00f9ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe040100);
    udelay(10);
    //Line 257
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f80203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff030303);
    udelay(10);
    //Line 258
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00f6fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd060102);
    udelay(10);
    //Line 259
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01f7fafd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0afa0403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0303);
    udelay(10);
    //Line 260
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff8fa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05080702);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04f805fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfb0707);
    udelay(10);
    //Line 261
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f208f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x090004ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0105fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ee00ff);
    udelay(10);
    //Line 262
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01f701f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fff6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02f401fe);
    udelay(10);
    //Line 263
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fafbf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0103fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000202fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefb0002);
    udelay(10);
    //Line 264
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301f9fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfeff01);
    udelay(10);
    //Line 265
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fefc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefefdff);
    udelay(10);
    //Line 266
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefefd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffef5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffd0101);
    udelay(10);
    //Line 267
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00040200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0501fe);
    udelay(10);
    //Line 268
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffeff);
    udelay(10);
    //Line 269
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdfe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201ff00);
    udelay(10);
    //Line 270
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff01);
    udelay(10);
    //Line 271
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020000);
    udelay(10);
    //Line 272
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ffff);
    udelay(10);
    //Line 273
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x011208f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbf50b08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0610feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8fced0b);
    udelay(10);
    //Line 274
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050101fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02030707);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0afc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfffe0b);
    udelay(10);
    //Line 275
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf90901fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03ff0a08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0105fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5f7f804);
    udelay(10);
    //Line 276
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0702fef5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000004ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb05000a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f9fd08);
    udelay(10);
    //Line 277
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0406fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfaff08ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe03ff0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfdff03);
    udelay(10);
    //Line 278
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf902fcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf5000506);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030506);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe04fcff);
    udelay(10);
    //Line 279
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0006fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfbfaff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff06fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000000);
    udelay(10);
    //Line 280
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000201);
    udelay(10);
    //Line 281
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010000);
    udelay(10);
    //Line 282
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 283
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 284
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000100);
    udelay(10);
    //Line 285
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf1fc0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0d02f9fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdf4effd);
    udelay(10);
    //Line 286
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc05fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fb03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000302);
    udelay(10);
    //Line 287
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb030002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fffa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfcfcff);
    udelay(10);
    //Line 288
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01feff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0004feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000101ff);
    udelay(10);
    //Line 289
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff05fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0300fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffff01);
    udelay(10);
    //Line 290
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd03fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff05fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010001);
    udelay(10);
    //Line 291
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fdff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fd03fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd030000);
    udelay(10);
    //Line 292
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefffe00);
    udelay(10);
    //Line 293
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff02ffff);
    udelay(10);
    //Line 294
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000000ff);
    udelay(10);
    //Line 295
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff00ff);
    udelay(10);
    //Line 296
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0203ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff01ff);
    udelay(10);
    //Line 297
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03030400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfe00fe);
    udelay(10);
    //Line 298
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd02fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fbfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020004fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000001);
    udelay(10);
    //Line 299
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000000);
    udelay(10);
    //Line 300
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000ff);
    udelay(10);
    //Line 301
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000000);
    udelay(10);
    //Line 302
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000000);
    udelay(10);
    //Line 303
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fffefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fbfe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfe0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0d030205);
    udelay(10);
    //Line 304
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0000ff);
    udelay(10);
    //Line 305
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02020100);
    udelay(10);
    //Line 306
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fd00);
    udelay(10);
    //Line 307
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00feff);
    udelay(10);
    //Line 308
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00ffff);
    udelay(10);
    //Line 309
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x060706f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xeaf9fd0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08ff06f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03f70301);
    udelay(10);
    //Line 310
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000306);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf1010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fb01fe);
    udelay(10);
    //Line 311
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03020402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf50402fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fdfefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcf70200);
    udelay(10);
    //Line 312
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05020603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafe00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fafffe);
    udelay(10);
    //Line 313
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0afd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0501fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fc04fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05f9f9ff);
    udelay(10);
    //Line 314
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0bf91b06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0b0601fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f702fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04f2fafb);
    udelay(10);
    //Line 315
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0101f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfc10);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0b04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000105);
    udelay(10);
    //Line 316
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfe00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0404fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08fdfc00);
    udelay(10);
    //Line 317
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040304fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0404);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100f806);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x09fb0403);
    udelay(10);
    //Line 318
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04030504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff01fe);
    udelay(10);
    //Line 319
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0306010a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020105fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fc01fd);
    udelay(10);
    //Line 320
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa02feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020cf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fffffd);
    udelay(10);
    //Line 321
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff040304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc050400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbf8fbfd);
    udelay(10);
    //Line 322
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb02fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0100);
    udelay(10);
    //Line 323
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0006fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc02fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010202fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffcfdfe);
    udelay(10);
    //Line 324
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0103feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc03fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000004fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe0100);
    udelay(10);
    //Line 325
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0202feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000200);
    udelay(10);
    //Line 326
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0204fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000301);
    udelay(10);
    //Line 327
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05050500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x1802eefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5ff07f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02fbf8);
    udelay(10);
    //Line 328
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000202fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0b01f904);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0007ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfbfff9);
    udelay(10);
    //Line 329
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030102fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0bfffa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe04f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfefdf4);
    udelay(10);
    //Line 330
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040402fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0a00fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe05f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffc01fb);
    udelay(10);
    //Line 331
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010402fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fcfdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfcfff4);
    udelay(10);
    //Line 332
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0303fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0b04f903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fa06f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfa02fb);
    udelay(10);
    //Line 333
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x060001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0301ff00);
    udelay(10);
    //Line 334
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00feffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff01);
    udelay(10);
    //Line 335
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020001ff);
    udelay(10);
    //Line 336
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0100);
    udelay(10);
    //Line 337
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010101);
    udelay(10);
    //Line 338
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04ff0100);
    udelay(10);
    //Line 339
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010801);
    udelay(10);
    //Line 340
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefe0000);
    udelay(10);
    //Line 341
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010301);
    udelay(10);
    //Line 342
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02feffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0200);
    udelay(10);
    //Line 343
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000200);
    udelay(10);
    //Line 344
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fdfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fc0100);
    udelay(10);
    //Line 345
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fc05ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf2fa050a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0bfeea06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07f80100);
    udelay(10);
    //Line 346
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfc0307);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010b0904);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fef001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040408fc);
    udelay(10);
    //Line 347
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf1010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc050505);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0704f4fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fd02fe);
    udelay(10);
    //Line 348
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf60501fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb05f602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0106fbfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0000);
    udelay(10);
    //Line 349
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fff301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010303fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402fd00);
    udelay(10);
    //Line 350
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfaff0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05fbf801);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc06f5fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0407fc00);
    udelay(10);
    //Line 351
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fbfffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8f30804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0404f507);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9060c0b);
    udelay(10);
    //Line 352
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffb02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fa04f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303f504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb09ff01);
    udelay(10);
    //Line 353
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfd04fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8f201fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02f902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6090405);
    udelay(10);
    //Line 354
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd04fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbf501fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0109faff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9120205);
    udelay(10);
    //Line 355
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01f70202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefc03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0700fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000cfd06);
    udelay(10);
    //Line 356
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcf509fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfafe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc18f5f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd16fb16);
    udelay(10);
    //Line 357
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x040101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe01ff02);
    udelay(10);
    //Line 358
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffefe00);
    udelay(10);
    //Line 359
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010101);
    udelay(10);
    //Line 360
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0000);
    udelay(10);
    //Line 361
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 362
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffffff);
    udelay(10);
    //Line 363
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fefbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060002fe);
    udelay(10);
    //Line 364
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0702ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0af600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06000c03);
    udelay(10);
    //Line 365
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0301fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc090501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07ff0402);
    udelay(10);
    //Line 366
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05ff0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fd01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fc0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500f800);
    udelay(10);
    //Line 367
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fe03);
    udelay(10);
    //Line 368
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfffff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf7fefa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000200);
    udelay(10);
    //Line 369
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd02ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000501);
    udelay(10);
    //Line 370
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe0009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f70300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010401);
    udelay(10);
    //Line 371
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 372
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0200fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000400);
    udelay(10);
    //Line 373
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0101);
    udelay(10);
    //Line 374
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffe00);
    udelay(10);
    //Line 375
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000201);
    udelay(10);
    //Line 376
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0bfcfe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fdf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0502fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff05ff);
    udelay(10);
    //Line 377
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffbff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf10bf601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08ff05fc);
    udelay(10);
    //Line 378
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfd06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f80500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7000602);
    udelay(10);
    //Line 379
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff00f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fcfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000801);
    udelay(10);
    //Line 380
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fc03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000400);
    udelay(10);
    //Line 381
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000201);
    udelay(10);
    //Line 382
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00fefe);
    udelay(10);
    //Line 383
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00ff01);
    udelay(10);
    //Line 384
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000000);
    udelay(10);
    //Line 385
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fe01);
    udelay(10);
    //Line 386
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000fe);
    udelay(10);
    //Line 387
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000000);
    udelay(10);
    //Line 388
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00fe00);
    udelay(10);
    //Line 389
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0cfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xe30ef301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010002);
    udelay(10);
    //Line 390
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf80200f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe080101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5fff500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fc00);
    udelay(10);
    //Line 391
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010101f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc050008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203f900);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fefbfd);
    udelay(10);
    //Line 392
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff01f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0001fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf703fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8ff0108);
    udelay(10);
    //Line 393
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfb00f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf00bf700);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ff0bff);
    udelay(10);
    //Line 394
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbf604);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0bfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffb01);
    udelay(10);
    //Line 395
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x080100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000406ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffff4ff);
    udelay(10);
    //Line 396
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03020103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05f90000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfafffe00);
    udelay(10);
    //Line 397
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fcfcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0703);
    udelay(10);
    //Line 398
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f5fcfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04f80200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf900fc01);
    udelay(10);
    //Line 399
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f5fbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fc0401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0200);
    udelay(10);
    //Line 400
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf802fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010604);
    udelay(10);
    //Line 401
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000503);
    udelay(10);
    //Line 402
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04ffff0f);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdf8fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6ff0301);
    udelay(10);
    //Line 403
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fd0d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fb03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9000801);
    udelay(10);
    //Line 404
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fd01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010005ff);
    udelay(10);
    //Line 405
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01faff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef801ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000a01);
    udelay(10);
    //Line 406
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fa0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc05ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010501);
    udelay(10);
    //Line 407
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb04feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000101);
    udelay(10);
    //Line 408
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00050100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fffe);
    udelay(10);
    //Line 409
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fe00);
    udelay(10);
    //Line 410
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05000301);
    udelay(10);
    //Line 411
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fc00);
    udelay(10);
    //Line 412
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fb00);
    udelay(10);
    //Line 413
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fe00);
    udelay(10);
    //Line 414
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00feff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000101);
    udelay(10);
    //Line 415
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0700fb04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010302fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafef603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefdfd00);
    udelay(10);
    //Line 416
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fe0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfaf3f6fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000f3fb);
    udelay(10);
    //Line 417
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020302ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101fd00);
    udelay(10);
    //Line 418
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0301fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500fa01);
    udelay(10);
    //Line 419
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020502fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020000ff);
    udelay(10);
    //Line 420
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 421
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 422
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000201);
    udelay(10);
    //Line 423
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0402);
    udelay(10);
    //Line 424
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000300);
    udelay(10);
    //Line 425
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0101);
    udelay(10);
    //Line 426
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000301);
    udelay(10);
    //Line 427
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff01);
    udelay(10);
    //Line 428
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fdff0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fa0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fffeff);
    udelay(10);
    //Line 429
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fb0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06010701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fffd01);
    udelay(10);
    //Line 430
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010700);
    udelay(10);
    //Line 431
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb050000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020103fe);
    udelay(10);
    //Line 432
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0004fe0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000500);
    udelay(10);
    //Line 433
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fdfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fa00);
    udelay(10);
    //Line 434
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x09fd0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050001ff);
    udelay(10);
    //Line 435
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050305ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000401);
    udelay(10);
    //Line 436
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffe01);
    udelay(10);
    //Line 437
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fcf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfdff04);
    udelay(10);
    //Line 438
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000000ff);
    udelay(10);
    //Line 439
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0200);
    udelay(10);
    //Line 440
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 441
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0df801fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdf30003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf3f30902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa0405ea);
    udelay(10);
    //Line 442
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf4f80004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9020500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03e60301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf60015f5);
    udelay(10);
    //Line 443
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbf901f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff200f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfb0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0aff);
    udelay(10);
    //Line 444
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf7f90102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05020bfc);
    udelay(10);
    //Line 445
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0ff7ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfd04fd);
    udelay(10);
    //Line 446
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000203fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fffffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fefd03);
    udelay(10);
    //Line 447
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef8ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfaff02ff);
    udelay(10);
    //Line 448
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01070102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0001fc);
    udelay(10);
    //Line 449
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010109);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000101);
    udelay(10);
    //Line 450
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030005fe);
    udelay(10);
    //Line 451
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe050102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010107fe);
    udelay(10);
    //Line 452
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb00fe01);
    udelay(10);
    //Line 453
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0108fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fd01);
    udelay(10);
    //Line 454
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xef020106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04f8020a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0d03ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x120104fe);
    udelay(10);
    //Line 455
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb01fdf3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9fdf000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf90402f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02f7080c);
    udelay(10);
    //Line 456
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf601ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0303f20a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x080105fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08000304);
    udelay(10);
    //Line 457
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf0040203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03f3d6fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050b06eb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0302f401);
    udelay(10);
    //Line 458
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf801ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe03f9fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd02fe01);
    udelay(10);
    //Line 459
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf8000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfcfff6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfff9fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fb01);
    udelay(10);
    //Line 460
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001f408);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0100fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000402);
    udelay(10);
    //Line 461
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07fffffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000006fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfbfc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf800fc01);
    udelay(10);
    //Line 462
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0605fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc010100);
    udelay(10);
    //Line 463
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0501ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0304fa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x07050000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000003);
    udelay(10);
    //Line 464
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfef8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffeffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8ff0300);
    udelay(10);
    //Line 465
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fffdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0004fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfd0302);
    udelay(10);
    //Line 466
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fffefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdf9fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfff8fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfffb03);
    udelay(10);
    //Line 467
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdfb03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0700fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff08ff);
    udelay(10);
    //Line 468
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02050202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010a0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ff0400);
    udelay(10);
    //Line 469
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fd05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050007ff);
    udelay(10);
    //Line 470
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03030104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08040302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0603);
    udelay(10);
    //Line 471
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0302);
    udelay(10);
    //Line 472
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafcfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0003ff);
    udelay(10);
    //Line 473
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd05ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03010000);
    udelay(10);
    //Line 474
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0004ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 475
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fe0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010102);
    udelay(10);
    //Line 476
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000501);
    udelay(10);
    //Line 477
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000102);
    udelay(10);
    //Line 478
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000000ff);
    udelay(10);
    //Line 479
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0003fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000ff);
    udelay(10);
    //Line 480
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc020008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000401);
    udelay(10);
    //Line 481
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07010006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfefa08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffc00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6ff01fe);
    udelay(10);
    //Line 482
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfefe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0404);
    udelay(10);
    //Line 483
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc02fff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x040402ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000c04);
    udelay(10);
    //Line 484
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffa00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9ff0304);
    udelay(10);
    //Line 485
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0002fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ffff);
    udelay(10);
    //Line 486
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffcfc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600fcfe);
    udelay(10);
    //Line 487
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01feff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040002ff);
    udelay(10);
    //Line 488
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fd00);
    udelay(10);
    //Line 489
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fa00);
    udelay(10);
    //Line 490
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0300ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fe01);
    udelay(10);
    //Line 491
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0106fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300ffff);
    udelay(10);
    //Line 492
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fcff);
    udelay(10);
    //Line 493
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010300ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0601050d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020b0304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff030500);
    udelay(10);
    //Line 494
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9000009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa0707fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08f60003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000900);
    udelay(10);
    //Line 495
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdf9fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f7fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08fff900);
    udelay(10);
    //Line 496
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff0008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdff05fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeefff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fefe);
    udelay(10);
    //Line 497
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd00fef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020305fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd040502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc01fe02);
    udelay(10);
    //Line 498
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf70102fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020802fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x07000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101fafd);
    udelay(10);
    //Line 499
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfcfef2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000301f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00f802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fffcfe);
    udelay(10);
    //Line 500
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf901fff7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0001);
    udelay(10);
    //Line 501
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fcfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff04ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00040001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe0100);
    udelay(10);
    //Line 502
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02f90200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffff901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060003ff);
    udelay(10);
    //Line 503
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd02fafd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fafefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe01fefc);
    udelay(10);
    //Line 504
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe04fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020003ff);
    udelay(10);
    //Line 505
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefd03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000300);
    udelay(10);
    //Line 506
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf00100db);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030802fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf006f3fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xeb00fe08);
    udelay(10);
    //Line 507
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf80101f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x07f4fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xedfff806);
    udelay(10);
    //Line 508
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fcfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf0fefb05);
    udelay(10);
    //Line 509
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05ff00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010403f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb06f802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff02fd);
    udelay(10);
    //Line 510
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400f4fa);
    udelay(10);
    //Line 511
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030100fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00f8fe);
    udelay(10);
    //Line 512
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fffefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0201);
    udelay(10);
    //Line 513
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000103);
    udelay(10);
    //Line 514
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdfefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300f901);
    udelay(10);
    //Line 515
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfd0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0401fefd);
    udelay(10);
    //Line 516
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000200);
    udelay(10);
    //Line 517
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fcfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000302ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000101);
    udelay(10);
    //Line 518
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000100fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fa01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7ff0202);
    udelay(10);
    //Line 519
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0500000b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0b06f800);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400ee01);
    udelay(10);
    //Line 520
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe06ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0207f701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0a00f4fe);
    udelay(10);
    //Line 521
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f90002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fdfe);
    udelay(10);
    //Line 522
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfd030d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ec0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf800f7fd);
    udelay(10);
    //Line 523
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd010006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fc0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010101fd);
    udelay(10);
    //Line 524
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff01);
    udelay(10);
    //Line 525
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fefefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ff0102);
    udelay(10);
    //Line 526
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02f70100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000ff);
    udelay(10);
    //Line 527
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000400);
    udelay(10);
    //Line 528
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000303fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010101);
    udelay(10);
    //Line 529
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000101);
    udelay(10);
    //Line 530
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000500);
    udelay(10);
    //Line 531
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010001ff);
    udelay(10);
    //Line 532
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf50000ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0404000c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf6ed0701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0601e50d);
    udelay(10);
    //Line 533
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xecfd00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02f201ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fb0a02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf001060c);
    udelay(10);
    //Line 534
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f801f1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6fd0308);
    udelay(10);
    //Line 535
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff03ffef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff040200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffafbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffffccfc);
    udelay(10);
    //Line 536
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0003ff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020202fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5fff102);
    udelay(10);
    //Line 537
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc020003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04f4fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0402ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0503);
    udelay(10);
    //Line 538
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fb00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfc0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000102);
    udelay(10);
    //Line 539
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500faff);
    udelay(10);
    //Line 540
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000f903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fe00);
    udelay(10);
    //Line 541
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefefe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf801ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fc00);
    udelay(10);
    //Line 542
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8fe01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000000fe);
    udelay(10);
    //Line 543
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010103);
    udelay(10);
    //Line 544
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00feff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fbfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefffe04);
    udelay(10);
    //Line 545
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa03fff1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0403fff6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xef07f400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf402fa08);
    udelay(10);
    //Line 546
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0b03fddd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0500f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcf8ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xedf8f8fc);
    udelay(10);
    //Line 547
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0100f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfdfdf4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00fcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7fdf202);
    udelay(10);
    //Line 548
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffef8f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf50d0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000206);
    udelay(10);
    //Line 549
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0400ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fffa00);
    udelay(10);
    //Line 550
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fa00);
    udelay(10);
    //Line 551
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010202fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0502);
    udelay(10);
    //Line 552
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff05ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 553
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffd00);
    udelay(10);
    //Line 554
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00fefe);
    udelay(10);
    //Line 555
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000102);
    udelay(10);
    //Line 556
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000100);
    udelay(10);
    //Line 557
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0000ff);
    udelay(10);
    //Line 558
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0bfffd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff03f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffaf501);
    udelay(10);
    //Line 559
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fbfc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0300ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fff2ff);
    udelay(10);
    //Line 560
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04050001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0700fffe);
    udelay(10);
    //Line 561
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fe03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500fbff);
    udelay(10);
    //Line 562
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060102fe);
    udelay(10);
    //Line 563
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05000801);
    udelay(10);
    //Line 564
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0501ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000ff);
    udelay(10);
    //Line 565
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01feffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000200);
    udelay(10);
    //Line 566
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000400);
    udelay(10);
    //Line 567
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000303);
    udelay(10);
    //Line 568
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000000);
    udelay(10);
    //Line 569
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000200);
    udelay(10);
    //Line 570
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000100);
    udelay(10);
    //Line 571
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x060200fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc050005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0500feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00f0ff);
    udelay(10);
    //Line 572
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03040001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc030410);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x080c02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100f400);
    udelay(10);
    //Line 573
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0203feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffcff);
    udelay(10);
    //Line 574
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fb0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fff4ff);
    udelay(10);
    //Line 575
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05ff0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04ffff02);
    udelay(10);
    //Line 576
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc04ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000400);
    udelay(10);
    //Line 577
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfdffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000500);
    udelay(10);
    //Line 578
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000302);
    udelay(10);
    //Line 579
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000306);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010003fe);
    udelay(10);
    //Line 580
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000201);
    udelay(10);
    //Line 581
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0102);
    udelay(10);
    //Line 582
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000201);
    udelay(10);
    //Line 583
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0001ff);
    udelay(10);
    //Line 584
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0303ff0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020bff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0401fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8010902);
    udelay(10);
    //Line 585
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030100fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd04fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f4fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefffa01);
    udelay(10);
    //Line 586
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010101f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020109);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fc01);
    udelay(10);
    //Line 587
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf30c0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0703);
    udelay(10);
    //Line 588
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x070000f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf704fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08fff9fd);
    udelay(10);
    //Line 589
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a01fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd070700);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fef200);
    udelay(10);
    //Line 590
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0001fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdf80200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8ff0402);
    udelay(10);
    //Line 591
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03f9fdf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0302);
    udelay(10);
    //Line 592
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fafaf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fefb01);
    udelay(10);
    //Line 593
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fe0105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02f1f5f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefa0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fb00);
    udelay(10);
    //Line 594
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fefa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf501fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010201);
    udelay(10);
    //Line 595
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbff0001);
    udelay(10);
    //Line 596
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfffd01);
    udelay(10);
    //Line 597
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0600f9fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe040402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05f801fd);
    udelay(10);
    //Line 598
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fd0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfef6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f503fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0a000401);
    udelay(10);
    //Line 599
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf8000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0305fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x09000800);
    udelay(10);
    //Line 600
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf6fe0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02f904fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fffeff);
    udelay(10);
    //Line 601
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400ffff);
    udelay(10);
    //Line 602
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0000f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0304fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0602);
    udelay(10);
    //Line 603
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0402fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000102);
    udelay(10);
    //Line 604
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fe01);
    udelay(10);
    //Line 605
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000300f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030103fe);
    udelay(10);
    //Line 606
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040001ff);
    udelay(10);
    //Line 607
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100feff);
    udelay(10);
    //Line 608
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fcff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000402);
    udelay(10);
    //Line 609
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fefe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000201);
    udelay(10);
    //Line 610
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fffe09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x12fc00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fefa01);
    udelay(10);
    //Line 611
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0502ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04030502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x060e0500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffefa02);
    udelay(10);
    //Line 612
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fe0008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd02ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x070004fd);
    udelay(10);
    //Line 613
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06feff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000afb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07000101);
    udelay(10);
    //Line 614
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fef602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff04fe);
    udelay(10);
    //Line 615
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0001f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00050201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0904);
    udelay(10);
    //Line 616
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0000);
    udelay(10);
    //Line 617
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0200ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000102);
    udelay(10);
    //Line 618
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030003ff);
    udelay(10);
    //Line 619
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff030504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0003ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000201);
    udelay(10);
    //Line 620
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fa02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fdff);
    udelay(10);
    //Line 621
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000301);
    udelay(10);
    //Line 622
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 623
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf00000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0406fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf800fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020107);
    udelay(10);
    //Line 624
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05eb0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfefd02);
    udelay(10);
    //Line 625
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050100f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff01f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f7fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9fde600);
    udelay(10);
    //Line 626
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa04fbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0800fef7);
    udelay(10);
    //Line 627
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fdfb0d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0104ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fa00);
    udelay(10);
    //Line 628
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f40600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8fe0500);
    udelay(10);
    //Line 629
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f8fff7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f00101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000100);
    udelay(10);
    //Line 630
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fd00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf905ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fffd00);
    udelay(10);
    //Line 631
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff6fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f30301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00faff);
    udelay(10);
    //Line 632
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8fafd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010101fe);
    udelay(10);
    //Line 633
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0002fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0006fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000102);
    udelay(10);
    //Line 634
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05000007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f9fbfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fd03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9ffff00);
    udelay(10);
    //Line 635
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfffb02);
    udelay(10);
    //Line 636
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa02ffed);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf90700fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xe609f100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xef000a06);
    udelay(10);
    //Line 637
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x070100f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf90600fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf800f101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600eafa);
    udelay(10);
    //Line 638
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000100f0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0500fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf803f3ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200f1ff);
    udelay(10);
    //Line 639
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fa04fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8020303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000c06);
    udelay(10);
    //Line 640
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fd01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fefef8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff07ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07000303);
    udelay(10);
    //Line 641
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04090100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fff101);
    udelay(10);
    //Line 642
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00ff01);
    udelay(10);
    //Line 643
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9ff0401);
    udelay(10);
    //Line 644
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00ff00);
    udelay(10);
    //Line 645
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fb00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00fe01);
    udelay(10);
    //Line 646
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000200);
    udelay(10);
    //Line 647
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeffffff);
    udelay(10);
    //Line 648
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffcff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fd01);
    udelay(10);
    //Line 649
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf3000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd00fdf4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff07fcfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf700fd08);
    udelay(10);
    //Line 650
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf3020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fcf0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02f9f8f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbffe808);
    udelay(10);
    //Line 651
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0102f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0400fcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00ee05);
    udelay(10);
    //Line 652
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa06fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000304);
    udelay(10);
    //Line 653
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe06ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030000fe);
    udelay(10);
    //Line 654
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020100f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020500ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fff901);
    udelay(10);
    //Line 655
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300ffff);
    udelay(10);
    //Line 656
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fb0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000102ff);
    udelay(10);
    //Line 657
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06f80300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0101ff);
    udelay(10);
    //Line 658
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020000ff);
    udelay(10);
    //Line 659
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0400ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600fffe);
    udelay(10);
    //Line 660
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000100);
    udelay(10);
    //Line 661
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020101ff);
    udelay(10);
    //Line 662
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0bfdff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcff04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fef702);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffcedfe);
    udelay(10);
    //Line 663
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07fd0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0604ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0700f8fe);
    udelay(10);
    //Line 664
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff04fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff02ff);
    udelay(10);
    //Line 665
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0401ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0202);
    udelay(10);
    //Line 666
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00050400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdffff03);
    udelay(10);
    //Line 667
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000402ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0302fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000500);
    udelay(10);
    //Line 668
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000401);
    udelay(10);
    //Line 669
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000005ff);
    udelay(10);
    //Line 670
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000101);
    udelay(10);
    //Line 671
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000500);
    udelay(10);
    //Line 672
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000200);
    udelay(10);
    //Line 673
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff00);
    udelay(10);
    //Line 674
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000200);
    udelay(10);
    //Line 675
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fa0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01f50401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010005fc);
    udelay(10);
    //Line 676
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc01ff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0802);
    udelay(10);
    //Line 677
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fbff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000404);
    udelay(10);
    //Line 678
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0700fd01);
    udelay(10);
    //Line 679
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfdfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010306ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd07fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0a010102);
    udelay(10);
    //Line 680
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000108);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfe07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f901fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fb00);
    udelay(10);
    //Line 681
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0000ff);
    udelay(10);
    //Line 682
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefbfffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000301);
    udelay(10);
    //Line 683
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefeff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fdfe);
    udelay(10);
    //Line 684
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000500fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0101);
    udelay(10);
    //Line 685
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03feffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000101);
    udelay(10);
    //Line 686
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200feff);
    udelay(10);
    //Line 687
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02030000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600fd00);
    udelay(10);
    //Line 688
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fffff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9020202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010c02);
    udelay(10);
    //Line 689
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfaff0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbfaf8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcf60400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000800);
    udelay(10);
    //Line 690
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0402);
    udelay(10);
    //Line 691
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfb03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0402);
    udelay(10);
    //Line 692
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0302);
    udelay(10);
    //Line 693
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0700fd01);
    udelay(10);
    //Line 694
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000102);
    udelay(10);
    //Line 695
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000100fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0400fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe03ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000101);
    udelay(10);
    //Line 696
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fe00);
    udelay(10);
    //Line 697
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300fffa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fd00);
    udelay(10);
    //Line 698
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000001);
    udelay(10);
    //Line 699
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ff01);
    udelay(10);
    //Line 700
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff00);
    udelay(10);
    //Line 701
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5fb02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000dff);
    udelay(10);
    //Line 702
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fdfcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcff06ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000c00);
    udelay(10);
    //Line 703
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000d01);
    udelay(10);
    //Line 704
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030005fe);
    udelay(10);
    //Line 705
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020103ff);
    udelay(10);
    //Line 706
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 707
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000202);
    udelay(10);
    //Line 708
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fff901);
    udelay(10);
    //Line 709
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fd00);
    udelay(10);
    //Line 710
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000201);
    udelay(10);
    //Line 711
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fffd01);
    udelay(10);
    //Line 712
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff00);
    udelay(10);
    //Line 713
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ffff);
    udelay(10);
    //Line 714
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00040201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500fefd);
    udelay(10);
    //Line 715
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff04fc06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0007fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050101fd);
    udelay(10);
    //Line 716
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fcfd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fdfe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0300);
    udelay(10);
    //Line 717
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06020103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0601);
    udelay(10);
    //Line 718
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f90400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0302);
    udelay(10);
    //Line 719
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fc0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000601);
    udelay(10);
    //Line 720
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0301fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03010301);
    udelay(10);
    //Line 721
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00040000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300ff00);
    udelay(10);
    //Line 722
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 723
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffd01);
    udelay(10);
    //Line 724
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0401);
    udelay(10);
    //Line 725
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe04ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200feff);
    udelay(10);
    //Line 726
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 727
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02f707fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfcfcfc);
    udelay(10);
    //Line 728
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff01ff);
    udelay(10);
    //Line 729
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020001ff);
    udelay(10);
    //Line 730
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f8ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0001fe);
    udelay(10);
    //Line 731
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000401);
    udelay(10);
    //Line 732
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00040101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0204fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fc00);
    udelay(10);
    //Line 733
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fe00);
    udelay(10);
    //Line 734
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0104fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05000100);
    udelay(10);
    //Line 735
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000100);
    udelay(10);
    //Line 736
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 737
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000001ff);
    udelay(10);
    //Line 738
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000300);
    udelay(10);
    //Line 739
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000201);
    udelay(10);
    //Line 740
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf6ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0100fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf6fd05ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06000afb);
    udelay(10);
    //Line 741
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030000f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf90a03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000cfe);
    udelay(10);
    //Line 742
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01040400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05000001);
    udelay(10);
    //Line 743
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff090400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07000301);
    udelay(10);
    //Line 744
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00f5fe);
    udelay(10);
    //Line 745
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000903);
    udelay(10);
    //Line 746
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000301);
    udelay(10);
    //Line 747
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff01);
    udelay(10);
    //Line 748
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000001);
    udelay(10);
    //Line 749
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbffff02);
    udelay(10);
    //Line 750
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffffd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0000ff);
    udelay(10);
    //Line 751
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 752
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01040200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fc00);
    udelay(10);
    //Line 753
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040000f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9fc0105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0802ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffef8);
    udelay(10);
    //Line 754
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff080200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04ff05fb);
    udelay(10);
    //Line 755
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00070405);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0c02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000001ff);
    udelay(10);
    //Line 756
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020300fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe050106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00f9fe);
    udelay(10);
    //Line 757
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f802ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fffe);
    udelay(10);
    //Line 758
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefd0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fbfffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0105ff);
    udelay(10);
    //Line 759
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe0007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000002fd);
    udelay(10);
    //Line 760
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000600);
    udelay(10);
    //Line 761
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0001ff);
    udelay(10);
    //Line 762
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020001ff);
    udelay(10);
    //Line 763
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020001fe);
    udelay(10);
    //Line 764
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000feff);
    udelay(10);
    //Line 765
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000200);
    udelay(10);
    //Line 766
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf200ffee);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x060201f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf900f801);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd01f105);
    udelay(10);
    //Line 767
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0100f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfff80101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ef0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf2ff040c);
    udelay(10);
    //Line 768
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf60201f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fc00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fdfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf800fd06);
    udelay(10);
    //Line 769
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a0300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ffe4fb);
    udelay(10);
    //Line 770
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe05fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf700f700);
    udelay(10);
    //Line 771
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0002ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fb0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9f70002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf0ff0502);
    udelay(10);
    //Line 772
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fc01);
    udelay(10);
    //Line 773
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300f900);
    udelay(10);
    //Line 774
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd03fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00ff02);
    udelay(10);
    //Line 775
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06010004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef80001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00feff);
    udelay(10);
    //Line 776
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafefd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000001);
    udelay(10);
    //Line 777
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000103);
    udelay(10);
    //Line 778
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000300ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00ff03);
    udelay(10);
    //Line 779
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x15e0e111);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x181c0ee8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x20180ce8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x141ae418);
    udelay(10);
    //Line 780
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x201d18de);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x181e2716);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xe4dfe719);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x16e81317);
    udelay(10);
    //Line 781
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xed2220ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xe3e5f319);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xdee8f215);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xefe61ce6);
    udelay(10);
    //Line 782
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xdfe1ea1e);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xeae3ddeb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x1b2217e4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xe61befe9);
    udelay(10);
    //Line 783
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf104f800);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fff8ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ef03ff);
    udelay(10);
    //Line 784
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfafe0ffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030604fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff07e7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000f8fa);
    udelay(10);
    //Line 785
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0602f906);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0308ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fb0503);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02070703);
    udelay(10);
    //Line 786
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0103f9fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf70312fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff030505);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfef10208);
    udelay(10);
    //Line 787
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x080210fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xed06f406);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5f4fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01070001);
    udelay(10);
    //Line 788
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fb0504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8fbfdf5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00eaf3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0503f4);
    udelay(10);
    //Line 789
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fb0000);
    udelay(10);
    
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_1_reg, 0x00000000);

    //mean, std settings
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_3_reg, 0x02f0f4ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_4_reg, 0xf5f4f9ee);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_5_reg, 0x00f5faf4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_6_reg, 0x000018fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_7_reg, 0x13141317);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_8_reg, 0x13121313);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_9_reg, 0x16151314);

    //TH settings
    //scale[25:22], dnn_th[15:8], kw_weight[4:0]
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x03c00000, (scale<<22));
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x0000ff00, (dnn_th<<8));
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x0000001f, kw_weight);

    ret |= rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_tcon_1_reg, &SVAD_tcon_1_reg);
    
    SVAD_INFO("\033[1;32;33m" "TH settings: 0x%08x" "\033[m", SVAD_tcon_1_reg);
#endif
}
EXPORT_SYMBOL(rtk_svad_set_model_g);
