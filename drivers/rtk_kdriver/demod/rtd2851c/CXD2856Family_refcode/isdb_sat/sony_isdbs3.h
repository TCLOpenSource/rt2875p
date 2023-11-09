/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/09/02
  Modification ID : 4de34284da29630b3c6ce40872f48f739b35e390
------------------------------------------------------------------------------*/
/**
 @file    sony_isdbs3.h

          This file provides ISDB-S3 related type definitions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_ISDBS3_H
#define SONY_ISDBS3_H

#include "sony_common.h"

/*--------------------------------------------------------------------
  Enumerations
--------------------------------------------------------------------*/
/**
 @brief Type of StreamID.
*/
typedef enum sony_isdbs3_streamid_type_t {
    SONY_ISDBS3_STREAMID_TYPE_STREAMID,                /**< StreamID(16bit) */
    SONY_ISDBS3_STREAMID_TYPE_RELATIVE_STREAM_NUMBER,  /**< Relative StreamID number */
} sony_isdbs3_streamid_type_t;

/**
 @brief Modulation
*/
typedef enum sony_isdbs3_mod_t {
    SONY_ISDBS3_MOD_RESERVED_0,                /**< Reserved 0 */
    SONY_ISDBS3_MOD_BPSK,                      /**< BPSK */
    SONY_ISDBS3_MOD_QPSK,                      /**< QPSK */
    SONY_ISDBS3_MOD_8PSK,                      /**< 8PSK */
    SONY_ISDBS3_MOD_16APSK,                    /**< 16APSK */
    SONY_ISDBS3_MOD_32APSK,                    /**< 32APSK */
    SONY_ISDBS3_MOD_RESERVED_6,                /**< Reserved 6 */
    SONY_ISDBS3_MOD_RESERVED_7,                /**< Reserved 7 */
    SONY_ISDBS3_MOD_RESERVED_8,                /**< Reserved 8 */
    SONY_ISDBS3_MOD_RESERVED_9,                /**< Reserved 9 */
    SONY_ISDBS3_MOD_RESERVED_10,               /**< Reserved 10 */
    SONY_ISDBS3_MOD_RESERVED_11,               /**< Reserved 11 */
    SONY_ISDBS3_MOD_RESERVED_12,               /**< Reserved 12 */
    SONY_ISDBS3_MOD_RESERVED_13,               /**< Reserved 13 */
    SONY_ISDBS3_MOD_RESERVED_14,               /**< Reserved 14 */
    SONY_ISDBS3_MOD_UNUSED_15                  /**< unused (1111) */
} sony_isdbs3_mod_t;

/**
 @brief Code Rate
*/
typedef enum sony_isdbs3_cod_t {
    SONY_ISDBS3_COD_RESERVED_0,                /**< Reserved 0 */
    SONY_ISDBS3_COD_41_120,                    /**< 41/120 (1/3) */
    SONY_ISDBS3_COD_49_120,                    /**< 49/120 (2/5) */
    SONY_ISDBS3_COD_61_120,                    /**< 61/120 (1/2) */
    SONY_ISDBS3_COD_73_120,                    /**< 73/120 (3/5) */
    SONY_ISDBS3_COD_81_120,                    /**< 81/120 (2/3) */
    SONY_ISDBS3_COD_89_120,                    /**< 89/120 (3/4) */
    SONY_ISDBS3_COD_93_120,                    /**< 93/120 (7/9) */
    SONY_ISDBS3_COD_97_120,                    /**< 97/120 (4/5) */
    SONY_ISDBS3_COD_101_120,                   /**< 101/120 (5/6) */
    SONY_ISDBS3_COD_105_120,                   /**< 105/120 (7/8) */
    SONY_ISDBS3_COD_109_120,                   /**< 109/120 (9/10) */
    SONY_ISDBS3_COD_RESERVED_12,               /**< Reserved 12 */
    SONY_ISDBS3_COD_RESERVED_13,               /**< Reserved 13 */
    SONY_ISDBS3_COD_RESERVED_14,               /**< Reserved 14 */
    SONY_ISDBS3_COD_UNUSED_15                  /**< unused (1111) */
} sony_isdbs3_cod_t;

/**
 @brief Stream Type
*/
typedef enum sony_isdbs3_stream_type_t {
    SONY_ISDBS3_STREAM_TYPE_RESERVED_0,               /**< Reserved 0 */
    SONY_ISDBS3_STREAM_TYPE_MPEG2_TS,                 /**< MPEG-2 TS  */
    SONY_ISDBS3_STREAM_TYPE_TLV,                      /**< TLV        */
    SONY_ISDBS3_STREAM_TYPE_NO_TYPE_ALLOCATED = 0xFF, /**< No type allocated (11111111) */
} sony_isdbs3_stream_type_t;

/*------------------------------------------------------------------------------
  Structs
------------------------------------------------------------------------------*/
/**
  @brief Transmission mode, slot and backoff information.
*/
typedef struct sony_isdbs3_tmcc_modcod_slot_info_t {
    sony_isdbs3_mod_t            mod;        /**< Modulation */
    sony_isdbs3_cod_t            cod;        /**< Code rate */
    uint8_t                      slotNum;    /**< Slot number */
    uint8_t                      backoff;    /**< Backoff */
} sony_isdbs3_tmcc_modcod_slot_info_t;

/**
  @brief TMCC information.
*/
typedef struct sony_isdbs3_tmcc_info_t {
    uint8_t                             changeOrder;                                   /**< Incremented each time TMCC is renewed */
    sony_isdbs3_tmcc_modcod_slot_info_t modcodSlotInfo[8];                             /**< Transmission mode and slot information */
    sony_isdbs3_stream_type_t           streamTypeForEachRelativeStream[16];           /**< Stream type and relative stream number information */
    uint16_t                            packetLengthForEachRelativeStream[16];         /**< Packet length */
    uint8_t                             syncPatternBitLengthForEachRelativeStream[16]; /**< Sync pattern bit length */
    uint32_t                            syncPatternForEachRelativeStream[16];          /**< Sync pattern */
    uint8_t                             relativeStreamForEachSlot[120];                /**< Relative stream number and slot information */
    uint16_t                            streamidForEachRelativeStream[16];             /**< Corresponding table between relative StreamID */
    uint8_t                             ewsFlag;                                       /**< Emergency Warning Broadcasting */
    uint8_t                             uplinkInfo;                                    /**< Site diversity warning */
} sony_isdbs3_tmcc_info_t;

#endif /* SONY_ISDBS3_H */
