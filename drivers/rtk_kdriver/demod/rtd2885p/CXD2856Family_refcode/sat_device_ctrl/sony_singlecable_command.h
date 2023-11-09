/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_singlecable_command.h

          This file provides the function to generate "single cable commands".
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_SINGLECABLE_COMMAND_H
#define SONY_SINGLECABLE_COMMAND_H

#include "sony_common.h"
#include "sony_demod_sat_device_ctrl.h"

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/**
 @brief Definition of address byte for Single cable message.
*/
typedef enum {
    SONY_SINGLECABLE_ADDRESS_ALL_DEVICES            = 0x00, /**< Any device (Master to all devices) */
    SONY_SINGLECABLE_ADDRESS_ALL_LNB_SMATV_SWITCHER = 0x10, /**< Any LNB, Switcher or SMATV (Master to all...) */
    SONY_SINGLECABLE_ADDRESS_LNB_DEVICE             = 0x11  /**< LNB devices. */
} sony_singlecable_address_t;

/**
 @brief Definition of bank.
*/
typedef enum {
    SONY_SINGLECABLE_BANK_0 = 0,    /**< Position A, Vertical,   Low  band. */
    SONY_SINGLECABLE_BANK_1 = 1,    /**< Position A, Vertical,   High band. */
    SONY_SINGLECABLE_BANK_2 = 2,    /**< Position A, Horizontal, Low  band. */
    SONY_SINGLECABLE_BANK_3 = 3,    /**< Position A, Horizontal, High band. */
    SONY_SINGLECABLE_BANK_4 = 4,    /**< Position B, Vertical,   Low  band. */
    SONY_SINGLECABLE_BANK_5 = 5,    /**< Position B, Vertical,   High band. */
    SONY_SINGLECABLE_BANK_6 = 6,    /**< Position B, Horizontal, Low  band. */
    SONY_SINGLECABLE_BANK_7 = 7     /**< Position B, Horizontal, High band. */
} sony_singlecable_bank_t;

/**
 @brief Number of Satellite Positions definition.
*/
typedef enum {
    SONY_SINGLECABLE_NUM_OF_SAT_POS_1,   /**< 1 Satellite Position */
    SONY_SINGLECABLE_NUM_OF_SAT_POS_2    /**< 2 Satellite Positions */
} sony_singlecable_num_of_sat_pos_t;

/**
 @brief RF Band
*/
typedef enum {
    SONY_SINGLECABLE_RF_BAND_STANDARD,  /**< Standard RF */
    SONY_SINGLECABLE_RF_BAND_WIDE       /**< Wide band RF */
} sony_singlecable_rf_band_t;

/**
 @brief Number of User band (UB) slots definition.
*/
typedef enum {
    SONY_SINGLECABLE_NUM_OF_UB_2,   /**< 2 UB slots */
    SONY_SINGLECABLE_NUM_OF_UB_4,   /**< 4 UB slots */
    SONY_SINGLECABLE_NUM_OF_UB_6,   /**< 6 UB slots */
    SONY_SINGLECABLE_NUM_OF_UB_8    /**< 8 UB slots */
} sony_singlecable_num_of_ub_slots_t;

/**
 @brief Definition of local oscillator frequency.
*/
typedef enum {
    SONY_SINGLECABLE_LOFREQ_NONE            =   0x00, /**< None (switcher) */
    SONY_SINGLECABLE_LOFREQ_UNKNOWN         =   0x01, /**< Unknown */
    SONY_SINGLECABLE_LOFREQ_9750_MHZ        =   0x02, /**<  9750MHz */
    SONY_SINGLECABLE_LOFREQ_10000_MHZ       =   0x03, /**< 10000MHz */
    SONY_SINGLECABLE_LOFREQ_10600_MHZ       =   0x04, /**< 10600MHz */
    SONY_SINGLECABLE_LOFREQ_10750_MHZ       =   0x05, /**< 10750MHz */
    SONY_SINGLECABLE_LOFREQ_11000_MHZ       =   0x06, /**< 11000MHz */
    SONY_SINGLECABLE_LOFREQ_11250_MHZ       =   0x07, /**< 11250MHz */
    SONY_SINGLECABLE_LOFREQ_11475_MHZ       =   0x08, /**< 11475MHz */
    SONY_SINGLECABLE_LOFREQ_20250_MHZ       =   0x09, /**< 20250MHz */
    SONY_SINGLECABLE_LOFREQ_5150_MHZ        =   0x0A, /**<  5150MHz */
    SONY_SINGLECABLE_LOFREQ_1585_MHZ        =   0x0B, /**<  1585MHz */
    SONY_SINGLECABLE_LOFREQ_13850_MHZ       =   0x0C, /**< 13850MHz */
    SONY_SINGLECABLE_LOFREQ_WB_NONE         =   0x10, /**< Wide band RF, None(switcher) */
    SONY_SINGLECABLE_LOFREQ_WB_10000_MHZ    =   0x11, /**< Wide band RF, 10000MHz */
    SONY_SINGLECABLE_LOFREQ_WB_10200_MHZ    =   0x12, /**< Wide band RF, 10200MHz */
    SONY_SINGLECABLE_LOFREQ_WB_13250_MHZ    =   0x13, /**< Wide band RF, 13250MHz */
    SONY_SINGLECABLE_LOFREQ_WB_13450_MHZ    =   0x14  /**< Wide band RF, 13450MHz */
} sony_singlecable_lofreq_t;

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief structure for Single cable Config_Nb table.
*/
typedef struct {
    sony_singlecable_num_of_sat_pos_t numberOfSatPos;       /**< Number of Satellite Positions. */
    sony_singlecable_rf_band_t rfBand;                      /**< RF Band(Standard or Wide). */
    sony_singlecable_num_of_ub_slots_t numberOfUBSlots;     /**< Number of User band (UB) slots. */
} sony_singlecable_config_nb_t;

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
/**
 @brief Create "ODU_Channel_change" command, addressing a Single Outdoor Unit.  This is used
        to retune a specific user band to a new centre frequency.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot number allocated to the user. (1-8)
 @param ubSlotFreqKHz User band slot center frequency allocated to the user in KHz.
 @param bank Target bank to tune.
 @param centerFreqKHz Frequency to tune in KHz.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_Channel_change (sony_diseqc_message_t * pMessage,
                                                           sony_singlecable_address_t address,
                                                           uint8_t userbandId,
                                                           uint32_t ubSlotFreqKHz,
                                                           sony_singlecable_bank_t bank,
                                                           uint32_t centerFreqKHz);

/**
 @brief Create "ODU_PowerOFF" command, addressing a Single Outdoor Unit.  This is used to reduce
        power consumption and release a user band when the corresponding demodulator is turned off.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot number allocated to the user. (1-8)

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_PowerOFF (sony_diseqc_message_t * pMessage,
                                                     sony_singlecable_address_t address,
                                                     uint8_t userbandId);

/**
 @brief Create "ODU_UbxSignal_ON" command, addressing a Single Outdoor Unit. This is used to output
        an RF tone at the centre frequency of each user band on the cable.  This is primarily used
        for the receiver to scan the cable and detect the user band frequencies.

 @param pMessage The message instance.
 @param address Address byte.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_UbxSignal_ON (sony_diseqc_message_t * pMessage,
                                                         sony_singlecable_address_t address);

/**
 @brief Create "ODU_Config" command, addressing a Single Outdoor Unit.  This is primarily used to
        identify useful parameters of the installation and of the SCIF device.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot number allocated to the user.(1-8)
 @param numberOfSatPos The number of satellite position.
 @param rfBand The RF band(Standard(S) or Wide band(WB)).
 @param numberOfUBSlots The number of user band slots.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_Config (sony_diseqc_message_t * pMessage,
                                                   sony_singlecable_address_t address,
                                                   uint8_t userbandId,
                                                   sony_singlecable_num_of_sat_pos_t numberOfSatPos,
                                                   sony_singlecable_rf_band_t rfBand,
                                                   sony_singlecable_num_of_ub_slots_t numberOfUBSlots);

/**
 @brief Create "ODU_LoFreq" command, addressing a Single Outdoor Unit.  This is used to determine
        the value of the local oscillator frequency used in the LNB of the installation.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot number allocated to the user.(1-8)
 @param loFreq Local oscillator frequency.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_LoFreq (sony_diseqc_message_t * pMessage,
                                                   sony_singlecable_address_t address,
                                                   uint8_t userbandId,
                                                   sony_singlecable_lofreq_t loFreq);

/**
 @brief Create "ODU_Channel_change_MDU" command, addressing a Multi-Dwelling Unit.  This is used
        to retune a specific user band to a new centre frequency.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot number allocated to the user.(1-8)
 @param ubSlotFreqKHz User band slot center frequency allocated to the user in KHz.
 @param bank Target bank to tune.
 @param centerFreqKHz Center frequency to tune in KHz.
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_Channel_change_MDU (sony_diseqc_message_t * pMessage,
                                                               sony_singlecable_address_t address,
                                                               uint8_t userbandId,
                                                               uint32_t ubSlotFreqKHz,
                                                               sony_singlecable_bank_t bank,
                                                               uint32_t centerFreqKHz,
                                                               uint8_t pin);

/**
 @brief Create "ODU_PowerOFF_MDU" command, addressing a Multi-Dwelling Unit.  This is used to reduce
        power consumption and release a user band when the corresponding demodulator is turned off.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot allocated to the user.(1-8)
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_PowerOFF_MDU (sony_diseqc_message_t * pMessage,
                                                         sony_singlecable_address_t address,
                                                         uint8_t userbandId,
                                                         uint8_t pin);

/**
 @brief Create "ODU_UbxSignal_ON_MDU" command, addressing a Multi-Dwelling Unit. This is used to output
        an RF tone at the centre frequency of each user band on the cable.  This is primarily used
        for the receiver to scan the cable and detect the user band frequencies.

 @param pMessage The message instance.
 @param address Address byte.
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_UbxSignal_ON_MDU (sony_diseqc_message_t * pMessage,
                                                             sony_singlecable_address_t address,
                                                             uint8_t pin);

/**
 @brief Create "ODU_Config_MDU" command, addressing a Multi-Dwelling Unit.  This is primarily used to
        identify useful parameters of the installation and of the SCIF device.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band allocated to the user.(1-8)
 @param numberOfSatPos The number of satellite position.
 @param rfBand The RF band(Standard(S) or Wide band(WB)).
 @param numberOfUBSlots The number of user band slots.
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_Config_MDU (sony_diseqc_message_t * pMessage,
                                                       sony_singlecable_address_t address,
                                                       uint8_t userbandId,
                                                       sony_singlecable_num_of_sat_pos_t numberOfSatPos,
                                                       sony_singlecable_rf_band_t rfBand,
                                                       sony_singlecable_num_of_ub_slots_t numberOfUBSlots,
                                                       uint8_t pin);

/**
 @brief Create "ODU_LoFreq_MDU" command, addressing a Multi-Dwelling Unit.  This is used to determine
        the value of the local oscillator frequency used in the LNB of the installation.

 @param pMessage The message instance.
 @param address Address byte.
 @param userbandId User band slot allocated to the user.(1-8)
 @param loFreq Local oscillator frequency.
 @param pin PIN code.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_singlecable_command_ODU_LoFreq_MDU (sony_diseqc_message_t * pMessage,
                                                       sony_singlecable_address_t address,
                                                       uint8_t userbandId,
                                                       sony_singlecable_lofreq_t loFreq,
                                                       uint8_t pin);

#endif /* SONY_SINGLECABLE_COMMAND_H */
