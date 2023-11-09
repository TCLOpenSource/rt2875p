#ifndef __RTK_AMP_WA156819_H__
#define __RTK_AMP_WA156819_H__



#define WA156819_ADDR_SIZE   (1)
#define WA156819_DATA_SIZE   (1)

#define WA156819_Soundon      0x0C
#define WA156819_SOFT_MUTE_DATA        0x33

#define WA156819_MUTE_ON(data)      (WA156819_SOFT_MUTE_DATA | (data))
#define WA156819_MUTE_OFF(data)     (~WA156819_SOFT_MUTE_DATA & (data))

void WA156819_func(int bus_id, unsigned short addr);
int WA156819_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
int WA156819_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
int WA156819_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr);
void WA156819_amp_reset(void);
#endif