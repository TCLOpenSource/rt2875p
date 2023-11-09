/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/

#include "sony_math.h"
#include "sony_demod_isdbs.h"
#include "sony_demod_isdbs_monitor.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
/*
 @brief Confirm demodulator lock
*/
static sony_result_t IsDmdLocked(sony_demod_t* pDemod);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_isdbs_monitor_SyncStat (sony_demod_t * pDemod,
                                                 uint8_t * pAGCLockStat,
                                                 uint8_t * pTSLockStat,
                                                 uint8_t * pTMCCLockStat)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[3];
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_SyncStat");

    if ((!pDemod) || (!pAGCLockStat) || (!pTSLockStat) || (!pTMCCLockStat)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
      /*   slave     Bank      Addr      Bit      Signal name
       *  --------------------------------------------------------------
       *  <SLV-T>    A0h       10h       [5]      IRFAGC_LOCK
       *  <SLV-T>    A0h       12h       [6]      ITSLOCK
       *  <SLV-T>    A0h       12h       [5]      ITMCCLOCK
       */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pAGCLockStat = (uint8_t)((data[0] & 0x20) ? 1 : 0);
    *pTSLockStat = (uint8_t)((data[2] & 0x40) ? 1 : 0);
    *pTMCCLockStat = (uint8_t)((data[2] & 0x20) ? 1 : 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                      int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[3];
    uint32_t regValue = 0;
    int32_t cfrl_ctrlval = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_CarrierOffset");

    if ((!pDemod) || (!pOffset)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       45h       [4:0]    ICFRL_CTRLVAL[20:16]
     * <SLV-T>    A0h       46h       [7:0]    ICFRL_CTRLVAL[15:8]
     * <SLV-T>    A0h       47h       [7:0]    ICFRL_CTRLVAL[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x45, data, 3) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    regValue = (((uint32_t)data[0] & 0x1F) << 16) | (((uint32_t)data[1] & 0xFF) <<  8) | ((uint32_t)data[2] & 0xFF);
    cfrl_ctrlval = sony_Convert2SComplement (regValue, 21);

    /*----------------------------------------------------------------
      samplingRate = 64000(KHz)

      Offset(KHz) = cfrl_ctrlval * (-1) * 64000 / 2^20
                  = cfrl_ctrlval * (-1) * 125 * 2^9 / 2^20
                  = cfrl_ctrlval * (-1) * 125 / 2^11
    ----------------------------------------------------------------*/
    if(cfrl_ctrlval > 0){
        *pOffset = ((cfrl_ctrlval * (-125)) - 1024) / 2048;
    } else {
        *pOffset = ((cfrl_ctrlval * (-125)) + 1024) / 2048;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                 uint32_t * pIFAGCOut)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGCOut)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit       Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       1Fh       [4:0]     IRFAGC_GAIN[12:8]
     * <SLV-T>    A0h       20h       [7:0]     IRFAGC_GAIN[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1F, data, 2) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pIFAGCOut = (((uint32_t)data[0] & 0x1F) << 8) | (uint32_t)(data[1] & 0xFF);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_CNR (sony_demod_t * pDemod,
                                            int32_t * pCNR)
{
    uint8_t rdata[3] = {0x00, 0x00, 0x00};
    uint16_t value = 0x0000;
    int32_t index = 0;
    int32_t minIndex = 0;
    int32_t maxIndex = 0;
    const struct {
        uint32_t value;
        int32_t cnr_x1000;
    } isdbs_cn_data[] = {
        {0x05af, 0},
        {0x0597, 100},
        {0x057e, 200},
        {0x0567, 300},
        {0x0550, 400},
        {0x0539, 500},
        {0x0522, 600},
        {0x050c, 700},
        {0x04f6, 800},
        {0x04e1, 900},
        {0x04cc, 1000},
        {0x04b6, 1100},
        {0x04a1, 1200},
        {0x048c, 1300},
        {0x0477, 1400},
        {0x0463, 1500},
        {0x044f, 1600},
        {0x043c, 1700},
        {0x0428, 1800},
        {0x0416, 1900},
        {0x0403, 2000},
        {0x03ef, 2100},
        {0x03dc, 2200},
        {0x03c9, 2300},
        {0x03b6, 2400},
        {0x03a4, 2500},
        {0x0392, 2600},
        {0x0381, 2700},
        {0x036f, 2800},
        {0x035f, 2900},
        {0x034e, 3000},
        {0x033d, 3100},
        {0x032d, 3200},
        {0x031d, 3300},
        {0x030d, 3400},
        {0x02fd, 3500},
        {0x02ee, 3600},
        {0x02df, 3700},
        {0x02d0, 3800},
        {0x02c2, 3900},
        {0x02b4, 4000},
        {0x02a6, 4100},
        {0x0299, 4200},
        {0x028c, 4300},
        {0x027f, 4400},
        {0x0272, 4500},
        {0x0265, 4600},
        {0x0259, 4700},
        {0x024d, 4800},
        {0x0241, 4900},
        {0x0236, 5000},
        {0x022b, 5100},
        {0x0220, 5200},
        {0x0215, 5300},
        {0x020a, 5400},
        {0x0200, 5500},
        {0x01f6, 5600},
        {0x01ec, 5700},
        {0x01e2, 5800},
        {0x01d8, 5900},
        {0x01cf, 6000},
        {0x01c6, 6100},
        {0x01bc, 6200},
        {0x01b3, 6300},
        {0x01aa, 6400},
        {0x01a2, 6500},
        {0x0199, 6600},
        {0x0191, 6700},
        {0x0189, 6800},
        {0x0181, 6900},
        {0x0179, 7000},
        {0x0171, 7100},
        {0x0169, 7200},
        {0x0161, 7300},
        {0x015a, 7400},
        {0x0153, 7500},
        {0x014b, 7600},
        {0x0144, 7700},
        {0x013d, 7800},
        {0x0137, 7900},
        {0x0130, 8000},
        {0x012a, 8100},
        {0x0124, 8200},
        {0x011e, 8300},
        {0x0118, 8400},
        {0x0112, 8500},
        {0x010c, 8600},
        {0x0107, 8700},
        {0x0101, 8800},
        {0x00fc, 8900},
        {0x00f7, 9000},
        {0x00f2, 9100},
        {0x00ec, 9200},
        {0x00e7, 9300},
        {0x00e2, 9400},
        {0x00dd, 9500},
        {0x00d8, 9600},
        {0x00d4, 9700},
        {0x00cf, 9800},
        {0x00ca, 9900},
        {0x00c6, 10000},
        {0x00c2, 10100},
        {0x00be, 10200},
        {0x00b9, 10300},
        {0x00b5, 10400},
        {0x00b1, 10500},
        {0x00ae, 10600},
        {0x00aa, 10700},
        {0x00a6, 10800},
        {0x00a3, 10900},
        {0x009f, 11000},
        {0x009b, 11100},
        {0x0098, 11200},
        {0x0095, 11300},
        {0x0091, 11400},
        {0x008e, 11500},
        {0x008b, 11600},
        {0x0088, 11700},
        {0x0085, 11800},
        {0x0082, 11900},
        {0x007f, 12000},
        {0x007c, 12100},
        {0x007a, 12200},
        {0x0077, 12300},
        {0x0074, 12400},
        {0x0072, 12500},
        {0x006f, 12600},
        {0x006d, 12700},
        {0x006b, 12800},
        {0x0068, 12900},
        {0x0066, 13000},
        {0x0064, 13100},
        {0x0061, 13200},
        {0x005f, 13300},
        {0x005d, 13400},
        {0x005b, 13500},
        {0x0059, 13600},
        {0x0057, 13700},
        {0x0055, 13800},
        {0x0053, 13900},
        {0x0051, 14000},
        {0x004f, 14100},
        {0x004e, 14200},
        {0x004c, 14300},
        {0x004a, 14400},
        {0x0049, 14500},
        {0x0047, 14600},
        {0x0045, 14700},
        {0x0044, 14800},
        {0x0042, 14900},
        {0x0041, 15000},
        {0x003f, 15100},
        {0x003e, 15200},
        {0x003c, 15300},
        {0x003b, 15400},
        {0x003a, 15500},
        {0x0038, 15600},
        {0x0037, 15700},
        {0x0036, 15800},
        {0x0034, 15900},
        {0x0033, 16000},
        {0x0032, 16100},
        {0x0031, 16200},
        {0x0030, 16300},
        {0x002f, 16400},
        {0x002e, 16500},
        {0x002d, 16600},
        {0x002c, 16700},
        {0x002b, 16800},
        {0x002a, 16900},
        {0x0029, 17000},
        {0x0028, 17100},
        {0x0027, 17200},
        {0x0026, 17300},
        {0x0025, 17400},
        {0x0024, 17500},
        {0x0023, 17600},
        {0x0022, 17800},
        {0x0021, 17900},
        {0x0020, 18000},
        {0x001f, 18200},
        {0x001e, 18300},
        {0x001d, 18500},
        {0x001c, 18700},
        {0x001b, 18900},
        {0x001a, 19000},
        {0x0019, 19200},
        {0x0018, 19300},
        {0x0017, 19500},
        {0x0016, 19700},
        {0x0015, 19900},
        {0x0014, 20000},
    };

    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_CNR");

    if ((!pDemod) || (!pCNR)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*   slave     Bank      Addr      Bit     Signal name
     *  --------------------------------------------------------------
     *   <SLV-T>    A1h       10h       [0]     ICPM_QUICKRDY
     *   <SLV-T>    A1h       11h       [4:0]   ICPM_QUICKCNDT[12:8]
     *   <SLV-T>    A1h       12h       [7:0]   ICPM_QUICKCNDT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, rdata, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((rdata[0] & 0x01) == 0) {
        /* C/N monitor calculation is not completed */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    value = (uint16_t)((((uint32_t)rdata[1] & 0x1F) << 8) | rdata[2]);

    minIndex = 0;
    maxIndex = sizeof(isdbs_cn_data)/sizeof(isdbs_cn_data[0]) - 1;

    /* Cropping */
    if(value >= isdbs_cn_data[minIndex].value) {
        *pCNR = isdbs_cn_data[minIndex].cnr_x1000;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }
    if(value <= isdbs_cn_data[maxIndex].value) {
        *pCNR = isdbs_cn_data[maxIndex].cnr_x1000;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    /* Binary Search */
    while (1) {
        index = (maxIndex + minIndex) / 2;

        if (value == isdbs_cn_data[index].value) {
            *pCNR = isdbs_cn_data[index].cnr_x1000;
            break;
        } else if (value > isdbs_cn_data[index].value) {
            maxIndex = index;
        } else {
            minIndex = index;
        }

        if ((maxIndex - minIndex) <= 1) {
            if (value == isdbs_cn_data[maxIndex].value) {
                *pCNR = isdbs_cn_data[maxIndex].cnr_x1000;
                break;
            } else {
                *pCNR = isdbs_cn_data[minIndex].cnr_x1000;
                break;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs_monitor_PER (sony_demod_t * pDemod,
                                            uint32_t * pPERH, uint32_t * pPERL)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[4];
    uint32_t packetError[2];
    uint32_t measuredSlots[2];
    uint8_t valid[2];
    uint32_t * pPER[2];
    int i = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_PER");

    if ((!pDemod) || (!pPERH) || (!pPERL)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    pPER[0] = pPERH;
    pPER[1] = pPERL;

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *   Slave     Bank      Addr      Bit       Signal name
     * -------------------------------------------------------------
     *  <SLV-T>    C0h       56h       [7:0]     IRSPKERR1[15:8]
     *  <SLV-T>    C0h       57h       [7:0]     IRSPKERR1[7:0]
     *  <SLV-T>    C0h       58h       [7:0]     IRSSLTNUM1[15:8]
     *  <SLV-T>    C0h       59h       [7:0]     IRSSLTNUM1[7:0]
     *  <SLV-T>    C0h       79h       [4]       IRSILGL1
     *
     *  <SLV-T>    C0h       60h       [7:0]     IRSPKERR2[15:8]
     *  <SLV-T>    C0h       61h       [7:0]     IRSPKERR2[7:0]
     *  <SLV-T>    C0h       62h       [7:0]     IRSSLTNUM2[15:8]
     *  <SLV-T>    C0h       63h       [7:0]     IRSSLTNUM2[7:0]
     *  <SLV-T>    C0h       79h       [3]       IRSILGL2
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x79, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* If 1, the PER value is invalid */
    valid[0] = (rdata[0] & 0x10) ? 0 : 1; /* High hierarchy */
    valid[1] = (rdata[0] & 0x08) ? 0 : 1; /* Low hierarchy */

    /* High hierarchy */
    if (valid[0]) {
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x56, rdata, 4) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        packetError[0] = (rdata[0] << 8) | rdata[1];
        measuredSlots[0] = (rdata[2] << 8) | rdata[3];
    }

    /* Low hierarchy */
    if (valid[1]) {
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x60, rdata, 4) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        packetError[1] = (rdata[0] << 8) | rdata[1];
        measuredSlots[1] = (rdata[2] << 8) | rdata[3];
    }

    SLVT_UnFreezeReg (pDemod);

    for (i = 0; i <= 1; i++) {
        uint32_t Div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (!valid[i] || (measuredSlots[i] == 0) || (measuredSlots[i] < packetError[i])) {
            *pPER[i] = SONY_DEMOD_ISDBS_MONITOR_PER_INVALID;
            continue;
        }

        Div = measuredSlots[i];

        Q = (packetError[i] * 1000) / Div;
        R = (packetError[i] * 1000) % Div;
        R *= 1000;
        Q = Q * 1000 + R / Div;
        R = R % Div;
        /* rounding */
        if (R  <= (Div/2)) {
            *pPER[i] = Q;
        }
        else {
            *pPER[i] = Q + 1;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs_monitor_ModCod (sony_demod_t * pDemod,
                                               sony_isdbs_modcod_t * pModCodH, sony_isdbs_modcod_t * pModCodL)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_ModCod");

    if ((!pDemod) || (!pModCodH) || (!pModCodL)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *    Slave     Bank      Addr      Bit       Signal name
     * -------------------------------------------------------------
     *   <SLV-T>    C0h       8Ah       [7:4]     ISTMODEH[3:0]
     *   <SLV-T>    C0h       8Ah       [3:0]     ISTMODEL[3:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x8A, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pModCodH = (sony_isdbs_modcod_t)((rdata >> 4) & 0x0F);
    *pModCodL = (sony_isdbs_modcod_t)(rdata & 0x0F);

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_isdbs_monitor_PreRSBER (sony_demod_t * pDemod,
                                                 uint32_t * pBERH, uint32_t * pBERL, uint32_t * pBERTMCC)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[3];
    /* High hierarchy, low hierarchy, TMCC */
    uint32_t bitError[3];
    uint32_t measuredSlots[3];
    uint8_t valid[3];
    uint32_t * pBER[3];
    int i = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_PreRSBER");

    if ((!pDemod) || (!pBERH) || (!pBERL) || (!pBERTMCC)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    pBER[0] = pBERH;
    pBER[1] = pBERL;
    pBER[2] = pBERTMCC;

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *  Slave      Bank      Addr      Bit       Signal name
     * -------------------------------------------------------------
     *  <SLV-T>    C0h       50h       [6:0]     IRSBIERR1[22:16]
     *  <SLV-T>    C0h       51h       [7:0]     IRSBIERR1[15:8]
     *  <SLV-T>    C0h       52h       [7:0]     IRSBIERR1[7:0]
     *  <SLV-T>    C0h       58h       [7:0]     IRSSLTNUM1[15:8]
     *  <SLV-T>    C0h       59h       [7:0]     IRSSLTNUM1[7:0]
     *  <SLV-T>    C0h       79h       [4]       IRSILGL1
     *
     *  <SLV-T>    C0h       5Ah       [6:0]     IRSBIERR2[22:16]
     *  <SLV-T>    C0h       5Bh       [7:0]     IRSBIERR2[15:8]
     *  <SLV-T>    C0h       5Ch       [7:0]     IRSBIERR2[7:0]
     *  <SLV-T>    C0h       62h       [7:0]     IRSSLTNUM2[15:8]
     *  <SLV-T>    C0h       63h       [7:0]     IRSSLTNUM2[7:0]
     *  <SLV-T>    C0h       79h       [3]       IRSILGL2
     *
     *  <SLV-T>    C0h       7Bh       [5:0]     IRSBIERRT[13:8]
     *  <SLV-T>    C0h       7Ch       [7:0]     IRSBIERRT[7:0]
     *  <SLV-T>    C0h       7Fh       [7:0]     IRSSFNUMT[7:0]
     *  <SLV-T>    C0h       79h       [0]       IRSILGLT
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x79, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* If 1, the BER value is invalid */
    valid[0] = (rdata[0] & 0x10) ? 0 : 1; /* High hierarchy */
    valid[1] = (rdata[0] & 0x08) ? 0 : 1; /* Low hierarchy */
    valid[2] = (rdata[0] & 0x01) ? 0 : 1; /* TMCC */

    /* High hierarchy */
    if (valid[0]) {
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x50, rdata, 3) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        bitError[0] = ((rdata[0] & 0x7F) << 16) | (rdata[1] << 8) | rdata[2];

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x58, rdata, 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        measuredSlots[0] = (rdata[0] << 8) | rdata[1];
    }

    /* Low hierarchy */
    if (valid[1]) {
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x5A, rdata, 3) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        bitError[1] = ((rdata[0] & 0x7F) << 16) | (rdata[1] << 8) | rdata[2];

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x62, rdata, 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        measuredSlots[1] = (rdata[0] << 8) | rdata[1];
    }

    /* TMCC */
    if (valid[2]) {
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7B, rdata, 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        bitError[2] = ((rdata[0] & 0x3F) << 8) | rdata[1];

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7F, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        measuredSlots[2] = rdata[0];
    }

    SLVT_UnFreezeReg (pDemod);

    for (i = 0; i <= 2; i++) {
        uint32_t Div = 0;
        uint32_t Q = 0;
        uint32_t R = 0;

        if (!valid[i] || (measuredSlots[i] == 0)) {
            *pBER[i] = SONY_DEMOD_ISDBS_MONITOR_PRERSBER_INVALID;
            continue;
        }

        if ((i == 0) || (i == 1)) {
            /* High/Low hierarchy BER calculation
             * Pre-RS BER x 1e7 = Total Number of Packet Errors x 1e7 / Number of Measured Slots * 204 * 8
             *                  = (bitError x 10000000) / (measuredSlots * 8 * 204)
             *                  = (bitError x 312500) / (measuredSlots * 51)
             *                  = (bitError * 250 * 1250) / (measuredSlots * 51)
             *                  (Divide in 2 steps to prevent overflow.)
             */
            Div = measuredSlots[i] * 51;
            Q = (bitError[i] * 250) / Div;
            R = (bitError[i] * 250) % Div;

            R *= 1250;
            Q = Q * 1250 + R / Div;
            R = R % Div;

            if (R >= (Div/2)) {
                *pBER[i] = Q + 1;
            } else {
                *pBER[i] = Q;
            }
        } else {
            /* TMCC BER calculation
             * Pre-RS BER x 1e7 = Total Number of Packet Errors  x 1e7 / Number of Measured Slots * 512
             *                  = (bitError x 10000000) / (measuredSlots * 8 * 64)
             *                  = (bitError x 78125) / (measuredSlots * 4)
             *                  = (bitError * 625 * 125) / (measuredSlots * 4)
             *                  (Divide in 2 steps to prevent overflow.)
             */
            Div = measuredSlots[i] * 4;
            Q = (bitError[i] * 625) / Div;
            R = (bitError[i] * 625) % Div;

            R *= 125;
            Q = Q * 125 + R / Div;
            R = R % Div;

            if (R >= (Div/2)) {
                *pBER[i] = Q + 1;
            } else {
                *pBER[i] = Q;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs_monitor_TMCCInfo(sony_demod_t * pDemod,
                                                sony_isdbs_tmcc_info_t * pTMCCInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[57] = {0x00};
    uint32_t i = 0;
    SONY_TRACE_ENTER("sony_demod_isdbs_monitor_TMCCInfo");

    if ((!pDemod) || (!pTMCCInfo)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC9 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC9);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }
    /*
     *   slave     Bank      Addr      Bit      Signal name                 Meaning
     *  -----------------------------------------------------------------------------------------------------------------
     *   <SLV-T>    C9h       10h       [4:0]    ITMCCCHG_ARK               Order of change flag
     */

    /* Order of change flag */
    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 1);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    pTMCCInfo->changeOrder = (data[0] & 0x1F);

    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*
     *   slave     Bank      Addr      Bit      Signal name                 Meaning                                Series Num
     *  -----------------------------------------------------------------------------------------------------------------
     *   <SLV-T>    C0h       11h       [3:0]    ITMCCDATA_ARK[378:375]     Transmission Mode1 Modulation and Code rate  0
     *   <SLV-T>    C0h       12h       [5:0]    ITMCCDATA_ARK[374:369]     Transmission Mode1 Number of Slot            1
     *   <SLV-T>    C0h       13h       [3:0]    ITMCCDATA_ARK[368:365]     Transmission Mode2 Modulation and Code rate  2
     *   <SLV-T>    C0h       14h       [5:0]    ITMCCDATA_ARK[364:359]     Transmission Mode2 Number of Slot            3
     *   <SLV-T>    C0h       15h       [3:0]    ITMCCDATA_ARK[358:355]     Transmission Mode3 Modulation and Code rate  4
     *   <SLV-T>    C0h       16h       [5:0]    ITMCCDATA_ARK[354:349]     Transmission Mode3 Number of Slot            5
     *   <SLV-T>    C0h       17h       [3:0]    ITMCCDATA_ARK[348:345]     Transmission Mode4 Modulation and Code rate  6
     *   <SLV-T>    C0h       18h       [5:0]    ITMCCDATA_ARK[344:339]     Transmission Mode4 Number of Slot            7
     *   <SLV-T>    C0h       19h       [6:4]    ITMCCDATA_ARK[338:336]     Relative TSID for slot1                      8
     *   <SLV-T>    C0h       19h       [2:0]    ITMCCDATA_ARK[335:333]     Relative TSID for slot2                      8
     *   <SLV-T>    C0h       1Ah       [6:4]    ITMCCDATA_ARK[332:330]     Relative TSID for slot3                      9
     *   <SLV-T>    C0h       1Ah       [2:0]    ITMCCDATA_ARK[329:327]     Relative TSID for slot4                      9
     *   <SLV-T>    C0h       1Bh       [6:4]    ITMCCDATA_ARK[326:324]     Relative TSID for slot5                     10
     *   <SLV-T>    C0h       1Bh       [2:0]    ITMCCDATA_ARK[323:321]     Relative TSID for slot6                     10
     *   <SLV-T>    C0h       1Ch       [6:4]    ITMCCDATA_ARK[320:318]     Relative TSID for slot7                     11
     *   <SLV-T>    C0h       1Ch       [2:0]    ITMCCDATA_ARK[317:315]     Relative TSID for slot8                     11
     *   <SLV-T>    C0h       1Dh       [6:4]    ITMCCDATA_ARK[314:312]     Relative TSID for slot9                     12
     *   <SLV-T>    C0h       1Dh       [2:0]    ITMCCDATA_ARK[311:309]     Relative TSID for slot10                    12
     *   <SLV-T>    C0h       1Eh       [6:4]    ITMCCDATA_ARK[308:306]     Relative TSID for slot11                    13
     *   <SLV-T>    C0h       1Eh       [2:0]    ITMCCDATA_ARK[305:303]     Relative TSID for slot12                    13
     *   <SLV-T>    C0h       1Fh       [6:4]    ITMCCDATA_ARK[302:300]     Relative TSID for slot13                    14
     *   <SLV-T>    C0h       1Fh       [2:0]    ITMCCDATA_ARK[299:297]     Relative TSID for slot14                    14
     *   <SLV-T>    C0h       20h       [6:4]    ITMCCDATA_ARK[296:294]     Relative TSID for slot15                    15
     *   <SLV-T>    C0h       20h       [2:0]    ITMCCDATA_ARK[293:291]     Relative TSID for slot16                    15
     *   <SLV-T>    C0h       21h       [6:4]    ITMCCDATA_ARK[290:288]     Relative TSID for slot17                    16
     *   <SLV-T>    C0h       21h       [2:0]    ITMCCDATA_ARK[287:285]     Relative TSID for slot18                    16
     *   <SLV-T>    C0h       22h       [6:4]    ITMCCDATA_ARK[284:282]     Relative TSID for slot19                    17
     *   <SLV-T>    C0h       22h       [2:0]    ITMCCDATA_ARK[281:279]     Relative TSID for slot20                    17
     *   <SLV-T>    C0h       23h       [6:4]    ITMCCDATA_ARK[278:276]     Relative TSID for slot21                    18
     *   <SLV-T>    C0h       23h       [2:0]    ITMCCDATA_ARK[275:273]     Relative TSID for slot22                    18
     *   <SLV-T>    C0h       24h       [6:4]    ITMCCDATA_ARK[272:270]     Relative TSID for slot23                    19
     *   <SLV-T>    C0h       24h       [2:0]    ITMCCDATA_ARK[269:267]     Relative TSID for slot24                    19
     *   <SLV-T>    C0h       25h       [6:4]    ITMCCDATA_ARK[266:264]     Relative TSID for slot25                    20
     *   <SLV-T>    C0h       25h       [2:0]    ITMCCDATA_ARK[263:261]     Relative TSID for slot26                    20
     *   <SLV-T>    C0h       26h       [6:4]    ITMCCDATA_ARK[260:258]     Relative TSID for slot27                    21
     *   <SLV-T>    C0h       26h       [2:0]    ITMCCDATA_ARK[257:255]     Relative TSID for slot28                    21
     *   <SLV-T>    C0h       27h       [6:4]    ITMCCDATA_ARK[254:252]     Relative TSID for slot29                    22
     *   <SLV-T>    C0h       27h       [2:0]    ITMCCDATA_ARK[251:249]     Relative TSID for slot30                    22
     *   <SLV-T>    C0h       28h       [6:4]    ITMCCDATA_ARK[248:246]     Relative TSID for slot31                    23
     *   <SLV-T>    C0h       28h       [2:0]    ITMCCDATA_ARK[245:243]     Relative TSID for slot32                    23
     *   <SLV-T>    C0h       29h       [6:4]    ITMCCDATA_ARK[242:240]     Relative TSID for slot33                    24
     *   <SLV-T>    C0h       29h       [2:0]    ITMCCDATA_ARK[239:237]     Relative TSID for slot34                    24
     *   <SLV-T>    C0h       2Ah       [6:4]    ITMCCDATA_ARK[236:234]     Relative TSID for slot35                    25
     *   <SLV-T>    C0h       2Ah       [2:0]    ITMCCDATA_ARK[233:231]     Relative TSID for slot36                    25
     *   <SLV-T>    C0h       2Bh       [6:4]    ITMCCDATA_ARK[230:228]     Relative TSID for slot37                    26
     *   <SLV-T>    C0h       2Bh       [2:0]    ITMCCDATA_ARK[227:225]     Relative TSID for slot38                    26
     *   <SLV-T>    C0h       2Ch       [6:4]    ITMCCDATA_ARK[224:222]     Relative TSID for slot39                    27
     *   <SLV-T>    C0h       2Ch       [2:0]    ITMCCDATA_ARK[221:219]     Relative TSID for slot40                    27
     *   <SLV-T>    C0h       2Dh       [6:4]    ITMCCDATA_ARK[218:216]     Relative TSID for slot41                    28
     *   <SLV-T>    C0h       2Dh       [2:0]    ITMCCDATA_ARK[215:213]     Relative TSID for slot42                    28
     *   <SLV-T>    C0h       2Eh       [6:4]    ITMCCDATA_ARK[212:210]     Relative TSID for slot43                    29
     *   <SLV-T>    C0h       2Eh       [2:0]    ITMCCDATA_ARK[209:207]     Relative TSID for slot44                    29
     *   <SLV-T>    C0h       2Fh       [6:4]    ITMCCDATA_ARK[206:204]     Relative TSID for slot45                    30
     *   <SLV-T>    C0h       2Fh       [2:0]    ITMCCDATA_ARK[203:201]     Relative TSID for slot46                    30
     *   <SLV-T>    C0h       30h       [6:4]    ITMCCDATA_ARK[200:198]     Relative TSID for slot47                    31
     *   <SLV-T>    C0h       30h       [2:0]    ITMCCDATA_ARK[197:195]     Relative TSID for slot48                    31
     *   <SLV-T>    C0h       31h       [7:0]    ITMCCDATA_ARK[194:187]     Corresponding TSID[15:8] for relative TS0   32
     *   <SLV-T>    C0h       32h       [7:0]    ITMCCDATA_ARK[186:179]     Corresponding TSID[7:0]  for relative TS0   33
     *   <SLV-T>    C0h       33h       [7:0]    ITMCCDATA_ARK[178:171]     Corresponding TSID[15:8] for relative TS1   34
     *   <SLV-T>    C0h       34h       [7:0]    ITMCCDATA_ARK[170:163]     Corresponding TSID[7:0]  for relative TS1   35
     *   <SLV-T>    C0h       35h       [7:0]    ITMCCDATA_ARK[162:155]     Corresponding TSID[15:8] for relative TS2   36
     *   <SLV-T>    C0h       36h       [7:0]    ITMCCDATA_ARK[154:147]     Corresponding TSID[7:0]  for relative TS2   37
     *   <SLV-T>    C0h       37h       [7:0]    ITMCCDATA_ARK[146:139]     Corresponding TSID[15:8] for relative TS3   38
     *   <SLV-T>    C0h       38h       [7:0]    ITMCCDATA_ARK[138:131]     Corresponding TSID[7:0]  for relative TS3   39
     *   <SLV-T>    C0h       39h       [7:0]    ITMCCDATA_ARK[130:123]     Corresponding TSID[15:8] for relative TS4   40
     *   <SLV-T>    C0h       3Ah       [7:0]    ITMCCDATA_ARK[122:115]     Corresponding TSID[7:0]  for relative TS4   41
     *   <SLV-T>    C0h       3Bh       [7:0]    ITMCCDATA_ARK[114:107]     Corresponding TSID[15:8] for relative TS5   42
     *   <SLV-T>    C0h       3Ch       [7:0]    ITMCCDATA_ARK[106:99]      Corresponding TSID[7:0]  for relative TS5   43
     *   <SLV-T>    C0h       3Dh       [7:0]    ITMCCDATA_ARK[98:91]       Corresponding TSID[15:8] for relative TS6   44
     *   <SLV-T>    C0h       3Eh       [7:0]    ITMCCDATA_ARK[90:83]       Corresponding TSID[7:0]  for relative TS6   45
     *   <SLV-T>    C0h       3Fh       [7:0]    ITMCCDATA_ARK[82:75]       Corresponding TSID[15:8] for relative TS7   46
     *   <SLV-T>    C0h       40h       [7:0]    ITMCCDATA_ARK[74:67]       Corresponding TSID[7:0]  for relative TS7   47
     *   <SLV-T>    C0h       41h       [5]      ITMCCDATA_ARK[66]          EWS flag                                    48
     *   <SLV-T>    C0h       41h       [4]      ITMCCDATA_ARK[65]          Site Diversity flag                         48
     *   <SLV-T>    C0h       41h       [3:1]    ITMCCDATA_ARK[64:62]       Site Diversity information                  48
     *   <SLV-T>    C0h       41h       [0]      ITMCCDATA_ARK[61]          Extension information flag                  48
     *   <SLV-T>    C0h       42h       [7:0]    ITMCCDATA_ARK[60:56]       Extension information1                      49
     *   <SLV-T>    C0h       43h       [7:0]    ITMCCDATA_ARK[55:48]       Extension information2                      50
     *   <SLV-T>    C0h       44h       [7:0]    ITMCCDATA_ARK[47:40]       Extension information3                      51
     *   <SLV-T>    C0h       45h       [7:0]    ITMCCDATA_ARK[39:32]       Extension information4                      52
     *   <SLV-T>    C0h       46h       [7:0]    ITMCCDATA_ARK[31:24]       Extension information5                      53
     *   <SLV-T>    C0h       47h       [7:0]    ITMCCDATA_ARK[23:16]       Extension information6                      54
     *   <SLV-T>    C0h       48h       [7:0]    ITMCCDATA_ARK[15:8]        Extension information7                      55
     *   <SLV-T>    C0h       49h       [7:0]    ITMCCDATA_ARK[7:0]         Extension information8                      56
     */

    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, data, 57);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    SLVT_UnFreezeReg (pDemod);

    /* Transmission Mode1-4 Modulation, Code rate and Slot Number bit0 - bit7 */
    for (i = 0; i < 4; i++) {
        pTMCCInfo->modcodSlotInfo[i].modCod = (sony_isdbs_modcod_t) (data[(2 * i)] & 0x0F);
        pTMCCInfo->modcodSlotInfo[i].slotNum = (data[(2 * i + 1)] & 0x3F);
    }

    /* Relative TS ID for each slot bit8 - bit31*/
    for (i = 0;i < 24; i++) {
        pTMCCInfo->relativeTSForEachSlot[2 * i] =      ((data[i + 8] >> 4) & 0x07);
        pTMCCInfo->relativeTSForEachSlot[(2 * i) + 1] = (data[i + 8] & 0x07);
    }

    /* Corresponding table between relative TS and TSID bit32- bit47 */
    for (i = 0;i < 8; i++) {
        pTMCCInfo->tsidForEachRelativeTS[i] = (uint16_t)(((uint32_t)data[(2 * i) + 32] << 8) | data[(2 * i + 1) + 32]);
    }

    pTMCCInfo->ewsFlag = (uint8_t)((data[48] & 0x20) ? 1 : 0);
    pTMCCInfo->uplinkInfo = (uint8_t)((data[48] & 0x1E) >> 1);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_SlotNum (sony_demod_t * pDemod,
                                                uint8_t * pSlotNumH, uint8_t * pSlotNumL)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[2];
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_SlotNum");

    if ((!pDemod) || (!pSlotNumH) || (!pSlotNumL)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*   Slave     Bank      Addr      Bit       Signal name
     * -------------------------------------------------------------
     *  <SLV-T>    C0h       8Bh       [5:0]     ISSLTNH[5:0]
     *  <SLV-T>    C0h       8Ch       [5:0]     ISSLTNL[5:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x8B, rdata, 2) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pSlotNumH = rdata[0] & 0x3F;
    *pSlotNumL = rdata[1] & 0x3F;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_SiteDiversityInfo (sony_demod_t * pDemod,
                                                          uint8_t * pSiteDiversityInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_SiteDiversityInfo");

    if ((!pDemod) || (!pSiteDiversityInfo)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*    Slave     Bank      Addr      Bit       Signal name
     *    -------------------------------------------------------------
     *    <SLV-T>    C0h       4Bh       [1]       ITMCCDATA[65]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pSiteDiversityInfo =  (uint8_t)((rdata & 0x02) ? 1 : 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs_monitor_EWSChange (sony_demod_t * pDemod,
                                                  uint8_t * pEWSChange)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_EWSChange");

    if ((!pDemod) || (!pEWSChange)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked (pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*    Slave     Bank      Addr      Bit       Signal name
     *    -------------------------------------------------------------
     *    <SLV-T>    C0h       4Bh       [2]       IEWS
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pEWSChange =  (uint8_t)((rdata & 0x04) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbs_monitor_TMCCChange (sony_demod_t * pDemod,
                                                   uint8_t * pTMCCChange)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_TMCCChange");

    if ((!pDemod) || (!pTMCCChange)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*  Slave     Bank      Addr      Bit       Signal name
     *  -------------------------------------------------------------
     *  <SLV-T>    C0h       4Bh       [0]       ITMCCCHG
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);
    *pTMCCChange =  (uint8_t)((rdata & 0x01) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}
sony_result_t sony_demod_isdbs_ClearEWSChange (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_isdbs_ClearEWSChange");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*  Slave   Bank    Addr    Bit    Default     Setting     Signal name
     *  --------------------------------------------------------------------
     *  <SLV-T>  C0h     EDh     [1]    1'b0        1'b1        OTMCCEWSCLR
     */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xED, 0x02);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}
sony_result_t sony_demod_isdbs_ClearTMCCChange (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_isdbs_ClearTMCCChange");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xC0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*  Slave   Bank    Addr    Bit    Default     Setting     Signal name
     *  --------------------------------------------------------------------
     *  <SLV-T>  C0h     EDh     [0]    1'b0        1'b1        OTMCCCHGCLR
     */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xED, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbs_monitor_TSIDError (sony_demod_t * pDemod, uint8_t * pTSIDError)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs_monitor_TSIDError");

    if ((!pDemod) || (!pTSIDError)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = IsDmdLocked(pDemod);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }
    /* Set SLV-T Bank : 0xC9 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xC9);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*   slave   Bank    Addr    Bit     Signal name             Meaning
     *  -------------------------------------------------------------------------
     *   <SLV-T>  C9h     10h     [7]     ITMFLG1_ARK        0:match, 1:unmatch
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);
    *pTSIDError =  (uint8_t)((rdata & 0x80) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t IsDmdLocked(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t agcLock = 0;
    uint8_t tsLock  = 0;
    uint8_t tmccLock  = 0;
    SONY_TRACE_ENTER("IsDmdLocked");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_isdbs_monitor_SyncStat(pDemod, &agcLock, &tsLock, &tmccLock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (tmccLock) {
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    } else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }
}

