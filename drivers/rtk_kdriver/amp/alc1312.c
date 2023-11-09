#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif

ALC1312_REG Alc1312_DefautTbl[] =
{
	/////////////////initial
	{ ALC_1312_MX0000_CLOCK0            , {0xFF,0xFF}, 0},

	// enable RC clock
	{ ALC_1312_MX0004_CLOCK2            , {0xC0,0x00}, 0},
	{ ALC_1312_MX0006_CLOCK3            , {0x35,0x70}, 0},
	{ ALC_1312_MX0008_CLOCK4            , {0x30,0x00}, 0},
	{ ALC_1312_MX000A_PLL1              , {0x10,0x5E}, 0},
	{ ALC_1312_MX000C_PLL2              , {0x00,0x00}, 0},
	{ ALC_1312_MX0322_POWER1            , {0x40,0x0F}, 0},
	{ ALC_1312_MX0326_POWER3            , {0x00,0x0F}, 0},
	{ ALC_1312_MX0350_POWER_STATUS      , {0x01,0x88}, 0},
	{ ALC_1312_MX0324_POWER2            , {0xC0,0xA0}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x05,0x10}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x25,0x00}, 0},
	{ ALC_1312_MX0103_MASTER_VOLUME_L   , {0x03,0x3E}, 0},//Master Volume L = 0dB
	{ ALC_1312_MX0104_MASTER_VOLUME_R   , {0x03,0x3E}, 0},//Master Volume L = 0dB
	{ ALC_1312_MX0300_CLASS_D           , {0xB8,0x03}, 0},//400K HV32dB
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x00,0x02}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x65,0x05}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x03,0x00}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x01,0x45}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x03,0x02}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x28,0x02}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x03,0x0C}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x0F,0xC0}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x01,0x0C}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0xC0,0x00}, 0},

	// Turn on Efuse & Auto Readback mode
	{ ALC_1312_MX0326_POWER3            , {0x00,0xAF}, 0},
	{ ALC_1312_MX0326_POWER3            , {0x00,0xFF}, 0},
	{ {0x00 ,0x80}                    , {0x80,0x00}, 0},
	{ {0x00 ,0x80}                    , {0x80,0x01}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x02,0x20}, 0},
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x00,0x00}, 0},


	// Turn off Efuse
	{ ALC_1312_MX0326_POWER3            , {0x00,0xAF}, 0},
	{ ALC_1312_MX0326_POWER3            , {0x00,0x0F}, 500}, // need to Delay 0.5sec


	// enable PLL
	{ ALC_1312_MX0004_CLOCK2            , {0x10,0x00}, 0},

	// Turn on PWM
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x02,0x16}, 0},// DC detect H_threshold
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x04,0x70}, 0},
	{ ALC_1312_MX006A_PRIVATE_REG_INDEX , {0x02,0x17}, 0},// DC detect L_threshold
	{ ALC_1312_MX006C_PRIVATE_REG_DATA  , {0x04,0x00}, 0},
	{ {0x02 ,0x14}                    , {0x80,0x00}, 0}, // DC detect enable
	{ ALC_1312_MX0350_POWER_STATUS      , {0x01,0x08}, 0},
	{ {0xFF ,0xFF}                    , {0xFF,0xFF}, 0},
};

#define ALC1312_REG_DEFAULT_SIZE (sizeof(Alc1312_DefautTbl)/sizeof(ALC1312_REG))

void alc1312_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[8] = {0};

    int i=0;
    int timer = 0;


    AMP_ERR("[%s] start \n", __func__ );
    for(i = 0; i < ALC1312_REG_DEFAULT_SIZE ; i++)
    {
        data[0] = Alc1312_DefautTbl[i].bAddr[0];
        data[1] = Alc1312_DefautTbl[i].bAddr[1];
        if(data[0] == ALC1312_END_FLAG && data[1] == ALC1312_END_FLAG )
        {
            break;
        }
        data[2] = Alc1312_DefautTbl[i].bArray[0];
        data[3] = Alc1312_DefautTbl[i].bArray[1];

        msleep( Alc1312_DefautTbl[i].delayTime );


        #ifdef ALC1312_DEBUG
            AMP_INFO("%s audio amp write addr[0]:0x%X----addr[1]:0x%X----data[1]=0x%X----data[2]=0x%X\n",__func__,data[0],data[1],data[2],data[3]);
        #endif

        if ( i2c_master_send_ex_flag(amp_i2c_id, slave_addr, &data[0], 4,I2C_M_FAST_SPEED) < 0 )
        {
            AMP_ERR("%s line:%d program ACL1312 failed\n",__func__, __LINE__);
            AMP_ERR("\nwrite addr[0]:0x%X----addr[1]:0x%X----data[1]=0x%X----data[2]=0x%X\n",data[0],data[1],data[2],data[3]);
            if(timer == 3){
                AMP_ERR("%s line:%d program ACL1312 failed times:%d\n",__func__, __LINE__,timer);
                break;
            }

            i--;
            timer++;
            continue;
        }
        else
        {
            timer = 0;
        }
    }

    AMP_ERR("[%s] done\n", __func__ );
}

int alc1312_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int i = 0;
    unsigned char addr[2] ={0};
    unsigned char data[2] ={0};


    for(i = 0; i < ALC1312_REG_DEFAULT_SIZE ; i++)
    {

        //Read
        addr[0] = Alc1312_DefautTbl[i].bAddr[0];
        addr[1] = Alc1312_DefautTbl[i].bAddr[1];

        if(addr[0] == ALC1312_END_FLAG && addr[1] == ALC1312_END_FLAG )
        {
            break;
        }

        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, addr, ALC1312_ADDR_SIZE) < 0 ){
            AMP_ERR("%s I2C Write Address Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
            return (-1);
        }

        data[0] =0;
        data[1] =0;        

        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1312_DATA_SIZE);

        if(result<0) {
            AMP_ERR("%s Read AMP Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
            return (-1);
        }

        AMP_WARN("%s Addr=0x%02X Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X\n", __func__ ,slave_addr,addr[0],addr[1],data[0], data[1]);

    }    

    // show mute vomlue control 

    addr[0] = 0x01;
    addr[1] = 0x02;

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, addr, ALC1312_ADDR_SIZE) < 0 )
    {
        AMP_ERR("%s I2C Write Address Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
        return (-1);
    }

    data[0] =0;
    data[1] =0;        

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1312_DATA_SIZE);

    if(result<0)
    {
        AMP_ERR("%s Read AMP Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
        return (-1);
    }

    AMP_WARN("%s Addr=0x%02X Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X\n", __func__ ,slave_addr,addr[0],addr[1],data[0], data[1]);


    return ret;
}

int alc1312_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;

    int result = 0;
    unsigned char addr[2] =ALC_1312_MX0102_SOFT_VOLUME;
    unsigned char data[2] ={0};
    unsigned char send_data[4] ={0};

    //Read
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], ALC1312_ADDR_SIZE) < 0 )
    {
        AMP_ERR("%s I2C Write Address Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ , addr[0] , addr[1] );
        return (-1);
    }
    
    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1312_DATA_SIZE);

    if(result<0)
    {
        AMP_ERR("%s Read AMP Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ , addr[0] , addr[1] );
        return (-1);      
    }

    if (on_off == AMP_MUTE_OFF){
        data[0] = ALC_1312_MUTE_OFF(data[0]);
    }
    else
    {
        data[0] = ALC_1312_MUTE_ON(data[0]);
    }

    //Write
    send_data[0] = addr[0];
    send_data[1] = addr[1];
    send_data[2] = data[0];
    send_data[3] = data[1];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 4) < 0 ){
        AMP_ERR("%s I2C Write Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X failed\n", __func__ ,send_data[0],send_data[1],send_data[2],send_data[3]);
        return (-1);
    }
    else{
        AMP_WARN("%s Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X \n", __func__ ,send_data[0],send_data[1],send_data[2],send_data[3]);
    }

    return ret;
}

int alc1312_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char send_data[4] ={0};

    send_data[0] = data_wr[0];
    send_data[1] = data_wr[1];
    send_data[2] = data_wr[2];
    send_data[3] = data_wr[3];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], ALC1312_ADDR_SIZE+ALC1312_DATA_SIZE) < 0 ){
        AMP_ERR("%s I2C Write Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X failed\n", __func__ ,send_data[0],send_data[1],send_data[2],send_data[3]);
        return (-1);
    }
    else{
        AMP_WARN("%s Addr=0x%02X Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X \n", __func__ , slave_addr,send_data[0],send_data[1], send_data[2],send_data[3]);
    }

    return 0;
}

int alc1312_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;

    int result = 0;
    unsigned char addr[2] ={0};
    unsigned char data[2] ={0};

    //Read
    addr[0] = reg[0];
    addr[1] = reg[1];
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, addr, ALC1312_ADDR_SIZE) < 0 ){
        AMP_ERR("%s I2C Write Address Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,ALC1312_DATA_SIZE);

    if(result<0) {
        AMP_ERR("%s Read AMP Reg[0]=0x%02X Reg[1]=0x%02X failed\n", __func__ ,addr[0] ,addr[1]);
        return (-1);
    }

    AMP_WARN("%s Addr=0x%02X Reg[0]=0x%02X Reg[1]=0x%02X Data[0]=0x%02X Data[1]=0x%02X\n", __func__ ,slave_addr,addr[0],addr[1],data[0], data[1]);

    return ret;
}
