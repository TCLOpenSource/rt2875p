#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif
#include "rtk_amp_interface.h"
#include "ad82120.h"

#ifdef BUILD_QUICK_SHOW
static AD82120_REG AD82120_InitTbl[] = {
        {AD82120_SCTL1, 0x00, "State_Control_1"},           //##State_Control_1
        {AD82120_SCTL2, 0x81, "State_Control_2"},           //##State_Control_2
        {AD82120_SCTL3, 0x7f, "State_Control_3"},           //##State_Control_3
        {AD82120_MVOL,  0x18, "Master_volume_control"},     //##Master_volume_control
        {AD82120_C1VOL, 0x18, "Channel_1_volume_control"},  //##Channel_1_volume_control
        {AD82120_C2VOL, 0x18, "Channel_2_volume_control"},  //##Channel_2_volume_control
        {AD82120_C3VOL, 0x18, "Channel_3_volume_control"},  //##Channel_3_volume_control
        {AD82120_C4VOL, 0x18, "Channel_4_volume_control"},  //##Channel_4_volume_control
        {AD82120_C5VOL, 0x18, "Channel_5_volume_control"},  //##Channel_5_volume_control
        {AD82120_C6VOL, 0x18, "Channel_6_volume_control"},  //##Channel_6_volume_control
        {AD82120_SCTL4, 0x90, "State_Control_4"},           //##State_Control_4
        {AD82120_SCTL6, 0x80, "State_Control_6"},           //##State_Control_6

};
#else
static AD82120_REG AD82120_InitTbl[] = {
        {AD82120_SCTL1, 0x00, "State_Control_1"},           //##State_Control_1
        {AD82120_SCTL2, 0x81, "State_Control_2"},           //##State_Control_2
        {AD82120_SCTL3, 0x7f, "State_Control_3"},           //##State_Control_3
        {AD82120_MVOL,  0x18, "Master_volume_control"},     //##Master_volume_control
        {AD82120_C1VOL, 0x18, "Channel_1_volume_control"},  //##Channel_1_volume_control
        {AD82120_C2VOL, 0x18, "Channel_2_volume_control"},  //##Channel_2_volume_control
        {AD82120_C3VOL, 0x18, "Channel_3_volume_control"},  //##Channel_3_volume_control
        {AD82120_C4VOL, 0x18, "Channel_4_volume_control"},  //##Channel_4_volume_control
        {AD82120_C5VOL, 0x18, "Channel_5_volume_control"},  //##Channel_5_volume_control
        {AD82120_C6VOL, 0x18, "Channel_6_volume_control"},  //##Channel_6_volume_control
        {AD82120_BTONE, 0x00, "Reserve"},                   //##Reserve
        {AD82120_TTONE, 0x00, "Reserve"},                   //##Reserve
        {AD82120_SCTL4, 0x90, "State_Control_4"},           //##State_Control_4
        {AD82120_C1CFG, 0x00, "Channel_1_configuration"},   //##Channel_1_configuration_registers
        {AD82120_C2CFG, 0x00, "Channel_2_configuration"},   //##Channel_2_configuration_registers
        {AD82120_C3CFG, 0x00, "Channel_3_configuration"},   //##Channel_3_configuration_registers
        {AD82120_C4CFG, 0x00, "Channel_4_configuration"},   //##Channel_4_configuration_registers
        {AD82120_C5CFG, 0x00, "Channel_5_configuration"},   //##Channel_5_configuration_registers
        {AD82120_C6CFG, 0x00, "Channel_6_configuration"},   //##Channel_6_configuration_registers
        {AD82120_C7CFG, 0x00, "Channel_7_configuration"},   //##Channel_7_configuration_registers
        {AD82120_C8CFG, 0x00, "Channel_8_configuration"},   //##Channel_8_configuration_registers
        {AD82120_LAR1,  0x6a, "Reserve"},                   //##Reserve
        {AD82120_LAR2,  0x6a, "Reserve"},                   //##Reserve
        {AD82120_LAR3,  0x6a, "Reserve"},                   //##Reserve
        {AD82120_LAR4,  0x6a, "Reserve"},                   //##Reserve
        {AD82120_ERDLY, 0x00, "Reserve"},                   //##Reserve
        {AD82120_SCTL5, 0x28, "State_Control_5"},           //##State_Control_5
        {AD82120_SCTL6, 0x80, "State_Control_6"},           //##State_Control_6
        {AD82120_SCTL7, 0x20, "State_Control_7"},           //##State_Control_7
        {AD82120_CFADDR,0x7f, "RAM_Base_Address"},          //##Coefficient_RAM_Base_Address
        {AD82120_A1CF1, 0x00, "First_4bits_of_coefficients_A1"},    //##First_4bits_of_coefficients_A1
        {AD82120_A1CF2, 0x00, "Second_8bits_of_coefficients_A1"},   //##Second_8bits_of_coefficients_A1
        {AD82120_A1CF3, 0x00, "Third_8bits_of_coefficients_A1"},    //##Third_8bits_of_coefficients_A1
        {AD82120_A1CF4, 0x00, "Fourth_bits_of_coefficients_A1"},    //##Fourth_bits_of_coefficients_A1
        {AD82120_A2CF1, 0x00, "First_4bits_of_coefficients_A2"},    //##First_4bits_of_coefficients_A2
        {AD82120_A2CF2, 0x00, "Second_8bits_of_coefficients_A2"},   //##Second_8bits_of_coefficients_A2
        {AD82120_A2CF3, 0x00, "Third_8bits_of_coefficients_A2"},    //##Third_8bits_of_coefficients_A2
        {AD82120_A2CF4, 0x00, "Fourth_8bits_of_coefficients_A2"},   //##Fourth_8bits_of_coefficients_A2
        {AD82120_B1CF1, 0x00, "First_4bits_of_coefficients_B1"},    //##First_4bits_of_coefficients_B1
        {AD82120_B1CF2, 0x00, "Second_8bits_of_coefficients_B1"},   //##Second_8bits_of_coefficients_B1
        {AD82120_B1CF3, 0x00, "Third_8bits_of_coefficients_B1"},    //##Third_8bits_of_coefficients_B1
        {AD82120_B1CF4, 0x00, "Fourth_8bits_of_coefficients_B1"},   //##Fourth_8bits_of_coefficients_B1
        {AD82120_B2CF1, 0x00, "First_4bits_of_coefficients_B2"},    //##First_4bits_of_coefficients_B2
        {AD82120_B2CF2, 0x00, "Second_8bits_of_coefficients_B2"},   //##Second_8bits_of_coefficients_B2
        {AD82120_B2CF3, 0x00, "Third_8bits_of_coefficients_B2"},    //##Third_8bits_of_coefficients_B2
        {AD82120_B2CF4, 0x00, "Fourth_bits_of_coefficients_B2"},    //##Fourth_bits_of_coefficients_B2
        {AD82120_A0CF1, 0x00, "First_4bits_of_coefficients_A0"},    //##First_4bits_of_coefficients_A0
        {AD82120_A0CF2, 0x80, "Second_8bits_of_coefficients_A0"},   //##Second_8bits_of_coefficients_A0
        {AD82120_A0CF3, 0x00, "Third_8bits_of_coefficients_A0"},    //##Third_8bits_of_coefficients_A0
        {AD82120_A0CF4, 0x00, "Fourth_8bits_of_coefficients_A0"},   //##Fourth_8bits_of_coefficients_A0
        {AD82120_CFRW,  0x00, "Coefficient_RAM_RW_control"},        //##Coefficient_RAM_RW_control
        {AD82120_SCTL8, 0x02, "State_Control_8"},                   //##State_Control_8
        {AD82120_SCTL9, 0xf0, "State_Control_9"},                   //##State_Control_9
        {AD82120_VFT1,  0x00, "Volume_Fine_tune"},                  //##Volume_Fine_tune
        {AD82120_VFT2,  0x00, "Volume_Fine_tune"},                  //##Volume_Fine_tune
        {AD82120_ID,    0x04, "Device_ID_register"},                //##Device_ID_register
        {AD82120_LMC,   0x00, "Level_Meter_Clear"},                 //##Level_Meter_Clear
        {AD82120_PMC,   0x00, "Power_Meter_Clear"},                 //##Power_Meter_Clear
        {AD82120_1STC1LM, 0x00, "First_8bits_of_C1_Level_Meter"},   //##First_8bits_of_C1_Level_Meter
        {AD82120_2NDC1LM, 0x00, "Second_8bits_of_C1_Level_Meter"},  //##Second_8bits_of_C1_Level_Meter
        {AD82120_3RDC1LM, 0xc0, "Third_8bits_of_C1_Level_Meter"},   //##Third_8bits_of_C1_Level_Meter
        {AD82120_4THC1LM, 0x83, "Fourth_8bits_of_C1_Level_Meter"},  //##Fourth_8bits_of_C1_Level_Meter
        {AD82120_1STC2LM, 0x00, "First_8bits_of_C2_Level_Meter"},   //##First_8bits_of_C2_Level_Meter
        {AD82120_2NDC2LM, 0x00, "Second_8bits_of_C2_Level_Meter"},  //##Second_8bits_of_C2_Level_Meter
        {AD82120_3RDC2LM, 0xc0, "Third_8bits_of_C2_Level_Meter"},   //##Third_8bits_of_C2_Level_Meter
        {AD82120_4THC2LM, 0x94, "Fourth_8bits_of_C2_Level_Meter"},  //##Fourth_8bits_of_C2_Level_Meter
        {AD82120_1STC3LM, 0x00, "First_8bits_of_C3_Level_Meter"},   //##First_8bits_of_C3_Level_Meter
        {AD82120_2NDC3LM, 0x00, "Second_8bits_of_C3_Level_Meter"},  //##Second_8bits_of_C3_Level_Meter
        {AD82120_3RDC3LM, 0x00, "Third_8bits_of_C3_Level_Meter"},   //##Third_8bits_of_C3_Level_Meter
        {AD82120_4THC3LM, 0x00, "Third_8bits_of_C3_Level_Meter"},   //##Fourth_8bits_of_C3_Level_Meter
        {AD82120_1STC4LM, 0x00, "First_8bits_of_C4_Level_Meter"},   //##First_8bits_of_C4_Level_Meter
        {AD82120_2NDC4LM, 0x00, "Second_8bits_of_C4_Level_Meter"},  //##Second_8bits_of_C4_Level_Meter
        {AD82120_3RDC4LM, 0x00, "Third_8bits_of_C4_Level_Meter"},   //##Third_8bits_of_C4_Level_Meter
        {AD82120_4THC4LM, 0x00, "Fourth_8bits_of_C4_Level_Meter"},  //##Fourth_8bits_of_C4_Level_Meter
        {AD82120_1STC5LM, 0x00, "First_8bits_of_C5_Level_Meter"},   //##First_8bits_of_C5_Level_Meter
        {AD82120_2NDC5LM, 0x00, "Second_8bits_of_C5_Level_Meter"},  //##Second_8bits_of_C5_Level_Meter
        {AD82120_3RDC5LM, 0x00, "Third_8bits_of_C5_Level_Meter"},   //##Third_8bits_of_C5_Level_Meter
        {AD82120_4THC5LM, 0x00, "Fourth_8bits_of_C5_Level_Meter"},  //##Fourth_8bits_of_C5_Level_Meter
        {AD82120_1STC6LM, 0x00, "First_8bits_of_C6_Level_Meter"},   //##First_8bits_of_C6_Level_Meter
        {AD82120_2NDC6LM, 0x00, "Second_8bits_of_C6_Level_Meter"},  //##Second_8bits_of_C6_Level_Meter
        {AD82120_3RDC6LM, 0x00, "Third_8bits_of_C6_Level_Meter"},   //##Third_8bits_of_C6_Level_Meter
        {AD82120_4THC6LM, 0x00, "Fourth_8bits_of_C6_Level_Meter"},  //##Fourth_8bits_of_C6_Level_Meter
        {AD82120_1STC7LM, 0x00, "First_8bits_of_C7_Level_Meter"},   //##First_8bits_of_C7_Level_Meter
        {AD82120_2NDC7LM, 0x00, "Second_8bits_of_C7_Level_Meter"},  //##Second_8bits_of_C7_Level_Meter
        {AD82120_3RDC7LM, 0x00, "Third_8bits_of_C7_Level_Meter"},   //##Third_8bits_of_C7_Level_Meter
        {AD82120_4THC7LM, 0x00, "Fourth_8bits_of_C7_Level_Meter"},  //##Fourth_8bits_of_C7_Level_Meter
        {AD82120_1STC8LM, 0x00, "First_8bits_of_C8_Level_Meter"},   //##First_8bits_of_C8_Level_Meter
        {AD82120_2NDC8LM, 0x00, "Second_8bits_of_C8_Level_Meter"},  //##Second_8bits_of_C8_Level_Meter
        {AD82120_3RDC8LM, 0x00, "Third_8bits_of_C8_Level_Meter"},   //##Third_8bits_of_C8_Level_Meter
        {AD82120_4THC8LM, 0x00, "Fourth_8bits_of_C8_Level_Meter"},  //##Fourth_8bits_of_C8_Level_Meter
        {AD82120_I2S_OUT, 0x2d, "I2S_data_output_sel_register"},    //##I2S_data_output_section_register
        {AD82120_MKHB,    0x00, "Mono_Key_High_Byte"},              //##Mono_Key_High_Byte
        {AD82120_MKLB,    0x00, "Mono_Key_Low_Byte"},               //##Mono_Key_Low_Byte
        {AD82120_HI_RES,  0x07, "Hires_Item"},                      //##Hires_Item
        {AD82120_ANA_GAIN,0x00, "Analog_gain"},                     //##Analog_gain
        {0x5f, 0x00, "Reserve"},                                    //##Reserve
        {0x60, 0x00, "Reserve"},                                    //##Reserve
        {0x61, 0x00, "Reserve"},                                    //##Reserve
        {0x62, 0x00, "Reserve"},                                    //##Reserve
        {0x63, 0x00, "Reserve"},                                    //##Reserve
        {0x64, 0x00, "Reserve"},                                    //##Reserve
        {0x65, 0x00, "Reserve"},                                    //##Reserve
        {0x66, 0x00, "Reserve"},                                    //##Reserve
        {0x67, 0x00, "Reserve"},                                    //##Reserve
        {0x68, 0x00, "Reserve"},                                    //##Reserve
        {0x69, 0x00, "Reserve"},                                    //##Reserve
        {0x6a, 0x00, "Reserve"},                                    //##Reserve
        {0x6b, 0x00, "Reserve"},                                    //##Reserve
        {AD82120_PMF_FS_R, 0x01, "FS_and_PMF_read_out"},            //##FS_and_PMF_read_out
        {AD82120_OC,       0x00, "OC_level_setting"},               //##OC_level_setting
        {AD82120_DTC,      0x40, "DTC_setting"},                    //##DTC_setting
        {AD82120_TMR0,     0x74, "Testmode_register0"},             //##Testmode_register0
        {0x70,             0x00, "Reserve"},                        //##Reserve
        {AD82120_TMR1,     0x40, "Testmode_register1"},             //##Testmode_register1
        {AD82120_TMR2,     0x38, "Testmode_register2"},             //##Testmode_register2
        {AD82120_Dither,   0x18, "Dither_signal_setting"},          //##Dither_signal_setting
        {AD82120_EDL,      0x06, "Error_delay"},                    //##Error_delay
        {AD82120_MBIST_1ST_E, 0x55, "First_8bits_of_MBIST_Even"},   //##First_8bits_of_MBIST_Even
        {AD82120_MBIST_2ND_E, 0x55, "Second_8bits_of_MBIST_Even"},  //##Second_8bits_of_MBIST_Even
        {AD82120_MBIST_3RD_E, 0x55, "Third_8bits_of_MBIST_Even"},   //##Third_8bits_of_MBIST_Even
        {AD82120_MBIST_4TH_E, 0x55, "Fourth_8bits_of_MBIST_Even"},  //##Fourth_8bits_of_MBIST_Even
        {AD82120_MBIST_1ST_O, 0x55, "First_8bits_of_MBIST_Odd"},    //##First_8bits_of_MBIST_Odd
        {AD82120_MBIST_2ND_O, 0x55, "Second_8bits_of_MBIST_Odd"},   //##Second_8bits_of_MBIST_Odd
        {AD82120_MBIST_3RD_O, 0x55, "Third_8bits_of_MBIST_Odd"},    //##Third_8bits_of_MBIST_Odd
        {AD82120_MBIST_4TH_O, 0x55, "Fourth_8bits_of_MBIST_Odd"},   //##Fourth_8bits_of_MBIST_Odd
        {AD82120_ERR_REG,     0xfe, "Error_register"},              //##Error_register
        {AD82120_ERR_RECORD,  0xfe, "Error_latch_register"},        //##Error_latch_register
        {AD82120_ERR_CLEAR,   0x00, "Error_clear_register"},        //##Error_clear_register
        {0x80, 0x00, "Protection_register_set"},                    //##Protection_register_set
        {0x81, 0x00, "Memory_MBIST_status"},                        //##Memory_MBIST_status
        {0x82, 0x00, "PWM_output_control"},                         //##PWM_output_control
        {0x83, 0x00, "Testmode_control_register"},                  //##Testmode_control_register
        {0x84, 0x00, "RAM1_test_register_address"},                 //##RAM1_test_register_address
        {0x85, 0x00, "First_8bits_of_RAM1_data"},                   //##First_8bits_of_RAM1_data
};
#endif
static unsigned char AD82120_ram1_tab[][5] = {
        {0x00, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A1 
        {0x01, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A2 
        {0x02, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B1 
        {0x03, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B2 
        {0x04, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A0 
        {0x05, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A1 
        {0x06, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A2 
        {0x07, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_B1 
        {0x08, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_B2 
        {0x09, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A0 
        {0x0a, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ3_A1 
        {0x0b, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ3_A2 
        {0x0c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ3_B1 
        {0x0d, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ3_B2 
        {0x0e, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ3_A0 
        {0x0f, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ4_A1 
        {0x10, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ4_A2 
        {0x11, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ4_B1 
        {0x12, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ4_B2 
        {0x13, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ4_A0 
        {0x14, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ5_A1 
        {0x15, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ5_A2 
        {0x16, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ5_B1 
        {0x17, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ5_B2 
        {0x18, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ5_A0 
        {0x19, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ6_A1 
        {0x1a, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ6_A2 
        {0x1b, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ6_B1 
        {0x1c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ6_B2 
        {0x1d, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ6_A0 
        {0x1e, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ7_A1 
        {0x1f, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ7_A2 
        {0x20, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ7_B1 
        {0x21, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ7_B2 
        {0x22, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ7_A0 
        {0x23, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ8_A1 
        {0x24, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ8_A2 
        {0x25, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ8_B1 
        {0x26, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ8_B2 
        {0x27, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ8_A0 
        {0x28, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ9_A1 
        {0x29, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ9_A2 
        {0x2a, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ9_B1 
        {0x2b, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ9_B2 
        {0x2c, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ9_A0 
        {0x2d, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ10_A1 
        {0x2e, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ10_A2 
        {0x2f, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ10_B1 
        {0x30, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ10_B2 
        {0x31, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ10_A0 
        {0x32, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ11_A1 
        {0x33, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ11_A2 
        {0x34, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ11_B1 
        {0x35, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ11_B2 
        {0x36, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ11_A0 
        {0x37, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ12_A1 
        {0x38, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ12_A2 
        {0x39, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ12_B1 
        {0x3a, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ12_B2 
        {0x3b, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ12_A0 
        {0x3c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ13_A1 
        {0x3d, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ13_A2 
        {0x3e, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ13_B1 
        {0x3f, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ13_B2 
        {0x40, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ13_A0 
        {0x41, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ14_A1 
        {0x42, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ14_A2 
        {0x43, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ14_B1 
        {0x44, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ14_B2 
        {0x45, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ14_A0 
        {0x46, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ15_A1 
        {0x47, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ15_A2 
        {0x48, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ15_B1 
        {0x49, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ15_B2 
        {0x4a, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ15_A0 
        {0x4b, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ16_A1 
        {0x4c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ16_A2 
        {0x4d, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ16_B1 
        {0x4e, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ16_B2 
        {0x4f, 0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ16_A0 
        {0x50, 0x07, 0xff, 0xff, 0xf0},//##Channel_1_Mixer1 
        {0x51, 0x00, 0x00, 0x00, 0x00},//##Channel_1_Mixer2 
        {0x52, 0x00, 0x7e, 0x88, 0xe0},//##Channel_1_Prescale 
        {0x53, 0x02, 0x00, 0x00, 0x00},//##Channel_1_Postscale 
        {0x54, 0x02, 0x00, 0x00, 0x00},//##CH1.2_Power_Clipping 
        {0x55, 0x00, 0x00, 0x01, 0xa0},//##Noise_Gate_Attack_Level 
        {0x56, 0x00, 0x00, 0x05, 0x30},//##Noise_Gate_Release_Level 
        {0x57, 0x00, 0x01, 0x00, 0x00},//##DRC1_Energy_Coefficient 
        {0x58, 0x00, 0x01, 0x00, 0x00},//##DRC2_Energy_Coefficient 
        {0x59, 0x00, 0x01, 0x00, 0x00},//##DRC3_Energy_Coefficient 
        {0x5a, 0x00, 0x01, 0x00, 0x00},//##DRC4_Energy_Coefficient 
        {0x5b, 0x00, 0x00, 0x19, 0x04},//##DRC1_Power_Meter 
        {0x5c, 0x00, 0x00, 0x00, 0x00},//##DRC3_Power_Meter 
        {0x5d, 0x00, 0x00, 0x00, 0x00},//##DRC5_Power_Meter 
        {0x5e, 0x00, 0x00, 0x00, 0x00},//##DRC7_Power_Meter 
        {0x5f, 0x02, 0x00, 0x00, 0x00},//##Channel_1_DRC_GAIN1 
        {0x60, 0x02, 0x00, 0x00, 0x00},//##Channel_1_DRC_GAIN2 
        {0x61, 0x02, 0x00, 0x00, 0x00},//##Channel_1_DRC_GAIN3 
        {0x62, 0x0e, 0x01, 0xc0, 0x70},//##DRC1_FF_threshold 
        {0x63, 0x02, 0x00, 0x00, 0x00},//##DRC1_FF_slope 
        {0x64, 0x00, 0x00, 0x40, 0x00},//##DRC1_FF_aa 
        {0x65, 0x00, 0x00, 0x10, 0x00},//##DRC1_FF_da 
        {0x66, 0x0e, 0x01, 0xc0, 0x70},//##DRC2_FF_threshold
        {0x67, 0x02, 0x00, 0x00, 0x00},//##DRC2_FF_slope 
        {0x68, 0x00, 0x00, 0x40, 0x00},//##DRC2_FF_aa 
        {0x69, 0x00, 0x00, 0x10, 0x00},//##DRC2_FF_da 
        {0x6a, 0x0e, 0x01, 0xc0, 0x70},//##DRC3_FF_threshold 
        {0x6b, 0x02, 0x00, 0x00, 0x00},//##DRC3_FF_slope 
        {0x6c, 0x00, 0x00, 0x40, 0x00},//##DRC3_FF_aa 
        {0x6d, 0x00, 0x00, 0x10, 0x00},//##DRC3_FF_da 
        {0x6e, 0x0e, 0x01, 0xc0, 0x70},//##DRC4_FF_threshold 
        {0x6f, 0x02, 0x00, 0x00, 0x00},//##DRC4_FF_slope 
        {0x70, 0x00, 0x00, 0x40, 0x00},//##DRC4_FF_aa 
        {0x71, 0x00, 0x00, 0x10, 0x00},//##DRC4_FF_da 
        {0x72, 0x00, 0x7f, 0xe0, 0x01},//##DRC1_gain 
        {0x73, 0x00, 0x00, 0x00, 0x00},//##DRC3_gain 
        {0x74, 0x00, 0x00, 0x00, 0x00},//##DRC5_gain 
        {0x75, 0x00, 0x00, 0x00, 0x00},//##DRC7_gain 
        {0x76, 0x00, 0x80, 0x00, 0x00},//##I2SO_LCH_gain 
        {0x77, 0x02, 0x00, 0x00, 0x00},//##SRS_gain 
        {0x78, 0x00, 0x00, 0x00, 0x00},//##State_Control_1
        {0x79, 0x00, 0x00, 0x00, 0x00},//##State_Control_2
        {0x7a, 0x00, 0x00, 0x00, 0x00},//##State_Control_3
        {0x7b, 0x00, 0x00, 0x00, 0x00},//##Master_volume_control
        {0x7c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_volume_control
        {0x7d, 0x00, 0x00, 0x00, 0x00},//##Channel_2_volume_control
        {0x7e, 0x00, 0x00, 0x00, 0x00},//##Channel_3_volume_control
        {0x7f, 0x00, 0x00, 0x00, 0x00},//##Channel_4_volume_control
};

static unsigned char AD82120_ram2_tab[][5]= {
        {0x00, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ1_A1 
        {0x01, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ1_A2 
        {0x02, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ1_B1 
        {0x03, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ1_B2 
        {0x04, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ1_A0 
        {0x05, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ2_A1 
        {0x06, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ2_A2 
        {0x07, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ2_B1 
        {0x08, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ2_B2 
        {0x09, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ2_A0 
        {0x0a, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ3_A1 
        {0x0b, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ3_A2 
        {0x0c, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ3_B1 
        {0x0d, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ3_B2 
        {0x0e, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ3_A0 
        {0x0f, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ4_A1 
        {0x10, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ4_A2 
        {0x11, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ4_B1 
        {0x12, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ4_B2 
        {0x13, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ4_A0 
        {0x14, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ5_A1 
        {0x15, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ5_A2 
        {0x16, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ5_B1 
        {0x17, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ5_B2 
        {0x18, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ5_A0 
        {0x19, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ6_A1 
        {0x1a, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ6_A2 
        {0x1b, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ6_B1 
        {0x1c, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ6_B2 
        {0x1d, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ6_A0 
        {0x1e, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ7_A1 
        {0x1f, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ7_A2 
        {0x20, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ7_B1 
        {0x21, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ7_B2 
        {0x22, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ7_A0 
        {0x23, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ8_A1 
        {0x24, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ8_A2 
        {0x25, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ8_B1 
        {0x26, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ8_B2 
        {0x27, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ8_A0 
        {0x28, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ9_A1 
        {0x29, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ9_A2 
        {0x2a, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ9_B1 
        {0x2b, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ9_B2 
        {0x2c, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ9_A0 
        {0x2d, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ10_A1 
        {0x2e, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ10_A2 
        {0x2f, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ10_B1 
        {0x30, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ10_B2 
        {0x31, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ10_A0 
        {0x32, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ11_A1 
        {0x33, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ11_A2 
        {0x34, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ11_B1 
        {0x35, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ11_B2 
        {0x36, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ11_A0 
        {0x37, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ12_A1 
        {0x38, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ12_A2 
        {0x39, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ12_B1 
        {0x3a, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ12_B2 
        {0x3b, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ12_A0 
        {0x3c, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ13_A1 
        {0x3d, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ13_A2 
        {0x3e, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ13_B1 
        {0x3f, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ13_B2 
        {0x40, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ13_A0 
        {0x41, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ14_A1 
        {0x42, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ14_A2 
        {0x43, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ14_B1 
        {0x44, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ14_B2 
        {0x45, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ14_A0 
        {0x46, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ15_A1 
        {0x47, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ15_A2 
        {0x48, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ15_B1 
        {0x49, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ15_B2 
        {0x4a, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ15_A0 
        {0x4b, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ16_A1 
        {0x4c, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ16_A2 
        {0x4d, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ16_B1 
        {0x4e, 0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ16_B2 
        {0x4f, 0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ16_A0 
        {0x50, 0x00, 0x00, 0x00, 0x00},//##Channel_2_Mixer1 
        {0x51, 0x07, 0xff, 0xff, 0xf0},//##Channel_2_Mixer2 
        {0x52, 0x00, 0x7e, 0x88, 0xe0},//##Channel_2_Prescale 
        {0x53, 0x02, 0x00, 0x00, 0x00},//##Channel_2_Postscale 
        {0x54, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x55, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x56, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x57, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x58, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x59, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x5a, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x5b, 0x00, 0x00, 0x18, 0x88},//##DRC2_Power_Meter
        {0x5c, 0x00, 0x00, 0x00, 0x00},//##DRC4_Power_Mete
        {0x5d, 0x00, 0x00, 0x00, 0x00},//##DRC6_Power_Meter
        {0x5e, 0x00, 0x00, 0x00, 0x00},//##DRC8_Power_Meter
        {0x5f, 0x02, 0x00, 0x00, 0x00},//##Channel_2_DRC_GAIN1 
        {0x60, 0x02, 0x00, 0x00, 0x00},//##Channel_2_DRC_GAIN2 
        {0x61, 0x02, 0x00, 0x00, 0x00},//##Channel_2_DRC_GAIN3 
        {0x62, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x63, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x64, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x65, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x66, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x67, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x68, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x69, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6a, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6b, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6c, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6d, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6e, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x6f, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x70, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x71, 0x00, 0x00, 0x00, 0x00},//##Reserve
        {0x72, 0x00, 0x7f, 0xe0, 0x01},//##DRC2_gain 
        {0x73, 0x00, 0x00, 0x00, 0x00},//##DRC4_gain 
        {0x74, 0x00, 0x00, 0x00, 0x00},//##DRC6_gain 
        {0x75, 0x00, 0x00, 0x00, 0x00},//##DRC8_gain 
        {0x76, 0x00, 0x80, 0x00, 0x00},//##I2SO_RCH_gain 
        {0x77, 0x00, 0x00, 0x00, 0x00},//##Reserve 
        {0x78, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A1 
        {0x79, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A2 
        {0x7a, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B1 
        {0x7b, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B2 
        {0x7c, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A0 
        {0x7d, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A1 
        {0x7e, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A2 
        {0x7f, 0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ2_B1 
};



void ad82120_write_ram(int amp_i2c_id,unsigned short slave_addr)
{
    int i = 0;
    unsigned char data[2];
    for(i=0; i<(sizeof(AD82120_ram1_tab)/5);i++)
    {
        data[0] = AD82120_CFADDR;
        data[1] = AD82120_ram1_tab[i][0];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF1;
        data[1] = AD82120_ram1_tab[i][1];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF2;
        data[1] = AD82120_ram1_tab[i][2];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF3;
        data[1] = AD82120_ram1_tab[i][3];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF4;
        data[1] = AD82120_ram1_tab[i][4];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_CFRW;
        data[1] = 0x01;
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");
    }
 
    for(i=0; i<(sizeof(AD82120_ram2_tab)/5);i++)
    {
        data[0] = AD82120_CFADDR;
        data[1] = AD82120_ram2_tab[i][0];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF1;
        data[1] = AD82120_ram2_tab[i][1];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF2;
        data[1] = AD82120_ram2_tab[i][2];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF3;
        data[1] = AD82120_ram2_tab[i][3];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_A1CF4;
        data[1] = AD82120_ram2_tab[i][4];
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");

        data[0] = AD82120_CFRW;
        data[1] = 0x41;
        if(i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data,2 ,I2C_M_FAST_SPEED)<0)
            AMP_ERR("program AD82120 failed\n");
    }
    
}

void ad82120_func(int amp_i2c_id,unsigned short addr)
{
    unsigned char data[2];
    unsigned char data_len=0;
    unsigned char index=0;
#ifdef AD82120_DEBUG
    unsigned char i;
#endif
    int retry_cnt = 5;

    AMP_NOTICE("AD82120 Initial Start\n");

    // software reset amp
    data[0] = 0x1a;
    data[1] = 0x00;
    for (index = 0; index <= retry_cnt; index++) {
        if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) { //--reset amp
            AMP_ERR("%s line: %d, program AD82120 failed (retry=%d)\n", __func__, __LINE__, index);
            mdelay(5);
            if(index == retry_cnt) return;
        }
        else {
            break;
        }
    }
    mdelay(5);

    data[0] = 0x1a;
    data[1] = 0x28;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) //--Normal operation
        AMP_ERR("%s line: %d, program AD82120 failed\n", __func__, __LINE__);
    mdelay(20);

    data[0] = 0x02;
    data[1] = 0x7f;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1, I2C_M_FAST_SPEED) < 0) //--mute amp
        AMP_ERR("%s line: %d, program AD82120 failed\n", __func__, __LINE__);

    for(index = 0; index < (sizeof(AD82120_InitTbl)/sizeof(AD82120_REG)); index ++)
    {
        data[0] = AD82120_InitTbl[index].bAddr;
        data_len = 1;
        data[1] = AD82120_InitTbl[index].bArray;
        if(data[0] == 0x02)
            continue;
        if(data[0] >= 0x71 && data[0]<= 0x7C)
            continue;
#ifdef AD82120_DEBUG
        AMP_WARN("===write addr:0x%x len:%d===\n",data[0],data_len);
        AMP_WARN("%x ", data[1]);
#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program AD82120 failed\n");
    }

    data[0] = 0x02;
    data[1] = 0x7f;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED) < 0) //--mute amp
        AMP_ERR("%s line: %d, program AD82120 failed\n", __func__, __LINE__);
    udelay(100);

   // ad82120_write_ram(amp_i2c_id ,addr);
    //udelay(100);

    data[0] = 0x02;
    data[1] = 0x00;
    if (i2c_master_send_ex_flag(amp_i2c_id, addr, data, data_len+1 ,I2C_M_FAST_SPEED) < 0) //--unmute amp
        AMP_ERR("%s line: %d, program AD82120 failed\n", __func__, __LINE__);
    AMP_NOTICE("AD82120 Initial End\n");
}

int ad82120_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int i = 0;
    unsigned char addr[1] ={0};
    unsigned char data[2] ={0};
    //Read Register
    AMP_WARN("== Dump 00~83 Reg ==\n");
    for (i=0x00; i<(sizeof(AD82120_InitTbl)/sizeof(AD82120_REG)) ;++i){
        addr[0] = AD82120_InitTbl[i].bAddr;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }

        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,1);
        AMP_WARN(":%s,Reg:0x%x Data:0x%x\n", AD82120_InitTbl[i].reg_name,addr[0],data[0]);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            break;
       }
    }

    return ret;
}




int ad82120_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr)
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
int ad82120_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
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
int ad82120_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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




