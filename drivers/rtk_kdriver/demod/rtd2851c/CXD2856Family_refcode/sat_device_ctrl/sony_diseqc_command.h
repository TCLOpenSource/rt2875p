/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**
 @file    sony_diseqc_command.h

          This file provides definitions for DiSEqC command.
*/
/*----------------------------------------------------------------------------*/
#ifndef SONY_DISEQC_COMMAND_H
#define SONY_DISEQC_COMMAND_H

#include "sony_demod_sat_device_ctrl.h"

/**
 @brief Definition of framing byte for DiSEqC message.
*/
typedef enum sony_diseqc_framing_t {
    SONY_DISEQC_FRAMING_MASTER_NO_REPLY_FIRST        = 0xE0, /**< Command from master, no reply required. (first transmission) */
    SONY_DISEQC_FRAMING_MASTER_NO_REPLY_REPEATED     = 0xE1, /**< Command from master, no reply required. (repeated transmission) */
    SONY_DISEQC_FRAMING_MASTER_REPLY_FIRST           = 0xE2, /**< Command from master, reply required. (first transmission) */
    SONY_DISEQC_FRAMING_MASTER_REPLY_REPEATED        = 0xE3, /**< Command from master, reply required. (repeated transmission) */
    SONY_DISEQC_FRAMING_SLAVE_REPLY_OK               = 0xE4, /**< Reply from slave, command OK, no errors detected.  */
    SONY_DISEQC_FRAMING_SLAVE_REPLY_CMD_UNSUPPORTED  = 0xE5, /**< Reply from slave, command not supported. */
    SONY_DISEQC_FRAMING_SLAVE_REPLY_ERR_PARITY       = 0xE6, /**< Reply from slave, parity error detected. */
    SONY_DISEQC_FRAMING_SLAVE_REPLY_CMD_UNKNOWN      = 0xE7  /**< Reply from slave, command not recoginised/unknown. Request repeat. */
} sony_diseqc_framing_t;

/**
 @brief Definition of address byte for DiSEqC message.
*/
typedef enum sony_diseqc_address_t {
    SONY_DISEQC_ADDRESS_ALL_DEVICES                       = 0x00, /**< Any device (Master to all devices) */
    SONY_DISEQC_ADDRESS_ALL_LNB_SMATV_SWITCHER            = 0x10, /**< Any LNB, Switcher or SMATV (Master to all...) */
    SONY_DISEQC_ADDRESS_LNB_DEVICE                        = 0x11, /**< LNB devices. */
    SONY_DISEQC_ADDRESS_LNB_WITH_LOOP_THROUGH             = 0x12, /**< LNB with loop-through switching. */
    SONY_DISEQC_ADDRESS_SWITCHER_DEVICE_DC_BLK            = 0x14, /**< Switcher (inc. DC blocking) */
    SONY_DISEQC_ADDRESS_SWITCHER_DEVICE_DC_LOOP           = 0x15, /**< Switcher with DC loop through. */
    SONY_DISEQC_ADDRESS_SMATV_DEVICE                      = 0x18, /**< SMATV devices. */
    SONY_DISEQC_ADDRESS_ANY_POLARISER                     = 0x20, /**< Any Polariser (Master to all Polarisers) */
    SONY_DISEQC_ADDRESS_LIN_POL_CONTROLLER                = 0x21, /**< Linear Polarisation Controller. */
    SONY_DISEQC_ADDRESS_ANY_POSITIONER                    = 0x30, /**< Any Positioner (Master to all Positioners) */
    SONY_DISEQC_ADDRESS_POLAR_POSITIONER                  = 0x31, /**< Polar/Azimuth Positioner. */
    SONY_DISEQC_ADDRESS_ELEVATION_POSITIONER              = 0x32, /**< Elevation Positioner. */
    SONY_DISEQC_ADDRESS_ANY_INSTALLER_AID                 = 0x40, /**< Any Installer Aid (Master to all Installer Aids). */
    SONY_DISEQC_ADDRESS_SIGNAL_STRENGTH_ANALOGUE          = 0x41, /**< Signal strength 0 Analogue value. */
    SONY_DISEQC_ADDRESS_ANY_INTELLI_SLAVES                = 0x70, /**< Any Intelligent Slave Interfaces (Master to all) */
    SONY_DISEQC_ADDRESS_SUBSCRIBER_CONTROLLED_HEADENDS    = 0x71  /**< Interface for subscriber controlled headends. */
} sony_diseqc_address_t;

#endif /* SONY_DISEQC_COMMAND_H */
