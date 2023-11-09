#ifndef __I2C_H5_CUSTOMIZED_H__
#define __I2C_H5_CUSTOMIZED_H__

#include <rtk_kdriver/i2c-rtk-api.h>

#define H5X_I2C_PORT                3

#define H5X_I2C_SPEED_FLAG      I2C_M_HIGH_SPEED
#define H5X_I2C_ADDR 0x32

typedef enum {

   // For I2C only.
    I2C_HANDSHAKE                   = 0x30,
    I2C_ONLY_SCALER                 = 0x31,
    I2C_ONLY_PQ                     = 0x32,
    I2C_ONLY_SYSTEM                 = 0x33,
    I2C_ONLY_BLOCK_START            = 0x34,
    I2C_ONLY_BLOCK_TRANS            = 0X35,
    I2C_ONLY_BLOCK_END              = 0X36,
    I2C_ONLY_LOGBUFFER              = 0x37,
}H5_I2C_CMDS;


#endif


