/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_dvbs2.h

          This file provides DVB-S2 related type definitions.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DVBS2_H
#define SONY_DVBS2_H

#include "sony_common.h"

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief DVB-S2 code rate.
*/
typedef enum {
    SONY_DVBS2_CODERATE_1_4,                /**< 1/4 */
    SONY_DVBS2_CODERATE_1_3,                /**< 1/3 */
    SONY_DVBS2_CODERATE_2_5,                /**< 2/5 */
    SONY_DVBS2_CODERATE_1_2,                /**< 1/2 */
    SONY_DVBS2_CODERATE_3_5,                /**< 3/5 */
    SONY_DVBS2_CODERATE_2_3,                /**< 2/3 */
    SONY_DVBS2_CODERATE_3_4,                /**< 3/4 */
    SONY_DVBS2_CODERATE_4_5,                /**< 4/5 */
    SONY_DVBS2_CODERATE_5_6,                /**< 5/6 */
    SONY_DVBS2_CODERATE_8_9,                /**< 8/9 */
    SONY_DVBS2_CODERATE_9_10,               /**< 9/10 */
    SONY_DVBS2_CODERATE_RESERVED_29,        /**< Reserved */
    SONY_DVBS2_CODERATE_RESERVED_30,        /**< Reserved */
    SONY_DVBS2_CODERATE_RESERVED_31,        /**< Reserved */
    SONY_DVBS2_CODERATE_INVALID             /**< Invalid */
} sony_dvbs2_coderate_t;

/**
 @brief DVB-S2 modulation.
*/
typedef enum {
    SONY_DVBS2_MODULATION_QPSK,             /**< QPSK */
    SONY_DVBS2_MODULATION_8PSK,             /**< 8PSK */
    SONY_DVBS2_MODULATION_16APSK,           /**< 16APSK */
    SONY_DVBS2_MODULATION_32APSK,           /**< 32APSK */
    SONY_DVBS2_MODULATION_RESERVED_29,      /**< Reserved */
    SONY_DVBS2_MODULATION_RESERVED_30,      /**< Reserved */
    SONY_DVBS2_MODULATION_RESERVED_31,      /**< Reserved */
    SONY_DVBS2_MODULATION_DUMMY_PLFRAME,    /**< Dummy PL Frame */
    SONY_DVBS2_MODULATION_INVALID           /**< Invalid */
} sony_dvbs2_modulation_t;

/**
 @brief DVB-S2 Stream Input.
*/
typedef enum {
    SONY_DVBS2_STREAM_GENERIC_PACKETIZED = 0x00,   /**< Generic Packetized Stream Input. */
    SONY_DVBS2_STREAM_GENERIC_CONTINUOUS = 0x01,   /**< Generic Continuous Stream Input. */
    SONY_DVBS2_STREAM_RESERVED = 0x02,             /**< Reserved. */
    SONY_DVBS2_STREAM_TRANSPORT = 0x03             /**< Transport Stream Input. */
} sony_dvbs2_stream_t;

/**
 @brief DVB-S2 Transmission Roll-off factor.
*/
typedef enum {
    SONY_DVBS2_ROLLOFF_35 = 0x00,           /**< Roll-off = 0.35. */
    SONY_DVBS2_ROLLOFF_25 = 0x01,           /**< Roll-off = 0.25. */
    SONY_DVBS2_ROLLOFF_20 = 0x02,           /**< Roll-off = 0.20. */
    SONY_DVBS2_ROLLOFF_RESERVED = 0x03      /**< Reserved. */
} sony_dvbs2_rolloff_t;

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief DVB-S2 PLS code information.
*/
typedef struct {
    sony_dvbs2_modulation_t modulation; /**< Modulation. */
    sony_dvbs2_coderate_t codeRate;     /**< Code rate. */
    uint8_t isShortFrame;               /**< FEC Frame size (0:normal, 1:short). */
    uint8_t isPilotOn;                  /**< Pilot mode (0:Off, 1:On). */
} sony_dvbs2_plscode_t;

/**
 @brief DVB-S2 base-band Header (BBHEADER) information.
*/
typedef struct {
    sony_dvbs2_stream_t streamInput;            /**< Stream Input (Transport or Generic). */
    uint8_t isSingleInputStream;                /**< Multiple Input Stream or Single Input Stream. */
    uint8_t isConstantCodingModulation;         /**< Constant or Adaptive Coding and Modulation. */
    uint8_t issyIndicator;                      /**< Input Stream Synchronisation Indicator. */
    uint8_t nullPacketDeletion;                 /**< Null Packet Deletion active/not active. */
    sony_dvbs2_rolloff_t rollOff;               /**< Transmission Roll-Off factor. */
    uint8_t inputStreamIdentifier;              /**< Input Stream Identifier (only valid for MIS). */
    uint16_t userPacketLength;                  /**< User Packet Length in bits. */
    uint16_t dataFieldLength;                   /**< Data Field Length in bits. */
    uint8_t syncByte;                           /**< User Packet Sync Byte. */
} sony_dvbs2_bbheader_t;

#endif /* SONY_DVBS2_H */
