#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#include <malloc.h>
#endif

//*************************************************************
// PUBLIC FUNCTIONS
//*************************************************************
#define AMP_DEBUG   (0)

typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *bArray;
}TAS5711_REG;

unsigned char    TAS5711_Reg06Value[]={0x00};
unsigned char    TAS5711_Reg0AValue[]={0xff};
unsigned char    TAS5711_Reg05Value[]={0x02};
#ifdef Board_AMP_POWER_8OHM
unsigned char    TAS5711_Reg08Value[]={0x1a};
unsigned char    TAS5711_Reg09Value[]={0x1a};
#else  //6ohm
unsigned char    TAS5711_Reg08Value[]={0x30};
unsigned char    TAS5711_Reg09Value[]={0x30};
#endif
unsigned char    TAS5711_Reg0EValue[]={0xD1};
unsigned char    TAS5711_Reg19Value[]={0x3A};
unsigned char    TAS5711_Reg1BValue[]={0x00};
#ifdef Board_AMP_POWER_8OHM
unsigned char    TAS5711_Reg07Value[]={0x30};
#else  //6ohm
unsigned char    TAS5711_Reg07Value[]={0xff};
#endif
unsigned char    TAS5711_Reg14Value[]={0x48};
unsigned char    TAS5711_Reg13Value[]={0xA0};
unsigned char    TAS5711_Reg12Value[]={0x60};
unsigned char    TAS5711_Reg11Value[]={0xB8};
unsigned char    TAS5711_Reg03Value[]={0x80};
unsigned char    TAS5711_Reg1AValue[]={0X0F};
unsigned char    TAS5711_Reg20Value[]={0x00,0x89,0x77,0x72};
unsigned char    TAS5711_Reg21Value[]={0x00,0x00,0x42,0x03};
unsigned char    TAS5711_Reg25Value[]={0x01,0x10,0x32,0x45};
unsigned char    TAS5711_Reg50Value[]={0x00,0x00,0x00,0x00};
unsigned char    TAS5711_Reg51Value[]={0x00,0x40,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char    TAS5711_Reg10Value[]={0x02};
unsigned char    TAS5711_Reg0BValue[]={0x00};
unsigned char    TAS5711_Reg1CValue[]={0x02};
unsigned char    TAS5711_Reg0DValue[]={0x35};
unsigned char    TAS5711_Reg39Value[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char    TAS5711_Reg29Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2AValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2BValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2CValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2DValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2EValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg2FValue[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg30Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg31Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg32Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg33Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg34Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg35Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg36Value[]={0x00,0x00,0x08,0x92,0x00,0x00,0x11,0x25,0x00,0x00,0x08,0x92,0x00,0xFA,0x13,0x89,0x0F,0x85,0xCA,0x2B};
unsigned char    TAS5711_Reg5AValue[]={0x00,0x00,0x01,0x64,0x00,0x00,0x02,0xC8,0x00,0x00,0x01,0x64,0x00,0xFD,0xA1,0x53,0x0F,0x82,0x59,0x1D};
unsigned char    TAS5711_Reg5BValue[]={0x00,0x00,0x01,0x64,0x00,0x00,0x02,0xC8,0x00,0x00,0x01,0x64,0x00,0xFD,0xA1,0x53,0x0F,0x82,0x59,0x1D};
unsigned char    TAS5711_Reg3AValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char    TAS5711_Reg3BValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char    TAS5711_Reg3CValue[]={0x00,0x00,0x00,0xF9,0x00,0x7F,0xFF,0x06};
unsigned char    TAS5711_Reg3DValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char    TAS5711_Reg3EValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char    TAS5711_Reg3FValue[]={0x00,0x00,0x00,0xF9,0x00,0x7F,0xFF,0x06};
unsigned char    TAS5711_Reg40Value[]={0xFD,0xCC,0x99,0xDB};
unsigned char    TAS5711_Reg41Value[]={0x0F,0x81,0x47,0xAF};
unsigned char    TAS5711_Reg42Value[]={0x00,0x08,0x42,0x10};
unsigned char    TAS5711_Reg43Value[]={0xFD,0xE1,0xDC,0x81};
unsigned char    TAS5711_Reg44Value[]={0x0F,0x83,0x33,0x34};
unsigned char    TAS5711_Reg45Value[]={0x00,0x08,0x42,0x10};
unsigned char    TAS5711_Reg46Value[]={0x00,0x00,0x00,0x03};

static TAS5711_REG TAS5711_InitTbl[] =
{
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_OSCILLATOR_TRIM,sizeof(TAS5711_Reg1BValue),TAS5711_Reg1BValue},
    {TAS5711_SOFT_MUTE,sizeof(TAS5711_Reg06Value),TAS5711_Reg06Value},
    {TAS5711_CHANNEL_3_VOL,sizeof(TAS5711_Reg0AValue),TAS5711_Reg0AValue},
    {TAS5711_SYSTEM_CTRL1,sizeof(TAS5711_Reg03Value),TAS5711_Reg03Value},
    {TAS5711_CHANNEL_1_VOL,sizeof(TAS5711_Reg08Value),TAS5711_Reg08Value},
    {TAS5711_CHANNEL_2_VOL,sizeof(TAS5711_Reg09Value),TAS5711_Reg09Value},
    {TAS5711_IC_DELAY_CHANNEL4,sizeof(TAS5711_Reg14Value),TAS5711_Reg14Value},
    {TAS5711_IC_DELAY_CHANNEL3,sizeof(TAS5711_Reg13Value),TAS5711_Reg13Value},
    {TAS5711_IC_DELAY_CHANNEL2,sizeof(TAS5711_Reg12Value),TAS5711_Reg12Value},
    {TAS5711_IC_DELAY_CHANNEL1,sizeof(TAS5711_Reg11Value),TAS5711_Reg11Value},
    {TAS5711_VOLUME_CONFIG,sizeof(TAS5711_Reg0EValue),TAS5711_Reg0EValue},
    {TAS5711_START_STOP_PERIOD,sizeof(TAS5711_Reg1AValue),TAS5711_Reg1AValue},
    {TAS5711_INPUT_MUX,sizeof(TAS5711_Reg20Value),TAS5711_Reg20Value},
    {TAS5711_CH4_SOURCE_SELECT,sizeof(TAS5711_Reg21Value),TAS5711_Reg21Value},
    {TAS5711_PWM_MUX,sizeof(TAS5711_Reg25Value),TAS5711_Reg25Value},
    {TAS5711_MODULATION_LIMIT,sizeof(TAS5711_Reg10Value),TAS5711_Reg10Value},
    {0x0B,sizeof(TAS5711_Reg0BValue),TAS5711_Reg0BValue},
    {TAS5711_BKND_ERR,sizeof(TAS5711_Reg1CValue),TAS5711_Reg1CValue},
    {TAS5711_BKND_ERR,sizeof(TAS5711_Reg19Value),TAS5711_Reg19Value},
    {TAS5711_BANK_SWITCH_CTRL,sizeof(TAS5711_Reg50Value),TAS5711_Reg50Value},
    {TAS5711_CH1_OUTPUT_MIXER,sizeof(TAS5711_Reg51Value),TAS5711_Reg51Value},
    {TAS5711_DRC1_AE,sizeof(TAS5711_Reg3AValue),TAS5711_Reg3AValue},
    {TAS5711_DRC1_AA,sizeof(TAS5711_Reg3BValue),TAS5711_Reg3BValue},
    {TAS5711_DRC1_AD,sizeof(TAS5711_Reg3CValue),TAS5711_Reg3CValue},
    {TAS5711_DRC1_T,sizeof(TAS5711_Reg40Value),TAS5711_Reg40Value},
    {TAS5711_DRC1_K,sizeof(TAS5711_Reg41Value),TAS5711_Reg41Value},
    {TAS5711_DRC1_O,sizeof(TAS5711_Reg42Value),TAS5711_Reg42Value},
    {TAS5711_DRC2_AE,sizeof(TAS5711_Reg3DValue),TAS5711_Reg3DValue},
    {TAS5711_DRC2_AA,sizeof(TAS5711_Reg3EValue),TAS5711_Reg3EValue},
    {TAS5711_DRC2_AD,sizeof(TAS5711_Reg3FValue),TAS5711_Reg3FValue},
    {TAS5711_DRC2_T,sizeof(TAS5711_Reg43Value),TAS5711_Reg43Value},
    {TAS5711_DRC2_K,sizeof(TAS5711_Reg44Value),TAS5711_Reg44Value},
    {TAS5711_DRC2_O,sizeof(TAS5711_Reg45Value),TAS5711_Reg45Value},
    {TAS5711_DRC_CONTROL,sizeof(TAS5711_Reg46Value),TAS5711_Reg46Value},
    {TAS5711_CH1_BQ0,sizeof(TAS5711_Reg29Value),TAS5711_Reg29Value},
    {TAS5711_CH1_BQ1,sizeof(TAS5711_Reg2AValue),TAS5711_Reg2AValue},
    {TAS5711_CH1_BQ2,sizeof(TAS5711_Reg2BValue),TAS5711_Reg2BValue},
    {TAS5711_CH1_BQ3,sizeof(TAS5711_Reg2CValue),TAS5711_Reg2CValue},
    {TAS5711_CH1_BQ4,sizeof(TAS5711_Reg2DValue),TAS5711_Reg2DValue},
    {TAS5711_CH1_BQ5,sizeof(TAS5711_Reg2EValue),TAS5711_Reg2EValue},
    {TAS5711_CH1_BQ6,sizeof(TAS5711_Reg2FValue),TAS5711_Reg2FValue},
    {TAS5711_CH2_BQ0,sizeof(TAS5711_Reg30Value),TAS5711_Reg30Value},
    {TAS5711_CH2_BQ1,sizeof(TAS5711_Reg31Value),TAS5711_Reg31Value},
    {TAS5711_CH2_BQ2,sizeof(TAS5711_Reg32Value),TAS5711_Reg32Value},
    {TAS5711_CH2_BQ3,sizeof(TAS5711_Reg33Value),TAS5711_Reg33Value},
    {TAS5711_CH2_BQ4,sizeof(TAS5711_Reg34Value),TAS5711_Reg34Value},
    {TAS5711_CH2_BQ5,sizeof(TAS5711_Reg35Value),TAS5711_Reg35Value},
    {TAS5711_CH2_BQ6,sizeof(TAS5711_Reg36Value),TAS5711_Reg36Value},
    {TAS5711_MASTER_VOLUME,sizeof(TAS5711_Reg07Value),TAS5711_Reg07Value},
    {TAS5711_SYSTEM_CTRL2,sizeof(TAS5711_Reg05Value),TAS5711_Reg05Value},
    {0x0D,sizeof(TAS5711_Reg0DValue),TAS5711_Reg0DValue},
};

void tas5711_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[24];
    unsigned char data_len;
    unsigned char index;
#if AMP_DEBUG
        unsigned char i;
#endif
    for(index = 0; index < sizeof(TAS5711_InitTbl)/sizeof(TAS5711_REG); index ++)
    {
        data[0] = TAS5711_InitTbl[index].bAddr;
        data_len = TAS5711_InitTbl[index].bDataLen;
        memcpy(&data[1],TAS5711_InitTbl[index].bArray,TAS5711_InitTbl[index].bDataLen);

#if AMP_DEBUG
        AMP_INFO("\n===write addr:0x%x len:%d===\n",data[0],data_len);
        for(i = 0 ; i < TAS5711_InitTbl[index].bDataLen; ++i)
            AMP_INFO("%x ", data[i]);

        AMP_INFO("\n");
#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program TAS5711 failed\n");

    }
}

