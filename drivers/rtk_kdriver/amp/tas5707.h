#ifndef __TAS5707_H__
#define __TAS5707_H__

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

#define TAS5707_CLOCK_CTRL              0x00
#define TAS5707_DEVICE_ID               0x01
#define TAS5707_ERROR_STATUS            0x02
#define TAS5707_SYSTEM_CTRL1            0x03
#define TAS5707_DATA_INTERFACE          0x04
#define TAS5707_SYSTEM_CTRL2            0x05
#define TAS5707_SOFT_MUTE               0x06
#define TAS5707_MASTER_VOLUME           0x07
#define TAS5707_CHANNEL_1_VOL           0x08
#define TAS5707_CHANNEL_2_VOL           0x09
#define TAS5707_FINE_MASTER_VOL         0x0A
#define TAS5707_VOLUME_CONFIG           0x0E
#define TAS5707_MODULATION_LIMIT        0x10
#define TAS5707_IC_DELAY_CHANNEL1       0x11
#define TAS5707_IC_DELAY_CHANNEL2       0x12
#define TAS5707_IC_DELAY_CHANNEL3       0x13
#define TAS5707_IC_DELAY_CHANNEL4       0x14
#define TAS5707_START_STOP_PERIOD       0x1A
#define TAS5707_OSCILLATOR_TRIM         0x1B
#define TAS5707_BKND_ERR                0x1C
#define TAS5707_INPUT_MUX               0x20
#define TAS5707_PWM_MUX                 0x25
#define TAS5707_CH1_BQ0                 0x29
#define TAS5707_CH1_BQ1                 0x2A
#define TAS5707_CH1_BQ2                 0x2B
#define TAS5707_CH1_BQ3                 0x2C
#define TAS5707_CH1_BQ4                 0x2D
#define TAS5707_CH1_BQ5                 0x2E
#define TAS5707_CH1_BQ6                 0x2F
#define TAS5707_CH2_BQ0                 0x30
#define TAS5707_CH2_BQ1                 0x31
#define TAS5707_CH2_BQ2                 0x32
#define TAS5707_CH2_BQ3                 0x33
#define TAS5707_CH2_BQ4                 0x34
#define TAS5707_CH2_BQ5                 0x35
#define TAS5707_CH2_BQ6                 0x36
#define TAS5707_DRC_AE                  0x3A
#define TAS5707_DRC_AA                  0x3B
#define TAS5707_DRC_AD                  0x3C
#define TAS5707_DRC_T                   0x40
#define TAS5707_DRC_K                   0x41
#define TAS5707_DRC_O                   0x42
#define TAS5707_DRC_CONTROL             0x46
#define TAS5707_BANK_SWITCH_CTRL        0x50
#define TAS5707_UPDATE_ADDRESS          0xF9

#define TAS5707_DEBUG                   0
#define TAS5707_ADDR_SIZE               (1)
#define TAS5707_DATA_SIZE               (1)
#define TAS5707_SOFT_MUTE_DATA          0x03
#define TAS5707_MUTE_ON(data)           (TAS5707_SOFT_MUTE_DATA | (data))
#define TAS5707_MUTE_OFF(data)          (~TAS5707_SOFT_MUTE_DATA & (data))

void tas5707_amp_reset(void);
void tas5707_func(int bus_id, unsigned short addr);
int tas5707_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int tas5707_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int tas5707_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int tas5707_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
#endif


