/*
 * ad82050.c  --  ad82050 ALSA SoC Audio driver
 *
 * Copyright 1998 Elite Semiconductor Memory Technology
 *
 * Author: ESMT Audio/Power Product BU Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public version 2 as
 * published by the Free Software Foundation.
 */
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif
#include "rtk_amp_interface.h"
#include "ad82050.h"

typedef struct
{
    unsigned char bAddr;
    unsigned char bArray;
}AD82050_REG;

static AD82050_REG AD82050_InitTbl[] = {
                     {0x00, 0x05},//##State_Control_1
                     {0x01, 0x82},//##State_Control_2
                     {0x02, 0x10},//##State_Control_3
                     {0x03, 0x20},//##Master_volume_control
                     {0x04, 0x18},//##Channel_1_volume_control
                     {0x05, 0x18},//##Channel_2_volume_control
                     {0x06, 0xa0},//##Under_Voltage_selection_for_high_voltage_supply
                     {0x07, 0x36},//##State_control_4
                     {0x08, 0xa0},//##PWM_control
                     {0x09, 0x6a},//##quaternary_and_ternary_switching_level
                     {0x0a, 0x55},//##PWM_Duty
                     {0x0b, 0x00},//##CH1_2_configuration
                     {0x0c, 0x1f},//##Top_8_bits_prescale
                     {0x0d, 0xa2},//##Middle_8_bits_prescale
                     {0x0e, 0x3a},//##Bottom_8_bits_prescale
                     {0x0f, 0x20},//##Top_8_bits_prostcale
                     {0x10, 0x00},//##Middle_8_bits_postscale
                     {0x11, 0x00},//##Bottom_8_bits_postscale
                     {0x12, 0x00},//##Magic_Key_High_Byte
                     {0x13, 0x00},//##Magic_Key_Low_Byte
                     {0x14, 0x70},//##Device_Number
                     {0x15, 0x20},//##Top_8_bits_of_Power_Clipping
                     {0x16, 0x00},//##Middle_8_bits_of_Power_Clipping
                     {0x17, 0x00},//##Bottom_8_bits_of_Power_Clipping
                     {0x18, 0x00},//##Noise_Gate
                     {0x19, 0x00},//##Volume_Finetune
                     {0x1a, 0x00},//##Top_8_bits_of_Noise_Gate_Attack_Level
                     {0x1b, 0x00},//##Middle_8_bits_of_Noise_Gate_Attack_Level
                     {0x1c, 0x1a},//##Bottom_8_bits_of_Noise_Gate_Attack_Level
                     {0x1d, 0x00},//##Top_8_bits_of_Noise_Gate_Release_Level
                     {0x1e, 0x00},//##Middle_8_bits_of_Noise_Gate_Release_Level
                     {0x1f, 0x53},//##Bottom_8_bits_of_Noise_Gate_Release_Level
                     {0x20, 0x00},//##Top_8_bits_of_DRC_Energy_Coefficient
                     {0x21, 0x10},//##Bottom_8_bits_of_DRC_Energy_Coefficient
                     {0x22, 0x0e},//##Top_8_bits_of_Compensate_Coefficient_A0
                     {0x23, 0xbf},//##Middle_8_bits_of_Compensate_Coefficient_A0
                     {0x24, 0x37},//##Bottom_8_bits_of_Compensate_Coefficient_A0
                     {0x25, 0x01},//##Top_8_bits_of_Compensate_Coefficient_A1
                     {0x26, 0x73},//##Middle_8_bits_of_Compensate_Coefficient_A1
                     {0x27, 0xba},//##Bottom_8_bits_of_Compensate_Coefficient_A1
                     {0x28, 0xff},//##Top_8_bits_of_Compensate_Coefficient_B1
                     {0x29, 0xcd},//##Middle_8_bits_of_Compensate_Coefficient_B1
                     {0x2a, 0x0f},//##Bottom_8_bits_of_Compensate_Coefficient_B1
                     {0x2b, 0x05},//##TDM_word_width_and_I2S_Data_output_select
                     {0x2c, 0x00},//##TDM_Offset
                     {0x2d, 0x00},//##Top_8_bits_of_DRC_aa
                     {0x2e, 0x04},//##Middle_8_bits_of_DRC_aa
                     {0x2f, 0x00},//##Bottom_8_bits_of_DRC_aa
                     {0x30, 0x00},//##Top_8_bits_of_DRC_da
                     {0x31, 0x01},//##Middle_8_bits_of_DRC_da
                     {0x32, 0x00},//##Bottom_8_bits_of_DRC_da
                     {0x33, 0xf0},//##Top_8_bits_of_DRC_TH
                     {0x34, 0x0e},//##Middle_8_bits_of_DRC_TH
                     {0x35, 0x04},//##Bottom_8_bits_of_DRC_TH
                     {0x36, 0x20},//##Top_8_bits_of_DRC_Slope
                     {0x37, 0x00},//##Middle_8_bits_of_DRC_Slope
                     {0x38, 0x00},//##Bottom_8_bits_of_DRC_Slope
                     {0x39, 0x08},//##Top_8_bits_of_I2S_GAIN
                     {0x3a, 0x00},//##Middle_8_bits_of_I2S_GAIN
                     {0x3b, 0x00},//##Bottom_8_bits_of_I2S_GAIN
                     {0x3d, 0x0e},//##PWM_Shift
                     {0x3e, 0x12},//##FS_and_PMF_read_out
                     {0x3f, 0x01},//##Hi_res
                     {0x40, 0x00},//##MBIST
                     {0x41, 0x55},//##Top_8_bits_of_MBIST_even
                     {0x42, 0x55},//##Middle_8_bits_of_MBIST_even
                     {0x43, 0x55},//##Bottom_8_bits_of_MBIST_even
                     {0x44, 0x55},//##Top_8_bits_of_MBIST_odd
                     {0x45, 0x55},//##Middle_8_bits_of_MBIST_odd
                     {0x46, 0x55},//##Bottom_8_bits_of_MBIST_odd
                     {0x47, 0x00},//##Test_mode_register
                     {0x48, 0x62},//##BSOV_Select
                     {0x49, 0x88},//##OC_Select_2
                     {0x4a, 0x00},//##OC_GVDD_BYP
                     {0x4b, 0x8c},//##Min_duty_test
                     {0x4d, 0x00},//##PLL_Tset
                     {0x4e, 0xff},//##Error_register_status
                     {0x4f, 0xff},//####Error_latch_status
                     {0x50, 0x00},//##Error_clear_register
                     {0x51, 0x30},//##Error_dealy
                     {0x52, 0x00},//##Error_protect
                     {0x53, 0x00},//##Test_mode
                     {0x54, 0x00},//##PWM_CTRL
                     {0x55, 0x00},//##RAM1_base_address
                     {0x56, 0x00},//##Top_8-bits_of_RAM1_coefficients
                     {0x57, 0x00},//##Middle_8-bits_of_RAM1_coefficients
                     {0x58, 0x00},//##Bottom_8-bits_of_RAM1_coefficients
                     {0x59, 0x00},//##RAM1_read/write_control
                     {AD82050_END_FLAG ,0x00}
};
void ad82050_func(int amp_i2c_id,unsigned short addr)
{
    unsigned char data[2];
    unsigned char data_len=0;
    unsigned char index=0;
#ifdef AD82050_DEBUG
    //unsigned char i;
#endif
    //int retry_cnt = 5;

    AMP_ERR("\nAD82050 Initial Start\n");

    // software reset amp
    data[0] = 0x02;
    data[1] = 0x00;
    i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED); //--reset amp
    mdelay(5);

    data[0] = 0x02;
    data[1] = 0x10;
    i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED); //--Normal operation
    mdelay(20);

    data[0] = 0x02;
    data[1] = 0x18;
    i2c_master_send_ex_flag(amp_i2c_id, addr,data, data_len+1 ,I2C_M_FAST_SPEED); //--mute amp
   for(index = 0; index < (sizeof(AD82050_InitTbl)/sizeof(AD82050_REG)); index ++)
    {
        data[0] = AD82050_InitTbl[index].bAddr;
        data_len = 1;
        data[1] = AD82050_InitTbl[index].bArray;
        if(data[0] == AD82050_END_FLAG)
            break;
#ifdef AD82050_DEBUG
        AMP_WARN("===write addr:0x%x len:%d===\n",data[0],data_len);
        AMP_WARN("%x ", data[1]);
#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program AD82050 failed\n");
    }
    data[0] = 0x02;
    data[1] = 0x10;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED) < 0) //--unmute amp
        AMP_ERR("%s line: %d, program AD82050 failed\n", __func__, __LINE__);
    AMP_ERR("\nAD82050 Initial End\n");
}


void ad82050_amp_reset(void)
{

    AMP_WARN("ad82050_amp_reset\n");
    rtk_amp_pin_set("PIN_AMP_RESET", 0);
    mdelay(10);
    rtk_amp_pin_set("PIN_AMP_RESET", 1);
    mdelay(10);
}


int ad82050_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    unsigned char data[2] ={0};

    data[0] = 0x02;
    if(on_off){
        AMP_WARN("AD82050 unmute amp\n");
        data[1] = 0x10;    //--unmute amp
    }
    else{
        AMP_WARN("AD82050 mute amp\n");
        data[1] = 0x18;    //--mute amp
    }
    return i2c_master_send_ex(amp_i2c_id, slave_addr, data, 2); 

}


int ad82050_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
{

    unsigned char send_data[2] ={0};

    send_data[0] = data_wr[0];
    send_data[1] = data_wr[1];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,send_data[0],send_data[1]);
    }

    return 0;
}


int ad82050_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;

    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};

    //Read
    addr[0] = reg[0];
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, addr, AD82050_ADDR_SIZE) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,AD82050_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
        return (-1);
    }

     AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,addr[0],data[0]);

    return ret;
}
