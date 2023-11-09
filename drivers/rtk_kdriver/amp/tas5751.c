#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#else
#include <no_os/slab.h>
#include <timer.h>
#include <malloc.h>
#endif

//*************************************************************
// PUBLIC FUNCTIONS
//*************************************************************
#define TAS5751_END_FLAG 	(0xff)
#define AMP_DEBUG 	(0)

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *bArray;
}TAS5751_REG;

typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *reg_name;
}TAS5751_DUMP;


unsigned char    TAS5751_Reg00Value[]={0x6c}; //CLOCK_CTRL
unsigned char    TAS5751_Reg02Value[]={0x00}; //ERROR_STATUS
unsigned char    TAS5751_Reg03Value[]={0xa0}; //SYSTEM_CTRL1
unsigned char    TAS5751_Reg04Value[]={0x05}; //DATA_INTERFACE
unsigned char    TAS5751_Reg05Value[]={0x07}; //SYSTEM_CTRL2
unsigned char    TAS5751_Reg06Value[]={0x40}; //SOFT_MUTE
unsigned char    TAS5751_Reg07Value[]={0x01, 0x10, 0x00 }; //MASTER_VOLUME
unsigned char    TAS5751_Reg08Value[]={0x00, 0xC0, 0x00}; //CHANNEL_1_VOL
unsigned char    TAS5751_Reg09Value[]={0x00, 0xC0, 0x00}; //CHANNEL_2_VOL
unsigned char    TAS5751_Reg0AValue[]={0x00, 0xC0, 0x00}; //CHANNEL_3_VOL
unsigned char    TAS5751_Reg0EValue[]={0xf0}; //VOLUME_CONFIG
unsigned char    TAS5751_Reg10Value[]={0x01}; //MODULATION_LIMIT
unsigned char    TAS5751_Reg11Value[]={0xB8}; //IC_DELAY_CHANNEL1
unsigned char    TAS5751_Reg12Value[]={0x60}; //IC_DELAY_CHANNEL2
unsigned char    TAS5751_Reg13Value[]={0xA0}; //IC_DELAY_CHANNEL3
unsigned char    TAS5751_Reg14Value[]={0x48}; //IC_DELAY_CHANNEL4
unsigned char    TAS5751_Reg19Value[]={0x30}; //PWM_SHUTDOWN_GROUP
unsigned char    TAS5751_Reg1AValue[]={0x68}; //START_STOP_PERIOD
unsigned char    TAS5751_Reg1BValue[]={0x00}; //OSCILLATOR_TRIM
unsigned char    TAS5751_Reg1CValue[]={0x57}; //BKND_ERR
unsigned char    TAS5751_Reg20Value[]={0x00,0x89,0x77,0x72,0x00}; //INPUT_MUX
unsigned char    TAS5751_Reg25Value[]={0x01,0x02,0x13,0x45,0x00}; //PWM_MUX
unsigned char    TAS5751_Reg26Value[]={0x00,0x7f,0xf0,0xd7,0x07,0x00,0x1e,0x53,0x00,0x7f,0xf0,0xd7,0x00,0xff,0xe1,0xab,0x07,0x80,0x1e,0x51,0x00}; //CH1_BQ0
unsigned char    TAS5751_Reg27Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ1
unsigned char    TAS5751_Reg28Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ2
unsigned char    TAS5751_Reg29Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ3
unsigned char    TAS5751_Reg2AValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ4
unsigned char    TAS5751_Reg2BValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ5
unsigned char    TAS5751_Reg2CValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ6
unsigned char    TAS5751_Reg2DValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ7
unsigned char    TAS5751_Reg2EValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ8
unsigned char    TAS5751_Reg2FValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ9
unsigned char    TAS5751_Reg30Value[]={0x00,0x7f,0xf0,0xd7,0x07,0x00,0x1e,0x53,0x00,0x7f,0xf0,0xd7,0x00,0xff,0xe1,0xab,0x07,0x80,0x1e,0x51,0x00}; //CH2_BQ0
unsigned char    TAS5751_Reg31Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ1
unsigned char    TAS5751_Reg32Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ2
unsigned char    TAS5751_Reg33Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ3
unsigned char    TAS5751_Reg34Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ4
unsigned char    TAS5751_Reg35Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ5
unsigned char    TAS5751_Reg36Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ6
unsigned char    TAS5751_Reg37Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ7
unsigned char    TAS5751_Reg38Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ8
unsigned char    TAS5751_Reg39Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ9
unsigned char    TAS5751_Reg3BValue[]={0x00,0x07,0x60,0x53,0x00,0x78,0x9f,0xac,0x00}; //AGL1_SOFTENING_FILTER
unsigned char    TAS5751_Reg3CValue[]={0x00,0x00,0x00,0x6d,0xff,0xff,0xff,0xe5,0x00}; //AGL1_ATTACK_RATE
unsigned char    TAS5751_Reg3EValue[]={0x00,0x07,0x60,0x53,0x00,0x78,0x9f,0xac,0x00}; //AGL2_SOFTENING_FILTER
unsigned char    TAS5751_Reg3FValue[]={0x00,0x00,0x00,0x6d,0xff,0xff,0xff,0xe5,0x00}; //AGL2_ATTACK_RATE
unsigned char    TAS5751_Reg40Value[]={0x08,0x00,0x00,0x00,0x00}; //AGL1_ATTACK_THRESHOLD
unsigned char    TAS5751_Reg41Value[]={0x08,0x00,0x00,0x00,0x00}; //AGL3_ATTACK_THRESHOLD
unsigned char    TAS5751_Reg42Value[]={0x00,0x00,0x00,0x6d,0xff,0xff,0xff,0xe5,0x00}; //AGL3_ATTACK_RATE
unsigned char    TAS5751_Reg43Value[]={0x08,0x00,0x00,0x00,0x00}; //AGL2_ATTACK_THRESHOLD
unsigned char    TAS5751_Reg44Value[]={0x07,0x21,0x48,0x2b,0x00}; //AGL4_ATTACK_THRESHOLD
unsigned char    TAS5751_Reg45Value[]={0x00,0x00,0x00,0x6d,0xff,0xff,0xff,0xe5,0x00}; //AGL4_ATTACK_RATE
unsigned char    TAS5751_Reg46Value[]={0x00,0x02,0x00,0x0f,0x00}; //AGL_CONTROL
unsigned char    TAS5751_Reg47Value[]={0x00,0x07,0x60,0x53,0x00,0x78,0x9f,0xac,0x00}; //AGL3_SOFTENING_FILTER
unsigned char    TAS5751_Reg48Value[]={0x00,0x07,0x60,0x53,0x00,0x78,0x9f,0xac,0x00}; //AGL4_SOFTENING_FILTER
unsigned char    TAS5751_Reg4FValue[]={0x00,0x00,0x00,0x08,0x00}; //PWM_SWITCH_RATE_CTRL
unsigned char    TAS5751_Reg50Value[]={0x0f,0x70,0x80,0x00,0x00}; //BANK_SWITCH_CTRL
unsigned char    TAS5751_Reg51Value[]={0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x07,0x80,0x00,0x00,0x00}; //CH1_OUTPUT_MIXER
unsigned char    TAS5751_Reg52Value[]={0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x00,0x07,0x80,0x00,0x00,0x00}; //CH2_OUTPUT_MIXER
unsigned char    TAS5751_Reg56Value[]={0x00,0x80,0x00,0x00,0x00}; //OUTPUT_POST_SCALE
unsigned char    TAS5751_Reg57Value[]={0x00,0x02,0x00,0x00,0x00}; //OUTPUT_PRE_SCALE
unsigned char    TAS5751_Reg58Value[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH1_BQ10
unsigned char    TAS5751_Reg59Value[]={0x00,0x00,0x0c,0x27,0x00,0x00,0x18,0x4f,0x00,0x00,0x0c,0x27,0x00,0xf6,0x23,0xa6,0x07,0x89,0xab,0xbc,0x00}; //CH1_CROSS_BQ0
unsigned char    TAS5751_Reg5AValue[]={0x00,0x59,0x0c,0xdc,0x07,0x4d,0xe6,0x47,0x00,0x59,0x0c,0xdc,0x00,0xab,0x0d,0xc1,0x07,0xc6,0xda,0x4f,0x00}; //CH1_CROSS_BQ1
unsigned char    TAS5751_Reg5BValue[]={0x00,0x03,0x85,0xfc,0x00,0x07,0x0b,0xf8,0x00,0x03,0x85,0xfc,0x00,0xab,0x0d,0xc1,0x07,0xc6,0xda,0x4f,0x00}; //CH1_CROSS_BQ2
unsigned char    TAS5751_Reg5CValue[]={0x00,0x7b,0x1d,0xfa,0x07,0x09,0xc4,0x0b,0x00,0x7b,0x1d,0xfa,0x00,0xf6,0x23,0xa6,0x07,0x89,0xab,0xbc,0x00}; //CH1_CROSS_BQ3
unsigned char    TAS5751_Reg5DValue[]={0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //CH2_BQ10
unsigned char    TAS5751_Reg5EValue[]={0x00,0x00,0x0c,0x27,0x00,0x00,0x18,0x4f,0x00,0x00,0x0c,0x27,0x00,0xf6,0x23,0xa6,0x07,0x89,0xab,0xbc,0x00}; //CH2_CROSS_BQ0
unsigned char    TAS5751_Reg5FValue[]={0x00,0x59,0x0c,0xdc,0x07,0x4d,0xe6,0x47,0x00,0x59,0x0c,0xdc,0x00,0xab,0x0d,0xc1,0x07,0xc6,0xda,0x4f,0x00}; //CH2_CROSS_BQ1
unsigned char    TAS5751_Reg60Value[]={0x00,0x03,0x85,0xfc,0x00,0x07,0x0b,0xf8,0x00,0x03,0x85,0xfc,0x00,0xab,0x0d,0xc1,0x07,0xc6,0xda,0x4f,0x00}; //CH2_CROSS_BQ2
unsigned char    TAS5751_Reg61Value[]={0x00,0x7b,0x1d,0xfa,0x07,0x09,0xc4,0x0b,0x00,0x7b,0x1d,0xfa,0x00,0xf6,0x23,0xa6,0x07,0x89,0xab,0xbc,0x00}; //CH2_CROSS_BQ3
unsigned char    TAS5751_Reg70Value[]={0x00,0x80,0x00,0x00,0x00}; //CH1_INLINE_MIXER
unsigned char    TAS5751_Reg71Value[]={0x00,0x00,0x00,0x00,0x00}; //CH1_INLINE_AGL_EN_MIXER
unsigned char    TAS5751_Reg72Value[]={0x00,0x00,0x00,0x00,0x00}; //RIGHT_CHANNEL_MIXER
unsigned char    TAS5751_Reg73Value[]={0x00,0x80,0x00,0x00,0x00}; //LEFT_CHANNEL_MIXER
unsigned char    TAS5751_Reg74Value[]={0x00,0x80,0x00,0x00,0x00}; //INLINE_MIXER
unsigned char    TAS5751_Reg75Value[]={0x00,0x00,0x00,0x00,0x00}; //INLINE_AGL_EN_MIXER
unsigned char    TAS5751_Reg76Value[]={0x00,0x00,0x00,0x00,0x00}; //CH2_LEFT_CHANNEL_MIXER
unsigned char    TAS5751_Reg77Value[]={0x00,0x80,0x00,0x00,0x00}; //CH2_RIGHT_CHANNEL_MIXER
unsigned char    TAS5751_Reg05Value2[]={0x00}; //SYSTEM_CTRL2
unsigned char    TAS5751_Reg06Value2[]={0x00}; //SOFT_MUTE
static TAS5751_REG TAS5751_InitTbl[] = 
{
// -----------------------------------------------------------------------------
// Initialization Sequence
// -----------------------------------------------------------------------------
    {TAS5751_SOFT_MUTE,               sizeof(TAS5751_Reg06Value),  TAS5751_Reg06Value },
    {TAS5751_SYSTEM_CTRL2,            sizeof(TAS5751_Reg05Value),  TAS5751_Reg05Value },
// -----------------------------------------------------------------------------
// Amplifier Configuration
// -----------------------------------------------------------------------------
    {TAS5751_CLOCK_CTRL,              sizeof(TAS5751_Reg00Value),  TAS5751_Reg00Value },
    {TAS5751_ERROR_STATUS,            sizeof(TAS5751_Reg02Value),  TAS5751_Reg02Value },
    {TAS5751_SYSTEM_CTRL1,            sizeof(TAS5751_Reg03Value),  TAS5751_Reg03Value },
    {TAS5751_DATA_INTERFACE,          sizeof(TAS5751_Reg04Value),  TAS5751_Reg04Value },
    {TAS5751_MASTER_VOLUME,           sizeof(TAS5751_Reg07Value),  TAS5751_Reg07Value },
    {TAS5751_CHANNEL_1_VOL,           sizeof(TAS5751_Reg08Value),  TAS5751_Reg08Value },
    {TAS5751_CHANNEL_2_VOL,           sizeof(TAS5751_Reg09Value),  TAS5751_Reg09Value },
    {TAS5751_CHANNEL_3_VOL,           sizeof(TAS5751_Reg0AValue),  TAS5751_Reg0AValue },
    {TAS5751_VOLUME_CONFIG,           sizeof(TAS5751_Reg0EValue),  TAS5751_Reg0EValue },
    {TAS5751_MODULATION_LIMIT,        sizeof(TAS5751_Reg10Value),  TAS5751_Reg10Value },
    {TAS5751_IC_DELAY_CHANNEL1,       sizeof(TAS5751_Reg11Value),  TAS5751_Reg11Value },
    {TAS5751_IC_DELAY_CHANNEL2,       sizeof(TAS5751_Reg12Value),  TAS5751_Reg12Value },
    {TAS5751_IC_DELAY_CHANNEL3,       sizeof(TAS5751_Reg13Value),  TAS5751_Reg13Value },
    {TAS5751_IC_DELAY_CHANNEL4,       sizeof(TAS5751_Reg14Value),  TAS5751_Reg14Value },
    {TAS5751_PWM_SHUTDOWN_GROUP,      sizeof(TAS5751_Reg19Value),  TAS5751_Reg19Value },
    {TAS5751_START_STOP_PERIOD,       sizeof(TAS5751_Reg1AValue),  TAS5751_Reg1AValue },
    {TAS5751_OSCILLATOR_TRIM,         sizeof(TAS5751_Reg1BValue),  TAS5751_Reg1BValue },
    {TAS5751_BKND_ERR,                sizeof(TAS5751_Reg1CValue),  TAS5751_Reg1CValue },
    {TAS5751_INPUT_MUX,               sizeof(TAS5751_Reg20Value),  TAS5751_Reg20Value },
    {TAS5751_PWM_MUX,                 sizeof(TAS5751_Reg25Value),  TAS5751_Reg25Value },
    {TAS5751_CH1_BQ0,                 sizeof(TAS5751_Reg26Value),  TAS5751_Reg26Value },
    {TAS5751_CH1_BQ1,                 sizeof(TAS5751_Reg27Value),  TAS5751_Reg27Value },
    {TAS5751_CH1_BQ2,                 sizeof(TAS5751_Reg28Value),  TAS5751_Reg28Value },
    {TAS5751_CH1_BQ3,                 sizeof(TAS5751_Reg29Value),  TAS5751_Reg29Value },
    {TAS5751_CH1_BQ4,                 sizeof(TAS5751_Reg2AValue),  TAS5751_Reg2AValue },
    {TAS5751_CH1_BQ5,                 sizeof(TAS5751_Reg2BValue),  TAS5751_Reg2BValue },
    {TAS5751_CH1_BQ6,                 sizeof(TAS5751_Reg2CValue),  TAS5751_Reg2CValue },
    {TAS5751_CH1_BQ7,                 sizeof(TAS5751_Reg2DValue),  TAS5751_Reg2DValue },
    {TAS5751_CH1_BQ8,                 sizeof(TAS5751_Reg2EValue),  TAS5751_Reg2EValue },
    {TAS5751_CH1_BQ9,                 sizeof(TAS5751_Reg2FValue),  TAS5751_Reg2FValue },
    {TAS5751_CH2_BQ0,                 sizeof(TAS5751_Reg30Value),  TAS5751_Reg30Value },
    {TAS5751_CH2_BQ1,                 sizeof(TAS5751_Reg31Value),  TAS5751_Reg31Value },
    {TAS5751_CH2_BQ2,                 sizeof(TAS5751_Reg32Value),  TAS5751_Reg32Value },
    {TAS5751_CH2_BQ3,                 sizeof(TAS5751_Reg33Value),  TAS5751_Reg33Value },
    {TAS5751_CH2_BQ4,                 sizeof(TAS5751_Reg34Value),  TAS5751_Reg34Value },
    {TAS5751_CH2_BQ5,                 sizeof(TAS5751_Reg35Value),  TAS5751_Reg35Value },
    {TAS5751_CH2_BQ6,                 sizeof(TAS5751_Reg36Value),  TAS5751_Reg36Value },
    {TAS5751_CH2_BQ7,                 sizeof(TAS5751_Reg37Value),  TAS5751_Reg37Value },
    {TAS5751_CH2_BQ8,                 sizeof(TAS5751_Reg38Value),  TAS5751_Reg38Value },
    {TAS5751_CH2_BQ9,                 sizeof(TAS5751_Reg39Value),  TAS5751_Reg39Value },
    {TAS5751_AGL1_SOFTENING_FILTER,   sizeof(TAS5751_Reg3BValue),  TAS5751_Reg3BValue },
    {TAS5751_AGL1_ATTACK_RATE,        sizeof(TAS5751_Reg3CValue),  TAS5751_Reg3CValue },
    {TAS5751_AGL2_SOFTENING_FILTER,   sizeof(TAS5751_Reg3EValue),  TAS5751_Reg3EValue },
    {TAS5751_AGL2_ATTACK_RATE,        sizeof(TAS5751_Reg3FValue),  TAS5751_Reg3FValue },
    {TAS5751_AGL1_ATTACK_THRESHOLD,   sizeof(TAS5751_Reg40Value),  TAS5751_Reg40Value },
    {TAS5751_AGL3_ATTACK_THRESHOLD,   sizeof(TAS5751_Reg41Value),  TAS5751_Reg41Value },
    {TAS5751_AGL3_ATTACK_RATE,        sizeof(TAS5751_Reg42Value),  TAS5751_Reg42Value },
    {TAS5751_AGL2_ATTACK_THRESHOLD,   sizeof(TAS5751_Reg43Value),  TAS5751_Reg43Value },
    {TAS5751_AGL4_ATTACK_THRESHOLD,   sizeof(TAS5751_Reg44Value),  TAS5751_Reg44Value },
    {TAS5751_AGL4_ATTACK_RATE,        sizeof(TAS5751_Reg45Value),  TAS5751_Reg45Value },
    {TAS5751_AGL_CONTROL,             sizeof(TAS5751_Reg46Value),  TAS5751_Reg46Value },
    {TAS5751_AGL3_SOFTENING_FILTER,   sizeof(TAS5751_Reg47Value),  TAS5751_Reg47Value },
    {TAS5751_AGL4_SOFTENING_FILTER,   sizeof(TAS5751_Reg48Value),  TAS5751_Reg48Value },
    {TAS5751_PWM_SWITCH_RATE_CTRL,    sizeof(TAS5751_Reg4FValue),  TAS5751_Reg4FValue },
    {TAS5751_BANK_SWITCH_CTRL,        sizeof(TAS5751_Reg50Value),  TAS5751_Reg50Value },
    {TAS5751_CH1_OUTPUT_MIXER,        sizeof(TAS5751_Reg51Value),  TAS5751_Reg51Value },
    {TAS5751_CH2_OUTPUT_MIXER,        sizeof(TAS5751_Reg52Value),  TAS5751_Reg52Value },
    {TAS5751_OUTPUT_POST_SCALE,       sizeof(TAS5751_Reg56Value),  TAS5751_Reg56Value },
    {TAS5751_OUTPUT_PRE_SCALE,        sizeof(TAS5751_Reg57Value),  TAS5751_Reg57Value },
    {TAS5751_CH1_BQ10,                sizeof(TAS5751_Reg58Value),  TAS5751_Reg58Value },
    {TAS5751_CH1_CROSS_BQ0,           sizeof(TAS5751_Reg59Value),  TAS5751_Reg59Value },
    {TAS5751_CH1_CROSS_BQ1,           sizeof(TAS5751_Reg5AValue),  TAS5751_Reg5AValue },
    {TAS5751_CH1_CROSS_BQ2,           sizeof(TAS5751_Reg5BValue),  TAS5751_Reg5BValue },
    {TAS5751_CH1_CROSS_BQ3,           sizeof(TAS5751_Reg5CValue),  TAS5751_Reg5CValue },
    {TAS5751_CH2_BQ10,                sizeof(TAS5751_Reg5DValue),  TAS5751_Reg5DValue },
    {TAS5751_CH2_CROSS_BQ0,           sizeof(TAS5751_Reg5EValue),  TAS5751_Reg5EValue },
    {TAS5751_CH2_CROSS_BQ1,           sizeof(TAS5751_Reg5FValue),  TAS5751_Reg5FValue },
    {TAS5751_CH2_CROSS_BQ2,           sizeof(TAS5751_Reg60Value),  TAS5751_Reg60Value },
    {TAS5751_CH2_CROSS_BQ3,           sizeof(TAS5751_Reg61Value),  TAS5751_Reg61Value },
    {TAS5751_CH1_INLINE_MIXER,        sizeof(TAS5751_Reg70Value),  TAS5751_Reg70Value },
    {TAS5751_CH1_INLINE_AGL_EN_MIXER, sizeof(TAS5751_Reg71Value),  TAS5751_Reg71Value },
    {TAS5751_CH1_RIGHT_CHANNEL_MIXER, sizeof(TAS5751_Reg72Value),  TAS5751_Reg72Value },
    {TAS5751_CH1_LEFT_CHANNEL_MIXER,  sizeof(TAS5751_Reg73Value),  TAS5751_Reg73Value },
    {TAS5751_CH2_INLINE_MIXER,        sizeof(TAS5751_Reg74Value),  TAS5751_Reg74Value },
    {TAS5751_CH2_INLINE_AGL_EN_MIXER, sizeof(TAS5751_Reg75Value),  TAS5751_Reg75Value },
    {TAS5751_CH2_LEFT_CHANNEL_MIXER,  sizeof(TAS5751_Reg76Value),  TAS5751_Reg76Value },
    {TAS5751_CH2_RIGHT_CHANNEL_MIXER, sizeof(TAS5751_Reg77Value),  TAS5751_Reg77Value },
// -----------------------------------------------------------------------------
// Unmute Sequence
// -----------------------------------------------------------------------------
    {TAS5751_SYSTEM_CTRL2,   sizeof(TAS5751_Reg05Value2), TAS5751_Reg05Value2 },
    {TAS5751_SOFT_MUTE,      sizeof(TAS5751_Reg06Value2), TAS5751_Reg06Value2 },
};


static TAS5751_DUMP TAS5751_Dump[] =
{
    {TAS5751_CLOCK_CTRL           ,1 ,"Clock_CTRL"           },
    {TAS5751_DEVICE_ID            ,1 ,"DEVICE_ID"            },
    {TAS5751_ERROR_STATUS         ,1 ,"ERROR_STATUS"         },
    {TAS5751_SYSTEM_CTRL1         ,1 ,"SYSTEM_CTRL1"         },
    {TAS5751_DATA_INTERFACE       ,1 ,"DATA_INTERFACE"       },
    {TAS5751_SYSTEM_CTRL2         ,1 ,"SYSTEM_CTRL2"         },
    {TAS5751_SOFT_MUTE            ,1 ,"SOFT_MUTE"            },
    {TAS5751_MASTER_VOLUME        ,1 ,"MASTER_VOLUME"        },
    {TAS5751_CHANNEL_1_VOL        ,1 ,"CHANNEL_1_VOL"        },
    {TAS5751_CHANNEL_2_VOL        ,1 ,"CHANNEL_2_VOL"        },
    {TAS5751_CHANNEL_3_VOL        ,1 ,"CHANNEL_3_VOL"        },
    {TAS5751_VOLUME_CONFIG        ,1 ,"VOLUME_CONFIG"        },
    {TAS5751_MODULATION_LIMIT     ,1 ,"MODULATION_LIMIT"     },
    {TAS5751_IC_DELAY_CHANNEL1    ,1 ,"IC_DELAY_CHANNEL1"    },
    {TAS5751_IC_DELAY_CHANNEL2    ,1 ,"IC_DELAY_CHANNEL2"    },
    {TAS5751_IC_DELAY_CHANNEL3    ,1 ,"IC_DELAY_CHANNEL3"    },
    {TAS5751_IC_DELAY_CHANNEL4    ,1 ,"IC_DELAY_CHANNEL4"    },
    {TAS5751_START_STOP_PERIOD    ,1 ,"START_STOP_PERIOD"    },
    {TAS5751_OSCILLATOR_TRIM      ,1 ,"OSCILLATOR_TRIM"      },
    {TAS5751_BKND_ERR             ,1 ,"BKND_ERR"             },
    {TAS5751_INPUT_MUX            ,4 ,"INPUT_MUX"            },
    {TAS5751_PWM_MUX              ,4 ,"PWM_MUX"              },
    {TAS5751_CH1_BQ0              ,20,"CH1_BQ0"              },
    {TAS5751_CH1_BQ1              ,20,"CH1_BQ1"              },
    {TAS5751_CH1_BQ2              ,20,"CH1_BQ2"              },
    {TAS5751_CH1_BQ3              ,20,"CH1_BQ3"              },
    {TAS5751_CH1_BQ4              ,20,"CH1_BQ4"              },
    {TAS5751_CH1_BQ5              ,20,"CH1_BQ5"              },
    {TAS5751_CH1_BQ6              ,20,"CH1_BQ6"              },
    {TAS5751_CH1_BQ7              ,20,"CH1_BQ7"              },
    {TAS5751_CH1_BQ8              ,20,"CH1_BQ8"              },
    {TAS5751_CH1_BQ9              ,20,"CH1_BQ9"              },
    {TAS5751_CH2_BQ0              ,20,"CH2_BQ0"              },
    {TAS5751_CH2_BQ1              ,20,"CH2_BQ1"              },
    {TAS5751_CH2_BQ2              ,20,"CH2_BQ2"              },
    {TAS5751_CH2_BQ3              ,20,"CH2_BQ3"              },
    {TAS5751_CH2_BQ4              ,20,"CH2_BQ4"              },
    {TAS5751_CH2_BQ5              ,20,"CH2_BQ5"              },
    {TAS5751_CH2_BQ6              ,20,"CH2_BQ6"              },
    {TAS5751_CH2_BQ7              ,20,"CH2_BQ7"              },
    {TAS5751_CH2_BQ8              ,20,"CH2_BQ8"              },
    {TAS5751_CH2_BQ9              ,20,"CH2_BQ9"              },
    {TAS5751_AGL1_SOFTENING_FILTER,8 ,"AGL1_SOFTENING_FILTER"},
    {TAS5751_AGL1_ATTACK_RATE     ,8 ,"AGL1_ATTACK_RATE"     },
    {TAS5751_AGL2_SOFTENING_FILTER,8 ,"AGL2_SOFTENING_FILTER"},
    {TAS5751_AGL2_ATTACK_RATE     ,8 ,"AGL2_ATTACK_RATE"     },
    {TAS5751_AGL1_ATTACK_THRESHOLD,4 ,"AGL1_ATTACK_THRESHOLD"},
    {TAS5751_AGL3_ATTACK_THRESHOLD,4 ,"AGL3_ATTACK_THRESHOLD"},
    {TAS5751_AGL3_ATTACK_RATE     ,8 ,"AGL3_ATTACK_RATE"     },
    {TAS5751_AGL2_ATTACK_THRESHOLD,4 ,"AGL2_ATTACK_THRESHOLD"},
    {TAS5751_AGL4_ATTACK_THRESHOLD,4 ,"AGL4_ATTACK_THRESHOLD"},
    {TAS5751_AGL4_ATTACK_RATE     ,8 ,"AGL4_ATTACK_RATE"     },
    {TAS5751_AGL_CONTROL          ,4 ,"AGL_CONTROL"          },
    {TAS5751_AGL3_SOFTENING_FILTER,8 ,"AGL3_SOFTENING_FILTER"},
    {TAS5751_AGL4_SOFTENING_FILTER,8 ,"AGL4_SOFTENING_FILTER"},
    {TAS5751_PWM_SWITCH_RATE_CTRL ,4 ,"PWM_SWITCH_RATE_CTRL" },
    {TAS5751_BANK_SWITCH_CTRL     ,4 ,"BANK_SWITCH_CTRL"     },
    {TAS5751_CH1_OUTPUT_MIXER     ,12,"CH1_OUTPUT_MIXER"     },
    {TAS5751_CH2_OUTPUT_MIXER     ,12,"CH2_OUTPUT_MIXER"     },
    {TAS5751_OUTPUT_POST_SCALE    ,4 ,"OUTPUT_POST_SCALE"    },
    {TAS5751_OUTPUT_PRE_SCALE     ,4 ,"OUTPUT_PRE_SCALE"     },
    {TAS5751_CH1_BQ10             ,20 ,"CH1_BQ10"            },
    {TAS5751_CH1_CROSS_BQ0        ,20 ,"CH1_CROSS_BQ0"       },
    {TAS5751_CH1_CROSS_BQ1        ,20 ,"CH1_CROSS_BQ1"       },
    {TAS5751_CH1_CROSS_BQ2        ,20 ,"CH1_CROSS_BQ2"       },
    {TAS5751_CH1_CROSS_BQ3        ,20 ,"_CH1_CROSS_BQ3"      },
    {TAS5751_CH2_BQ10             ,20 ,"CH2_BQ10"            },
    {TAS5751_CH2_CROSS_BQ0        ,20 ,"CH2_CROSS_BQ0"       },
    {TAS5751_CH2_CROSS_BQ1        ,20 ,"CH2_CROSS_BQ1"       },
    {TAS5751_CH2_CROSS_BQ2        ,20 ,"CH2_CROSS_BQ2"       },
    {TAS5751_CH2_CROSS_BQ3        ,20 ,"CH2_CROSS_BQ3"       },
    {TAS5751_IDF_POST_SCALE       ,4  ,"IDF_POST_SCALE"      },
    {TAS5751_LEFT_CHANNEL_PWM_LEVEL,4 ,"LEFT_CHANNEL_PWM"    },
    {TAS5751_RIGHT_CHANNEL_PWM_LEVEL,4 ,"RIGHT_CHANNEL_PWM"  },
    {TAS5751_CH1_INLINE_MIXER       ,4 ,"CH1_INLINE_MIXER"   },
    {TAS5751_CH1_INLINE_AGL_EN_MIXER,4 ,"CH1_INLINE_AGL_EN"  },
    {TAS5751_CH1_RIGHT_CHANNEL_MIXER,4 ,"CH1_RIGHT_CHANNEL"  },
    {TAS5751_CH1_LEFT_CHANNEL_MIXER ,4 ,"CH1_LEFT_CHANNEL"   },
    {TAS5751_CH2_INLINE_MIXER       ,4 ,"CH2_INLINE_MIXER"   },
    {TAS5751_CH2_INLINE_AGL_EN_MIXER,4 ,"CH2_INLINE_AGL_EN"  },
    {TAS5751_CH2_LEFT_CHANNEL_MIXER ,4 ,"CH2_LEFT_CHANNEL"   },
    {TAS5751_CH2_RIGHT_CHANNEL_MIXER,4 ,"CH2_RIGHT_CHANNEL"  },
    {TAS5751_UPDATE_ADDRESS_KEY     ,4 ,"UPDATE_ADDRESS_KEY" },
    {TAS5751_UPDATE_ADDRESS         ,4 ,"UPDATE_ADDRESS"     },
};


void tas5751_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[24];
    unsigned char data_len;
    unsigned char index;
#if TAS5751_DEBUG
    unsigned char i;
#endif
    AMP_WARN("\nTAS5751 Initial Start\n");
    for(index = 0; index < (sizeof(TAS5751_InitTbl)/sizeof(TAS5751_REG)); index ++)
    {
        data[0] = TAS5751_InitTbl[index].bAddr;
        data_len = TAS5751_InitTbl[index].bDataLen;
        memcpy(&data[1],TAS5751_InitTbl[index].bArray,TAS5751_InitTbl[index].bDataLen);

#if TAS5707_DEBUG
        AMP_WARN("===write addr:0x%x len:%d===\n",data[0],data_len);
        for(i = 0 ; i < TAS5751_InitTbl[index].bDataLen; ++i)
            AMP_WARN("%x ", data[i]);

#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program TAS5751 failed\n");

    }

}

int tas5751_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int index = 0;
    int data_len = 0;
    int data_index = 0;
    unsigned char addr[1] ={0};
    unsigned char *data = NULL;
    //Read SUBADDRESS Register
    AMP_WARN("== Dump SUBADDRESS 00~F9 Reg Size:%d==\n",sizeof(TAS5751_Dump)/sizeof(TAS5751_DUMP));
    for (index = 0; index < (sizeof(TAS5751_Dump)/sizeof(TAS5751_DUMP)) ;++index){
        addr[0] = TAS5751_Dump[index].bAddr;
        data_len = TAS5751_Dump[index].bDataLen;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        data = kmalloc(data_len, GFP_KERNEL);
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,data_len);
        AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5751_Dump[index].reg_name,addr[0],data_len);
        for(data_index = 0 ; data_index < data_len; ++data_index){
            AMP_WARN("data[%d]:%x ", data_index, data[data_index]);
        }

        mdelay(5);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            kfree(data);
            break;
        }
        kfree(data);
    }

    return ret;
}

int tas5751_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};
    unsigned char send_data[3] ={0};

    //Read
    addr[0] = TAS5751_SOFT_MUTE;
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,TAS5751_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",TAS5751_SOFT_MUTE);
        return (-1);
    }

    if (on_off == AMP_MUTE_ON){
        data[0] = TAS5751_MUTE_ON(data[0]);
    }
    else if (on_off == AMP_MUTE_OFF){
        data[0] = TAS5751_MUTE_OFF(data[0]);
    }

    //Write
    send_data[0] = TAS5751_SOFT_MUTE;
    send_data[1] = data[0];


    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,TAS5751_SOFT_MUTE,send_data[1]);
    }

    return ret;
}

int tas5751_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
{

    unsigned char send_data[2] ={0};

    send_data[0] = data_wr[0];
    send_data[1] = data_wr[1];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data_H:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x\n", slave_addr,send_data[0],send_data[1]);
    }

    return 0;
}

int tas5751_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;
    int index = 0;
    int result = 0;
    int data_len = 0;
    int data_index = 0;
    unsigned char addr[1] ={0};
    unsigned char *data = NULL;

    //Read
    addr[0] = reg[0];
    for (index = 0; index < (sizeof(TAS5751_Dump)/sizeof(TAS5751_DUMP)) ;++index){
        if(addr[0] == TAS5751_Dump[index].bAddr){
            data_len = TAS5751_Dump[index].bDataLen;
            break;
        }
    }

    if(data_len == 0){
        AMP_ERR("Read AMP REG:0x%x failed !! No this Register address\n",*reg);
        return (-1);
    }

    data = kmalloc(data_len, GFP_KERNEL);

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        ret = (-1);
        goto failed;
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,data_len);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",*reg);
        ret = (-1);
        goto failed;
    }

    AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5751_Dump[index].reg_name,addr[0],data_len);
    for(data_index = 0 ; data_index < data_len; ++data_index){
        AMP_WARN("data[%d]:%x ", data_index, data[data_index]);
    }

failed:
    kfree(data);
    return ret;
}
