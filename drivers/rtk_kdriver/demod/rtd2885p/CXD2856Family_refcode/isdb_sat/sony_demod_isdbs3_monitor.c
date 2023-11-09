/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/

#include "sony_math.h"
#include "sony_demod_isdbs3.h"
#include "sony_demod_isdbs3_monitor.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
/*
 @brief Confirm demodulator lock
*/
static sony_result_t IsDmdLocked(sony_demod_t* pDemod);

/*
 @brief Freeze TMCC information
*/
static sony_result_t FreezeTMCCInfo(sony_demod_t* pDemod);

/*
 @brief Unfreeze TMCC information
*/
static sony_result_t UnfreezeTMCCInfo(sony_demod_t* pDemod);

/*
 @brief Confirm TMCC Read OK
*/
static sony_result_t IsTMCCReadOK(sony_demod_t* pDemod);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_isdbs3_monitor_SyncStat (sony_demod_t * pDemod,
                                                  uint8_t * pAGCLockStat,
                                                  uint8_t * pTSTLVLockStat,
                                                  uint8_t * pTMCCLockStat)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2];
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_SyncStat");

    if ((!pDemod) || (!pAGCLockStat) || (!pTSTLVLockStat) || (!pTMCCLockStat)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
      /*   slave     Bank      Addr      Bit      Signal name
       *  --------------------------------------------------------------
       *  <SLV-T>    A0h       10h       [5]      IRFAGC_LOCK
       *  <SLV-T>    A0h       11h       [6]      ITSTLVLOCK_ARK2
       *  <SLV-T>    A0h       11h       [7]      ITMCCLOCK_ARK2
       */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 2) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pAGCLockStat = (uint8_t)((data[0] & 0x20) ? 1 : 0);
    *pTSTLVLockStat = (uint8_t)((data[1] & 0x40) ? 1 : 0);
    *pTMCCLockStat = (uint8_t)((data[1] & 0x80) ? 1 : 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                       int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[3];
    uint32_t regValue = 0;
    int32_t cfrl_ctrlval = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_CarrierOffset");

    if ((!pDemod) || (!pOffset)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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
      samplingRate = 96000(KHz)

      Offset(KHz) = cfrl_ctrlval * (-1) * 96000 / 2^20
                  = cfrl_ctrlval * (-1) * 375 * 2^8 / 2^20
                  = cfrl_ctrlval * (-1) * 375 / 2^12
    ----------------------------------------------------------------*/
    if(cfrl_ctrlval > 0){
        *pOffset = ((cfrl_ctrlval * (-375)) - 2048) / 4096;
    } else {
        *pOffset = ((cfrl_ctrlval * (-375)) + 2048) / 4096;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                  uint32_t * pIFAGCOut)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGCOut)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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


sony_result_t sony_demod_isdbs3_monitor_IQSense (sony_demod_t * pDemod,
                                                 sony_demod_sat_iq_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;

    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_IQSense");

    if ((!pDemod) || (!pSense)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xAB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    ABh       23h       [0]      ICFRL_FIQINV
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x23, &rdata, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pSense = (rdata & 0x01) ? SONY_DEMOD_SAT_IQ_SENSE_INV : SONY_DEMOD_SAT_IQ_SENSE_NORMAL;

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_isdbs3_monitor_CNR (sony_demod_t * pDemod,
                                             int32_t * pCNR)
{
    uint8_t rdata[4] = {0x00, 0x00, 0x00, 0x00};
    uint32_t value = 0;
    int32_t index = 0;
    int32_t minIndex = 0;
    int32_t maxIndex = 0;
    const struct {
        uint32_t value;
        int32_t cnr_x1000;
    } isdbs3_cn_data[] = {
        {0x10da5, -4000},
        {0x107dd, -3900},
        {0x1023c, -3800},
        {0x0fcc1, -3700},
        {0x0f76b, -3600},
        {0x0f237, -3500},
        {0x0ed25, -3400},
        {0x0e834, -3300},
        {0x0e361, -3200},
        {0x0deab, -3100},
        {0x0da13, -3000},
        {0x0d596, -2900},
        {0x0d133, -2800},
        {0x0cceb, -2700},
        {0x0c8bb, -2600},
        {0x0c4a4, -2500},
        {0x0c0a3, -2400},
        {0x0bcba, -2300},
        {0x0b8e6, -2200},
        {0x0b527, -2100},
        {0x0b17d, -2000},
        {0x0ade6, -1900},
        {0x0aa63, -1800},
        {0x0a6f3, -1700},
        {0x0a395, -1600},
        {0x0a049, -1500},
        {0x09d0e, -1400},
        {0x099e4, -1300},
        {0x096cb, -1200},
        {0x093c1, -1100},
        {0x090c7, -1000},
        {0x08ddc, -900},
        {0x08b00, -800},
        {0x08833, -700},
        {0x08573, -600},
        {0x082c2, -500},
        {0x0801e, -400},
        {0x07d87, -300},
        {0x07afd, -200},
        {0x0787f, -100},
        {0x0760c, 0},
        {0x073a9, 100},
        {0x0714f, 200},
        {0x06f02, 300},
        {0x06cbf, 400},
        {0x06a88, 500},
        {0x0685b, 600},
        {0x06639, 700},
        {0x06421, 800},
        {0x06214, 900},
        {0x06010, 1000},
        {0x05e16, 1100},
        {0x05c26, 1200},
        {0x05a40, 1300},
        {0x05862, 1400},
        {0x0568e, 1500},
        {0x054c2, 1600},
        {0x05300, 1700},
        {0x05146, 1800},
        {0x04f94, 1900},
        {0x04deb, 2000},
        {0x04c49, 2100},
        {0x04ab0, 2200},
        {0x0491f, 2300},
        {0x04795, 2400},
        {0x04613, 2500},
        {0x04498, 2600},
        {0x04325, 2700},
        {0x041b8, 2800},
        {0x04053, 2900},
        {0x03ef5, 3000},
        {0x03d9e, 3100},
        {0x03c4d, 3200},
        {0x03b02, 3300},
        {0x039bf, 3400},
        {0x03881, 3500},
        {0x0374a, 3600},
        {0x03619, 3700},
        {0x034ee, 3800},
        {0x033c9, 3900},
        {0x032aa, 4000},
        {0x03191, 4100},
        {0x0307d, 4200},
        {0x02f6f, 4300},
        {0x02e66, 4400},
        {0x02d62, 4500},
        {0x02c64, 4600},
        {0x02b6b, 4700},
        {0x02a77, 4800},
        {0x02988, 4900},
        {0x0289e, 5000},
        {0x027b9, 5100},
        {0x026d9, 5200},
        {0x025fd, 5300},
        {0x02526, 5400},
        {0x02453, 5500},
        {0x02385, 5600},
        {0x022bb, 5700},
        {0x021f5, 5800},
        {0x02134, 5900},
        {0x02076, 6000},
        {0x01fbd, 6100},
        {0x01f08, 6200},
        {0x01e56, 6300},
        {0x01da9, 6400},
        {0x01cff, 6500},
        {0x01c58, 6600},
        {0x01bb5, 6700},
        {0x01b16, 6800},
        {0x01a7b, 6900},
        {0x019e2, 7000},
        {0x0194d, 7100},
        {0x018bc, 7200},
        {0x0182d, 7300},
        {0x017a2, 7400},
        {0x0171a, 7500},
        {0x01695, 7600},
        {0x01612, 7700},
        {0x01593, 7800},
        {0x01517, 7900},
        {0x0149d, 8000},
        {0x01426, 8100},
        {0x013b2, 8200},
        {0x01340, 8300},
        {0x012d1, 8400},
        {0x01264, 8500},
        {0x011fa, 8600},
        {0x01193, 8700},
        {0x0112d, 8800},
        {0x010ca, 8900},
        {0x01069, 9000},
        {0x0100b, 9100},
        {0x00fae, 9200},
        {0x00f54, 9300},
        {0x00efb, 9400},
        {0x00ea5, 9500},
        {0x00e51, 9600},
        {0x00dff, 9700},
        {0x00dae, 9800},
        {0x00d60, 9900},
        {0x00d13, 10000},
        {0x00cc8, 10100},
        {0x00c7e, 10200},
        {0x00c37, 10300},
        {0x00bf1, 10400},
        {0x00bac, 10500},
        {0x00b6a, 10600},
        {0x00b28, 10700},
        {0x00ae8, 10800},
        {0x00aaa, 10900},
        {0x00a6d, 11000},
        {0x00a32, 11100},
        {0x009f8, 11200},
        {0x009bf, 11300},
        {0x00987, 11400},
        {0x00951, 11500},
        {0x0091c, 11600},
        {0x008e8, 11700},
        {0x008b6, 11800},
        {0x00884, 11900},
        {0x00854, 12000},
        {0x00825, 12100},
        {0x007f7, 12200},
        {0x007ca, 12300},
        {0x0079e, 12400},
        {0x00773, 12500},
        {0x00749, 12600},
        {0x00720, 12700},
        {0x006f8, 12800},
        {0x006d0, 12900},
        {0x006aa, 13000},
        {0x00685, 13100},
        {0x00660, 13200},
        {0x0063c, 13300},
        {0x00619, 13400},
        {0x005f7, 13500},
        {0x005d6, 13600},
        {0x005b5, 13700},
        {0x00595, 13800},
        {0x00576, 13900},
        {0x00558, 14000},
        {0x0053a, 14100},
        {0x0051d, 14200},
        {0x00501, 14300},
        {0x004e5, 14400},
        {0x004c9, 14500},
        {0x004af, 14600},
        {0x00495, 14700},
        {0x0047b, 14800},
        {0x00462, 14900},
        {0x0044a, 15000},
        {0x00432, 15100},
        {0x0041b, 15200},
        {0x00404, 15300},
        {0x003ee, 15400},
        {0x003d8, 15500},
        {0x003c3, 15600},
        {0x003ae, 15700},
        {0x0039a, 15800},
        {0x00386, 15900},
        {0x00373, 16000},
        {0x00360, 16100},
        {0x0034d, 16200},
        {0x0033b, 16300},
        {0x00329, 16400},
        {0x00318, 16500},
        {0x00307, 16600},
        {0x002f6, 16700},
        {0x002e6, 16800},
        {0x002d6, 16900},
        {0x002c7, 17000},
        {0x002b7, 17100},
        {0x002a9, 17200},
        {0x0029a, 17300},
        {0x0028c, 17400},
        {0x0027e, 17500},
        {0x00270, 17600},
        {0x00263, 17700},
        {0x00256, 17800},
        {0x00249, 17900},
        {0x0023d, 18000},
        {0x00231, 18100},
        {0x00225, 18200},
        {0x00219, 18300},
        {0x0020e, 18400},
        {0x00203, 18500},
        {0x001f8, 18600},
        {0x001ed, 18700},
        {0x001e3, 18800},
        {0x001d9, 18900},
        {0x001cf, 19000},
        {0x001c5, 19100},
        {0x001bc, 19200},
        {0x001b2, 19300},
        {0x001a9, 19400},
        {0x001a0, 19500},
        {0x00198, 19600},
        {0x0018f, 19700},
        {0x00187, 19800},
        {0x0017f, 19900},
        {0x00177, 20000},
        {0x0016f, 20100},
        {0x00168, 20200},
        {0x00160, 20300},
        {0x00159, 20400},
        {0x00152, 20500},
        {0x0014b, 20600},
        {0x00144, 20700},
        {0x0013e, 20800},
        {0x00137, 20900},
        {0x00131, 21000},
        {0x0012b, 21100},
        {0x00125, 21200},
        {0x0011f, 21300},
        {0x00119, 21400},
        {0x00113, 21500},
        {0x0010e, 21600},
        {0x00108, 21700},
        {0x00103, 21800},
        {0x000fe, 21900},
        {0x000f9, 22000},
        {0x000f4, 22100},
        {0x000ef, 22200},
        {0x000eb, 22300},
        {0x000e6, 22400},
        {0x000e2, 22500},
        {0x000de, 22600},
        {0x000da, 22700},
        {0x000d5, 22800},
        {0x000d1, 22900},
        {0x000cd, 23000},
        {0x000ca, 23100},
        {0x000c6, 23200},
        {0x000c2, 23300},
        {0x000be, 23400},
        {0x000bb, 23500},
        {0x000b7, 23600},
        {0x000b4, 23700},
        {0x000b1, 23800},
        {0x000ae, 23900},
        {0x000aa, 24000},
        {0x000a7, 24100},
        {0x000a4, 24200},
        {0x000a2, 24300},
        {0x0009f, 24400},
        {0x0009c, 24500},
        {0x00099, 24600},
        {0x00097, 24700},
        {0x00094, 24800},
        {0x00092, 24900},
        {0x0008f, 25000},
        {0x0008d, 25100},
        {0x0008b, 25200},
        {0x00088, 25300},
        {0x00086, 25400},
        {0x00084, 25500},
        {0x00082, 25600},
        {0x00080, 25700},
        {0x0007e, 25800},
        {0x0007c, 25900},
        {0x0007a, 26000},
        {0x00078, 26100},
        {0x00076, 26200},
        {0x00074, 26300},
        {0x00073, 26400},
        {0x00071, 26500},
        {0x0006f, 26600},
        {0x0006d, 26700},
        {0x0006c, 26800},
        {0x0006a, 26900},
        {0x00069, 27000},
        {0x00067, 27100},
        {0x00066, 27200},
        {0x00064, 27300},
        {0x00063, 27400},
        {0x00061, 27500},
        {0x00060, 27600},
        {0x0005f, 27700},
        {0x0005d, 27800},
        {0x0005c, 27900},
        {0x0005b, 28000},
        {0x0005a, 28100},
        {0x00059, 28200},
        {0x00057, 28300},
        {0x00056, 28400},
        {0x00055, 28500},
        {0x00054, 28600},
        {0x00053, 28700},
        {0x00052, 28800},
        {0x00051, 28900},
        {0x00050, 29000},
        {0x0004f, 29100},
        {0x0004e, 29200},
        {0x0004d, 29300},
        {0x0004c, 29400},
        {0x0004b, 29500},
        {0x0004a, 29600},
        {0x00049, 29700},
        {0x00048, 29900},
        {0x00047, 30000},
    };

    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_CNR");

    if ((!pDemod) || (!pCNR)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit     Signal name
     * --------------------------------------------------------------
     *  <SLV-T>    D0h       F3h       [0]     ICPM_FINERDY
     *  <SLV-T>    D0h       F4h       [0]     ICPM_FINECNDT[16]
     *  <SLV-T>    D0h       F5h       [7:0]   ICPM_FINECNDT[15:8]
     *  <SLV-T>    D0h       F6h       [7:0]   ICPM_FINECNDT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF3, rdata, 4) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((rdata[0] & 0x01) == 0) {
        /* C/N monitor calculation is not completed */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    value = (uint32_t)((((uint32_t)rdata[1] & 0x01) << 16) | ((uint32_t)rdata[2] << 8) | rdata[3]);

    minIndex = 0;
    maxIndex = sizeof(isdbs3_cn_data)/sizeof(isdbs3_cn_data[0]) - 1;

    /* Cropping */
    if(value >= isdbs3_cn_data[minIndex].value) {
        *pCNR = isdbs3_cn_data[minIndex].cnr_x1000;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }
    if(value <= isdbs3_cn_data[maxIndex].value) {
        *pCNR = isdbs3_cn_data[maxIndex].cnr_x1000;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    /* Binary Search */
    while (1) {
        index = (maxIndex + minIndex) / 2;

        if (value == isdbs3_cn_data[index].value) {
            *pCNR = isdbs3_cn_data[index].cnr_x1000;
            break;
        } else if (value > isdbs3_cn_data[index].value) {
            maxIndex = index;
        } else {
            minIndex = index;
        }

        if ((maxIndex - minIndex) <= 1) {
            if (value == isdbs3_cn_data[maxIndex].value) {
                *pCNR = isdbs3_cn_data[maxIndex].cnr_x1000;
                break;
            } else {
                *pCNR = isdbs3_cn_data[minIndex].cnr_x1000;
                break;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs3_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                    uint32_t * pBERH, uint32_t * pBERL)
{
    uint8_t rdata[4];
    uint8_t valid[2];
    uint32_t bitError[2];
    uint32_t measuredPeriod[2];
    uint32_t * pBER[2];
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    int i = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_PreLDPCBER");

    if ((!pDemod) || (!pBERH) || (!pBERL)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *   slave     Bank      Addr      Bit      Signal name        Comment
     * ---------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       C0h       [0]      IFLBER_VALID0_K2   (for Transmission mode 1)
     *  <SLV-T>    D0h       C0h       [1]      IFLBER_VALID1_K2   (for Transmission mode 2)
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC0, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    valid[0] = (rdata[0] & 0x01) ? 1 : 0; /* Transmission mode 1(First layer) */
    valid[1] = (rdata[0] & 0x02) ? 1 : 0; /* Transmission mode 2(Second layer) */

    /*
     *  slave     Bank      Addr      Bit      Signal name                 Comment
     * ---------------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       C1h       [3:0]    IFLBER_BITERR0_K2[27:24]  (for Transmission mode 1)
     *  <SLV-T>    D0h       C2h       [7:0]    IFLBER_BITERR0_K2[23:16]  (for Transmission mode 1)
     *  <SLV-T>    D0h       C3h       [7:0]    IFLBER_BITERR0_K2[15:8]   (for Transmission mode 1)
     *  <SLV-T>    D0h       C4h       [7:0]    IFLBER_BITERR0_K2[7:0]    (for Transmission mode 1)
     *  <SLV-T>    D0h       C5h       [3:0]    IFLBER_BITERR1_K2[27:24]  (for Transmission mode 2)
     *  <SLV-T>    D0h       C6h       [7:0]    IFLBER_BITERR1_K2[23:16]  (for Transmission mode 2)
     *  <SLV-T>    D0h       C7h       [7:0]    IFLBER_BITERR1_K2[15:8]   (for Transmission mode 2)
     *  <SLV-T>    D0h       C8h       [7:0]    IFLBER_BITERR1_K2[7:0]    (for Transmission mode 2)
     *
     *  slave     Bank    Addr    Bit     Default    Setting       Signal name           Comment
     * -----------------------------------------------------------------------------------------------------------
     *  <SLV-T>    D0h     D0h     [3:0]    8'h08      period       OFLBER_MES0_K2[3:0]  (for Transmission mode 1)
     *  <SLV-T>    D0h     D1h     [3:0]    8'h08      period       OFLBER_MES1_K2[3:0]  (for Transmission mode 2)
     */

    /* Transmission mode 1 */
    if (valid[0]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC1, rdata, 4) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Bit error count */
        bitError[0] = ((uint32_t)(rdata[0] & 0x0F) << 24) |
                      ((uint32_t)(rdata[1] & 0xFF) << 16) |
                      ((uint32_t)(rdata[2] & 0xFF) <<  8) |
                      (uint32_t)(rdata[3] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD0, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        measuredPeriod[0] = (uint32_t)(1 << (rdata[0] & 0x0F));
    }

    /* Transmission mode 2 */
    if (valid[1]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC5, rdata, 4) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Bit error count */
        bitError[1] = ((uint32_t)(rdata[0] & 0x0F) << 24) |
                      ((uint32_t)(rdata[1] & 0xFF) << 16) |
                      ((uint32_t)(rdata[2] & 0xFF) <<  8) |
                      (uint32_t)(rdata[3] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD1, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        measuredPeriod[1] = (uint32_t)(1 << (rdata[0] & 0x0F));
    }

    SLVT_UnFreezeReg (pDemod);

    pBER[0] = pBERH;
    pBER[1] = pBERL;

    for (i = 0; i < 2; i++) {
        if (!valid[i]) {
            *pBER[i] = SONY_DEMOD_ISDBS3_MONITOR_PRELDPCBER_INVALID;
            continue;
        }

        if (bitError[i] > (measuredPeriod[i] * 44880)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        /*--------------------------------------------------------------------
          BER = bitError / (period * 44880)
              = (bitError * 10^7) / (period * 44880)
              = (bitError * 10^6) / (period * 4488)
              = (bitError * 125000) / (period * 561)
              = (bitError * 10) * 125 * 100 / (period * 561)
        --------------------------------------------------------------------*/
        tempDiv = measuredPeriod[i] * 561;
        if (tempDiv == 0){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (bitError[i] * 10) / tempDiv;
        tempR = (bitError[i] * 10) % tempDiv;

        tempR *= 125;
        tempQ = (tempQ * 125) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if (tempR >= (tempDiv/2)){
            *pBER[i] = tempQ + 1;
        } else {
            *pBER[i] = tempQ;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs3_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                   uint32_t * pBERH, uint32_t * pBERL)
{
    uint8_t rdata[3];
    uint8_t valid[2];
    uint32_t bitError[2];
    uint32_t periodExp[2];
    uint32_t * pBER[2];
    sony_isdbs3_cod_t cod[2];
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    uint32_t tempA = 0;
    uint32_t tempB = 0;
    int i = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_PreBCHBER");

    if ((!pDemod) || (!pBERL) || (!pBERH)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *  slave     Bank      Addr      Bit      Signal name         Comment
     * --------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       A0h       [0]      IF2BER_VALID0_K2   (for Transmission mode 1)
     *  <SLV-T>    D0h       A0h       [1]      IF2BER_VALID1_K2   (for Transmission mode 2)
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA0, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    valid[0] = (rdata[0] & 0x01) ? 1 : 0; /* Transmission mode 1(First layer) */
    valid[1] = (rdata[0] & 0x02) ? 1 : 0; /* Transmission mode 2(Second layer) */

    /*
     *  slave     Bank      Addr      Bit      Signal name                   Comment
     * -----------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       A1h       [6:0]    IF2BER_BITERR0_K2[22:16]     (for Transmission mode 1)
     *  <SLV-T>    D0h       A2h       [7:0]    IF2BER_BITERR0_K2[15:8]      (for Transmission mode 1)
     *  <SLV-T>    D0h       A3h       [7:0]    IF2BER_BITERR0_K2[7:0]       (for Transmission mode 1)
     *  <SLV-T>    D0h       A4h       [6:0]    IF2BER_BITERR1_K2[22:16]     (for Transmission mode 2)
     *  <SLV-T>    D0h       A5h       [7:0]    IF2BER_BITERR1_K2[15:8]      (for Transmission mode 2)
     *  <SLV-T>    D0h       A6h       [7:0]    IF2BER_BITERR1_K2[7:0]       (for Transmission mode 2)
     *
     *  slave     Bank    Addr    Bit     Default    Setting       Signal name           Comment
     * ----------------------------------------------------------------------------------------------------------
     *  <SLV-T>    D0h     B3h     [3:0]    8'h08      period       OF2BER_MES0_K2[3:0]  (for Transmission mode 1)
     *  <SLV-T>    D0h     B4h     [3:0]    8'h08      period       OF2BER_MES1_K2[3:0]  (for Transmission mode 2)
     */

    /* Transmission mode 1 */
    if (valid[0]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA1, rdata, 3) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Bit error count */
        bitError[0] = ((uint32_t)(rdata[0] & 0x7F) << 16) |
                      ((uint32_t)(rdata[1] & 0xFF) <<  8) |
                      (uint32_t)(rdata[2] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB3, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        periodExp[0] = (uint32_t)(rdata[0] & 0x0F);
    }

    /* Transmission mode 2 */
    if (valid[1]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA4, rdata, 3) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Bit error count */
        bitError[1] = ((uint32_t)(rdata[0] & 0x7F) << 16) |
                      ((uint32_t)(rdata[1] & 0xFF) <<  8) |
                      (uint32_t)(rdata[2] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB4, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        periodExp[1] = (uint32_t)(rdata[0] & 0x0F);
    }

    /*
     *   Slave     Bank      Addr      Bit       Signal name               Comment
     * ---------------------------------------------------------------------------------------------
     * <SLV-T>    D0h       ABh       [6:0]     IF2BER_MDCD0_ARK2[6:0]     (for Transmission mode 1)
     * <SLV-T>    D0h       ACh       [6:0]     IF2BER_MDCD1_ARK2[6:0]     (for Transmission mode 2)
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xAB, rdata, 2) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    /* Code rate */
    cod[0] = (sony_isdbs3_cod_t)(rdata[0] & 0x0F);
    cod[1] = (sony_isdbs3_cod_t)(rdata[1] & 0x0F);

    pBER[0] = pBERH;
    pBER[1] = pBERL;

    for (i = 0; i < 2; i++) {
        if (!valid[i]) {
            *pBER[i] = SONY_DEMOD_ISDBS3_MONITOR_PREBCHBER_INVALID;
            continue;
        }

        switch(cod[i])
        {
        case SONY_ISDBS3_COD_41_120:  tempA =  41; tempB = 120; break;
        case SONY_ISDBS3_COD_49_120:  tempA =  49; tempB = 120; break;
        case SONY_ISDBS3_COD_61_120:  tempA =  61; tempB = 120; break;
        case SONY_ISDBS3_COD_73_120:  tempA =  73; tempB = 120; break;
        case SONY_ISDBS3_COD_81_120:  tempA =  81; tempB = 120; break;
        case SONY_ISDBS3_COD_89_120:  tempA =  89; tempB = 120; break;
        case SONY_ISDBS3_COD_93_120:  tempA =  93; tempB = 120; break;
        case SONY_ISDBS3_COD_97_120:  tempA =  97; tempB = 120; break;
        case SONY_ISDBS3_COD_101_120: tempA = 101; tempB = 120; break;
        case SONY_ISDBS3_COD_105_120: tempA = 105; tempB = 120; break;
        case SONY_ISDBS3_COD_109_120: tempA = 109; tempB = 120; break;
        default: SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        if(periodExp[i] >= 10) {
            /*--------------------------------------------------------------------
              BER = bitError / (2^N * 44880 * codeRate)       (codeRate = A/B)
                  = (bitError * 10^9) / (2^N * 44880 * A/B)
                  = (bitError * 10^8 * B) / (2^N * 4488 * A)
                  = (bitError * 12500000 * B) / (2^N * 561 * A)
                  (NOTE: 12500000 / 2^5 = 390625)
                  = (bitError * B * 390625) / (2^(N-5) * 561 * A)
                  = ((bitError * B) * 25 * 25 * 25 * 25) / (2^(N-5) * 561 * A)
            --------------------------------------------------------------------*/
            tempDiv = (1 << (periodExp[i] - 5)) * 561 * tempA;
            if (tempDiv == 0){
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
            }

            tempQ = (bitError[i] * tempB) / tempDiv;
            tempR = (bitError[i] * tempB) % tempDiv;

            tempR *= 25;
            tempQ = (tempQ * 25) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 25;
            tempQ = (tempQ * 25) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 25;
            tempQ = (tempQ * 25) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 25;
            tempQ = (tempQ * 25) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            if (tempR >= (tempDiv/2)){
                *pBER[i] = tempQ + 1;
            } else {
                *pBER[i] = tempQ;
            }
        }
        else {
            /*--------------------------------------------------------------------
              BER = bitError / (2^N * 44880 * codeRate)       (codeRate = A/B)
                  = (bitError * 10^9) / (2^N * 44880 * A/B)
                  = (bitError * 10^8 * B) / (2^N * 4488 * A)
                  = (bitError * 12500000 * B) / (2^N * 561 * A)
                  = ((bitError * B) * 125 * 100 * 100 * 10) / (2^N * 561 * A)
            --------------------------------------------------------------------*/
            tempDiv = (1 << periodExp[i]) * 561 * tempA;
            if (tempDiv == 0){
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
            }

            tempQ = (bitError[i] * tempB) / tempDiv;
            tempR = (bitError[i] * tempB) % tempDiv;

            tempR *= 125;
            tempQ = (tempQ * 125) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 100;
            tempQ = (tempQ * 100) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 100;
            tempQ = (tempQ * 100) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            tempR *= 10;
            tempQ = (tempQ * 10) + (tempR / tempDiv);
            tempR = tempR % tempDiv;

            if (tempR >= (tempDiv/2)){
                *pBER[i] = tempQ + 1;
            } else {
                *pBER[i] = tempQ;
            }
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs3_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                    uint32_t * pFERH, uint32_t * pFERL)
{
    uint8_t rdata[2];
    uint8_t valid[2];
    uint32_t frameError[2];
    uint32_t frameCount[2];
    uint32_t * pFER[2];
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    int i = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_PostBCHFER");

    if ((!pDemod) || (!pFERL)|| (!pFERH)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *  slave     Bank      Addr      Bit      Signal name          Comment
     * ---------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       A0h       [0]      IF2BER_VALID0_K2    (for Transmission mode 1)
     *  <SLV-T>    D0h       A0h       [1]      IF2BER_VALID1_K2    (for Transmission mode 2)
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA0, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    valid[0] = (rdata[0] & 0x01) ? 1 : 0; /* Transmission mode 1(First layer) */
    valid[1] = (rdata[0] & 0x02) ? 1 : 0; /* Transmission mode 2(Second layer) */

    /*
     *  slave     Bank      Addr      Bit      Signal name                 Comment
     * ------------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       A7h       [7:0]    IF2BER_FRMERR0_K2[15:8]    (for Transmission mode 1)
     *  <SLV-T>    D0h       A8h       [7:0]    IF2BER_FRMERR0_K2[7:0]     (for Transmission mode 1)
     *  <SLV-T>    D0h       A9h       [7:0]    IF2BER_FRMERR1_K2[15:8]    (for Transmission mode 2)
     *  <SLV-T>    D0h       AAh       [7:0]    IF2BER_FRMERR1_K2[7:0]     (for Transmission mode 2)
     *
     *  slave     Bank    Addr    Bit     Default    Setting       Signal name            Comment
     * ------------------------------------------------------------------------------------------------------------
     *  <SLV-T>    D0h     B3h     [3:0]    8'h08      period       OF2BER_MES0_K2[3:0]   (for Transmission mode 1)
     *  <SLV-T>    D0h     B4h     [3:0]    8'h08      period       OF2BER_MES1_K2[3:0]   (for Transmission mode 2)
     */

    /* Transmission mode 1 */
    if (valid[0]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA7, rdata, 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Frame error count */
        frameError[0] = ((uint32_t)(rdata[0] & 0xFF) << 8) |
                        (uint32_t)(rdata[1] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB3, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        frameCount[0] = (uint32_t)(1 << (rdata[0] & 0x0F));
    }

    /* Transmission mode 2 */
    if (valid[1]){
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA9, rdata, 2) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Frame error count */
        frameError[1] = ((uint32_t)(rdata[0] & 0xFF) << 8) |
                        (uint32_t)(rdata[1] & 0xFF);

        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB4, rdata, 1) != SONY_RESULT_OK) {
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        frameCount[1] = (uint32_t)(1 << (rdata[0] & 0x0F));
    }

    SLVT_UnFreezeReg (pDemod);

    pFER[0] = pFERH;
    pFER[1] = pFERL;

    for (i = 0; i < 2; i++) {
        if (!valid[i]) {
            *pFER[i] = SONY_DEMOD_ISDBS3_MONITOR_POSTBCHFER_INVALID;
            continue;
        }

        /*--------------------------------------------------------------------
          FER = frameError / frameCount
              = (frameError * 10^6) / frameCount
              = ((frameError * 10000) * 100) / frameCount
        --------------------------------------------------------------------*/
        tempDiv = frameCount[i];
        if ((tempDiv == 0) || (frameError[i] > frameCount[i])){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (frameError[i] * 10000) / tempDiv;
        tempR = (frameError[i] * 10000) % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if ((tempDiv != 1) && (tempR >= (tempDiv/2))){
            *pFER[i] = tempQ + 1;
        } else {
            *pFER[i] = tempQ;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs3_monitor_ModCod (sony_demod_t * pDemod,
                                                sony_isdbs3_mod_t * pModH, sony_isdbs3_mod_t * pModL,
                                                sony_isdbs3_cod_t * pCodH, sony_isdbs3_cod_t * pCodL)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[2];
    uint8_t valid[2];

    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_ModCod");

    if ((!pDemod) || (!pModH) || (!pCodH) || (!pModL) || (!pCodL)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xD0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     *  slave     Bank      Addr      Bit      Signal name         Comment
     * --------------------------------------------------------------------------------------
     *  <SLV-T>    D0h       A0h       [0]      IF2BER_VALID0_K2   (for Transmission mode 1)
     *  <SLV-T>    D0h       A0h       [1]      IF2BER_VALID1_K2   (for Transmission mode 2)
     */

    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA0, rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    valid[0] = (rdata[0] & 0x01) ? 1 : 0; /* Transmission mode 1(First layer) */
    valid[1] = (rdata[0] & 0x02) ? 1 : 0; /* Transmission mode 2(Second layer) */

    /*
     *   Slave     Bank      Addr      Bit       Signal name               Comment
     * ---------------------------------------------------------------------------------------------
     * <SLV-T>    D0h       ABh       [6:0]     IF2BER_MDCD0_ARK2[6:0]     (for Transmission mode 1)
     * <SLV-T>    D0h       ACh       [6:0]     IF2BER_MDCD1_ARK2[6:0]     (for Transmission mode 2)
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xAB, rdata, 2) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    if (valid[0]){
        *pModH = (sony_isdbs3_mod_t)((rdata[0] >> 4) & 0x07);
        *pCodH = (sony_isdbs3_cod_t)(rdata[0] & 0x0F);
    } else {
        *pModH = SONY_ISDBS3_MOD_UNUSED_15;
        *pCodH = SONY_ISDBS3_COD_UNUSED_15;
    }
    if (valid[1]){
        *pModL = (sony_isdbs3_mod_t)((rdata[1] >> 4) & 0x07);
        *pCodL = (sony_isdbs3_cod_t)(rdata[1] & 0x0F);
    } else {
        *pModL = SONY_ISDBS3_MOD_UNUSED_15;
        *pCodL = SONY_ISDBS3_COD_UNUSED_15;
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_isdbs3_monitor_TMCCInfo(sony_demod_t * pDemod,
                                                 sony_isdbs3_tmcc_info_t * pTMCCInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[240] = {0x00};
    uint32_t i = 0;
    SONY_TRACE_ENTER("sony_demod_isdbs3_monitor_TMCCInfo");

    if ((!pDemod) || (!pTMCCInfo)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Check TMCC lock */
    result = IsDmdLocked(pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /* Freeze TMCC information */
    if (FreezeTMCCInfo (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Verify TMCC information read OK flag */
    if (IsTMCCReadOK (pDemod) != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0xDB */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xDB);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*
     *   slave     Bank      Addr      Bit      Signal name                 Meaning                                Series Num
     *  -----------------------------------------------------------------------------------------------------------------
     *   <SLV-T>    DBh       10h       [7:0]    ICHANGE_INST_ARK2[7:0]     TMCC change Counter                          0
     *   <SLV-T>    DBh       11h       [7:0]    IMODCOD1_ARK2[7:0]         Transmission Mode1 Modulation and Code rate  1
     *   <SLV-T>    DBh       12h       [7:0]    ISLOTNUM1_ARK2[7:0]        Transmission Mode1 Slot number               2
     *   <SLV-T>    DBh       13h       [7:0]    IBACKOFF1_ARK2[7:0]        Transmission Mode1 Backoff                   3
     *   <SLV-T>    DBh       14h       [7:0]    IMODCOD2_ARK2[7:0]         Transmission Mode2 Modulation and Code rate  4
     *   <SLV-T>    DBh       15h       [7:0]    ISLOTNUM2_ARK2[7:0]        Transmission Mode2 Slot number               5
     *   <SLV-T>    DBh       16h       [7:0]    IBACKOFF2_ARK2[7:0]        Transmission Mode2 Backoff                   6
     *   <SLV-T>    DBh       17h       [7:0]    IMODCOD3_ARK2[7:0]         ...                                          7
     *   <SLV-T>    DBh       18h       [7:0]    ISLOTNUM3_ARK2[7:0]                                                     8
     *   <SLV-T>    DBh       19h       [7:0]    IBACKOFF3_ARK2[7:0]                                                     9
     *   <SLV-T>    DBh       1Ah       [7:0]    IMODCOD4_ARK2[7:0]                                                     10
     *   <SLV-T>    DBh       1Bh       [7:0]    ISLOTNUM4_ARK2[7:0]                                                    11
     *   <SLV-T>    DBh       1Ch       [7:0]    IBACKOFF4_ARK2[7:0]                                                    12
     *   <SLV-T>    DBh       1Dh       [7:0]    IMODCOD5_ARK2[7:0]                                                     13
     *   <SLV-T>    DBh       1Eh       [7:0]    ISLOTNUM5_ARK2[7:0]                                                    14
     *   <SLV-T>    DBh       1Fh       [7:0]    IBACKOFF5_ARK2[7:0]                                                    15
     *   <SLV-T>    DBh       20h       [7:0]    IMODCOD6_ARK2[7:0]                                                     16
     *   <SLV-T>    DBh       21h       [7:0]    ISLOTNUM6_ARK2[7:0]                                                    17
     *   <SLV-T>    DBh       22h       [7:0]    IBACKOFF6_ARK2[7:0]                                                    18
     *   <SLV-T>    DBh       23h       [7:0]    IMODCOD7_ARK2[7:0]                                                     19
     *   <SLV-T>    DBh       24h       [7:0]    ISLOTNUM7_ARK2[7:0]                                                    20
     *   <SLV-T>    DBh       25h       [7:0]    IBACKOFF7_ARK2[7:0]                                                    21
     *   <SLV-T>    DBh       26h       [7:0]    IMODCOD8_ARK2[7:0]                                                     22
     *   <SLV-T>    DBh       27h       [7:0]    ISLOTNUM8_ARK2[7:0]                                                    23
     *   <SLV-T>    DBh       28h       [7:0]    IBACKOFF8_ARK2[7:0]                                                    24
     *   <SLV-T>    DBh       29h       [7:0]    ISTREAM_TYPE0_ARK2[7:0]                                                25
     *   <SLV-T>    DBh       2Ah       [7:0]    ISTREAM_TYPE1_ARK2[7:0]                                                26
     *   <SLV-T>    DBh       2Bh       [7:0]    ISTREAM_TYPE2_ARK2[7:0]                                                27
     *   <SLV-T>    DBh       2Ch       [7:0]    ISTREAM_TYPE3_ARK2[7:0]                                                28
     *   <SLV-T>    DBh       2Dh       [7:0]    ISTREAM_TYPE4_ARK2[7:0]                                                29
     *   <SLV-T>    DBh       2Eh       [7:0]    ISTREAM_TYPE5_ARK2[7:0]                                                30
     *   <SLV-T>    DBh       2Fh       [7:0]    ISTREAM_TYPE6_ARK2[7:0]                                                31
     *   <SLV-T>    DBh       30h       [7:0]    ISTREAM_TYPE7_ARK2[7:0]                                                32
     *   <SLV-T>    DBh       31h       [7:0]    ISTREAM_TYPE8_ARK2[7:0]                                                33
     *   <SLV-T>    DBh       32h       [7:0]    ISTREAM_TYPE9_ARK2[7:0]                                                34
     *   <SLV-T>    DBh       33h       [7:0]    ISTREAM_TYPE10_ARK2[7:0]                                               35
     *   <SLV-T>    DBh       34h       [7:0]    ISTREAM_TYPE11_ARK2[7:0]                                               36
     *   <SLV-T>    DBh       35h       [7:0]    ISTREAM_TYPE12_ARK2[7:0]                                               37
     *   <SLV-T>    DBh       36h       [7:0]    ISTREAM_TYPE13_ARK2[7:0]                                               38
     *   <SLV-T>    DBh       37h       [7:0]    ISTREAM_TYPE14_ARK2[7:0]                                               39
     *   <SLV-T>    DBh       38h       [7:0]    ISTREAM_TYPE15_ARK2[7:0]                                               40
     *   <SLV-T>    DBh       39h       [7:0]    IPACKET_LENGTH0_ARK2[15:8]                                             41
     *   <SLV-T>    DBh       3Ah       [7:0]    IPACKET_LENGTH0_ARK2[7:0]                                              42
     *   <SLV-T>    DBh       3Bh       [7:0]    IPACKET_LENGTH1_ARK2[15:8]                                             43
     *   <SLV-T>    DBh       3Ch       [7:0]    IPACKET_LENGTH1_ARK2[7:0]                                              44
     *   <SLV-T>    DBh       3Dh       [7:0]    IPACKET_LENGTH2_ARK2[15:8]                                             45
     *   <SLV-T>    DBh       3Eh       [7:0]    IPACKET_LENGTH2_ARK2[7:0]                                              46
     *   <SLV-T>    DBh       3Fh       [7:0]    IPACKET_LENGTH3_ARK2[15:8]                                             47
     *   <SLV-T>    DBh       40h       [7:0]    IPACKET_LENGTH3_ARK2[7:0]                                              48
     *   <SLV-T>    DBh       41h       [7:0]    IPACKET_LENGTH4_ARK2[15:8]                                             49
     *   <SLV-T>    DBh       42h       [7:0]    IPACKET_LENGTH4_ARK2[7:0]                                              50
     *   <SLV-T>    DBh       43h       [7:0]    IPACKET_LENGTH5_ARK2[15:8]                                             51
     *   <SLV-T>    DBh       44h       [7:0]    IPACKET_LENGTH5_ARK2[7:0]                                              52
     *   <SLV-T>    DBh       45h       [7:0]    IPACKET_LENGTH6_ARK2[15:8]                                             53
     *   <SLV-T>    DBh       46h       [7:0]    IPACKET_LENGTH6_ARK2[7:0]                                              54
     *   <SLV-T>    DBh       47h       [7:0]    IPACKET_LENGTH7_ARK2[15:8]                                             55
     *   <SLV-T>    DBh       48h       [7:0]    IPACKET_LENGTH7_ARK2[7:0]                                              56
     *   <SLV-T>    DBh       49h       [7:0]    IPACKET_LENGTH8_ARK2[15:8]                                             57
     *   <SLV-T>    DBh       4Ah       [7:0]    IPACKET_LENGTH8_ARK2[7:0]                                              58
     *   <SLV-T>    DBh       4Bh       [7:0]    IPACKET_LENGTH9_ARK2[15:8]                                             59
     *   <SLV-T>    DBh       4Ch       [7:0]    IPACKET_LENGTH9_ARK2[7:0]                                              60
     *   <SLV-T>    DBh       4Dh       [7:0]    IPACKET_LENGTH10_ARK2[15:8]                                            61
     *   <SLV-T>    DBh       4Eh       [7:0]    IPACKET_LENGTH10_ARK2[7:0]                                             62
     *   <SLV-T>    DBh       4Fh       [7:0]    IPACKET_LENGTH11_ARK2[15:8]                                            63
     *   <SLV-T>    DBh       50h       [7:0]    IPACKET_LENGTH11_ARK2[7:0]                                             64
     *   <SLV-T>    DBh       51h       [7:0]    IPACKET_LENGTH12_ARK2[15:8]                                            65
     *   <SLV-T>    DBh       52h       [7:0]    IPACKET_LENGTH12_ARK2[7:0]                                             66
     *   <SLV-T>    DBh       53h       [7:0]    IPACKET_LENGTH13_ARK2[15:8]                                            67
     *   <SLV-T>    DBh       54h       [7:0]    IPACKET_LENGTH13_ARK2[7:0]                                             68
     *   <SLV-T>    DBh       55h       [7:0]    IPACKET_LENGTH14_ARK2[15:8]                                            69
     *   <SLV-T>    DBh       56h       [7:0]    IPACKET_LENGTH14_ARK2[7:0]                                             70
     *   <SLV-T>    DBh       57h       [7:0]    IPACKET_LENGTH15_ARK2[15:8]                                            71
     *   <SLV-T>    DBh       58h       [7:0]    IPACKET_LENGTH15_ARK2[7:0]                                             72
     *   <SLV-T>    DBh       59h       [7:0]    IBIT_LENGTH0_ARK2[7:0]                                                 73
     *   <SLV-T>    DBh       5Ah       [7:0]    IBIT_LENGTH1_ARK2[7:0]                                                 74
     *   <SLV-T>    DBh       5Bh       [7:0]    IBIT_LENGTH2_ARK2[7:0]                                                 75
     *   <SLV-T>    DBh       5Ch       [7:0]    IBIT_LENGTH3_ARK2[7:0]                                                 76
     *   <SLV-T>    DBh       5Dh       [7:0]    IBIT_LENGTH4_ARK2[7:0]                                                 77
     *   <SLV-T>    DBh       5Eh       [7:0]    IBIT_LENGTH5_ARK2[7:0]                                                 78
     *   <SLV-T>    DBh       5Fh       [7:0]    IBIT_LENGTH6_ARK2[7:0]                                                 79
     *   <SLV-T>    DBh       60h       [7:0]    IBIT_LENGTH7_ARK2[7:0]                                                 80
     *   <SLV-T>    DBh       61h       [7:0]    IBIT_LENGTH8_ARK2[7:0]                                                 81
     *   <SLV-T>    DBh       62h       [7:0]    IBIT_LENGTH9_ARK2[7:0]                                                 82
     *   <SLV-T>    DBh       63h       [7:0]    IBIT_LENGTH10_ARK2[7:0]                                                83
     *   <SLV-T>    DBh       64h       [7:0]    IBIT_LENGTH11_ARK2[7:0]                                                84
     *   <SLV-T>    DBh       65h       [7:0]    IBIT_LENGTH12_ARK2[7:0]                                                85
     *   <SLV-T>    DBh       66h       [7:0]    IBIT_LENGTH13_ARK2[7:0]                                                86
     *   <SLV-T>    DBh       67h       [7:0]    IBIT_LENGTH14_ARK2[7:0]                                                87
     *   <SLV-T>    DBh       68h       [7:0]    IBIT_LENGTH15_ARK2[7:0]                                                88
     *   <SLV-T>    DBh       69h       [7:0]    ISYNC_PTN0_ARK2[31:24]                                                 89
     *   <SLV-T>    DBh       6Ah       [7:0]    ISYNC_PTN0_ARK2[23:16]                                                 90
     *   <SLV-T>    DBh       6Bh       [7:0]    ISYNC_PTN0_ARK2[15:8]                                                  91
     *   <SLV-T>    DBh       6Ch       [7:0]    ISYNC_PTN0_ARK2[7:0]                                                   92
     *   <SLV-T>    DBh       6Dh       [7:0]    ISYNC_PTN1_ARK2[31:24]                                                 93
     *   <SLV-T>    DBh       6Eh       [7:0]    ISYNC_PTN1_ARK2[23:16]                                                 94
     *   <SLV-T>    DBh       6Fh       [7:0]    ISYNC_PTN1_ARK2[15:8]                                                  95
     *   <SLV-T>    DBh       70h       [7:0]    ISYNC_PTN1_ARK2[7:0]                                                   96
     *   <SLV-T>    DBh       71h       [7:0]    ISYNC_PTN2_ARK2[31:24]                                                 97
     *   <SLV-T>    DBh       72h       [7:0]    ISYNC_PTN2_ARK2[23:16]                                                 98
     *   <SLV-T>    DBh       73h       [7:0]    ISYNC_PTN2_ARK2[15:8]                                                  99
     *   <SLV-T>    DBh       74h       [7:0]    ISYNC_PTN2_ARK2[7:0]                                                  100
     *   <SLV-T>    DBh       75h       [7:0]    ISYNC_PTN3_ARK2[31:24]                                                101
     *   <SLV-T>    DBh       76h       [7:0]    ISYNC_PTN3_ARK2[23:16]                                                102
     *   <SLV-T>    DBh       77h       [7:0]    ISYNC_PTN3_ARK2[15:8]                                                 103
     *   <SLV-T>    DBh       78h       [7:0]    ISYNC_PTN3_ARK2[7:0]                                                  104
     *   <SLV-T>    DBh       79h       [7:0]    ISYNC_PTN4_ARK2[31:24]                                                105
     *   <SLV-T>    DBh       7Ah       [7:0]    ISYNC_PTN4_ARK2[23:16]                                                106
     *   <SLV-T>    DBh       7Bh       [7:0]    ISYNC_PTN4_ARK2[15:8]                                                 107
     *   <SLV-T>    DBh       7Ch       [7:0]    ISYNC_PTN4_ARK2[7:0]                                                  108
     *   <SLV-T>    DBh       7Dh       [7:0]    ISYNC_PTN5_ARK2[31:24]                                                109
     *   <SLV-T>    DBh       7Eh       [7:0]    ISYNC_PTN5_ARK2[23:16]                                                110
     *   <SLV-T>    DBh       7Fh       [7:0]    ISYNC_PTN5_ARK2[15:8]                                                 111
     *   <SLV-T>    DBh       80h       [7:0]    ISYNC_PTN5_ARK2[7:0]                                                  112
     *   <SLV-T>    DBh       81h       [7:0]    ISYNC_PTN6_ARK2[31:24]                                                113
     *   <SLV-T>    DBh       82h       [7:0]    ISYNC_PTN6_ARK2[23:16]                                                114
     *   <SLV-T>    DBh       83h       [7:0]    ISYNC_PTN6_ARK2[15:8]                                                 115
     *   <SLV-T>    DBh       84h       [7:0]    ISYNC_PTN6_ARK2[7:0]                                                  116
     *   <SLV-T>    DBh       85h       [7:0]    ISYNC_PTN7_ARK2[31:24]                                                117
     *   <SLV-T>    DBh       86h       [7:0]    ISYNC_PTN7_ARK2[23:16]                                                118
     *   <SLV-T>    DBh       87h       [7:0]    ISYNC_PTN7_ARK2[15:8]                                                 119
     *   <SLV-T>    DBh       88h       [7:0]    ISYNC_PTN7_ARK2[7:0]                                                  120
     *   <SLV-T>    DBh       89h       [7:0]    ISYNC_PTN8_ARK2[31:24]                                                121
     *   <SLV-T>    DBh       8Ah       [7:0]    ISYNC_PTN8_ARK2[23:16]                                                122
     *   <SLV-T>    DBh       8Bh       [7:0]    ISYNC_PTN8_ARK2[15:8]                                                 123
     *   <SLV-T>    DBh       8Ch       [7:0]    ISYNC_PTN8_ARK2[7:0]                                                  124
     *   <SLV-T>    DBh       8Dh       [7:0]    ISYNC_PTN9_ARK2[31:24]                                                125
     *   <SLV-T>    DBh       8Eh       [7:0]    ISYNC_PTN9_ARK2[23:16]                                                126
     *   <SLV-T>    DBh       8Fh       [7:0]    ISYNC_PTN9_ARK2[15:8]                                                 127
     *   <SLV-T>    DBh       90h       [7:0]    ISYNC_PTN9_ARK2[7:0]                                                  128
     *   <SLV-T>    DBh       91h       [7:0]    ISYNC_PTN10_ARK2[31:24]                                               129
     *   <SLV-T>    DBh       92h       [7:0]    ISYNC_PTN10_ARK2[23:16]                                               130
     *   <SLV-T>    DBh       93h       [7:0]    ISYNC_PTN10_ARK2[15:8]                                                131
     *   <SLV-T>    DBh       94h       [7:0]    ISYNC_PTN10_ARK2[7:0]                                                 132
     *   <SLV-T>    DBh       95h       [7:0]    ISYNC_PTN11_ARK2[31:24]                                               133
     *   <SLV-T>    DBh       96h       [7:0]    ISYNC_PTN11_ARK2[23:16]                                               134
     *   <SLV-T>    DBh       97h       [7:0]    ISYNC_PTN11_ARK2[15:8]                                                135
     *   <SLV-T>    DBh       98h       [7:0]    ISYNC_PTN11_ARK2[7:0]                                                 136
     *   <SLV-T>    DBh       99h       [7:0]    ISYNC_PTN12_ARK2[31:24]                                               137
     *   <SLV-T>    DBh       9Ah       [7:0]    ISYNC_PTN12_ARK2[23:16]                                               138
     *   <SLV-T>    DBh       9Bh       [7:0]    ISYNC_PTN12_ARK2[15:8]                                                139
     *   <SLV-T>    DBh       9Ch       [7:0]    ISYNC_PTN12_ARK2[7:0]                                                 140
     *   <SLV-T>    DBh       9Dh       [7:0]    ISYNC_PTN13_ARK2[31:24]                                               141
     *   <SLV-T>    DBh       9Eh       [7:0]    ISYNC_PTN13_ARK2[23:16]                                               142
     *   <SLV-T>    DBh       9Fh       [7:0]    ISYNC_PTN13_ARK2[15:8]                                                143
     *   <SLV-T>    DBh       A0h       [7:0]    ISYNC_PTN13_ARK2[7:0]                                                 144
     *   <SLV-T>    DBh       A1h       [7:0]    ISYNC_PTN14_ARK2[31:24]                                               145
     *   <SLV-T>    DBh       A2h       [7:0]    ISYNC_PTN14_ARK2[23:16]                                               146
     *   <SLV-T>    DBh       A3h       [7:0]    ISYNC_PTN14_ARK2[15:8]                                                147
     *   <SLV-T>    DBh       A4h       [7:0]    ISYNC_PTN14_ARK2[7:0]                                                 148
     *   <SLV-T>    DBh       A5h       [7:0]    ISYNC_PTN15_ARK2[31:24]                                               149
     *   <SLV-T>    DBh       A6h       [7:0]    ISYNC_PTN15_ARK2[23:16]                                               150
     *   <SLV-T>    DBh       A7h       [7:0]    ISYNC_PTN15_ARK2[15:8]                                                151
     *   <SLV-T>    DBh       A8h       [7:0]    ISYNC_PTN15_ARK2[7:0]                                                 152
     *   <SLV-T>    DBh       A9h       [7:0]    ITOP_PTR1_ARK2[15:8]                                                  153
     *   <SLV-T>    DBh       AAh       [7:0]    ITOP_PTR1_ARK2[7:0]                                                   154
     *   <SLV-T>    DBh       ABh       [7:0]    ILAST_PTR1_ARK2[15:8]                                                 155
     *   <SLV-T>    DBh       ACh       [7:0]    ILAST_PTR1_ARK2[7:0]                                                  156
     *   <SLV-T>    DBh       ADh       [7:0]    ITOP_PTR2_ARK2[15:8]                                                  157
     *   <SLV-T>    DBh       AEh       [7:0]    ITOP_PTR2_ARK2[7:0]                                                   158
     *   <SLV-T>    DBh       AFh       [7:0]    ILAST_PTR2_ARK2[15:8]                                                 159
     *   <SLV-T>    DBh       B0h       [7:0]    ILAST_PTR2_ARK2[7:0]                                                  160
     *       ...	  	    	                                                                                       ...
     *   <SLV-T>    DBh       FFh       [7:0]    ILAST_PTR22_ARK2[15:8]                                                239
     *   <SLV-T>    DCh       10h       [7:0]    ILAST_PTR22_ARK2[7:0]                                                   0
     *       ...	  	    	                                                                                       ...
     *   <SLV-T>    DCh       FFh       [7:0]    ILAST_PTR82_ARK2[15:8]                                                239
     *   <SLV-T>    DDh       10h       [7:0]    ILAST_PTR82_ARK2[7:0]                                                   0
     *       ...	  	    	                                                                                       ...
     *   <SLV-T>    DDh       A5h       [7:0]    ITOP_PTR120_ARK2[15:8]                                                149
     *   <SLV-T>    DDh       A6h       [7:0]    ITOP_PTR120_ARK2[7:0]                                                 150
     *   <SLV-T>    DDh       A7h       [7:0]    ILAST_PTR120_ARK2[15:8]                                               151
     *   <SLV-T>    DDh       A8h       [7:0]    ILAST_PTR120_ARK2[7:0]                                                152
     *   <SLV-T>    DDh       A9h       [7:0]    ISTREAM_INFO1_ARK2[3:0],ISTREAM_INFO2_ARK2[3:0]                       153
     *   <SLV-T>    DDh       AAh       [7:0]    ISTREAM_INFO3_ARK2[3:0],ISTREAM_INFO4_ARK2[3:0]                       154
     *   <SLV-T>    DDh       ABh       [7:0]    ISTREAM_INFO5_ARK2[3:0],ISTREAM_INFO6_ARK2[3:0]                       155
     *       ...	  	    	                                                                                       ...
     *   <SLV-T>    DDh       E4h       [7:0]    ISTREAM_INFO119_ARK2[3:0],ISTREAM_INFO120_ARK2[3:0]                   212
     *   <SLV-T>    DDh       E5h       [7:0]    ISTREAM_ID0_ARK2[15:8]                                                213
     *   <SLV-T>    DDh       E6h       [7:0]    ISTREAM_ID0_ARK2[7:0]                                                 214
     *   <SLV-T>    DDh       E7h       [7:0]    ISTREAM_ID1_ARK2[15:8]                                                215
     *   <SLV-T>    DDh       E8h       [7:0]    ISTREAM_ID1_ARK2[7:0]                                                 216
     *   <SLV-T>    DDh       E9h       [7:0]    ISTREAM_ID2_ARK2[15:8]                                                217
     *   <SLV-T>    DDh       EAh       [7:0]    ISTREAM_ID2_ARK2[7:0]                                                 218
     *   <SLV-T>    DDh       EBh       [7:0]    ISTREAM_ID3_ARK2[15:8]                                                219
     *   <SLV-T>    DDh       ECh       [7:0]    ISTREAM_ID3_ARK2[7:0]                                                 220
     *   <SLV-T>    DDh       EDh       [7:0]    ISTREAM_ID4_ARK2[15:8]                                                221
     *   <SLV-T>    DDh       EEh       [7:0]    ISTREAM_ID4_ARK2[7:0]                                                 222
     *   <SLV-T>    DDh       EFh       [7:0]    ISTREAM_ID5_ARK2[15:8]                                                223
     *   <SLV-T>    DDh       F0h       [7:0]    ISTREAM_ID5_ARK2[7:0]                                                 224
     *   <SLV-T>    DDh       F1h       [7:0]    ISTREAM_ID6_ARK2[15:8]                                                225
     *   <SLV-T>    DDh       F2h       [7:0]    ISTREAM_ID6_ARK2[7:0]                                                 226
     *   <SLV-T>    DDh       F3h       [7:0]    ISTREAM_ID7_ARK2[15:8]                                                227
     *   <SLV-T>    DDh       F4h       [7:0]    ISTREAM_ID7_ARK2[7:0]                                                 228
     *   <SLV-T>    DDh       F5h       [7:0]    ISTREAM_ID8_ARK2[15:8]                                                229
     *   <SLV-T>    DDh       F6h       [7:0]    ISTREAM_ID8_ARK2[7:0]                                                 230
     *   <SLV-T>    DDh       F7h       [7:0]    ISTREAM_ID9_ARK2[15:8]                                                231
     *   <SLV-T>    DDh       F8h       [7:0]    ISTREAM_ID9_ARK2[7:0]                                                 232
     *   <SLV-T>    DDh       F9h       [7:0]    ISTREAM_ID10_ARK2[15:8]                                               233
     *   <SLV-T>    DDh       FAh       [7:0]    ISTREAM_ID10_ARK2[7:0]                                                234
     *   <SLV-T>    DDh       FBh       [7:0]    ISTREAM_ID11_ARK2[15:8]                                               235
     *   <SLV-T>    DDh       FCh       [7:0]    ISTREAM_ID11_ARK2[7:0]                                                236
     *   <SLV-T>    DDh       FDh       [7:0]    ISTREAM_ID12_ARK2[15:8]                                               237
     *   <SLV-T>    DDh       FEh       [7:0]    ISTREAM_ID12_ARK2[7:0]                                                238
     *   <SLV-T>    DDh       FFh       [7:0]    ISTREAM_ID13_ARK2[15:8]                                               239
     *   <SLV-T>    DEh       10h       [7:0]    ISTREAM_ID13_ARK2[7:0]                                                  0
     *   <SLV-T>    DEh       11h       [7:0]    ISTREAM_ID14_ARK2[15:8]                                                 1
     *   <SLV-T>    DEh       12h       [7:0]    ISTREAM_ID14_ARK2[7:0]                                                  2
     *   <SLV-T>    DEh       13h       [7:0]    ISTREAM_ID15_ARK2[15:8]                                                 3
     *   <SLV-T>    DEh       14h       [7:0]    ISTREAM_ID15_ARK2[7:0]                                                  4
     *   <SLV-T>    DEh       15h       [7:4]    IEWS_ARK2[0],IUPLINK_ARK2[2:0]                                          5
     *   <SLV-T>    DEh       16h       [7:0]    IEXP_ID_ARK2[15:8]                                                      6
     *   <SLV-T>    DEh       17h       [7:0]    IEXP_ID_ARK2[7:0]                                                       7
     *   <SLV-T>    DEh       18h       [7:0]    IEXP_AREA0_ARK2[7:0]                                                    8
     *   <SLV-T>    DEh       19h       [7:0]    IEXP_AREA1_ARK2[7:0]                                                    9
     *   <SLV-T>    DEh       1Ah       [7:0]    IEXP_AREA2_ARK2[7:0]                                                   10
     *   <SLV-T>    DEh       1Bh       [7:0]    IEXP_AREA3_ARK2[7:0]                                                   11
     *        ...	  	    	                                                                                       ...
     *   <SLV-T>    DEh       FFh       [7:0]    IEXP_AREA231_ARK2[7:0]                                                239
     *   <SLV-T>    DFh       10h       [7:0]    IEXP_AREA232_ARK2[7:0]                                                  0
     *        ...	  	    	                                                                                       ...
     *   <SLV-T>    DFh       E8h       [7:0]    IEXP_AREA448_ARK2[7:0]                                                216
     *   <SLV-T>    DFh       E9h       [7:2]    IEXP_AREA449_ARK2[5:0]                                                217
     */

    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 240);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Order of change flag */
    pTMCCInfo->changeOrder = (data[0] & 0xFF);

    /* Transmission Mode1-8 Modulation, Code rate, Slot Number and Backoff */
    for (i = 0; i < 8; i++) {
        pTMCCInfo->modcodSlotInfo[i].mod = (sony_isdbs3_mod_t) ((data[(3 * i) + 1] >> 4) & 0x0F);
        pTMCCInfo->modcodSlotInfo[i].cod = (sony_isdbs3_cod_t) (data[(3 * i) + 1] & 0x0F);
        pTMCCInfo->modcodSlotInfo[i].slotNum = (data[(3 * i + 2)] & 0xFF);
        pTMCCInfo->modcodSlotInfo[i].backoff = (data[(3 * i + 3)] & 0xFF);
    }

    /* Stream Type */
    for (i = 0; i < 16; i++) {
        pTMCCInfo->streamTypeForEachRelativeStream[i] = (sony_isdbs3_stream_type_t) (data[i + 25] & 0xFF);
    }

    /* Packt Length */
    for (i = 0; i < 16; i++) {
        pTMCCInfo->packetLengthForEachRelativeStream[i] = (uint16_t)((data[(2 * i) + 41] << 8) | data[(2 * i + 1) + 41]);
    }

    /* Sync Pattern Bit Length */
    for (i = 0; i < 16; i++) {
        pTMCCInfo->syncPatternBitLengthForEachRelativeStream[i] = (uint8_t) (data[i + 73] & 0xFF);
    }

    /* Sync Pattern */
    for (i = 0; i < 16; i++) {
        pTMCCInfo->syncPatternForEachRelativeStream[i] = (uint32_t) ((data[(4 * i)     + 89] << 24) |
                                                                     (data[(4 * i + 1) + 89] << 16) |
                                                                     (data[(4 * i + 2) + 89] << 8)  |
                                                                     (data[(4 * i + 3) + 89]));
    }

    /* Set SLV-T Bank : 0xDD */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xDD);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 240);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Stream Information */
    for (i = 0;i < 60; i++) {
        pTMCCInfo->relativeStreamForEachSlot[2 * i]       = (uint8_t)((data[i + 153] >> 4) & 0x0F);
        pTMCCInfo->relativeStreamForEachSlot[(2 * i) + 1] = (uint8_t)(data[i + 153] & 0x0F);
    }

    /* Stream ID (0~12) */
    for (i = 0; i < 13; i++) {
        pTMCCInfo->streamidForEachRelativeStream[i] = (uint16_t)((data[(2 * i) + 213] << 8) | data[(2 * i + 1) + 213]);
    }

    /* Stream ID (13) */
    pTMCCInfo->streamidForEachRelativeStream[13] = (uint16_t)(data[239] << 8);

    /* Set SLV-T Bank : 0xDE */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xDE);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    result = pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 6);
    if (result != SONY_RESULT_OK) {
        UnfreezeTMCCInfo (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /* Stream ID (13) */
    pTMCCInfo->streamidForEachRelativeStream[13] |= data[0];

    /* Stream ID (14~15) */
    for (i = 0; i < 2; i++) {
        pTMCCInfo->streamidForEachRelativeStream[i + 14] = (uint16_t)((data[(2 * i) + 1] << 8) | data[(2 * i + 1) + 1]);
    }

    pTMCCInfo->ewsFlag = (uint8_t)((data[5] & 0x80) ? 1 : 0);
    pTMCCInfo->uplinkInfo = (uint8_t)((data[5] >> 4) & 0x07);

    UnfreezeTMCCInfo (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_monitor_CurrentStreamModCodSlotNum (sony_demod_t *pDemod, sony_isdbs3_tmcc_info_t * pTmccInfo,
                                                                    sony_isdbs3_mod_t * pModH, sony_isdbs3_mod_t * pModL,
                                                                    sony_isdbs3_cod_t * pCodH, sony_isdbs3_cod_t * pCodL,
                                                                    uint8_t * pSlotNumH, uint8_t * pSlotNumL)
{
    uint16_t streamID = 0;
    uint8_t relativeStreamNum = 255;

    SONY_TRACE_ENTER("sony_demod_isdbs3_monitor_CurrentStreamModCodSlotNum");

    /* Null check */
    if ((!pDemod) || (!pTmccInfo) || (!pModH) || (!pModL) || (!pCodH) || (!pCodL) || (!pSlotNumH) || (!pSlotNumL)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    {
        uint8_t data[2];

        /* Set SLV-T Bank : 0xD0 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         *   slave   Bank    Addr    Bit    Default     Setting     Signal name
         *  -------------------------------------------------------------------------
         *   <SLV-T>  D0h     87h     [7:0]    8'h00      8'h40      OSTREAM_ID[15:8]
         *   <SLV-T>  D0h     88h     [7:0]    8'h00      8'hF2      OSTREAM_ID[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, data, 2) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        streamID = (uint16_t)((data[0] << 8) | (data[1]));
    }

    /* Get relative stream number from stream ID. */
    {
        int i = 0;

        for (i = 0; i < 16; i++) {
            if (pTmccInfo->streamidForEachRelativeStream[i] == streamID) {
                relativeStreamNum = (uint8_t)i;
                break;
            }
        }
    }

    if (relativeStreamNum > 15) {
        /* Stream ID is not found. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    {
        int i = 0;
        int j = 0;
        int slotTotal = 0;

        *pModH = SONY_ISDBS3_MOD_UNUSED_15;
        *pModL = SONY_ISDBS3_MOD_UNUSED_15;
        *pCodH = SONY_ISDBS3_COD_UNUSED_15;
        *pCodL = SONY_ISDBS3_COD_UNUSED_15;
        *pSlotNumH = 0;
        *pSlotNumL = 0;

        for (i = 0; i < 8; i++) {
            for (j = slotTotal; j < slotTotal + pTmccInfo->modcodSlotInfo[i].slotNum; j++) {
                if (j >= 120) {
                    /* Invalid TMCC information */
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
                }

                /* For each transmission mode, search slot-relative stream number table information to find target stream number. */
                if (pTmccInfo->relativeStreamForEachSlot[j] == relativeStreamNum) {
                    if (*pModH == SONY_ISDBS3_MOD_UNUSED_15) {
                        /* First transmission mode. (higher layer) */
                        (*pSlotNumH)++;
                    } else if (*pModL == SONY_ISDBS3_MOD_UNUSED_15) {
                        /* Second transmission mode. (lower layer) */
                        (*pSlotNumL)++;
                    }
                }
            }

            if ((*pModH == SONY_ISDBS3_MOD_UNUSED_15) && (*pSlotNumH > 0)) {
                *pModH = pTmccInfo->modcodSlotInfo[i].mod;
                *pCodH = pTmccInfo->modcodSlotInfo[i].cod;
            } else if ((*pModL == SONY_ISDBS3_MOD_UNUSED_15) && (*pSlotNumL > 0)) {
                *pModL = pTmccInfo->modcodSlotInfo[i].mod;
                *pCodL = pTmccInfo->modcodSlotInfo[i].cod;
            }

            if ((*pModH != SONY_ISDBS3_MOD_UNUSED_15) && (*pModL != SONY_ISDBS3_MOD_UNUSED_15)) {
                break;
            }

            slotTotal += pTmccInfo->modcodSlotInfo[i].slotNum;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_isdbs3_monitor_SiteDiversityInfo (sony_demod_t * pDemod,
                                                           uint8_t * pSiteDiversityInfo)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_SiteDiversityInfo");

    if ((!pDemod) || (!pSiteDiversityInfo)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*    Slave     Bank      Addr      Bit       Signal name
     *  -------------------------------------------------------------
     *  <SLV-T>    D0h       F0h       [1]       IUPLINK_ARK2
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF0, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pSiteDiversityInfo =  (uint8_t)((rdata & 0x02) ? 1 : 0);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_isdbs3_monitor_EWSChange (sony_demod_t * pDemod,
                                                   uint8_t * pEWSChange)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_EWSChange");

    if ((!pDemod) || (!pEWSChange)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*    Slave     Bank      Addr      Bit       Signal name
     *  -------------------------------------------------------------
     *  <SLV-T>    D0h       F0h       [2]       IEWS_ARK2
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF0, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    *pEWSChange =  (uint8_t)((rdata & 0x04) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbs3_monitor_TMCCChange (sony_demod_t * pDemod,
                                                    uint8_t * pTMCCChange)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_TMCCChange");

    if ((!pDemod) || (!pTMCCChange)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*    Slave     Bank      Addr      Bit       Signal name
     *  -------------------------------------------------------------
     *   <SLV-T>    D0h       F0h       [0]       ITMCCCHG_ARK2
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF0, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);
    *pTMCCChange =  (uint8_t)((rdata & 0x01) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}
sony_result_t sony_demod_isdbs3_ClearEWSChange (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_ClearEWSChange");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*    Slave   Bank    Addr    Bit    Default     Setting     Signal name
     *  -------------------------------------------------------------------------
     *   <SLV-T>  D0h     F1h     [2]    1'b0        1'b1        OTMCCEWSCLR_ARK2
     */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF1, 0x04);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}
sony_result_t sony_demod_isdbs3_ClearTMCCChange (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_ClearTMCCChange");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*    Slave   Bank    Addr    Bit    Default     Setting     Signal name
     *  -------------------------------------------------------------------------
     *   <SLV-T>  D0h     F1h     [0]    1'b0        1'b1        OTMCCCHGCLR_ARK2
     */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF1, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

sony_result_t sony_demod_isdbs3_monitor_StreamIDError (sony_demod_t * pDemod, uint8_t * pStreamIDError)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER ("sony_demod_isdbs3_monitor_StreamIDError");

    if ((!pDemod) || (!pStreamIDError)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_ISDBS3) {
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

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN(result);
    }

    /*   slave   Bank    Addr    Bit     Signal name             Meaning
     *  -------------------------------------------------------------------------
     *   <SLV-T>  D0h     95h     [0]    NORELATIVE_STREAM_ARK2   0:match, 1:unmatch
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x95, &rdata, 1) != SONY_RESULT_OK) {
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);
    *pStreamIDError =  (uint8_t)((rdata & 0x01) ? 1 : 0);

    SONY_TRACE_RETURN(result);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t IsDmdLocked(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t agcLock = 0;
    uint8_t tstlvLock  = 0;
    uint8_t tmccLock  = 0;
    SONY_TRACE_ENTER("IsDmdLocked");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_isdbs3_monitor_SyncStat(pDemod, &agcLock, &tstlvLock, &tmccLock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    if (tmccLock) {
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    } else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }
}

static sony_result_t FreezeTMCCInfo(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("FreezeTMCCInfo");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*   slave     Bank      Addr      Bit      Default    Setting    Signal name
     *  ---------------------------------------------------------------------------------
     *  <SLV-T>    D0h       FAh       [0]      1'b0       1'b1       OTMCC_FREEZE_ARK2
     */

    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFA, 0x01);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t UnfreezeTMCCInfo(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER("UnfreezeTMCCInfo");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*   slave     Bank      Addr      Bit      Default    Setting    Signal name
     *  ---------------------------------------------------------------------------------
     *  <SLV-T>    D0h       FAh       [0]      1'b0       1'b0       OTMCC_FREEZE_ARK2
     */

    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFA, 0x00);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t IsTMCCReadOK(sony_demod_t* pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata = 0;
    SONY_TRACE_ENTER("IsTMCCReadOK");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xD0 */
    result = pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xD0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN(result);
    }

    /*   slave     Bank      Addr      Bit      Signal name
     *  --------------------------------------------------------------
     *  <SLV-T>    D0h       FBh       [0]      ITMCC_READOK_ARK2
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xFB, &rdata, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (rdata & 0x01) {
        SONY_TRACE_RETURN(SONY_RESULT_OK);
    } else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
    }
}

