/*------------------------------------------------------------------------------
  Copyright 2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_lnbc.h"
#include "ti_tps65235.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData);

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData);

static sony_result_t ti_tps65235_Initialize (sony_lnbc_t * pLnbc);

static sony_result_t ti_tps65235_SetConfig (sony_lnbc_t * pLnbc,
                                            sony_lnbc_config_id_t configId,
                                            int32_t value);

static sony_result_t ti_tps65235_SetVoltage (sony_lnbc_t * pLnbc,
                                             sony_lnbc_voltage_t voltage);

static sony_result_t ti_tps65235_SetTone (sony_lnbc_t * pLnbc,
                                          sony_lnbc_tone_t tone);

static sony_result_t ti_tps65235_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                  sony_lnbc_transmit_mode_t mode);

static sony_result_t ti_tps65235_Sleep (sony_lnbc_t * pLnbc);

static sony_result_t ti_tps65235_WakeUp (sony_lnbc_t * pLnbc);

static sony_result_t ti_tps65235_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                uint32_t * pStatus,
                                                uint32_t * pStatusSupported);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t ti_tps65235_Create (sony_lnbc_t * pLnbc,
                                  uint8_t i2cAddress,
                                  sony_i2c_t * pI2c)
{
    SONY_TRACE_ENTER ("ti_tps65235_Create");

    if ((!pLnbc) || (!pI2c)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->i2cAddress = i2cAddress;
    pLnbc->pI2c = pI2c;

    pLnbc->Initialize = ti_tps65235_Initialize;
    pLnbc->SetConfig = ti_tps65235_SetConfig;
    pLnbc->SetVoltage = ti_tps65235_SetVoltage;
    pLnbc->SetTone = ti_tps65235_SetTone;
    pLnbc->SetTransmitMode = ti_tps65235_SetTransmitMode;
    pLnbc->Sleep = ti_tps65235_Sleep;
    pLnbc->WakeUp = ti_tps65235_WakeUp;
    pLnbc->GetDiagStatus = ti_tps65235_GetDiagStatus;

    pLnbc->isInternalTone = 0;
    pLnbc->lowVoltage = TI_TPS65235_CONFIG_VOLTAGE_LOW_13_4V; /* Default value */
    pLnbc->highVoltage = TI_TPS65235_CONFIG_VOLTAGE_HIGH_18_2V; /* Default value */

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO;
    pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_Initialize (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_Initialize");

    if (!pLnbc) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->state = SONY_LNBC_STATE_UNKNOWN;
    pLnbc->voltage = SONY_LNBC_VOLTAGE_LOW;
    pLnbc->tone = SONY_LNBC_TONE_AUTO; /* Auto is only supported */

    if (pLnbc->transmitMode != SONY_LNBC_TRANSMIT_MODE_AUTO) {
        pLnbc->transmitMode = SONY_LNBC_TRANSMIT_MODE_TX;
    }

    result = create_1st_byte (pLnbc, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_2nd_byte (pLnbc, 0, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_SetConfig (sony_lnbc_t * pLnbc,
                                            sony_lnbc_config_id_t configId,
                                            int32_t value)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("ti_tps65235_SetConfig");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(configId)
    {
    case SONY_LNBC_CONFIG_ID_LOW_VOLTAGE:
        if ((value != TI_TPS65235_CONFIG_VOLTAGE_LOW_13_4V) &&
            (value != TI_TPS65235_CONFIG_VOLTAGE_LOW_14_6V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->lowVoltage = value;
            result = ti_tps65235_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_HIGH_VOLTAGE:
        if ((value != TI_TPS65235_CONFIG_VOLTAGE_HIGH_18_2V) &&
            (value != TI_TPS65235_CONFIG_VOLTAGE_HIGH_19_4V)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
        } else {
            pLnbc->highVoltage = value;
            result = ti_tps65235_SetVoltage (pLnbc, pLnbc->voltage);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        break;

    case SONY_LNBC_CONFIG_ID_TONE_INTERNAL:
        SONY_TRACE_RETURN (SONY_RESULT_OK); /* Internal/External tone is controlled by EXTM pin. No need to set register.
                                                     External: gated by EXTM logic pulse, Internal: gated by EXTM logic envelop */

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_SetVoltage (sony_lnbc_t * pLnbc,
                                             sony_lnbc_voltage_t voltage)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_SetVoltage");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_1st_byte (pLnbc, voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->voltage = voltage;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_SetTone (sony_lnbc_t * pLnbc,
                                          sony_lnbc_tone_t tone)
{
    SONY_TRACE_ENTER ("ti_tps65235_SetTone");

    if (!pLnbc){
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

    case SONY_LNBC_TONE_AUTO:
        /* Do nothing */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pLnbc->tone = tone;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_SetTransmitMode (sony_lnbc_t * pLnbc,
                                                  sony_lnbc_transmit_mode_t mode)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_SetTransmitMode");

    if (!pLnbc){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pLnbc->state != SONY_LNBC_STATE_ACTIVE) &&
        (pLnbc->state != SONY_LNBC_STATE_SLEEP)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = create_2nd_byte (pLnbc, 1, mode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->state == SONY_LNBC_STATE_ACTIVE) {
        if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    pLnbc->transmitMode = mode;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_Sleep (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_Sleep");

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

    result = create_2nd_byte (pLnbc, 0, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_SLEEP;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_WakeUp (sony_lnbc_t * pLnbc)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_WakeUp");

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

    result = create_1st_byte (pLnbc, pLnbc->voltage, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x00, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = create_2nd_byte (pLnbc, 1, pLnbc->transmitMode, &data);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (pLnbc->pI2c->WriteOneRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x01, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pLnbc->state = SONY_LNBC_STATE_ACTIVE;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ti_tps65235_GetDiagStatus (sony_lnbc_t * pLnbc,
                                                uint32_t * pStatus,
                                                uint32_t * pStatusSupported)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("ti_tps65235_GetDiagStatus");

    if ((!pLnbc) || (!pStatus)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pStatusSupported) {
        *pStatusSupported = SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE | SONY_LNBC_DIAG_STATUS_CABLEOPEN
            | SONY_LNBC_DIAG_STATUS_OVERCURRENT | SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    if (pLnbc->pI2c->ReadRegister (pLnbc->pI2c, pLnbc->i2cAddress, 0x02, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Status Register
     * +---------+---------+---------+---------+---------+---------+----------+----------+
     * | Reserve |TDETGOOD | LDO_ON  |  T125   |   TSD   |   OCP   |CABLE_GOOD|VOUT_GOOD |
     * +---------+---------+---------+---------+---------+---------+----------+----------+
     */

    *pStatus = 0;

    if (!(data & 0x01)) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_VOUTOUTOFRANGE;
    }

    if (!(data & 0x02)) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_CABLEOPEN;
    }

    if (data & 0x04) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_OVERCURRENT;
    }

    if (data & 0x08) {
        *pStatus |= SONY_LNBC_DIAG_STATUS_THERMALSHUTDOWN;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_1st_byte (sony_lnbc_t * pLnbc,
                                      sony_lnbc_voltage_t voltage,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_1st_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Control Register 1
     * +-------+-------+-------+-------+-------+-------+-------+---------+
     * |I2C_CON|PWM/PSM|   -   | VSET3 | VSET2 | VSET1 | VSET0 |EXTM_TONE|
     * +-------+-------+-------+-------+-------+-------+-------+---------+
     * |   ?   |   1   |   0   |   ?   |   ?   |   ?   |   ?   |    0    |
     * +-------+-------+-------+-------+-------+-------+-------+---------+
     * I2C_CON = 0 : I2C control disabled
     *           1 : I2C control enabled
     *
     * +------+------+------+------+------+
     * |VSET3 |VSET2 |VSET1 |VSET0 |LNB(V)|
     * +------+------+------+------+------+
     * |  0   |  1   |  0   |  0   | 13.4 |
     * +------+------+------+------+------+
     * |  0   |  1   |  1   |  0   | 14.6 |
     * +------+------+------+------+------+
     * |  1   |  1   |  0   |  0   | 18.2 |
     * +------+------+------+------+------+
     * |  1   |  1   |  1   |  0   | 19.4 |
     * +------+------+------+------+------+
     */
    *pData = 0x40;

    switch(voltage)
    {
    case SONY_LNBC_VOLTAGE_LOW:
    case SONY_LNBC_VOLTAGE_HIGH:
        /* I2C_CON = 1 */
        *pData |= 0x80;
        break;

    case SONY_LNBC_VOLTAGE_AUTO:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((voltage == SONY_LNBC_VOLTAGE_LOW) && (pLnbc->lowVoltage == TI_TPS65235_CONFIG_VOLTAGE_LOW_13_4V)){
        /* VSET[3:0] = 0x4 */
        *pData |= (0x4 << 1);
    } else if ((voltage == SONY_LNBC_VOLTAGE_LOW) && (pLnbc->lowVoltage == TI_TPS65235_CONFIG_VOLTAGE_LOW_14_6V)){
        /* VSET[3:0] = 0x6 */
        *pData |= (0x6 << 1);
    } else if ((voltage == SONY_LNBC_VOLTAGE_HIGH) && (pLnbc->highVoltage == TI_TPS65235_CONFIG_VOLTAGE_HIGH_18_2V)){
        /* VSET[3:0] = 0xC */
        *pData |= (0xC << 1);
    } else if ((voltage == SONY_LNBC_VOLTAGE_HIGH) && (pLnbc->highVoltage == TI_TPS65235_CONFIG_VOLTAGE_HIGH_19_4V)){
        /* VSET[3:0] = 0xE */
        *pData |= (0xE << 1);
    } else {
        /* VSET[3:0] = 0x4 */
        *pData |= (0x4 << 1);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t create_2nd_byte (sony_lnbc_t * pLnbc,
                                      uint8_t isEnable,
                                      sony_lnbc_transmit_mode_t mode,
                                      uint8_t * pData)
{
    SONY_TRACE_ENTER ("create_2nd_byte");

    if ((!pLnbc) || (!pData)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Control Register 2
     * +---------+---------+---------+---------+---------+---------+---------+----------+
     * | TONEAMP |  TIMER  |  l_SW   |  FSET   |   EN    |DOUTMODE |TONE_AUTO|TONE_TRANS|
     * +---------+---------+---------+---------+---------+---------+---------+----------+
     * |    0    |    0    |    0    |    1    |    ?    |    0    |    ?    |    ?     |
     * +---------+---------+---------+---------+---------+---------+---------+----------+
     * EN         = 0 : LNB output disabled
     *              1 : LNB output voltage Enabled
     * TONE_AUTO  = 0 : GDR (External bypass FET control) is controlled by TONE_TRANS
     *              1 : GDR (External bypass FET control) is automatically controlled by 22kHz tones transmit
     * TONE_TRANS = 0 : GDR output with VLNB voltage. Bypass FET is OFF for tone receiving from satellite,
     *              1 : GDR output with VCP voltage. Bypass FET is ON for tone transmit from TPS65235,
     */
    *pData = 0x10;

    if (isEnable){
        /* EN = 1 */
        *pData |= 0x08;
    }

    switch(mode)
    {
    case SONY_LNBC_TRANSMIT_MODE_TX:
        /* TONE_TRANS = 1 */
        *pData |= 0x01;
        break;

    case SONY_LNBC_TRANSMIT_MODE_RX:
        /* TONE_TRANS = 0 */
        break;

    case SONY_LNBC_TRANSMIT_MODE_AUTO:
        /* TONE_AUTO = 1 */
        *pData |= 0x02;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
