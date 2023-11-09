/*------------------------------------------------------------------------------
  Copyright 2016-2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "intersil_isl9492.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_tone_t tone,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData);

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData);

static sony_result_t create_3rd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData);

static sony_result_t intersil_isl9492_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t intersil_isl9492_SetConfig (sony_lnbc_t * pLnbc,
                                                 sony_lnbc_config_id_t configId,
                                                 int32_t value);

static sony_result_t intersil_isl9492_SetVoltage (sony_lnbc_t * pLnbc,
                                                  sony_lnbc_voltage_t voltage);

static sony_result_t intersil_isl9492_SetTone (sony_lnbc_t * pLnbc,
                                               sony_lnbc_tone_t tone);

static sony_result_t intersil_isl9492_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                       sony_lnbc_transmit_mode_t mode);

static sony_result_t intersil_isl9492_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t intersil_isl9492_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t intersil_isl9492_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                     uint32_t * pStatus,
                                                     uint32_t * pStatusSupported);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t intersil_isl9492_Create (sony_lnbc_t * pLnbc,
                                       uint8_t i2cAddress,
                                       sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("intersil_isl9492_Create");

    if ((!pLnbc) || (!pI2c)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = intersil_isl9492_Initialize;
    pLnbc->SetConfig = intersil_isl9492_SetConfig;
    pLnbc->SetVoltage = intersil_isl9492_SetVoltage;
    pLnbc->SetTone = intersil_isl9492_SetTone;
    pLnbc->SetTransmitMode = intersil_isl9492_SetTransmitMode;
    pLnbc->Sleep = intersil_isl9492_Sleep;
    pLnbc->WakeUp = intersil_isl9492_WakeUp;
    pLnbc->GetDiagStatus = intersil_isl9492_GetDiagStatus;

    pLnbc->isInternalTone = 0;
    pLnbc->lowVoltage = INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_13_5V; /* Default value */
    pLnbc->highVoltage = INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_18_5V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_Initialize (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_Initialize");

    if (!pLnbc) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;

    if (pLnbc->voltage != SONY_LNBC_VOLTAGE_AUTO) {
        pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    }

    if (pLnbc->tone != SONY_LNBC_TONE_AUTO) {
        pLnbc->tone = SONY_LNBC_TONE_OFF;
    }

    if (pLnbc->transmitMode != SONY_LNBC_TRANSMIT_MODE_AUTO) {
        pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;
    }

    result = create_1st_byte (pLnbc, pLnbc->tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_2nd_byte (pLnbc, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_3rd_byte (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_SetConfig (sony_lnbc_t * pLnbc,
                                                 sony_lnbc_config_id_t configId,
                                                 int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("intersil_isl9492_SetConfig");

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
        if ((value != 0) && (value != 1)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->isInternalTone = value;
            result = intersil_isl9492_SetTone (pLnbc, pLnbc->tone);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_13_5V) &&
            (value != INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_14_2V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = intersil_isl9492_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_18_5V) &&
            (value != INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_20_0V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = intersil_isl9492_SetVoltage (pLnbc, pLnbc->voltage);
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

static sony_result_t intersil_isl9492_SetVoltage (sony_lnbc_t * pLnbc,
                                                  sony_lnbc_voltage_t voltage)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pLnbc->voltage == SONY_LNBC_VOLTAGE_AUTO){
        switch(voltage)
        {
        case SONY_LNBC_VOLTAGE_AUTO:
            /* Do nothing */
            break;

        case SONY_LNBC_VOLTAGE_LOW:
        case SONY_LNBC_VOLTAGE_HIGH:
            result = create_2nd_byte (pLnbc, voltage, &data);
            if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

            if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
                if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }
            }
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
    } else if ((pLnbc->voltage == SONY_LNBC_VOLTAGE_LOW) || (pLnbc->voltage == SONY_LNBC_VOLTAGE_HIGH)){
        switch(voltage)
        {
        case SONY_LNBC_VOLTAGE_AUTO:
            /* Low/High -> Auto */
            result = create_2nd_byte (pLnbc, voltage, &data);
            if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

            if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
                if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }
            }
            break;

        case SONY_LNBC_VOLTAGE_LOW:
        case SONY_LNBC_VOLTAGE_HIGH:
            /* Do nothing */
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_3rd_byte (pLnbc, 1, voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_SetTone (sony_lnbc_t * pLnbc,
                                               sony_lnbc_tone_t tone)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_SetTone");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                       sony_lnbc_transmit_mode_t mode)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, pLnbc->tone, mode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_Sleep (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_Sleep");

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

    result = create_3rd_byte (pLnbc, 0, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}
    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_WakeUp (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_WakeUp");

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

    result = create_1st_byte (pLnbc, pLnbc->tone, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_2nd_byte (pLnbc, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_3rd_byte (pLnbc, 1, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->Write(pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t intersil_isl9492_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                     uint32_t * pStatus,
                                                     uint32_t * pStatusSupported)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("intersil_isl9492_GetDiagStatus");

    if ((!pLnbc) || (!pStatus)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pStatusSupported) {
        *pStatusSupported = SONY_LNBC_DIAG_STATUS_BACKBIAS | SONY_LNBC_DIAG_STATUS_OVERCURRENT
            | SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE | SONY_LNBC_DIAG_STATUS_CABLEOPEN
            | SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    if (pLnbc->pI2c->Read (pLnbc->pI2c, pLnbc->i2cAddress, &data, 1, SONY_I2C_START_EN|SONY_I2C_STOP_EN) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Status Register (SR1)
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     * |  SR1H   |  SR1M   |  SR1L   |   OTF   |  CABF   |  OUVF   |  OLF    |   BCF   |
     * +---------+---------+---------+---------+---------+---------+---------+---------+
     */

    *pStatus = 0;

    if (data & 0x01) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_BACKBIAS;
    }

    if (data & 0x02) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_OVERCURRENT;
    }

    if (data & 0x04) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE;
    }

    if (data & 0x08) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_CABLEOPEN;
    }

    if (data & 0x10) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_tone_t tone,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_1st_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* SR2
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |SR2H |SR2M |SR2L | ENT |MSEL | TTH |Res* |Res* |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  0  |  1  |  ?  |  ?  |  ?  |  0  |  0  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * ENT  = 0 : Tone OFF ( or auto )
     *      = 1 : Tone ON
     *
     * MSEL = 0 : Envelope (internal tone) / I2C control
     *      = 1 : PWM      (external tone)
     *
     * TTH  = 0 : RX mode
     *      = 1 : TX mode
     */
    *pData = 0x20;

    switch(tone)
    {
    case SONY_LNBC_TONE_OFF:
    case SONY_LNBC_TONE_AUTO:
        /* ENT = 0 */
        break;

    case SONY_LNBC_TONE_ON:
        /* ENT = 1 */
        *pData |= 0x10;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(tone)
    {
    case SONY_LNBC_TONE_AUTO:
        if (pLnbc->isInternalTone){
            /* MSEL = 0 */
        } else {
            /* MSEL = 1 */
            *pData |= 0x08;
        }
        break;

    case SONY_LNBC_TONE_OFF:
    case SONY_LNBC_TONE_ON:
        /* MSEL = 0 */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(mode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        /* TTH = 1 */
        *pData |= 0x04;
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        /* TTH = 0 */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_2nd_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* SR3
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |SR3H |SR3M |SR3L |Res* |VSPEN| DCL |ISELH|ISELL|
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  1  |  0  |  0  |  ?  |  0  |  0  |  0  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * VSPEN = 0 : TXEN enable
     *       = 1 : TXEN disable
     */
    *pData = 0x40;

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
    case SONY_LNBC_VOLTAGE_HIGH:
        /* VSPEN = 1 */
        *pData |= 0x08;
        break;

    case SONY_LNBC_VOLTAGE_AUTO:
        /* VSPEN = 0 */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_3rd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_3rd_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* SR4
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |SR4H |SR4M |SR4L | EN  |DLIN |Res* |VTOP |VBOT |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * |  0  |  1  |  1  |  ?  |  ?  |  0  |  ?  |  ?  |
     * +-----+-----+-----+-----+-----+-----+-----+-----+
     * EN   = 0 : SLEEP
     *      = 1 : ACTIVE
     *
     * DLIN = 0 : SLEEP
     *      = 1 : ACTIVE
     *
     * VTOP = 0 : Others
     *      = 1 : (Auto & HIGH-20V) / (High)
     *
     * VBOT = 0 : Others
     *      = 1 : (Auto & LOW-14V) / (Low & LOW-14V) / (High & HIGH-20V)
     */
    *pData = 0x60;

    if (isEnable){
        /* EN = 1, DLIN = 1 */
        *pData |= 0x18;
    }

    if ((voltage == SONY_LNBC_VOLTAGE_AUTO) && (pLnbc->highVoltage == INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_20_0V)){
        /* VTOP = 1 */
        *pData |= 0x02;
    } else if (voltage == SONY_LNBC_VOLTAGE_HIGH){
        /* VTOP = 1 */
        *pData |= 0x02;
    } else {
        /* VTOP = 0 */
    }

    if ((voltage == SONY_LNBC_VOLTAGE_AUTO) && (pLnbc->lowVoltage == INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_14_2V)){
        /* VBOT = 1 */
        *pData |= 0x01;
    } else if ((voltage == SONY_LNBC_VOLTAGE_LOW) && (pLnbc->lowVoltage == INTERSIL_ISL9492_CONFIG_VOLTAGE_LOW_14_2V)){
        /* VBOT = 1 */
        *pData |= 0x01;
    } else if ((voltage == SONY_LNBC_VOLTAGE_HIGH) && (pLnbc->highVoltage == INTERSIL_ISL9492_CONFIG_VOLTAGE_HIGH_20_0V)){
        /* VBOT = 1 */
        *pData |= 0x01;
    } else {
        /* VBOT = 0 */
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
