/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "allegro_a8303_1.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t allegro_a8303_1_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8303_1_SetConfig (sony_lnbc_t * pLnbc,
                                              sony_lnbc_config_id_t configId,
                                              int32_t value);

static sony_result_t allegro_a8303_1_SetVoltage (sony_lnbc_t * pLnbc,
                                               sony_lnbc_voltage_t voltage);

static sony_result_t allegro_a8303_1_SetTone (sony_lnbc_t * pLnbc,
                                            sony_lnbc_tone_t tone);

static sony_result_t allegro_a8303_1_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                    sony_lnbc_transmit_mode_t mode);

static sony_result_t allegro_a8303_1_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8303_1_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8303_1_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                    uint32_t * pStatus,
                                                    uint32_t * pStatusSupported);

static sony_result_t create_data (sony_lnbc_t * pLnbc,
                                  uint8_t isEnable,
                                  sony_lnbc_voltage_t voltage,
                                  uint8_t * pData);
/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t allegro_a8303_1_Create (sony_lnbc_t * pLnbc,
                                    uint8_t i2cAddress,
                                    sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("allegro_a8303_1_Create");

    if ((!pI2c) || (!pLnbc)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = allegro_a8303_1_Initialize;
    pLnbc->SetConfig = allegro_a8303_1_SetConfig;
    pLnbc->SetVoltage = allegro_a8303_1_SetVoltage;
    pLnbc->SetTone = allegro_a8303_1_SetTone;
    pLnbc->SetTransmitMode = allegro_a8303_1_SetTransmitMode;
    pLnbc->Sleep = allegro_a8303_1_Sleep;
    pLnbc->WakeUp = allegro_a8303_1_WakeUp;
    pLnbc->GetDiagStatus = allegro_a8303_1_GetDiagStatus;

    pLnbc->isInternalTone = 1; /* Fixed to 1 */
    pLnbc->lowVoltage = ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_13_333V; /* Default value */
    pLnbc->highVoltage = ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_18_667V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    /* A8303-1 do not have any I2C register to change TX/RX. */
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_AUTO;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_Initialize (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("allegro_a8303_1_Initialize");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO; /* Auto is only supported */

    if (pLnbc->transmitMode != SONY_LNBC_TRANSMIT_MODE_AUTO) {
        pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;
    }

    data = 0x00;
    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN | SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN | SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    data = 0x00;
    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN | SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN | SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_data (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_SetConfig (sony_lnbc_t * pLnbc,
                                              sony_lnbc_config_id_t configId,
                                              int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("allegro_a8303_1_SetConfig");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(configId)
    {
    case SONY_LNBC_CONFIG_ID_TONE_INTERNAL:
        if (value == 1) {
            pLnbc->isInternalTone = value;
        } else if (value == 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        }
        break;

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_11_667V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_13_333V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_13_667V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_14_333V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_15_667V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = allegro_a8303_1_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_18_667V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_19_000V) &&
            (value != ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_19_667V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = allegro_a8303_1_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_SetVoltage (sony_lnbc_t * pLnbc,
                                                 sony_lnbc_voltage_t voltage)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("allegro_a8303_1_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_data (pLnbc, 1, voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_SetTone (sony_lnbc_t * pLnbc,
                                            sony_lnbc_tone_t tone)
{
    SONY_TRACE_ENTER ("allegro_a8303_1_SetTone");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(tone)
    {
    case SONY_LNBC_TONE_ON:
    case SONY_LNBC_TONE_OFF:
        /* A8303_1 doesn't support this function. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        break;

    case SONY_LNBC_TONE_AUTO:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                      sony_lnbc_transmit_mode_t mode)
{
    SONY_TRACE_ENTER ("allegro_a8303_1_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pLnbc->transmitMode = mode;

    /* A8303_1 doesn't need to control something. */
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_Sleep (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("allegro_a8303_1_Sleep");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Continue */
        break;

    case SONY_LNBC_STATE_SLEEP:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_data (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_WakeUp (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("allegro_a8303_1_WakeUp");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Do nothing */
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_SLEEP:
        /* Continue */
        break;

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        /* Error */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_data (pLnbc, 1, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8303_1_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                    uint32_t * pStatus,
                                                    uint32_t * pStatusSupported)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8303_1_GetDiagStatus");

    if ((!pLnbc) || (!pStatus)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pStatusSupported) {
        *pStatusSupported = SONY_LNBC_DIAG_STATUS_LNBCDISABLE | SONY_LNBC_DIAG_STATUS_OVERCURRENT
            | SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE | SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN
            | SONY_LNBC_DIAG_STATUS_VINOUTOFRANGE;
    }

    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Cannot use repeated start */

    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* STATUS register
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * |  UVLO   |   TSD   |  TDET   |   PNG   |  TRIMS  |   OCP   |  CPOK   |   DIS   |
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     */

    *pStatus = 0;

    if (data & 0x01) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_LNBCDISABLE;
    }

    if (data & 0x04) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_OVERCURRENT;
    }

    if (data & 0x10) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE;
    }

    if (data & 0x40) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    if (data & 0x80) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_VINOUTOFRANGE;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_data (sony_lnbc_t * pLnbc,
                                  uint8_t isEnable,
                                  sony_lnbc_voltage_t voltage,
                                  uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_data");

    if ((!pLnbc) || (!pData)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    *pData = 0;

    if (isEnable){
        *pData |= 0x08;
    }

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
        switch(pLnbc->lowVoltage)
        {
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_11_667V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_13_333V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_13_667V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_14_333V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_LOW_15_667V:
            *pData |= pLnbc->lowVoltage;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    case SONY_LNBC_VOLTAGE_HIGH:
        switch(pLnbc->highVoltage)
        {
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_18_667V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_19_000V:
        case ALLEGRO_A8303_1_CONFIG_VOLTAGE_HIGH_19_667V:
            *pData |= pLnbc->highVoltage;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
