/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    st_lnbh29.h

          This file provides the functions to control LNBH29(ST).
*/
/*----------------------------------------------------------------------------*/
#ifndef ST_LNBH29_H
#define ST_LNBH29_H

#include "sony_common.h"
#include "sony_lnbc.h"

/**
 @brief Use 13.333V for low level output voltage.

 If you would like to use 13.0V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define ST_LNBH29_CONFIG_VOLTAGE_LOW_13_0V        0

/**
 @brief Use 13.667V for low level output voltage.

 If you would like to use 13.667V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define ST_LNBH29_CONFIG_VOLTAGE_LOW_13_667V        1

/**
 @brief Use 18.150V for high level output voltage.

 If you would like to use 18.150V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_150V      10

/**
 @brief Use 18.817V for high level output voltage.

 If you would like to use 18.817V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define ST_LNBH29_CONFIG_VOLTAGE_HIGH_18_817V      11

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create a instance for LNBH29(ST).

 @param pLnbc LNB controller driver instance.
 @param i2cAddress I2C address.
 @param pI2c I2C instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t st_lnbh29_Create (sony_lnbc_t * pLnbc,
                                uint8_t i2cAddress,
                                sony_i2c_t * pI2c);

#endif /* ST_LNBH29_H */
