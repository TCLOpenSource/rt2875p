#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif

ALC1310_REG Alc1310_DefautTbl[] =
{
    //init
    {ALC_1310_MX05_PLL_SETTING,{0x40,0x3e}},
    {ALC_1310_MX02_SEQ_CTRL,{0x80,0x00}},
    {ALC_1310_MX09_DRC_SETTING_1,{0x80,0x00}},
    //DC
    {ALC_1310_PR6A_REG_INDEX,{0x00,0xdc}},
    {ALC_1310_PR6C_REG_DATA,{0x00,0x24}},
    //D9
    {0x14,{0x00,0xd9}},
    {0x50,{0xa4,0x10}},

    //DRC
    {ALC_1310_MX09_DRC_SETTING_1,{0xC8,0x00}},//DRC Enable
    {ALC_1310_MX0A_DRC_SETTING_2,{0x05,0x00}},
    {ALC_1310_MX0C_DRC_SETTING_4,{0x04,0x02}},
    {ALC_1310_MX0E_DRC_SETTING_5,{0x06,0x00}},

    /* down layer register and value */
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x9d}},
    {ALC_1310_PR6C_REG_DATA,{0x46,0x00}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x9e}},
    {ALC_1310_PR6C_REG_DATA,{0x26,0x26}},

    //b0
    {ALC_1310_PR6A_REG_INDEX,{0x00,0xb0}},
    {ALC_1310_PR6C_REG_DATA,{0x88,0x03}},					//Silence detection:level is -80dB

    {ALC_1310_MX13_VOLUME_SETTING_2,{0x00,0x0c}},					//unmute
    {ALC_1310_MX13_VOLUME_SETTING_2,{0x00,0x0c}},					//unmute

    //{0x12,0x02,{0xff,0xff}},					//L/R Volume
    //{0x12,0x02,{0x00,0x00}},					//L/R Volume

    //BQ1 BQ1Ty= 4 BQ1Fs=44100 BQ1Fc=100BQ1Ga=1 BQ1Fb=120
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x00}}, 
    {ALC_1310_PR6C_REG_DATA,{0x02,0x00}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x01}},
    {ALC_1310_PR6C_REG_DATA,{0x87,0x8E}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x02}},
    {ALC_1310_PR6C_REG_DATA,{0x1C,0x09}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x03}},
    {ALC_1310_PR6C_REG_DATA,{0xD4,0xC5}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x04}},
    {ALC_1310_PR6C_REG_DATA,{0x01,0xF6}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x05}},
    {ALC_1310_PR6C_REG_DATA,{0x45,0x96}},
                            
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x06}},
    {ALC_1310_PR6C_REG_DATA,{0x03,0xF7}},
                            
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x07}},
    {ALC_1310_PR6C_REG_DATA,{0x37,0xBD}},
                            
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x08}},
    {ALC_1310_PR6C_REG_DATA,{0x1E,0x08}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x09}},
    {ALC_1310_PR6C_REG_DATA,{0xAD,0xE1}},


    //BQ2 BQ2Ty= 4 BQ2Fs=44100 BQ2Fc=6000 BQ2Ga=0.5 BQ2Fb=1500
    {ALC_1310_PR6A_REG_INDEX,{0x00,0xa}}, 
    {ALC_1310_PR6C_REG_DATA,{0x02,0x02}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x0b}},
    {ALC_1310_PR6C_REG_DATA,{0x9D,0x55}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x0c}},
    {ALC_1310_PR6C_REG_DATA,{0x1D,0x9C}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x0d}},
    {ALC_1310_PR6C_REG_DATA,{0xF6,0x0A}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x0e}},
    {ALC_1310_PR6C_REG_DATA,{0x01,0xA2}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x0f}},
    {ALC_1310_PR6C_REG_DATA,{0xFE,0xEA}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x10}},
    {ALC_1310_PR6C_REG_DATA,{0x02,0x66}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x11}},
    {ALC_1310_PR6C_REG_DATA,{0x28,0xC3}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x12}},
    {ALC_1310_PR6C_REG_DATA,{0x1E,0x58}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x13}},
    {ALC_1310_PR6C_REG_DATA,{0x40,0x03}},

    //BQ3 BQ3Ty= 4 BQ3Fs=44100 BQ3Fc=11000 BQ3Ga=1.5 BQ3Fb=14000
    {ALC_1310_PR6A_REG_INDEX,{0x00,0x14}},
    {ALC_1310_PR6C_REG_DATA,{0x02,0x25}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x15}},
    {ALC_1310_PR6C_REG_DATA,{0x88,0x62}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x16}},
    {ALC_1310_PR6C_REG_DATA,{0x1F,0xFD}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x17}},
    {ALC_1310_PR6C_REG_DATA,{0xEC,0x29}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x18}},
    {ALC_1310_PR6C_REG_DATA,{0x00,0x47}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x19}},
    {ALC_1310_PR6C_REG_DATA,{0x09,0xFB}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x1a}},
    {ALC_1310_PR6C_REG_DATA,{0x00,0x02}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x1b}},
    {ALC_1310_PR6C_REG_DATA,{0x3A,0xD3}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x1c}},
    {ALC_1310_PR6C_REG_DATA,{0x1F,0x8E}},

    {ALC_1310_PR6A_REG_INDEX,{0x00,0x1d}},
    {ALC_1310_PR6C_REG_DATA,{0x38,0x7E}},

    {ALC_1310_MX04_BQ_SETUP,{0xc7,0x00}},

    {ALC1310_END_FLAG ,{0x01,0x00}},
};
#define ALC1310_REG_DEFAULT_SIZE (sizeof(Alc1310_DefautTbl)/sizeof(ALC1310_REG))

void alc1310_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[8] = {0};
    int u8Index=0;
    int timer = 0;
    for(u8Index = 0; ; u8Index ++)
    {
        data[0] = Alc1310_DefautTbl[u8Index].bAddr;
        if(data[0] == ALC1310_END_FLAG)
        {
            break;
        }
        data[1] = Alc1310_DefautTbl[u8Index].bArray[0];
        data[2] = Alc1310_DefautTbl[u8Index].bArray[1];

        #ifdef AMP_DEBUG
            AMP_INFO("%s audio amp write addr:0x%x----data[1]=0x%x----data[2]=0x%x\n",__func__,data[0],data[1],data[2]);
        #endif

        if ( i2c_master_send_ex_flag(amp_i2c_id, slave_addr, &data[0], 3,I2C_M_FAST_SPEED) < 0 )
        {
            AMP_ERR("%s line:%d program ACL1310 failed\n",__func__, __LINE__);
            AMP_ERR("\nwrite addr:0x%x----data[1]=0x%x----data[2]=0x%x\n",data[0],data[1],data[2]);
            if(timer == 3){
                AMP_ERR("%s line:%d program ACL1310 failed times:%d\n",__func__, __LINE__,timer);
                break;
            }

            u8Index--;
            timer++;
            continue;
        }
        else
        {
            timer = 0;
        }
    }
}

int alc1310_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int i = 0;
    unsigned char addr[1] ={0};
    unsigned char data[3] ={0};
    //Read MX REgister
    AMP_WARN("== Dump MX 00~7F Reg ==\n");
    for (i=0x00; i<= 0x7f/*AMP_MX_REG_SIZE*/ ;++i){
        addr[0] = i;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1310_DATA_SIZE);
        AMP_WARN("MX_Reg:0x%x DataH:0x%x DataL:0x%x\n", addr[0],data[0], data[1]);
        mdelay(5);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            break;
       }
    }

    //Read PR Register
    AMP_WARN("== Dump PR 00~1F Reg ==\n");
    for (i=0; i<= AMP_PR_REG1_END ;++i){
    //PR Register Address Write
        data[0] = ALC_1310_PR6A_REG_INDEX;
        data[1] = 0;
        data[2] = i;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &data[0], 3) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",data[0]);
            break;
        }
        AMP_WARN("[6A][Addr] DataH:0x%x DataL:0x%x\n", data[1], data[2]);

        //PR Register Data Read
        addr[0] = ALC_1310_PR6C_REG_DATA;
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1310_DATA_SIZE);
        AMP_WARN("[6C][Data] DataH:0x%x DataL:0x%x\n", data[0], data[1]);
        mdelay(5);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            break;
       }
    }
    
    AMP_WARN("== Dump PR 90~FF Reg ==\n");
    for (i=AMP_PR_REG2_START; i<= AMP_PR_REG2_END ;++i){
        //PR Register Address Write
        data[0] = ALC_1310_PR6A_REG_INDEX;
        data[1] = 0;
        data[2] = i;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &data[0], 3) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",data[0]);
            break;
        }
        AMP_WARN("[6A][Addr] DataH:0x%x DataL:0x%x\n", data[1], data[2]);

        //PR Register Data Read
        addr[0] = ALC_1310_PR6C_REG_DATA;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1310_DATA_SIZE);
        AMP_WARN("[6C][Data] DataH:0x%x DataL:0x%x\n", data[0], data[1]);
        mdelay(5);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            break;
       }
    }

    return ret;
}

int alc1310_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[2] ={0};
    unsigned char send_data[3] ={0};

    //Read
    addr[0] = ALC_1310_MX13_VOLUME_SETTING_2;
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }
    
    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1310_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",ALC_1310_MX13_VOLUME_SETTING_2);
        return (-1);      
    }

    if (on_off == AMP_MUTE_ON){
        data[0] = ALC_1310_MUTE_ON(data[0]);
    }
    else if (on_off == AMP_MUTE_OFF){
        data[0] = ALC_1310_MUTE_OFF(data[0]);
    }

    //Write
    send_data[0] = ALC_1310_MX13_VOLUME_SETTING_2;
    send_data[1] = data[0];
    send_data[2] = data[1];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 3) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data_H:0x%x Data_L:0x%x failed\n",send_data[0],send_data[1],send_data[2]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data_H:0x%x Data_L:0x%x\n", slave_addr,ALC_1310_MX13_VOLUME_SETTING_2,data[0], data[1]);
    }

    return ret;
}

int alc1310_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
{

    unsigned char send_data[3] ={0};

    send_data[0] = data_wr[0];
    send_data[1] = data_wr[1];
    send_data[2] = data_wr[2];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 3) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data_H:0x%x Data_L:0x%x failed\n",send_data[0],send_data[1],send_data[2]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data_H:0x%x Data_L:0x%x\n", slave_addr,send_data[0],send_data[1], send_data[2]);
    }

    return 0;
}

int alc1310_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[2] ={0};

    //Read
    addr[0] = reg[0];
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1310_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",*reg);
        return (-1);
    }

    AMP_WARN("Addr:0x%x Reg:0x%x Data_H:0x%x Data_L:0x%x\n", slave_addr,addr[0],data[0], data[1]);

    return ret;
}
