#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif
#include "WA156819.h"
#include "rtk_amp_interface.h"

#define END_FLAG 	(0xff)
#define AMP_DEBUG 	(0)

typedef struct
{
    unsigned char bAddr;
    unsigned char bArray;
}WA156819_REG;

static WA156819_REG WA156819B_InitTbl[] = 
{
 {0x0B, 0x01},//Soft Reset contorl
 {0x02, 0x00},//MCLK ( 48k = 0x00, 96k = 0x01, 36k=0x02 )
 {0x5D, 0x01},//SHDN High
 {0x17, 0x9F},//channel 1 vol = 0.0 dB (Input 0dB )
 {0x18, 0x9F},//channel 2 vol = 0.0 dB (Input 0dB )
 {0x43, 0x02},//PWM output mode = DBTL(Ternary)
 {0x4A, 0x00},
 {0x3C, 0x7E},
 {0x76, 0x0F},//Monitor2 = SDATA Out
 {0x44, 0x0E},
 {WA156819_Soundon, 0xFF}, //sound on
 {END_FLAG ,0x00},//END
};

void WA156819_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2];
    int u8Index=0;
    int timer = 0;
    for(u8Index = 0; ; u8Index ++)
    {
        data[0] = WA156819B_InitTbl[u8Index].bAddr;
        data[1] = WA156819B_InitTbl[u8Index].bArray;

        if(data[0] == END_FLAG)
            break;

#if AD82010_DEBUG
        AMP_WARN("%s audio amp write Reg:0x%x----Data=0x%x\n",__func__,data[0],data[1]);
#endif

        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , &data[0], 2 ,I2C_M_FAST_SPEED) < 0)
        {
            AMP_ERR("%s line:%d program WA156819 failed\n",__func__, __LINE__);
            AMP_ERR("%s line:%d WA156819 Slave address:%x\n",__func__, __LINE__,slave_addr);
            AMP_ERR("\nwrite Reg:0x%x----Data=0x%x\n",data[0],data[1]);
            if(timer == 3){
                AMP_ERR("%s line:%d program WA156819 failed times:%d\n",__func__, __LINE__,timer);
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

void WA156819_amp_reset(void)
{

    AMP_WARN("wa156819_amp_reset\n");
    rtk_amp_pin_set("PIN_AMP_RESET", 0);
    mdelay(10);
    rtk_amp_pin_set("PIN_AMP_RESET", 1);
    mdelay(1);
}


int WA156819_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};
    unsigned char send_data[2] ={0};

    //Read
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,WA156819_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
        return (-1);
    }

    if (on_off == AMP_MUTE_ON){
        data[0] = WA156819_MUTE_ON(data[0]);
    }
    else if (on_off == AMP_MUTE_OFF){
        data[0] = WA156819_MUTE_OFF(data[0]);
    }

    //Write
    send_data[0] = WA156819_SOFT_MUTE_DATA;
    send_data[1] = data[0];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x\n", slave_addr,WA156819_SOFT_MUTE_DATA,data[0]);
    }

    return ret;
}


int WA156819_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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


int WA156819_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;

    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};

    //Read
    addr[0] = reg[0];
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, addr, WA156819_ADDR_SIZE) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,WA156819_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
        return (-1);
    }

     AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,addr[0],data[0]);

    return ret;
}


