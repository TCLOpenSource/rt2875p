#ifndef __RTK_AMP_ALC1312_H__
#define __RTK_AMP_ALC1312_H__



//Register
#define	ALC_1312_MX0000_CLOCK0 	           {0x00,0x00}
#define	ALC_1312_MX0002_CLOCK1 	           {0x00,0x02}
#define	ALC_1312_MX0004_CLOCK2 	           {0x00,0x04}
#define	ALC_1312_MX0006_CLOCK3 	           {0x00,0x06}
#define	ALC_1312_MX0008_CLOCK4 	           {0x00,0x08}
#define	ALC_1312_MX000A_PLL1 	               {0x00,0x0A}
#define	ALC_1312_MX000C_PLL2 	               {0x00,0x0C}
#define	ALC_1312_MX0020_CLOCKDETECT         {0x00,0x20}
#define	ALC_1312_MX0022_SILENCE_DETECT     {0x00,0x22}

#define	ALC_1312_MX006A_PRIVATE_REG_INDEX  {0x00,0x6A}
#define	ALC_1312_MX006C_PRIVATE_REG_DATA   {0x00,0x6C}
#define	ALC_1312_MX0102_SOFT_VOLUME         {0x01,0x02}
#define	ALC_1312_MX0103_MASTER_VOLUME_L    {0x01,0x03}
#define	ALC_1312_MX0104_MASTER_VOLUME_R    {0x01,0x04}

#define	ALC_1312_MX0300_CLASS_D             {0x03,0x00}
#define	ALC_1312_MX0322_POWER1              {0x03,0x22}
#define	ALC_1312_MX0324_POWER2              {0x03,0x24}
#define	ALC_1312_MX0326_POWER3              {0x03,0x26}
#define	ALC_1312_MX0350_POWER_STATUS       {0x03,0x50}


#define ALC_1312_SVOL_MUTE_EN_L        (0x01 << 7)
#define ALC_1312_SVOL_MUTE_EN_R        (0x01 << 6)

#define ALC_1312_MUTE_ON(data)           ( (ALC_1312_SVOL_MUTE_EN_L|ALC_1312_SVOL_MUTE_EN_R) | (data))
#define ALC_1312_MUTE_OFF(data)          (~(ALC_1312_SVOL_MUTE_EN_L|ALC_1312_SVOL_MUTE_EN_R) & (data))
  
#define ALC1312_DEVICE_ADDR  (0x54)  // 0x56 , 0x34 , 0x36
#define ALC1312_END_FLAG     (0xFF)

#define ALC1312_DEBUG       (1)
#define ALC1312_ADDR_SIZE   (2)
#define ALC1312_DATA_SIZE   (2)



typedef struct
{
    unsigned char bAddr[2];
    unsigned char bArray[2];
    int delayTime;
}ALC1312_REG;

void alc1312_func(int amp_i2c_id, unsigned short slave_addr);
int  alc1312_dump_all(const char *buf, int amp_i2c_id,unsigned short slave_addr);
int  alc1312_mute_set(int on_off, int amp_i2c_id, unsigned short slave_addr);
int  alc1312_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int  alc1312_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);

#endif
