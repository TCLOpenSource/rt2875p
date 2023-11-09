/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_demod.h"
#include "sony_demod_sat_device_ctrl.h"

/*------------------------------------------------------------------------------
  Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_sat_device_ctrl_Enable (sony_demod_t * pDemod, uint8_t enable)
{
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_Enable");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable/Enable HiZ Setting 1 (DSQOUT) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, (uint8_t)(enable ? 0x00 : 0x20), 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DSQIN pin setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9C, (uint8_t)(enable ? 0x40 : 0x00)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_DSQOUTSetting (sony_demod_t * pDemod,
                                                        sony_dsqout_mode_t mode,
                                                        uint8_t toneFreqKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_DSQOUTSetting");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if(toneFreqKHz == 22){
        /* Settings for 22KHz tone. */
        /* Keep default value. */
    } else if(toneFreqKHz == 44){
        /* Settings for 44KHz tone. */
        /* ODISEQC_CKDIV[11:8] */
        result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x30, 0x02, 0xFF);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /* ODISEQC_CKDIV[7:0] */
        result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x31, 0x21, 0xFF);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /*
         * No need to set ODISEQC_CKDIV220K because that register is for DiSEqC2.x.
         * 44KHz tone is not for DiSEqC.
         */
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }

    switch(mode)
    {
    case SONY_DSQOUT_MODE_PWM:
        /* Default */
        data = 0x00;
        break;

    case SONY_DSQOUT_MODE_ENVELOPE:
        data = 0x01;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* ODISEQC_LVLOUTON[0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x75, data, 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_RXENSetting (sony_demod_t * pDemod,
                                                      sony_rxen_mode_t mode,
                                                      uint8_t posDelay)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};
    uint32_t value = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_RXENSetting");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(mode)
    {
    case SONY_RXEN_MODE_NORMAL:
        /*
         * RXEN signal is normal.
         *   High : RX mode.
         *   Low  : TX mode.
         */
        data[0] = 0x00;
        break;

    case SONY_RXEN_MODE_INV:
        /*
         * RXEN signal is inverted.
         *   High : TX mode.
         *   Low  : RX mode.
         */
        data[0] = 0x01;
        break;

    case SONY_RXEN_MODE_FIXED_LOW:
        /*
         * RXEN signal is fixed low.
         */
        data[0] = 0x02;
        break;

    case SONY_RXEN_MODE_FIXED_HIGH:
        /*
         * RXEN signal is fixed high.
         */
        data[0] = 0x03;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* ODISEQC_RXENSEL[1:0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x78, data[0], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Fixed to 22KHz base */
    value = posDelay * 22;

    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)( value       & 0xFF);

    /*
     * "!" is the time to finish sending the DiSEqC command.
     *
     *              +-------------+
     *              |             |
     *  ---!--------+             +---------
     *      <------>
     *       posDelay(ms)
     */

    /* ODISEQC_RXPOSDLY[13:8] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x7D, data[0], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    /* ODISEQC_RXPOSDLY[7:0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x7E, data[1], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_TXENSetting (sony_demod_t * pDemod,
                                                      sony_txen_mode_t mode,
                                                      uint8_t posDelay,
                                                      uint8_t negDelay)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};
    uint32_t value = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_TXENSetting");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(mode)
    {
    case SONY_TXEN_MODE_NORMAL:
        data[0] = 0x00;
        break;

    case SONY_TXEN_MODE_INV:
        data[0] = 0x01;
        break;

    case SONY_TXEN_MODE_FIXED_LOW:
        data[0] = 0x02;
        break;

    case SONY_TXEN_MODE_FIXED_HIGH:
        data[0] = 0x03;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* ODISEQC_TXENSEL[1:0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x77, data[0], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Fixed to 22KHz base */
    value = posDelay * 22;

    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)( value       & 0xFF);

    /*
     * "!" is the time to start sequence to transmit single cable command.
     * "#" is the time to finish sequence to transmit single cable command.
     *
     *              +-------------+
     *              |             |
     *  ---!--------+             +--------#-
     *      <------>               <------>
     *       posDelay(ms)           negDelay(ms)
     */

    /* ODISEQC_TXPOSDLY[13:8] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x79, data[0], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    /* ODISEQC_TXPOSDLY[7:0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x7A, data[1], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    value = negDelay * 22;

    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)( value       & 0xFF);

    /* ODISEQC_TXNEGDLY[13:8] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x7B, data[0], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
    /* ODISEQC_TXNEGDLY[7:0] */
    result = sony_demod_SetAndSaveRegisterBits (pDemod, pDemod->i2cAddressSLVT, 0xBB, 0x7C, data[1], 0xFF);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_OutputTone(sony_demod_t * pDemod,
                                                    uint8_t isEnable)
{
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_OutputTone");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (isEnable){
        /* Tone ON */
        data = 0x01;
    } else {
        /* Tone OFF */
        data = 0x00;
    }

    /* ODISEQC_CBON[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x36, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_SetToneBurst(sony_demod_t * pDemod,
                                                      sony_toneburst_mode_t toneBurstMode)
{
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetToneBurst");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch(toneBurstMode)
    {
    case SONY_TONEBURST_MODE_OFF:
        /* ODISEQC_TBON[0] Disable */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x34, 0x00) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        SONY_TRACE_RETURN (SONY_RESULT_OK);

    case SONY_TONEBURST_MODE_A:
        data = 0x00;
        break;

    case SONY_TONEBURST_MODE_B:
        data = 0x01;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* ODISEQC_TBON[0] Enable */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x34, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ODISEQC_TBSEL[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_SetDiseqcCommand(sony_demod_t * pDemod,
                                                          uint8_t isEnable,
                                                          sony_diseqc_message_t * pCommand1,
                                                          uint8_t count1,
                                                          sony_diseqc_message_t * pCommand2,
                                                          uint8_t count2)
{
    int32_t index = 0;
    uint8_t data[SONY_DISEQC_MESSAGE_LENGTH];
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetDiseqcCommand");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (isEnable){
        if ((!pCommand1) || (count1 == 0)){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* ODISEQC_DISEQCON[0] Enable */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x33, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Length check */
        if (pCommand1->length > SONY_DISEQC_MESSAGE_LENGTH){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* ODISEQC_DCLEN0[3:0] Length of command1 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x3D, pCommand1->length) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Data of command1 */
        for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
            data[index] = 0x00;
        }
        for(index = 0; index < pCommand1->length; index++){
            data[index] = pCommand1->data[index];
        }
        /* ODISEQC_DCDAT0_1[7:0] - ODISEQC_DCDAT0_C[7:0] */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x3E, data, SONY_DISEQC_MESSAGE_LENGTH) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* ODISEQC_DCREP0[3:0] Repeat count of command1 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x37, count1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if ((!pCommand2) || (count2 == 0)){
            /* ODISEQC_DCREP1[3:0] Repeat count of command2 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x38, 0x00) != SONY_RESULT_OK){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        } else {
            /* Length check */
            if (pCommand2->length > SONY_DISEQC_MESSAGE_LENGTH){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
            }
            /* ODISEQC_DCLEN1[3:0] Length of command2 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4A, pCommand2->length) != SONY_RESULT_OK){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Data of command2 */
            for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
                data[index] = 0x00;
            }
            for(index = 0; index < pCommand2->length; index++){
                data[index] = pCommand2->data[index];
            }
            /* ODISEQC_DCDAT1_1[7:0] - ODISEQC_DCDAT1_C[7:0] */
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, data, SONY_DISEQC_MESSAGE_LENGTH) != SONY_RESULT_OK){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* ODISEQC_DCREP1[3:0] Repeat count of command2 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x38, count2) != SONY_RESULT_OK){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
    } else {
        /* ODISEQC_DISEQCON[0] Disable */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x33, 0x00) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_SetTxIdleTime(sony_demod_t * pDemod,
                                                       uint8_t idleTimeMs)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t value = 0;
    uint8_t data[2];
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetTxIdleTime");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Fixed to 22KHz */
    value = idleTimeMs * 22;
    if (value > 0x0FFF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    data[0] = (uint8_t)((value >> 8) & 0x0F);
    data[1] = (uint8_t)( value       & 0xFF);

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ODISEQC_TIMIDLETX[11:0] */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x39, data, 2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_SetRxIdleTime(sony_demod_t * pDemod,
                                                       uint8_t idleTimeMs)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t value = 0;
    uint8_t data[2];
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetRxIdleTime");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Fixed to 22KHz */
    value = idleTimeMs * 220;
    if (value > 0xFFFF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)( value       & 0xFF);

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* ODISEQC_TIMIDLERX[15:0] */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x67, data, 2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_SetR2RTime (sony_demod_t * pDemod,
                                                     uint32_t r2rTime)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t value = 0;
    uint8_t data[2];
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetR2RTime");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    value = r2rTime * 22;
    if (value > 0x3FFF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    data[0] = (uint8_t)((value >> 8) & 0x3F);
    data[1] = (uint8_t)( value       & 0xFF);

    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x71, data, 2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_SetDiseqcReplyMode(sony_demod_t * pDemod,
                                                            uint8_t isEnable,
                                                            uint16_t replyTimeoutMs)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t value = 0;
    uint8_t data[2];
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetDiseqcReplyMode");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (isEnable){

        value = (uint32_t)replyTimeoutMs * 22;
        if (value > 0x3FFF){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        data[0] = (uint8_t)((value >> 8) & 0x3F);
        data[1] = (uint8_t)( value       & 0xFF);

        /* ODISEQC_TIMIDLEREPLY[13:0] */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x3B, data, 2) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* diseqc_readok[0] Clear reply data register. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x74, 0x12) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else {
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x74, 0x00) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_StartTransmit(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_StartTransmit");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* diseqc_transmit[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x32, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_GetTransmitStatus(sony_demod_t * pDemod,
                                                           uint8_t * pStatus)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_GetTransmitStatus");

    if ((!pDemod) || (!pStatus)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* IDISEQC_TXSTATE[1:0] */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pStatus = data;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_GetReplyMessage(sony_demod_t * pDemod,
                                                         sony_diseqc_message_t * pReplyMessage)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    int32_t index = 0;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_GetReplyMessage");

    if ((!pDemod) || (!pReplyMessage)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* IDISEQC_DRVAL,IDISEQC_DROVF,IDISEQC_DRPERR,IDISEQC_DRDERR (4bit) */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data == 0x08){
        /* Success to receive reply message. */
        /* IDISEQC_DRLEN[4:0] Length of reply message */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x12, &data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (data > SONY_DISEQC_MESSAGE_LENGTH){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        pReplyMessage->length = data;
        for(index = 0; index < SONY_DISEQC_MESSAGE_LENGTH; index++){
            pReplyMessage->data[index] = 0;
        }
        /* IDISEQC_DRDAT0[7:0] - IDISEQC_DRDATF[7:0] */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x13, pReplyMessage->data, pReplyMessage->length) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_sat_device_ctrl_GetRXENMode (sony_demod_t * pDemod,
                                                      uint8_t * pIsEnable)
{
    uint8_t hiz = 0;
    uint8_t data[3] = {0, 0, 0};
    uint8_t tshiz = 0;
    uint8_t tsdata[4] = {0, 0, 0, 0};

    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_GetRXENMode");

    if ((!pDemod) || (!pIsEnable)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Slave    Bank    Addr    Bit      Default   Name
     * -----------------------------------------------------------
     * <SLV-X>  00h     A3h     [3:0]    4'h00     OREG_GPIO0_SEL
     * <SLV-X>  00h     A4h     [3:0]    4'h00     OREG_GPIO1_SEL
     * <SLV-X>  00h     A5h     [3:0]    4'h00     OREG_GPIO2_SEL
     * <SLV-X>  00h     82h     [2:0]    3'b111    OREG_GPIO_HIZ    0: HiZ Off, 1: HiZ On
     */

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x82, &hiz, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0xA3, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit      Default   Name
     * -----------------------------------------------------------
     * <SLV-X>  00h     B3h     [3:0]    4'h4      OREG_GPIOTD0_SEL
     * <SLV-X>  00h     B3h     [7:4]    4'h4      OREG_GPIOTD1_SEL
     * <SLV-X>  00h     B4h     [3:0]    4'h4      OREG_GPIOTD2_SEL
     * <SLV-X>  00h     B4h     [7:4]    4'h4      OREG_GPIOTD3_SEL
     * <SLV-X>  00h     B5h     [3:0]    4'h4      OREG_GPIOTD4_SEL
     * <SLV-X>  00h     B5h     [7:4]    4'h4      OREG_GPIOTD5_SEL
     * <SLV-X>  00h     B6h     [3:0]    4'h4      OREG_GPIOTD6_SEL
     * <SLV-X>  00h     B6h     [7:4]    4'h4      OREG_GPIOTD7_SEL
     * <SLV-T>  00h     81h     [7:0]    8'hFF     OREG_TSDATA_HIZ  0: HiZ Off, 1: HiZ On
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0xB3, tsdata, 4) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, &tshiz, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((((hiz & 0x01) == 0) && (data[0] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((hiz & 0x02) == 0) && (data[1] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((hiz & 0x04) == 0) && (data[2] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x01) == 0) && ((tsdata[0] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x02) == 0) && (((tsdata[0] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x04) == 0) && ((tsdata[1] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x08) == 0) && (((tsdata[1] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x10) == 0) && ((tsdata[2] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x20) == 0) && (((tsdata[2] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x40) == 0) && ((tsdata[3] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN)) ||
        (((tshiz & 0x80) == 0) && (((tsdata[3] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_RX_EN))) {

        /* Set SLV-T Bank : 0xBB */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* ODISEQC_RXENSEL[1:0] */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x78, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        switch(data[0])
        {
        case 0x00: /* Normal */
        case 0x01: /* Inverted */
            *pIsEnable = 1;
            break;

        case 0x02: /* Fixed low */
        case 0x03: /* Fixed high */
            *pIsEnable = 0;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
    } else {
        *pIsEnable = 0;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_GetTXENMode (sony_demod_t * pDemod,
                                                      uint8_t * pIsEnable)
{
    uint8_t hiz = 0;
    uint8_t data[3] = {0, 0, 0};
    uint8_t tshiz = 0;
    uint8_t tsdata[4] = {0, 0, 0, 0};

    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_GetTXENMode");

    if ((!pDemod) || (!pIsEnable)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Slave    Bank    Addr    Bit      Default   Name
     * -----------------------------------------------------------
     * <SLV-X>  00h     A3h     [3:0]    4'h00     OREG_GPIO0_SEL
     * <SLV-X>  00h     A4h     [3:0]    4'h00     OREG_GPIO1_SEL
     * <SLV-X>  00h     A5h     [3:0]    4'h00     OREG_GPIO2_SEL
     * <SLV-X>  00h     82h     [2:0]    3'b111    OREG_GPIO_HIZ    0: HiZ Off, 1: HiZ On
     */

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x82, &hiz, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0xA3, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit      Default   Name
     * -----------------------------------------------------------
     * <SLV-X>  00h     B3h     [3:0]    4'h4      OREG_GPIOTD0_SEL
     * <SLV-X>  00h     B3h     [7:4]    4'h4      OREG_GPIOTD1_SEL
     * <SLV-X>  00h     B4h     [3:0]    4'h4      OREG_GPIOTD2_SEL
     * <SLV-X>  00h     B4h     [7:4]    4'h4      OREG_GPIOTD3_SEL
     * <SLV-X>  00h     B5h     [3:0]    4'h4      OREG_GPIOTD4_SEL
     * <SLV-X>  00h     B5h     [7:4]    4'h4      OREG_GPIOTD5_SEL
     * <SLV-X>  00h     B6h     [3:0]    4'h4      OREG_GPIOTD6_SEL
     * <SLV-X>  00h     B6h     [7:4]    4'h4      OREG_GPIOTD7_SEL
     * <SLV-T>  00h     81h     [7:0]    8'hFF     OREG_TSDATA_HIZ  0: HiZ Off, 1: HiZ On
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0xB3, tsdata, 4) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, &tshiz, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((((hiz & 0x01) == 0) && (data[0] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((hiz & 0x02) == 0) && (data[1] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((hiz & 0x04) == 0) && (data[2] == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x01) == 0) && ((tsdata[0] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x02) == 0) && (((tsdata[0] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x04) == 0) && ((tsdata[1] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x08) == 0) && (((tsdata[1] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x10) == 0) && ((tsdata[2] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x20) == 0) && (((tsdata[2] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x40) == 0) && ((tsdata[3] & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN)) ||
        (((tshiz & 0x80) == 0) && (((tsdata[3] >> 4) & 0x0F) == (uint8_t)SONY_DEMOD_GPIO_MODE_DISEQC_TX_EN))) {

        /* Set SLV-T Bank : 0xBB */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* ODISEQC_TXENSEL[1:0] */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x77, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        switch(data[0])
        {
        case 0x00: /* Normal */
        case 0x01: /* Inverted */
            *pIsEnable = 1;
            break;

        case 0x02: /* Fixed low */
        case 0x03: /* Fixed high */
            *pIsEnable = 0;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
    } else {
        *pIsEnable = 0;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_SetDiseqcReplyMask (sony_demod_t * pDemod,
                                                             uint8_t pattern,
                                                             uint8_t mask)
{
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_SetDiseqcReplyMask");
    /* Set SLV-T Bank : 0xBB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xBB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6B, pattern) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6C, mask) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_EnableSinglecable (sony_demod_t * pDemod,
                                                            uint8_t enable)
{
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_EnableSinglecable");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pDemod->isSinglecable = enable;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
