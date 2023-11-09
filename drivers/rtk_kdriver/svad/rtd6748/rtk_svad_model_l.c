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

void rtk_svad_set_model_l(unsigned int scale, unsigned int dnn_th, unsigned int kw_weight)
{
#ifdef MIO_I2C_MODE
    int ret = 0;
    unsigned int SVAD_tcon_1_reg = 0;
    unsigned int reg_value_sys_srst = 0;
    unsigned int reg_value_sys_clken = 0;
#endif
    
    SVAD_INFO("\033[1;32;33m" "rtk_svad_set_model_l: scale=%u, dnn_th=%u, kw_weight=%u" "\033[m", scale, dnn_th, kw_weight);

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
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf20a0bf0);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf0fcfa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef9f6ef);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bf4f9fb);
    udelay(10);
    //Line 2
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7070bf7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdf9fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbf5f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfcfafb);
    udelay(10);
    //Line 3
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80507fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103f7ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0cfdfdfc);
    udelay(10);
    //Line 4
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0408ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0304fa02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200f806);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fcffff);
    udelay(10);
    //Line 5
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010600);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0102);
    udelay(10);
    //Line 6
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0102);
    udelay(10);
    //Line 7
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0501);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa010202);
    udelay(10);
    //Line 8
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030900);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fe0402);
    udelay(10);
    //Line 9
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfeff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0aff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb03fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7000403);
    udelay(10);
    //Line 10
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010405);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fc03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f90302);
    udelay(10);
    //Line 11
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02080601);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafcf9f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f701fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fff4f5);
    udelay(10);
    //Line 12
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fcf4f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fa02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fff5f5);
    udelay(10);
    //Line 13
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcffeefa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafa01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501f2f4);
    udelay(10);
    //Line 14
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0304ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffff5fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfa01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03f3f2);
    udelay(10);
    //Line 15
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200faf6);
    udelay(10);
    //Line 16
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd06fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdfa);
    udelay(10);
    //Line 17
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0106fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffd);
    udelay(10);
    //Line 18
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fbfd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020500);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    udelay(10);
    //Line 19
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbfc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000403);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff01);
    udelay(10);
    //Line 20
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfd03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402f602);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc05fcfd);
    udelay(10);
    //Line 21
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0e0dfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfb08f5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7f4fafa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x11fbfffb);
    udelay(10);
    //Line 22
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0800f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf808f4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5f7fef9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fbfef9);
    udelay(10);
    //Line 23
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020400);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf90bf8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafb00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0000);
    udelay(10);
    //Line 24
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040304);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0509ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010206);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010703);
    udelay(10);
    //Line 25
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd050400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010507);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010706);
    udelay(10);
    //Line 26
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030105);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030201);
    udelay(10);
    //Line 27
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030202);
    udelay(10);
    //Line 28
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010100);
    udelay(10);
    //Line 29
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020301);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb010001);
    udelay(10);
    //Line 30
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000304ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fa03fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafd00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfefefc);
    udelay(10);
    //Line 31
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04070b0c);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0608080c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb060d0b);
    udelay(10);
    //Line 32
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030303);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0401);
    udelay(10);
    //Line 33
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0003fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    udelay(10);
    //Line 34
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd07fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefb02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010301);
    udelay(10);
    //Line 35
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff03fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010302);
    udelay(10);
    //Line 36
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0003ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000104ff);
    udelay(10);
    //Line 37
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0103ff);
    udelay(10);
    //Line 38
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fefe);
    udelay(10);
    //Line 39
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020006fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000301);
    udelay(10);
    //Line 40
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd07fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0301);
    udelay(10);
    //Line 41
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09f4f905);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0709fe0b);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x090a0806);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3080207);
    udelay(10);
    //Line 42
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef9fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001f703);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf304ffff);
    udelay(10);
    //Line 43
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfd04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0204ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf20301fd);
    udelay(10);
    //Line 44
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04040103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf300fd01);
    udelay(10);
    //Line 45
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8020400);
    udelay(10);
    //Line 46
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0503fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0102);
    udelay(10);
    //Line 47
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0104fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fc04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0304);
    udelay(10);
    //Line 48
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0103fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb0201);
    udelay(10);
    //Line 49
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0303ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00f804);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0503fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa0203);
    udelay(10);
    //Line 50
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0605ef03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fe01);
    udelay(10);
    //Line 51
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff01fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfefd);
    udelay(10);
    //Line 52
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0308);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05050000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0506);
    udelay(10);
    //Line 53
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030403ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030303);
    udelay(10);
    //Line 54
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff04ff);
    udelay(10);
    //Line 55
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffa00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03feffff);
    udelay(10);
    //Line 56
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003f501);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000fe);
    udelay(10);
    //Line 57
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100fd);
    udelay(10);
    //Line 58
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ea04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fc01);
    udelay(10);
    //Line 59
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000dd03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010401fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa04f6ff);
    udelay(10);
    //Line 60
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0206e000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fc0502);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf803f1fb);
    udelay(10);
    //Line 61
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffafe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefefc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fcfd);
    udelay(10);
    //Line 62
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefb01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdfe);
    udelay(10);
    //Line 63
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffefe);
    udelay(10);
    //Line 64
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffffe);
    udelay(10);
    //Line 65
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000201);
    udelay(10);
    //Line 66
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030001fc);
    udelay(10);
    //Line 67
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010300);
    udelay(10);
    //Line 68
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08000201);
    udelay(10);
    //Line 69
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050101fe);
    udelay(10);
    //Line 70
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05feff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0605fefe);
    udelay(10);
    //Line 71
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040009);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00060103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa040102);
    udelay(10);
    //Line 72
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fa00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb04feff);
    udelay(10);
    //Line 73
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0104ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff00);
    udelay(10);
    //Line 74
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fe00);
    udelay(10);
    //Line 75
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020102);
    udelay(10);
    //Line 76
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    udelay(10);
    //Line 77
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffeff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000500);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0202);
    udelay(10);
    //Line 78
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0005fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfc0d01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0201);
    udelay(10);
    //Line 79
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0404fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fd09fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0503);
    udelay(10);
    //Line 80
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90503fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fb1600);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffb0700);
    udelay(10);
    //Line 81
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0afe000d);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0e1b0511);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x10100718);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa09130f);
    udelay(10);
    //Line 82
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02faf801);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefbf9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfbfa);
    udelay(10);
    //Line 83
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfcfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd03fef9);
    udelay(10);
    //Line 84
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100fe);
    udelay(10);
    //Line 85
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0001);
    udelay(10);
    //Line 86
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fb01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff03);
    udelay(10);
    //Line 87
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0201);
    udelay(10);
    //Line 88
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fc0202);
    udelay(10);
    //Line 89
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000103);
    udelay(10);
    //Line 90
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201f9fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefe01);
    udelay(10);
    //Line 91
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06edf407);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000504);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502fefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7060302);
    udelay(10);
    //Line 92
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf6f900);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000406);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000604);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000603);
    udelay(10);
    //Line 93
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbfc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000701);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0503);
    udelay(10);
    //Line 94
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcfdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0aff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0700);
    udelay(10);
    //Line 95
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020104fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0603);
    udelay(10);
    //Line 96
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010500);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04020404);
    udelay(10);
    //Line 97
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0403);
    udelay(10);
    //Line 98
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040101ff);
    udelay(10);
    //Line 99
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000101);
    udelay(10);
    //Line 100
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffc04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010301);
    udelay(10);
    //Line 101
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0702fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefe03);
    udelay(10);
    //Line 102
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050300ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000203);
    udelay(10);
    //Line 103
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    udelay(10);
    //Line 104
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    udelay(10);
    //Line 105
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000105);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020001);
    udelay(10);
    //Line 106
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0405);
    udelay(10);
    //Line 107
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfefd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fd03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020304);
    udelay(10);
    //Line 108
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403f206);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    udelay(10);
    //Line 109
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402e001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f6ff);
    udelay(10);
    //Line 110
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0905e1ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fe0502);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04f3fd);
    udelay(10);
    //Line 111
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fafcfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfaeefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb02f9fb);
    udelay(10);
    //Line 112
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fbfa05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03f800);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb010101);
    udelay(10);
    //Line 113
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05faf806);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefef903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000602);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb010406);
    udelay(10);
    //Line 114
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fbfb06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04f400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9020102);
    udelay(10);
    //Line 115
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03faf904);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00f001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7010000);
    udelay(10);
    //Line 116
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f8fa04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fff001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf801ffff);
    udelay(10);
    //Line 117
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfc03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02f0ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fffdff);
    udelay(10);
    //Line 118
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fbfc03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffeefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0004ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf804fcff);
    udelay(10);
    //Line 119
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f9fc01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302e401);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf501f9fd);
    udelay(10);
    //Line 120
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f9fc05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0702dbfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc06ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb08f4f9);
    udelay(10);
    //Line 121
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08fdff0a);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03070009);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0005080a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7060806);
    udelay(10);
    //Line 122
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd040303);
    udelay(10);
    //Line 123
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    udelay(10);
    //Line 124
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfc03fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fdfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    udelay(10);
    //Line 125
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfc0afc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfc01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010103ff);
    udelay(10);
    //Line 126
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0008fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0002);
    udelay(10);
    //Line 127
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe07fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfbff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ffff);
    udelay(10);
    //Line 128
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd07fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe01ff);
    udelay(10);
    //Line 129
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd07ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0300);
    udelay(10);
    //Line 130
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8001300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000804);
    udelay(10);
    //Line 131
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02070807);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0405ff0b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8020c0a);
    udelay(10);
    //Line 132
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040304);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02080104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030608);
    udelay(10);
    //Line 133
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020404);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00050302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0605);
    udelay(10);
    //Line 134
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000401);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0201);
    udelay(10);
    //Line 135
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0203);
    udelay(10);
    //Line 136
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe01);
    udelay(10);
    //Line 137
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0100);
    udelay(10);
    //Line 138
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe06ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0100);
    udelay(10);
    //Line 139
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0405ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010400);
    udelay(10);
    //Line 140
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0506fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe0c01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf902ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00ff);
    udelay(10);
    //Line 141
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02eef304);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010502);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040a01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5050408);
    udelay(10);
    //Line 142
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f5f802);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0706);
    udelay(10);
    //Line 143
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fafb02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020501);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010405);
    udelay(10);
    //Line 144
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000204fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0401);
    udelay(10);
    //Line 145
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfcff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020007ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0402);
    udelay(10);
    //Line 146
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020105fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fdfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0200);
    udelay(10);
    //Line 147
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    udelay(10);
    //Line 148
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fafb02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe01);
    udelay(10);
    //Line 149
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f8f902);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010200);
    udelay(10);
    //Line 150
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f8f901);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000200);
    udelay(10);
    //Line 151
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x070c0b0a);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05090007);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fe0e0c);
    udelay(10);
    //Line 152
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020503);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0604);
    udelay(10);
    //Line 153
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfefc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    udelay(10);
    //Line 154
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffa00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfefd);
    udelay(10);
    //Line 155
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef902fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf800fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefdfc);
    udelay(10);
    //Line 156
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0103);
    udelay(10);
    //Line 157
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020302);
    udelay(10);
    //Line 158
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010304);
    udelay(10);
    //Line 159
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa000200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020102);
    udelay(10);
    //Line 160
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030301);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafc0801);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000503);
    udelay(10);
    //Line 161
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03090209);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0808ff09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000709);
    udelay(10);
    //Line 162
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0104fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010203);
    udelay(10);
    //Line 163
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fa01ff);
    udelay(10);
    //Line 164
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100f4fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fc00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefe01);
    udelay(10);
    //Line 165
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feedff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdfdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fafe);
    udelay(10);
    //Line 166
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ee00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefefc);
    udelay(10);
    //Line 167
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffef1fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fc01);
    udelay(10);
    //Line 168
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ef01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00fd00);
    udelay(10);
    //Line 169
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0305e5ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa03faff);
    udelay(10);
    //Line 170
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0504e301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fb0603);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106f3f8);
    udelay(10);
    //Line 171
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0b01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0506);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf904030a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020903);
    udelay(10);
    //Line 172
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafffcfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf80000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc03fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0505fdfd);
    udelay(10);
    //Line 173
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010300);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffa0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010100);
    udelay(10);
    //Line 174
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefb0200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000100);
    udelay(10);
    //Line 175
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef90100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200fd);
    udelay(10);
    //Line 176
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef9ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf803ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff01fb);
    udelay(10);
    //Line 177
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0107fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0704);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f10300);
    udelay(10);
    //Line 178
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fd04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0402);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f80102);
    udelay(10);
    //Line 179
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fcfe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fd0201);
    udelay(10);
    //Line 180
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fe0001);
    udelay(10);
    //Line 181
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0101);
    udelay(10);
    //Line 182
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0300ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb0300);
    udelay(10);
    //Line 183
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08050705);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fc0704);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fd0608);
    udelay(10);
    //Line 184
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffe);
    udelay(10);
    //Line 185
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc0103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fcfe);
    udelay(10);
    //Line 186
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7faf702);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe02fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fafe);
    udelay(10);
    //Line 187
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfef303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030106fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffd01);
    udelay(10);
    //Line 188
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfff604);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0401fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0900fc01);
    udelay(10);
    //Line 189
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010300);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020303);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe9fe0302);
    udelay(10);
    //Line 190
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff05fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafeff02);
    udelay(10);
    //Line 191
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc04f901);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feffff);
    udelay(10);
    //Line 192
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0004fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb07fdfb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fbf9);
    udelay(10);
    //Line 193
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010207fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc080100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040100ff);
    udelay(10);
    //Line 194
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060407f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fbfe06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa040800);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb020100);
    udelay(10);
    //Line 195
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffefb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefffb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa01fefc);
    udelay(10);
    //Line 196
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04020300);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0303);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9000305);
    udelay(10);
    //Line 197
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000000);
    udelay(10);
    //Line 198
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0201fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa00ff03);
    udelay(10);
    //Line 199
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf500fffe);
    udelay(10);
    //Line 200
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0403ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0000ff);
    udelay(10);
    //Line 201
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03060502);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f9fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff0406);
    udelay(10);
    //Line 202
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fa0006);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000102);
    udelay(10);
    //Line 203
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefaff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000003);
    udelay(10);
    //Line 204
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefbffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01ff02);
    udelay(10);
    //Line 205
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef60101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6000001);
    udelay(10);
    //Line 206
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe02f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f406ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3020000);
    udelay(10);
    //Line 207
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f1f8fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf203fef5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fcf0);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a04f0f1);
    udelay(10);
    //Line 208
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000500);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0002);
    udelay(10);
    //Line 209
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010205ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0201);
    udelay(10);
    //Line 210
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010005fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    udelay(10);
    //Line 211
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000303ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaff02ff);
    udelay(10);
    //Line 212
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0600);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe0302);
    udelay(10);
    //Line 213
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbfff8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafd00fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fef7f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa06faf7);
    udelay(10);
    //Line 214
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000f06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05feff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeffe06ff);
    udelay(10);
    //Line 215
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100ff);
    udelay(10);
    //Line 216
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0501ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff01ff);
    udelay(10);
    //Line 217
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0101);
    udelay(10);
    //Line 218
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefd04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0405fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0002);
    udelay(10);
    //Line 219
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a060b04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb0106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4ff0603);
    udelay(10);
    //Line 220
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030400);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0300fd);
    udelay(10);
    //Line 221
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf802fcfd);
    udelay(10);
    //Line 222
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7ff0202);
    udelay(10);
    //Line 223
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7ff0002);
    udelay(10);
    //Line 224
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7020000);
    udelay(10);
    //Line 225
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203f5f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0008fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf601fe00);
    udelay(10);
    //Line 226
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfa0705);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6000504);
    udelay(10);
    //Line 227
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000203fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0101);
    udelay(10);
    //Line 228
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000400);
    udelay(10);
    //Line 229
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010003);
    udelay(10);
    //Line 230
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefa01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe05ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000001);
    udelay(10);
    //Line 231
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030100f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07feff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f905ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fd0002);
    udelay(10);
    //Line 232
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06030602);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fb0501);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0402);
    udelay(10);
    //Line 233
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fd0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0101ff);
    udelay(10);
    //Line 234
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fd01);
    udelay(10);
    //Line 235
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08ff0400);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    udelay(10);
    //Line 236
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafe0a);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafffdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffffe);
    udelay(10);
    //Line 237
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0400ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404ffff);
    udelay(10);
    //Line 238
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0400fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 239
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0500ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060001ff);
    udelay(10);
    //Line 240
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd04fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    udelay(10);
    //Line 241
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff04f8ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    udelay(10);
    //Line 242
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd05ff09);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030a08ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a07f703);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffd02);
    udelay(10);
    //Line 243
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0700);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0003fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0101);
    udelay(10);
    //Line 244
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0500);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff03fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd060001);
    udelay(10);
    //Line 245
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc00ff);
    udelay(10);
    //Line 246
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f90101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff04fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb060100);
    udelay(10);
    //Line 247
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04f702fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbf700);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff07fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000200);
    udelay(10);
    //Line 248
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07f502f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf5f601);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fb0afd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70704fc);
    udelay(10);
    //Line 249
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02fbfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000901);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9f6ff00);
    udelay(10);
    //Line 250
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcf200ff);
    udelay(10);
    //Line 251
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fd04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f30203);
    udelay(10);
    //Line 252
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f400ff);
    udelay(10);
    //Line 253
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000400fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fefb02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f60202);
    udelay(10);
    //Line 254
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fb05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0307fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fdf502);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02f7ff04);
    udelay(10);
    //Line 255
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000305);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010107);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef50607);
    udelay(10);
    //Line 256
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc0000);
    udelay(10);
    //Line 257
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fc0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd01ff);
    udelay(10);
    //Line 258
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0502fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe00fe);
    udelay(10);
    //Line 259
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe02fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0501);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050104ff);
    udelay(10);
    //Line 260
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08010100);
    udelay(10);
    //Line 261
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfafe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1005fdfe);
    udelay(10);
    //Line 262
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fef9ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0101fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fbfc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fbfd);
    udelay(10);
    //Line 263
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302feff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0901fe01);
    udelay(10);
    //Line 264
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0002);
    udelay(10);
    //Line 265
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04ff0003);
    udelay(10);
    //Line 266
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffe04);
    udelay(10);
    //Line 267
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    udelay(10);
    //Line 268
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 269
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 270
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    udelay(10);
    //Line 271
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    udelay(10);
    //Line 272
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfefffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fff8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020102fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe05);
    udelay(10);
    //Line 273
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd070002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfdf7ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf400f7fe);
    udelay(10);
    //Line 274
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040301fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05040100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb01fafe);
    udelay(10);
    //Line 275
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040302fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040201fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fefe);
    udelay(10);
    //Line 276
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010301f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe01);
    udelay(10);
    //Line 277
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010002);
    udelay(10);
    //Line 278
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101feff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04030303);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fe01);
    udelay(10);
    //Line 279
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000105f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fd03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe06f1f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04030404);
    udelay(10);
    //Line 280
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffaff);
    udelay(10);
    //Line 281
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    udelay(10);
    //Line 282
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff01);
    udelay(10);
    //Line 283
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    udelay(10);
    //Line 284
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002ff);
    udelay(10);
    //Line 285
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0dff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0302fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07ff0301);
    udelay(10);
    //Line 286
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fdff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fdfe00);
    udelay(10);
    //Line 287
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05f7fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fd0100);
    udelay(10);
    //Line 288
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fafd03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fdfdfe);
    udelay(10);
    //Line 289
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fcfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fcfc00);
    udelay(10);
    //Line 290
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x18fefb02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0205fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd040100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07fbff02);
    udelay(10);
    //Line 291
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffdf9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf802ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0701ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffc02);
    udelay(10);
    //Line 292
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020206);
    udelay(10);
    //Line 293
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fefc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfc01);
    udelay(10);
    //Line 294
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02050001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    udelay(10);
    //Line 295
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff01);
    udelay(10);
    //Line 296
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 297
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fb0f02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0100ff);
    udelay(10);
    //Line 298
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fcfa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010200);
    udelay(10);
    //Line 299
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    udelay(10);
    //Line 300
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020002fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    udelay(10);
    //Line 301
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0004fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    udelay(10);
    //Line 302
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0303f6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0602fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    udelay(10);
    //Line 303
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0201ff);
    udelay(10);
    //Line 304
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0201fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefcfa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa02fe02);
    udelay(10);
    //Line 305
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe01f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefcf8fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010103);
    udelay(10);
    //Line 306
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfbfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000103);
    udelay(10);
    //Line 307
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000105);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0701);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010108);
    udelay(10);
    //Line 308
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05020207);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020403);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030603);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020203);
    udelay(10);
    //Line 309
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05faf904);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0f800f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07ec0300);
    udelay(10);
    //Line 310
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff9f901);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fc0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f4feff);
    udelay(10);
    //Line 311
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020200);
    udelay(10);
    //Line 312
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    udelay(10);
    //Line 313
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    udelay(10);
    //Line 314
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0100);
    udelay(10);
    //Line 315
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfc08);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f803fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fff6fe);
    udelay(10);
    //Line 316
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000207);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa030102);
    udelay(10);
    //Line 317
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5040007);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fb0008);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffd07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9020200);
    udelay(10);
    //Line 318
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6080207);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fafe08);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80306fc);
    udelay(10);
    //Line 319
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4060203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01faff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf90204fa);
    udelay(10);
    //Line 320
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf40c04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffafc05);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fa04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80203f7);
    udelay(10);
    //Line 321
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fafa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000104);
    udelay(10);
    //Line 322
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fff802);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc05fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffe03);
    udelay(10);
    //Line 323
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01f803);
    udelay(10);
    //Line 324
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    udelay(10);
    //Line 325
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020103ff);
    udelay(10);
    //Line 326
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000303);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000404);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfdfe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000006fc);
    udelay(10);
    //Line 327
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0300f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020805fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0404fb07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030103);
    udelay(10);
    //Line 328
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff03fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe03ff);
    udelay(10);
    //Line 329
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02ff);
    udelay(10);
    //Line 330
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02ff);
    udelay(10);
    //Line 331
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    udelay(10);
    //Line 332
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    udelay(10);
    //Line 333
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010303fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301050b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000001);
    udelay(10);
    //Line 334
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fa);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050201ff);
    udelay(10);
    //Line 335
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000000);
    udelay(10);
    //Line 336
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020002fe);
    udelay(10);
    //Line 337
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfcff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe01fd);
    udelay(10);
    //Line 338
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    udelay(10);
    //Line 339
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fdf611);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafffc03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f90606);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfcfc);
    udelay(10);
    //Line 340
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    udelay(10);
    //Line 341
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 342
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    udelay(10);
    //Line 343
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    udelay(10);
    //Line 344
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000201);
    udelay(10);
    //Line 345
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000305);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfdfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010600);
    udelay(10);
    //Line 346
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200010b);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdfe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07ffff00);
    udelay(10);
    //Line 347
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000305);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fafa03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fd02);
    udelay(10);
    //Line 348
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffb01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffa03);
    udelay(10);
    //Line 349
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ff06);
    udelay(10);
    //Line 350
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0505);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff06ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030006);
    udelay(10);
    //Line 351
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01090afc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0105fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0507fe02);
    udelay(10);
    //Line 352
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010601);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0505fc01);
    udelay(10);
    //Line 353
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010107fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff00);
    udelay(10);
    //Line 354
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff05fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303fe00);
    udelay(10);
    //Line 355
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfdff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102feff);
    udelay(10);
    //Line 356
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203fe03);
    udelay(10);
    //Line 357
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff901ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcfc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfeff05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbfd01);
    udelay(10);
    //Line 358
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfd04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010707);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffd03);
    udelay(10);
    //Line 359
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    udelay(10);
    //Line 360
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    udelay(10);
    //Line 361
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0102);
    udelay(10);
    //Line 362
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000500);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff04);
    udelay(10);
    //Line 363
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd0501);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fe01);
    udelay(10);
    //Line 364
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000103ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030305);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    udelay(10);
    //Line 365
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040207);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200feff);
    udelay(10);
    //Line 366
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030107);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fdff);
    udelay(10);
    //Line 367
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff040102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020104ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402ff00);
    udelay(10);
    //Line 368
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0101);
    udelay(10);
    //Line 369
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc0000);
    udelay(10);
    //Line 370
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff00);
    udelay(10);
    //Line 371
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03feff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdff);
    udelay(10);
    //Line 372
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020100);
    udelay(10);
    //Line 373
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203fffe);
    udelay(10);
    //Line 374
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    udelay(10);
    //Line 375
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ffff);
    udelay(10);
    //Line 376
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc030800);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000600f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010801);
    udelay(10);
    //Line 377
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd040601);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030006);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0402);
    udelay(10);
    //Line 378
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffb0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010007);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302fcff);
    udelay(10);
    //Line 379
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fb06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafefb01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010105);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef9fc03);
    udelay(10);
    //Line 380
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fd08);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb0001);
    udelay(10);
    //Line 381
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003ff);
    udelay(10);
    //Line 382
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030100);
    udelay(10);
    //Line 383
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010100);
    udelay(10);
    //Line 384
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010202fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0303ffff);
    udelay(10);
    //Line 385
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020300);
    udelay(10);
    //Line 386
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    udelay(10);
    //Line 387
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ffff);
    udelay(10);
    //Line 388
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fd00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdff);
    udelay(10);
    //Line 389
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe03ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fef7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0702fcfc);
    udelay(10);
    //Line 390
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fdfefb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000503);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafef90b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0201);
    udelay(10);
    //Line 391
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020009);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fffe);
    udelay(10);
    //Line 392
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020003ff);
    udelay(10);
    //Line 393
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000105ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020102fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060102fe);
    udelay(10);
    //Line 394
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000301ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fdfe);
    udelay(10);
    //Line 395
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020002fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03030002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0803fbff);
    udelay(10);
    //Line 396
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    udelay(10);
    //Line 397
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    udelay(10);
    //Line 398
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdff00);
    udelay(10);
    //Line 399
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030301ff);
    udelay(10);
    //Line 400
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0201);
    udelay(10);
    //Line 401
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    udelay(10);
    //Line 402
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefc03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040202fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fd01);
    udelay(10);
    //Line 403
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02f9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fafc03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203010f);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020002);
    udelay(10);
    //Line 404
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0205fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403fefe);
    udelay(10);
    //Line 405
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe03f5);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000100);
    udelay(10);
    //Line 406
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0201);
    udelay(10);
    //Line 407
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000101);
    udelay(10);
    //Line 408
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff00);
    udelay(10);
    //Line 409
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010401);
    udelay(10);
    //Line 410
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcff01);
    udelay(10);
    //Line 411
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    udelay(10);
    //Line 412
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    udelay(10);
    //Line 413
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    udelay(10);
    //Line 414
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefd00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fc0001);
    udelay(10);
    //Line 415
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffe1fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f5f3);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xedf001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xe4ee0801);
    udelay(10);
    //Line 416
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefbf6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101f2);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa010201);
    udelay(10);
    //Line 417
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fafffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8000300);
    udelay(10);
    //Line 418
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000403fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0203);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000201);
    udelay(10);
    //Line 419
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0201ff);
    udelay(10);
    //Line 420
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff00);
    udelay(10);
    //Line 421
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030102fe);
    udelay(10);
    //Line 422
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    udelay(10);
    //Line 423
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 424
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    udelay(10);
    //Line 425
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    udelay(10);
    //Line 426
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    udelay(10);
    //Line 427
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    udelay(10);
    //Line 428
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffef2);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fafe);
    udelay(10);
    //Line 429
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07000006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffd06);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000105);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fbfe);
    udelay(10);
    //Line 430
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb000104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010301);
    udelay(10);
    //Line 431
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0204);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103ff00);
    udelay(10);
    //Line 432
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01f3);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050004fe);
    udelay(10);
    //Line 433
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe01fe);
    udelay(10);
    //Line 434
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020005);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502ff00);
    udelay(10);
    //Line 435
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201feff);
    udelay(10);
    //Line 436
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020201fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fe);
    udelay(10);
    //Line 437
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    udelay(10);
    //Line 438
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feff00);
    udelay(10);
    //Line 439
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff00);
    udelay(10);
    //Line 440
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000200);
    udelay(10);
    //Line 441
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0402);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0304fa02);
    udelay(10);
    //Line 442
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030005);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0802fdfd);
    udelay(10);
    //Line 443
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd01fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05020101);
    udelay(10);
    //Line 444
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0205fff9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fb01);
    udelay(10);
    //Line 445
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0201fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000001);
    udelay(10);
    //Line 446
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    udelay(10);
    //Line 447
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd03fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0301);
    udelay(10);
    //Line 448
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff00fe);
    udelay(10);
    //Line 449
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffb00);
    udelay(10);
    //Line 450
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffc00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fc01);
    udelay(10);
    //Line 451
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020000);
    udelay(10);
    //Line 452
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402feff);
    udelay(10);
    //Line 453
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ffff);
    udelay(10);
    //Line 454
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfb02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0007fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fbf9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcfe02);
    udelay(10);
    //Line 455
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0805fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feff09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04010100);
    udelay(10);
    //Line 456
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020401);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02feff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ff0301);
    udelay(10);
    //Line 457
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fffe);
    udelay(10);
    //Line 458
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfffe);
    udelay(10);
    //Line 459
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff02);
    udelay(10);
    //Line 460
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    udelay(10);
    //Line 461
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300fffd);
    udelay(10);
    //Line 462
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00ff02);
    udelay(10);
    //Line 463
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030200fc);
    udelay(10);
    //Line 464
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 465
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    udelay(10);
    //Line 466
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff01);
    udelay(10);
    //Line 467
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfa02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffdfa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fbfe);
    udelay(10);
    //Line 468
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020402fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fcfa);
    udelay(10);
    //Line 469
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010104);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fd04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020206);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fd02);
    udelay(10);
    //Line 470
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fdfc);
    udelay(10);
    //Line 471
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    udelay(10);
    //Line 472
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    udelay(10);
    //Line 473
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502ff00);
    udelay(10);
    //Line 474
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040101fd);
    udelay(10);
    //Line 475
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    udelay(10);
    //Line 476
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010102ff);
    udelay(10);
    //Line 477
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020200ff);
    udelay(10);
    //Line 478
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040200fd);
    udelay(10);
    //Line 479
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00ff);
    udelay(10);
    //Line 480
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf5fefb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5fdff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0405f7fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00faf902);
    udelay(10);
    //Line 481
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc01f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0205fe06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fffb);
    udelay(10);
    //Line 482
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000504);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07010200);
    udelay(10);
    //Line 483
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe00ff);
    udelay(10);
    //Line 484
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0006);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010004ff);
    udelay(10);
    //Line 485
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02030001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff030b);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ffff);
    udelay(10);
    //Line 486
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff00);
    udelay(10);
    //Line 487
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    udelay(10);
    //Line 488
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fe01);
    udelay(10);
    //Line 489
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    udelay(10);
    //Line 490
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    udelay(10);
    //Line 491
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    udelay(10);
    //Line 492
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    udelay(10);
    //Line 493
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010207);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fafe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010109);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050106fe);
    udelay(10);
    //Line 494
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fefef9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000008);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010402);
    udelay(10);
    //Line 495
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fef8fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01ff0a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fbff);
    udelay(10);
    //Line 496
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06feff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f8f6fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000005);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fef902);
    udelay(10);
    //Line 497
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefafb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030107);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003f903);
    udelay(10);
    //Line 498
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0201ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020102fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020001);
    udelay(10);
    //Line 499
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010401);
    udelay(10);
    //Line 500
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020101);
    udelay(10);
    //Line 501
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000203fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffffff);
    udelay(10);
    //Line 502
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000205);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000103);
    udelay(10);
    //Line 503
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010204);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    udelay(10);
    //Line 504
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff00ff);
    udelay(10);
    //Line 505
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    udelay(10);
    //Line 506
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fef8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0bfffdff);
    udelay(10);
    //Line 507
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0006fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff04f8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x09fdfe03);
    udelay(10);
    //Line 508
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00020c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    udelay(10);
    //Line 509
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe01);
    udelay(10);
    //Line 510
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefd04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ff00);
    udelay(10);
    //Line 511
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000203);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040200ff);
    udelay(10);
    //Line 512
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0003fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010200);
    udelay(10);
    //Line 513
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd0102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe0201);
    udelay(10);
    //Line 514
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fcfb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe01fe);
    udelay(10);
    //Line 515
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0104ff);
    udelay(10);
    //Line 516
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020200);
    udelay(10);
    //Line 517
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    udelay(10);
    //Line 518
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fffe);
    udelay(10);
    //Line 519
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fb06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcf0f9);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf5010c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeed0204);
    udelay(10);
    //Line 520
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fb00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffd03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fcfe);
    udelay(10);
    //Line 521
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ff0101);
    udelay(10);
    //Line 522
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700fc07);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfcfe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fa0503);
    udelay(10);
    //Line 523
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf801fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9f90403);
    udelay(10);
    //Line 524
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd00fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd040001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010102ec);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefcfc01);
    udelay(10);
    //Line 525
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010102fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff03fe);
    udelay(10);
    //Line 526
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030401f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fd01);
    udelay(10);
    //Line 527
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefc01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafdfc00);
    udelay(10);
    //Line 528
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fdff);
    udelay(10);
    //Line 529
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fff9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffcff);
    udelay(10);
    //Line 530
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefdf9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001ff);
    udelay(10);
    //Line 531
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfbfd03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0400);
    udelay(10);
    //Line 532
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200f307);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02edf8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf702020c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdf5ff01);
    udelay(10);
    //Line 533
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02f903);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040100f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfff403);
    udelay(10);
    //Line 534
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4f9fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00040400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd00f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd00ff);
    udelay(10);
    //Line 535
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020008);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000203fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0303);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fd0203);
    udelay(10);
    //Line 536
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff040200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302feff);
    udelay(10);
    //Line 537
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010202ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100f4);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fa);
    udelay(10);
    //Line 538
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffefc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010200);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fef7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    udelay(10);
    //Line 539
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fc0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    udelay(10);
    //Line 540
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe00fe);
    udelay(10);
    //Line 541
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0100fe);
    udelay(10);
    //Line 542
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000201fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff00ff);
    udelay(10);
    //Line 543
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    udelay(10);
    //Line 544
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000002fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020100);
    udelay(10);
    //Line 545
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fd05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffeef4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3f3ff09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe03fe);
    udelay(10);
    //Line 546
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0206fafd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fffefd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffd00);
    udelay(10);
    //Line 547
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fffefb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fcffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfbfeff);
    udelay(10);
    //Line 548
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0106);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffa03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fc0309);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc000601);
    udelay(10);
    //Line 549
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfdff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0005fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcfc05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfd0200);
    udelay(10);
    //Line 550
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010103ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030302ed);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdfefe);
    udelay(10);
    //Line 551
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb0504);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fdff00);
    udelay(10);
    //Line 552
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020101fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fcfe);
    udelay(10);
    //Line 553
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01feffff);
    udelay(10);
    //Line 554
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffeff);
    udelay(10);
    //Line 555
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    udelay(10);
    //Line 556
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0101);
    udelay(10);
    //Line 557
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fd0202);
    udelay(10);
    //Line 558
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050206ff);
    udelay(10);
    //Line 559
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x08010005);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000404ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030208);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030102fd);
    udelay(10);
    //Line 560
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0600fc04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02f8fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfef900);
    udelay(10);
    //Line 561
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0503fd01);
    udelay(10);
    //Line 562
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x07010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02040203);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fcff);
    udelay(10);
    //Line 563
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    udelay(10);
    //Line 564
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff02fd);
    udelay(10);
    //Line 565
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    udelay(10);
    //Line 566
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000300);
    udelay(10);
    //Line 567
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    udelay(10);
    //Line 568
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    udelay(10);
    //Line 569
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    udelay(10);
    //Line 570
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000000);
    udelay(10);
    //Line 571
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fff107);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01f7fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4f6fd08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3fb0300);
    udelay(10);
    //Line 572
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcfffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400fe03);
    udelay(10);
    //Line 573
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fafffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0301);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fe01);
    udelay(10);
    //Line 574
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe050003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fcff01);
    udelay(10);
    //Line 575
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffe06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04000000);
    udelay(10);
    //Line 576
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000301);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fafb);
    udelay(10);
    //Line 577
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fefe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0004ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffc02);
    udelay(10);
    //Line 578
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdffff00);
    udelay(10);
    //Line 579
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd0000);
    udelay(10);
    //Line 580
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffc03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfef90000);
    udelay(10);
    //Line 581
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfd02);
    udelay(10);
    //Line 582
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0100);
    udelay(10);
    //Line 583
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020102);
    udelay(10);
    //Line 584
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0001fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fcff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8010309);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030102fc);
    udelay(10);
    //Line 585
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf900fef9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0106f8f4);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb01f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcff04f6);
    udelay(10);
    //Line 586
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf80200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003fefd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfa01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fd);
    udelay(10);
    //Line 587
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9010103);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0006ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfb04fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafd0401);
    udelay(10);
    //Line 588
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffd03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020200);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fffe01);
    udelay(10);
    //Line 589
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0201f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfffe00);
    udelay(10);
    //Line 590
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fff6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefdff);
    udelay(10);
    //Line 591
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0202fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    udelay(10);
    //Line 592
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffc00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020400fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000002);
    udelay(10);
    //Line 593
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03020000);
    udelay(10);
    //Line 594
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03010101);
    udelay(10);
    //Line 595
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0000);
    udelay(10);
    //Line 596
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010102);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0001);
    udelay(10);
    //Line 597
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403f901);
    udelay(10);
    //Line 598
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05010003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000801fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0105fd06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0701fbfc);
    udelay(10);
    //Line 599
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0003fd08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fd00fe);
    udelay(10);
    //Line 600
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000202fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fefc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x060000ff);
    udelay(10);
    //Line 601
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff01fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020400);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0000);
    udelay(10);
    //Line 602
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefcff07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfc0102);
    udelay(10);
    //Line 603
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fc03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06feff00);
    udelay(10);
    //Line 604
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000401);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000400);
    udelay(10);
    //Line 605
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00feff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    udelay(10);
    //Line 606
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0001);
    udelay(10);
    //Line 607
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100ff);
    udelay(10);
    //Line 608
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0200);
    udelay(10);
    //Line 609
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000003fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020102ff);
    udelay(10);
    //Line 610
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfffbfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefff8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0f9ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fe05ff);
    udelay(10);
    //Line 611
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fdf3);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc02f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0105fb);
    udelay(10);
    //Line 612
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fefe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0108);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0101ff);
    udelay(10);
    //Line 613
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffc05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7ff0102);
    udelay(10);
    //Line 614
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf703fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0403ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff0302);
    udelay(10);
    //Line 615
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fd01);
    udelay(10);
    //Line 616
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04020102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020303);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030108);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fd01);
    udelay(10);
    //Line 617
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ffffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffaff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0200ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb02fd00);
    udelay(10);
    //Line 618
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010100);
    udelay(10);
    //Line 619
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03000004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0401);
    udelay(10);
    //Line 620
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefeff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010206);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff00);
    udelay(10);
    //Line 621
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000103fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fb0100);
    udelay(10);
    //Line 622
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfb0000);
    udelay(10);
    //Line 623
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe03fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffd04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050300ff);
    udelay(10);
    //Line 624
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f905fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfdff08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010300);
    udelay(10);
    //Line 625
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0206ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fefd00);
    udelay(10);
    //Line 626
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffcfd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbfafb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffef9fd);
    udelay(10);
    //Line 627
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01040002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030200fc);
    udelay(10);
    //Line 628
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfb03fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001f2);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020001);
    udelay(10);
    //Line 629
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010005);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0602ff00);
    udelay(10);
    //Line 630
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000203);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000200);
    udelay(10);
    //Line 631
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020201);
    udelay(10);
    //Line 632
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000200);
    udelay(10);
    //Line 633
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0106);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    udelay(10);
    //Line 634
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffdff);
    udelay(10);
    //Line 635
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020201);
    udelay(10);
    //Line 636
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff900fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0a08);
    udelay(10);
    //Line 637
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020409);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0209);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7000401);
    udelay(10);
    //Line 638
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200fefe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd03fdfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0202fe09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500fa00);
    udelay(10);
    //Line 639
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fffd06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02faff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefeff0c);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    udelay(10);
    //Line 640
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0700ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe04fc03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fd08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ff02);
    udelay(10);
    //Line 641
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000000);
    udelay(10);
    //Line 642
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0101);
    udelay(10);
    //Line 643
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff0002);
    udelay(10);
    //Line 644
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000002);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff02ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020104ff);
    udelay(10);
    //Line 645
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010102);
    udelay(10);
    //Line 646
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0000);
    udelay(10);
    //Line 647
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    udelay(10);
    //Line 648
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    udelay(10);
    //Line 649
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00030201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020200f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe02fd);
    udelay(10);
    //Line 650
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fa04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefb000b);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfaff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f10301);
    udelay(10);
    //Line 651
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe0e);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05feff00);
    udelay(10);
    //Line 652
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0203ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0601ff01);
    udelay(10);
    //Line 653
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000202);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefeff);
    udelay(10);
    //Line 654
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe010201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000202);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00ff);
    udelay(10);
    //Line 655
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000105);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0001);
    udelay(10);
    //Line 656
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000005);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ffff);
    udelay(10);
    //Line 657
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ffff);
    udelay(10);
    //Line 658
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeffff);
    udelay(10);
    //Line 659
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    udelay(10);
    //Line 660
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fe);
    udelay(10);
    //Line 661
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030001fd);
    udelay(10);
    //Line 662
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdf6fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfa01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeefdff08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf2fc0501);
    udelay(10);
    //Line 663
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfe05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbf9fbff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff05f3);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020502);
    udelay(10);
    //Line 664
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0101f5);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000302fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00f1);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf20100ff);
    udelay(10);
    //Line 665
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0401f7);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050305fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfe03);
    udelay(10);
    //Line 666
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfe01f8);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe020201);
    udelay(10);
    //Line 667
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fb02);
    udelay(10);
    //Line 668
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100ff);
    udelay(10);
    //Line 669
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010001);
    udelay(10);
    //Line 670
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0004);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020000);
    udelay(10);
    //Line 671
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff01);
    udelay(10);
    //Line 672
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    udelay(10);
    //Line 673
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010304);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00ff00);
    udelay(10);
    //Line 674
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ffff);
    udelay(10);
    //Line 675
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefffe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fd0205);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010301f2);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000fa);
    udelay(10);
    //Line 676
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe030307);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcff00);
    udelay(10);
    //Line 677
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010103);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00ff09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0200);
    udelay(10);
    //Line 678
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010203);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0201);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd020401);
    udelay(10);
    //Line 679
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020003ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefe00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06fe04ff);
    udelay(10);
    //Line 680
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefc08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffc0301);
    udelay(10);
    //Line 681
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe02fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x090100fc);
    udelay(10);
    //Line 682
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000003);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfe00);
    udelay(10);
    //Line 683
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x040000fe);
    udelay(10);
    //Line 684
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000200);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    udelay(10);
    //Line 685
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x050300fe);
    udelay(10);
    //Line 686
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    udelay(10);
    //Line 687
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010101fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020100ff);
    udelay(10);
    //Line 688
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff05);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fefd04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fc0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01fd);
    udelay(10);
    //Line 689
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fef9);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f702ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefa0002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0501);
    udelay(10);
    //Line 690
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fd04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020103fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01ff07);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fd0101);
    udelay(10);
    //Line 691
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fbff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe00ef);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x06ffff01);
    udelay(10);
    //Line 692
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010003ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010300f9);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0c01fd00);
    udelay(10);
    //Line 693
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fb03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfafe01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0001);
    udelay(10);
    //Line 694
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfeff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fb00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff02ff);
    udelay(10);
    //Line 695
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030302ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502ffff);
    udelay(10);
    //Line 696
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010201fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02000001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ff01);
    udelay(10);
    //Line 697
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0300ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fe00);
    udelay(10);
    //Line 698
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000300fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff01);
    udelay(10);
    //Line 699
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000102ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010302fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0403ffff);
    udelay(10);
    //Line 700
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff01);
    udelay(10);
    //Line 701
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0502ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fb03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04feff09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffcf902);
    udelay(10);
    //Line 702
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0109);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0102);
    udelay(10);
    //Line 703
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffc04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd020a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0201);
    udelay(10);
    //Line 704
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030101f6);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fb0102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001f5);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fffdfd);
    udelay(10);
    //Line 705
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0204);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0009);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0302);
    udelay(10);
    //Line 706
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdfd04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020003);
    udelay(10);
    //Line 707
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fbfd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffc04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000203);
    udelay(10);
    //Line 708
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fdff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd00fc02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fc0001);
    udelay(10);
    //Line 709
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe03ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff03);
    udelay(10);
    //Line 710
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff00);
    udelay(10);
    //Line 711
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010001fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000102);
    udelay(10);
    //Line 712
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010200ff);
    udelay(10);
    //Line 713
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd010001);
    udelay(10);
    //Line 714
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0005f3fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfaf3fe05);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0404ff);
    udelay(10);
    //Line 715
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fdf5);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x080502fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfffb02);
    udelay(10);
    //Line 716
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fafffb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcff04);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6fe0500);
    udelay(10);
    //Line 717
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd0004);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00060102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfefd03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0103ff);
    udelay(10);
    //Line 718
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffd01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010202fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030600ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fa00);
    udelay(10);
    //Line 719
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00f90300);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010304f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fd01);
    udelay(10);
    //Line 720
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffefd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fffe02);
    udelay(10);
    //Line 721
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02020100);
    udelay(10);
    //Line 722
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fe0100);
    udelay(10);
    //Line 723
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff00);
    udelay(10);
    //Line 724
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffe03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0000);
    udelay(10);
    //Line 725
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01020002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    udelay(10);
    //Line 726
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffffff);
    udelay(10);
    //Line 727
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fdf6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xef00ff04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0102fe);
    udelay(10);
    //Line 728
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff00fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8f905f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf70207fd);
    udelay(10);
    //Line 729
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9fa00f1);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc020201);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffcf3);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8000100);
    udelay(10);
    //Line 730
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0102f8);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0103ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03050000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfe00);
    udelay(10);
    //Line 731
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05ffff03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fcfb);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0202fa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0503fcfe);
    udelay(10);
    //Line 732
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002fafd);
    udelay(10);
    //Line 733
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fcfeff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc01f8fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0301fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301fb02);
    udelay(10);
    //Line 734
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fd01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020401);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefc0001);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc02ff);
    udelay(10);
    //Line 735
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd000100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000100);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff04ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010200);
    udelay(10);
    //Line 736
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd0100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000100);
    udelay(10);
    //Line 737
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03ff01fd);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffffff00);
    udelay(10);
    //Line 738
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010002);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe000100);
    udelay(10);
    //Line 739
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff02ff01);
    udelay(10);
    //Line 740
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0200f0fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001fdf8);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf0f8fe09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1f30501);
    udelay(10);
    //Line 741
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffffb02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff01f7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x030306f6);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfafd0101);
    udelay(10);
    //Line 742
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffa0300);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf40303fd);
    udelay(10);
    //Line 743
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0000fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0501fc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0301ff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc00fffe);
    udelay(10);
    //Line 744
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd01ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000302fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fc0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefd0400);
    udelay(10);
    //Line 745
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fe0006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000302);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe01fe00);
    udelay(10);
    //Line 746
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd0001);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020003);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff0a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8ff03ff);
    udelay(10);
    //Line 747
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fffe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0002ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0201fe01);
    udelay(10);
    //Line 748
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010202);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0102ff);
    udelay(10);
    //Line 749
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff020002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe0201);
    udelay(10);
    //Line 750
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fffdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000103);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101ff00);
    udelay(10);
    //Line 751
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000100ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000101);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffeff00);
    udelay(10);
    //Line 752
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01000102);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfcfd0000);
    udelay(10);
    //Line 753
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff02fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefefefc);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfb0504);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8020501);
    udelay(10);
    //Line 754
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fd02fa);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd04f7);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa0007fb);
    udelay(10);
    //Line 755
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb00fffd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020105fe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fffefa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf9000100);
    udelay(10);
    //Line 756
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf901ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030302);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03fffffc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7fc0201);
    udelay(10);
    //Line 757
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fcfffc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd030004);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000304f4);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf802fe01);
    udelay(10);
    //Line 758
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05fe0101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdff0104);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe05fffa);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc02fd00);
    udelay(10);
    //Line 759
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefe01);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fe01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff03fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02fdff);
    udelay(10);
    //Line 760
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fcfe03);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fdfefe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0201fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000005ff);
    udelay(10);
    //Line 761
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd02fe02);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff02fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010003fd);
    udelay(10);
    //Line 762
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0100fe00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00ff08);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0101fe);
    udelay(10);
    //Line 763
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe02ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff0600);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfff90301);
    udelay(10);
    //Line 764
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefe00fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fbff03);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01030100);
    udelay(10);
    //Line 765
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fe00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ffff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0101fefb);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0100ff);
    udelay(10);
    //Line 766
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe00f404);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ebf6);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xeaebff06);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f90400);
    udelay(10);
    //Line 767
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfb0001fd);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0103fbf7);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fffffe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000fd01);
    udelay(10);
    //Line 768
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf5ffff00);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfc0003);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfdfe0300);
    udelay(10);
    //Line 769
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbfd0006);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff00fd02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff00ff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffe0300);
    udelay(10);
    //Line 770
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff01ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0102ff02);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00020203);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfbff00ff);
    udelay(10);
    //Line 771
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000101fc);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020400fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefdff02);
    udelay(10);
    //Line 772
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xffff01fe);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ffff01);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010002fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfa01feff);
    udelay(10);
    //Line 773
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010102);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x010300fe);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02ff0001);
    udelay(10);
    //Line 774
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000101);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001feff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00010100);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    udelay(10);
    //Line 775
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff0000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0001ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02010000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02fefe01);
    udelay(10);
    //Line 776
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfeff0100);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01010001);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefffe02);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00ff0201);
    udelay(10);
    //Line 777
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000001ff);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000000ff);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000200fc);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0400feff);
    udelay(10);
    //Line 778
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01ff0000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0000ff00);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff000002);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff010000);
    udelay(10);
    //Line 779
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xedf2ef0c);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d0e0b0c);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xec0a0b0a);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0a090b0b);
    udelay(10);
    //Line 780
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf1eff4ed);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d0b08f0);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0c080ff1);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf3f40df3);
    udelay(10);
    //Line 781
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x110f0df1);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7f3f4f3);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x14f5f4f2);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f7f4f9);
    udelay(10);
    //Line 782
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0d120c12);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf4f7f511);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f4f214);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0c0cf30c);
    udelay(10);
    //Line 783
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf8fbf903);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0708fd07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0508fe09);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfd080104);
    udelay(10);
    //Line 784
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff030105);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0206fdfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0500ff03);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfefafd08);
    udelay(10);
    //Line 785
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00fdf403);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xff01fd07);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x05040604);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf7f701ff);
    udelay(10);
    //Line 786
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0af7f40a);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01fbfcfe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0402fa01);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x020a040b);
    udelay(10);
    //Line 787
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0401fe04);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fe0afe);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0302ffff);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04fefc09);
    udelay(10);
    //Line 788
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfc0204fb);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfe0a0600);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x0902f702);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x03f607fd);
    udelay(10);
    //Line 789
    rtd_outl(SMART_VAD_svad_coef_sram_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);
    rtd_outl(SMART_VAD_svad_coef_sram_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x12f00000);
    udelay(10);
    
    rtd_outl(SMART_VAD_svad_coef_sram_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x00000000);

    //mean, std settings
    rtd_outl(SMART_VAD_svad_dmic_3_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x04edf203);
    rtd_outl(SMART_VAD_svad_dmic_4_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xf6f2f8ed);
    rtd_outl(SMART_VAD_svad_dmic_5_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01f3fef4);
    rtd_outl(SMART_VAD_svad_dmic_6_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x000019f8);
    rtd_outl(SMART_VAD_svad_dmic_7_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x1214141a);
    rtd_outl(SMART_VAD_svad_dmic_8_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x13121313);
    rtd_outl(SMART_VAD_svad_dmic_9_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x17161514);

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
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf20a0bf0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfaf0fcfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef9f6ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0bf4f9fb);
    udelay(10);
    //Line 2
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf7070bf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fdf9fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfbf5f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0bfcfafb);
    udelay(10);
    //Line 3
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf80507fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103f7ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fff500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0cfdfdfc);
    udelay(10);
    //Line 4
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0408ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0304fa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0200f806);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fcffff);
    udelay(10);
    //Line 5
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd010600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fe0102);
    udelay(10);
    //Line 6
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0102);
    udelay(10);
    //Line 7
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa010202);
    udelay(10);
    //Line 8
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030900);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5fe0402);
    udelay(10);
    //Line 9
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfeff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff0aff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb03fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7000403);
    udelay(10);
    //Line 10
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010405);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc000903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103fc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6f90302);
    udelay(10);
    //Line 11
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02080601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafcf9f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf6f701fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fff4f5);
    udelay(10);
    //Line 12
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02040402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9fcf4f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5fa02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fff5f5);
    udelay(10);
    //Line 13
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03030102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcffeefa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafa01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0501f2f4);
    udelay(10);
    //Line 14
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0304ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffff5fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfa01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe03f3f2);
    udelay(10);
    //Line 15
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0201fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffafffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200faf6);
    udelay(10);
    //Line 16
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd06fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fdfa);
    udelay(10);
    //Line 17
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfcfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0106fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fffd);
    udelay(10);
    //Line 18
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fbfd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010001ff);
    udelay(10);
    //Line 19
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fbfc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0404fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04000403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002ff01);
    udelay(10);
    //Line 20
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fcfd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0402f602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc05fcfd);
    udelay(10);
    //Line 21
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0e0dfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfb08f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf7f4fafa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x11fbfffb);
    udelay(10);
    //Line 22
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0800f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdf808f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf5f7fef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fbfef9);
    udelay(10);
    //Line 23
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00020400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfaf90bf8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafb00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fe0000);
    udelay(10);
    //Line 24
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01040304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0509ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010206);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06010703);
    udelay(10);
    //Line 25
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0302ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd050400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010507);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010706);
    udelay(10);
    //Line 26
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03030105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01030201);
    udelay(10);
    //Line 27
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfb0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfe02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00030202);
    udelay(10);
    //Line 28
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8ff01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010100);
    udelay(10);
    //Line 29
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb010001);
    udelay(10);
    //Line 30
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000304ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8fa03fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafd00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfefefc);
    udelay(10);
    //Line 31
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fdff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04070b0c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0608080c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb060d0b);
    udelay(10);
    //Line 32
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe030303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff04fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0401);
    udelay(10);
    //Line 33
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0003fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0300);
    udelay(10);
    //Line 34
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfd07fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefb02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010301);
    udelay(10);
    //Line 35
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdff03fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010302);
    udelay(10);
    //Line 36
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0003ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000104ff);
    udelay(10);
    //Line 37
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0103ff);
    udelay(10);
    //Line 38
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fefe);
    udelay(10);
    //Line 39
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020006fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000301);
    udelay(10);
    //Line 40
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010201fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfd07fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0301);
    udelay(10);
    //Line 41
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x09f4f905);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0709fe0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x090a0806);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf3080207);
    udelay(10);
    //Line 42
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfef9fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001f703);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf304ffff);
    udelay(10);
    //Line 43
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fbfd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0204ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf20301fd);
    udelay(10);
    //Line 44
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfffd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04040103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf300fd01);
    udelay(10);
    //Line 45
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8020400);
    udelay(10);
    //Line 46
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0503fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfe0102);
    udelay(10);
    //Line 47
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0104fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0402fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffc0304);
    udelay(10);
    //Line 48
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0103fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0004fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fb0201);
    udelay(10);
    //Line 49
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0303ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00f804);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0503fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fa0203);
    udelay(10);
    //Line 50
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0605ef03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0200fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fe01);
    udelay(10);
    //Line 51
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04ff01fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fdfefd);
    udelay(10);
    //Line 52
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0308);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05050000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0506);
    udelay(10);
    //Line 53
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030403ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01030303);
    udelay(10);
    //Line 54
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff04ff);
    udelay(10);
    //Line 55
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03feffff);
    udelay(10);
    //Line 56
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0003f501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fdffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030000fe);
    udelay(10);
    //Line 57
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01f300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000100fd);
    udelay(10);
    //Line 58
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd00ea04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc01fc01);
    udelay(10);
    //Line 59
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000dd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010401fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa04f6ff);
    udelay(10);
    //Line 60
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0206e000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fc0502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf803f1fb);
    udelay(10);
    //Line 61
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff03fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffafe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fcfd);
    udelay(10);
    //Line 62
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfefb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fdfe);
    udelay(10);
    //Line 63
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffefe);
    udelay(10);
    //Line 64
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000200fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffffe);
    udelay(10);
    //Line 65
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0300ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000201);
    udelay(10);
    //Line 66
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffdfdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030001fc);
    udelay(10);
    //Line 67
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03010300);
    udelay(10);
    //Line 68
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04010302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08000201);
    udelay(10);
    //Line 69
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03030000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050101fe);
    udelay(10);
    //Line 70
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05feff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0301fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04ff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0605fefe);
    udelay(10);
    //Line 71
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fdfd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02040009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00060103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa040102);
    udelay(10);
    //Line 72
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fa00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb04feff);
    udelay(10);
    //Line 73
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0501ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0104ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd01ff00);
    udelay(10);
    //Line 74
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe04fe00);
    udelay(10);
    //Line 75
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020102);
    udelay(10);
    //Line 76
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0002ff);
    udelay(10);
    //Line 77
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fffeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc000500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff03fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0202);
    udelay(10);
    //Line 78
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0005fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfc0d01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefe0201);
    udelay(10);
    //Line 79
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0404fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf9fd09fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfffcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefd0503);
    udelay(10);
    //Line 80
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf90503fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8fb1600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc01fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffb0700);
    udelay(10);
    //Line 81
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0afe000d);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0e1b0511);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x10100718);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa09130f);
    udelay(10);
    //Line 82
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02faf801);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefbf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefdfbfa);
    udelay(10);
    //Line 83
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfcfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd03fef9);
    udelay(10);
    //Line 84
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0100fe);
    udelay(10);
    //Line 85
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefd0001);
    udelay(10);
    //Line 86
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff03);
    udelay(10);
    //Line 87
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc00fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc00fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0201);
    udelay(10);
    //Line 88
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fc0202);
    udelay(10);
    //Line 89
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfffefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000103);
    udelay(10);
    //Line 90
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfdffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0201f9fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fefe01);
    udelay(10);
    //Line 91
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06edf407);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0502fefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7060302);
    udelay(10);
    //Line 92
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbf6f900);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000406);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04000604);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000603);
    udelay(10);
    //Line 93
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefbfc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0503);
    udelay(10);
    //Line 94
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfcfdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfe0aff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff03fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffd0700);
    udelay(10);
    //Line 95
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020104fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0603);
    udelay(10);
    //Line 96
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04020404);
    udelay(10);
    //Line 97
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0403);
    udelay(10);
    //Line 98
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040101ff);
    udelay(10);
    //Line 99
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fefe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06000101);
    udelay(10);
    //Line 100
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fffc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0300ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05010301);
    udelay(10);
    //Line 101
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0702fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0402fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfefe03);
    udelay(10);
    //Line 102
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0300ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050300ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000203);
    udelay(10);
    //Line 103
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff01);
    udelay(10);
    //Line 104
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcfe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fe00);
    udelay(10);
    //Line 105
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0401fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020001);
    udelay(10);
    //Line 106
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0501fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefd0405);
    udelay(10);
    //Line 107
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fdfefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0301ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202fd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00020304);
    udelay(10);
    //Line 108
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0403f206);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0303fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ffff);
    udelay(10);
    //Line 109
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0402e001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0401ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01f6ff);
    udelay(10);
    //Line 110
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fdfd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0905e1ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fe0502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe04f3fd);
    udelay(10);
    //Line 111
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fafcfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfaeefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcff04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb02f9fb);
    udelay(10);
    //Line 112
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fbfa05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff03f800);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb010101);
    udelay(10);
    //Line 113
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05faf806);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefef903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb010406);
    udelay(10);
    //Line 114
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fbfb06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe04f400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9020102);
    udelay(10);
    //Line 115
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03faf904);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00f001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7010000);
    udelay(10);
    //Line 116
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02f8fa04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fff001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf801ffff);
    udelay(10);
    //Line 117
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc02f0ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5fffdff);
    udelay(10);
    //Line 118
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fbfc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffeefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0004ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf804fcff);
    udelay(10);
    //Line 119
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05f9fc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0302e401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fb0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf501f9fd);
    udelay(10);
    //Line 120
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03f9fc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0702dbfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefc06ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb08f4f9);
    udelay(10);
    //Line 121
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08fdff0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03070009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0005080a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7060806);
    udelay(10);
    //Line 122
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff030104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd040303);
    udelay(10);
    //Line 123
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0002ff);
    udelay(10);
    //Line 124
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfc03fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb00fdfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffffff01);
    udelay(10);
    //Line 125
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfc0afc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfc01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010103ff);
    udelay(10);
    //Line 126
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0008fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0002);
    udelay(10);
    //Line 127
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefe07fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfbff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300ffff);
    udelay(10);
    //Line 128
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfd07fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe01ff);
    udelay(10);
    //Line 129
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfd07ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0300);
    udelay(10);
    //Line 130
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf8001300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc030102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000804);
    udelay(10);
    //Line 131
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fcfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02070807);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0405ff0b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8020c0a);
    udelay(10);
    //Line 132
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fefe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00040304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02080104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc030608);
    udelay(10);
    //Line 133
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020404);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00050302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfe0605);
    udelay(10);
    //Line 134
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffffdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0201);
    udelay(10);
    //Line 135
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffefffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0203);
    udelay(10);
    //Line 136
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00feff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefffe01);
    udelay(10);
    //Line 137
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfe01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0100);
    udelay(10);
    //Line 138
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff03ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfe06ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0100);
    udelay(10);
    //Line 139
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0405ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfefeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc010400);
    udelay(10);
    //Line 140
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0506fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfe0c01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf902ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff00ff);
    udelay(10);
    //Line 141
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02eef304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02040a01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5050408);
    udelay(10);
    //Line 142
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04f5f802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0706);
    udelay(10);
    //Line 143
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fafb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010405);
    udelay(10);
    //Line 144
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfcfdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000204fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe0401);
    udelay(10);
    //Line 145
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffdfcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020007ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fdff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fe0402);
    udelay(10);
    //Line 146
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020105fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fdfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05ff0200);
    udelay(10);
    //Line 147
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdfe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fb01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000001);
    udelay(10);
    //Line 148
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fafb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff03fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff02fe01);
    udelay(10);
    //Line 149
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00f8f902);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010200);
    udelay(10);
    //Line 150
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00f8f901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0202feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000200);
    udelay(10);
    //Line 151
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x070c0b0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05090007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6fe0e0c);
    udelay(10);
    //Line 152
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020503);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02030205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0604);
    udelay(10);
    //Line 153
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fdfefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000100ff);
    udelay(10);
    //Line 154
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffa00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfbfefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fdfefd);
    udelay(10);
    //Line 155
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfef902fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdf800fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fefdfc);
    udelay(10);
    //Line 156
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfc00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0103);
    udelay(10);
    //Line 157
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc030301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff020302);
    udelay(10);
    //Line 158
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010304);
    udelay(10);
    //Line 159
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd000400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020102);
    udelay(10);
    //Line 160
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00030301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafc0801);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000503);
    udelay(10);
    //Line 161
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03090209);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0808ff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000709);
    udelay(10);
    //Line 162
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0104fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010203);
    udelay(10);
    //Line 163
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd030200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0400fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fa01ff);
    udelay(10);
    //Line 164
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0100fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100f4fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fc00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fefe01);
    udelay(10);
    //Line 165
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02feedff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fdfdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fafe);
    udelay(10);
    //Line 166
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ee00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefefefc);
    udelay(10);
    //Line 167
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffef1fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fc01);
    udelay(10);
    //Line 168
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0201ef01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0601ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00fd00);
    udelay(10);
    //Line 169
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0305e5ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa03faff);
    udelay(10);
    //Line 170
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04ff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0504e301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fb0603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0106f3f8);
    udelay(10);
    //Line 171
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fe0b01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0506);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf904030a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020903);
    udelay(10);
    //Line 172
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfafffcfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdf80000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc03fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0505fdfd);
    udelay(10);
    //Line 173
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffa0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe030103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04010100);
    udelay(10);
    //Line 174
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefb0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06000100);
    udelay(10);
    //Line 175
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfef90100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010200fd);
    udelay(10);
    //Line 176
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfef9ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf803ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfaff01fb);
    udelay(10);
    //Line 177
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0107fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fd0704);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6f10300);
    udelay(10);
    //Line 178
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f80102);
    udelay(10);
    //Line 179
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fcfe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fd0201);
    udelay(10);
    //Line 180
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fe0001);
    udelay(10);
    //Line 181
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fd0101);
    udelay(10);
    //Line 182
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0300ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03ff0302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fb0300);
    udelay(10);
    //Line 183
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08050705);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0402fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fc0704);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf5fd0608);
    udelay(10);
    //Line 184
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fefcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fffe);
    udelay(10);
    //Line 185
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefc0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fefe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0102fcfe);
    udelay(10);
    //Line 186
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf7faf702);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfe02fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0202fafe);
    udelay(10);
    //Line 187
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfef303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030106fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffd01);
    udelay(10);
    //Line 188
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfff604);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0401fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0900fc01);
    udelay(10);
    //Line 189
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xe9fe0302);
    udelay(10);
    //Line 190
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01040000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff05fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfafeff02);
    udelay(10);
    //Line 191
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc04f901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01feffff);
    udelay(10);
    //Line 192
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0004fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb07fdfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fbf9);
    udelay(10);
    //Line 193
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010207fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc080100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040100ff);
    udelay(10);
    //Line 194
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x060407f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05fbfe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa040800);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb020100);
    udelay(10);
    //Line 195
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfffefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fcfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa01fefc);
    udelay(10);
    //Line 196
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04020300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02020301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9000305);
    udelay(10);
    //Line 197
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000000);
    udelay(10);
    //Line 198
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0201fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa00ff03);
    udelay(10);
    //Line 199
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf500fffe);
    udelay(10);
    //Line 200
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0403ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb0000ff);
    udelay(10);
    //Line 201
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03060502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03f9fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8ff0406);
    udelay(10);
    //Line 202
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fa0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000102);
    udelay(10);
    //Line 203
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0401feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefaff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000003);
    udelay(10);
    //Line 204
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefbffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb01ff02);
    udelay(10);
    //Line 205
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfef60101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6000001);
    udelay(10);
    //Line 206
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fe02f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfcff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8f406ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf3020000);
    udelay(10);
    //Line 207
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf6f1f8fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf203fef5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103fcf0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0a04f0f1);
    udelay(10);
    //Line 208
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01030101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04ff02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff0002);
    udelay(10);
    //Line 209
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010205ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0201);
    udelay(10);
    //Line 210
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010005fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0100);
    udelay(10);
    //Line 211
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000303ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfaff02ff);
    udelay(10);
    //Line 212
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0303ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc0600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfe0302);
    udelay(10);
    //Line 213
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfbfff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafd00fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fef7f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa06faf7);
    udelay(10);
    //Line 214
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000f06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05feff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xeffe06ff);
    udelay(10);
    //Line 215
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0100ff);
    udelay(10);
    //Line 216
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0501ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff01ff);
    udelay(10);
    //Line 217
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0101);
    udelay(10);
    //Line 218
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfefd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0405fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0502fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0002);
    udelay(10);
    //Line 219
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0a060b04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0202ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fb0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf4ff0603);
    udelay(10);
    //Line 220
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd030400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa0300fd);
    udelay(10);
    //Line 221
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf802fcfd);
    udelay(10);
    //Line 222
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000003fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fcff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7ff0202);
    udelay(10);
    //Line 223
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7ff0002);
    udelay(10);
    //Line 224
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7020000);
    udelay(10);
    //Line 225
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0203f5f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0008fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf601fe00);
    udelay(10);
    //Line 226
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0500fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfa0705);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6000504);
    udelay(10);
    //Line 227
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000203fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefc0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff0101);
    udelay(10);
    //Line 228
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe0302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000400);
    udelay(10);
    //Line 229
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fe01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefd03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010003);
    udelay(10);
    //Line 230
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfe05ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000001);
    udelay(10);
    //Line 231
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030100f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x07feff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04f905ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6fd0002);
    udelay(10);
    //Line 232
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06030602);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fb0501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0402);
    udelay(10);
    //Line 233
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fd0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fd0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0101ff);
    udelay(10);
    //Line 234
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc01fd01);
    udelay(10);
    //Line 235
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fbff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x08ff0400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffff00);
    udelay(10);
    //Line 236
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffafe0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafffdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fe03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffffe);
    udelay(10);
    //Line 237
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0400ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0404ffff);
    udelay(10);
    //Line 238
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0400fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 239
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0500ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060001ff);
    udelay(10);
    //Line 240
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd04fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300ff00);
    udelay(10);
    //Line 241
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb01fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff04f8ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fffe);
    udelay(10);
    //Line 242
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd05ff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030a08ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0a07f703);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffd02);
    udelay(10);
    //Line 243
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0700);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0003fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0101);
    udelay(10);
    //Line 244
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff03fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd060001);
    udelay(10);
    //Line 245
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fcfffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffc00ff);
    udelay(10);
    //Line 246
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02f90101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff04fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb060100);
    udelay(10);
    //Line 247
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04f702fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfbf700);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcff07fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000200);
    udelay(10);
    //Line 248
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07f502f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfaf5f601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf6fb0afd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf70704fc);
    udelay(10);
    //Line 249
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc02fbfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9f6ff00);
    udelay(10);
    //Line 250
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0102fcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcf200ff);
    udelay(10);
    //Line 251
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0203fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05fd04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f30203);
    udelay(10);
    //Line 252
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0202fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f400ff);
    udelay(10);
    //Line 253
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0102fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000400fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fefb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01f60202);
    udelay(10);
    //Line 254
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc00fb05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0307fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x06fdf502);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02f7ff04);
    udelay(10);
    //Line 255
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000305);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfef50607);
    udelay(10);
    //Line 256
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefffd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfffffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fc0000);
    udelay(10);
    //Line 257
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fc0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfa0300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefd01ff);
    udelay(10);
    //Line 258
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fcff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd0502fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fe00fe);
    udelay(10);
    //Line 259
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe02fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff030101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050104ff);
    udelay(10);
    //Line 260
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0302fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x08010100);
    udelay(10);
    //Line 261
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfafe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x1005fdfe);
    udelay(10);
    //Line 262
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf8fef9ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd0101fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fbfc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201fbfd);
    udelay(10);
    //Line 263
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0302feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0901fe01);
    udelay(10);
    //Line 264
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0400ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05ff0002);
    udelay(10);
    //Line 265
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04ff0003);
    udelay(10);
    //Line 266
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fffe04);
    udelay(10);
    //Line 267
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000001);
    udelay(10);
    //Line 268
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 269
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 270
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000000);
    udelay(10);
    //Line 271
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01ff00);
    udelay(10);
    //Line 272
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfefffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020102fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fe05);
    udelay(10);
    //Line 273
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd070002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfdf7ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf400f7fe);
    udelay(10);
    //Line 274
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040301fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe04fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb01fafe);
    udelay(10);
    //Line 275
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040302fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040201fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc01fefe);
    udelay(10);
    //Line 276
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010301f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fe01);
    udelay(10);
    //Line 277
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe02fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010002);
    udelay(10);
    //Line 278
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04030303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fe01);
    udelay(10);
    //Line 279
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000105f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06fd03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe06f1f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04030404);
    udelay(10);
    //Line 280
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfffaff);
    udelay(10);
    //Line 281
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010002);
    udelay(10);
    //Line 282
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ff01);
    udelay(10);
    //Line 283
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000103);
    udelay(10);
    //Line 284
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0002ff);
    udelay(10);
    //Line 285
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0dff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0302fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07ff0301);
    udelay(10);
    //Line 286
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fdff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fdfe00);
    udelay(10);
    //Line 287
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05f7fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fd0100);
    udelay(10);
    //Line 288
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07fafd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fdfdfe);
    udelay(10);
    //Line 289
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07fcfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fcfc00);
    udelay(10);
    //Line 290
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x18fefb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0205fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07fbff02);
    udelay(10);
    //Line 291
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefffdf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf802ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0701ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfffc02);
    udelay(10);
    //Line 292
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x06000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020206);
    udelay(10);
    //Line 293
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x05fefc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffdfc01);
    udelay(10);
    //Line 294
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0301ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02050001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fe02);
    udelay(10);
    //Line 295
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeffff01);
    udelay(10);
    //Line 296
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x040000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0201fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 297
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf6010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fb0f02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0100ff);
    udelay(10);
    //Line 298
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fcfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010200);
    udelay(10);
    //Line 299
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff03fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000100);
    udelay(10);
    //Line 300
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020002fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0101);
    udelay(10);
    //Line 301
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000101fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0004fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000002ff);
    udelay(10);
    //Line 302
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fffefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0303f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0602fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0100);
    udelay(10);
    //Line 303
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff020103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010002f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0201ff);
    udelay(10);
    //Line 304
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0201fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fefcfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa02fe02);
    udelay(10);
    //Line 305
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffe01f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefcf8fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc010103);
    udelay(10);
    //Line 306
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffd0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfbfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000103);
    udelay(10);
    //Line 307
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0701);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010108);
    udelay(10);
    //Line 308
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05020207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02030603);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc020203);
    udelay(10);
    //Line 309
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05faf904);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffafc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf0f800f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07ec0300);
    udelay(10);
    //Line 310
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff9f901);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfefc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fc0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00f4feff);
    udelay(10);
    //Line 311
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020200);
    udelay(10);
    //Line 312
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010001ff);
    udelay(10);
    //Line 313
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0001);
    udelay(10);
    //Line 314
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0100);
    udelay(10);
    //Line 315
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffeff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfc08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f803fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fff6fe);
    udelay(10);
    //Line 316
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa030102);
    udelay(10);
    //Line 317
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf5040007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fb0008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffd07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9020200);
    udelay(10);
    //Line 318
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf6080207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fafe08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffe08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf80306fc);
    udelay(10);
    //Line 319
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf4060203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01faff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf90204fa);
    udelay(10);
    //Line 320
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf40c04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffafc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fa04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf80203f7);
    udelay(10);
    //Line 321
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff01f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00fafa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000104);
    udelay(10);
    //Line 322
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fff802);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefc05fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfffe03);
    udelay(10);
    //Line 323
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd01f803);
    udelay(10);
    //Line 324
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010101);
    udelay(10);
    //Line 325
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020103ff);
    udelay(10);
    //Line 326
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000404);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfdfe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000006fc);
    udelay(10);
    //Line 327
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0300f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020805fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0404fb07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff030103);
    udelay(10);
    //Line 328
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fbffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff03fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe03ff);
    udelay(10);
    //Line 329
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff02ff);
    udelay(10);
    //Line 330
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff02ff);
    udelay(10);
    //Line 331
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000100);
    udelay(10);
    //Line 332
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020001fe);
    udelay(10);
    //Line 333
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffdfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010303fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301050b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000001);
    udelay(10);
    //Line 334
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050201ff);
    udelay(10);
    //Line 335
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe02fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000000);
    udelay(10);
    //Line 336
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020002fe);
    udelay(10);
    //Line 337
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfcff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe01fd);
    udelay(10);
    //Line 338
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffdfd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0101);
    udelay(10);
    //Line 339
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fdf611);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafffc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00f90606);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfbfcfc);
    udelay(10);
    //Line 340
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010201);
    udelay(10);
    //Line 341
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 342
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000101fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010000);
    udelay(10);
    //Line 343
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000001);
    udelay(10);
    //Line 344
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000201);
    udelay(10);
    //Line 345
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000305);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfdfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06010600);
    udelay(10);
    //Line 346
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200010b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdfe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07ffff00);
    udelay(10);
    //Line 347
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000305);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fafa03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefc01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fd02);
    udelay(10);
    //Line 348
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfffb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fffa03);
    udelay(10);
    //Line 349
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd010203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0302ff06);
    udelay(10);
    //Line 350
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff0505);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff06ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02030006);
    udelay(10);
    //Line 351
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01090afc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0105fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0403fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0507fe02);
    udelay(10);
    //Line 352
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0505fc01);
    udelay(10);
    //Line 353
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010107fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0302fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0103ff00);
    udelay(10);
    //Line 354
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff05fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff03fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0303fe00);
    udelay(10);
    //Line 355
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffdfdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0102feff);
    udelay(10);
    //Line 356
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fbfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0203fe03);
    udelay(10);
    //Line 357
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfff901ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fcfc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfeff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fbfd01);
    udelay(10);
    //Line 358
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffdfd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010707);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffd03);
    udelay(10);
    //Line 359
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000001);
    udelay(10);
    //Line 360
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0101);
    udelay(10);
    //Line 361
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0102);
    udelay(10);
    //Line 362
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000500);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ffff04);
    udelay(10);
    //Line 363
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfcfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfd0501);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00fe01);
    udelay(10);
    //Line 364
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000103ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03030305);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000001ff);
    udelay(10);
    //Line 365
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02040207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200feff);
    udelay(10);
    //Line 366
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03030107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402fdff);
    udelay(10);
    //Line 367
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff040102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020104ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402ff00);
    udelay(10);
    //Line 368
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfd0101);
    udelay(10);
    //Line 369
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefc0000);
    udelay(10);
    //Line 370
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe02fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00feff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd01ff00);
    udelay(10);
    //Line 371
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03feff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fdff);
    udelay(10);
    //Line 372
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff020100);
    udelay(10);
    //Line 373
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0203fffe);
    udelay(10);
    //Line 374
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0001);
    udelay(10);
    //Line 375
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0301ffff);
    udelay(10);
    //Line 376
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc030800);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000600f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05010801);
    udelay(10);
    //Line 377
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd040601);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03030006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0402);
    udelay(10);
    //Line 378
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffb0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010007);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0302fcff);
    udelay(10);
    //Line 379
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0400fb06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfafefb01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfef9fc03);
    udelay(10);
    //Line 380
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300fd08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fd0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fb0001);
    udelay(10);
    //Line 381
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000003ff);
    udelay(10);
    //Line 382
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01030100);
    udelay(10);
    //Line 383
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010100);
    udelay(10);
    //Line 384
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010202fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0303ffff);
    udelay(10);
    //Line 385
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020300);
    udelay(10);
    //Line 386
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000100);
    udelay(10);
    //Line 387
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0102ffff);
    udelay(10);
    //Line 388
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fdff);
    udelay(10);
    //Line 389
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe03ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0402fef7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0702fcfc);
    udelay(10);
    //Line 390
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fdfefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000503);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfafef90b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fc0201);
    udelay(10);
    //Line 391
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0700fffe);
    udelay(10);
    //Line 392
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020003ff);
    udelay(10);
    //Line 393
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000105ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020102fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060102fe);
    udelay(10);
    //Line 394
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000301ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fdfe);
    udelay(10);
    //Line 395
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020002fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03030002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0803fbff);
    udelay(10);
    //Line 396
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000fe);
    udelay(10);
    //Line 397
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fdff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff01ff);
    udelay(10);
    //Line 398
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fdff00);
    udelay(10);
    //Line 399
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030301ff);
    udelay(10);
    //Line 400
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0201);
    udelay(10);
    //Line 401
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01feffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff01ff);
    udelay(10);
    //Line 402
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040202fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd02fd01);
    udelay(10);
    //Line 403
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe02f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fafc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203010f);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff020002);
    udelay(10);
    //Line 404
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0205fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0403fefe);
    udelay(10);
    //Line 405
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe03f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000100);
    udelay(10);
    //Line 406
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fd0201);
    udelay(10);
    //Line 407
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff020004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000101);
    udelay(10);
    //Line 408
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfd01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020000fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0103ff00);
    udelay(10);
    //Line 409
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffc0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010401);
    udelay(10);
    //Line 410
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fcff01);
    udelay(10);
    //Line 411
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0101);
    udelay(10);
    //Line 412
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010000ff);
    udelay(10);
    //Line 413
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0101);
    udelay(10);
    //Line 414
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefefd00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fc0001);
    udelay(10);
    //Line 415
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeffe1fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01f5f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xedf001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xe4ee0801);
    udelay(10);
    //Line 416
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefbf6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000101f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa010201);
    udelay(10);
    //Line 417
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fafffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0200fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8000300);
    udelay(10);
    //Line 418
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000403fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb000201);
    udelay(10);
    //Line 419
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0201ff);
    udelay(10);
    //Line 420
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff02ff00);
    udelay(10);
    //Line 421
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030102fe);
    udelay(10);
    //Line 422
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000001);
    udelay(10);
    //Line 423
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 424
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000100ff);
    udelay(10);
    //Line 425
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0201ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010100);
    udelay(10);
    //Line 426
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff00);
    udelay(10);
    //Line 427
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010001);
    udelay(10);
    //Line 428
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fcffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fb0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffef2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff03fafe);
    udelay(10);
    //Line 429
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffd06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fbfe);
    udelay(10);
    //Line 430
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb000104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05010301);
    udelay(10);
    //Line 431
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0103ff00);
    udelay(10);
    //Line 432
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe01f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050004fe);
    udelay(10);
    //Line 433
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe01fe);
    udelay(10);
    //Line 434
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0502ff00);
    udelay(10);
    //Line 435
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201feff);
    udelay(10);
    //Line 436
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020201fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010100fe);
    udelay(10);
    //Line 437
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ffff00);
    udelay(10);
    //Line 438
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02feff00);
    udelay(10);
    //Line 439
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeffff00);
    udelay(10);
    //Line 440
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000200);
    udelay(10);
    //Line 441
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fd0402);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0103fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0304fa02);
    udelay(10);
    //Line 442
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffeffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00030005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0802fdfd);
    udelay(10);
    //Line 443
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfd01fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe04fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffdff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05020101);
    udelay(10);
    //Line 444
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0205fff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fb01);
    udelay(10);
    //Line 445
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0201fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000001);
    udelay(10);
    //Line 446
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fc0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff00fe);
    udelay(10);
    //Line 447
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfa0201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fd03fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe0301);
    udelay(10);
    //Line 448
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffd0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05ff00fe);
    udelay(10);
    //Line 449
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fffb00);
    udelay(10);
    //Line 450
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0401ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fc01);
    udelay(10);
    //Line 451
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020000);
    udelay(10);
    //Line 452
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402feff);
    udelay(10);
    //Line 453
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201ffff);
    udelay(10);
    //Line 454
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfb02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfa0007fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fbf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fcfe02);
    udelay(10);
    //Line 455
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf7fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0805fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02feff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04010100);
    udelay(10);
    //Line 456
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd020401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02feff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ff0301);
    udelay(10);
    //Line 457
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020001f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fffe);
    udelay(10);
    //Line 458
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0002fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fd0200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fcfffe);
    udelay(10);
    //Line 459
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff02);
    udelay(10);
    //Line 460
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fe01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ff00);
    udelay(10);
    //Line 461
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0300fffd);
    udelay(10);
    //Line 462
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd00ff02);
    udelay(10);
    //Line 463
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030200fc);
    udelay(10);
    //Line 464
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 465
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010100ff);
    udelay(10);
    //Line 466
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00ff01);
    udelay(10);
    //Line 467
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdfa02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffdfa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0600fbfe);
    udelay(10);
    //Line 468
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020402fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0401fcfa);
    udelay(10);
    //Line 469
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020206);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101fd02);
    udelay(10);
    //Line 470
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fefffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff01fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101fdfc);
    udelay(10);
    //Line 471
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010201);
    udelay(10);
    //Line 472
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010001ff);
    udelay(10);
    //Line 473
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0502ff00);
    udelay(10);
    //Line 474
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040101fd);
    udelay(10);
    //Line 475
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010101);
    udelay(10);
    //Line 476
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010102ff);
    udelay(10);
    //Line 477
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001fffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020200ff);
    udelay(10);
    //Line 478
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040200fd);
    udelay(10);
    //Line 479
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff00ff);
    udelay(10);
    //Line 480
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbf5fefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf5fdff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0405f7fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00faf902);
    udelay(10);
    //Line 481
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfc01f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0205fe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0402fffb);
    udelay(10);
    //Line 482
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x07010200);
    udelay(10);
    //Line 483
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fe00ff);
    udelay(10);
    //Line 484
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010004ff);
    udelay(10);
    //Line 485
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02030001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff030b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0301ffff);
    udelay(10);
    //Line 486
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ffff00);
    udelay(10);
    //Line 487
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff01ff);
    udelay(10);
    //Line 488
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200fe01);
    udelay(10);
    //Line 489
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010100);
    udelay(10);
    //Line 490
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01000100);
    udelay(10);
    //Line 491
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0101);
    udelay(10);
    //Line 492
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000000);
    udelay(10);
    //Line 493
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010207);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fafe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010109);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050106fe);
    udelay(10);
    //Line 494
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9fefef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fb03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06010402);
    udelay(10);
    //Line 495
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffcff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fef8fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc01ff0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002fbff);
    udelay(10);
    //Line 496
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06feff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f8f6fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8fef902);
    udelay(10);
    //Line 497
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0500ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefafb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0003f903);
    udelay(10);
    //Line 498
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0201ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020102fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdff0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff020001);
    udelay(10);
    //Line 499
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010401);
    udelay(10);
    //Line 500
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd030104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020101);
    udelay(10);
    //Line 501
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000203fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ffffff);
    udelay(10);
    //Line 502
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01feff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc000205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000103);
    udelay(10);
    //Line 503
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010100);
    udelay(10);
    //Line 504
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff00ff);
    udelay(10);
    //Line 505
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002ff00);
    udelay(10);
    //Line 506
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0302ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202fef8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0bfffdff);
    udelay(10);
    //Line 507
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0006fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff04f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfb010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x09fdfe03);
    udelay(10);
    //Line 508
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fc0203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00020c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010100);
    udelay(10);
    //Line 509
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd02fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fe01);
    udelay(10);
    //Line 510
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00030003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfefd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0302ff00);
    udelay(10);
    //Line 511
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff03ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040200ff);
    udelay(10);
    //Line 512
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfe01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0003fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010200);
    udelay(10);
    //Line 513
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfd0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfe0201);
    udelay(10);
    //Line 514
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe01fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fcfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fe01fe);
    udelay(10);
    //Line 515
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0104ff);
    udelay(10);
    //Line 516
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020200);
    udelay(10);
    //Line 517
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff01);
    udelay(10);
    //Line 518
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0202fffe);
    udelay(10);
    //Line 519
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fb06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fcf0f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdf5010c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeed0204);
    udelay(10);
    //Line 520
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fffe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fb00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fcfe);
    udelay(10);
    //Line 521
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc000006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x030000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05ff0101);
    udelay(10);
    //Line 522
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0700fc07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfcfe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fa0503);
    udelay(10);
    //Line 523
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf801fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9f90403);
    udelay(10);
    //Line 524
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcfd00fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd040001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010102ec);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefcfc01);
    udelay(10);
    //Line 525
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02ff0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010102fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff03fe);
    udelay(10);
    //Line 526
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030401f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fd01);
    udelay(10);
    //Line 527
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fffe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fefc01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfafdfc00);
    udelay(10);
    //Line 528
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff01fdff);
    udelay(10);
    //Line 529
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fff9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffcff);
    udelay(10);
    //Line 530
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fefd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfefdf9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010001ff);
    udelay(10);
    //Line 531
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000003ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfbfd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0400);
    udelay(10);
    //Line 532
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200f307);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02edf8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf702020c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdf5ff01);
    udelay(10);
    //Line 533
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fbfd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02f903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x040100f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfff403);
    udelay(10);
    //Line 534
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf4f9fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00040400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fd00f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfd00ff);
    udelay(10);
    //Line 535
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01020008);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000203fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fd0203);
    udelay(10);
    //Line 536
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff040200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0302feff);
    udelay(10);
    //Line 537
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010202ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020100f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0000fa);
    udelay(10);
    //Line 538
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fffefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fef7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe00ff);
    udelay(10);
    //Line 539
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fc0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0101ff);
    udelay(10);
    //Line 540
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefe00fe);
    udelay(10);
    //Line 541
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0100fe);
    udelay(10);
    //Line 542
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000201fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff00ff);
    udelay(10);
    //Line 543
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010100);
    udelay(10);
    //Line 544
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000002fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020100);
    udelay(10);
    //Line 545
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001fd05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffeef4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf3f3ff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe03fe);
    udelay(10);
    //Line 546
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0206fafd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04fffefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfffd00);
    udelay(10);
    //Line 547
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf6fffefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x03fcffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfbfeff);
    udelay(10);
    //Line 548
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffa03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf9fc0309);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc000601);
    udelay(10);
    //Line 549
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfdff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0005fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fcfc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfd0200);
    udelay(10);
    //Line 550
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010103ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030302ed);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefdfefe);
    udelay(10);
    //Line 551
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fefffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fb0504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02020104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9fdff00);
    udelay(10);
    //Line 552
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe00fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020101fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fcfe);
    udelay(10);
    //Line 553
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01feffff);
    udelay(10);
    //Line 554
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fffeff);
    udelay(10);
    //Line 555
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0200ff00);
    udelay(10);
    //Line 556
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffd0101);
    udelay(10);
    //Line 557
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fd0202);
    udelay(10);
    //Line 558
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfffefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050206ff);
    udelay(10);
    //Line 559
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x08010005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000404ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030208);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030102fd);
    udelay(10);
    //Line 560
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0600fc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02f8fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefefe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfef900);
    udelay(10);
    //Line 561
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0503fd01);
    udelay(10);
    //Line 562
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x07010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02040203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0202fcff);
    udelay(10);
    //Line 563
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffffff00);
    udelay(10);
    //Line 564
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fd01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff02fd);
    udelay(10);
    //Line 565
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfe02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010000fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00010100);
    udelay(10);
    //Line 566
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000300);
    udelay(10);
    //Line 567
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010002ff);
    udelay(10);
    //Line 568
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000101);
    udelay(10);
    //Line 569
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffeff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0001);
    udelay(10);
    //Line 570
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000000);
    udelay(10);
    //Line 571
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fff107);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01f7fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf4f6fd08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf3fb0300);
    udelay(10);
    //Line 572
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffcfffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400fe03);
    udelay(10);
    //Line 573
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fafffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfcfd0301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0106fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fe01);
    udelay(10);
    //Line 574
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd010203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe050003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fcff01);
    udelay(10);
    //Line 575
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffe06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0003ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04000000);
    udelay(10);
    //Line 576
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000301);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02fafb);
    udelay(10);
    //Line 577
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03fefe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0004ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fffc02);
    udelay(10);
    //Line 578
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdffff00);
    udelay(10);
    //Line 579
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fffe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01040100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fd0000);
    udelay(10);
    //Line 580
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fffc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfef90000);
    udelay(10);
    //Line 581
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01040000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fdfd02);
    udelay(10);
    //Line 582
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfd0100);
    udelay(10);
    //Line 583
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020102);
    udelay(10);
    //Line 584
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0001fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fcff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8010309);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030102fc);
    udelay(10);
    //Line 585
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf900fef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0106f8f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fb01f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcff04f6);
    udelay(10);
    //Line 586
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf80200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0003fefd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfa01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000001fd);
    udelay(10);
    //Line 587
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0006ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfb04fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfafd0401);
    udelay(10);
    //Line 588
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefefffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefffd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe020200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9fffe01);
    udelay(10);
    //Line 589
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0201f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfffe00);
    udelay(10);
    //Line 590
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fff6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffefdff);
    udelay(10);
    //Line 591
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0202fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffffff);
    udelay(10);
    //Line 592
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020400fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03000002);
    udelay(10);
    //Line 593
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03020000);
    udelay(10);
    //Line 594
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03010101);
    udelay(10);
    //Line 595
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0000);
    udelay(10);
    //Line 596
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe0001);
    udelay(10);
    //Line 597
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000202fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0403f901);
    udelay(10);
    //Line 598
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05010003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000801fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0105fd06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0701fbfc);
    udelay(10);
    //Line 599
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0003fd08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fd00fe);
    udelay(10);
    //Line 600
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000202fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0200fefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x060000ff);
    udelay(10);
    //Line 601
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff01fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020400);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe0000);
    udelay(10);
    //Line 602
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefcff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfc0102);
    udelay(10);
    //Line 603
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0102fc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0102fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06feff00);
    udelay(10);
    //Line 604
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000400);
    udelay(10);
    //Line 605
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00feff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0000);
    udelay(10);
    //Line 606
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffeff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeff0001);
    udelay(10);
    //Line 607
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010201fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020100ff);
    udelay(10);
    //Line 608
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fdff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0102f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefe0200);
    udelay(10);
    //Line 609
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000003fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020102ff);
    udelay(10);
    //Line 610
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfffbfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fefff8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf0f9ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7fe05ff);
    udelay(10);
    //Line 611
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fdf3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc02f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0105fb);
    udelay(10);
    //Line 612
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fefe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0108);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa0101ff);
    udelay(10);
    //Line 613
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfe0202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffc05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7ff0102);
    udelay(10);
    //Line 614
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf703fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0403ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fdff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbff0302);
    udelay(10);
    //Line 615
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02fd01);
    udelay(10);
    //Line 616
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04020102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01020303);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01030108);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe00fd01);
    udelay(10);
    //Line 617
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03ffffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffaff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0200ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb02fd00);
    udelay(10);
    //Line 618
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe010100);
    udelay(10);
    //Line 619
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x03000004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0401);
    udelay(10);
    //Line 620
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0102fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fefeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe010206);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff00);
    udelay(10);
    //Line 621
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe02ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000103fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fb0100);
    udelay(10);
    //Line 622
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbfb0000);
    udelay(10);
    //Line 623
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe03fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050300ff);
    udelay(10);
    //Line 624
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f905fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfdff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06010300);
    udelay(10);
    //Line 625
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010201fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0206ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fefd00);
    udelay(10);
    //Line 626
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fffcfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbfafb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffeff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffef9fd);
    udelay(10);
    //Line 627
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01040002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030200fc);
    udelay(10);
    //Line 628
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfb03fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffe0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0001f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02020001);
    udelay(10);
    //Line 629
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0602ff00);
    udelay(10);
    //Line 630
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0201fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd000203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000200);
    udelay(10);
    //Line 631
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020201);
    udelay(10);
    //Line 632
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000200);
    udelay(10);
    //Line 633
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0106);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffffff01);
    udelay(10);
    //Line 634
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffffdff);
    udelay(10);
    //Line 635
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0001ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd020201);
    udelay(10);
    //Line 636
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe030202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfff900fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fe0a08);
    udelay(10);
    //Line 637
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff020409);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fd0209);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7000401);
    udelay(10);
    //Line 638
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200fefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd03fdfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0202fe09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0500fa00);
    udelay(10);
    //Line 639
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06fffd06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff02faff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefeff0c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ff01);
    udelay(10);
    //Line 640
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0700ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe04fc03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00fd08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400ff02);
    udelay(10);
    //Line 641
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02000000);
    udelay(10);
    //Line 642
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0101);
    udelay(10);
    //Line 643
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff01fe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03ff0002);
    udelay(10);
    //Line 644
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff02ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020104ff);
    udelay(10);
    //Line 645
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010201fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010102);
    udelay(10);
    //Line 646
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0000);
    udelay(10);
    //Line 647
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff0101);
    udelay(10);
    //Line 648
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000100ff);
    udelay(10);
    //Line 649
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00030201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020200f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe02fd);
    udelay(10);
    //Line 650
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fa04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefb000b);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfaff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8f10301);
    udelay(10);
    //Line 651
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0601ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100fe0e);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05feff00);
    udelay(10);
    //Line 652
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0203ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0601ff01);
    udelay(10);
    //Line 653
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fefeff);
    udelay(10);
    //Line 654
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff00ff);
    udelay(10);
    //Line 655
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00020004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0001);
    udelay(10);
    //Line 656
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000005);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000ffff);
    udelay(10);
    //Line 657
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010101fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02ffff);
    udelay(10);
    //Line 658
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe00ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffeffff);
    udelay(10);
    //Line 659
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ffff);
    udelay(10);
    //Line 660
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000101fe);
    udelay(10);
    //Line 661
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0000fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x030001fd);
    udelay(10);
    //Line 662
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefdf6fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdfa01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xeefdff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf2fc0501);
    udelay(10);
    //Line 663
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fcfe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbf9fbff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbff05f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc020502);
    udelay(10);
    //Line 664
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0101f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000302fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ff00f1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf20100ff);
    udelay(10);
    //Line 665
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0401f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfbfe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x050305fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fcfe03);
    udelay(10);
    //Line 666
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x050100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfe01f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe020201);
    udelay(10);
    //Line 667
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x06010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff03fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fb02);
    udelay(10);
    //Line 668
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff02ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020100ff);
    udelay(10);
    //Line 669
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0102fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02010001);
    udelay(10);
    //Line 670
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x020001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffc0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc020000);
    udelay(10);
    //Line 671
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff0002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd01ff01);
    udelay(10);
    //Line 672
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0101);
    udelay(10);
    //Line 673
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010304);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00ff00);
    udelay(10);
    //Line 674
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000000fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe01ffff);
    udelay(10);
    //Line 675
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fefffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fd0205);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010301f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0000fa);
    udelay(10);
    //Line 676
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0400ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe030307);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fcff00);
    udelay(10);
    //Line 677
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fefe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd010103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe00ff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffd0200);
    udelay(10);
    //Line 678
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd020401);
    udelay(10);
    //Line 679
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020003ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06fe04ff);
    udelay(10);
    //Line 680
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffefc08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffc0301);
    udelay(10);
    //Line 681
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fe02fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x090100fc);
    udelay(10);
    //Line 682
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02000003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fdfe00);
    udelay(10);
    //Line 683
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x040000fe);
    udelay(10);
    //Line 684
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000200);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00ff00);
    udelay(10);
    //Line 685
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x050300fe);
    udelay(10);
    //Line 686
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff000100);
    udelay(10);
    //Line 687
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010101fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020100ff);
    udelay(10);
    //Line 688
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0000ff05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fefd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fc0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffff01fd);
    udelay(10);
    //Line 689
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb00fef9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01f702ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefa0002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0501);
    udelay(10);
    //Line 690
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020103fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff01ff07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fd0101);
    udelay(10);
    //Line 691
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fbff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe00ef);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x06ffff01);
    udelay(10);
    //Line 692
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0001ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010003ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010300f9);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0c01fd00);
    udelay(10);
    //Line 693
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fb03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfafe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfc0001);
    udelay(10);
    //Line 694
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdfeff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01fb00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdff02ff);
    udelay(10);
    //Line 695
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fffe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030302ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0502ffff);
    udelay(10);
    //Line 696
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010201fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0001ff01);
    udelay(10);
    //Line 697
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0300ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff00fe00);
    udelay(10);
    //Line 698
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000300fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100ff01);
    udelay(10);
    //Line 699
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000102ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010302fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0403ffff);
    udelay(10);
    //Line 700
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002ff01);
    udelay(10);
    //Line 701
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0502ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102fb03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x04feff09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffcf902);
    udelay(10);
    //Line 702
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0301fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fc00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0109);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0102);
    udelay(10);
    //Line 703
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfcfd020a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fe0201);
    udelay(10);
    //Line 704
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x030101f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00fb0102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010001f5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x05fffdfd);
    udelay(10);
    //Line 705
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfeff0204);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0009);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0302);
    udelay(10);
    //Line 706
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffdfd04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01020003);
    udelay(10);
    //Line 707
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fbfd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffc04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd000203);
    udelay(10);
    //Line 708
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01fdff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000200fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfd00fc02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fc0001);
    udelay(10);
    //Line 709
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe03ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffffff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfeffff03);
    udelay(10);
    //Line 710
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01010101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff02ff00);
    udelay(10);
    //Line 711
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010001fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0100ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000102);
    udelay(10);
    //Line 712
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010200ff);
    udelay(10);
    //Line 713
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd010001);
    udelay(10);
    //Line 714
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfcffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0005f3fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfaf3fe05);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb0404ff);
    udelay(10);
    //Line 715
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00fffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0100fdf5);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x080502fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfffb02);
    udelay(10);
    //Line 716
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9fafffb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fcff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeff02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6fe0500);
    udelay(10);
    //Line 717
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd0004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00060102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfefd03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd0103ff);
    udelay(10);
    //Line 718
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffd01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010202fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030600ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0100fa00);
    udelay(10);
    //Line 719
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02ff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00f90300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010304f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfb00fd01);
    udelay(10);
    //Line 720
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fbff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfffefd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9fffe02);
    udelay(10);
    //Line 721
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02020100);
    udelay(10);
    //Line 722
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd01ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff02fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03fe0100);
    udelay(10);
    //Line 723
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe01ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0102fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffff00);
    udelay(10);
    //Line 724
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00010201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffffe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01fd0000);
    udelay(10);
    //Line 725
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0000);
    udelay(10);
    //Line 726
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000200fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ffffff);
    udelay(10);
    //Line 727
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fdf6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xef00ff04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe0102fe);
    udelay(10);
    //Line 728
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfdff00fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf8f905f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf70207fd);
    udelay(10);
    //Line 729
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf9fa00f1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc020201);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffcf3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8000100);
    udelay(10);
    //Line 730
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0102f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc0103ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03050000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fcfe00);
    udelay(10);
    //Line 731
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05ffff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fcfb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0202fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0503fcfe);
    udelay(10);
    //Line 732
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fe0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0200ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffe0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0002fafd);
    udelay(10);
    //Line 733
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fcfeff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfc01f8fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe0301fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0301fb02);
    udelay(10);
    //Line 734
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe02fd01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020401);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefc0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fc02ff);
    udelay(10);
    //Line 735
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff04ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01010200);
    udelay(10);
    //Line 736
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd0100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00000100);
    udelay(10);
    //Line 737
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0101ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03ff01fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xffffff00);
    udelay(10);
    //Line 738
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0002ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff0103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe000100);
    udelay(10);
    //Line 739
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000100fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x01ff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff02ff01);
    udelay(10);
    //Line 740
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0200f0fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001fdf8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf0f8fe09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf1f30501);
    udelay(10);
    //Line 741
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfffffb02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdff01f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x030306f6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfafd0101);
    udelay(10);
    //Line 742
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefefffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0101fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffa0300);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf40303fd);
    udelay(10);
    //Line 743
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe0000fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff0501fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0301ff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc00fffe);
    udelay(10);
    //Line 744
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd01ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000302fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fc0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefd0400);
    udelay(10);
    //Line 745
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x02fe0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe01fe00);
    udelay(10);
    //Line 746
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fd0001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00020003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfffeff0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8ff03ff);
    udelay(10);
    //Line 747
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0002ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0201fe01);
    udelay(10);
    //Line 748
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010202);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefe0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0102ff);
    udelay(10);
    //Line 749
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ff0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff020002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00fe0201);
    udelay(10);
    //Line 750
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0101ff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fffdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000103);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0101ff00);
    udelay(10);
    //Line 751
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000100ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffeff00);
    udelay(10);
    //Line 752
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff00ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01000102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00ff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfcfd0000);
    udelay(10);
    //Line 753
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbff02fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfefefefc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfb0504);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf8020501);
    udelay(10);
    //Line 754
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0401fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fd02fa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfbfd04f7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa0007fb);
    udelay(10);
    //Line 755
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb00fffd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x020105fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00fffefa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf9000100);
    udelay(10);
    //Line 756
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf901ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd030302);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x03fffffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7fc0201);
    udelay(10);
    //Line 757
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fcfffc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfd030004);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000304f4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf802fe01);
    udelay(10);
    //Line 758
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x05fe0101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfdff0104);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfe05fffa);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc02fd00);
    udelay(10);
    //Line 759
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x04fefe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fe01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff03fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfe02fdff);
    udelay(10);
    //Line 760
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01fcfe03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fdfefe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfc0201fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x000005ff);
    udelay(10);
    //Line 761
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfd02fe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff02fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x010003fd);
    udelay(10);
    //Line 762
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0100fe00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff00ff08);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff0101fe);
    udelay(10);
    //Line 763
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe02ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xffff0600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfeffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfff90301);
    udelay(10);
    //Line 764
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfefe00fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x02fbff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000101fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01030100);
    udelay(10);
    //Line 765
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fe00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00ffff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0101fefb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfc0100ff);
    udelay(10);
    //Line 766
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfe00f404);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ebf6);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xeaebff06);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6f90400);
    udelay(10);
    //Line 767
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfb0001fd);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0103fbf7);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02fffffe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0000fd01);
    udelay(10);
    //Line 768
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf5ffff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfdfc0003);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfdfe0300);
    udelay(10);
    //Line 769
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfbfd0006);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff00fd02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xffff00ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfffe0300);
    udelay(10);
    //Line 770
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff01ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0102ff02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00020203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfbff00ff);
    udelay(10);
    //Line 771
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000101fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x010002ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x020400fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefdff02);
    udelay(10);
    //Line 772
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xffff01fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01ffff01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010002fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfa01feff);
    udelay(10);
    //Line 773
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00010102);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x010300fe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02ff0001);
    udelay(10);
    //Line 774
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff000101);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001feff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00010100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x01ff0000);
    udelay(10);
    //Line 775
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff0000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0001ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x02010000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x02fefe01);
    udelay(10);
    //Line 776
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfeff0100);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01010001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xfefffe02);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x00ff0201);
    udelay(10);
    //Line 777
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x000001ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x000000ff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x000200fc);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0400feff);
    udelay(10);
    //Line 778
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x01ff0000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0000ff00);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xff000002);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xff010000);
    udelay(10);
    //Line 779
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xedf2ef0c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0d0e0b0c);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xec0a0b0a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0a090b0b);
    udelay(10);
    //Line 780
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf1eff4ed);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0d0b08f0);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0c080ff1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf3f40df3);
    udelay(10);
    //Line 781
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x110f0df1);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf7f3f4f3);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x14f5f4f2);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf6f7f4f9);
    udelay(10);
    //Line 782
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0d120c12);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xf4f7f511);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0xf6f4f214);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x0c0cf30c);
    udelay(10);
    //Line 783
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xf8fbf903);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0708fd07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0508fe09);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfd080104);
    udelay(10);
    //Line 784
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xff030105);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x0206fdfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0500ff03);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xfefafd08);
    udelay(10);
    //Line 785
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00fdf403);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xff01fd07);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x05040604);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0xf7f701ff);
    udelay(10);
    //Line 786
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0af7f40a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x01fbfcfe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0402fa01);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x020a040b);
    udelay(10);
    //Line 787
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x0401fe04);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x04fe0afe);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0302ffff);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x04fefc09);
    udelay(10);
    //Line 788
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0xfc0204fb);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0xfe0a0600);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x0902f702);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x03f607fd);
    udelay(10);
    //Line 789
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_2_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_3_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_4_reg, 0x00000000);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_5_reg, 0x12f00000);
    udelay(10);
    
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_coef_sram_1_reg, 0x00000000);

    //mean, std settings
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_3_reg, 0x04edf203);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_4_reg, 0xf6f2f8ed);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_5_reg, 0x01f3fef4);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_6_reg, 0x000019f8);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_7_reg, 0x1214141a);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_8_reg, 0x13121313);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_9_reg, 0x17161514);

    //TH settings
    //scale[25:22], dnn_th[15:8], kw_weight[4:0]
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x03c00000, (scale<<22));
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x0000ff00, (dnn_th<<8));
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_1_reg, ~0x0000001f, kw_weight);
    
    ret |= rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_tcon_1_reg, &SVAD_tcon_1_reg);
    
    SVAD_INFO("\033[1;32;33m" "TH settings: 0x%08x" "\033[m", SVAD_tcon_1_reg);
#endif
}
EXPORT_SYMBOL(rtk_svad_set_model_l);
