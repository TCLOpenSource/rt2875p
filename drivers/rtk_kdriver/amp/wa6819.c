#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <malloc.h>
#include <timer.h>
#endif
#include <rtk_kdriver/rtk_gpio.h>
//*************************************************************
// PUBLIC FUNCTIONS
//*************************************************************
#ifdef AMP_DEBUG
#undef AMP_DEBUG
#define AMP_DEBUG (0)
#endif

typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *bArray;
}WA6819_REG;

unsigned char    WA6819_Reg4ByteValue1[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg4ByteValue2[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg4ByteValue3[]={0x10,0x00,0x00,0x00};

unsigned char    WA6819_Reg0BTbl0Value[]={0x01};
unsigned char    WA6819_Reg02Tbl0Value[]={0x00};
unsigned char    WA6819_Reg33Tbl0Value[]={0x03};
unsigned char    WA6819_Reg7ETbl0Value[]={0x03};

unsigned char    WA6819_Reg7ETbl2Value1[]={0x00};
unsigned char    WA6819_Reg33Tbl2Value1[]={0x00};
unsigned char    WA6819_Reg20Tbl2Value[]={0x80};
unsigned char    WA6819_Reg21Tbl2Value[]={0x01};
unsigned char    WA6819_Reg22Tbl2Value[]={0x00};
unsigned char    WA6819_Reg23Tbl2Value[]={0x01};
unsigned char    WA6819_Reg26Tbl2Value[]={0x80};
unsigned char    WA6819_Reg27Tbl2Value[]={0x41};
unsigned char    WA6819_Reg2ATbl2Value[]={0x6A};
unsigned char    WA6819_Reg2BTbl2Value[]={0x01};
unsigned char    WA6819_Reg29Tbl2Value[]={0x00};
unsigned char    WA6819_Reg2CTbl2Value[]={0x00};
unsigned char    WA6819_Reg17Tbl2Value[]={0xAE};
unsigned char    WA6819_Reg18Tbl2Value[]={0xAE};
unsigned char    WA6819_Reg43Tbl2Value[]={0x00};
unsigned char    WA6819_Reg3CTbl2Value[]={0x7B};
unsigned char    WA6819_Reg33Tbl2Value2[]={0x03};
unsigned char    WA6819_Reg7ETbl2Value2[]={0x03};

unsigned char    WA6819_Reg00Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg01Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg02Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg03Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg04Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg05Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg06Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg07Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg08Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg09Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg0ATbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg0BTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg0CTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg0DTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg0ETbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg0FTbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg10Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg11Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg12Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg13Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg14Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg15Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg16Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg17Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg18Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg19Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg1ATbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg1BTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg1CTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg1DTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg1ETbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg1FTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg20Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg21Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg22Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg23Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg24Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg25Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg26Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg27Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg28Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg29Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg2ATbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg2BTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg2CTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg2DTbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg2ETbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg2FTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg30Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg31Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg5CTbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg5DTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg5ETbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg5FTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg60Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg61Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg62Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg63Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg64Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg65Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg66Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg67Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg68Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg69Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg6ATbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg6BTbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg6CTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg6DTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg6ETbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg6FTbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg70Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg71Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg72Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg73Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg74Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg75Tbl3Value[]={0x11,0x00,0x00,0x00};
unsigned char    WA6819_Reg76Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg77Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg78Tbl3Value[]={0x20,0x00,0x00,0x00};
unsigned char    WA6819_Reg79Tbl3Value[]={0x20,0x00,0x00,0x00};

unsigned char    WA6819_Reg7ETbl4Value[]={0x08};

unsigned char    WA6819_Reg7ETbl6Value[]={0x00};
unsigned char    WA6819_Reg33Tbl6Value[]={0x00};
unsigned char    WA6819_Reg19Tbl6Value[]={0x00};
unsigned char    WA6819_Reg1ATbl6Value[]={0x00};
unsigned char    WA6819_Reg1BTbl6Value[]={0x00};
unsigned char    WA6819_Reg1CTbl6Value[]={0x00};
unsigned char    WA6819_Reg1DTbl6Value[]={0x00};
unsigned char    WA6819_Reg1ETbl6Value[]={0x00};
unsigned char    WA6819_Reg1FTbl6Value[]={0x00};
unsigned char    WA6819_Reg0ETbl6Value1[]={0x00};
unsigned char    WA6819_Reg0FTbl6Value1[]={0x00};
unsigned char    WA6819_Reg10Tbl6Value[]={0x00};
unsigned char    WA6819_Reg11Tbl6Value[]={0x00};
unsigned char    WA6819_Reg12Tbl6Value[]={0x00};
unsigned char    WA6819_Reg13Tbl6Value[]={0x00};
unsigned char    WA6819_Reg14Tbl6Value[]={0x00};
unsigned char    WA6819_Reg15Tbl6Value[]={0x00};
unsigned char    WA6819_Reg68Tbl6Value[]={0x60};
unsigned char    WA6819_Reg24Tbl6Value[]={0x00};
unsigned char    WA6819_Reg25Tbl6Value[]={0x0A};
unsigned char    WA6819_Reg4DTbl6Value[]={0x0A};
unsigned char    WA6819_Reg4ETbl6Value[]={0x0A};
unsigned char    WA6819_Reg4FTbl6Value[]={0x0A};
unsigned char    WA6819_Reg2DTbl6Value[]={0x0A};
unsigned char    WA6819_Reg07Tbl6Value[]={0x01};
unsigned char    WA6819_Reg0ETbl6Value2[]={0x00};
unsigned char    WA6819_Reg0FTbl6Value2[]={0x00};
unsigned char    WA6819_Reg31Tbl6Value[]={0x00};
unsigned char    WA6819_Reg32Tbl6Value[]={0x0F};
unsigned char    WA6819_Reg08Tbl6Value[]={0x00};

unsigned char    WA6819_Reg3DTbl8Value[]={0x00};
unsigned char    WA6819_Reg0CTbl8Value[]={0xFF};
unsigned char    WA6819_Reg16Tbl8Value[]={0x00};
unsigned char    WA6819_Reg35Tbl8Value[]={0x04};
unsigned char    WA6819_Reg34Tbl8Value[]={0x00};
unsigned char    WA6819_Reg33Tbl8Value[]={0x00};

static WA6819_REG WA6819_InitTb0[] =
{
    { 0x0B, sizeof(WA6819_Reg0BTbl0Value), WA6819_Reg0BTbl0Value }, // Soft Reset contorl
    { 0x02, sizeof(WA6819_Reg02Tbl0Value), WA6819_Reg02Tbl0Value }, // MCLK ( 48k = 0x00, 96k = 0x01, 36k=0x02 )
    { 0x33, sizeof(WA6819_Reg33Tbl0Value), WA6819_Reg33Tbl0Value }, // Soft Mute On Control
    { 0x7E, sizeof(WA6819_Reg7ETbl0Value), WA6819_Reg7ETbl0Value }, // Enable coefficient write for ch1/ch2
};

static WA6819_REG WA6819_CoefficientTbl[] =
{
    //DRC Coefficients
    { 0x32, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x33, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x34, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x35, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x36, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x37, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x38, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x39, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x3A, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x3B, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x3C, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x3D, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x3E, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x3F, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x40, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x41, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x42, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x43, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x44, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x45, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x46, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x47, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x48, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x49, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x4A, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x4B, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x4C, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x4D, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x4E, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x4F, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
};

static WA6819_REG WA6819_InitTb2[] =
{
    { 0x7E, sizeof(WA6819_Reg7ETbl2Value1), WA6819_Reg7ETbl2Value1 }, // Disable coefficient write for ch1/ch2
    { 0x33, sizeof(WA6819_Reg33Tbl2Value1), WA6819_Reg33Tbl2Value1 }, // Soft Mute Off Control/ch2
    { 0x20, sizeof(WA6819_Reg20Tbl2Value), WA6819_Reg20Tbl2Value }, // Low band DRC threshold = 9.0 dB
    { 0x21, sizeof(WA6819_Reg21Tbl2Value), WA6819_Reg21Tbl2Value }, // DRC Low band At/Rt
    { 0x22, sizeof(WA6819_Reg22Tbl2Value), WA6819_Reg22Tbl2Value }, // High band DRC threshold = 0.6 dB
    { 0x23, sizeof(WA6819_Reg23Tbl2Value), WA6819_Reg23Tbl2Value }, // DRC High band At/Re
    { 0x26, sizeof(WA6819_Reg26Tbl2Value), WA6819_Reg26Tbl2Value }, // Post band DRC threshold = 9.0 dB
    { 0x27, sizeof(WA6819_Reg27Tbl2Value), WA6819_Reg27Tbl2Value }, // DRC Post band At/Re
    { 0x2A, sizeof(WA6819_Reg2ATbl2Value), WA6819_Reg2ATbl2Value }, // Sub band DRC threshold = -1.7 dB
    { 0x2B, sizeof(WA6819_Reg2BTbl2Value), WA6819_Reg2BTbl2Value }, // DRC Sub band At/Re
    { 0x29, sizeof(WA6819_Reg29Tbl2Value), WA6819_Reg29Tbl2Value }, // 2Band DRC mode enable
    { 0x2C, sizeof(WA6819_Reg2CTbl2Value), WA6819_Reg2CTbl2Value }, // Sub band mode enable
    { 0x17, sizeof(WA6819_Reg17Tbl2Value), WA6819_Reg17Tbl2Value }, // channel 1 vol
    { 0x18, sizeof(WA6819_Reg18Tbl2Value), WA6819_Reg18Tbl2Value }, // channel 2 vol
    { 0x43, sizeof(WA6819_Reg43Tbl2Value), WA6819_Reg43Tbl2Value }, // PWM Output mode (0x02 = DBTL mode / 0x00 = AD mode)
    { 0x3C, sizeof(WA6819_Reg3CTbl2Value), WA6819_Reg3CTbl2Value }, // Prescaler Value CH12
    { 0x33, sizeof(WA6819_Reg33Tbl2Value2), WA6819_Reg33Tbl2Value2 }, // Soft Mute On Control/ch2
    { 0x7E, sizeof(WA6819_Reg7ETbl2Value2), WA6819_Reg7ETbl2Value2 }, // Enable coefficient write for ch1/ch2
};

static WA6819_REG WA6819_CoefficientTb3[] =
{
    // BQ Coefficients
    { 0x00, sizeof(WA6819_Reg00Tbl3Value), WA6819_Reg00Tbl3Value },
    { 0x01, sizeof(WA6819_Reg01Tbl3Value), WA6819_Reg01Tbl3Value },
    { 0x02, sizeof(WA6819_Reg02Tbl3Value), WA6819_Reg02Tbl3Value },
    { 0x03, sizeof(WA6819_Reg03Tbl3Value), WA6819_Reg03Tbl3Value },
    { 0x04, sizeof(WA6819_Reg04Tbl3Value), WA6819_Reg04Tbl3Value },
    { 0x05, sizeof(WA6819_Reg05Tbl3Value), WA6819_Reg05Tbl3Value },
    { 0x06, sizeof(WA6819_Reg06Tbl3Value), WA6819_Reg06Tbl3Value },
    { 0x07, sizeof(WA6819_Reg07Tbl3Value), WA6819_Reg07Tbl3Value },
    { 0x08, sizeof(WA6819_Reg08Tbl3Value), WA6819_Reg08Tbl3Value },
    { 0x09, sizeof(WA6819_Reg09Tbl3Value), WA6819_Reg09Tbl3Value },
    { 0x0A, sizeof(WA6819_Reg0ATbl3Value), WA6819_Reg0ATbl3Value },
    { 0x0B, sizeof(WA6819_Reg0BTbl3Value), WA6819_Reg0BTbl3Value },
    { 0x0C, sizeof(WA6819_Reg0CTbl3Value), WA6819_Reg0CTbl3Value },
    { 0x0D, sizeof(WA6819_Reg0DTbl3Value), WA6819_Reg0DTbl3Value },
    { 0x0E, sizeof(WA6819_Reg0ETbl3Value), WA6819_Reg0ETbl3Value },
    { 0x0F, sizeof(WA6819_Reg0FTbl3Value), WA6819_Reg0FTbl3Value },
    { 0x10, sizeof(WA6819_Reg10Tbl3Value), WA6819_Reg10Tbl3Value },
    { 0x11, sizeof(WA6819_Reg11Tbl3Value), WA6819_Reg11Tbl3Value },
    { 0x12, sizeof(WA6819_Reg12Tbl3Value), WA6819_Reg12Tbl3Value },
    { 0x13, sizeof(WA6819_Reg13Tbl3Value), WA6819_Reg13Tbl3Value },
    { 0x14, sizeof(WA6819_Reg14Tbl3Value), WA6819_Reg14Tbl3Value },
    { 0x15, sizeof(WA6819_Reg15Tbl3Value), WA6819_Reg15Tbl3Value },
    { 0x16, sizeof(WA6819_Reg16Tbl3Value), WA6819_Reg16Tbl3Value },
    { 0x17, sizeof(WA6819_Reg17Tbl3Value), WA6819_Reg17Tbl3Value },
    { 0x18, sizeof(WA6819_Reg18Tbl3Value), WA6819_Reg18Tbl3Value },
    { 0x19, sizeof(WA6819_Reg19Tbl3Value), WA6819_Reg19Tbl3Value },
    { 0x1A, sizeof(WA6819_Reg1ATbl3Value), WA6819_Reg1ATbl3Value },
    { 0x1B, sizeof(WA6819_Reg1BTbl3Value), WA6819_Reg1BTbl3Value },
    { 0x1C, sizeof(WA6819_Reg1CTbl3Value), WA6819_Reg1CTbl3Value },
    { 0x1D, sizeof(WA6819_Reg1DTbl3Value), WA6819_Reg1DTbl3Value },
    { 0x1E, sizeof(WA6819_Reg1ETbl3Value), WA6819_Reg1ETbl3Value },
    { 0x1F, sizeof(WA6819_Reg1FTbl3Value), WA6819_Reg1FTbl3Value },
    { 0x20, sizeof(WA6819_Reg20Tbl3Value), WA6819_Reg20Tbl3Value },
    { 0x21, sizeof(WA6819_Reg21Tbl3Value), WA6819_Reg21Tbl3Value },
    { 0x22, sizeof(WA6819_Reg22Tbl3Value), WA6819_Reg22Tbl3Value },
    { 0x23, sizeof(WA6819_Reg23Tbl3Value), WA6819_Reg23Tbl3Value },
    { 0x24, sizeof(WA6819_Reg24Tbl3Value), WA6819_Reg24Tbl3Value },
    { 0x25, sizeof(WA6819_Reg25Tbl3Value), WA6819_Reg25Tbl3Value },
    { 0x26, sizeof(WA6819_Reg26Tbl3Value), WA6819_Reg26Tbl3Value },
    { 0x27, sizeof(WA6819_Reg27Tbl3Value), WA6819_Reg27Tbl3Value },
    { 0x28, sizeof(WA6819_Reg28Tbl3Value), WA6819_Reg28Tbl3Value },
    { 0x29, sizeof(WA6819_Reg29Tbl3Value), WA6819_Reg29Tbl3Value },
    { 0x2A, sizeof(WA6819_Reg2ATbl3Value), WA6819_Reg2ATbl3Value },
    { 0x2B, sizeof(WA6819_Reg2BTbl3Value), WA6819_Reg2BTbl3Value },
    { 0x2C, sizeof(WA6819_Reg2CTbl3Value), WA6819_Reg2CTbl3Value },
    { 0x2D, sizeof(WA6819_Reg2DTbl3Value), WA6819_Reg2DTbl3Value },
    { 0x2E, sizeof(WA6819_Reg2ETbl3Value), WA6819_Reg2ETbl3Value },
    { 0x2F, sizeof(WA6819_Reg2FTbl3Value), WA6819_Reg2FTbl3Value },
    { 0x30, sizeof(WA6819_Reg30Tbl3Value), WA6819_Reg30Tbl3Value },
    { 0x31, sizeof(WA6819_Reg31Tbl3Value), WA6819_Reg31Tbl3Value },
    { 0x5C, sizeof(WA6819_Reg5CTbl3Value), WA6819_Reg5CTbl3Value },
    { 0x5D, sizeof(WA6819_Reg5DTbl3Value), WA6819_Reg5DTbl3Value },
    { 0x5E, sizeof(WA6819_Reg5ETbl3Value), WA6819_Reg5ETbl3Value },
    { 0x5F, sizeof(WA6819_Reg5FTbl3Value), WA6819_Reg5FTbl3Value },
    { 0x60, sizeof(WA6819_Reg60Tbl3Value), WA6819_Reg60Tbl3Value },
    { 0x61, sizeof(WA6819_Reg61Tbl3Value), WA6819_Reg61Tbl3Value },
    { 0x62, sizeof(WA6819_Reg62Tbl3Value), WA6819_Reg62Tbl3Value },
    { 0x63, sizeof(WA6819_Reg63Tbl3Value), WA6819_Reg63Tbl3Value },
    { 0x64, sizeof(WA6819_Reg64Tbl3Value), WA6819_Reg64Tbl3Value },
    { 0x65, sizeof(WA6819_Reg65Tbl3Value), WA6819_Reg65Tbl3Value },
    { 0x66, sizeof(WA6819_Reg66Tbl3Value), WA6819_Reg66Tbl3Value },
    { 0x67, sizeof(WA6819_Reg67Tbl3Value), WA6819_Reg67Tbl3Value },
    { 0x68, sizeof(WA6819_Reg68Tbl3Value), WA6819_Reg68Tbl3Value },
    { 0x69, sizeof(WA6819_Reg69Tbl3Value), WA6819_Reg69Tbl3Value },
    { 0x6A, sizeof(WA6819_Reg6ATbl3Value), WA6819_Reg6ATbl3Value },
    { 0x6B, sizeof(WA6819_Reg6BTbl3Value), WA6819_Reg6BTbl3Value },
    { 0x6C, sizeof(WA6819_Reg6CTbl3Value), WA6819_Reg6CTbl3Value },
    { 0x6D, sizeof(WA6819_Reg6DTbl3Value), WA6819_Reg6DTbl3Value },
    { 0x6E, sizeof(WA6819_Reg6ETbl3Value), WA6819_Reg6ETbl3Value },
    { 0x6F, sizeof(WA6819_Reg6FTbl3Value), WA6819_Reg6FTbl3Value },
    { 0x70, sizeof(WA6819_Reg70Tbl3Value), WA6819_Reg70Tbl3Value },
    { 0x71, sizeof(WA6819_Reg71Tbl3Value), WA6819_Reg71Tbl3Value },
    { 0x72, sizeof(WA6819_Reg72Tbl3Value), WA6819_Reg72Tbl3Value },
    { 0x73, sizeof(WA6819_Reg73Tbl3Value), WA6819_Reg73Tbl3Value },
    { 0x74, sizeof(WA6819_Reg74Tbl3Value), WA6819_Reg74Tbl3Value },
    { 0x75, sizeof(WA6819_Reg75Tbl3Value), WA6819_Reg75Tbl3Value },
    { 0x76, sizeof(WA6819_Reg76Tbl3Value), WA6819_Reg76Tbl3Value },
    { 0x77, sizeof(WA6819_Reg77Tbl3Value), WA6819_Reg77Tbl3Value },
    { 0x78, sizeof(WA6819_Reg78Tbl3Value), WA6819_Reg78Tbl3Value },
    { 0x79, sizeof(WA6819_Reg79Tbl3Value), WA6819_Reg79Tbl3Value },
};

static WA6819_REG WA6819_InitTb4[] =
{
    { 0x7E, sizeof(WA6819_Reg7ETbl4Value), WA6819_Reg7ETbl4Value }, // Enable APEQ settings
};

static WA6819_REG WA6819_CoefficientTb5[] =
{
    // APEQ Coefficients
    { 0x00, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x01, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x02, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x03, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x04, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x05, sizeof(WA6819_Reg4ByteValue3), WA6819_Reg4ByteValue3 },
    { 0x06, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x07, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x08, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x09, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0A, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0B, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0C, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0D, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0E, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x0F, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x10, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x11, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x12, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x13, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x14, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x15, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x16, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x17, sizeof(WA6819_Reg4ByteValue2), WA6819_Reg4ByteValue2 },
    { 0x1F, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x20, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x21, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x22, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x23, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
    { 0x24, sizeof(WA6819_Reg4ByteValue1), WA6819_Reg4ByteValue1 },
};

static WA6819_REG WA6819_InitTb6[] =
{
    { 0x7E, sizeof(WA6819_Reg7ETbl6Value), WA6819_Reg7ETbl6Value }, // Disable coefficient write for ch1/ch2
    { 0x33, sizeof(WA6819_Reg33Tbl6Value), WA6819_Reg33Tbl6Value }, // Soft Mute Off Control/ch2
    { 0x19, sizeof(WA6819_Reg19Tbl6Value), WA6819_Reg19Tbl6Value }, // APEQ Path Option
    { 0x1A, sizeof(WA6819_Reg1ATbl6Value), WA6819_Reg1ATbl6Value }, // APEQ1 At/Re time
    { 0x1B, sizeof(WA6819_Reg1BTbl6Value), WA6819_Reg1BTbl6Value }, // APEQ2 At/Re time
    { 0x1C, sizeof(WA6819_Reg1CTbl6Value), WA6819_Reg1CTbl6Value }, // APEQ3 At/Re time
    { 0x1D, sizeof(WA6819_Reg1DTbl6Value), WA6819_Reg1DTbl6Value }, // APEQ4 At/Re time
    { 0x1E, sizeof(WA6819_Reg1ETbl6Value), WA6819_Reg1ETbl6Value }, // APEQ5 At/Re time
    { 0x1F, sizeof(WA6819_Reg1FTbl6Value), WA6819_Reg1FTbl6Value }, // APEQ6 At/Re time
    { 0x0E, sizeof(WA6819_Reg0ETbl6Value1), WA6819_Reg0ETbl6Value1 }, // CH1 BQ1 ~ BQ3 Enable
    { 0x0F, sizeof(WA6819_Reg0FTbl6Value1), WA6819_Reg0FTbl6Value1 }, // CH2 BQ1 ~ BQ3 Enable
    { 0x10, sizeof(WA6819_Reg10Tbl6Value), WA6819_Reg10Tbl6Value }, // CH1 BQ4 ~ BQ6 Enable
    { 0x11, sizeof(WA6819_Reg11Tbl6Value), WA6819_Reg11Tbl6Value }, // CH2 BQ4 ~ BQ6 Enable
    { 0x12, sizeof(WA6819_Reg12Tbl6Value), WA6819_Reg12Tbl6Value }, // CH1 BQ7 ~ BQ10 Enable
    { 0x13, sizeof(WA6819_Reg13Tbl6Value), WA6819_Reg13Tbl6Value }, // CH2 BQ7 ~ BQ10 Enable
    { 0x14, sizeof(WA6819_Reg14Tbl6Value), WA6819_Reg14Tbl6Value }, // CH1 BQ11 ~ BQ12 Enable
    { 0x15, sizeof(WA6819_Reg15Tbl6Value), WA6819_Reg15Tbl6Value }, // CH2 BQ11 ~ BQ12 Enable
    { 0x68, sizeof(WA6819_Reg68Tbl6Value), WA6819_Reg68Tbl6Value }, // Phase Control
    { 0x24, sizeof(WA6819_Reg24Tbl6Value), WA6819_Reg24Tbl6Value }, // RS DRC Enable 
    { 0x25, sizeof(WA6819_Reg25Tbl6Value), WA6819_Reg25Tbl6Value }, // Vrms Period for Low-Band 
    { 0x4D, sizeof(WA6819_Reg4DTbl6Value), WA6819_Reg4DTbl6Value }, // Vrms Period for High-Band 
    { 0x4E, sizeof(WA6819_Reg4ETbl6Value), WA6819_Reg4ETbl6Value }, // Vrms Period for Sub-Band 
    { 0x4F, sizeof(WA6819_Reg4FTbl6Value), WA6819_Reg4FTbl6Value }, // Vrms Period for Post-Band 
    { 0x2D, sizeof(WA6819_Reg2DTbl6Value), WA6819_Reg2DTbl6Value }, // Threshold Table Mapping 
    { 0x07, sizeof(WA6819_Reg07Tbl6Value), WA6819_Reg07Tbl6Value }, // Vrms DRC Release Filter On 
    { 0x0E, sizeof(WA6819_Reg0ETbl6Value2), WA6819_Reg0ETbl6Value2 }, // BQ1 Loudness filter CH1 
    { 0x0F, sizeof(WA6819_Reg0FTbl6Value2), WA6819_Reg0FTbl6Value2 }, // BQ1 Loudness filter CH2 
    { 0x31, sizeof(WA6819_Reg31Tbl6Value), WA6819_Reg31Tbl6Value }, // ReBASS Parameter Selection 
    { 0x32, sizeof(WA6819_Reg32Tbl6Value), WA6819_Reg32Tbl6Value }, // ReBASS Parameter Value 
    { 0x08, sizeof(WA6819_Reg08Tbl6Value), WA6819_Reg08Tbl6Value }, // TV Volume 
};

static WA6819_REG WA6819_InitTb7[] = {}; //Only use ON PBTL MODE

static WA6819_REG WA6819_InitTb8[] =
{
    { 0x3D, sizeof(WA6819_Reg3DTbl8Value), WA6819_Reg3DTbl8Value }, // WCK Synchronizing Control 
    { 0x0C, sizeof(WA6819_Reg0CTbl8Value), WA6819_Reg0CTbl8Value }, // Master volume Control 
    { 0x16, sizeof(WA6819_Reg16Tbl8Value), WA6819_Reg16Tbl8Value }, // Master Fine Volume : 0 dB 
    { 0x35, sizeof(WA6819_Reg35Tbl8Value), WA6819_Reg35Tbl8Value }, //Reset the Auto_PWM_MASK_restore_counter to 0, PWM MASK output is high 
    { 0x34, sizeof(WA6819_Reg34Tbl8Value), WA6819_Reg34Tbl8Value }, // PWM switching off 
    { 0x33, sizeof(WA6819_Reg33Tbl8Value), WA6819_Reg33Tbl8Value }, // soft-mute off (increase Volume) 
};

unsigned short WA6819_I2C1byteWrite(int amp_i2c_id, unsigned short slave_addr, WA6819_REG *WA6819_Tbl, unsigned short num)
{
    unsigned char data[2];
    unsigned char data_len;
    unsigned char index;
    for(index = 0; index < num ; index ++) {
        data[0] = WA6819_Tbl[index].bAddr;
        data_len = WA6819_Tbl[index].bDataLen;
        memcpy(&data[1],WA6819_Tbl[index].bArray,WA6819_Tbl[index].bDataLen);

        #if AMP_DEBUG
        AMP_ERR("\n===write addr:0x%x len:%d  data1:0x%x===\n",data[0],data_len,data[1]);
        AMP_ERR("\n");
        #endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data, 2, I2C_M_FAST_SPEED) < 0) {
            AMP_ERR("program wa6819 failed\n");
        }
    }

    return num;
}


unsigned short WA6819_I2C4byteWrite(int amp_i2c_id, unsigned short slave_addr, WA6819_REG *WA6819_Tbl, unsigned short num)
{
    unsigned char data[5];
    unsigned char data_len;
    unsigned char index;
    for(index = 0; index < num ; index ++) {
        data[0] = WA6819_Tbl[index].bAddr;
        data_len = WA6819_Tbl[index].bDataLen;
        memcpy(&data[1],WA6819_Tbl[index].bArray,WA6819_Tbl[index].bDataLen);
        #if AMP_DEBUG
        AMP_ERR("\n===write addr:0x%x len:%d  data1:0x%x data2:0x%x data3:0x%x data4:0x%x===\n",data[0],data_len,data[1],data[2],data[3],data[4]);
        AMP_ERR("\n");
        #endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr, data, 5, I2C_M_FAST_SPEED) < 0) {
            AMP_ERR("program wa6819 failed\n");
        }
    }

    return num;
}

void wa6819_func(int amp_i2c_id, unsigned short slave_addr)
{
    RTK_GPIO_ID AMP_RESET_pin;
    unsigned long long param = 0 ;
    if (pcb_mgr_get_enum_info_byname("PIN_AMP_RESET", &param) != 0) {
        AMP_WARN("%s PIN_AMP_RESET is not existed , please check pcb parameters = %llx \n", __func__, param);
    }
    AMP_RESET_pin = (GET_PIN_TYPE(param) == PCB_PIN_TYPE_ISO_GPIO) ? rtk_gpio_id(ISO_GPIO, GET_PIN_INDEX(param)) : rtk_gpio_id(MIS_GPIO, GET_PIN_INDEX(param)); 
    mdelay(10);
    rtk_gpio_output(AMP_RESET_pin, 1);
    mdelay(5);
    AMP_ERR("program wa6819 write\n");
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb0, sizeof(WA6819_InitTb0)/sizeof(WA6819_REG));
    WA6819_I2C4byteWrite(amp_i2c_id, slave_addr, WA6819_CoefficientTbl, sizeof(WA6819_CoefficientTbl)/sizeof(WA6819_REG));
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb2, sizeof(WA6819_InitTb2)/sizeof(WA6819_REG));
    WA6819_I2C4byteWrite(amp_i2c_id, slave_addr, WA6819_CoefficientTb3, sizeof(WA6819_CoefficientTb3)/sizeof(WA6819_REG));
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb4, sizeof(WA6819_InitTb4)/sizeof(WA6819_REG));
    WA6819_I2C4byteWrite(amp_i2c_id, slave_addr, WA6819_CoefficientTb5, sizeof(WA6819_CoefficientTb5)/sizeof(WA6819_REG));
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb6, sizeof(WA6819_InitTb6)/sizeof(WA6819_REG));
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb7, sizeof(WA6819_InitTb7)/sizeof(WA6819_REG));
    WA6819_I2C1byteWrite(amp_i2c_id, slave_addr, WA6819_InitTb8, sizeof(WA6819_InitTb8)/sizeof(WA6819_REG));
}
