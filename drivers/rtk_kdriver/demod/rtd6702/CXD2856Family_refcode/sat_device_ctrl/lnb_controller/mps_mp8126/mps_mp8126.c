/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "mps_mp8126.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t mps_mp8126_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t mps_mp8126_SetConfig (sony_lnbc_t * pLnbc,
                                           sony_lnbc_config_id_t configId,
                                           int32_t value);

static sony_result_t mps_mp8126_SetVoltage (sony_lnbc_t * pLnbc,
                                            sony_lnbc_voltage_t voltage);

static sony_result_t mps_mp8126_SetTone (sony_lnbc_t * pLnbc,
                                         sony_lnbc_tone_t tone);

static sony_result_t mps_mp8126_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                 sony_lnbc_transmit_mode_t mode);

static sony_result_t mps_mp8126_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t mps_mp8126_WakeUp (sony_lnbc_t * pLnbc);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t mps_mp8126_Create (sony_lnbc_t * pLnbc,
                                 sony_demod_t * pDemod,
                                 sony_demod_gpio_pin_t voltagePinId,
                                 sony_demod_gpio_pin_t linedropPinId,
                                 sony_demod_gpio_pin_t enablePinId,
                                 mps_mp8126_t * pMP8126)
{
    SONY_TRACE_ENTER ("mps_mp8126_Create");

    if ((!pLnbc) || (!pDemod) || (!pMP8126)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = 0;
    pLnbc->pI2c = NULL;

    pLnbc->Initialize = mps_mp8126_Initialize;
    pLnbc->SetConfig = mps_mp8126_SetConfig;
    pLnbc->SetVoltage = mps_mp8126_SetVoltage;
    pLnbc->SetTone = mps_mp8126_SetTone;
    pLnbc->SetTransmitMode = mps_mp8126_SetTransmitMode;
    pLnbc->Sleep = mps_mp8126_Sleep;
    pLnbc->WakeUp = mps_mp8126_WakeUp;
    pLnbc->GetDiagStatus = NULL; /* Not supported */
    pLnbc->user = (void *)pMP8126;

    pLnbc->isInternalTone = 0;
    pLnbc->lowVoltage = MPS_MP8126_CONFIG_VOLTAGE_LOW_13V; /* Default value */
    pLnbc->highVoltage = MPS_MP8126_CONFIG_VOLTAGE_HIGH_18V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    pMP8126->pDemod = pDemod;
    pMP8126->voltagePinId = voltagePinId;
    pMP8126->linedropPinId = linedropPinId;
    pMP8126->enablePinId = enablePinId;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t mps_mp8126_Initialize (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    mps_mp8126_t * pMP8126;

    SONY_TRACE_ENTER ("mps_mp8126_Initialize");

    if ((!pLnbc) || (!pLnbc->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;

    if (pLnbc->voltage != SONY_LNBC_VOLTAGE_AUTO) {
        pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    }

    pLnbc->tone = SONY_LNBC_TONE_AUTO; /* Auto is only supported */
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX; /* RX is not supported */

    pMP8126 = (mps_mp8126_t *)(pLnbc->user);

    result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->enablePinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    /* Disable the LNBC. */
    result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->enablePinId, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO) {
        result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->voltagePinId, 1, SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->linedropPinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* In auto mode, linedrop cannot be controlled by every SetVoltage call. So set fixed value. */
        if ((pLnbc->lowVoltage == MPS_MP8126_CONFIG_VOLTAGE_LOW_14V) || (pLnbc->highVoltage == MPS_MP8126_CONFIG_VOLTAGE_HIGH_19V)) {
            /* Set LineDrop Pin to high state. */
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 1);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* Set LineDrop Pin to low state. */
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 0);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
    } else {
        result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->voltagePinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->linedropPinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Set Voltage Pin to low state. */
        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->voltagePinId, 0);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Set LineDrop Pin to low state. */
        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 0);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t mps_mp8126_SetConfig (sony_lnbc_t * pLnbc,
                                           sony_lnbc_config_id_t configId,
                                           int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("mps_mp8126_SetConfig");

    if ((!pLnbc) || (!pLnbc->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(configId)
    {
    case SONY_LNBC_CONFIG_ID_TONE_INTERNAL:
        if (value == 0) {
            pLnbc->isInternalTone = value;
        } else if (value == 1) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
        break;

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != MPS_MP8126_CONFIG_VOLTAGE_LOW_13V) &&
            (value != MPS_MP8126_CONFIG_VOLTAGE_LOW_14V)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = mps_mp8126_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != MPS_MP8126_CONFIG_VOLTAGE_HIGH_18V) &&
            (value != MPS_MP8126_CONFIG_VOLTAGE_HIGH_19V)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = mps_mp8126_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t mps_mp8126_SetVoltage (sony_lnbc_t * pLnbc,
                                            sony_lnbc_voltage_t voltage)
{
    sony_result_t result = SONY_RESULT_OK;
    mps_mp8126_t * pMP8126;

    SONY_TRACE_ENTER ("mps_mp8126_SetVoltage");

    if ((!pLnbc) || (!pLnbc->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pMP8126 = (mps_mp8126_t *)(pLnbc->user);

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
        if (pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO) {
            /* Configure voltage GPIO pin as an output. */
            result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->voltagePinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }

        if (pLnbc->lowVoltage == MPS_MP8126_CONFIG_VOLTAGE_LOW_13V) {
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 0);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else if (pLnbc->lowVoltage == MPS_MP8126_CONFIG_VOLTAGE_LOW_14V) {
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 1);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->voltagePinId, 0);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_LNBC_VOLTAGE_HIGH:
        if (pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO) {
            /* Configure voltage GPIO pin as an output. */
            result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->voltagePinId, 1, SONY_DEMOD_GPIO_MODE_OUTPUT);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }

        if (pLnbc->highVoltage == MPS_MP8126_CONFIG_VOLTAGE_HIGH_18V) {
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 0);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else if (pLnbc->highVoltage == MPS_MP8126_CONFIG_VOLTAGE_HIGH_19V) {
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 1);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->voltagePinId, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_LNBC_VOLTAGE_AUTO:
        /* In auto mode, linedrop cannot be controlled by every SetVoltage call. So set fixed value. */
        if ((pLnbc->lowVoltage == MPS_MP8126_CONFIG_VOLTAGE_LOW_14V) || (pLnbc->highVoltage == MPS_MP8126_CONFIG_VOLTAGE_HIGH_19V)) {
            /* Set LineDrop Pin to high state. */
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 1);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* Set LineDrop Pin to low state. */
            result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->linedropPinId, 0);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }

        /* Configure voltage GPIO pin as DISEQC TX. */
        result = sony_demod_GPIOSetConfig (pMP8126->pDemod, pMP8126->voltagePinId, 1, SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (result);
}

static sony_result_t mps_mp8126_SetTone (sony_lnbc_t * pLnbc,
                                         sony_lnbc_tone_t tone)
{
    SONY_TRACE_ENTER ("mps_mp8126_SetTone");

    if (!pLnbc) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(tone)
    {
    case SONY_LNBC_TONE_OFF:
    case SONY_LNBC_TONE_ON:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        break;

    case SONY_LNBC_TONE_AUTO:
        /* Do nothing */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t mps_mp8126_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                 sony_lnbc_transmit_mode_t mode)
{
    SONY_TRACE_ENTER ("mps_mp8126_SetTransmitMode");

    if (!pLnbc) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(mode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        /* Do nothing */
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t mps_mp8126_Sleep (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    mps_mp8126_t * pMP8126;

    SONY_TRACE_ENTER ("mps_mp8126_Sleep");

    if ((!pLnbc) || (!pLnbc->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pMP8126 = (mps_mp8126_t *)(pLnbc->user);

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->enablePinId, 0);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_LNBC_STATE_SLEEP:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (result);
}

static sony_result_t mps_mp8126_WakeUp (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    mps_mp8126_t * pMP8126;

    SONY_TRACE_ENTER ("mps_mp8126_WakeUp");

    if ((!pLnbc) || (!pLnbc->user)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pMP8126 = (mps_mp8126_t *)(pLnbc->user);

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_SLEEP:
        result = sony_demod_GPIOWrite (pMP8126->pDemod, pMP8126->enablePinId, 1);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (result);
}
