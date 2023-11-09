/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_dvbs.h

          This file provides DVB-S related type definitions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DVBS_H
#define SONY_DVBS_H

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief DVB-S code rate.
*/
typedef enum {
    SONY_DVBS_CODERATE_1_2,             /**< 1/2 */
    SONY_DVBS_CODERATE_2_3,             /**< 2/3 */
    SONY_DVBS_CODERATE_3_4,             /**< 3/4 */
    SONY_DVBS_CODERATE_5_6,             /**< 5/6 */
    SONY_DVBS_CODERATE_7_8,             /**< 7/8 */
    SONY_DVBS_CODERATE_INVALID          /**< Invalid */
} sony_dvbs_coderate_t;

/**
 @brief DVB-S modulation.
*/
typedef enum {
    SONY_DVBS_MODULATION_QPSK,          /**< QPSK */
    SONY_DVBS_MODULATION_INVALID        /**< Invalid */
} sony_dvbs_modulation_t;

#endif /* SONY_DVBS_H */
