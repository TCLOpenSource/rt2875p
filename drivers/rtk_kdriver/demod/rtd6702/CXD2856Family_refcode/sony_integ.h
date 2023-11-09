/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/04/13
  Modification ID : 8382e363f23ef874189bb623136d4ecd6d419004
------------------------------------------------------------------------------*/
/**
 @file    sony_integ.h

          This file provides the integration layer control interface.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_INTEG_H
#define SONY_INTEG_H

#include "sony_demod.h"

#ifdef SONY_DEMOD_SUPPORT_SAT
#include "sony_tuner_sat.h"
#endif

#ifdef SONY_DEMOD_SUPPORT_SAT_DEVICE_CTRL
#include "sony_lnbc.h"
#endif

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
//#include "sony_tuner_terr_cable.h"
#endif

#include "sony_common.h"

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/**
 @brief The high level driver object.
        This is the primary interface used for controlling the demodulator and
        connected tuner devices.

        This object is the combination of the demodulator and the tuner devices
        to represent a single front end system. It can be used wholly, partly or
        just as a reference for developing the application software.
*/
typedef struct sony_integ_t {
    sony_demod_t * pDemod;                          /**< Instance of the demodulator. */
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
    //sony_tuner_terr_cable_t * pTunerTerrCable;      /**< The connected tuner for terrestrial and cable systems. */
#endif
#ifdef SONY_DEMOD_SUPPORT_SAT
    sony_tuner_sat_t * pTunerSat;                   /**< The connected tuner satellite . */
#endif
#ifdef SONY_DEMOD_SUPPORT_SAT_DEVICE_CTRL
    sony_lnbc_t * pLnbc;                            /**< The connected lnb controller. */
#endif

    sony_atomic_t cancel;                           /**< Cancellation indicator variable. */

    void * user;                                    /**< Storage for user specific data to be passed in/out of the integration layer. */
} sony_integ_t;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
/**
 @brief Construct the driver.

        This function is called by the application in order to setup the
        ::sony_demod_t structure and provide references for the
        ::sony_integ_t::pDemod, ::sony_integ_t::pTunerTerrCable,
        ::sony_integ_t::pTunerSat and ::sony_integ_t::pLnbc members.

        This MUST be called before calling ::sony_integ_Initialize.

@note Passing NULL as pTunerTerrCable, pTunerSat or pLnbc will disable communication
      with the device without throwing an arguement error.

        Notes on driver configuration:
        - By default, the demodulator uses an inverted AGC.  If the connected
          tuner uses a non-inverted IFAGC, call ::sony_demod_SetConfig with
          ::SONY_DEMOD_CONFIG_IFAGCNEG = 0 to setup the demodulator with postive
          IFAGC sense, after calling ::sony_integ_Initialize.

 @note  Memory is not allocated dynamically.

 @param pInteg The driver object to create. It must be a valid pointer to
        allocated memory for a ::sony_integ_t structure.
 @param pDemod Reference to memory allocated for the demodulator instance. The
        create function will setup this demodulator instance also.
 @param pCreateParam Parameters to create this driver. (I2C slave address, Xtal frequency...)
 @param pDemodI2c The I2C driver that the demod will use as the I2C interface.
 @param pTunerTerrCable The tuner driver to use with this instance of the driver
        for terrestrial/cable systems. Note : The tuner I2C interface should
        have been setup before this call.
 @param pTunerSat The tuner driver to use with this instance of the driver for
        satellite systems. Note : The tuner I2C interface should have been setup
        before this call.
 @param pLnbc The LNB control interface.

 @return SONY_RESULT_OK if successfully created integration layer driver structure.
*/
sony_result_t sony_integ_Create (sony_integ_t * pInteg,
                                 sony_demod_t * pDemod,
                                 sony_demod_create_param_t * pCreateParam,
                                 sony_i2c_t * pDemodI2c
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
                                 //,sony_tuner_terr_cable_t * pTunerTerrCable
#endif
#ifdef SONY_DEMOD_SUPPORT_SAT
                                 ,sony_tuner_sat_t * pTunerSat
#endif
#ifdef SONY_DEMOD_SUPPORT_SAT_DEVICE_CTRL
                                 ,sony_lnbc_t * pLnbc
#endif
                                 );

/**
 @brief Initialize the demodulator and tuners.

        After initialization, the demodulator, tuner and LNB controller are in a low power state
        (demod state = ::SONY_DEMOD_STATE_SLEEP) awaiting tune or scan commands.

        Normally be used from Power On (::SONY_DEMOD_STATE_UNKNOWN).
        Calling from Active, Sleep or Shutdown states will cause a demodulator reset.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Initialize (sony_integ_t * pInteg);

/**
 @brief Put the demodulator and tuner devices into a low power state.

        This function can be called from Shutdown or Active states.
        Note that LNB controller is not controlled by this function.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Sleep (sony_integ_t * pInteg);

/**
 @brief Shutdown the demodulator, tuner and LNB controller into a low power disabled state.

        Call ::sony_integ_Sleep to go out from Shutdown state.
        ::sony_integ_Shutdown can be directly called from Sleep or Active states.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_Shutdown (sony_integ_t * pInteg);

/**
 @brief Cancels current Tune or Scan operation in the demod and tuner parts.

        This function is thread safe, calling thread will get the result
        SONY_RESULT_ERROR_CANCEL.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if able to cancel the pending operation.
*/
sony_result_t sony_integ_Cancel (sony_integ_t * pInteg);

/**
 @brief Check cancel request from integ APIs.

        This API is called from sony_integ_XXX APIs to check that
        cancellation request by ::sony_integ_Cancel API is exist or not.
        The user can change this API code to check their system dependent
        cancellation request.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if there is no cancellation request.
         SONY_RESULT_ERROR_CANCEL if cancellation request exists.
*/
sony_result_t sony_integ_CheckCancellation (sony_integ_t * pInteg);

#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
/**
 @brief Set terrestrial/cable tuner sleep.

        This API is called from satellite sony_integ_XXX_Tune APIs to set
        unnecessary terrestrial/cable tuner to sleep state.

        Note that for terrestial/cable and satellite dual supported tuners,
        this code may be unnecessary.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_terr_cable_SetTunerSleep (sony_integ_t * pInteg);
#endif /* SONY_DEMOD_SUPPORT_TERR_OR_CABLE */

#ifdef SONY_DEMOD_SUPPORT_SAT
/**
 @brief Set satellite tuner sleep.

        This API is called from terrestrial/cable sony_integ_XXX_Tune APIs to set
        unnecessary satellite tuner to sleep state.

        Note that for terrestial/cable and satellite dual supported tuners,
        this code may be unnecessary.

 @param pInteg The driver instance.

 @return SONY_RESULT_OK if OK.
*/
sony_result_t sony_integ_sat_SetTunerSleep (sony_integ_t * pInteg);
#endif /* SONY_DEMOD_SUPPORT_SAT */

#endif /* SONY_INTEG_H */
