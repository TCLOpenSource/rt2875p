/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
/**
 @file    sony_j83b.h

          This file provides J.83B related type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_J83B_H
#define SONY_J83B_H

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief J.83B constellation.
*/
typedef enum {
    SONY_J83B_CONSTELLATION_RESERVED_0,     /**< RESERVED */
    SONY_J83B_CONSTELLATION_RESERVED_1,     /**< RESERVED */
    SONY_J83B_CONSTELLATION_64QAM,          /**< 64-QAM */
    SONY_J83B_CONSTELLATION_RESERVED_3,     /**< RESERVED */
    SONY_J83B_CONSTELLATION_256QAM          /**< 256-QAM */
} sony_j83b_constellation_t;


/**
 @brief J.83B Interleave Mode.
*/
typedef enum {
    SONY_J83B_INTERLEAVE_ENHANCED_128_1,     /**< Enhanced Interleaving mode I = 128, J = 1  */
    SONY_J83B_INTERLEAVE_REDUCED_128_1,      /**< Reduced  Interleaving mode I = 128, J = 1  */
    SONY_J83B_INTERLEAVE_ENHANCED_128_2,     /**< Enhanced Interleaving mode I = 128, J = 2  */
    SONY_J83B_INTERLEAVE_REDUCED_64_2,       /**< Reduced  Interleaving mode I = 64,  J = 2  */
    SONY_J83B_INTERLEAVE_ENHANCED_128_3,     /**< Enhanced Interleaving mode I = 128, J = 3  */
    SONY_J83B_INTERLEAVE_REDUCED_32_4,       /**< Reduced  Interleaving mode I = 32,  J = 4  */
    SONY_J83B_INTERLEAVE_ENHANCED_128_4,     /**< Enhanced Interleaving mode I = 128, J = 4  */
    SONY_J83B_INTERLEAVE_REDUCED_16_8,       /**< Reduced  Interleaving mode I = 16,  J = 8  */
    SONY_J83B_INTERLEAVE_ENHANCED_128_5,     /**< Enhanced Interleaving mode I = 128, J = 5  */
    SONY_J83B_INTERLEAVE_REDUCED_8_16,       /**< Reduced  Interleaving mode I = 8,   J = 16 */
    SONY_J83B_INTERLEAVE_ENHANCED_128_6,     /**< Enhanced Interleaving mode I = 128, J = 6  */
    SONY_J83B_INTERLEAVE_RESERVED_11,        /**< Reserved */
    SONY_J83B_INTERLEAVE_ENHANCED_128_7,     /**< Enhanced Interleaving mode I = 128, J = 7  */
    SONY_J83B_INTERLEAVE_RESERVED_13,        /**< Reserved */
    SONY_J83B_INTERLEAVE_ENHANCED_128_8,     /**< Enhanced Interleaving mode I = 128, J = 8 */
    SONY_J83B_INTERLEAVE_RESERVED_15         /**< Reserved */
} sony_j83b_interleave_t;

#endif /* SONY_J83B_H */
