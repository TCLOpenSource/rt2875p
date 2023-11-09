/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/30
  Modification ID : 65a89e38a656dad732328ac1c2fb40d287b79851
------------------------------------------------------------------------------*/

#include "sony_demod_isdbs3.h"
#include "sony_demod_isdbs3_monitor.h"

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
/**
 @brief Configure the demodulator from Sleep to Active ISDB-S3.
*/
static sony_result_t SLtoAIS3 (sony_demod_t * pDemod);

/**
 @brief Configure the demodulator from Active ISDB-S3 to Sleep.
*/
static sony_result_t AIS3toSL (sony_demod_t * pDemod);

/**
 @brief Configure the demodulator from Active ISDB-S3 to Active ISDB-S3.
*/
static sony_result_t AIS3toAIS3 (sony_demod_t * pDemod);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_isdbs3_Tune (sony_demod_t * pDemod,
                                      sony_isdbs3_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state == SONY_DEMOD_STATE_ACTIVE) && (pDemod->system == SONY_DTV_SYSTEM_ISDBS3)) {
        /* Demodulator Active and set to ISDB-S3 mode */
        /* Set StreamID */
        result = sony_demod_isdbs3_SetStreamID (pDemod, pTuneParam->streamid);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = AIS3toAIS3 (pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }
    else if ((pDemod->state == SONY_DEMOD_STATE_ACTIVE) && (pDemod->system != SONY_DTV_SYSTEM_ISDBS3)) {
        /* Demodulator Active but not ISDB-S3 mode */
        result = sony_demod_Sleep (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pDemod->system = SONY_DTV_SYSTEM_ISDBS3;
        /* Set StreamID */
        result = sony_demod_isdbs3_SetStreamID (pDemod, pTuneParam->streamid);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = SLtoAIS3 (pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }
    else if (pDemod->state == SONY_DEMOD_STATE_SLEEP){
        /* Demodulator in Sleep mode */
        pDemod->system = SONY_DTV_SYSTEM_ISDBS3;
        /* Set StreamID */
        result = sony_demod_isdbs3_SetStreamID (pDemod, pTuneParam->streamid);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        result = SLtoAIS3 (pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pDemod->state = SONY_DEMOD_STATE_ACTIVE;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_Sleep (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_isdbs3_Sleep");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->state == SONY_DEMOD_STATE_ACTIVE) && (pDemod->system == SONY_DTV_SYSTEM_ISDBS3)) {
        result = AIS3toSL (pDemod);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
    }

    pDemod->state = SONY_DEMOD_STATE_SLEEP;
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_CheckTMCCLock (sony_demod_t * pDemod,
                                                sony_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t agcLock = 0;
    uint8_t tstlvLock = 0;
    uint8_t tmccLock = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs3_CheckTMCCLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbs3_monitor_SyncStat(pDemod, &agcLock, &tstlvLock, &tmccLock);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if(tmccLock){
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCKED;
    } else {
        *pLock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_CheckTSTLVLock (sony_demod_t * pDemod,
                                                sony_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t agcLock = 0;
    uint8_t tstlvLock = 0;
    uint8_t tmccLock = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs3_CheckTSTLVLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_isdbs3_monitor_SyncStat(pDemod, &agcLock, &tstlvLock, &tmccLock);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if(tstlvLock){
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCKED;
    } else {
        *pLock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_SetStreamID(sony_demod_t * pDemod, uint16_t stream_id)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2];

    SONY_TRACE_ENTER("sony_demod_isdbs3_SetStreamID");

    /* Null check */
    if(!pDemod){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     *    slave   Bank    Addr    Bit    Default     Setting     Signal name
     *  -------------------------------------------------------------------------
     *   <SLV-T>  D0h     87h     [7:0]    8'h00      8'h40      OSTREAM_ID[15:8]
     *   <SLV-T>  D0h     88h     [7:0]    8'h00      8'hF2      OSTREAM_ID[7:0]
     */
    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set StreamID */
    data[0] = (uint8_t)((stream_id >> 8) & 0xFF);
    data[1] = (uint8_t)(stream_id & 0xFF);

    if (pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, data, 2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t SLtoAIS3 (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("SLtoAIS3");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->isdbs3TLVOutput) {
        /* Configure TLV clock Mode and Frequency. */
        result = sony_demod_SetTLVClockModeAndFreq (pDemod, SONY_DTV_SYSTEM_ISDBS3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    } else
#endif
    {
        /* Configure TS clock Mode and Frequency. */
        result = sony_demod_SetTSClockModeAndFreq (pDemod, SONY_DTV_SYSTEM_ISDBS3);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set demod mode */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x17, 0x0D) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable S/S2 auto detection */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2D, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set TS/TLV mode */
    {
        uint8_t data = 0;

#ifdef SONY_DEMOD_SUPPORT_TLV
        if (pDemod->isdbs3TLVOutput) {
            data = 0x01;
        } else
#endif
        {
            data = 0x00;
        }

        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA9, data) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
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

    /* ISDB-S3 initial setting
     *
     *  slave     Bank    Addr    Bit    Default     Value         Signal name
     * --------------------------------------------------------------------------------
     *  <SLV-T>    00h     20h     [0]      8'h00     8'h01        OREG_RFAGC_SRST_OFF
     *  <SLV-T>    A3h     43h     [3:0]    8'h0A     8'h0B        OBBAGC_LPGAINBF[3:0]
     *  <SLV-T>    A3h     44h     [3:0]    8'h0A     8'h0B        OBBAGC_LPGAINAF[3:0]
     *  <SLV-T>    AEh     46h     [7:0]    8'h05     8'h04        OM_C_TH[15:8]
     *  <SLV-T>    AEh     47h     [7:0]    8'h78     8'h91        OM_C_TH[7:0]
     *  <SLV-T>    B6h     74h     [7:0]    8'h4A     8'h59        OM_K_A[15:8]
     *  <SLV-T>    D5h     61h     [7:0]    8'h38     8'h48        OH_K_A[7:0]
     *  <SLV-T>    D5h     62h     [7:0]    8'h6E     8'hFE        OH_K_C[7:0]
     *  <SLV-T>    D5h     63h     [7:0]    8'hCE     8'h4E        OH_K_F1[7:0]
     *  <SLV-T>    D5h     64h     [7:0]    8'hCE     8'h6E        OH_K_F2[7:0]
     *  <SLV-T>    D5h     65h     [7:0]    8'hAE     8'hFE        OH_B_A[7:0]
     *  <SLV-T>    D5h     67h     [7:0]    8'hFE     8'h4E        OH_B_F1[7:0]
     *  <SLV-T>    D5h     68h     [7:0]    8'hCE     8'h6E        OH_B_F2[7:0]
     *  <SLV-T>    D5h     90h     [7:0]    8'h08     8'h10        OH_T_A[7:0]
     *  <SLV-T>    D5h     91h     [7:0]    8'h02     8'h03        OH_T_C[7:0]
     *  <SLV-T>    D5h     92h     [7:0]    8'h12     8'h10        OH_T_F1[7:0]
     *  <SLV-T>    D6h     18h     [7:0]    8'h2E     8'h33        OL_C_TH[15:8]
     *  <SLV-T>    D6h     19h     [7:0]    8'hE0     8'hEB        OL_C_TH[7:0]
     *  <SLV-T>    D6h     1Ch     [7:0]    8'h5B     8'h59        OL_K_A[7:0]
     *  <SLV-T>    D6h     1Eh     [7:0]    8'hCE     8'h6E        OL_K_F1[7:0]
     *  <SLV-T>    D6h     21h     [7:0]    8'hFE     8'h4E        OL_B_F1[7:0]
     *  <SLV-T>    D6h     22h     [7:0]    8'hFF     8'h80        OL_T_A[7:0]
     *  <SLV-T>    D6h     23h     [7:0]    8'h95     8'h80        OL_T_C[7:0]
     *  <SLV-T>    D6h     24h     [7:0]    8'h7C     8'h10        OL_T_F1[7:0]
     *  <SLV-T>    D6h     51h     [7:0]    8'hB1     8'hB3        OCPRL_G1MAX_K_ARK2[3:0],OCPRL_G1MIN_K_ARK2[3:0]
     *  <SLV-T>    D6h     52h     [7:0]    8'hB1     8'hB3        OCPRL_G1MAX_B_ARK2[3:0],OCPRL_G1MIN_B_ARK2[3:0]
     *  <SLV-T>    D6h     53h     [7:0]    8'hC2     8'hC3        OCPRL_G1MAX_D_ARK2[3:0],OCPRL_G1MIN_D_ARK2[3:0]
     *  <SLV-T>    DAh     BCh     [1:0]    8'h00     8'h02        OTMCCCHGTRK[1:0]
     *  <SLV-T>    DAh     D6h     [7:4]    8'h90     8'h60        OTSLK_GOODIDX_B_ARK2[3:0]
     *  <SLV-T>    DAh     D7h     [7:4]    8'h90     8'h70        OTSLK_GOODIDX_Q_ARK2[3:0]
     */
    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xA3 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x0B, 0x0B};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0xAE */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAE) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x04, 0x91};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x46, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0xB6 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xB6) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x74, 0x59) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xD5 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD5) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x48, 0xFE, 0x4E, 0x6E, 0xFE};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x61, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    {
        const uint8_t data[] = {0x4E, 0x6E};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x67, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    {
        const uint8_t data[] = {0x10, 0x03, 0x10};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x90, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0xD6 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD6) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x33, 0xEB};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x18, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1C, 0x59) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1E, 0x6E) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x4E, 0x80, 0x80, 0x10};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x21, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    {
        const uint8_t data[] = {0xB3, 0xB3, 0xC3};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x51, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0xDA */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xDA) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBC, 0x02) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x60, 0x70};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD6, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* ISDB-S3 symbol rate setting
     *  slave     Bank    Addr    Bit    Default     Value         Signal name
     * --------------------------------------------------------------------------------
     *  <SLV-T>    AEh     20h     [3:0]    8'h07      8'h08        OSYMBOLRATE[19:16]
     *  <SLV-T>    AEh     21h     [7:0]    8'h37      8'h70        OSYMBOLRATE[15:8]
     *  <SLV-T>    AEh     22h     [7:0]    8'h0A      8'h64        OSYMBOLRATE[7:0]
     */
    /* Set SLV-T Bank : 0xAE */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAE) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x08, 0x70, 0x64};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* TLV error packet setting
     *  slave    Bank    Addr    Bit    default   Value          Name
     * ---------------------------------------------------------------------------------
     *  <SLV-T>   DAh     CBh    [0]     8'h00      8'h01        OERRPKT_ON
     *  <SLV-T>   DAh     C1h    [0]     8'h00      8'h01        OLENDIVERRPKT
     */
    /* Set SLV-T Bank : 0xDA */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xDA) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xCB, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC1, 0x01) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  IQ Spectrum inversion
     *   slave     Bank    Addr    Bit    Default     Setting       Signal name
     *  --------------------------------------------------------------------------------
     *   <SLV-T>   A0h     D7h     [0]      8'h00      8'h01      OTUIF_POLAR
     */
    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7,
        (uint8_t)((pDemod->satTunerIqSense == SONY_DEMOD_SAT_IQ_SENSE_INV)? 0x01 : 0x00)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable HiZ Setting 1 (TAGC(Hi-Z), SAGC, TSVALID, TSSYNC, TSCLK) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x10, 0x1F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* disable Hi-Z setting 2 */
    result = sony_demod_SetTSDataPinHiZ (pDemod, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t AIS3toSL (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("AIS3toSL");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->isdbs3TLVOutput) {
        /* Set SLV-T Bank : 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TLV output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC0, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else
#endif
    {
        /* Set SLV-T Bank : 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TS output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
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

    /* ISDB-S3 Cancel Setting
     *  slave     Bank    Addr    Bit    Default     Value         Signal name
     * --------------------------------------------------------------------------------
     *  <SLV-T>  A3h     43h     [3:0]    8'h0A     8'h0A      OBBAGC_LPGAINBF[3:0]
     *  <SLV-T>  A3h     44h     [3:0]    8'h0A     8'h0A      OBBAGC_LPGAINAF[3:0]
     */
    /* Set SLV-T Bank : 0xA3 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = {0x0A, 0x0A};

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x43, data, sizeof (data)) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
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

    /* ---- */

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

static sony_result_t AIS3toAIS3 (sony_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("AIS3toAIS3");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->isdbs3TLVOutput) {
        /* Set SLV-T Bank : 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TLV output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC0, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else
#endif
    {
        /* Set SLV-T Bank : 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TS output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

