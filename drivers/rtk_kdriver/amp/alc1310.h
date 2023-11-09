#ifndef __RTK_AMP_H__
#define __RTK_AMP_H__



//Register
#define	ALC_1310_MX00_RESET 	         0x00
#define ALC_1310_MX01_THERMAL_FB         0x01
#define ALC_1310_MX02_SEQ_CTRL           0x02
#define ALC_1310_MX04_BQ_SETUP           0x04
#define ALC_1310_MX05_PLL_SETTING        0x05
#define ALC_1310_MX09_DRC_SETTING_1      0x09
#define ALC_1310_MX0A_DRC_SETTING_2      0x0A
#define ALC_1310_MX0B_DRC_SETTING_3      0x0B
#define ALC_1310_MX0C_DRC_SETTING_4      0x0C
#define ALC_1310_MX0E_DRC_SETTING_5      0x0E
#define ALC_1310_PR9A_DRC_SETTING_6      0x9A
#define ALC_1310_PR9C_DRC_SETTING_7      0x9C
#define ALC_1310_PR9D_DRC_SETTING_8      0x9D
#define ALC_1310_PR9E_DRC_SETTING_9      0x9E
#define ALC_1310_MX12_VOLUME_SETTING_1   0x12
#define ALC_1310_MX13_VOLUME_SETTING_2   0x13
#define ALC_1310_MX16_DPWM_1             0x16
#define ALC_1310_MX17_DPWM_2             0x17
#define ALC_1310_MX18_DPWM_3             0x18
#define ALC_1310_MX1D_I2C_SETTING        0x1D
#define ALC_1310_PR6A_REG_INDEX          0x6A
#define ALC_1310_PR6C_REG_DATA           0x6C
#define ALC_1310_MX7C_VENDER_ID          0x7C
#define ALC_1310_MX7E_DEVICE_ID          0x7E

#define ALC_1310_MX13_VOLUME_MUTE        0xC0
#define ALC_1310_MUTE_ON(data)           (ALC_1310_MX13_VOLUME_MUTE | (data))
#define ALC_1310_MUTE_OFF(data)          (~ALC_1310_MX13_VOLUME_MUTE & (data))

#define AMP_DEVICE_ADDR  (0x1A)
#define ALC1310_END_FLAG (0xFF)
#define AMP_DEBUG       (0)
#define ALC1310_ADDR_SIZE   (1)
#define ALC1310_DATA_SIZE   (2)
#define AMP_MUTE_ON	(1)
#define AMP_MUTE_OFF    (0)

#define AMP_MX_REG_SIZE   (0x7F)
#define AMP_PR_REG1_END   (0x1F)
#define AMP_PR_REG2_START (0x90)
#define AMP_PR_REG2_END   (0xFF)

typedef struct
{
    unsigned char bAddr;
    unsigned char bArray[2];
}ALC1310_REG;//markchen add for 1310

void alc1310_func(int amp_i2c_id, unsigned short slave_addr);
int alc1310_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int alc1310_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int alc1310_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int alc1310_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
#endif
