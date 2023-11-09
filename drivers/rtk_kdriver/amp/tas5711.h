#ifndef __TAS5711_H__
#define __TAS5711_H__

#define TAS5711_CLOCK_CTRL              0x00
#define TAS5711_DEVICE_ID               0x01
#define TAS5711_ERROR_STATUS            0x02
#define TAS5711_SYSTEM_CTRL1            0x03
#define TAS5711_DATA_INTERFACE          0x04
#define TAS5711_SYSTEM_CTRL2            0x05
#define TAS5711_SOFT_MUTE               0x06
#define TAS5711_MASTER_VOLUME           0x07
#define TAS5711_CHANNEL_1_VOL           0x08
#define TAS5711_CHANNEL_2_VOL           0x09
#define TAS5711_CHANNEL_3_VOL           0x0A
#define TAS5711_VOLUME_CONFIG           0x0E
#define TAS5711_MODULATION_LIMIT        0x10
#define TAS5711_IC_DELAY_CHANNEL1       0x11
#define TAS5711_IC_DELAY_CHANNEL2       0x12
#define TAS5711_IC_DELAY_CHANNEL3       0x13
#define TAS5711_IC_DELAY_CHANNEL4       0x14
#define TAS5711_PWM_SHUTDOWN            0x19
#define TAS5711_START_STOP_PERIOD       0x1A
#define TAS5711_OSCILLATOR_TRIM         0x1B
#define TAS5711_BKND_ERR                0x1C
#define TAS5711_INPUT_MUX               0x20
#define TAS5711_CH4_SOURCE_SELECT       0x21
#define TAS5711_PWM_MUX                 0x25
#define TAS5711_CH1_BQ0                 0x29
#define TAS5711_CH1_BQ1                 0x2A
#define TAS5711_CH1_BQ2                 0x2B
#define TAS5711_CH1_BQ3                 0x2C
#define TAS5711_CH1_BQ4                 0x2D
#define TAS5711_CH1_BQ5                 0x2E
#define TAS5711_CH1_BQ6                 0x2F
#define TAS5711_CH2_BQ0                 0x30
#define TAS5711_CH2_BQ1                 0x31
#define TAS5711_CH2_BQ2                 0x32
#define TAS5711_CH2_BQ3                 0x33
#define TAS5711_CH2_BQ4                 0x34
#define TAS5711_CH2_BQ5                 0x35
#define TAS5711_CH2_BQ6                 0x36
#define TAS5711_DRC1_AE                 0x3A
#define TAS5711_DRC1_AA                 0x3B
#define TAS5711_DRC1_AD                 0x3C
#define TAS5711_DRC2_AE                 0x3D
#define TAS5711_DRC2_AA                 0x3E
#define TAS5711_DRC2_AD                 0x3F
#define TAS5711_DRC1_T                  0x40
#define TAS5711_DRC1_K                  0x41
#define TAS5711_DRC1_O                  0x42
#define TAS5711_DRC2_T                  0x43
#define TAS5711_DRC2_K                  0x44
#define TAS5711_DRC2_O                  0x45
#define TAS5711_DRC_CONTROL             0x46
#define TAS5711_BANK_SWITCH_CTRL        0x50
#define TAS5711_CH1_OUTPUT_MIXER        0x51
#define TAS5711_CH2_OUTPUT_MIXER        0x52
#define TAS5711_CH1_INPUT_MIXER         0x53
#define TAS5711_CH2_INPUT_MIXER         0x54
#define TAS5711_CH3_INPUT_MIXER         0x55
#define TAS5711_OUTPUT_POST_SCALE       0x56
#define TAS5711_OUTPUT_PRE_SCALE        0x57
#define TAS5711_CH1_BQ7                 0x58
#define TAS5711_CH1_BQ8                 0x59
#define TAS5711_SUB_BQ0                 0x5A
#define TAS5711_SUB_BQ1                 0x5B
#define TAS5711_CH2_BQ7                 0x5C
#define TAS5711_CH2_BQ8                 0x5D
#define TAS5711_PSEUDO_CH2_BQ0          0x5E
#define TAS5711_CH4_OUTPUT_MIXER        0x60
#define TAS5711_CH4_INPUT_MIXER         0x61
#define TAS5711_IDF_POST_SCALE          0x62
#define TAS5711_DEV_ADDR_ENABLE         0xF8
#define TAS5711_DEV_ADDR_UPDATE         0xF9

void tas5711_func(int bus_id, unsigned short addr);

#endif


