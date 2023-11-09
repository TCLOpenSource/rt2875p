/*------------------------------------------------------------------------------
  Copyright 2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/**
 @file    ti_tps65235.h

          This file provides the functions to control TPS65235(TI).
*/
/*----------------------------------------------------------------------------*/
#ifndef TI_TPS65235_H
#define TI_TPS65235_H

#include "sony_common.h"
#include "sony_lnbc.h"

/**
 @brief Use 13.4V for low level output voltage.

 If you would like to use 13.4V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define TI_TPS65235_CONFIG_VOLTAGE_LOW_13_4V       0

/**
 @brief Use 14.6V for low level output voltage.

 If you would like to use 14.6V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define TI_TPS65235_CONFIG_VOLTAGE_LOW_14_6V       1

/**
 @brief Use 18.2V for high level output voltage.

 If you would like to use 18.2V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define TI_TPS65235_CONFIG_VOLTAGE_HIGH_18_2V     10

/**
 @brief Use 19.4V for high level output voltage.

 If you would like to use 19.4V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define TI_TPS65235_CONFIG_VOLTAGE_HIGH_19_4V     11

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create a instance for TPS65235(TI).

 @param pLnbc LNB controller driver instance.
 @param i2cAddress I2C address.
 @param pI2c I2C instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t ti_tps65235_Create (sony_lnbc_t * pLnbc,
                                  uint8_t i2cAddress,
                                  sony_i2c_t * pI2c);

#endif /* TI_TPS65235_H */
