#ifndef __TAS5751_H__
#define __TAS5751_H__

#define CFG_META_SWITCH (255)
#define CFG_META_DELAY  (254)
#define CFG_META_BURST  (253)

#define TAS5751_CLOCK_CTRL              0x00
#define TAS5751_DEVICE_ID               0x01
#define TAS5751_ERROR_STATUS            0x02
#define TAS5751_SYSTEM_CTRL1            0x03
#define TAS5751_DATA_INTERFACE          0x04
#define TAS5751_SYSTEM_CTRL2            0x05
#define TAS5751_SOFT_MUTE               0x06
#define TAS5751_MASTER_VOLUME           0x07
#define TAS5751_CHANNEL_1_VOL           0x08
#define TAS5751_CHANNEL_2_VOL           0x09
#define TAS5751_CHANNEL_3_VOL           0x0A
#define TAS5751_VOLUME_CONFIG           0x0E
#define TAS5751_MODULATION_LIMIT        0x10
#define TAS5751_IC_DELAY_CHANNEL1       0x11
#define TAS5751_IC_DELAY_CHANNEL2       0x12
#define TAS5751_IC_DELAY_CHANNEL3       0x13
#define TAS5751_IC_DELAY_CHANNEL4       0x14
#define TAS5751_PWM_START               0x18
#define TAS5751_PWM_SHUTDOWN_GROUP      0x19
#define TAS5751_START_STOP_PERIOD       0x1A
#define TAS5751_OSCILLATOR_TRIM         0x1B
#define TAS5751_BKND_ERR                0x1C
#define TAS5751_INPUT_MUX               0x20
#define TAS5751_PWM_MUX                 0x25
#define TAS5751_CH1_BQ0                 0x26
#define TAS5751_CH1_BQ1                 0x27
#define TAS5751_CH1_BQ2                 0x28
#define TAS5751_CH1_BQ3                 0x29
#define TAS5751_CH1_BQ4                 0x2A
#define TAS5751_CH1_BQ5                 0x2B
#define TAS5751_CH1_BQ6                 0x2C
#define TAS5751_CH1_BQ7                 0x2D
#define TAS5751_CH1_BQ8                 0x2E
#define TAS5751_CH1_BQ9                 0x2F
#define TAS5751_CH2_BQ0                 0x30
#define TAS5751_CH2_BQ1                 0x31
#define TAS5751_CH2_BQ2                 0x32
#define TAS5751_CH2_BQ3                 0x33
#define TAS5751_CH2_BQ4                 0x34
#define TAS5751_CH2_BQ5                 0x35
#define TAS5751_CH2_BQ6                 0x36
#define TAS5751_CH2_BQ7                 0x37
#define TAS5751_CH2_BQ8                 0x38
#define TAS5751_CH2_BQ9                 0x39
#define TAS5751_AGL1_SOFTENING_FILTER   0x3B
#define TAS5751_AGL1_ATTACK_RATE        0x3C
#define TAS5751_AGL2_SOFTENING_FILTER   0x3E
#define TAS5751_AGL2_ATTACK_RATE        0x3F
#define TAS5751_AGL1_ATTACK_THRESHOLD   0x40
#define TAS5751_AGL3_ATTACK_THRESHOLD   0x41
#define TAS5751_AGL3_ATTACK_RATE        0x42
#define TAS5751_AGL2_ATTACK_THRESHOLD   0x43
#define TAS5751_AGL4_ATTACK_THRESHOLD   0x44
#define TAS5751_AGL4_ATTACK_RATE        0x45
#define TAS5751_AGL_CONTROL             0x46
#define TAS5751_AGL3_SOFTENING_FILTER   0x47
#define TAS5751_AGL4_SOFTENING_FILTER   0x48
#define TAS5751_PWM_SWITCH_RATE_CTRL    0x4F
#define TAS5751_BANK_SWITCH_CTRL        0x50
#define TAS5751_CH1_OUTPUT_MIXER        0x51
#define TAS5751_CH2_OUTPUT_MIXER        0x52
#define TAS5751_OUTPUT_POST_SCALE       0x56
#define TAS5751_OUTPUT_PRE_SCALE        0x57
#define TAS5751_CH1_BQ10                0x58
#define TAS5751_CH1_CROSS_BQ0           0x59
#define TAS5751_CH1_CROSS_BQ1           0x5A
#define TAS5751_CH1_CROSS_BQ2           0x5B
#define TAS5751_CH1_CROSS_BQ3           0x5C
#define TAS5751_CH2_BQ10                0x5D
#define TAS5751_CH2_CROSS_BQ0           0x5E
#define TAS5751_CH2_CROSS_BQ1           0x5F
#define TAS5751_CH2_CROSS_BQ2           0x60
#define TAS5751_CH2_CROSS_BQ3           0x61
#define TAS5751_IDF_POST_SCALE          0x62
#define TAS5751_LEFT_CHANNEL_PWM_LEVEL  0x6B
#define TAS5751_RIGHT_CHANNEL_PWM_LEVEL 0x6C
#define TAS5751_CH1_INLINE_MIXER        0x70
#define TAS5751_CH1_INLINE_AGL_EN_MIXER 0x71
#define TAS5751_CH1_RIGHT_CHANNEL_MIXER 0x72
#define TAS5751_CH1_LEFT_CHANNEL_MIXER  0x73
#define TAS5751_CH2_INLINE_MIXER        0x74
#define TAS5751_CH2_INLINE_AGL_EN_MIXER 0x75
#define TAS5751_CH2_LEFT_CHANNEL_MIXER  0x76
#define TAS5751_CH2_RIGHT_CHANNEL_MIXER 0x77
#define TAS5751_UPDATE_ADDRESS_KEY      0xF8
#define TAS5751_UPDATE_ADDRESS          0xF9

#define TAS5751_DEBUG                   0
#define TAS5751_ADDR_SIZE               (1)
#define TAS5751_DATA_SIZE               (1)
#define TAS5751_SOFT_MUTE_DATA          0x07
#define TAS5751_MUTE_ON(data)           (TAS5751_SOFT_MUTE_DATA | (data))
#define TAS5751_MUTE_OFF(data)          (~TAS5751_SOFT_MUTE_DATA & (data))

void tas5751_func(int amp_i2c_id, unsigned short addr);
int tas5751_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int tas5751_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int tas5751_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int tas5751_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
#endif

