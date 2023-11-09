/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_dvbc2.h

          This file provides DVB-C2 related type definitions.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DVBC2_H
#define SONY_DVBC2_H

#include "sony_common.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
#define SONY_DVBC2_DS_MAX           255     /**< Maximum number of Data Slices in DVB-C2 system */
#define SONY_DVBC2_NUM_NOTCH_MAX    15      /**< Maximum number of notches for a C2 system */

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief The DVBC2 LDPC Code Length  definition.
*/
typedef enum {
    SONY_DVBC2_LDPC_SHORT = 0x00,                   /**< DVBC2 Short FECFRAME (16K). */
    SONY_DVBC2_LDPC_NORMAL = 0x01,                  /**< DVBC2 Normal FECFRAME (64K). */
    SONY_DVBC2_LDPC_UNKNOWN = 0xFF                  /**< DVBC2 Type Unknown. */
} sony_dvbc2_ldpc_code_length_t;

/**
 @brief DVBC2 PLP code rate definition.
*/
typedef enum {
    SONY_DVBC2_R_RSVD1 = 0x00,                      /**< Reserved. */
    SONY_DVBC2_R2_3 = 0x01,                         /**< DVBC2 2/3 code rate. */
    SONY_DVBC2_R3_4 = 0x02,                         /**< DVBC2 3/4 code rate. */
    SONY_DVBC2_R4_5 = 0x03,                         /**< DVBC2 4/5 code rate. */
    SONY_DVBC2_R5_6 = 0x04,                         /**< DVBC2 5/6 code rate. */
    SONY_DVBC2_R8_9 = 0x05,                         /**< DVBC2 8/9 code rate. */
    SONY_DVBC2_R9_10 = 0x06,                        /**< DVBC2 9/10 code rate. */
    SONY_DVBC2_R_RSVD2 = 0x07,                      /**< Reserved. */
    SONY_DVBC2_CR_UNKNOWN = 0xFF                    /**< UNKNOWN DVBC2 PLP code rate. */
} sony_dvbc2_plp_code_rate_t;

/**
 @brief The DVBC2 Guard interval type definition.
*/
typedef enum {
    SONY_DVBC2_GI_1_128 = 0x00,                     /**< DVBC2 1/128 guard interval*/
    SONY_DVBC2_GI_1_64 = 0x01,                      /**< DVBC2 1/64 guard interval*/
    SONY_DVBC2_GI_RSVD1 = 0x02,                     /**< Reserved. */
    SONY_DVBC2_GI_RSVD2 = 0x03,                     /**< Reserved. */
    SONY_DVBC2_GI_UNKNOWN = 0xFF                    /**< UNKNOWN guard interval*/
} sony_dvbc2_l1_gi_t;

/**
 @brief The DVBC2 Data Slice Type.
*/
typedef enum {
    SONY_DVBC2_DS_TYPE_1 = 0x00,                     /**< DVBC2 Data Slice Type 1 */
    SONY_DVBC2_DS_TYPE_2 = 0x01,                     /**< DVBC2 Data Slice Type 2*/
    SONY_DVBC2_DS_TYPE_UNKNOWN = 0xFF                /**< UNKNOWN Data Slice Type */
} sony_dvbc2_ds_type_t;

/**
 @brief DVBC2 types of PLP available.
*/
typedef enum {
    SONY_DVBC2_PLP_COMMON,                      /**< DVBC2 Common PLP type. */
    SONY_DVBC2_PLP_GROUPED_DATA,                /**< DVBC2 Grouped Data PLP type. */
    SONY_DVBC2_PLP_NORMAL_DATA,                 /**< DVBC2 Normal Data PLP type. */
    SONY_DVBC2_PLP_RESERVED                     /**< DVBC2 Reserved PLP type. */
} sony_dvbc2_plp_type_t;

/**
 @brief DVB-C2 constellation.
*/
typedef enum {
    SONY_DVBC2_CONSTELLATION_RSVD1 = 0x00,          /**< Reserved Value */
    SONY_DVBC2_CONSTELLATION_16QAM = 0x01,          /**< 16-QAM */
    SONY_DVBC2_CONSTELLATION_64QAM = 0x02,          /**< 64-QAM */
    SONY_DVBC2_CONSTELLATION_256QAM = 0x03,         /**< 256-QAM */
    SONY_DVBC2_CONSTELLATION_1024QAM = 0x04,        /**< 1024-QAM */
    SONY_DVBC2_CONSTELLATION_4096QAM = 0x05,        /**< 4096-QAM */
    SONY_DVBC2_CONSTELLATION_RSVD2 = 0x06,          /**< Reserved Value */
    SONY_DVBC2_CONSTELLATION_RSVD3 = 0x07,          /**< Reserved Value */
    SONY_DVBC2_CONSTELLATION_UNKNOWN = 0xFF         /**< UNKNOWN constellation*/
} sony_dvbc2_constellation_t;

/**
 @brief DVB-C2 time interleaved depth of the current data slice
*/
typedef enum {
    SONY_DVBC2_DS_TI_DEPTH_NONE = 0x00,             /**< DVBC2 no time interleaved depth */
    SONY_DVBC2_DS_TI_DEPTH_4_OFDM_SYM = 0x01,       /**< DVBC2 4 OFDM symbols time interleaved depth */
    SONY_DVBC2_DS_TI_DEPTH_8_OFDM_SYM = 0x02,       /**< DVBC2 8 OFDM symbols time interleaved depth */
    SONY_DVBC2_DS_TI_DEPTH_16_OFDM_SYM = 0x03       /**< DVBC2 16 OFDM symbols time interleaved depth */
} sony_dvbc2_ds_ti_depth_t;

/**
 @brief DVB-C2 FEC header type
*/
typedef enum {
    SONY_DVBC2_FEC_HEADER_TYPE_ROBUST = 0x00,           /**< DVBC2 Robust FEC header*/
    SONY_DVBC2_FEC_HEADER_TYPE_HIGH_EFFICIENCY = 0x01   /**< DVBC2 High efficiency FEC header*/
} sony_dvbc2_fec_header_type_t;

/**
 @brief DVB-C2 PLP payload type
*/
typedef enum {
    SONY_DVBC2_PLP_PAYLOAD_TYPE_GFPS = 0x00,        /**< DVBC2 GFPS PLP payload */
    SONY_DVBC2_PLP_PAYLOAD_TYPE_GCS = 0x01,         /**< DVBC2 GCS PLP payload */
    SONY_DVBC2_PLP_PAYLOAD_TYPE_GSE = 0x02,         /**< DVBC2 GSE PLP payload */
    SONY_DVBC2_PLP_PAYLOAD_TYPE_TS = 0x03,          /**< DVBC2 TS PLP payload */
    SONY_DVBC2_PLP_PAYLOAD_TYPE_RESERVED = 0x04     /**< Reserved */
} sony_dvbc2_plp_payload_type_t;

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief L1 signalling notch data structure.
*/
typedef struct sony_dvbc2_l1_notch_data_t {
    uint16_t notchStart;                            /**<  Notch start */
    uint16_t notchWidth;                            /**<  Notch width */
} sony_dvbc2_l1_notch_data_t;

/**
 @brief L1 signalling information structure definition.
*/
typedef struct sony_dvbc2_l1_t {
    uint16_t networkID;                             /**< The signal network ID. */
    uint16_t systemID;                              /**< The DVB-C2 system ID. */
    uint32_t startFrequency;                        /**< The start frequency of the signal in units of carriers. */
    uint16_t bandwidth;                             /**< The bandwidth of the signal in units of carriers. */
    sony_dvbc2_l1_gi_t gi;                          /**< Guard interval enumaration of the signal. */
    uint16_t frameLength;                           /**< The frame length. */
    uint8_t changeCntL1;                            /**< Change counter of L1 data. */
    uint8_t numDs;                                  /**< The number of data slices available in current DVB_C2 signal. */
    uint8_t numNotchs;                              /**< The number of notches present in current DVB_C2 signal. */
    uint8_t dsID;                                   /**< The ID of the currently tuned to Data Slice. */
    uint16_t dsTunePos;                             /**< The tuning offset of the currently tuned to Data Slice in units of (carriers/Dx) from the start frequency */
    int16_t dsOffsetOnLeft;                         /**< The left offset of the currently tuned to Data Slice. */
    int16_t dsOffsetOnRight;                        /**< The right offset of the currently tuned to Data Slice. */
    sony_dvbc2_ds_ti_depth_t dsTiDepth;             /**< The time interleaved depth of the currently tuned to Data Slice.*/
    sony_dvbc2_ds_type_t dsType;                    /**< The type (1 or 2) of the currently tuned to Data Slice. */
    sony_dvbc2_fec_header_type_t fecHdrType;        /**< The type of the FEC header. */
    uint8_t dsConstConf;                            /**< Indicates if the data slice configuration is fixed or variable */
    uint8_t dsLeftNotch;                            /**< Indicates the presence of a notch band to the left of the current data slice */
    uint8_t dsNumPLP;                               /**< The number of PLPs available (1 - 255) in the currently tuned to Data Slice. */
    uint8_t plpID;                                  /**< The ID of the selected PLP in the currently tuned to Data Slice. */
    uint8_t isPlpBundled;                           /**< Flag to indicate if the selected PLP in the currently tuned to Data Slice is bundled. */
    sony_dvbc2_plp_type_t plpType;                  /**< The type of the currently selected PLP. */
    sony_dvbc2_plp_payload_type_t plpPayloadType;   /**< The type of payload contained in the currently selected PLP. */
    uint8_t plpGroupID;                             /**< The group ID of the currently selected PLP. */
    uint16_t plpStart;                              /**< Indicates the position of the first complete XFEC frame of the associated in the current C2 frame */
    sony_dvbc2_ldpc_code_length_t plpFecType;       /**< The FEC type of the currently selected PLP. */
    sony_dvbc2_constellation_t plpMod;              /**< The modulation of the currently selected PLP */
    sony_dvbc2_plp_code_rate_t plpCod;              /**< The code rate of the currently selected PLP */
    uint8_t psisiReproc;                            /**< Indicates whether or not PSI/SI reprocessing is performed or not */
    uint16_t transportID;                           /**< The Transport Stream ID of the selected stream. */
    uint16_t origNetworkID;                         /**< The original network ID. */
    uint8_t reservedTone;                           /**< Reserved Tone. */

    /* Notch data */
    sony_dvbc2_l1_notch_data_t notches[SONY_DVBC2_NUM_NOTCH_MAX];        /**< Data detailing notch start and width */
} sony_dvbc2_l1_t;
#endif /* SONY_DVBC2_H */
