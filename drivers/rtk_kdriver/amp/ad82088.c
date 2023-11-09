#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <no_os/slab.h>
#include <timer.h>
#endif
#include "rtk_amp_interface.h"
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/pcbMgr.h>
#include "ad82088.h"

static AD82088_REG AD82088_InitTbl[] = {
		{0x00, 0x04},//##State_Control_1
		{0x01, 0x81},//##State_Control_2
		{0x02, 0x7f},//##State_Control_3
		{0x03, 0x14},//##Master_volume_control
		{0x04, 0x18},//##Channel_1_volume_control
		{0x05, 0x18},//##Channel_2_volume_control
		{0x06, 0x18},//##Channel_3_volume_control
		{0x07, 0x18},//##Channel_4_volume_control
		{0x08, 0x18},//##Channel_5_volume_control
		{0x09, 0x18},//##Channel_6_volume_control
		{0x0a, 0x10},//##Bass_Tone_Boost_and_Cut
		{0x0b, 0x10},//##treble_Tone_Boost_and_Cut
		{0x0c, 0x98},//##State_Control_4
		{0x0d, 0x00},//##Channel_1_configuration_registers
		{0x0e, 0x00},//##Channel_2_configuration_registers
		{0x0f, 0x00},//##Channel_3_configuration_registers
		{0x10, 0x00},//##Channel_4_configuration_registers
		{0x11, 0x00},//##Channel_5_configuration_registers
		{0x12, 0x00},//##Channel_6_configuration_registers
		{0x13, 0x00},//##Channel_7_configuration_registers
		{0x14, 0x00},//##Channel_8_configuration_registers
		{0x15, 0x6a},//##DRC1_limiter_attack/release_rate
		{0x16, 0x6a},//##DRC2_limiter_attack/release_rate
		{0x17, 0x6a},//##DRC3_limiter_attack/release_rate
		{0x18, 0x6a},//##DRC4_limiter_attack/release_rate
		{0x19, 0x06},//##Error_Delay
		{0x1a, 0x32},//##State_Control_5
		{0x1b, 0x01},//##HVUV_selection
		{0x1c, 0x00},//##State_Control_6
		{0x1d, 0x7f},//##Coefficient_RAM_Base_Address
		{0x1e, 0x00},//##Top_8-bits_of_coefficients_A1
		{0x1f, 0x00},//##Middle_8-bits_of_coefficients_A1
		{0x20, 0x00},//##Bottom_8-bits_of_coefficients_A1
		{0x21, 0x00},//##Top_8-bits_of_coefficients_A2
		{0x22, 0x00},//##Middle_8-bits_of_coefficients_A2
		{0x23, 0x00},//##Bottom_8-bits_of_coefficients_A2
		{0x24, 0x00},//##Top_8-bits_of_coefficients_B1
		{0x25, 0x00},//##Middle_8-bits_of_coefficients_B1
		{0x26, 0x00},//##Bottom_8-bits_of_coefficients_B1
		{0x27, 0x00},//##Top_8-bits_of_coefficients_B2
		{0x28, 0x00},//##Middle_8-bits_of_coefficients_B2
		{0x29, 0x00},//##Bottom_8-bits_of_coefficients_B2
		{0x2a, 0x40},//##Top_8-bits_of_coefficients_A0
		{0x2b, 0x00},//##Middle_8-bits_of_coefficients_A0
		{0x2c, 0x00},//##Bottom_8-bits_of_coefficients_A0
		{0x2d, 0x40},//##Coefficient_R/W_control
		{0x2e, 0x00},//##Protection_Enable/Disable
		{0x2f, 0x00},//##Memory_BIST_status
		{0x30, 0x00},//##Power_Stage_Status(Read_only)
		{0x31, 0x00},//##PWM_Output_Control
		{0x32, 0x00},//##Test_Mode_Control_Reg.
		{0x33, 0x6d},//##Qua-Ternary/Ternary_Switch_Level
		{0x34, 0x00},//##Volume_Fine_tune
		{0x35, 0x00},//##Volume_Fine_tune
		{0x36, 0x60},//##OC_bypass_&_GVDD_selection
		{0x37, 0x52},//##Device_ID_register
		{0x38, 0x00},//##RAM1_test_register_address
		{0x39, 0x00},//##Top_8-bits_of_RAM1_Data
		{0x3a, 0x00},//##Middle_8-bits_of_RAM1_Data
		{0x3b, 0x00},//##Bottom_8-bits_of_RAM1_Data
		{0x3c, 0x00},//##RAM1_test_r/w_control
		{0x3d, 0x00},//##RAM2_test_register_address
		{0x3e, 0x00},//##Top_8-bits_of_RAM2_Data
		{0x3f, 0x00},//##Middle_8-bits_of_RAM2_Data
		{0x40, 0x00},//##Bottom_8-bits_of_RAM2_Data
		{0x41, 0x00},//##RAM2_test_r/w_control
		{0x42, 0x00},//##Level_Meter_Clear
		{0x43, 0x00},//##Power_Meter_Clear
		{0x44, 0x20},//##TOP_of_C1_Level_Meter
		{0x45, 0x00},//##Middle_of_C1_Level_Meter
		{0x46, 0x00},//##Bottom_of_C1_Level_Meter
		{0x47, 0x20},//##TOP_of_C2_Level_Meter
		{0x48, 0x00},//##Middle_of_C2_Level_Meter
		{0x49, 0x00},//##Bottom_of_C2_Level_Meter
		{0x4a, 0x00},//##TOP_of_C3_Level_Meter
		{0x4b, 0x00},//##Middle_of_C3_Level_Meter
		{0x4c, 0x00},//##Bottom_of_C3_Level_Meter
		{0x4d, 0x00},//##TOP_of_C4_Level_Meter
		{0x4e, 0x00},//##Middle_of_C4_Level_Meter
		{0x4f, 0x00},//##Bottom_of_C4_Level_Meter
		{0x50, 0x00},//##TOP_of_C5_Level_Meter
		{0x51, 0x00},//##Middle_of_C5_Level_Meter
		{0x52, 0x00},//##Bottom_of_C5_Level_Meter
		{0x53, 0x00},//##TOP_of_C6_Level_Meter
		{0x54, 0x00},//##Middle_of_C6_Level_Meter
		{0x55, 0x00},//##Bottom_of_C6_Level_Meter
		{0x56, 0x00},//##TOP_of_C7_Level_Meter
		{0x57, 0x00},//##Middle_of_C7_Level_Meter
		{0x58, 0x00},//##Bottom_of_C7_Level_Meter
		{0x59, 0x00},//##TOP_of_C8_Level_Meter
		{0x5a, 0x00},//##Middle_of_C8_Level_Meter
		{0x5b, 0x00},//##Bottom_of_C8_Level_Meter
		{0x5c, 0x06},//##I2S_Data_Output_Selection_Register
		{0x5d, 0x00},//##Reserve
		{0x5e, 0x00},//##Reserve
		{0x5f, 0x00},//##Reserve
		{0x60, 0x00},//##Reserve
		{0x61, 0x00},//##Reserve
		{0x62, 0x00},//##Reserve
		{0x63, 0x00},//##Reserve
		{0x64, 0x00},//##Reserve
		{0x65, 0x00},//##Reserve
		{0x66, 0x00},//##Reserve
		{0x67, 0x00},//##Reserve
		{0x68, 0x00},//##Reserve
		{0x69, 0x00},//##Reserve
		{0x6a, 0x00},//##Reserve
		{0x6b, 0x00},//##Reserve
		{0x6c, 0x00},//##Reserve
		{0x6d, 0x00},//##Reserve
		{0x6e, 0x00},//##Reserve
		{0x6f, 0x00},//##Reserve
		{0x70, 0x00},//##Reserve
		{0x71, 0x00},//##Reserve
		{0x72, 0x00},//##Reserve
		{0x73, 0x00},//##Reserve
		{0x74, 0x00},//##Mono_Key_High_Byte
		{0x75, 0x00},//##Mono_Key_Low_Byte
		{0x76, 0x00},//##Boost_Control
		{0x77, 0x07},//##Hi-res_Item
		{0x78, 0x40},//##Test_Mode_register
		{0x79, 0x62},//##Boost_Strap_OV/UV_Selection
		{0x7a, 0x8c},//##OC_Selection_2
		{0x7b, 0x55},//##MBIST_User_Program_Top_Byte_Even
		{0x7c, 0x55},//##MBIST_User_Program_Middle_Byte_Even
		{0x7d, 0x55},//##MBIST_User_Program_Bottom_Byte_Even
		{0x7e, 0x55},//##MBIST_User_Program_Top_Byte_Odd
		{0x7f, 0x55},//##MBIST_User_Program_Middle_Byte_Odd
		{0x80, 0x55},//##MBIST_User_Program_Bottom_Byte_Odd
		{0x81, 0x00},//##ERROR_clear_register
		{0x82, 0x0c},//##Minimum_duty_test
		{0x83, 0x06},//##Reserve
		{0x84, 0xfe},//##Reserve
		{0x85, 0xfe},//##Reserve
};

void ad82088_amp_reset(void)
{
    mdelay(30);
    AMP_WARN("Ad82088_amp_reset\n");
    rtk_amp_pin_set("PIN_AMP_RESET", 0);
    rtk_amp_pin_set("PIN_AMP_MUTE", 0);
    mdelay(10);
    rtk_amp_pin_set("PIN_AMP_RESET", 1);
    mdelay(1);
    rtk_amp_pin_set("PIN_AMP_MUTE", 1);
    mdelay(30);
}

void ad82088_amp_mute_set(int value)
{
    int ret = 0;
    unsigned long long amp_mute_value = 0;
    ret = pcb_mgr_get_enum_info_byname("PIN_AMP_MUTE",&amp_mute_value);
    if(ret == 0){
        rtk_amp_pin_set("PIN_AMP_MUTE", 1);
    }
}

void ad82088_func(int amp_i2c_id,unsigned short addr)
{
    unsigned char data[2];
    unsigned char data_len=0;
    unsigned char index=0;
#ifdef AD82088_DEBUG
    unsigned char i;
#endif
    int retry_cnt = 5;

    ad82088_amp_mute_set(1);
    mdelay(20);

    AMP_ERR("\nAD82088 Initial Start\n");

    // software reset amp
    data[0] = 0x1a;
    data[1] = 0x12;
    for (index = 0; index <= retry_cnt; index++) {
        if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) { //--reset amp
            AMP_ERR("%s line: %d, program AD82088 failed (retry=%d)\n", __func__, __LINE__, index);
            mdelay(5);
            if(index == retry_cnt) return;
        }
        else {
            break;
        }
    }
    mdelay(10);

    data[0] = 0x1a;
    data[1] = 0x32;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) //--Normal operation
        AMP_ERR("%s line: %d, program AD82088 failed\n", __func__, __LINE__);
    mdelay(25);

    data[0] = 0x02;
    data[1] = 0x7f;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) //--mute amp
        AMP_ERR("%s line: %d, program AD82088 failed\n", __func__, __LINE__);

    for(index = 0; index < (sizeof(AD82088_InitTbl)/sizeof(AD82088_REG)); index ++)
    {
        data[0] = AD82088_InitTbl[index].bAddr;
        data_len = 1;
        data[1] = AD82088_InitTbl[index].bArray;
        if(data[0] == 0x02)
            continue;

#ifdef AD82088_DEBUG
        AMP_WARN("===write addr:0x%x len:%d===\n",data[0],data_len);
        AMP_WARN("%x ", data[1]);
#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program AD82088 failed\n");
    }

    data[0] = 0x02;
    data[1] = 0x7f;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED) < 0) //--mute amp
        AMP_ERR("%s line: %d, program AD82088 failed\n", __func__, __LINE__);
    mdelay(5);

   // ad82088_write_ram(amp_i2c_id ,addr);
    //udelay(100);

    data[0] = 0x02;
    data[1] = 0x00;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED) < 0) //--unmute amp
        AMP_ERR("%s line: %d, program AD82088 failed\n", __func__, __LINE__);
    AMP_ERR("\nAD82088 Initial End\n");
}

int ad82088_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr)
{
    return 0;
}

int ad82088_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2] ={0};

    data[0] = 0x02;
    if(on_off){
        AMP_WARN("AD82120 unmute amp\n");
        data[1] = 0x00;    //--unmute amp
    }
    else{
        AMP_WARN("AD82120 mute amp\n");
        data[1] = 0x7f;    //--mute amp
    }
    return i2c_master_send_ex(amp_i2c_id, slave_addr, data, 2); 
}

int ad82088_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};

    //Read
    addr[0] = reg[0];
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,AD82120_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",*reg);
        return (-1);
    }

    AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,addr[0],data[0]);
    return ret;
}

int ad82088_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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
