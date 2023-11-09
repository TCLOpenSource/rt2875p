/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    allegro_a8304.h

          This file provides the functions to control A8304(Allegro).
*/
/*----------------------------------------------------------------------------*/
#ifndef ALLEGRO_A8304_H
#define ALLEGRO_A8304_H

#include "sony_common.h"
#include "sony_lnbc.h"

/**
 @brief Use 13.333V for low level output voltage.

 If you would like to use 13.333V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define ALLEGRO_A8304_CONFIG_VOLTAGE_LOW_13_333V        0

/**
 @brief Use 14.333V for low level output voltage.

 If you would like to use 14.333V as "Low level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_LOW_VOLTAGE, value).
*/
#define ALLEGRO_A8304_CONFIG_VOLTAGE_LOW_14_333V        1

/**
 @brief Use 18.667V for high level output voltage.

 If you would like to use 18.667V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define ALLEGRO_A8304_CONFIG_VOLTAGE_HIGH_18_667V      10

/**
 @brief Use 19.667V for high level output voltage.

 If you would like to use 19.667V as "High level voltage",
 please set this parameter by sony_lnbc_t::SetConfig(SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE, value).
*/
#define ALLEGRO_A8304_CONFIG_VOLTAGE_HIGH_19_667V      11

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create a instance for A8304(Allegro).

 @param pLnbc LNB controller driver instance.
 @param i2cAddress I2C address.
 @param pI2c I2C instance.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t allegro_a8304_Create (sony_lnbc_t * pLnbc,
                                    uint8_t i2cAddress,
                                    sony_i2c_t * pI2c);

#endif /* ALLEGRO_A8304_H */
