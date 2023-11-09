/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    intersil_isl9492.h

          This file provides the functions to control ISL9492(Intersil).
*/
/*----------------------------------------------------------------------------*/
#ifndef INTERSIL_ISL9492_H
#define INTERSIL_ISL9492_H

#include "sony_common.h"
#include "sony_lnbc.h"

/**
 @brief Use 13.5V for low level output voltage.

 If you would like to use 13.5V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_13_5V       0

/**
 @brief Use 14.2V for low level output voltage.

 If you would like to use 14.2V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_14_2V       1

/**
 @brief Use 18.5V for high level output voltage.

 If you would like to use 18.5V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_18_5V     10

/**
 @brief Use 20.0V for high level output voltage.

 If you would like to use 20.0V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_20_0V     11

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create a instance for ISL9492(Intersil).

 @param pLnbc LNB controller driver instance.
 @param i2cAddress I2C address.
 @param pI2c I2C instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t intersil_isl9492_Create (sony_lnbc_t * pLnbc,
                                       uint8_t i2cAddress,
                                       sony_i2c_t * pI2c);

#endif /* INTERSIL_ISL9492_H */
