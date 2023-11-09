/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_demod.h"
#include "sony_demod_dvbc2.h"
#include "sony_demod_dvbc2_monitor.h"
#include "sony_math.h"

/*------------------------------------------------------------------------------
    Static Function Prototypes
------------------------------------------------------------------------------*/
/*
 @brief Confirm that the current L1 Data is valid.
*/
static sony_result_t CheckL1DataReady (sony_demod_t * pDemod);

/*
 @brief Check that a common PLP is available
*/
static sony_result_t CheckCommonPLPEnable (sony_demod_t * pDemod);

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbc2_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                 uint32_t * pIFAGC)
{
    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGC)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit              Name
     * --------------------------------------------------------
     * <SLV-T>   20h     26h     [3:0]        IIFAGC_OUT[11:8]
     * <SLV-T>   20h     27h     [7:0]        IIFAGC_OUT[7:0]
     */
    {
        uint8_t data[2];
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x26, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pIFAGC = ((data[0] & 0x0F) << 8) | (data[1]);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_SpectrumSense (sony_demod_t * pDemod,
                                                      sony_demod_terr_cable_spectrum_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t syncState   = 0;
    uint8_t tsLock      = 0;
    uint8_t earlyUnlock = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_SpectrumSense");

    if ((!pDemod) || (!pSense)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze the register. */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyUnlock);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Only valid after lock. */
    if (syncState != 6) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    {
        uint8_t data = 0;

        /* Set SLV-T Bank : 0x28 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x28) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name                mean
         * ---------------------------------------------------------------------------------
         * <SLV-T>   28h     E6h     [0]        IREG_CRCG2_SINVP1      0:not invert,   1:invert
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE6, &data, sizeof (data)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        SLVT_UnFreezeReg (pDemod);

        if (pDemod->confSense == SONY_DEMOD_TERR_CABLE_SPECTRUM_INV) {
            *pSense = (data & 0x01) ? SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL : SONY_DEMOD_TERR_CABLE_SPECTRUM_INV;
        } else {
            *pSense = (data & 0x01) ? SONY_DEMOD_TERR_CABLE_SPECTRUM_INV : SONY_DEMOD_TERR_CABLE_SPECTRUM_NORMAL;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                      int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_CarrierOffset");

    if ((!pDemod) || (!pOffset)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[4];
        uint32_t ctlVal = 0;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (syncState != 6) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ---------------------------------------------------------------
         * <SLV-T>   20h     4Ch     [3:0]      IREG_CRCG_CTLVAL[27:24]
         * <SLV-T>   20h     4Dh     [7:0]      IREG_CRCG_CTLVAL[23:16]
         * <SLV-T>   20h     4Eh     [7:0]      IREG_CRCG_CTLVAL[15:8]
         * <SLV-T>   20h     4Fh     [7:0]      IREG_CRCG_CTLVAL[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4C, data, sizeof (data)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        SLVT_UnFreezeReg (pDemod);
        /*
         *    Carrier Offset [Hz] = -(IREG_CRCG_CTLVAL * (2^-30) * 8 * BW) / (7 *10^-6)
         *
         *    Note: (2^-30 * 8) / (7 * 10^-6) = 1.064368657 * 10^-3
         *    And: 1 / ((2^-30 * 8) / (7 * 10^-6)) = 939.52
         */
        ctlVal = ((data[0] & 0x0F) << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
        *pOffset = sony_Convert2SComplement (ctlVal, 28);

        if((SONY_DTV_BW_6_MHZ == pDemod->bandwidth) || (SONY_DTV_BW_8_MHZ == pDemod->bandwidth )) {
            *pOffset = -1 * ((*pOffset) * (uint8_t)pDemod->bandwidth / 940);
        }
        else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }

    /* Compensate for inverted spectrum. */
    {
        /* Inverted RF spectrum with non-inverting tuner architecture. */
        if (pDemod->confSense == SONY_DEMOD_TERR_CABLE_SPECTRUM_INV) {
            *pOffset *= -1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_SNR (sony_demod_t * pDemod,
                                            int32_t * pSNR)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_SNR");

    if ((!pDemod) || (!pSNR)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint16_t reg;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        uint8_t data[2];

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Confirm sequence state == 6. */
        if (syncState != 6) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     28h     [7:0]        IREG_SNMON_OD[15:8]
         * <SLV-T>   20h     29h     [7:0]        IREG_SNMON_OD[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x28, data, sizeof (data)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        SLVT_UnFreezeReg (pDemod);

        reg = ((data[0] & 0xFF) << 8) | data[1];

        /* Confirm valid range, clip as necessary */
        if (reg == 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* IREG_SNMON_OD is clipped at a maximum of 51441.
         *
         * SNR = 10 * log10 (IREG_SNMON_OD / (55000 - IREG_SNMON_OD)) + 38.4
         *     = 10 * (log10(IREG_SNMON_OD) - log10(55000 - IREG_SNMON_OD)) + 38.4
         * sony_log10 returns log10(x) * 100
         * Therefore SNR(dB) * 1000 :
         *     = 10 * 10 * (sony_log10(IREG_SNMON_OD) - sony_log10(55000 - IREG_SNMON_OD) + 38400
         */
        if (reg > 51441) {
            reg = 51441;
        }

        *pSNR = 10 * 10 * ((int32_t) sony_math_log10 (reg) - (int32_t) sony_math_log10 (55000 - reg));
        *pSNR += 38400;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                   uint32_t * pBER)
{
    uint32_t bitError;
    uint32_t n_ldpc = 0;
    uint32_t period;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_PreLDPCBER");

    if ((!pDemod) || (!pBER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[4];

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     39h     [4]          ILBER_VALID
         * <SLV-T>   20h     39h     [3:0]        IBER1_BITERR[27:24]
         * <SLV-T>   20h     3Ah     [7:0]        IBER1_BITERR[23:16]
         * <SLV-T>   20h     3Bh     [7:0]        IBER1_BITERR[15:8]
         * <SLV-T>   20h     3Ch     [7:0]        IBER1_BITERR[7:0]
         */

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set SLV-T Bank : = 0x20. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x39, data, sizeof (data)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(data[0] & 0x10)) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        bitError = ((data[0] & 0x0F) << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     6Fh     [3:0]        OREG_LBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6F, data, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        period = data[0] & 0x0F;

        /* Set SLV-T Bank : = 0x52. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   52h     15h     [4]          IREG_C2_FEC_TYPE0
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x15, &data[0], 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        SLVT_UnFreezeReg (pDemod);

        if ((data[0] & 0x10) == 0x00) {
            /* Short. */
            n_ldpc = 16200;
        }
        else {
            /* Normal. */
            n_ldpc = 64800;
        }
    }

    if (bitError > ((1U << period) * n_ldpc)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* BER = IBER1_BITERR / ( 2^OREG_LBER_MES * N_ldpc )

             IREG_C2_FEC_TYPE0    |  N_ldpc
          ------------------------+----------
               1'b0(short)        |  16200
               1'b1(normal)       |  64800
          ------------------------+----------
    */

    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (period >= 4) {
            /*
              BER = bitError * 10000000 / (2^N * n_ldpc)
                  = bitError * 3125 / (2^(N-4) * (n_ldpc / 200))
                  (NOTE: 10000000 / 2^4 / 200 = 3125)
                  = bitError * 5 * 625 / (2^(N-4) * (n_ldpc / 200))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << (period - 4)) * (n_ldpc / 200);

            Q = (bitError * 5) / div;
            R = (bitError * 5) % div;

            R *= 625;
            Q = Q * 625 + R / div;
            R = R % div;
        }
        else {
            /*
              BER = bitError * 10000000 / (2^N * n_ldpc)
                  = bitError * 50000 / (2^N * (n_ldpc / 200))
                  = bitError * 10 * 5000 / (2^N * (n_ldpc / 200))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << period) * (n_ldpc / 200);

            Q = (bitError * 10) / div;
            R = (bitError * 10) % div;

            R *= 5000;
            Q = Q * 5000 + R / div;
            R = R % div;
        }

        /* rounding */
        if (R >= div/2) {
            *pBER = Q + 1;
        }
        else {
            *pBER = Q;
        }
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                  uint32_t * pBER)
{
    uint32_t bitError;
    uint32_t n_bch;
    uint32_t period;
    sony_result_t result = SONY_RESULT_OK;
    static const uint16_t nBCHBitsLookup[2][6] = {
      /* RSVD   R2_3   R3_4   R4_5   R5_6   R8_9 */
        {0,     10800, 11880, 12600, 13320, 14400}, /* Short */
        {0,     43200, 48600, 51840, 54000, 58320}  /* Normal */
    };

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_PreBCHBER");

    if ((!pDemod) || (!pBER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[4];
        uint8_t fecType;
        uint8_t codeRate;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set SLV-T Bank : = 0x24. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x24) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   24h     40h     [0]          IBBER_VALID
         * <SLV-T>   24h     41h     [5:0]        IBER0_BITERR[21:16]
         * <SLV-T>   24h     42h     [7:0]        IBER0_BITERR[15:8]
         * <SLV-T>   24h     43h     [7:0]        IBER0_BITERR[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x40, data, sizeof (data)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* The registers are valid when IBBER_VALID == 1 */
        if (!(data[0] & 0x01)) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        bitError = ((data[1] & 0x3F) << 16) | (data[2] << 8) | data[3];

        /* Set SLV-T Bank : = 0x20. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     72h    [3:0]        OREG_BBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x72, data, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        period = data[0] & 0x0F;

        /* Set SLV-T Bank : = 0x52. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   52h     15h     [4]          IREG_C2_FEC_TYPE0
         * <SLV-T>   52h     16h     [2:0]        IREG_C2_COD0[2:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x15, &data[0], 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        SLVT_UnFreezeReg (pDemod);

        fecType = (data[0] & 0x10) >> 4;
        codeRate = data[1] & 0x07;

        if (codeRate > 5) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        n_bch = nBCHBitsLookup[fecType][codeRate];
    }

    /* Protect against /0. */
    if ((n_bch == 0) || (bitError > ((1 << period) * n_bch))) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* BER = IBER0_BITERR / ( 2^OREG_BBER_MES * N_bch ) */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (period >= 6) {
            /*
              BER = bitError * 1000000000 / (2^N * n_bch)
                  = bitError * 390625 / (2^(N-6) * (n_bch / 40))
                  (NOTE: 1000000000 / 2^6 / 40 = 390625)
                  = bitError * 625 * 625 / (2^(N-6) * (n_bch / 40))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << (period - 6)) * (n_bch / 40);

            Q = (bitError * 625) / div;
            R = (bitError * 625) % div;

            R *= 625;
            Q = Q * 625 + R / div;
            R = R % div;
        }
        else {
            /*
              BER = bitError * 1000000000 / (2^N * n_bch)
                  = bitError * 25000000 / (2^N * (n_bch / 40))
                  = bitError * 1000 * 25000 / (2^N * (n_bch / 40))
                  (Divide in 2 steps to prevent overflow.)
            */
            div = (1U << period) * (n_bch / 40);

            Q = (bitError * 1000) / div;
            R = (bitError * 1000) % div;

            R *= 25000;
            Q = Q * 25000 + R / div;
            R = R % div;
        }

        /* rounding */
        if (R >= div/2) {
            *pBER = Q + 1;
        }
        else {
            *pBER = Q;
        }
    }
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                   uint32_t * pFER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t fecError = 0;
    uint32_t period = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_PostBCHFER");

    if ((!pDemod) || (!pFER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data[3];
        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     75h     [0]           IBBER_VALID
         * <SLV-T>   20h     76h     [6:0]         IBER2_FBERR[14:8]
         * <SLV-T>   20h     77h     [7:0]         IBER2_FBERR[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x75, data, 3) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (!(data[0] & 0x01)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        fecError = ((data[1] & 0x7F) << 8) | (data[2]);

        /* Read measurement period.
         * slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   20h     72h     [3:0]        OREG_BBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x72, data, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Period = 2^BBER_MES */
        period = (1 << (data[0] & 0x0F));
    }

    if ((period == 0) || (fecError > period)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      FER = fecError * 1000000 / period
          = fecError * 1000 * 1000 / period
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = period;

        Q = (fecError * 1000) / div;
        R = (fecError * 1000) % div;

        R *= 1000;
        Q = Q * 1000 + R / div;
        R = R % div;

        /* rounding */
        if ((div != 1) && (R >= div/2)) {
            *pFER = Q + 1;
        }
        else {
            *pFER = Q;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_PacketErrorNumber (sony_demod_t * pDemod,
                                                          uint32_t * pPEN)
{
    sony_result_t result = SONY_RESULT_OK;

    uint8_t data[3];

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_PacketErrorNumber");

    if ((!pDemod) || (!pPEN)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x24 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x24) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

   /* slave    Bank    Addr    Bit              Name
    * ------------------------------------------------------------
    * <SLV-T>   24h     FDh     [0]          IREG_SP_BERT_VALID
    * <SLV-T>   24h     FEh     [7:0]        IREG_SP_BERT_CWRJCTCNT[15:8]
    * <SLV-T>   24h     FFh     [7:0]        IREG_SP_BERT_CWRJCTCNT[7:0]
    */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFD, data, sizeof (data)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (!(data[0] & 0x01) ) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    *pPEN =  ((data[1] << 0x08) | data[2]);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_TSRate (sony_demod_t * pDemod,
                                               uint32_t * pTSRateKbps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t rd_smooth_dp = 0;
    uint32_t ep_ck_nume = 0;
    uint32_t ep_ck_deno = 0;
    uint8_t issy_on_data = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_TSRate");

    if ((!pDemod) || (!pTSRateKbps)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t data[8];
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Check TS lock. */
        if (!tsLock) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------------------
         * <SLV-T>   20h     E5h     [4:0]        IREG_SP_RD_SMOOTH_DP[28:24]
         * <SLV-T>   20h     E6h     [7:0]        IREG_SP_RD_SMOOTH_DP[23:16]
         * <SLV-T>   20h     E7h     [7:0]        IREG_SP_RD_SMOOTH_DP[15:8]
         * <SLV-T>   20h     E8h     [7:0]        IREG_SP_RD_SMOOTH_DP[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE5, &data[0], 4) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        rd_smooth_dp = (uint32_t) ((data[0] & 0x1F) << 24);
        rd_smooth_dp |= (uint32_t) (data[1] << 16);
        rd_smooth_dp |= (uint32_t) (data[2] << 8);
        rd_smooth_dp |= (uint32_t) data[3];

        /* rd_smooth_dp should be > 214958 = < 100Mbps */
        if (rd_smooth_dp < 214958) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set SLV-T Bank : 0x24 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x24) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------------------
         * <SLV-T>   24h     90h     [0]          IREG_SP_ISSY_ON_DATA
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x90, &data[0], 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        issy_on_data = data[0] & 0x01;

        /* Set SLV-T Bank : 0x25 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x25) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------------------
         * <SLV-T>   25h     B2h     [5:0]        IREG_SP_EP_CK_NUME[29:24]
         * <SLV-T>   25h     B3h     [7:0]        IREG_SP_EP_CK_NUME[23:16]
         * <SLV-T>   25h     B4h     [7:0]        IREG_SP_EP_CK_NUME[15:8]
         * <SLV-T>   25h     B5h     [7:0]        IREG_SP_EP_CK_NUME[7:0]
         * <SLV-T>   25h     B6h     [5:0]        IREG_SP_EP_CK_DENO[29:24]
         * <SLV-T>   25h     B7h     [7:0]        IREG_SP_EP_CK_DENO[23:16]
         * <SLV-T>   25h     B8h     [7:0]        IREG_SP_EP_CK_DENO[15:8]
         * <SLV-T>   25h     B9h     [7:0]        IREG_SP_EP_CK_DENO[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB2, &data[0], 8) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        ep_ck_nume = (uint32_t) ((data[0] & 0x3F) << 24);
        ep_ck_nume |= (uint32_t) (data[1] << 16);
        ep_ck_nume |= (uint32_t) (data[2] << 8);
        ep_ck_nume |= (uint32_t) data[3];

        ep_ck_deno = (uint32_t) ((data[4] & 0x3F) << 24);
        ep_ck_deno |= (uint32_t) (data[5] << 16);
        ep_ck_deno |= (uint32_t) (data[6] << 8);
        ep_ck_deno |= (uint32_t) data[7];

        /* Unfreeze registers */
        SLVT_UnFreezeReg (pDemod);
    }

    if (issy_on_data) {
        if ((ep_ck_deno == 0) || (ep_ck_nume == 0) || (ep_ck_deno >= ep_ck_nume)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
    }

    /* IREG_SP_ISSY_ON_DATA    TS Data output rate[Mbps]
     * ------------------------------------------------------------------------------------------------------------
     *          0              ick * 2^18 / IREG_SP_RD_SMOOTH_DP
     *          1              (ick * 2^18 / IREG_SP_RD_SMOOTH_DP ) * ( IREG_SP_EP_CK_DENO / IREG_SP_EP_CK_NUME )
     *
     * ick = 96
     *
     * TSRate = ick * 2^18 / IREG_SP_RD_SMOOTH_DP (Mbps)
     *        = ick * 2^18 * 10^3 / IREG_SP_RD_SMOOTH_DP (kbps)
     */
    {
        /* Calculate ick * 2^18 * 10^3 / IREG_SP_RD_SMOOTH_DP */
        /* Divide in 3 steps to prevent overflow */
        uint32_t ick = 96;
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = rd_smooth_dp;

        Q = ick * 262144U * 100U / div;
        R = ick * 262144U * 100U % div;

        R *= 5U;
        Q = Q * 5 + R / div;
        R = R % div;

        R *= 2U;
        Q = Q * 2 + R / div;
        R = R % div;

        /* Round up based on the remainder */
        if (R >= div/2) {
            *pTSRateKbps = Q + 1;
        }
        else {
            *pTSRateKbps = Q;
        }
    }

    if (issy_on_data) {
        /* TSRate *= DENO / NUME
           (NOTE: 0.88 <= (DENO / NUME) < 1.0)
           TSRate *= 1 - (NUME - DENO) / NUME
           TSRate -= TSRate * (NUME - DENO) / NUME

           Calculating
           TSRate * (NUME - DENO) / NUME
            = TSRate * (((NUME - DENO) >> N) / (NUME >> N)) (To avoid overflow) */

        uint32_t diff = ep_ck_nume - ep_ck_deno;

        /* TSRate < 100000 (100Mbps), 100000 * 0x7FFF = 0xC34E7960 (32bit) */
        while(diff > 0x7FFF){
            diff >>= 1;
            ep_ck_nume >>= 1;
        }

        *pTSRateKbps -= (*pTSRateKbps * diff + ep_ck_nume/2) / ep_ck_nume;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_SyncStat (sony_demod_t * pDemod,
                                                 uint8_t * pSyncStat,
                                                 uint8_t * pTSLockStat,
                                                 uint8_t * pUnlockDetected)
{
    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_SyncStat");

    if ((!pDemod) || (!pSyncStat) || (!pTSLockStat) || (!pUnlockDetected)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        uint8_t data;
        /* slave    Bank    Addr    Bit              Name                Meaning
         * -------------------------------------------------------------------
         * <SLV-T>   20h     10h     [2:0]        IREG_SEQ_OSTATE     0-5:UNLOCK 6:LOCK
         * <SLV-T>   20h     10h     [4]          IEARLY_NOOFDM       1: No OFDM
         * <SLV-T>   20h     10h     [5]          IREG_SP_TSLOCK      0:UNLOCK 1:LOCK
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pSyncStat = data & 0x07;
        *pTSLockStat = ((data & 0x20) ? 1 : 0);
        *pUnlockDetected = ((data & 0x10) ? 1 : 0);
    }

    /* Check for valid SyncStat value. */
    if (*pSyncStat == 0x07){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_L1Data (sony_demod_t * pDemod,
                                               sony_dvbc2_l1_t * l1Data)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[123];

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_L1Data");

    if ((!pDemod) || (!l1Data)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = CheckL1DataReady(pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        else {
            /*
             * The registers show L1 part2 information.
             *
             * slave    Bank    Addr    Bit              Name
             * ------------------------------------------------------------------------
             * <SLV-T>   52h     20h     [7:0]        IREG_NETWORK_ID[15:8]
             * <SLV-T>   52h     21h     [7:0]        IREG_NETWORK_ID[7:0]
             * <SLV-T>   52h     22h     [7:0]        IREG_C2_SYSTEM_ID[15:8]
             * <SLV-T>   52h     23h     [7:0]        IREG_C2_SYSTEM_ID[7:0]
             * <SLV-T>   52h     24h     [7:0]        IREG_START_FREQUENCY[23:16]
             * <SLV-T>   52h     25h     [7:0]        IREG_START_FREQUENCY[15:8]
             * <SLV-T>   52h     26h     [7:0]        IREG_START_FREQUENCY[7:0]
             * <SLV-T>   52h     27h     [7:0]        IREG_C2_BANDWIDTH[15:8]
             * <SLV-T>   52h     28h     [7:0]        IREG_C2_BANDWIDTH[7:0]
             * <SLV-T>   52h     29h     [1:0]        IREG_GUARD_INTERVAL[1:0]
             * <SLV-T>   52h     2Ah     [1:0]        IREG_C2_FRAME_LENGTH[9:8]
             * <SLV-T>   52h     2Bh     [7:0]        IREG_C2_FRAME_LENGTH[7:0]
             * <SLV-T>   52h     2Ch     [7:0]        IREG_L1_PART2_CHANGE_COUNTER[7:
             * <SLV-T>   52h     2Dh     [7:0]        IREG_NUM_DSLICE[7:0]
             * <SLV-T>   52h     2Eh     [3:0]        IREG_NUM_NOTCH[3:0]
             * <SLV-T>   52h     2Fh     [7:0]        IREG_DSLICE_ID[7:0]
             * <SLV-T>   52h     30h     [5:0]        IREG_DSLICE_TUNE_POS[13:8]
             * <SLV-T>   52h     31h     [7:0]        IREG_DSLICE_TUNE_POS[7:0]
             * <SLV-T>   52h     32h     [0]          IREG_DSLICE_OFFSET_LEFT[8]
             * <SLV-T>   52h     33h     [7:0]        IREG_DSLICE_OFFSET_LEFT[7:0]
             * <SLV-T>   52h     34h     [0]          IREG_DSLICE_OFFSET_RIGHT[8]
             * <SLV-T>   52h     35h     [7:0]        IREG_DSLICE_OFFSET_RIGHT[7:0]
             * <SLV-T>   52h     36h     [1:0]        IREG_DSLICE_TI_DEPTH[1:0]
             * <SLV-T>   52h     37h     [0]          IREG_DSLICE_TYPE
             * <SLV-T>   52h     38h     [0]          IREG_FEC_HEADER_TYPE
             * <SLV-T>   52h     39h     [0]          IREG_DSLICE_CONST_CONF
             * <SLV-T>   52h     3Ah     [0]          IREG_DSLICE_LEFT_NOTCH
             * <SLV-T>   52h     3Bh     [7:0]        IREG_DSLICE_NUM_PLP[7:0]
             * <SLV-T>   52h     3Ch     [7:0]        IREG_PLP_ID[7:0]
             * <SLV-T>   52h     3Dh     [0]          IREG_PLP_BUNDLED
             * <SLV-T>   52h     3Eh     [1:0]        IREG_PLP_TYPE[1:0]
             * <SLV-T>   52h     3Fh     [4:0]        IREG_PLP_PAYLOAD_TYPE[4:0]
             * <SLV-T>   52h     40h     [7:0]        IREG_PLP_GROUP_ID[7:0]
             * <SLV-T>   52h     41h     [5:0]        IREG_PLP_START[13:8]
             * <SLV-T>   52h     42h     [7:0]        IREG_PLP_START[7:0]
             * <SLV-T>   52h     43h     [0]          IREG_PLP_FEC_TYPE (**)
             * <SLV-T>   52h     44h     [2:0]        IREG_PLP_MOD[2:0] (**)
             * <SLV-T>   52h     45h     [2:0]        IREG_PLP_COD[2:0] (**)
             * <SLV-T>   52h     46h     [0]          IREG_PSISI_REPROCESSING
             * <SLV-T>   52h     47h     [7:0]        IREG_TRANSPORT_STREAM_ID[15:8]
             * <SLV-T>   52h     48h     [7:0]        IREG_TRANSPORT_STREAM_ID[7:0]
             * <SLV-T>   52h     49h     [7:0]        IREG_ORIGINAL_NETWORK_ID[15:8]
             * <SLV-T>   52h     4Ah     [7:0]        IREG_ORIGINAL_NETWORK_ID[7:0]
             * <SLV-T>   52h     4Bh     [7:0]        IREG_RESERVED_1[7:0]
             * <SLV-T>   52h     4Ch     [7:0]        IREG_RESERVED_2[7:0]
             *
             * <SLV-T>   52h     4Dh     [5:0]        IREG_NOTCH_START_0[13:8]
             * <SLV-T>   52h     4Eh     [7:0]        IREG_NOTCH_START_0[7:0]
             * <SLV-T>   52h     4Fh     [0]          IREG_NOTCH_WIDTH_0[8]
             * <SLV-T>   52h     50h     [7:0]        IREG_NOTCH_WIDTH_0[7:0]
             * <SLV-T>   52h     51h     [7:0]        IREG_RESERVED_3_0[7:0]
             *
             * <SLV-T>   52h     52h     [5:0]        IREG_NOTCH_START_1[13:8]
             * <SLV-T>   52h     53h     [7:0]        IREG_NOTCH_START_1[7:0]
             * <SLV-T>   52h     54h     [0]          IREG_NOTCH_WIDTH_1[8]
             * <SLV-T>   52h     55h     [7:0]        IREG_NOTCH_WIDTH_1[7:0]
             * <SLV-T>   52h     56h     [7:0]        IREG_RESERVED_3_1[7:0]
             *
             *                    (snip)
             *
             * <SLV-T>   52h     93h     [5:0]        IREG_NOTCH_START_14[13:8]
             * <SLV-T>   52h     94h     [7:0]        IREG_NOTCH_START_14[7:0]
             * <SLV-T>   52h     95h     [0]          IREG_NOTCH_WIDTH_14[8]
             * <SLV-T>   52h     96h     [7:0]        IREG_NOTCH_WIDTH_14[7:0]
             * <SLV-T>   52h     97h     [7:0]        IREG_RESERVED_3_14[7:0]
             *
             * <SLV-T>   52h     98h     [0]          IREG_RESERVED_TONE
             * <SLV-T>   52h     99h     [7:0]        IREG_RESERVED_4[15:8]
             * <SLV-T>   52h     9Ah     [7:0]        IREG_RESERVED_4[7:0]
             */

            /* Set SLV-T Bank : 0x52. */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
            if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, rdata, sizeof (rdata)) != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            SLVT_UnFreezeReg (pDemod);

            l1Data->networkID = (((uint16_t) rdata[0] << 8) | rdata[1]);
            l1Data->systemID = (((uint16_t) rdata[2] << 8) | rdata[3]);
            l1Data->startFrequency = (((uint32_t) rdata[4] << 16) | (rdata[5] << 8) | rdata[6]);
            l1Data->bandwidth = (((uint16_t) rdata[7] << 8) | rdata[8]);
            l1Data->gi = (sony_dvbc2_l1_gi_t) (rdata[9] & 0x03);
            l1Data->frameLength = (((uint16_t) rdata[10] << 8) | rdata[11]);
            l1Data->changeCntL1 = rdata[12];
            l1Data->numDs = rdata[13];
            l1Data->numNotchs = (rdata[14] & 0x0F);
            l1Data->dsID = rdata[15];
            l1Data->dsTunePos = (((uint16_t) (rdata[16] & 0x3F) << 8) | rdata[17]);
            l1Data->dsOffsetOnLeft  = (int16_t)sony_Convert2SComplement(((rdata[18] & 0x01) << 8) | rdata[19],9);
            l1Data->dsOffsetOnRight = (int16_t)sony_Convert2SComplement(((rdata[20] & 0x01) << 8) | rdata[21],9);
            l1Data->dsTiDepth = (sony_dvbc2_ds_ti_depth_t)(rdata[22] & 0x03);
            l1Data->dsType = (sony_dvbc2_ds_type_t)(rdata[23] & 0x01);
            l1Data->fecHdrType = (sony_dvbc2_fec_header_type_t)(rdata[24] & 0x01);
            l1Data->dsConstConf = (rdata[25] & 0x01);
            l1Data->dsLeftNotch = (rdata[26] & 0x01);
            l1Data->dsNumPLP = rdata[27];
            l1Data->plpID = rdata[28];
            l1Data->isPlpBundled = (rdata[29] & 0x01);
            l1Data->plpType = (sony_dvbc2_plp_type_t)(rdata[30] & 0x03);
            l1Data->plpPayloadType = (sony_dvbc2_plp_payload_type_t)(rdata[31] & 0x1F);
            l1Data->plpGroupID = rdata[32];
            l1Data->plpStart = (((uint16_t) (rdata[33] & 0x3F) << 8) | rdata[34]);

            if (l1Data->dsType == SONY_DVBC2_DS_TYPE_1){
                /* Data Slice Type 1, PLP FEC Type, Code Rate and Modulation are correct */
                l1Data->plpFecType = (sony_dvbc2_ldpc_code_length_t)(rdata[35] & 0x01);
                l1Data->plpMod = (sony_dvbc2_constellation_t)(rdata[36] & 0x07);
                l1Data->plpCod = (sony_dvbc2_plp_code_rate_t)(rdata[37] & 0x07);

                /* Code rate for [(rdata[37] & 0x07) == 0x05] is 8/9 for PLP FEC type 16K LDPC and 9/10 for PLP FEC type 64K LDPC */
                if ((l1Data->plpFecType == SONY_DVBC2_LDPC_NORMAL) && (l1Data->plpCod == SONY_DVBC2_R8_9)){
                    l1Data->plpCod = SONY_DVBC2_R9_10;
                }
            }
            else{
                /* Data Slice 2 : PLP FEC Type, Code Rate and Modulation will be incorrect so return UNKNOWN */
                l1Data->plpFecType = SONY_DVBC2_LDPC_UNKNOWN;
                l1Data->plpMod = SONY_DVBC2_CONSTELLATION_UNKNOWN;
                l1Data->plpCod = SONY_DVBC2_CR_UNKNOWN;
            }

            l1Data->psisiReproc = (rdata[38] & 0x01);
            l1Data->transportID = (((uint16_t) rdata[39] << 8) | rdata[40]);
            l1Data->origNetworkID = (((uint16_t) rdata[41] << 8) | rdata[42]);
            l1Data->reservedTone = rdata[120] & 0x01;

            /* Collate Notch Data */
            {
                uint8_t i = 0;

                if (l1Data->numNotchs > SONY_DVBC2_NUM_NOTCH_MAX) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
                }

                /* Sort through read data 5 bytes at a time (IREG_RESERVED_X is ignored */
                for (i = 0; i < l1Data->numNotchs; i++) {
                    l1Data->notches[i].notchStart =
                        (((uint16_t) rdata[45 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)] << 8) | (rdata[46 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)]));
                    l1Data->notches[i].notchWidth =
                        (((uint16_t) rdata[47 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)] << 8) | (rdata[48 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)]));
                }

                /* Clear the remainder of the notches */
                for (i = l1Data->numNotchs; i < SONY_DVBC2_NUM_NOTCH_MAX; i++) {
                    l1Data->notches[i].notchStart = 0;
                    l1Data->notches[i].notchWidth = 0;
                }
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_QAM (sony_demod_t * pDemod,
                                            sony_dvbc2_plp_type_t type,
                                            sony_dvbc2_constellation_t * pQAM)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    uint8_t addr = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_QAM");

    if ((!pDemod) || (!pQAM)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyUnlock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Check TS lock. */
        if (!tsLock) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /*
         * Modulation monitor
         * slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   52h     16h    [6:4]       IREG_C2_MOD0[2:0]  (Data PLP)
         * <SLV-T>   52h     19h    [6:4]       IREG_C2_MOD1[2:0]  (Common PLP)
         */
        if ((type == SONY_DVBC2_PLP_GROUPED_DATA) || (type == SONY_DVBC2_PLP_NORMAL_DATA)) {
            addr = 0x16;
        }
        else {
            addr = 0x19;

            /* Check common PLP is enabled */
            result = CheckCommonPLPEnable(pDemod);
            if (result != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (result);
            }
        }

        /* Set SLV-T Bank : 0x52. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* QAM of selected PLP type */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, addr, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pQAM = (sony_dvbc2_constellation_t) ((rdata & 0x70) >> 4);

        SLVT_UnFreezeReg (pDemod);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_LDPCCodeLength (sony_demod_t * pDemod,
                                                       sony_dvbc2_plp_type_t type,
                                                       sony_dvbc2_ldpc_code_length_t * pCodeLength)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    uint8_t addr = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_LDPCCodeLength");

    if ((!pDemod) || (!pCodeLength)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyUnlock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }

        /* Check TS lock. */
        if (!tsLock) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /*
         * LDPC Code Length monitor
         * slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   52h     15h    [4]        IREG_C2_FEC_TYPE0  (Data PLP)
         * <SLV-T>   52h     18h    [4]        IREG_C2_FEC_TYPE1  (Common PLP)
         */
        if ((type == SONY_DVBC2_PLP_GROUPED_DATA) || (type == SONY_DVBC2_PLP_NORMAL_DATA)) {
            addr = 0x15;
        }
        else {
            addr = 0x18;

            /* Check common PLP is enabled */
            result = CheckCommonPLPEnable(pDemod);
            if (result != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (result);
            }
        }

        /* Set SLV-T Bank : 0x52. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* LDPC Code Length of selected PLP type */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, addr, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        *pCodeLength = (sony_dvbc2_ldpc_code_length_t) ((rdata & 0x10) >> 4);

        SLVT_UnFreezeReg (pDemod);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_CodeRate (sony_demod_t * pDemod,
                                                 sony_dvbc2_plp_type_t type,
                                                 sony_dvbc2_plp_code_rate_t * pCodeRate)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[2];
    uint8_t addr = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_CodeRate");

    if ((!pDemod) || (!pCodeRate)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock;
        sony_dvbc2_ldpc_code_length_t ldpcCodeLength;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyUnlock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }

        /* Check TS lock. */
        if (!tsLock) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /*
         * Code Rate monitor
         * slave    Bank    Addr    Bit              Name
         * ------------------------------------------------------------
         * <SLV-T>   52h     15h    [4]         IREG_C2_FEC_TYPE0  (Data PLP)
         * <SLV-T>   52h     16h    [2:0]       IREG_C2_COD0[2:0]  (Data PLP)
         * <SLV-T>   52h     18h    [4]         IREG_C2_FEC_TYPE1  (Common PLP)
         * <SLV-T>   52h     19h    [2:0]       IREG_C2_COD1[2:0]  (Common PLP)
         */
        if ((type == SONY_DVBC2_PLP_GROUPED_DATA) || (type == SONY_DVBC2_PLP_NORMAL_DATA)) {
            addr = 0x15;
        }
        else if (type == SONY_DVBC2_PLP_COMMON) {
            addr = 0x18;

            /* Check for common PLP enabled */
            result = CheckCommonPLPEnable(pDemod);
            if (result != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (result);
            }
        }

        /* Set SLV-T Bank : 0x52. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* FEC Type and Code Rate of selected PLP type */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, addr, &rdata[0], sizeof (rdata)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        ldpcCodeLength = (sony_dvbc2_ldpc_code_length_t) ((rdata[0] & 0x10) >> 4);
        *pCodeRate = (sony_dvbc2_plp_code_rate_t) (rdata[1] & 0x07);

        /* Code rate for [(rdata[1] & 0x07) == 0x05] is 8/9 for PLP FEC type 16K LDPC and 9/10 for PLP FEC type 64K LDPC */
        if ((ldpcCodeLength == SONY_DVBC2_LDPC_NORMAL) && (*pCodeRate == SONY_DVBC2_R8_9)){
            *pCodeRate = SONY_DVBC2_R9_10;
        }

        SLVT_UnFreezeReg (pDemod);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_SamplingOffset (sony_demod_t * pDemod,
                                                       int32_t * pPPM)
{
    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_SamplingOffset");

    if ((!pDemod) || (!pPPM)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t ctlValReg[5];
        uint8_t nominalRateReg[5];
        uint32_t trlCtlVal = 0;
        uint32_t trcgNominalRate = 0;
        int32_t num;
        int32_t den;
        sony_result_t result = SONY_RESULT_OK;
        uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyLock = 0;
        int8_t diffUpper;

        /* Freeze registers */
        if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &syncState, &tsLock, &earlyLock);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (syncState != 6) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit              name
         * ---------------------------------------------------------------
         * <SLV-T>   20h     52h     [6:0]      IREG_TRL_CTLVAL_S[38:32]
         * <SLV-T>   20h     53h     [7:0]      IREG_TRL_CTLVAL_S[31:24]
         * <SLV-T>   20h     54h     [7:0]      IREG_TRL_CTLVAL_S[23:16]
         * <SLV-T>   20h     55h     [7:0]      IREG_TRL_CTLVAL_S[15:8]
         * <SLV-T>   20h     56h     [7:0]      IREG_TRL_CTLVAL_S[7:0]
         * <SLV-T>   20h     9Fh     [6:0]      OREG_TRCG_NOMINALRATE[38:32]
         * <SLV-T>   20h     A0h     [7:0]      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h     [7:0]      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h     [7:0]      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h     [7:0]      OREG_TRCG_NOMINALRATE[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x52, ctlValReg, sizeof (ctlValReg)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, nominalRateReg, sizeof (nominalRateReg)) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Unfreeze registers. */
        SLVT_UnFreezeReg (pDemod);

        diffUpper = (ctlValReg[0] & 0x7F) - (nominalRateReg[0] & 0x7F);

        /* Confirm offset range */
        if ((diffUpper < -1) || (diffUpper > 1)) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        /* Clock frequency offset(ppm)  = { ( IREG_TRL_CTLVAL_S - OREG_TRCG_NOMINALRATE ) / OREG_TRCG_NOMINALRATE } * 1000000
         * Numerator = IREG_TRL_CTLVAL_S - OREG_TRCG_NOMINALRATE
         * Denominator = OREG_TRCG_NOMINALRATE / 1*10^8
         */

        /* Top 7 bits can be ignored for subtraction as out of range. */
        trlCtlVal = ctlValReg[1] << 24;
        trlCtlVal |= ctlValReg[2] << 16;
        trlCtlVal |= ctlValReg[3] << 8;
        trlCtlVal |= ctlValReg[4];

        trcgNominalRate = nominalRateReg[1] << 24;
        trcgNominalRate |= nominalRateReg[2] << 16;
        trcgNominalRate |= nominalRateReg[3] << 8;
        trcgNominalRate |= nominalRateReg[4];

        /* Shift down 1 bit to avoid overflow in subtraction */
        trlCtlVal >>= 1;
        trcgNominalRate >>= 1;

        if (diffUpper == 1) {
            num = (int32_t)((trlCtlVal + 0x80000000u) - trcgNominalRate);
        } else if (diffUpper == -1) {
            num = -(int32_t)((trcgNominalRate + 0x80000000u) - trlCtlVal);
        } else {
            num = (int32_t)(trlCtlVal - trcgNominalRate);
        }

        /* OREG_TRCG_NOMINALRATE is 39bit therefore:
         * Denominator = (OREG_TRCG_NOMINALRATE [38:8] * 256) / 1*10^8
         *             = (OREG_TRCG_NOMINALRATE [38:8] * 2^8) / (2^8 * 5^8)
         *             = OREG_TRCG_NOMINALRATE [38:8] / 390625
         */
        den = (nominalRateReg[0] & 0x7F) << 24;
        den |= nominalRateReg[1] << 16;
        den |= nominalRateReg[2] << 8;
        den |= nominalRateReg[3];
        den = (den + (390625/2)) / 390625;

        /* Shift down to align with numerator */
        den >>= 1;

        /* Perform calculation */
        if (num >= 0) {
            *pPPM = (num + (den/2)) / den;
        }
        else {
            *pPPM = (num - (den/2)) / den;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbc2_monitor_PER (sony_demod_t * pDemod,
                                            uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t packetError = 0;
    uint32_t period = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_PER");

    if (!pDemod || !pPER) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    {
        uint8_t rdata[3];

        /* Set SLV-T Bank : 0x24 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x24) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*  slave    Bank    Addr    Bit     Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   24h     FAh      [0]    IREG_SP_B2PER_VALID
         * <SLV-T>   24h     FBh    [7:0]    IREG_SP_B2PER_PKTERR[15:8]
         * <SLV-T>   24h     FCh    [7:0]    IREG_SP_B2PER_PKTERR[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFA, rdata, 3) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if ((rdata[0] & 0x01) == 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);    /* Not ready... */
        }

        packetError = (rdata[1] << 8) | rdata[2];

        /*  slave    Bank    Addr    Bit     Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   24h     DCh    [3:0]    OREG_SP_PER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xDC, rdata, 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        period = 1U << (rdata[0] & 0x0F);
    }

    if ((period == 0) || (packetError > period)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /*
      PER = IREG_SP_B2PER_PKTERR / ( 2 ^ OREG_SP_PER_MES )

      PER = packetError * 1000000 / period
          = packetError * 1000 * 1000 / period
          (Divide in 2 steps to prevent overflow.)
    */
    {
        uint32_t div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        div = period;

        Q = (packetError * 1000) / div;
        R = (packetError * 1000) % div;

        R *= 1000;
        Q = Q * 1000 + R / div;
        R = R % div;

        /* rounding */
        if ((div != 1) && (R >= div/2)) {
            *pPER = Q + 1;
        }
        else {
            *pPER = Q;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_monitor_GI (sony_demod_t * pDemod,
                                           sony_dvbc2_l1_gi_t * pGI)
{
    uint8_t gi_data;
    uint8_t data;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_GI");

    if ((!pDemod) || (!pGI)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x50. */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* slave    Bank    Addr    Bit     Name
     *------------------------------------------------------
     * <SLV-T>   50h     6Eh    [0]     OREG_L1C2_PRESET_ON
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6E, &data, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data & 0x01){
        /* Dependent static DS */
        /* Slave     Bank    Addr   Bit  Name
         * --------------------------------------------
         * <SLV-T>   50h     55h    [4]  OREG_L1C2_GI64
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x55, &gi_data, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (gi_data & 0x10) {
            *pGI = SONY_DVBC2_GI_1_64;
        }
        else {
            *pGI = SONY_DVBC2_GI_1_128;
        }
    }
    else{
        /* Normal DS */
        result = CheckL1DataReady(pDemod);
        if (result != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        else{
            /* Set SLV-T Bank : 0x50 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Slave     Bank    Addr   Bit  Name
             * --------------------------------------------
             * <SLV-T>   50h     15h    [4]  IREG_L1C2_GI64
             */
            if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x15, &gi_data, 1) != SONY_RESULT_OK) {
                SLVT_UnFreezeReg (pDemod);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (gi_data & 0x10) {
                *pGI = SONY_DVBC2_GI_1_64;
            }
            else {
                *pGI = SONY_DVBC2_GI_1_128;
            }
        }
    }

    SLVT_UnFreezeReg (pDemod);

    SONY_TRACE_RETURN (result);
}

/*-----------------------------------------------------------------------------
    Static Functions
-----------------------------------------------------------------------------*/
static sony_result_t CheckL1DataReady (sony_demod_t * pDemod)
{
    uint8_t r_data_en;
    uint8_t preset_ready_mon;

    SONY_TRACE_ENTER ("CheckL1DataReady");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* wait IREG_L1P2R_DATA_EN=H && IREG_L1C2_PRESET_READY_MON[0]=H */

    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * slave    Bank    Addr    Bit              Name                        meaning
     * --------------------------------------------------------------------------------------
     * <SLV-T>   52h     1Fh     [0]       IREG_L1P2R_DATA_EN           0:invalid 1:valid
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1F, &r_data_en, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * slave    Bank    Addr    Bit              Name                        meaning
     * --------------------------------------------------------------------------------------
     * <SLV-T>   50h     C4h     [0]       IREG_L1C2_PRESET_READY_MON   0:invalid, 1:valid
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC4, &preset_ready_mon, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((r_data_en & 0x01) && (preset_ready_mon & 0x01)) {
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }
}

static sony_result_t CheckCommonPLPEnable (sony_demod_t * pDemod)
{
    uint8_t rdata;
    sony_result_t result;

    SONY_TRACE_ENTER ("CheckCommonPLPEnable");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * slave    Bank    Addr    Bit     Name
     *------------------------------------------------------
     * <SLV-T>   50h     6Eh    [0]     OREG_L1C2_PRESET_ON
     */
    /* Set SLV-T Bank : 0x50. */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6E, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (rdata & 0x01){
        /* Dependent static DS */
        /* slave    Bank    Addr    Bit     Name
         *------------------------------------------------------
         * <SLV-T>   50h     68h    [6]     OREG_L1C2_COMMON_EN
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x68, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only check OREG_L1C2_COMMON_EN */
        if (rdata & 0x40) {
            SONY_TRACE_RETURN(SONY_RESULT_OK); /* Common PLP is enabled */
        }
        else {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }
    }
    else {
        /* Normal DS */
        /* Check for L1 data ready */
        result = CheckL1DataReady(pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }


        /* Set SLV-T Bank : 0x50. */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* slave    Bank    Addr    Bit    Name
         *------------------------------------------------------
         * <SLV-T>   50h     48h    [6]     IREG_L1C2_COMMON_EN
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x48, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Only check IREG_L1C2_COMMON_EN */
        if (rdata & 0x40) {
            SONY_TRACE_RETURN(SONY_RESULT_OK); /* Common PLP is enabled */
        }
        else {
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }
    }
}
