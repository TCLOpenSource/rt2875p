#ifndef __RTK_AMP_AD82010_H__
#define __RTK_AMP_AD82010_H__

#define AD82010_SCTL1              0x00
#define AD82010_SCTL2              0x01
#define AD82010_SCTL3              0x02
#define AD82010_MVOL               0x03
#define AD82010_C1VOL              0x04
#define AD82010_C2VOL              0x05
#define AD82010_HVUV               0x06
#define AD82010_SCTL4              0x07
#define AD82010_LAR                0x08
#define AD82010_TEST               0x09
#define AD82010_RESERVED1          0x0A
#define AD82010_RESERVED2          0x0B
#define AD82010_STATUS             0x0C
#define AD82010_ACFG               0x0D
#define AD82010_TM_CTRL            0x0E
#define AD82010_PWM_CTRL           0x0F
#define AD82010_ATT                0x10
#define AD82010_ATM                0x11
#define AD82010_ATB                0x12
#define AD82010_PCT                0x13
#define AD82010_PCM                0x14
#define AD82010_PCB                0x15
#define AD82010_SCTL5              0x16
#define AD82010_VFT                0x17
#define AD82010_DTC                0x18
#define AD82010_RESERVED3          0x19
#define AD82010_NGALT              0x1A
#define AD82010_NGALM              0x1B
#define AD82010_NGALB              0x1C
#define AD82010_NGRTL              0x1D
#define AD82010_NGRLM              0x1E
#define AD82010_NGRLB              0x1F
#define AD82010_DRC_ECT            0x20
#define AD82010_DRC_ECB            0x21
#define AD82010_DRC_RTT            0x22
#define AD82010_DRC_RTM            0x23
#define AD82010_DRC_RTB            0x24
#define AD82010_DEVICE_ID          0x25
#define AD82010_RAM1_CFADDR        0x26
#define AD82010_RAM1_A1CF1         0x27
#define AD82010_RAM1_A1CF2         0x28
#define AD82010_RAM1_A1CF3         0x29
#define AD82010_RAM1_CFRW          0x2A
#define AD82010_HI_RES             0x2B
#define AD82010_MBIST              0x2C
#define AD82010_ERROR              0x2D
#define AD82010_MK_H               0x2E
#define AD82010_MK_L               0x2F

#define AD82010_VOLUME_MUTE        0x08
#define AD82010_MUTE_ON(data)      (AD82010_VOLUME_MUTE | (data))
#define AD82010_MUTE_OFF(data)     (~AD82010_VOLUME_MUTE & (data))

#define AD82010_ADDR_SIZE   (1)
#define AD82010_DATA_SIZE   (1)

#define AD82010_END_FLAG   (0xff)
#define AD82010_DEBUG      (0)


void ad82010_func(int amp_i2c_id,unsigned short addr);
int ad82010_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int ad82010_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int ad82010_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int ad82010_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
#endif

