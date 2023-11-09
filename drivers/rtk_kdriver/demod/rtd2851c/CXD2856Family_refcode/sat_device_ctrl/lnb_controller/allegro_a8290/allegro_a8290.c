/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "allegro_a8290.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_transmit_mode_t transmitMode,
                                      sony_lnbc_tone_t tone,
                                      uint8_t * pData);

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData);

static sony_result_t allegro_a8290_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8290_SetConfig (sony_lnbc_t * pLnbc,
                                              sony_lnbc_config_id_t configId,
                                              int32_t value);

static sony_result_t allegro_a8290_SetVoltage (sony_lnbc_t * pLnbc,
                                               sony_lnbc_voltage_t voltage);

static sony_result_t allegro_a8290_SetTone (sony_lnbc_t * pLnbc,
                                            sony_lnbc_tone_t tone);

static sony_result_t allegro_a8290_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                    sony_lnbc_transmit_mode_t mode);

static sony_result_t allegro_a8290_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8290_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t allegro_a8290_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                  uint32_t * pStatus,
                                                  uint32_t * pStatusSupported);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t allegro_a8290_Create (sony_lnbc_t * pLnbc,
                                    uint8_t i2cAddress,
                                    sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("allegro_a8290_Create");

    if ((!pLnbc) || (!pI2c)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = allegro_a8290_Initialize;
    pLnbc->SetConfig = allegro_a8290_SetConfig;
    pLnbc->SetVoltage = allegro_a8290_SetVoltage;
    pLnbc->SetTone = allegro_a8290_SetTone;
    pLnbc->SetTransmitMode = allegro_a8290_SetTransmitMode;
    pLnbc->Sleep = allegro_a8290_Sleep;
    pLnbc->WakeUp = allegro_a8290_WakeUp;
    pLnbc->GetDiagStatus = allegro_a8290_GetDiagStatus;

    pLnbc->isInternalTone = 0;
    pLnbc->lowVoltage = ALLEGRO_A8290_CONFIG_VOLTAGE_LOW_13_042V; /* Default value */
    pLnbc->highVoltage = ALLEGRO_A8290_CONFIG_VOLTAGE_HIGH_18_042V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_Initialize (sony_lnbc_t * pLnbc)
{
    uint8_t data = 0;
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("allegro_a8290_Initialize");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;

    if (pLnbc->tone != SONY_LNBC_TONE_AUTO) {
        pLnbc->tone = SONY_LNBC_TONE_OFF;
    }

    if (pLnbc->transmitMode != SONY_LNBC_TRANSMIT_MODE_AUTO) {
        pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;
    }

    /* Read once */
    if (pLnbc->pI2c->Read(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* 1st byte */
    result = create_1st_byte (pLnbc, pLnbc->transmitMode, pLnbc->tone, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* 2nd byte */
    result = create_2nd_byte (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_SetConfig (sony_lnbc_t * pLnbc,
                                              sony_lnbc_config_id_t configId,
                                              int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("allegro_a8290_SetConfig");

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
        if((value != 0) && (value != 1)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->isInternalTone = value;
            result = allegro_a8290_SetTone (pLnbc, pLnbc->tone);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != ALLEGRO_A8290_CONFIG_VOLTAGE_LOW_13_042V) &&
            (value != ALLEGRO_A8290_CONFIG_VOLTAGE_LOW_14_042V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = allegro_a8290_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != ALLEGRO_A8290_CONFIG_VOLTAGE_HIGH_18_042V) &&
            (value != ALLEGRO_A8290_CONFIG_VOLTAGE_HIGH_19_042V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = allegro_a8290_SetVoltage (pLnbc, pLnbc->voltage);
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

static sony_result_t allegro_a8290_SetVoltage (sony_lnbc_t * pLnbc,
                                               sony_lnbc_voltage_t voltage)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* 2nd byte */
    result = create_2nd_byte (pLnbc, 1, voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_SetTone (sony_lnbc_t * pLnbc,
                                            sony_lnbc_tone_t tone)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_SetTone");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* 1st byte */
    result = create_1st_byte (pLnbc, pLnbc->transmitMode, tone, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                    sony_lnbc_transmit_mode_t mode)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* 1st byte */
    result = create_1st_byte (pLnbc, mode, pLnbc->tone, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_Sleep (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_Sleep");

    if (!pLnbc) {
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

    /* 2nd byte */
    result = create_2nd_byte (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_WakeUp (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_WakeUp");

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

    /* 1st byte */
    result = create_1st_byte (pLnbc, pLnbc->transmitMode, pLnbc->tone, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* 2nd byte */
    result = create_2nd_byte (pLnbc, 1, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN(result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t allegro_a8290_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                  uint32_t * pStatus,
                                                  uint32_t * pStatusSupported)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("allegro_a8290_GetDiagStatus");

    if ((!pLnbc) || (!pStatus)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pStatusSupported) {
        *pStatusSupported = SONY_LNBC_DIAG_STATUS_LNBCDISABLE | SONY_LNBC_DIAG_STATUS_OVERCURRENT
            | SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE | SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN
            | SONY_LNBC_DIAG_STATUS_VINOUTOFRANGE;
    }

    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Status Register 1
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * |   VUV   |   TSD   |    -    |   PNG   |    -    |   OCP   |    -    |   DIS   |
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     *
     * Status Register 2
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * |    -    |    -    |    -    |    -    |    -    |  TDET   |    -    |   CAD   |
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * Note: To activate CAD (cable disconnected) bit, the user must set this IC
     *       to cable disconnect test mode.
     *       This means status register 2 is unnecessary to read.
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

static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_transmit_mode_t transmitMode,
                                      sony_lnbc_tone_t tone,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_1st_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  |BFC2 |  -  |  -  |CADT |TGATE|TMODE|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  1  |  0  |  ?  |  0  |  0  |  0  |  ?  |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    *pData = 0x80;

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  |BFC2 |  -  |  -  |CADT |TGATE|TMODE|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  1  |  0  | 1/0 |  0  |  0  |  0  |  ?  |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    switch(transmitMode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        /* BFC2 = 1 */
        *pData |= 0x20;
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        /* BFC2 = 0 */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  |BFC2 |  -  |  -  |CADT |TGATE|TMODE|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  1  |  0  | 1/0 |  0  |  0  |  0  | 1/0 |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    switch(tone)
    {
    case SONY_LNBC_TONE_ON:
    case SONY_LNBC_TONE_AUTO:
        /* TGATE = 1 */
        *pData |= 0x02;
        break;

    case SONY_LNBC_TONE_OFF:
        /* TGATE = 0 */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  |BFC2 |  -  |  -  |CADT |TGATE|TMODE|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  1  |  0  | 1/0 |  0  |  0  |  0  | 1/0 | 1/0 |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    if (pLnbc->isInternalTone){
        /* TMODE = 1 */
        *pData |= 0x01;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_2nd_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  | ENB | ODT |VSEL3|VSEL2|VSEL1|VSEL0|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  0  |  ?  |  1  |  ?  |  ?  |  ?  |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    *pData = 0x10;

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  | ENB | ODT |VSEL3|VSEL2|VSEL1|VSEL0|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  0  | 1/0 |  1  |  ?  |  ?  |  ?  |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    if (isEnable){
        /* ENB = 1 */
        *pData |= 0x20;
    }

    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | I1  | I0  | ENB | ODT |VSEL3|VSEL2|VSEL1|VSEL0|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  0  | 1/0 |  1  | 1/0 | 1/0 | 1/0 | 1/0 |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */
    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
        switch(pLnbc->lowVoltage)
        {
        case ALLEGRO_A8290_CONFIG_VOLTAGE_LOW_13_042V:
            /* 13.042V : 0001 */
            *pData |= 0x01;
            break;

        case ALLEGRO_A8290_CONFIG_VOLTAGE_LOW_14_042V:
            /* 14.042V : 0100 */
            *pData |= 0x04;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    case SONY_LNBC_VOLTAGE_HIGH:
        switch(pLnbc->highVoltage)
        {
        case ALLEGRO_A8290_CONFIG_VOLTAGE_HIGH_18_042V:
            /* 18.042V : 1000 */
            *pData |= 0x08;
            break;

        case ALLEGRO_A8290_CONFIG_VOLTAGE_HIGH_19_042V:
            /* 19.042V : 1011 */
            *pData |= 0x0B;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    case SONY_LNBC_VOLTAGE_AUTO:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
