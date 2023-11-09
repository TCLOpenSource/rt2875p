/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_isdbs.h

          This file provides ISDB-S related type definitions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_ISDBS_H
#define SONY_ISDBS_H

#include "sony_common.h"

/*--------------------------------------------------------------------
  Enumerations
--------------------------------------------------------------------*/
/**
 @brief Type of TSID.
*/
typedef enum sony_isdbs_tsid_type_t {
    SONY_ISDBS_TSID_TYPE_TSID,                /**< TSID(16bit) */
    SONY_ISDBS_TSID_TYPE_RELATIVE_TS_NUMBER,  /**< Relative TS number */
} sony_isdbs_tsid_type_t;

/**
 @brief The modulation and code rate.
*/
typedef enum sony_isdbs_modcod_t {
    SONY_ISDBS_MODCOD_RESERVED_0,             /**< Reserved 0 */
    SONY_ISDBS_MODCOD_BPSK_1_2,               /**< BPSK Code Rate 1/2 */
    SONY_ISDBS_MODCOD_QPSK_1_2,               /**< QPSK Code Rate 1/2 */
    SONY_ISDBS_MODCOD_QPSK_2_3,               /**< QPSK Code Rate 2/3 */
    SONY_ISDBS_MODCOD_QPSK_3_4,               /**< QPSK Code Rate 3/4 */
    SONY_ISDBS_MODCOD_QPSK_5_6,               /**< QPSK Code Rate 5/6 */
    SONY_ISDBS_MODCOD_QPSK_7_8,               /**< QPSK Code Rate 7/8 */
    SONY_ISDBS_MODCOD_TC8PSK_2_3,             /**< TC8PSK Code Rate 2/3 */
    SONY_ISDBS_MODCOD_RESERVED_8,             /**< Reserved 8 */
    SONY_ISDBS_MODCOD_RESERVED_9,             /**< Reserved 9 */
    SONY_ISDBS_MODCOD_RESERVED_10,            /**< Reserved 10 */
    SONY_ISDBS_MODCOD_RESERVED_11,            /**< Reserved 11 */
    SONY_ISDBS_MODCOD_RESERVED_12,            /**< Reserved 12 */
    SONY_ISDBS_MODCOD_RESERVED_13,            /**< Reserved 13 */
    SONY_ISDBS_MODCOD_RESERVED_14,            /**< Reserved 14 */
    SONY_ISDBS_MODCOD_UNUSED_15               /**< unused (1111) */
} sony_isdbs_modcod_t;

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
/**
  @brief Transmission mode and slot information.
*/
typedef struct sony_isdbs_tmcc_modcod_slot_info_t {
    sony_isdbs_modcod_t          modCod;     /**< Modulation scheme and Code rate */
    uint8_t                      slotNum;    /**< Slot number */
} sony_isdbs_tmcc_modcod_slot_info_t;

/**
  @brief TMCC information.
*/
typedef struct sony_isdbs_tmcc_info_t {
    uint8_t                            changeOrder;               /**< Incremented each time TMCC is renewed */
    sony_isdbs_tmcc_modcod_slot_info_t modcodSlotInfo[4];         /**< Transmission mode and slot information */
    uint8_t                            relativeTSForEachSlot[48]; /**< Relative TS ID for each slot */
    uint16_t                           tsidForEachRelativeTS[8];  /**< Corresponding table between relative TS and TSID */
    uint8_t                            ewsFlag;                   /**< Emergency Warning Broadcasting */
    uint8_t                            uplinkInfo;                /**< Site diversity warning */
} sony_isdbs_tmcc_info_t;

#endif /* SONY_ISDBS_H */
