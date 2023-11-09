#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#else
#include <timer.h>
#endif

typedef struct
{
    unsigned char bAddr;
    unsigned char bArray;
}AD82010_REG;

static AD82010_REG AD82010_InitTbl[] = 
{
    //init
    {AD82010_SCTL2,0x81},//Status control2(FS=48KHz/MCLK=64xFS)
    {AD82010_SCTL4,0x36},//Mixing-off/DRC-on/Power-clipping-on.
    {AD82010_MVOL,0x18},//Master Vol default=mute.
    {AD82010_C1VOL,0x18},//CH1 Vol default=2dB
    {AD82010_C2VOL,0x18},//CH2 Vol default=2dB
    {AD82010_SCTL3,0x11},//unmute,default power-on is mute.
    {AD82010_END_FLAG ,0x00},//END
};

void ad82010_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2];
    int u8Index=0;
    int timer = 0;
    for(u8Index = 0; ; u8Index ++)
    {
        data[0] = AD82010_InitTbl[u8Index].bAddr;
        data[1] = AD82010_InitTbl[u8Index].bArray;

        if(data[0] == AD82010_END_FLAG)
            break;

#if AD82010_DEBUG
        AMP_WARN("%s audio amp write Reg:0x%x----Data=0x%x\n",__func__,data[0],data[1]);
#endif

        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , &data[0], 2 ,I2C_M_FAST_SPEED) < 0)
        {
            AMP_ERR("%s line:%d program AD82010 failed\n",__func__, __LINE__);
            AMP_ERR("%s line:%d AD82010 Slave address:%x\n",__func__, __LINE__,slave_addr);
            AMP_ERR("\nwrite Reg:0x%x----Data=0x%x\n",data[0],data[1]);
            if(timer == 3){
                AMP_ERR("%s line:%d program AD82010 failed times:%d\n",__func__, __LINE__,timer);
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

int ad82010_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int i = 0;
    unsigned char addr[1] ={0};
    unsigned char data[2] ={0};
    //Read Register
    AMP_WARN("== Dump 00~2F Reg ==\n");
    for (i=0x00; i<= 0x2F ;++i){
        addr[0] = i;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,AD82010_DATA_SIZE);
        AMP_WARN("Reg:0x%x Data:0x%x \n", addr[0],data[0]);
        mdelay(5);
        if(result<0) {
            AMP_ERR("Read AMP REG:0x%x failed !! \n",addr[0]);
            break;
       }
    }

    return ret;
}

int ad82010_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    int result = 0;
    unsigned char addr[1] ={0};
    unsigned char data[1] ={0};
    unsigned char send_data[2] ={0};

    //Read
    addr[0] = AD82010_SCTL3;
    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        return (-1);
    }
    
    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,AD82010_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",AD82010_SCTL3);
        return (-1);      
    }

    if (on_off == AMP_MUTE_ON){
        data[0] = AD82010_MUTE_ON(data[0]);
    }
    else if (on_off == AMP_MUTE_OFF){
        data[0] = AD82010_MUTE_OFF(data[0]);
    }

    //Write
    send_data[0] = AD82010_SCTL3;
    send_data[1] = data[0];

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &send_data[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",send_data[0],send_data[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x\n", slave_addr,AD82010_SCTL3,data[0]);
    }

    return ret;
}

int ad82010_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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

int ad82010_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
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

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,AD82010_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",*reg);
        return (-1);
    }

    AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,addr[0],data[0]);

    return ret;
}
