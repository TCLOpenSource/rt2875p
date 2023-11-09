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
#include <rtk_kdriver/rtk_gpio.h>
//*************************************************************
// PUBLIC FUNCTIONS
//*************************************************************
typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *bArray;
}TAS5707_REG;

typedef struct
{
    unsigned char bAddr;
    unsigned char bDataLen;
    unsigned char *reg_name;
}TAS5707_DUMP;

//clear error status
unsigned char    Reg02Value[]={0x00};
unsigned char    Reg05Value[]={0x84};
//mixer & scale
unsigned char    Reg06Value[]={0x00};
unsigned char    Reg0AValue[]={0x3A};

#ifdef Board_AMP_POWER_8OHM
unsigned char    Reg08Value[]={0x1a};
unsigned char    Reg09Value[]={0x1a};
#else
unsigned char    Reg08Value[]={0x10};
unsigned char    Reg09Value[]={0x10};
#endif

unsigned char    Reg0EValue[]={0xD1};
unsigned char    Reg19Value[]={0x30};
unsigned char    Reg1BValue[]={0x00};

#ifdef Board_AMP_POWER_8OHM
unsigned char    Reg07Value[]={0x30};
#else
unsigned char    Reg07Value[]={0x3a};
#endif

unsigned char    Reg14Value[]={0x00};
unsigned char    Reg13Value[]={0x00};
unsigned char    Reg12Value[]={0x54};
unsigned char    Reg11Value[]={0xAC};
unsigned char    Reg03Value[]={0x80};
unsigned char    Reg1AValue[]={0x95};
unsigned char    Reg20Value[]={0x00,0x01,0x77,0x72};
unsigned char    Reg21Value[]={0x00,0x00,0x42,0x03};
unsigned char    Reg25Value[]={0x01,0x10,0x32,0x45};
unsigned char    Reg50Value[]={0x00,0x00,0x00,0x02};
unsigned char    Reg52Value[]={0x0f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

//EQ
#ifdef Board_AMP_POWER_8OHM
unsigned char    Reg29Value[]={0x00,0x7D,0xFE,0x49,0xFF,0x2B,0xE6,0xD8,0x00,0x6E,0x7E,0x40,0x00,0xD4,0x19,0x28,0xFF,0x93,0x83,0x77};
unsigned char    Reg2AValue[]={0x00,0x81,0x40,0xC0,0xFF,0x0A,0x34,0x96,0x00,0x75,0x11,0xAC,0x00,0xF5,0xCB,0x6A,0xFF,0x89,0xAD,0x93};
unsigned char    Reg2BValue[]={0x00,0x7D,0xFE,0x49,0xFF,0x3B,0x5E,0x62,0x00,0x6E,0x7E,0x40,0x00,0xC4,0xA1,0x9E,0xFF,0x93,0x83,0x77};
unsigned char    Reg2CValue[]={0x00,0x7D,0x6B,0x7C,0xFF,0x05,0x29,0x08,0x00,0x7D,0x6B,0x7C,0x00,0xFA,0xC9,0xA7,0xFF,0x85,0x1B,0xB7};
unsigned char    Reg2DValue[]={0x00,0x7D,0x6B,0x7C,0xFF,0x05,0x29,0x08,0x00,0x7D,0x6B,0x7C,0x00,0xFA,0xC9,0xA7,0xFF,0x85,0x1B,0xB7};
unsigned char    Reg2EValue[]={0x00,0x84,0xF7,0x6C,0xFF,0x60,0x0A,0x32,0x00,0x5D,0x40,0x59,0x00,0x9F,0xF5,0xCE,0xFF,0x9D,0xC8,0x3B};
unsigned char    Reg2FValue[]={0x00,0x8B,0x28,0xB2,0xFF,0xB2,0xBC,0x64,0x00,0x3E,0xBD,0xEA,0x00,0x4D,0x43,0x9C,0xFF,0xB6,0x19,0x63};
unsigned char    Reg30Value[]={0x00,0x7D,0xFE,0x49,0xFF,0x2B,0xE6,0xD8,0x00,0x6E,0x7E,0x40,0x00,0xD4,0x19,0x28,0xFF,0x93,0x83,0x77};
unsigned char    Reg31Value[]={0x00,0x81,0x40,0xC0,0xFF,0x0A,0x34,0x96,0x00,0x75,0x11,0xAC,0x00,0xF5,0xCB,0x6A,0xFF,0x89,0xAD,0x93};
unsigned char    Reg32Value[]={0x00,0x7D,0xFE,0x49,0xFF,0x3B,0x5E,0x62,0x00,0x6E,0x7E,0x40,0x00,0xC4,0xA1,0x9E,0xFF,0x93,0x83,0x77};
unsigned char    Reg33Value[]={0x00,0x7D,0x6B,0x7C,0xFF,0x05,0x29,0x08,0x00,0x7D,0x6B,0x7C,0x00,0xFA,0xC9,0xA7,0xFF,0x85,0x1B,0xB7};
unsigned char    Reg34Value[]={0x00,0x7D,0x6B,0x7C,0xFF,0x05,0x29,0x08,0x00,0x7D,0x6B,0x7C,0x00,0xFA,0xC9,0xA7,0xFF,0x85,0x1B,0xB7};
unsigned char    Reg35Value[]={0x00,0x84,0xF7,0x6C,0xFF,0x60,0x0A,0x32,0x00,0x5D,0x40,0x59,0x00,0x9F,0xF5,0xCE,0xFF,0x9D,0xC8,0x3B};
unsigned char    Reg36Value[]={0x00,0x8B,0x28,0xB2,0xFF,0xB2,0xBC,0x64,0x00,0x3E,0xBD,0xEA,0x00,0x4D,0x43,0x9C,0xFF,0xB6,0x19,0x63};
#else
unsigned char   Reg29Value[]={0x00,0x80,0x6D,0xA1,0xFF,0x04,0x16,0x3F,0x00,0x7C,0x43,0x8A,0x00,0xFB,0xE9,0xC1,0xFF,0x83,0x4E,0xD4};
unsigned char   Reg2AValue[]={0x00,0x80,0x66,0x07,0xFF,0x0B,0x53,0x74,0x00,0x79,0x11,0xA4,0x00,0xF4,0xAC,0x8C,0xFF,0x86,0x88,0x55};
unsigned char   Reg2BValue[]={0x00,0x7E,0x3C,0x31,0xFF,0x03,0x87,0x9E,0x00,0x7E,0x3C,0x31,0x00,0xFC,0x72,0x27,0xFF,0x83,0x81,0x63};
unsigned char   Reg2CValue[]={0x00,0x85,0x7F,0xA9,0xFF,0x7E,0x03,0x5A,0x00,0x50,0x07,0x36,0x00,0x81,0xFC,0xA6,0xFF,0xAA,0x79,0x20};
unsigned char   Reg2DValue[]={0x00,0x83,0xDB,0x0D,0xFF,0xE2,0x79,0x00,0x00,0x5E,0x5C,0xB7,0x00,0x1D,0x87,0x00,0xFF,0x9D,0xC8,0x3B};
unsigned char   Reg2EValue[]={0x00,0x85,0x7F,0xA9,0x00,0x37,0x43,0xCB,0x00,0x50,0x07,0x36,0xFF,0xC8,0xBC,0x35,0xFF,0xAA,0x79,0x20};
unsigned char   Reg30Value[]={0x00,0x80,0x6D,0xA1,0xFF,0x04,0x16,0x3F,0x00,0x7C,0x43,0x8A,0x00,0xFB,0xE9,0xC1,0xFF,0x83,0x4E,0xD4};
unsigned char   Reg31Value[]={0x00,0x80,0x66,0x07,0xFF,0x0B,0x53,0x74,0x00,0x79,0x11,0xA4,0x00,0xF4,0xAC,0x8C,0xFF,0x86,0x88,0x55};
unsigned char   Reg32Value[]={0x00,0x7E,0x3C,0x31,0xFF,0x03,0x87,0x9E,0x00,0x7E,0x3C,0x31,0x00,0xFC,0x72,0x27,0xFF,0x83,0x81,0x63};
unsigned char   Reg33Value[]={0x00,0x85,0x7F,0xA9,0xFF,0x7E,0x03,0x5A,0x00,0x50,0x07,0x36,0x00,0x81,0xFC,0xA6,0xFF,0xAA,0x79,0x20};
unsigned char   Reg34Value[]={0x00,0x83,0xDB,0x0D,0xFF,0xE2,0x79,0x00,0x00,0x5E,0x5C,0xB7,0x00,0x1D,0x87,0x00,0xFF,0x9D,0xC8,0x3B};
unsigned char   Reg35Value[]={0x00,0x85,0x7F,0xA9,0x00,0x37,0x43,0xCB,0x00,0x50,0x07,0x36,0xFF,0xC8,0xBC,0x35,0xFF,0xAA,0x79,0x20};
#endif

unsigned char   Reg5AValue[]={0x00,0x00,0x05,0x83,0x00,0x00,0x0B,0x06,0x00,0x00,0x05,0x83,0x00,0xFB,0x42,0xC1,0x0F,0x84,0xA7,0x33};
unsigned char   Reg5BValue[]={0x00,0x00,0x05,0x83,0x00,0x00,0x0B,0x06,0x00,0x00,0x05,0x83,0x00,0xFB,0x42,0xC1,0x0F,0x84,0xA7,0x33};

unsigned char   Reg3AValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char   Reg3BValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char   Reg3CValue[]={0x00,0x00,0x00,0xF9,0x00,0x7F,0xFF,0x06};
unsigned char   Reg3DValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char   Reg3EValue[]={0x00,0x05,0x39,0x47,0x00,0x7A,0xC6,0xB8};
unsigned char   Reg3FValue[]={0x00,0x00,0x00,0xF9,0x00,0x7F,0xFF,0x06};

unsigned char   Reg40Value[]={0xfd,0xD7,0x3B,0x2E}; 
unsigned char   Reg41Value[]={0x0F,0x70,0x80,0x00};
unsigned char   Reg42Value[]={0x00,0x08,0x42,0x10};
unsigned char   Reg43Value[]={0xFD,0x97,0x73,0x3D}; 
unsigned char   Reg44Value[]={0x0F,0x81,0x47,0xAF};
unsigned char   Reg45Value[]={0x00,0x08,0x42,0x10};
unsigned char   Reg46Value[]={0x00,0x00,0x00,0x03};

static TAS5707_REG TAS5707_InitTbl[] =
{
    {TAS5707_OSCILLATOR_TRIM,sizeof(Reg1BValue), Reg1BValue},
    {TAS5707_MASTER_VOLUME  ,sizeof(Reg07Value), Reg07Value},  //must all vol unmute and all_vol + 0db
    {TAS5707_CHANNEL_1_VOL  ,sizeof(Reg08Value), Reg08Value},  //must vol_L + 16db
    {TAS5707_CHANNEL_2_VOL  ,sizeof(Reg09Value), Reg09Value},  //must vol_R + 16db
    {TAS5707_SYSTEM_CTRL2   ,sizeof(Reg05Value), Reg05Value},  //Exit Shutdown Mode, must final
};

static TAS5707_DUMP TAS5707_Dump[] =
{
    {TAS5707_CLOCK_CTRL           ,1 ,"Clock_CTRL"        },
    {TAS5707_DEVICE_ID            ,1 ,"DEVICE_ID"         },
    {TAS5707_ERROR_STATUS         ,1 ,"ERROR_STATUS"      },
    {TAS5707_SYSTEM_CTRL1         ,1 ,"SYSTEM_CTRL1"      },
    {TAS5707_DATA_INTERFACE       ,1 ,"DATA_INTERFACE"    },
    {TAS5707_SYSTEM_CTRL2         ,1 ,"SYSTEM_CTRL2"      },
    {TAS5707_SOFT_MUTE            ,1 ,"SOFT_MUTE"         },
    {TAS5707_MASTER_VOLUME        ,1 ,"MASTER_VOLUME"     },
    {TAS5707_CHANNEL_1_VOL        ,1 ,"CHANNEL_1_VOL"     },
    {TAS5707_CHANNEL_2_VOL        ,1 ,"CHANNEL_2_VOL"     },
    {TAS5707_FINE_MASTER_VOL      ,1 ,"FINE_MASTER_VOL"   },
    {TAS5707_VOLUME_CONFIG        ,1 ,"VOLUME_CONFIG"     },
    {TAS5707_MODULATION_LIMIT     ,1 ,"MODULATION_LIMIT"  },
    {TAS5707_IC_DELAY_CHANNEL1    ,1 ,"IC_DELAY_CHANNEL1" },
    {TAS5707_IC_DELAY_CHANNEL2    ,1 ,"IC_DELAY_CHANNEL2" },
    {TAS5707_IC_DELAY_CHANNEL3    ,1 ,"IC_DELAY_CHANNEL3" },
    {TAS5707_IC_DELAY_CHANNEL4    ,1 ,"IC_DELAY_CHANNEL4" },
    {TAS5707_START_STOP_PERIOD    ,1 ,"START_STOP_PERIOD" },
    {TAS5707_OSCILLATOR_TRIM      ,1 ,"OSCILLATOR_TRIM"   },
    {TAS5707_BKND_ERR             ,1 ,"BKND_ERR"          },
    {TAS5707_INPUT_MUX            ,4 ,"INPUT_MUX"         },
    {TAS5707_PWM_MUX              ,4 ,"PWM_MUX"           },
    {TAS5707_CH1_BQ0              ,20,"CH1_BQ0"           },
    {TAS5707_CH1_BQ1              ,20,"CH1_BQ1"           },
    {TAS5707_CH1_BQ2              ,20,"CH1_BQ2"           },
    {TAS5707_CH1_BQ3              ,20,"CH1_BQ3"           },
    {TAS5707_CH1_BQ4              ,20,"CH1_BQ4"           },
    {TAS5707_CH1_BQ5              ,20,"CH1_BQ5"           },
    {TAS5707_CH1_BQ6              ,20,"CH1_BQ6"           },
    {TAS5707_CH2_BQ0              ,20,"CH2_BQ0"           },
    {TAS5707_CH2_BQ1              ,20,"CH2_BQ1"           },
    {TAS5707_CH2_BQ2              ,20,"CH2_BQ2"           },
    {TAS5707_CH2_BQ3              ,20,"CH2_BQ3"           },
    {TAS5707_CH2_BQ4              ,20,"CH2_BQ4"           },
    {TAS5707_CH2_BQ5              ,20,"CH2_BQ5"           },
    {TAS5707_CH2_BQ6              ,20,"CH2_BQ6"           },
    {TAS5707_DRC_AE               ,8 ,"DRC_AE"            },
    {TAS5707_DRC_AA               ,8 ,"DRC_AA"            },
    {TAS5707_DRC_AD               ,8 ,"DRC_AD"            },
    {TAS5707_DRC_T                ,4 ,"DRC_T"             },
    {TAS5707_DRC_K                ,4 ,"DRC_K"             },
    {TAS5707_DRC_O                ,4 ,"DRC_O"             },
    {TAS5707_DRC_CONTROL          ,4 ,"DRC_CONTROL"       },
    {TAS5707_BANK_SWITCH_CTRL     ,4 ,"BANK_SWITCH_CTRL"  },
    {TAS5707_UPDATE_ADDRESS       ,4 ,"UPDATE_ADDRESS"    },
};

void tas5707_amp_reset(void)
{
    RTK_GPIO_PCBINFO_T pinInfo;
    int ret = 0;

    AMP_WARN("\n tas5707_amp_reset\n");

    ret = rtk_gpio_get_pcb_info("PIN_AMP_RESET",&pinInfo);

    if(ret == 0){
        rtk_gpio_output(pinInfo.gid, 0);
        mdelay(5);
        rtk_gpio_output(pinInfo.gid, 1);
        mdelay(20);
    }
}

void tas5707_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[24];
    unsigned char data_len;
    unsigned char index;
#if TAS5707_DEBUG
    unsigned char i;
#endif
    AMP_WARN("\nTAS5707 Initial Start\n");
    for(index = 0; index < (sizeof(TAS5707_InitTbl)/sizeof(TAS5707_REG)); index ++)
    {
        data[0] = TAS5707_InitTbl[index].bAddr;
        data_len = TAS5707_InitTbl[index].bDataLen;
        memcpy(&data[1],TAS5707_InitTbl[index].bArray,TAS5707_InitTbl[index].bDataLen);

#if TAS5707_DEBUG
        AMP_WARN("===write addr:0x%x len:%d===\n",data[0],data_len);
        for(i = 0 ; i < TAS5707_InitTbl[index].bDataLen; ++i)
            AMP_WARN("%x ", data[i]);

#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data, data_len+1 ,I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program TAS5707 failed\n");

    }
}

int tas5707_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int index = 0;
    int data_len = 0;
    int data_index = 0;
    unsigned char addr[1] ={0};
    unsigned char *data = NULL;
    //Read SUBADDRESS Register
    AMP_WARN("== Dump SUBADDRESS 00~F9 Reg Size:%d==\n",sizeof(TAS5707_Dump)/sizeof(TAS5707_DUMP));
    for (index = 0; index < (sizeof(TAS5707_Dump)/sizeof(TAS5707_DUMP)) ;++index){
        addr[0] = TAS5707_Dump[index].bAddr;
        data_len = TAS5707_Dump[index].bDataLen;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        data = kmalloc(data_len, GFP_KERNEL);
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,data_len);
        AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5707_Dump[index].reg_name,addr[0],data_len);
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

int tas5707_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};
    unsigned char send_data[3] ={0};

    //Read
    addr[0] = TAS5707_SOFT_MUTE;
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,TAS5707_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",TAS5707_SOFT_MUTE);
        return (-1);
    }

    if (on_off == AMP_MUTE_ON){
        data[0] = TAS5707_MUTE_ON(data[0]);
    }
    else if (on_off == AMP_MUTE_OFF){
        data[0] = TAS5707_MUTE_OFF(data[0]);
    }

    //Write
    send_data[0] = TAS5707_SOFT_MUTE;
    send_data[1] = data[0];


    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,TAS5707_SOFT_MUTE,send_data[1]);
    }

    return ret;
}

int tas5707_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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

int tas5707_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
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
    for (index = 0; index < (sizeof(TAS5707_Dump)/sizeof(TAS5707_DUMP)) ;++index){
        if(addr[0] == TAS5707_Dump[index].bAddr){
            data_len = TAS5707_Dump[index].bDataLen;
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

    AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5707_Dump[index].reg_name,addr[0],data_len);
    for(data_index = 0 ; data_index < data_len; ++data_index){
        AMP_WARN("data[%d]:%x ", data_index, data[data_index]);
    }

failed:
    kfree(data);
    return ret;
}
