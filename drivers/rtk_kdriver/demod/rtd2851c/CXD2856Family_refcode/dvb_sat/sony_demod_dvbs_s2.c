/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/09/19
  Modification ID : c6dd9af39653338199a835e79fc5262ecbd324b6
------------------------------------------------------------------------------*/
#include "sony_dtv.h"
#include "sony_common.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t SLtoASX (sony_demod_t * pDemod,
                               sony_dtv_system_t dtvSystem,
                               uint32_t symbolRateKSps);

static sony_result_t ASXtoSL (sony_demod_t * pDemod);

static sony_result_t ASXtoASX (sony_demod_t * pDemod,
                               uint32_t symbolRateKSps);

static sony_result_t TSRStoSL(sony_demod_t * pDemod);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_Tune (sony_demod_t * pDemod,
                                       sony_dvbs_s2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pTuneParam->system != SONY_DTV_SYSTEM_DVBS) &&
        (pTuneParam->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pTuneParam->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state == SONY_DEMOD_STATE_ACTIVE) {
        /* Active */
        if (pDemod->dvbss2ScanMode > 0){
            /* Scan -> Sleep */
            result = TSRStoSL (pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            /* Sleep -> Active-S(X) */
            result = SLtoASX (pDemod, pTuneParam->system, pTuneParam->symbolRateKSps);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else if (pDemod->system == pTuneParam->system){
            /* Active-S(X) -> Active-S(X) */
            result = ASXtoASX (pDemod, pTuneParam->symbolRateKSps);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else {
            /* To Sleep */
            result = sony_demod_Sleep (pDemod);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
            /* Sleep -> Active-S(X) */
            result = SLtoASX (pDemod, pTuneParam->system, pTuneParam->symbolRateKSps);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
    } else if (pDemod->state == SONY_DEMOD_STATE_SLEEP){
        /* Sleep -> Active-S(X) */
        result = SLtoASX (pDemod, pTuneParam->system, pTuneParam->symbolRateKSps);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pDemod->state = SONY_DEMOD_STATE_ACTIVE;
    pDemod->system = pTuneParam->system;
    pDemod->dvbss2ScanMode = 0;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_Sleep (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_Sleep");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state == SONY_DEMOD_STATE_ACTIVE){
        if (pDemod->dvbss2ScanMode > 0){
            result = TSRStoSL (pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        } else if ((pDemod->system == SONY_DTV_SYSTEM_DVBS)
            || (pDemod->system == SONY_DTV_SYSTEM_DVBS2) || (pDemod->system == SONY_DTV_SYSTEM_ANY)){
            /* Active-S(X) -> Sleep-S */
            result = ASXtoSL (pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
        }
        pDemod->state = SONY_DEMOD_STATE_SLEEP;
    } else if (pDemod->state == SONY_DEMOD_STATE_SLEEP){
        /* Do nothing. */
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pDemod->dvbss2ScanMode = 0;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_CheckTSLock (sony_demod_t * pDemod,
                                              sony_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isLock = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_CheckTSLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat(pDemod, &isLock);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if(isLock){
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCKED;
    } else {
        *pLock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_CheckIQInvert (sony_demod_t * pDemod,
                                                uint8_t * pIsInvert)
{
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_CheckIQInvert");

    if ((!pDemod) || (!pIsInvert)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    *pIsInvert = 0;
    if (pDemod->isSinglecable){
        if (pDemod->isSinglecableIqInv){
            if (pDemod->satTunerIqSense == SONY_DEMOD_SAT_IQ_SENSE_NORMAL){
                *pIsInvert = 1;
            }
        } else {
            if (pDemod->satTunerIqSense == SONY_DEMOD_SAT_IQ_SENSE_INV){
                *pIsInvert = 1;
            }
        }
    } else {
        if (pDemod->satTunerIqSense == SONY_DEMOD_SAT_IQ_SENSE_INV){
            *pIsInvert = 1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_SetSymbolRate (sony_demod_t * pDemod,
                                                uint32_t symbolRateKSps)
{
    uint32_t regValue = 0;
    uint8_t data[3] = {0, 0, 0};
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_SetSymbolRate");

    if (!pDemod){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_ACTIVE) &&
        (pDemod->state != SONY_DEMOD_STATE_SLEEP)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /*
     * regValue = (symbolRateKSps * 2^14 / 1000) + 0.5
     *          = ((symbolRateKSps * 2^14) + 500) / 1000
     *          = ((symbolRateKSps * 16384) + 500) / 1000
     */
    regValue = ((symbolRateKSps * 16384) + 500) / 1000;
    if ((regValue == 0) || (regValue > 0x0FFFFF)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
    }
    data[0] = (uint8_t)((regValue >> 16) & 0x0F);
    data[1] = (uint8_t)((regValue >>  8) & 0xFF);
    data[2] = (uint8_t)( regValue        & 0xFF);

    /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
     * ----------------------------------------------------------------------------------
     * <SLV-T>    AEh     20h     [3:0]    8'h07      8'h0x        OSYMBOLRATE[19:16]
     * <SLV-T>    AEh     21h     [7:0]    8'h37      8'hxx        OSYMBOLRATE[15:8]
     * <SLV-T>    AEh     22h     [7:0]    8'h0A      8'hxx        OSYMBOLRATE[7:0]
     */
    /* Set SLV-T Bank : 0xAE */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAE) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t SLtoASX (sony_demod_t * pDemod,
                               sony_dtv_system_t dtvSystem,
                               uint32_t symbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("SLtoASX");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Configure TS clock Mode and Frequency. */
    /* NOTE: It is used for both DVB-S and S2 systems though only DVB-S is passed as a parameter. */
    result = sony_demod_SetTSClockModeAndFreq (pDemod, SONY_DTV_SYSTEM_DVBS);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set demod mode */
    if(dtvSystem == SONY_DTV_SYSTEM_DVBS){
        data = 0x0A;
    } else if(dtvSystem == SONY_DTV_SYSTEM_DVBS2){
        data = 0x0B;
    } else if(dtvSystem == SONY_DTV_SYSTEM_ANY){
        data = 0x0B;
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x17, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((dtvSystem == SONY_DTV_SYSTEM_DVBS) || (dtvSystem == SONY_DTV_SYSTEM_DVBS2)){
        /* DVB-S/S2 */
        data = 0x00;
    } else {
        /* ANY */
        data = 0x01;
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Enable S/S2 auto detection 1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2D, data) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set TS/TLV mode */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA9, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Enable demod clock */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x28, 0x31) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, 0x31) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* DVB-S/S2 initial setting
     *
     *   slave     Bank    Addr    Bit    Default     Setting       Signal name
     *  --------------------------------------------------------------------------------
     *   <SLV-T>    00h     20h     [0]     8'h00       8'h01        OREG_RFAGC_SRST_OFF
     *   <SLV-T>    18h     E3h     [1]     8'h00       8'h02        OREG_BBBYTESS_OLDMODE
     *   <SLV-T>    18h     E5h     [3:0]   8'h0F       8'h04        OREG_BSYNCDEPTH[3:0]
     */
    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x18 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x18) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE3, 0x02) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE5, 0x04) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*   slave     Bank    Addr    Bit    Default     Setting       Signal name
     *  --------------------------------------------------------------------------------
     *   <SLV-T>   A0h     D7h     [0]      8'h00      8'h00      OTUIF_POLAR
     */
    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OTUIF_POLAR[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set demod parameter */
    result = sony_demod_dvbs_s2_SetSymbolRate (pDemod, symbolRateKSps);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable HiZ Setting 1 (TAGC(Hi-Z), SAGC, TSVALID, TSSYNC, TSCLK) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x10, 0x1F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable Hi-Z setting 2 */
    result = sony_demod_SetTSDataPinHiZ (pDemod, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ASXtoSL (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("ASXtoSL");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable TS output */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Enable Hi-Z setting 1 (TAGC, SAGC, TSVALID, TSSYNC, TSCLK) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x1F, 0x1F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Enable Hi-Z setting 2 */
    result = sony_demod_SetTSDataPinHiZ (pDemod, 1);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* ---- */

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x11) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, 0x21) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* SADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x28, 0x13) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Demodulator SW reset */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFE, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable demodulator clock */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set tstlv mode to default */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA9, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable S/S2 auto detection */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2D, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set demodulator mode to default */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x17, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t ASXtoASX (sony_demod_t * pDemod, uint32_t symbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("ASXtoASX");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* disable TS output */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* set demod parameter */
    result = sony_demod_dvbs_s2_SetSymbolRate(pDemod, symbolRateKSps);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t TSRStoSL(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("TSRStoSL");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OTSDTDISABLE[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OREG_ARC_HSMODE[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xAE */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAE) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OFSM_SRSON[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OFSM_UNLOCKSEL_ARS[2:0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2E, 0x06) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OFSM_AUTOMODECHGON[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OREG_ARC_CKACHGAUTO[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2B, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OCFRL_CHSCANON[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x56, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xAB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OCFRL_CSBUFOFF[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF9, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xA3 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBD, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = ASXtoSL (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OTUIF_POLAR[0] */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
