/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
#include "sony_demod_dvbc_monitor.h"
#include "sony_math.h"
#include "sony_common.h"

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
/*
 @brief Confirms AR lock state for demodulator.
*/
static sony_result_t IsARLocked (sony_demod_t * pDemod);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbc_monitor_SyncStat (sony_demod_t * pDemod,
                                                uint8_t * pARLock,
                                                uint8_t * pTSLockStat,
                                                uint8_t * pUnlockDetected)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_SyncStat");

    if ((!pDemod) || (!pARLock) || (!pTSLockStat) || (!pUnlockDetected)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name                meaning
     * ---------------------------------------------------------------------------------
     * <SLV-T>   40h     88h     [0]          IREG_AR_LOCK        0:Unlock 1:Lock
     * <SLV-T>   40h     88h     [1]          IREG_AR_TIMEOUT     0:Checking, 1:DVB-C signal does not exist
     * <SLV-T>   40h     10h     [5]          IREG_TSIF_TS_LOCK   0:Unlock 1:Lock
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x88, &rdata, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pARLock = (uint8_t) ((rdata & 0x01) ? 1 : 0);
    *pUnlockDetected = (uint8_t) ((rdata & 0x02) ? 1 : 0);

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &rdata, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pTSLockStat = (uint8_t) ((rdata & 0x20) ? 1 : 0);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_IFAGCOut (sony_demod_t * pDemod, uint32_t * pIFAGCOut)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_IFAGCOut");

    if (!pDemod || !pIFAGCOut) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     49h     [3:0]       IIFAGC_OUT[11:8]
     * <SLV-T>   40h     4Ah     [7:0]       IIFAGC_OUT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x49, rdata, 2) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pIFAGCOut = (rdata[0] & 0x0F) << 8 | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_QAM (sony_demod_t * pDemod, sony_dvbc_constellation_t * pQAM)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_QAM");

    if (!pDemod || !pQAM) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*   slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     19h    [2:0]       IREG_QAM_LEVEL[2:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x19, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pQAM = (sony_dvbc_constellation_t) (rdata & 0x07);

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_SymbolRate (sony_demod_t * pDemod, uint32_t * pSymRate)
{
    uint8_t rdata1[2];
    uint8_t rdata2[5];
    uint32_t trialsrate;
    int32_t  tlfIntegral;
    uint32_t tsmDrate;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_SymbolRate");

    if (!pDemod || !pSymRate) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     1Ah    [3:0]       IREG_TRIALSRATE[11:8]
     * <SLV-T>   40h     1Bh    [7:0]       IREG_TRIALSRATE[7:0]
     * <SLV-T>   40h     9Bh    [7:0]       IREG_TLFINTEGRAL[31:24]
     * <SLV-T>   40h     9Ch    [7:0]       IREG_TLFINTEGRAL[23:16]
     * <SLV-T>   40h     9Dh    [7:0]       IREG_TLFINTEGRAL[15:8]
     * <SLV-T>   40h     9Eh    [7:0]       IREG_TLFINTEGRAL[7:0]
     * <SLV-T>   40h     9Fh    [1:0]       IREG_TSMDRATE[1:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1A, rdata1, sizeof(rdata1)) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Read IREG_TLFINTEGRAL and IREG_TSMDRATE */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9B, rdata2, sizeof(rdata2)) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    trialsrate = ((rdata1[0] & 0x0F) << 8) | rdata1[1];
    tlfIntegral = (int32_t)((rdata2[0] << 24) | (rdata2[1] << 16) | (rdata2[2] << 8) | rdata2[3]);
    tsmDrate = rdata2[4] & 0x03;

    /*
     * Symbol rate[MSymbol/sec] = (IREG_TRIALSRATE + (IREG_TLFINTEGRAL / ( 2^(24 + IREG_TSMDRATE)))) * ick / 16384
     * (IREG_TLFINTEGRAL is 2's compliment, ick = 48)
     *
     * Symbol rate[Symbol/sec] = (IREG_TRIALSRATE + (IREG_TLFINTEGRAL / ( 2^(24 + IREG_TSMDRATE)))) * 48 / 2^14 * 10^6
     *                         = (IREG_TRIALSRATE + (IREG_TLFINTEGRAL / ( 2^(24 + IREG_TSMDRATE)))) * 3 * 5^6 / 2^4
     *                         = IREG_TRIALSRATE * 3 * 5^6 / 16 + IREG_TLFINTEGRAL * 3 * 5^6 / ( 2^(28 + IREG_TSMDRATE))
     */
    /* Calculate IREG_TRIALSRATE * 3 * 5^6 / 16 */
    *pSymRate = (trialsrate * 46875 + 8) / 16;

    /* Calculate IREG_TLFINTEGRAL * 3 * 5^6 / ( 2^(28 + IREG_TSMDRATE)) */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        uint8_t isMinus = 0;

        if (tlfIntegral < 0) {
            isMinus = 1;
            tlfIntegral *= -1;
        }

        div = 65536; /* Divide by 2^16 first */

        /* Divide in 2 steps to prevent overflow. */
        Q = tlfIntegral / div;
        R = tlfIntegral % div;

        R *= 46875; /* 3 * 5^6 */
        Q = Q * 46875 + R / div;
        R = R % div;

        if (R >= div/2) {
            Q += 1;
        }

        /* Divide by 2^(12 + IREG_TSMDRATE) */
        Q = (Q + (1U << (11 + tsmDrate))) >> (12 + tsmDrate);

        if (isMinus) {
            *pSymRate -= Q;
        } else {
            *pSymRate += Q;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_CarrierOffset (sony_demod_t * pDemod, int32_t * pOffset)
{
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_CarrierOffset");

    if (!pDemod || !pOffset) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name
     * ---------------------------------------------------------------
     * <SLV-T>   40h     15h     [5:0]      IREG_FREQ_OFFSET[13:8]
     * <SLV-T>   40h     16h     [7:0]      IREG_FREQ_OFFSET[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x15, rdata, 2) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    /* 14bit signed value to 32bit signed value */
    *pOffset = sony_Convert2SComplement (((rdata[0] & 0x3F) << 8) | rdata[1], 14);

    /* Carrier offset[KHz] = IREG_FREQ_OFFSET * ADC_Clock(KHz) / 16384
     *
     * ADC_Clock(KHz) = 48000
     */
    *pOffset = *pOffset * 48000 / 16384;

    /* Compensate for inverted spectrum. */
    {
        /* Inverted RF spectrum with non-inverting tuner architecture. */
        if (pDemod->confSense == SONY_DEMOD_TERR_CABLE_SPECTRUM_INV) {
            *pOffset *= -1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_SpectrumSense (sony_demod_t * pDemod, sony_demod_terr_cable_spectrum_sense_t * pSense)
{
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_SpectrumSense");

    if (!pDemod || !pSense) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        sony_result_t result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave    Bank    Addr    Bit              Name                mean
     * ---------------------------------------------------------------------------------
     * <SLV-T>   40h     19h     [7]    IREG_DET_SPECTRUM_INV     0:Not inverted,   1:inverted
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x19, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    if (pDemod->confSense == SONY_DEMOD_TERR_CABLE_SPECTRUM_INV) {
        *pSense = (rdata & 0x80) ? SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL : SONY_DEMOD_TERR_CABLE_SPECTRUM_INV;
    } else {
        *pSense = (rdata & 0x80) ? SONY_DEMOD_TERR_CABLE_SPECTRUM_INV : SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_SNR (sony_demod_t * pDemod, int32_t * pSNR)
{
    sony_dvbc_constellation_t qam = SONY_DVBC_CONSTELLATION_16QAM;
    uint16_t reg = 0x00;
    uint8_t rdata[2];
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_SNR");

    if (!pDemod || !pSNR) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* AR lock check */
    {
        result = IsARLocked (pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*   slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     19h    [2:0]       IREG_QAM_LEVEL[2:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x19, &rdata[0], 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    qam = (sony_dvbc_constellation_t) (rdata[0] & 0x07);

    /*  slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     4Ch     [4:0]      IREG_SNR_ESTIMATE[12:8]
     * <SLV-T>   40h     4Dh     [7:0]      IREG_SNR_ESTIMATE[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4C, rdata, 2) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    reg = ((rdata[0] & 0x1F) << 8) | rdata[1];

    if (reg == 0) {
        /* log function will return INF */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    switch (qam) {
    case SONY_DVBC_CONSTELLATION_16QAM:
    case SONY_DVBC_CONSTELLATION_64QAM:
    case SONY_DVBC_CONSTELLATION_256QAM:
        /* IREG_SNR_ESTIMATE is clipped at a minimum of 126
         *
         * SNR(dB) = -9.50 * ln(IREG_SNR_ESTIMATE / (24320))
         */
        if (reg < 126) {
            reg = 126;
        }

        *pSNR = -95 * (int32_t) sony_math_log (reg) + 95941;
        break;
    case SONY_DVBC_CONSTELLATION_32QAM:
    case SONY_DVBC_CONSTELLATION_128QAM:
        /* IREG_SNR_ESTIMATE is clipped at a minimum of 69
         *
         * SNR(dB) = -8.75 * ln(IREG_SNR_ESTIMATE / (20800))
         */
        if (reg < 69) {
            reg = 69;
        }

        *pSNR = -88 * (int32_t) sony_math_log (reg) + 86999;
        break;
    default:
        /* Unknown value */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc_monitor_PreRSBER (sony_demod_t * pDemod, uint32_t * pBER)
{
    uint8_t rdata[3];
    uint32_t bitError = 0;
    uint32_t periodExp = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_PreRSBER");

    if (!pDemod || !pBER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    rdata[0] = rdata[1] = rdata[2] = 0;

    /*  slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     62h     [7]          IREG_BERN_VALID
     * <SLV-T>   40h     62h     [5:0]        IREG_BERN_BITECNT[21:16]
     * <SLV-T>   40h     63h     [7:0]        IREG_BERN_BITECNT[15:8]
     * <SLV-T>   40h     64h     [7:0]        IREG_BERN_BITECNT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x62, rdata, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Check IREG_BERN_VALID bit (bit 7) */
    if ((rdata[0] & 0x80) == 0) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
    }

    bitError = ((rdata[0] & 0x3F) << 16) | (rdata[1] << 8) | rdata[2];

    /*  slave    Bank    Addr    Bit    default          Name
     * -------------------------------------------------------------------------
     * <SLV-T>   40h     60h    [4:0]     5'h0B        OREG_BERN_PERIOD[4:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x60, rdata, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    periodExp = (rdata[0] & 0x1F);

    if ((periodExp <= 11) && (bitError > (1U << periodExp) * 204 * 8)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      BER = bitError * 10000000 / (2^N * 204 * 8)
          = bitError * 312500 / (2^N * 51)
          = bitError * 250 * 1250 / (2^N * 51)
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (periodExp <= 8) {
            div = (1U << periodExp) * 51;
        }
        else {
            div = (1U << 8) * 51;
        }

        Q = (bitError * 250) / div;
        R = (bitError * 250) % div;

        R *= 1250;
        Q = Q * 1250 + R / div;
        R = R % div;

        if (periodExp > 8) {
            /* rounding */
            *pBER = (Q + (1 << (periodExp - 9))) >> (periodExp - 8);
        }
        else {
            /* rounding */
            if (R >= div/2) {
                *pBER = Q + 1;
            } else {
                *pBER = Q;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_PacketErrorNumber (sony_demod_t * pDemod, uint32_t * pPEN)
{
    uint8_t rdata[3];

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_PacketErrorNumber");

    if (!pDemod || !pPEN) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x40 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name
     * ------------------------------------------------------------
     * <SLV-T>   40h     EAh     [7:0]        IREG_BER_CWRJCTCNT[15:8]
     * <SLV-T>   40h     EBh     [7:0]        IREG_BER_CWRJCTCNT[7:0]
     * <SLV-T>   40h     ECh     [0]          IREG_BER_CWRJCTCNT_VALID
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xEA, rdata, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (!(rdata[2] & 0x01)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    *pPEN = (rdata[0] << 8) | rdata[1];

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc_monitor_PER (sony_demod_t * pDemod, uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t packetError = 0;
    uint32_t periodExp = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_PER");

    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }
    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /*  slave    Bank    Addr    Bit     Name
     * ---------------------------------------------------------------------------------
     * <SLV-T>   40h     5Ch     [4:0]        OREG_PER_MES[4:0]
     * <SLV-T>   40h     5Dh     [2]          IREG_PER_VALID
     * <SLV-T>   40h     5Dh     [1:0]        IREG_PER_PKTERR[17:16]
     * <SLV-T>   40h     5Eh     [7:0]        IREG_PER_PKTERR[15:8]
     * <SLV-T>   40h     5Fh     [7:0]        IREG_PER_PKTERR[7:0]
     *
     * PER = IREG_PER_PKTERR / ( 2 ^ OREG_PER_MES )
     */

    {
        uint8_t rdata[4];

        /* Set SLV-T Bank : 0x40 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x40) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x5C, rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if ((rdata[1] & 0x04) == 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
        }

        packetError = ((rdata[1] & 0x03) << 16) | (rdata[2] << 8) | rdata[3];
        periodExp = rdata[0] & 0x1F;
    }

    if (packetError > (1U << periodExp)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      PER = packetError * 1000000 / 2^N
          = packetError * 5^6 / 2^(N-6) (If N > 6)

          = packetError * 10^6 / 2^N (If N <= 6, packetError <= 2^6)

      Max of packeError : 0x3FFFF
    */
    if (periodExp == 0) {
        *pPER = packetError * 1000000;
    } else if (periodExp > 6) {
        *pPER = (packetError * 15625 + (1 << (periodExp - 7))) >> (periodExp - 6);
    } else {
        *pPER = (packetError * 1000000 + (1 << (periodExp - 1))) >> periodExp;
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static functions
------------------------------------------------------------------------------*/
static sony_result_t IsARLocked (sony_demod_t * pDemod)
{
    uint8_t ar = 0;
    uint8_t tslock = 0;
    uint8_t earlyUnlock = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("IsARLocked");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbc_monitor_SyncStat (pDemod, &ar, &tslock,&earlyUnlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (ar != 1) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
