/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "st_lnbh23.h"

/*------------------------------------------------------------------------------
  Static Functions
------------------------------------------------------------------------------*/
static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      uint8_t pcl,
                                      sony_lnbc_voltage_t voltage,
                                      sony_lnbc_tone_t tone,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData);

static sony_result_t st_lnbh23_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t st_lnbh23_SetConfig (sony_lnbc_t * pLnbc,
                                          sony_lnbc_config_id_t configId,
                                          int32_t value);

static sony_result_t st_lnbh23_SetVoltage (sony_lnbc_t * pLnbc,
                                           sony_lnbc_voltage_t voltage);

static sony_result_t st_lnbh23_SetTone (sony_lnbc_t * pLnbc,
                                        sony_lnbc_tone_t tone);

static sony_result_t st_lnbh23_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                sony_lnbc_transmit_mode_t mode);

static sony_result_t st_lnbh23_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t st_lnbh23_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t st_lnbh23_GetDiagStatus (sony_lnbc_t * pLnbc,
                                              uint32_t * pStatus,
                                              uint32_t * pStatusSupported);

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t st_lnbh23_Create (sony_lnbc_t * pLnbc,
                                uint8_t i2cAddress,
                                sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("st_lnbh23_Create");

    if ((!pLnbc) || (!pI2c)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = st_lnbh23_Initialize;
    pLnbc->SetConfig = st_lnbh23_SetConfig;
    pLnbc->SetVoltage = st_lnbh23_SetVoltage;
    pLnbc->SetTone = st_lnbh23_SetTone;
    pLnbc->SetTransmitMode = st_lnbh23_SetTransmitMode;
    pLnbc->Sleep = st_lnbh23_Sleep;
    pLnbc->WakeUp = st_lnbh23_WakeUp;
    pLnbc->GetDiagStatus = st_lnbh23_GetDiagStatus;

    pLnbc->isInternalTone = 0; /* Not used */
    pLnbc->lowVoltage = ST_LNBH23_CONFIG_VOLTAGE_LOW_13_4V; /* Default value */
    pLnbc->highVoltage = ST_LNBH23_CONFIG_VOLTAGE_HIGH_18_5V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_Initialize (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh23_Initialize");

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

    result = create_1st_byte (pLnbc, 0, 0, pLnbc->voltage, pLnbc->tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_SetConfig (sony_lnbc_t * pLnbc,
                                          sony_lnbc_config_id_t configId,
                                          int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("st_lnbh23_SetConfig");

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
        /* This parameter is not available */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != ST_LNBH23_CONFIG_VOLTAGE_LOW_13_4V) &&
            (value != ST_LNBH23_CONFIG_VOLTAGE_LOW_14_4V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = st_lnbh23_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != ST_LNBH23_CONFIG_VOLTAGE_HIGH_18_5V) &&
            (value != ST_LNBH23_CONFIG_VOLTAGE_HIGH_19_5V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = st_lnbh23_SetVoltage (pLnbc, pLnbc->voltage);
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

static sony_result_t st_lnbh23_SetVoltage (sony_lnbc_t * pLnbc,
                                           sony_lnbc_voltage_t voltage)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};

    SONY_TRACE_ENTER ("st_lnbh23_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, 1, 0, voltage, pLnbc->tone, pLnbc->transmitMode, &data[1]);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        /* TTX should be "0" at changing voltage. */
        data[0] = (uint8_t)(data[1] & 0xBF);
        if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data[0], 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (data[0] != data[1]){
            if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data[1], 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_SetTone (sony_lnbc_t * pLnbc,
                                        sony_lnbc_tone_t tone)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh23_SetTone");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, 1, 0, pLnbc->voltage, tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                sony_lnbc_transmit_mode_t mode)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh23_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, 1, 0, pLnbc->voltage, pLnbc->tone, mode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_Sleep (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("st_lnbh23_Sleep");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        /* Do nothing */
        break;

    case SONY_LNBC_STATE_SLEEP:
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, 0, 0, pLnbc->voltage, pLnbc->tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_WakeUp (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("st_lnbh23_WakeUp");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(pLnbc->state)
    {
    case SONY_LNBC_STATE_ACTIVE:
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_LNBC_STATE_SLEEP:
        /* Do nothing */
        break;

    case SONY_LNBC_STATE_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, 1, 0, pLnbc->voltage, pLnbc->tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, (SONY_I2C_START_EN|SONY_I2C_STOP_EN)) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t st_lnbh23_GetDiagStatus (sony_lnbc_t * pLnbc,
                                              uint32_t * pStatus,
                                              uint32_t * pStatusSupported)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("st_lnbh23_GetDiagStatus");

    if ((!pLnbc) || (!pStatus)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pStatusSupported) {
        *pStatusSupported = SONY_LNBC_DIAG_STATUS_OVERCURRENT | SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN
            | SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE | SONY_LNBC_DIAG_STATUS_CABLEOPEN;
    }

    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Diagnostic received data
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * |  IMON   |  VMON   |  TMON   |   LLC   |  VSEL   |   EN    |   OTF   |   OLF   |
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     */

    *pStatus = 0;

    if (data & 0x01) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_OVERCURRENT;
    }

    if (data & 0x02) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    if (data & 0x40) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE;
    }

    if (data & 0x80) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_CABLEOPEN;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      uint8_t pcl,
                                      sony_lnbc_voltage_t voltage,
                                      sony_lnbc_tone_t tone,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData)
{
    /*
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * | PCL | TTX | TEN | LLC | VSEL| EN  |ITEST| AUX |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  ?  |  ?  |  ?  |  ?  |  ?  |  ?  |  0  |  0  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     */

    SONY_TRACE_ENTER ("create_1st_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (isEnable){
        *pData = 0x04;
    } else {
        /* Set "0" to all bit. */
        *pData = 0;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    if (pcl){
        *pData |= (uint8_t)0x80;
    }

    switch(mode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        *pData |= 0x40;
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        /* Do nothing */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(tone)
    {
    case SONY_LNBC_TONE_AUTO:
    case SONY_LNBC_TONE_OFF:
        /* Do nothing */
        break;

    case SONY_LNBC_TONE_ON:
        *pData |= 0x20;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
        switch(pLnbc->lowVoltage)
        {
        case ST_LNBH23_CONFIG_VOLTAGE_LOW_13_4V:
            /* Do nothing */
            break;

        case ST_LNBH23_CONFIG_VOLTAGE_LOW_14_4V:
            *pData |= 0x10;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
        break;

    case SONY_LNBC_VOLTAGE_HIGH:
        switch(pLnbc->highVoltage)
        {
        case ST_LNBH23_CONFIG_VOLTAGE_HIGH_18_5V:
            *pData |= 0x08;
            break;

        case ST_LNBH23_CONFIG_VOLTAGE_HIGH_19_5V:
            *pData |= 0x18;
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
