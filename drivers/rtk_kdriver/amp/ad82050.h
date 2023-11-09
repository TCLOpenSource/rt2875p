/*ad82050.h  --  ad82050 ALSA SoC Audio driver
 *
 * Copyright 1998 Elite Semiconductor Memory Technology
 *
 * Author: ESMT Audio/Power Product BU Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public  version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __AD82050_H__
#define __AD82050_H__

#define AD82050_END_FLAG   (0xff)
#define AD82050_DEBUG      (0)
#define AD82050_ADDR_SIZE              (1)
#define AD82050_DATA_SIZE              (1)

#endif /* __AD82050_H__ */

void ad82050_func(int amp_i2c_id,unsigned short addr);
void ad82050_amp_reset(void);
int ad82050_mute_set(int on_off, int amp_i2c_id,unsigned short slave_addr);
int ad82050_param_set(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
int ad82050_param_get(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
