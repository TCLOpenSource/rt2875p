#ifndef __TAS5805M_H__
#define __TAS5805M_H__

#define TAS5805M_DRV_NAME    "tas5805m"

//General Register
#define TAS5805M_RESET_CTRL              0x01
#define TAS5805M_DEVICE_CTRL_1           0x02
#define TAS5805M_DEVICE_CTRL_2           0x03
#define TAS5805M_I2C_PAGE_AUTO_INC       0x0F
#define TAS5805M_SIG_CH_CTRL             0x28
#define TAS5805M_CLOCK_DET_CTRL          0x29
#define TAS5805M_SDOUT_SEL               0x30
#define TAS5805M_I2S_CTRL                0x31
#define TAS5805M_SAP_CTRL1               0x33
#define TAS5805M_SAP_CTRL2               0x34
#define TAS5805M_SAP_CTRL3               0x35
#define TAS5805M_FS_MON                  0x37
#define TAS5805M_BCK_MON                 0x38
#define TAS5805M_CLKDET_STATUS           0x39
#define TAS5805M_DIG_VOL_CTRL            0x4C
#define TAS5805M_DIG_VOL_CTRL2           0x4E
#define TAS5805M_DIG_VOL_CTRL3           0x4F
#define TAS5805M_AUTO_MUTE_CTRL          0x50
#define TAS5805M_AUTO_MUTE_TIME          0x51
#define TAS5805M_ANA_CTRL                0x53
#define TAS5805M_AGAIN                   0x54
#define TAS5805M_BQ_WR_CTRL1             0x5C
#define TAS5805M_DAC_CTRL                0x5D
#define TAS5805M_ADR_PIN_CTRL            0x60
#define TAS5805M_ADR_PIN_CONFIG          0x61
#define TAS5805M_DSP_MISC                0x66
#define TAS5805M_DIE_ID                  0x67
#define TAS5805M_POWER_STATE             0x68
#define TAS5805M_AUTOMUTE_STATE          0x69
#define TAS5805M_PHASE_CTRL              0x6A
#define TAS5805M_SS_CTRL0                0x6B
#define TAS5805M_SS_CTRL1                0x6C
#define TAS5805M_SS_CTRL2                0x6D
#define TAS5805M_SS_CTRL3                0x6E
#define TAS5805M_SS_CTRL4                0x6F
#define TAS5805M_CHAN_FAULT              0x70
#define TAS5805M_GLOBAL_FAULT1           0x71
#define TAS5805M_GLOBAL_FAULT2           0x72
#define TAS5805M_OT_WARNING              0x73
#define TAS5805M_PIN_CONTROL1            0x74
#define TAS5805M_PIN_CONTROL2            0x75
#define TAS5805M_MISC_CONTROL            0x76
#define TAS5805M_FAULT_CLEAR             0x78

//Special Register
#define TAS5805M_REG_00                 (0x00)
#define TAS5805M_REG_24                 (0x24)
#define TAS5805M_REG_25                 (0x25)
#define TAS5805M_REG_26                 (0x26)
#define TAS5805M_REG_27                 (0x27)
#define TAS5805M_REG_28                 (0x28)
#define TAS5805M_REG_29                 (0x29)
#define TAS5805M_REG_2A                 (0x2a)
#define TAS5805M_REG_2B                 (0x2b)
#define TAS5805M_REG_35                 (0x35)
#define TAS5805M_REG_46                 (0x46)
#define TAS5805M_REG_7D                 (0x7D)
#define TAS5805M_REG_7E                 (0x7E)
#define TAS5805M_REG_7F                 (0x7F)

#define TAS5805M_PAGE_00                (0x00)
#define TAS5805M_PAGE_2A                (0x2A)

#define TAS5805M_BOOK_00                (0x00)
#define TAS5805M_BOOK_8C                (0x8C)

#define TAS5805M_DELAY                  (255)
#define TAS5805M_DEBUG                  (0)
#define TAS5805M_ADDR_SIZE              (1)
#define TAS5805M_DATA_SIZE              (1)

void tas5805m_func(int amp_i2c_id, unsigned short addr);
void tas5805m_amp_reset(void);
int tas5805m_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int tas5805m_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int tas5805m_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int tas5805m_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
#endif

