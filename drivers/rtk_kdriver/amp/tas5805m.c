#include "rtk_amp_interface.h"
#ifndef BUILD_QUICK_SHOW
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#else
#include <no_os/slab.h>
#include <timer.h>
#endif

//*************************************************************
// PUBLIC FUNCTIONS
//*************************************************************
typedef struct
{
    unsigned char bAddr;
    unsigned char bData;
}TAS5805M_REG;

typedef struct
{
    unsigned char bAddr;
    unsigned char *reg_name;
}TAS5805M_DUMP;

static TAS5805M_REG TAS5805m_InitTbl[] =
{
// -----------------------------------------------------------------------------
// Initialization Sequence
// -----------------------------------------------------------------------------
//RESET
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_DEVICE_CTRL_2, 0x02 }, //Hi-Z mode
    { TAS5805M_RESET_CTRL,    0x11 }, //Reset modules & Reset control port registers
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_DEVICE_CTRL_2, 0x02 },
    { TAS5805M_DELAY ,           5 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_DEVICE_CTRL_2, 0x00 }, // Sleep mode
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_REG_46,        0x11 },

    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_DEVICE_CTRL_2, 0x02 }, //Hi-Z mode
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_FAULT_CLEAR,   0x80 }, //ANALOG_FAULT_CLEAR

    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_ADR_PIN_CONFIG,0x0b }, //ADR as FAULTZ output
    { TAS5805M_ADR_PIN_CTRL,  0x01 }, //ADR is output
    { TAS5805M_REG_7D,        0x11 },
    { TAS5805M_REG_7E,        0xff },
    { TAS5805M_REG_00,        0x01 },
    { TAS5805M_AUTO_MUTE_TIME,0x05 },

    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_DEVICE_CTRL_1, 0x00 },
    { TAS5805M_ANA_CTRL,      0x00 }, //Class-D bandwidth control.80kHz;
    { TAS5805M_AGAIN,         0x00 }, //Analog Gain Control 0db
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_00,        0x00 },
//Tuning coeffs (generate DSP coeffs by PCC3 SW Tool)
//Register Tuning
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_SDOUT_SEL,     0x00 }, //SDOUT is the DSP output (post-processing)
    { TAS5805M_DIG_VOL_CTRL,  0x30 },
    { TAS5805M_DEVICE_CTRL_2, 0x03 }, //Play mode
    { TAS5805M_REG_00,        0x00 },
    { TAS5805M_REG_7F,        0x00 },
    { TAS5805M_FAULT_CLEAR,   0x80 }, //ANALOG_FAULT_CLEAR
};

static TAS5805M_DUMP TAS5805m_Dump[] =
{
    {TAS5805M_RESET_CTRL           ,"Reset_CTRL"},
    {TAS5805M_DEVICE_CTRL_1        ,"DEVICE_CTRL_1"},
    {TAS5805M_DEVICE_CTRL_2        ,"DEVICE_CTRL_2"},
    {TAS5805M_I2C_PAGE_AUTO_INC    ,"I2C_PAGE_AUTO_INC"},
    {TAS5805M_SIG_CH_CTRL          ,"SIG_CH_CTRL"},
    {TAS5805M_CLOCK_DET_CTRL       ,"CLOCK_DET_CTRL"},
    {TAS5805M_SDOUT_SEL            ,"SDOUT_SEL"},
    {TAS5805M_I2S_CTRL             ,"I2S_CTRL"},
    {TAS5805M_SAP_CTRL1            ,"SAP_CTRL1"},
    {TAS5805M_SAP_CTRL2            ,"SAP_CTRL2"},
    {TAS5805M_SAP_CTRL3            ,"SAP_CTRL3"},
    {TAS5805M_FS_MON               ,"FS_MON"},
    {TAS5805M_BCK_MON              ,"BCK_MON"},
    {TAS5805M_CLKDET_STATUS        ,"CLKDET_STATUS"},
    {TAS5805M_DIG_VOL_CTRL         ,"DIG_VOL_CTRL"},
    {TAS5805M_DIG_VOL_CTRL2        ,"DIG_VOL_CTRL2"},
    {TAS5805M_DIG_VOL_CTRL3        ,"DIG_VOL_CTRL3"},
    {TAS5805M_AUTO_MUTE_CTRL       ,"AUTO_MUTE_CTRL"},
    {TAS5805M_AUTO_MUTE_TIME       ,"AUTO_MUTE_TIME"},
    {TAS5805M_ANA_CTRL             ,"ANA_CTRL"},
    {TAS5805M_AGAIN                ,"AGAIN"},
    {TAS5805M_BQ_WR_CTRL1          ,"BQ_WR_CTRL1"},
    {TAS5805M_DAC_CTRL             ,"DAC_CTRL"},
    {TAS5805M_ADR_PIN_CTRL         ,"ADR_PIN_CTRL"},
    {TAS5805M_ADR_PIN_CONFIG       ,"ADR_PIN_CONFIG"},
    {TAS5805M_DSP_MISC             ,"DSP_MISC"},
    {TAS5805M_DIE_ID               ,"DIE_ID"},
    {TAS5805M_POWER_STATE          ,"POWER_STATE"},
    {TAS5805M_AUTOMUTE_STATE       ,"AUTOMUTE_STATE"},
    {TAS5805M_PHASE_CTRL           ,"PHASE_CTRL"},
    {TAS5805M_SS_CTRL0             ,"SS_CTRL0"},
    {TAS5805M_SS_CTRL1             ,"SS_CTRL1"},
    {TAS5805M_SS_CTRL2             ,"SS_CTRL2"},
    {TAS5805M_SS_CTRL3             ,"SS_CTRL3"},
    {TAS5805M_SS_CTRL4             ,"SS_CTRL4"},
    {TAS5805M_CHAN_FAULT           ,"CHAN_FAULT"},
    {TAS5805M_GLOBAL_FAULT1        ,"GLOBAL_FAULT1"},
    {TAS5805M_GLOBAL_FAULT2        ,"GLOBAL_FAULT2"},
    {TAS5805M_OT_WARNING           ,"OT_WARNING"},
    {TAS5805M_PIN_CONTROL1         ,"PIN_CONTROL1"},
    {TAS5805M_PIN_CONTROL2         ,"PIN_CONTROL2"},
    {TAS5805M_MISC_CONTROL         ,"MISC_CONTROL"},
    {TAS5805M_FAULT_CLEAR          ,"FAULT_CLEAR"},
};

void tas5805m_func(int amp_i2c_id, unsigned short slave_addr)
{
    unsigned char data[2] = {0};
    unsigned int index = 0;
#if TAS5805M_DEBUG
    unsigned char i = 0;
#endif

    for(index = 0; index < (sizeof(TAS5805m_InitTbl)/sizeof(TAS5805M_REG)); index ++)
    {
        if( TAS5805m_InitTbl[index].bAddr == TAS5805M_DELAY ) {
            mdelay(TAS5805m_InitTbl[index].bData);
            continue;
        }

        data[0] = TAS5805m_InitTbl[index].bAddr;
        data[1] = TAS5805m_InitTbl[index].bData;

#if TAS5805M_DEBUG
        AMP_WARN("index:%d ===write addr:0x%x len:%d===\n",index, data[0],TAS5805M_DATA_SIZE);
        for(i = 0 ; i < TAS5805m_InitTbl[index].bDataLen; ++i)
            AMP_WARN("%x ", data[i]);

#endif
        if (i2c_master_send_ex_flag(amp_i2c_id, slave_addr , data, TAS5805M_DATA_SIZE+1 , I2C_M_FAST_SPEED) < 0)
            AMP_ERR("program TAS5805m failed\n");

    }
}

void tas5805m_amp_reset(void)
{
    AMP_WARN("tas5805m_amp_reset\n");
    rtk_amp_pin_set("PIN_AMP_RESET", 0);
    mdelay(10);
    rtk_amp_pin_set("PIN_AMP_RESET", 1);
    mdelay(1);
}

int tas5805m_dump_all(const char *buf, int amp_i2c_id, unsigned short slave_addr)
{

    int ret = 0;
    int result = 0;
    int index = 0;
    int data_index = 0;
    unsigned char addr[1] ={0};
    unsigned char *data = NULL;
    //Read SUBADDRESS Register
    AMP_WARN("== Dump SUBADDRESS 00~7F Reg Size:%d==\n",sizeof(TAS5805m_Dump)/sizeof(TAS5805M_DUMP));
    for (index = 0; index < (sizeof(TAS5805m_Dump)/sizeof(TAS5805M_DUMP)) ;++index){
        addr[0] = TAS5805m_Dump[index].bAddr;
        if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
            AMP_ERR("I2C Write Reg:0x%x failed\n",addr[0]);
            break;
        }
        data = kmalloc(TAS5805M_DATA_SIZE, GFP_KERNEL);
        result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,TAS5805M_DATA_SIZE);
        AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5805m_Dump[index].reg_name,addr[0],TAS5805M_DATA_SIZE);
        for(data_index = 0 ; data_index < TAS5805M_DATA_SIZE; ++data_index){
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

int tas5805m_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr)
{
    int ret = 0;
    unsigned char reg03_value[2] ={TAS5805M_DEVICE_CTRL_2, 0};
    unsigned char reg35_value[2] ={TAS5805M_SAP_CTRL3    , 0};

    if (on_off == AMP_MUTE_ON)
    {
        //mute both left & right channels
        reg03_value[1] = 0x0b;
        reg35_value[1] = 0x00;
    }
    else
    {
        //unmute
        reg03_value[1] = 0x03;
        reg35_value[1] = 0x11;
    }

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &reg03_value[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",reg03_value[0],reg03_value[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,reg03_value[0],reg03_value[1]);
    }

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &reg35_value[0], 2) < 0 ){
        AMP_ERR("I2C Write Reg:0x%x Data:0x%x failed\n",reg35_value[0],reg35_value[1]);
        return (-1);
    }
    else{
        AMP_WARN("Addr:0x%x Reg:0x%x Data:0x%x \n", slave_addr,reg35_value[0],reg35_value[1]);
    }

    return ret;
}

int tas5805m_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr)
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

int tas5805m_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr)
{
    int ret = 0;
    int index = 0;
    int result = 0;
    int data_index = 0;
    unsigned char addr[1] ={0};
    unsigned char *data = NULL;

    //Read
    addr[0] = reg[0];
    for (index = 0; index < (sizeof(TAS5805m_Dump)/sizeof(TAS5805M_DUMP)) ;++index){
        if(addr[0] == TAS5805m_Dump[index].bAddr){
            break;
        }
    }
    if (index == (sizeof(TAS5805m_Dump)/sizeof(TAS5805M_DUMP)))
        goto failed;

    data = kmalloc(TAS5805M_DATA_SIZE, GFP_KERNEL);

    if ( i2c_master_send_ex(amp_i2c_id, slave_addr, &addr[0], 1) < 0 ){
        AMP_ERR("I2C Write Address Reg:0x%x failed\n",addr[0]);
        ret = (-1);
        goto failed;
    }

    result = i2c_master_recv_ex(amp_i2c_id,slave_addr,NULL,0,data,TAS5805M_DATA_SIZE);

    if(result<0) {
        AMP_ERR("Read AMP REG:0x%x failed !! \n",*reg);
        ret = (-1);
        goto failed;
    }

    AMP_WARN("===%s Reg addr:0x%x len:%d===\n",TAS5805m_Dump[index].reg_name,addr[0],TAS5805M_DATA_SIZE);
    AMP_WARN("data[%d]:%x ", data_index, data[0]);

failed:
    kfree(data);
    return ret;
}

