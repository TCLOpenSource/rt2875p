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
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_svad_def.h>

#include "rbus/stb_reg.h"
#include "rbus/wov_reg.h"
#include "rbus/pinmux_reg.h"
#include "rbus/MIO/smart_vad_reg.h"
#include "rbus/MIO/mio_sys_reg_reg.h"
#include "rbus/MIO/pinmux_mio_reg.h"

#define KW_OUT_GPIO_MODE

#ifndef MIO_I2C_MODE
#include "rbus/iso_spi_synchronizer_reg.h"
#endif

/******************************************************************************
                       I2C speed parameter
******************************************************************************/
#define I2C_M_NORMAL_SPEED      0x0000
#define I2C_M_FAST_SPEED        0x0002
#define I2C_M_HIGH_SPEED        0x0004
#define I2C_M_LOW_SPEED         0x0006


/******************************************************************************
                     SVAD only works on WOV DMIC path
*****************************************************************************/

extern unsigned char g_thread_en;
extern struct semaphore rtksvad_Semaphore;
extern int rtk_svad_thread_en(unsigned int thread_en);
extern int i2c_master_send_ex_flag(unsigned char bus_id, unsigned char addr, unsigned char *write_buff, unsigned int write_len, __u16 flags);
extern int i2c_master_recv_ex_flag(unsigned char bus_id, unsigned char addr, unsigned char *p_sub_addr, unsigned char sub_addr_len, unsigned char *p_read_buff, unsigned int read_len, __u16 flags);

extern void rtk_svad_set_model_g(unsigned int scale, unsigned int dnn_th, unsigned int kw_weight);
extern void rtk_svad_set_model_l(unsigned int scale, unsigned int dnn_th, unsigned int kw_weight);
int rtk_svad_4bytes_i2c_wr(unsigned int addr, unsigned int data);
int rtk_svad_4bytes_i2c_rd(unsigned int addr, unsigned int* data);
int rtk_svad_4bytes_i2c_mask(unsigned int addr, unsigned int mask, unsigned int data);
int rtk_svad_crt_clk_onoff(enum CRT_CLK clken);

void rtk_svad_set_dmic_input(void)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
    unsigned int reg_value_sys_srst = 0;
    unsigned int reg_value_sys_clken = 0;
#endif

    SVAD_INFO("\033[1;32;33m" "rtk_svad_set_dmic_input" "\033[m");

    if(((rtd_inl(STB_ST_SRST1_reg) & STB_ST_SRST1_rstn_wov_mask) != STB_ST_SRST1_rstn_wov_mask) || ((rtd_inl(STB_ST_CLKEN1_reg) & STB_ST_CLKEN1_clken_wov_mask) != STB_ST_CLKEN1_clken_wov_mask))
    {
        SVAD_INFO("\033[1;32;33m" "WOV CRT ON" "\033[m");
        CRT_CLK_OnOff(WOV, CLK_ON, NULL);
    }

    rtd_maskl(WOV_wov_vad_1_reg, 0xfff1ffff, 0x00020000);//3'b001: ad0_out_r (lg 2ch dmic use ad0)
    rtd_maskl(WOV_mm_vad_energy6_reg, 0xff8003ff, 0x00000000);//ptich_det_thr_st_valid [22:10] (for easy trigger)

    rtd_outl(WOV_stby_tcon_1_reg, 0xd54bfb00);// 3.375Mhz
    rtd_outl(WOV_stby_ad0_1_reg, 0x00006464);// dmic0 for MMD201_65DB dmic
    rtd_outl(WOV_stby_ad1_1_reg, 0x00006464);// dmic1 for MMD201_65DB dmic

    rtd_maskl(0xb8060454, 0xffffefff, 0x00001000);//EMCU_ST_IE_stie_wov_vad
    rtd_maskl(WOV_wov_ctrl_1_reg, 0x9fffffff, 0x60000000);//bit[30:29]=11
    
#ifdef MIO_I2C_MODE
    ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_SRST_reg, &reg_value_sys_srst);
    ret |= rtk_svad_4bytes_i2c_rd(SYS_REG_SYS_CLKEN_reg, &reg_value_sys_clken);

    if(((reg_value_sys_srst & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((reg_value_sys_clken & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
    {
        SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
        rtk_svad_crt_clk_onoff(CLK_ON);
    }
    
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_tcon_0_reg, 0xD54BE800);

    //Gain 10dB
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_0_reg, 0x46121001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_1_reg, 0x46121001);
    ret |= rtk_svad_4bytes_i2c_wr(SMART_VAD_svad_dmic_2_reg, 0x4a4a4a4a);
    
    #ifdef KW_OUT_GPIO_MODE
    //disable MK2 dmic clk
    rtd_maskl(PINMUX_ST_GPIO_ST_CFG_1_reg, 0xff0fffff, 0x00f00000);
    //enable MIO dmic clk
    ret |= rtk_svad_4bytes_i2c_mask(PINMUX_MIO_ST_GPIO_MIO_CFG_4_reg, 0xffff0fff, 0x00004000);
    //reset dmic clk
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_0_reg, 0xfffdffff, 0x00000000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00000000);
    msleep_interruptible(30);
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_0_reg, 0xfffdffff, 0x00020000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00020000);
    //enable MIO kw_out
    ret |= rtk_svad_4bytes_i2c_mask(PINMUX_MIO_ST_GPIO_MIO_CFG_9_reg, 0xffffff0f, 0x00000040);
    //disable SPI dmic sync
    //rtd_maskl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg, 0xffefffff, 0x00000000);
    #else
    //enable MK2 dmic clk
    rtd_maskl(PINMUX_ST_GPIO_ST_CFG_1_reg, 0xff0fffff, 0x00900000);
    //disable MIO dmic clk
    ret |= rtk_svad_4bytes_i2c_mask(PINMUX_MIO_ST_GPIO_MIO_CFG_4_reg, 0xffff0fff, 0x0000f000);
    //reset dmic clk
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_0_reg, 0xfffdffff, 0x00000000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00000000);
    msleep_interruptible(30);
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_tcon_0_reg, 0xfffdffff, 0x00020000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00020000);
    //disable MIO kw_out
    ret |= rtk_svad_4bytes_i2c_mask(PINMUX_MIO_ST_GPIO_MIO_CFG_9_reg, 0xffffff0f, 0x000000f0);
    //enable SPI dmic sync
    rtd_maskl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg, 0xffefffff, 0x00100000);
    #endif
#else
    if(((rtd_inl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_SRST_rstn_sv_mask) != SYS_REG_SYS_SRST_rstn_sv_mask) || ((rtd_inl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT) & SYS_REG_SYS_CLKEN_clken_sv_mask) != SYS_REG_SYS_CLKEN_clken_sv_mask))
    {
        SVAD_INFO("\033[1;32;33m" "SVAD CRT ON" "\033[m");
        rtk_svad_crt_clk_onoff(CLK_ON);
    }
    
    rtd_outl(SMART_VAD_svad_tcon_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xD54BE800);

    //Gain 10dB
    rtd_outl(SMART_VAD_svad_dmic_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x46121001);
    rtd_outl(SMART_VAD_svad_dmic_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x46121001);
    rtd_outl(SMART_VAD_svad_dmic_2_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x4a4a4a4a);
    
    #ifdef KW_OUT_GPIO_MODE
    //disable MK2 dmic clk
    rtd_maskl(PINMUX_ST_GPIO_ST_CFG_1_reg, 0xff0fffff, 0x00f00000);
    //enable MIO dmic clk
    rtd_maskl(PINMUX_MIO_ST_GPIO_MIO_CFG_4_reg+MIO_PINMUX_SPI_ADDR_SHIFT, 0xffff0fff, 0x00004000);
    //reset dmic clk
    rtd_maskl(SMART_VAD_svad_tcon_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdffff, 0x00000000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00000000);
    msleep_interruptible(30);
    rtd_maskl(SMART_VAD_svad_tcon_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdffff, 0x00020000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00020000);
    //enable MIO kw_out
    rtd_maskl(PINMUX_MIO_ST_GPIO_MIO_CFG_9_reg+MIO_PINMUX_SPI_ADDR_SHIFT, 0xffffff0f, 0x00000040);
    //disable SPI dmic sync
    rtd_maskl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg, 0xffefffff, 0x00000000);
    #else
    //enable MK2 dmic clk
    rtd_maskl(PINMUX_ST_GPIO_ST_CFG_1_reg, 0xff0fffff, 0x00900000);
    //disable MIO dmic clk
    rtd_maskl(PINMUX_MIO_ST_GPIO_MIO_CFG_4_reg+MIO_PINMUX_SPI_ADDR_SHIFT, 0xffff0fff, 0x0000f000);
    //reset dmic clk
    rtd_maskl(SMART_VAD_svad_tcon_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdffff, 0x00000000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00000000);
    msleep_interruptible(30);
    rtd_maskl(SMART_VAD_svad_tcon_0_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0xfffdffff, 0x00020000);
    rtd_maskl(WOV_stby_tcon_1_reg, 0xfffdffff, 0x00020000);
    //disable MIO kw_out
    rtd_maskl(PINMUX_MIO_ST_GPIO_MIO_CFG_9_reg+MIO_PINMUX_SPI_ADDR_SHIFT, 0xffffff0f, 0x000000f0);
    //enable SPI dmic sync
    rtd_maskl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg, 0xffefffff, 0x00100000);
    #endif
#endif
}

void rtk_svad_model_init(void)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
    unsigned int reg_value_svad_control = 0;
#endif

    SVAD_INFO("\033[1;32;33m" "rtk_svad_model_init" "\033[m");
    
    down(&rtksvad_Semaphore);
    
    g_thread_en = 0;
    
    rtk_svad_set_dmic_input();

#ifdef MIO_I2C_MODE
    //SRAM Settings
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~0xffefffff, 0x72670001);

    ret |= rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_contorl_reg, &reg_value_svad_control);

    if((reg_value_svad_control & SMART_VAD_svad_contorl_top_ctrl_mask) == SMART_VAD_svad_contorl_top_ctrl_mask)
    {
        //rtd_outl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01401006); //scale=5, dnn_th=16, kw_weight=6
        rtk_svad_set_model_l(5, 100, 12);//scale=5, dnn_th=100, kw_weight=12
    }
    else
    {
        //rtd_outl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02406408); //scale=9, dnn_th=100, kw_weight=8
        rtk_svad_set_model_g(9, 110, 9);//scale=9, dnn_th=110, kw_weight=9
    }

    #ifdef KW_OUT_GPIO_MODE
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~0xffefffff, 0x72670016);
    #else
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~0xffefffff, 0x70670016);
    #endif
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000001);
    
#else
    //SRAM Settings
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0xffefffff, 0x72670001);

    if((rtd_inl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT) & SMART_VAD_svad_contorl_top_ctrl_mask) == SMART_VAD_svad_contorl_top_ctrl_mask)
    {
        //rtd_outl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x01401006); //scale=5, dnn_th=16, kw_weight=6
        rtk_svad_set_model_l(5, 100, 12);//scale=5, dnn_th=100, kw_weight=12
    }
    else
    {
        //rtd_outl(SMART_VAD_svad_tcon_1_reg+MIO_SVAD_SPI_ADDR_SHIFT, 0x02406408); //scale=9, dnn_th=100, kw_weight=8
        rtk_svad_set_model_g(9, 110, 9);//scale=9, dnn_th=110, kw_weight=9
    }

    #ifdef KW_OUT_GPIO_MODE
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0xffefffff, 0x72670016);
    #else
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~0xffefffff, 0x70670016);
    #endif
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000001);
#endif
    
    rtk_svad_thread_en(1);
    
    g_thread_en = 1;

    up(&rtksvad_Semaphore);
}
EXPORT_SYMBOL(rtk_svad_model_init);

void rtk_svad_det_start(void)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
#endif

    SVAD_INFO("\033[1;32;33m" "rtk_svad_det_start" "\033[m");
    
    down(&rtksvad_Semaphore);
    
#ifdef MIO_I2C_MODE
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000001);	
#else
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000001);
#endif

    up(&rtksvad_Semaphore);
}
EXPORT_SYMBOL(rtk_svad_det_start);

void rtk_svad_det_stop(void)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
#endif
    
    SVAD_INFO("\033[1;32;33m" "rtk_svad_det_stop" "\033[m");
    
    down(&rtksvad_Semaphore);
    
#ifdef MIO_I2C_MODE
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_contorl_reg, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000000);	
#else
    rtd_maskl(SMART_VAD_svad_contorl_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_contorl_smart_vad_en_mask, 0x00000000);
#endif

    up(&rtksvad_Semaphore);
}
EXPORT_SYMBOL(rtk_svad_det_stop);

void rtk_svad_get_det_result(unsigned int *svad_det_result)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
    unsigned int reg_value_svad_irq = 0;
#endif   

    SVAD_INFO("\033[1;32;33m" "rtk_svad_get_det_result" "\033[m");
    
    down(&rtksvad_Semaphore);
    
#ifdef MIO_I2C_MODE
    ret |= rtk_svad_4bytes_i2c_rd(SMART_VAD_svad_irq_reg, &reg_value_svad_irq);
    if((reg_value_svad_irq & SMART_VAD_svad_irq_int_svad_kw_mask) == SMART_VAD_svad_irq_int_svad_kw_mask)
        *svad_det_result = 1;
    else
        *svad_det_result = 0;
#else
    if((rtd_inl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT) & SMART_VAD_svad_irq_int_svad_kw_mask) == SMART_VAD_svad_irq_int_svad_kw_mask)
        *svad_det_result = 1;
    else
        *svad_det_result = 0;
#endif
   
    *svad_det_result = 2;
    
    up(&rtksvad_Semaphore);
}
EXPORT_SYMBOL(rtk_svad_get_det_result);

void rtk_svad_reset_det_result(void)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
#endif

    SVAD_INFO("\033[1;32;33m" "rtk_svad_reset_det_result" "\033[m");
    
    down(&rtksvad_Semaphore);
    
#ifdef MIO_I2C_MODE  
    ret |= rtk_svad_4bytes_i2c_mask(SMART_VAD_svad_irq_reg, ~SMART_VAD_svad_irq_int_svad_kw_mask, 0x00000000);
#else
    rtd_maskl(SMART_VAD_svad_irq_reg+MIO_SVAD_SPI_ADDR_SHIFT, ~SMART_VAD_svad_irq_int_svad_kw_mask, 0x00000000);
#endif
    
    up(&rtksvad_Semaphore);
}
EXPORT_SYMBOL(rtk_svad_reset_det_result);

int rtk_svad_4bytes_i2c_wr(unsigned int addr, unsigned int data)
{
    int ret = FUNCTION_SUCCESS;
    unsigned char DeviceAddr = 0x60;
    unsigned char WritingBuffer[8];
    unsigned int WritingByteNum = 8;

    addr &= 0x1FFFFFFF; 

    WritingBuffer[0] = addr & 0xff;
    WritingBuffer[1] = (addr >> 8) & 0xff;
    WritingBuffer[2] = (addr >> 16) & 0xff;
    WritingBuffer[3] = (addr >> 24) & 0xff;

    WritingBuffer[4] = data & 0xff;
    WritingBuffer[5] = (data >> 8) & 0xff;
    WritingBuffer[6] = (data >> 16) & 0xff;
    WritingBuffer[7] = (data >> 24) & 0xff;

    ret = i2c_master_send_ex_flag(0, DeviceAddr>>1, WritingBuffer, WritingByteNum, I2C_M_NORMAL_SPEED);

    //if(ret != FUNCTION_SUCCESS)
    //goto error_4bytes_i2c_wr;

    return ret;

//error_4bytes_i2c_wr:
//    SVAD_INFO("rtk_svad_wr_result Error");
//    return FUNCTION_ERROR;

}
EXPORT_SYMBOL(rtk_svad_4bytes_i2c_wr);

int rtk_svad_4bytes_i2c_rd(unsigned int addr, unsigned int* data)
{
    int ret = FUNCTION_SUCCESS;
    unsigned char DeviceAddr = 0x60;
    unsigned char WritingBuffer[4], ReadingBuffer[4];
    unsigned int ReadingByteNum = 4, WritingByteNum = 4;

    addr &= 0x1FFFFFFF;

    WritingBuffer[0] = addr & 0xff;
    WritingBuffer[1] = (addr >> 8) & 0xff;
    WritingBuffer[2] = (addr >> 16) & 0xff;
    WritingBuffer[3] = (addr >> 24) & 0xff;

    ret = i2c_master_recv_ex_flag(0, DeviceAddr>>1,WritingBuffer,WritingByteNum,ReadingBuffer, ReadingByteNum, I2C_M_NORMAL_SPEED);
    //if(ret != FUNCTION_SUCCESS)
    //goto error_status_get_demod_registers;

    *data = (ReadingBuffer[3] << 24) | (ReadingBuffer[2] << 16) | (ReadingBuffer[1] << 8) | (ReadingBuffer[0]);
    //SVAD_INFO("rtk_svad_rd_result : ReadingBuffer3 2 1 0 -> 0x %x %x %x %x ", ReadingBuffer[3], ReadingBuffer[2], ReadingBuffer[1] ,ReadingBuffer[0]);

    return ret;

//error_status_get_demod_registers:
//error_status_set_demod_register_reading_address:
//    SVAD_INFO("rtk_svad_rd_result Error");
//    return FUNCTION_ERROR;

}
EXPORT_SYMBOL(rtk_svad_4bytes_i2c_rd);

int rtk_svad_4bytes_i2c_mask(unsigned int addr, unsigned int mask, unsigned int data)
{
    int ret = FUNCTION_SUCCESS;
    unsigned int readingData = 0, writingData = 0;

    ret = rtk_svad_4bytes_i2c_rd(addr, &readingData);
    writingData = (readingData & mask) | data;
    ret |= rtk_svad_4bytes_i2c_wr(addr, writingData);

    return ret;
}
EXPORT_SYMBOL(rtk_svad_4bytes_i2c_mask);

int rtk_svad_crt_clk_onoff(enum CRT_CLK clken)
{
#ifdef MIO_I2C_MODE
    int ret = FUNCTION_SUCCESS;
#endif

#ifdef MIO_I2C_MODE
    if (clken == CLK_ON) {
        /* HW mode */
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_CLKEN_reg, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift));
        udelay(10);
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_SRST_reg, BIT(SYS_REG_SYS_SRST_rstn_sv_shift));
        udelay(10);
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_SRST_reg, BIT(SYS_REG_SYS_SRST_rstn_sv_shift) | BIT(SYS_REG_SYS_SRST_write_data_shift));
        udelay(10);
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_CLKEN_reg, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift) | BIT(SYS_REG_SYS_CLKEN_write_data_shift));
    } else {
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_CLKEN_reg, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift));
        ret |= rtk_svad_4bytes_i2c_wr(SYS_REG_SYS_SRST_reg, BIT(SYS_REG_SYS_SRST_rstn_sv_shift));
    }
#else
    if (clken == CLK_ON) {
        /* HW mode */
        rtd_outl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_SRST_rstn_sv_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_SRST_rstn_sv_shift) | BIT(SYS_REG_SYS_SRST_write_data_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift) | BIT(SYS_REG_SYS_CLKEN_write_data_shift));
    } else {
        rtd_outl(SYS_REG_SYS_CLKEN_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_CLKEN_clken_sv_shift));
        rtd_outl(SYS_REG_SYS_SRST_reg+MIO_SYS_SPI_ADDR_SHIFT, BIT(SYS_REG_SYS_SRST_rstn_sv_shift));
    }
#endif

    return CRT_SUCCESS;
}
