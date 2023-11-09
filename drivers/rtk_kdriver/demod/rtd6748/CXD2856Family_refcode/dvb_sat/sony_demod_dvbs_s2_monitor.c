/*------------------------------------------------------------------------------
  Copyright 2016-2018 Sony Semiconductor Solutions Corporation

  Last Updated    : 2018/11/14
  Modification ID : 9eb23fa0bb07890d3eb414763d76286f3e14490e
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_math.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"

static const struct {
    uint32_t value;
    int32_t cnr_x1000;
} s_cn_data[] = {
    {0x033e, 0},
    {0x0339, 100},
    {0x0333, 200},
    {0x032e, 300},
    {0x0329, 400},
    {0x0324, 500},
    {0x031e, 600},
    {0x0319, 700},
    {0x0314, 800},
    {0x030f, 900},
    {0x030a, 1000},
    {0x02ff, 1100},
    {0x02f4, 1200},
    {0x02e9, 1300},
    {0x02de, 1400},
    {0x02d4, 1500},
    {0x02c9, 1600},
    {0x02bf, 1700},
    {0x02b5, 1800},
    {0x02ab, 1900},
    {0x02a1, 2000},
    {0x029b, 2100},
    {0x0295, 2200},
    {0x0290, 2300},
    {0x028a, 2400},
    {0x0284, 2500},
    {0x027f, 2600},
    {0x0279, 2700},
    {0x0274, 2800},
    {0x026e, 2900},
    {0x0269, 3000},
    {0x0262, 3100},
    {0x025c, 3200},
    {0x0255, 3300},
    {0x024f, 3400},
    {0x0249, 3500},
    {0x0242, 3600},
    {0x023c, 3700},
    {0x0236, 3800},
    {0x0230, 3900},
    {0x022a, 4000},
    {0x0223, 4100},
    {0x021c, 4200},
    {0x0215, 4300},
    {0x020e, 4400},
    {0x0207, 4500},
    {0x0201, 4600},
    {0x01fa, 4700},
    {0x01f4, 4800},
    {0x01ed, 4900},
    {0x01e7, 5000},
    {0x01e0, 5100},
    {0x01d9, 5200},
    {0x01d2, 5300},
    {0x01cb, 5400},
    {0x01c4, 5500},
    {0x01be, 5600},
    {0x01b7, 5700},
    {0x01b1, 5800},
    {0x01aa, 5900},
    {0x01a4, 6000},
    {0x019d, 6100},
    {0x0196, 6200},
    {0x018f, 6300},
    {0x0189, 6400},
    {0x0182, 6500},
    {0x017c, 6600},
    {0x0175, 6700},
    {0x016f, 6800},
    {0x0169, 6900},
    {0x0163, 7000},
    {0x015c, 7100},
    {0x0156, 7200},
    {0x0150, 7300},
    {0x014a, 7400},
    {0x0144, 7500},
    {0x013e, 7600},
    {0x0138, 7700},
    {0x0132, 7800},
    {0x012d, 7900},
    {0x0127, 8000},
    {0x0121, 8100},
    {0x011c, 8200},
    {0x0116, 8300},
    {0x0111, 8400},
    {0x010b, 8500},
    {0x0106, 8600},
    {0x0101, 8700},
    {0x00fc, 8800},
    {0x00f7, 8900},
    {0x00f2, 9000},
    {0x00ee, 9100},
    {0x00ea, 9200},
    {0x00e6, 9300},
    {0x00e2, 9400},
    {0x00de, 9500},
    {0x00da, 9600},
    {0x00d7, 9700},
    {0x00d3, 9800},
    {0x00d0, 9900},
    {0x00cc, 10000},
    {0x00c7, 10100},
    {0x00c3, 10200},
    {0x00bf, 10300},
    {0x00ba, 10400},
    {0x00b6, 10500},
    {0x00b2, 10600},
    {0x00ae, 10700},
    {0x00aa, 10800},
    {0x00a7, 10900},
    {0x00a3, 11000},
    {0x009f, 11100},
    {0x009c, 11200},
    {0x0098, 11300},
    {0x0094, 11400},
    {0x0091, 11500},
    {0x008e, 11600},
    {0x008a, 11700},
    {0x0087, 11800},
    {0x0084, 11900},
    {0x0081, 12000},
    {0x007e, 12100},
    {0x007b, 12200},
    {0x0079, 12300},
    {0x0076, 12400},
    {0x0073, 12500},
    {0x0071, 12600},
    {0x006e, 12700},
    {0x006c, 12800},
    {0x0069, 12900},
    {0x0067, 13000},
    {0x0065, 13100},
    {0x0062, 13200},
    {0x0060, 13300},
    {0x005e, 13400},
    {0x005c, 13500},
    {0x005a, 13600},
    {0x0058, 13700},
    {0x0056, 13800},
    {0x0054, 13900},
    {0x0052, 14000},
    {0x0050, 14100},
    {0x004e, 14200},
    {0x004c, 14300},
    {0x004b, 14400},
    {0x0049, 14500},
    {0x0047, 14600},
    {0x0046, 14700},
    {0x0044, 14800},
    {0x0043, 14900},
    {0x0041, 15000},
    {0x003f, 15100},
    {0x003e, 15200},
    {0x003c, 15300},
    {0x003b, 15400},
    {0x003a, 15500},
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

static const struct {
    uint32_t value;
    int32_t cnr_x1000;
} s2_cn_data[] = {
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

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t monitor_SamplingRateMode (sony_demod_t * pDemod,
                                               uint8_t * pIsHSMode);

static sony_result_t monitor_System (sony_demod_t * pDemod,
                                     sony_dtv_system_t * pSystem);

static sony_result_t monitor_CarrierOffset (sony_demod_t * pDemod,
                                            int32_t * pOffset);

static sony_result_t s_monitor_CodeRate (sony_demod_t * pDemod,
                                         sony_dvbs_coderate_t * pCodeRate);

static sony_result_t s_monitor_IQSense (sony_demod_t * pDemod,
                                        sony_demod_sat_iq_sense_t * pSense);

static sony_result_t s_monitor_CNR (sony_demod_t * pDemod,
                                    int32_t * pCNR);

static sony_result_t s_monitor_PER (sony_demod_t * pDemod,
                                    uint32_t * pPER);

static sony_result_t s2_monitor_IQSense (sony_demod_t * pDemod,
                                         sony_demod_sat_iq_sense_t * pSense);

static sony_result_t s2_monitor_CNR (sony_demod_t * pDemod,
                                     int32_t * pCNR);

static sony_result_t s2_monitor_PER (sony_demod_t * pDemod,
                                     uint32_t * pPER);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbs_s2_monitor_SyncStat (sony_demod_t * pDemod,
                                                   uint8_t * pTSLockStat)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_SyncStat");

    if ((!pDemod) || (!pTSLockStat)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS) &&
        (pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       11h       [2]      ITSLOCK
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data & 0x04){
        *pTSLockStat = 1;
    } else {
        *pTSLockStat = 0;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_CarrierOffset (sony_demod_t * pDemod,
                                                        int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t tsLock = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_CarrierOffset");

    if ((!pDemod) || (!pOffset)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS) &&
        (pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    result = sony_demod_dvbs_s2_monitor_SyncStat(pDemod, &tsLock);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (tsLock == 0){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    result = monitor_CarrierOffset (pDemod, pOffset);

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_IFAGCOut (sony_demod_t * pDemod,
                                                   uint32_t * pIFAGC)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[2] = {0, 0};
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_IFAGCOut");

    if ((!pDemod) || (!pIFAGC)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS) &&
        (pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit       Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       1Fh       [4:0]     IRFAGC_GAIN[12:8]
     * <SLV-T>    A0h       20h       [7:0]     IRFAGC_GAIN[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x1F, data, 2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pIFAGC = (((uint32_t)data[0] & 0x1F) << 8) | (uint32_t)(data[1] & 0xFF);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_System (sony_demod_t * pDemod,
                                                 sony_dtv_system_t * pSystem)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_System");

    if ((!pDemod) || (!pSystem)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS) &&
        (pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    result = monitor_System (pDemod, pSystem);

    SLVT_UnFreezeReg (pDemod);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_SymbolRate (sony_demod_t * pDemod,
                                                     uint32_t * pSymbolRateSps)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isHSMode = 0;
    uint32_t itrl_ckferr_sr = 0;
    int32_t ckferr_sr = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    uint32_t tempDiv = 0;
    uint8_t data[4];

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_SymbolRate");

    if ((!pDemod) || (!pSymbolRateSps)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS) &&
        (pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       10h       [0]      ITRL_LOCK
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 1) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((data[0] & 0x01) == 0x00){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    result = monitor_SamplingRateMode (pDemod, &isHSMode);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    /*
     *  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    AFh       38h       [2:0]    ITRL_CKFERR_SR[26:24]
     * <SLV-T>    AFh       39h       [7:0]    ITRL_CKFERR_SR[23:16]
     * <SLV-T>    AFh       3Ah       [7:0]    ITRL_CKFERR_SR[15:8]
     * <SLV-T>    AFh       3Bh       [7:0]    ITRL_CKFERR_SR[7:0]
     */
    /* Set SLV-T Bank : 0xAF */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAF) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x38, data, 4) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    itrl_ckferr_sr = ((uint32_t)(data[0] & 0x07) << 24) |
                 ((uint32_t)(data[1] & 0xFF) << 16) |
                 ((uint32_t)(data[2] & 0xFF) <<  8) |
                  (uint32_t)(data[3] & 0xFF);
    ckferr_sr = sony_Convert2SComplement (itrl_ckferr_sr, 27);

    SLVT_UnFreezeReg (pDemod);

    tempDiv = (uint32_t)((int32_t)65536 - ckferr_sr);

    /* Checks to prevent overflow in the calculation. */
    if (ckferr_sr >= 64051 || tempDiv >= 0x01FFFFFF){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    if (isHSMode){
        /*----------------------------------------------------------------
          samplingRate = 96000 (KHz)

          Symbol rate[Sps] = (0.5 * 96000) / ((1 - ckferr_sr/2^16) * 10^-3)
                           = 48 * 10^3 * 2^16 / ((2^16 - ckferr_sr) * 10^-3)
                           = 48 * 2^16 * 10^3 * 10^3 / 2^16 - ckferr_sr
                           = 48 * 65536 * 10^3 * 10^3 / (2^16 - ckferr_sr)
                           = 3145728000 * 100 * 10 / (65536 - ckferr_sr)
                           (Divide in 3 steps to prevent overflow.)
        ----------------------------------------------------------------*/
        tempQ = 3145728000u / tempDiv;
        tempR = 3145728000u % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + tempR / tempDiv;
        tempR = tempR % tempDiv;

        tempR *= 10;
        tempQ = (tempQ * 10) + tempR / tempDiv;
        tempR = tempR % tempDiv;

        /* Round up based on the remainder */
        if (tempR >= tempDiv/2) {
            *pSymbolRateSps = tempQ + 1;
        }
        else {
            *pSymbolRateSps = tempQ;
        }
    } else {
        /*----------------------------------------------------------------
          samplingRate = 64000 (KHz)

          Symbol rate[Sps] = (0.5 * 64000) / ((1 - ckferr_sr/2^16) * 10^-3)
                           = 32 * 10^3 * 2^16 / ((2^16 - ckferr_sr) * 10^-3)
                           = 32 * 2^16 * 10^3 * 10^3 / 2^16 - ckferr_sr
                           = 32 * 65536 * 10^3 * 10^3 / (2^16 - ckferr_sr)
                           = 2097152000 * 100 * 10 / (65536 - ckferr_sr)
                           (Divide in 3 steps to prevent overflow.)
        ----------------------------------------------------------------*/
        tempQ = 2097152000u / tempDiv;
        tempR = 2097152000u % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + tempR / tempDiv;
        tempR = tempR % tempDiv;

        tempR *= 10;
        tempQ = (tempQ * 10) + tempR / tempDiv;
        tempR = tempR % tempDiv;

        /* Round up based on the remainder */
        if (tempR >= tempDiv/2) {
            *pSymbolRateSps = tempQ + 1;
        }
        else {
            *pSymbolRateSps = tempQ;
        }
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_IQSense (sony_demod_t * pDemod,
                                                  sony_demod_sat_iq_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_IQSense");

    if ((!pDemod) || (!pSense)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        if ((dtvSystem != SONY_DTV_SYSTEM_DVBS) && (dtvSystem != SONY_DTV_SYSTEM_DVBS2)){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
    case SONY_DTV_SYSTEM_DVBS2:
        dtvSystem = pDemod->system;
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (dtvSystem == SONY_DTV_SYSTEM_DVBS) {
        result = s_monitor_IQSense (pDemod, pSense);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    } else {
        result = s2_monitor_IQSense (pDemod, pSense);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    }

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_monitor_CNR (sony_demod_t * pDemod,
                                              int32_t * pCNR)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_CNR");

    if ((!pDemod) || (!pCNR)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        if ((dtvSystem != SONY_DTV_SYSTEM_DVBS) && (dtvSystem != SONY_DTV_SYSTEM_DVBS2)){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
    case SONY_DTV_SYSTEM_DVBS2:
        dtvSystem = pDemod->system;
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (dtvSystem == SONY_DTV_SYSTEM_DVBS) {
        result = s_monitor_CNR (pDemod, pCNR);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    } else {
        result = s2_monitor_CNR (pDemod, pCNR);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    }

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_monitor_PER (sony_demod_t * pDemod,
                                              uint32_t * pPER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_PER");

    if ((!pDemod) || (!pPER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        if ((dtvSystem != SONY_DTV_SYSTEM_DVBS) && (dtvSystem != SONY_DTV_SYSTEM_DVBS2)){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
    case SONY_DTV_SYSTEM_DVBS2:
        dtvSystem = pDemod->system;
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (dtvSystem == SONY_DTV_SYSTEM_DVBS) {
        result = s_monitor_PER (pDemod, pPER);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    } else {
        result = s2_monitor_PER (pDemod, pPER);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
    }

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_monitor_CodeRate (sony_demod_t * pDemod,
                                                sony_dvbs_coderate_t * pCodeRate)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;

    SONY_TRACE_ENTER ("sony_demod_dvbs_monitor_CodeRate");

    if ((!pDemod) || (!pCodeRate)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN(result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = s_monitor_CodeRate (pDemod, pCodeRate);

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_monitor_PreViterbiBER (sony_demod_t * pDemod,
                                                     uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint8_t data[11];
    uint32_t bitError = 0;
    uint32_t bitCount = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_monitor_PreViterbiBER");

    if ((!pDemod) || (!pBER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       35h       [0]      IFVBER_VALID
     * <SLV-T>    A0h       36h       [5:0]    IFVBER_BITERR[21:16]
     * <SLV-T>    A0h       37h       [7:0]    IFVBER_BITERR[15:8]
     * <SLV-T>    A0h       38h       [7:0]    IFVBER_BITERR[7:0]
     * <SLV-T>    A0h       3Dh       [5:0]    IFVBER_BITNUM[21:16]
     * <SLV-T>    A0h       3Eh       [7:0]    IFVBER_BITNUM[15:8]
     * <SLV-T>    A0h       3Fh       [7:0]    IFVBER_BITNUM[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, data, 11) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        bitError = ((uint32_t)(data[1]  & 0x3F) << 16) |
                   ((uint32_t)(data[2]  & 0xFF) <<  8) |
                    (uint32_t)(data[3]  & 0xFF);
        bitCount = ((uint32_t)(data[8]  & 0x3F) << 16) |
                   ((uint32_t)(data[9]  & 0xFF) <<  8) |
                    (uint32_t)(data[10] & 0xFF);
        /*--------------------------------------------------------------------
          BER = bitError / bitCount
              = (bitError * 10^7) / bitCount
              = ((bitError * 625 * 125 * 128) / bitCount
        --------------------------------------------------------------------*/
        tempDiv = bitCount;
        if ((tempDiv == 0) || (bitError > bitCount)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (bitError * 625) / tempDiv;
        tempR = (bitError * 625) % tempDiv;

        tempR *= 125;
        tempQ = (tempQ * 125) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        tempR *= 128;
        tempQ = (tempQ * 128) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if ((tempDiv != 1) && (tempR >= (tempDiv/2))){
            *pBER = tempQ + 1;
        } else {
            *pBER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_monitor_PreRSBER (sony_demod_t * pDemod,
                                                uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint8_t data[4];
    uint32_t bitError = 0;
    uint32_t period = 0;
    uint32_t bitCount = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs_monitor_PreRSBER");

    if ((!pDemod) || (!pBER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       25h       [0]      IFBER_VALID
     * <SLV-T>    A0h       26h       [7:0]    IFBER_BITERR[23:16]
     * <SLV-T>    A0h       27h       [7:0]    IFBER_BITERR[15:8]
     * <SLV-T>    A0h       28h       [7:0]    IFBER_BITERR[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x25, data, 4) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        bitError = ((uint32_t)(data[1]  & 0xFF) << 16) |
                   ((uint32_t)(data[2]  & 0xFF) <<  8) |
                    (uint32_t)(data[3]  & 0xFF);

        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h     BAh     [3:0]    8'h08      period       OFBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBA, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*--------------------------------------------------------------------
          BER = ((bitError * 0.2) * 10^7) / (period * 204 * 8 * 8)
              = (bitError * 5^6 * 2^7) / (period * 102 * 2 * 2^3 * 2^3)
              = (bitError * 5^6) / (period * 102)
              = (bitError * 5^3 * 5^3) / (period * 102)
        --------------------------------------------------------------------*/
        period = (uint32_t)(1 << (data[0] & 0x0F));
        bitCount = period * 102;
        tempDiv = bitCount;
        if (tempDiv == 0){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (bitError * 125) / tempDiv;
        tempR = (bitError * 125) % tempDiv;

        tempR *= 125;
        tempQ = (tempQ * 125) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if (tempR >= (tempDiv/2)){
            *pBER = tempQ + 1;
        } else {
            *pBER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs2_monitor_PLSCode (sony_demod_t * pDemod,
                                                sony_dvbs2_plscode_t * pPLSCode)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint8_t data = 0;
    uint8_t isTSLock = 0;
    uint8_t mdcd_type = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_PLSCode");

    if ((!pDemod) || (!pPLSCode)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS2){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, &isTSLock);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    if (!isTSLock){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       21h       [6:2]    IMDCD_TYP[6:2]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x21, &data, 1) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SLVT_UnFreezeReg (pDemod);

    mdcd_type = (uint8_t)(data & 0x7F);

    switch((mdcd_type >> 2) & 0x1F)
    {
    case 0x01:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_1_4;
        break;
    case 0x02:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_1_3;
        break;
    case 0x03:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_2_5;
        break;
    case 0x04:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_1_2;
        break;
    case 0x05:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_3_5;
        break;
    case 0x06:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_2_3;
        break;
    case 0x07:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_3_4;
        break;
    case 0x08:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_4_5;
        break;
    case 0x09:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_5_6;
        break;
    case 0x0A:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_8_9;
        break;
    case 0x0B:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_QPSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_9_10;
        break;
    case 0x0C:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_3_5;
        break;
    case 0x0D:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_2_3;
        break;
    case 0x0E:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_3_4;
        break;
    case 0x0F:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_5_6;
        break;
    case 0x10:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_8_9;
        break;
    case 0x11:
        pPLSCode->modulation = SONY_DVBS2_MODULATION_8PSK;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_9_10;
        break;
    case 0x1D:
        /* Reserved */
        pPLSCode->modulation = SONY_DVBS2_MODULATION_RESERVED_29;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_RESERVED_29;
        break;
    case 0x1E:
        /* Reserved */
        pPLSCode->modulation = SONY_DVBS2_MODULATION_RESERVED_30;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_RESERVED_30;
        break;
    case 0x1F:
        /* Reserved */
        pPLSCode->modulation = SONY_DVBS2_MODULATION_RESERVED_31;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_RESERVED_31;
        break;
    default:
        /* 16APSK and 32APSK are not supported. */
        pPLSCode->modulation = SONY_DVBS2_MODULATION_INVALID;
        pPLSCode->codeRate = SONY_DVBS2_CODERATE_INVALID;
        break;
    }

    /* Frame type is always "Normal" */
    pPLSCode->isShortFrame = 0;

    /* Pilot */
    if (mdcd_type & 0x01){
        pPLSCode->isPilotOn = 1;
    } else {
        pPLSCode->isPilotOn = 0;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs2_monitor_PreLDPCBER (sony_demod_t * pDemod,
                                                   uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint32_t bitError = 0;
    uint32_t period = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    uint8_t data[5];
    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_PreLDPCBER");

    if ((!pDemod) || (!pBER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS2){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xB2 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xB2) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    B2h       30h       [0]      IFLBER_VALID
     * <SLV-T>    B2h       31h       [3:0]    IFLBER_BITERR[27:24]
     * <SLV-T>    B2h       32h       [7:0]    IFLBER_BITERR[23:16]
     * <SLV-T>    B2h       33h       [7:0]    IFLBER_BITERR[15:8]
     * <SLV-T>    B2h       34h       [7:0]    IFLBER_BITERR[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, data, 5) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        /* Bit error count */
        bitError = ((uint32_t)(data[1] & 0x0F) << 24) |
                   ((uint32_t)(data[2] & 0xFF) << 16) |
                   ((uint32_t)(data[3] & 0xFF) <<  8) |
                    (uint32_t)(data[4] & 0xFF);

        /* Set SLV-T Bank : 0xA0 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * -----------------------------------------------------------------------------
         * <SLV-T>    A0h     7Ah     [3:0]    8'h08      period       OFLBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        period = (uint32_t)(1 << (data[0] & 0x0F));

        if (bitError > (period * 64800)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        /*--------------------------------------------------------------------
          BER = bitError / (period * 64800)
              = (bitError * 10^7) / (period * 64800)
              = (bitError * 10^5) / (period * 648)
              = (bitError * 12500) / (period * 81)
              = (bitError * 10) * 1250 / (period * 81)
        --------------------------------------------------------------------*/
        tempDiv = period * 81;
        if (tempDiv == 0){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (bitError * 10) / tempDiv;
        tempR = (bitError * 10) % tempDiv;

        tempR *= 1250;
        tempQ = (tempQ * 1250) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if (tempR >= (tempDiv/2)){
            *pBER = tempQ + 1;
        } else {
            *pBER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs2_monitor_PreBCHBER (sony_demod_t * pDemod,
                                                  uint32_t * pBER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint32_t bitError = 0;
    uint32_t period = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    uint8_t data[4];
    sony_dvbs2_plscode_t plscode;
    uint32_t tempA = 0;
    uint32_t tempB = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_PreBCHBER");

    if ((!pDemod) || (!pBER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS2){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       2Fh       [0]      IF2BER_VALID
     * <SLV-T>    A0h       30h       [6:0]    IF2BER_BITERR[22:16]
     * <SLV-T>    A0h       31h       [7:0]    IF2BER_BITERR[15:8]
     * <SLV-T>    A0h       32h       [7:0]    IF2BER_BITERR[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2F, data, 4) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        /* Bit error count */
        bitError = ((uint32_t)(data[1] & 0x7F) << 16) |
                   ((uint32_t)(data[2] & 0xFF) <<  8) |
                    (uint32_t)(data[3] & 0xFF);

        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h     BCh     [3:0]    8'h08      period       OF2BER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBC, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        period = (uint32_t)(1 << (data[0] & 0x0F));

        /* Code rate */
        result = sony_demod_dvbs2_monitor_PLSCode (pDemod, &plscode);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }

        /*--------------------------------------------------------------------
          BER = bitError / (period * 64800 * codeRate)       (codeRate = A/B)
              = (bitError * 10^9) / (period * 64800 * A/B)
              = (bitError * 10^7 * B) / (period * 648 * A)
              = (bitError * 1250000 * B) / (period * 81 * A)
              = ((bitError * B * 10) * 125 * 100 * 10) / (period * 81 * A)
        --------------------------------------------------------------------*/
        switch(plscode.codeRate)
        {
        case SONY_DVBS2_CODERATE_1_4:  tempA = 1; tempB =  4; break;
        case SONY_DVBS2_CODERATE_1_3:  tempA = 1; tempB =  3; break;
        case SONY_DVBS2_CODERATE_2_5:  tempA = 2; tempB =  5; break;
        case SONY_DVBS2_CODERATE_1_2:  tempA = 1; tempB =  2; break;
        case SONY_DVBS2_CODERATE_3_5:  tempA = 3; tempB =  5; break;
        case SONY_DVBS2_CODERATE_2_3:  tempA = 2; tempB =  3; break;
        case SONY_DVBS2_CODERATE_3_4:  tempA = 3; tempB =  4; break;
        case SONY_DVBS2_CODERATE_4_5:  tempA = 4; tempB =  5; break;
        case SONY_DVBS2_CODERATE_5_6:  tempA = 5; tempB =  6; break;
        case SONY_DVBS2_CODERATE_8_9:  tempA = 8; tempB =  9; break;
        case SONY_DVBS2_CODERATE_9_10: tempA = 9; tempB = 10; break;
        default: SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }

        if((bitError * tempB) > (period * 64800 * tempA)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempDiv = period * 81 * tempA;
        if (tempDiv == 0){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (bitError * tempB * 10) / tempDiv;
        tempR = (bitError * tempB * 10) % tempDiv;

        tempR *= 125;
        tempQ = (tempQ * 125) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        tempR *= 10;
        tempQ = (tempQ * 10) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if (tempR >= (tempDiv/2)){
            *pBER = tempQ + 1;
        } else {
            *pBER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs2_monitor_PostBCHFER (sony_demod_t * pDemod,
                                                   uint32_t * pFER)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint8_t data[6];
    uint32_t frameError = 0;
    uint32_t frameCount = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_PostBCHFER");

    if ((!pDemod) || (!pFER)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS2){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       2Fh       [0]      IF2BER_VALID
     * <SLV-T>    A0h       33h       [7:0]    IF2BER_FRMERR[15:8]
     * <SLV-T>    A0h       34h       [7:0]    IF2BER_FRMERR[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2F, data, 6) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        frameError = ((uint32_t)(data[4] & 0xFF) << 8) |
                      (uint32_t)(data[5] & 0xFF);

        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h     BCh     [3:0]    8'h08      period       OF2BER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBC, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Measurement period */
        frameCount = (uint32_t)(1 << (data[0] & 0x0F));

        /*--------------------------------------------------------------------
          FER = frameError / frameCount
              = (frameError * 10^6) / frameCount
              = ((frameError * 10000) * 100) / frameCount
        --------------------------------------------------------------------*/
        tempDiv = frameCount;
        if ((tempDiv == 0) || (frameError > frameCount)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (frameError * 10000) / tempDiv;
        tempR = (frameError * 10000) % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if ((tempDiv != 1) && (tempR >= (tempDiv/2))){
            *pFER = tempQ + 1;
        } else {
            *pFER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs2_monitor_BBHeader (sony_demod_t * pDemod,
                                                 sony_dvbs2_bbheader_t * pBBHeader)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem = SONY_DTV_SYSTEM_UNKNOWN;
    uint8_t isTSLock = 0;
    uint8_t data[7];
    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_BBHeader");

    if ((!pDemod) || (!pBBHeader)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Freeze the register. */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch(pDemod->system)
    {
    case SONY_DTV_SYSTEM_ANY:
        result = monitor_System (pDemod, &dtvSystem);
        if (result != SONY_RESULT_OK){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (result);
        }
        if (dtvSystem != SONY_DTV_SYSTEM_DVBS2){
            SLVT_UnFreezeReg (pDemod);
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        break;

    default:
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, &isTSLock);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    if (!isTSLock){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * ------------------------------------------------
     * <SLV-T>    A0h       49h       [7:0]    IBBH[79:72]
     * <SLV-T>    A0h       4Ah       [7:0]    IBBH[71:64]
     * <SLV-T>    A0h       4Bh       [7:0]    IBBH[63:56]
     * <SLV-T>    A0h       4Ch       [7:0]    IBBH[55:48]
     * <SLV-T>    A0h       4Dh       [7:0]    IBBH[47:40]
     * <SLV-T>    A0h       4Eh       [7:0]    IBBH[39:32]
     * <SLV-T>    A0h       4Fh       [7:0]    IBBH[31:24]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x49, data, sizeof (data)) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    SLVT_UnFreezeReg (pDemod);

    /* Convert data to appropriate format. */
    pBBHeader->streamInput = (sony_dvbs2_stream_t) ((data[0] & 0xC0) >> 6);
    pBBHeader->isSingleInputStream = (data[0] & 0x20) >> 5;
    pBBHeader->isConstantCodingModulation = (data[0] & 0x10) >> 4;
    pBBHeader->issyIndicator = (data[0] & 0x08) >> 3;
    pBBHeader->nullPacketDeletion = (data[0] & 0x04) >> 2;
    pBBHeader->rollOff = (sony_dvbs2_rolloff_t) (data[0] & 0x03);
    pBBHeader->inputStreamIdentifier = data[1];
    pBBHeader->userPacketLength = (data[2] << 8);
    pBBHeader->userPacketLength |= data[3];
    pBBHeader->dataFieldLength = (data[4] << 8);
    pBBHeader->dataFieldLength |= data[5];
    pBBHeader->syncByte = data[6];

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_Pilot (sony_demod_t * pDemod,
                                                uint8_t * pPlscLock,
                                                uint8_t * pPilotOn)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_Pilot");

    if ((!pDemod) || (!pPlscLock) || (!pPilotOn)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SLVT_FreezeReg (pDemod);

    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, &data, 1) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    *pPlscLock = (uint8_t)(data & 0x01);
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x21, &data, 1) != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    *pPilotOn = (uint8_t)(data & 0x01);

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs_s2_monitor_ScanInfo (sony_demod_t * pDemod,
                                                   uint8_t * pTSLock,
                                                   int32_t * pOffset,
                                                   sony_dtv_system_t * pSystem)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_ScanInfo");

    if ((!pDemod) || (!pTSLock) || (!pOffset) || (!pSystem)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SLVT_FreezeReg (pDemod);

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, pTSLock);
    if ((result != SONY_RESULT_OK) || (*pTSLock == 0)){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    result = monitor_CarrierOffset (pDemod, pOffset);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    result = monitor_System (pDemod, pSystem);
    if (result != SONY_RESULT_OK){
        SLVT_UnFreezeReg (pDemod);
        SONY_TRACE_RETURN (result);
    }

    SLVT_UnFreezeReg (pDemod);
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbs_s2_monitor_TSRate (sony_demod_t * pDemod,
                                                 uint32_t * pTSRateKbps)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dtv_system_t dtvSystem;
    uint32_t symbolRateSps;
    uint32_t symbolRateKSps;

    SONY_TRACE_ENTER ("sony_demod_dvbs_s2_monitor_TSRate");

    if ((!pDemod) || (!pTSRateKbps)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_monitor_System (pDemod, &dtvSystem);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    result = sony_demod_dvbs_s2_monitor_SymbolRate (pDemod, &symbolRateSps);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    symbolRateKSps = (symbolRateSps + 500) / 1000;
    if (dtvSystem == SONY_DTV_SYSTEM_DVBS) {
        sony_dvbs_coderate_t codeRate;
        uint32_t numerator;
        uint32_t denominator;
        result = sony_demod_dvbs_monitor_CodeRate (pDemod, &codeRate);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        switch (codeRate) {
        case SONY_DVBS_CODERATE_1_2:
            numerator = 1;
            denominator = 2;
            break;
        case SONY_DVBS_CODERATE_2_3:
            numerator = 2;
            denominator = 3;
            break;
        case SONY_DVBS_CODERATE_3_4:
            numerator = 3;
            denominator = 4;
            break;
        case SONY_DVBS_CODERATE_5_6:
            numerator = 5;
            denominator = 6;
            break;
        case SONY_DVBS_CODERATE_7_8:
            numerator = 7;
            denominator = 8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }
        if (symbolRateKSps >= 60000) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        } else {
            /*
             * Bit rate = SR[MSps] * 2 * CodeRate * 188 / 204
             *          = SR[MSps] *     CodeRate * 188 / 102
             *          = SR[MSps] *     CodeRate *  94 /  51
             */
            *pTSRateKbps = ((symbolRateKSps * numerator * 94) + (51 * denominator / 2)) / (51 * denominator);
        }
    } else if (dtvSystem == SONY_DTV_SYSTEM_DVBS2) {
        sony_dvbs2_plscode_t plsCode;
        uint32_t kbch;
        result = sony_demod_dvbs2_monitor_PLSCode (pDemod, &plsCode);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        switch (plsCode.codeRate) {
        case SONY_DVBS2_CODERATE_1_2:
            kbch = 32208;
            break;
        case SONY_DVBS2_CODERATE_3_5:
            kbch = 38688;
            break;
        case SONY_DVBS2_CODERATE_2_3:
            kbch = 43040;
            break;
        case SONY_DVBS2_CODERATE_3_4:
            kbch = 48408;
            break;
        case SONY_DVBS2_CODERATE_4_5:
            kbch = 51648;
            break;
        case SONY_DVBS2_CODERATE_5_6:
            kbch = 53840;
            break;
        case SONY_DVBS2_CODERATE_8_9:
            kbch = 57472;
            break;
        case SONY_DVBS2_CODERATE_9_10:
            kbch = 58192;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }
        if (plsCode.modulation == SONY_DVBS2_MODULATION_8PSK) {
            if (plsCode.isPilotOn) {
                *pTSRateKbps = ((symbolRateKSps * (kbch - 80)) + 11097) / 22194;
            } else {
                *pTSRateKbps = ((symbolRateKSps * (kbch - 80)) + 10845) / 21690;
            }
        } else if (plsCode.modulation == SONY_DVBS2_MODULATION_QPSK) {
            if (plsCode.isPilotOn) {
                *pTSRateKbps = ((symbolRateKSps * (kbch - 80)) + 16641) / 33282;
            } else {
                *pTSRateKbps = ((symbolRateKSps * (kbch - 80)) + 16245) / 32490;
            }
        } else {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    SONY_TRACE_RETURN (result);
}

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static sony_result_t monitor_System (sony_demod_t * pDemod,
                                     sony_dtv_system_t * pSystem)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isTSLock = 0;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("monitor_System");

    if ((!pDemod) || (!pSystem)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, &isTSLock);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    if (!isTSLock){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       50h       [1:0]    IMODE[1:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x50, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch(data & 0x03)
    {
    case 0x00:
        *pSystem = SONY_DTV_SYSTEM_DVBS2;
        break;

    case 0x01:
        *pSystem = SONY_DTV_SYSTEM_DVBS;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t monitor_CarrierOffset (sony_demod_t * pDemod,
                                            int32_t * pOffset)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t iqInv = 0;
    uint8_t isHSMode = 0;
    uint8_t data[3];
    uint32_t regValue = 0;
    int32_t cfrl_ctrlval = 0;

    SONY_TRACE_ENTER ("monitor_CarrierOffset");

    result = monitor_SamplingRateMode (pDemod, &isHSMode);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       45h       [4:0]    ICFRL_CTRLVAL[20:16]
     * <SLV-T>    A0h       46h       [7:0]    ICFRL_CTRLVAL[15:8]
     * <SLV-T>    A0h       47h       [7:0]    ICFRL_CTRLVAL[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x45, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    regValue = (((uint32_t)data[0] & 0x1F) << 16) | (((uint32_t)data[1] & 0xFF) <<  8) | ((uint32_t)data[2] & 0xFF);
    cfrl_ctrlval = sony_Convert2SComplement(regValue, 21);

    if (isHSMode){
        /*----------------------------------------------------------------
          samplingRate = 96000 (KHz)

          Offset(KHz) = cfrl_ctrlval * (-1) * 96000 / 2^20
                      = cfrl_ctrlval * (-1) * (375 * 2^8) / 2^20
                      = cfrl_ctrlval * (-1) * 375 / 2^12
        ----------------------------------------------------------------*/
        if(cfrl_ctrlval > 0){
            *pOffset = ((cfrl_ctrlval * (-375)) - 2048) / 4096;
        } else {
            *pOffset = ((cfrl_ctrlval * (-375)) + 2048) / 4096;
        }
    } else {
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
    }

    result = sony_demod_dvbs_s2_CheckIQInvert (pDemod, &iqInv);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (iqInv){
        *pOffset *= (-1);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t s_monitor_CodeRate (sony_demod_t * pDemod,
                                         sony_dvbs_coderate_t * pCodeRate)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("s_monitor_CodeRate");

    if ((!pDemod) || (!pCodeRate)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xB1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xB1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    B1h       10h       [7]      ISYND_VAL
     * <SLV-T>    B1h       10h       [2:0]    ISYND_CDRATE[2:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data & 0x80){
        switch(data & 0x07)
        {
        case 0x00:
            *pCodeRate = SONY_DVBS_CODERATE_1_2;
            break;

        case 0x01:
            *pCodeRate = SONY_DVBS_CODERATE_2_3;
            break;

        case 0x02:
            *pCodeRate = SONY_DVBS_CODERATE_3_4;
            break;

        case 0x03:
            *pCodeRate = SONY_DVBS_CODERATE_5_6;
            break;

        case 0x04:
            *pCodeRate = SONY_DVBS_CODERATE_7_8;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t monitor_SamplingRateMode (sony_demod_t * pDemod,
                                               uint8_t * pIsHSMode)
{
    uint8_t data = 0;

    SONY_TRACE_ENTER ("monitor_SamplingRateMode");

    if ((!pDemod) || (!pIsHSMode)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       10h       [0]      ITRL_LOCK
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if (data & 0x01){
        /*  slave     Bank      Addr      Bit      Signal name
         * --------------------------------------------------------------
         * <SLV-T>    A0h       50h       [4]      IHSMODE
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x50, &data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        if (data & 0x10){
            /* High sample mode */
            *pIsHSMode = 1;
        } else {
            /* Low sample mode */
            *pIsHSMode = 0;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t s_monitor_IQSense (sony_demod_t * pDemod,
                                        sony_demod_sat_iq_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isTSLock = 0;
    uint8_t isInverted = 0;
    uint8_t data = 0;
    sony_dvbs_coderate_t codeRate;

    SONY_TRACE_ENTER ("s_monitor_IQSense");

    if ((!pDemod) || (!pSense)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, &isTSLock);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (!isTSLock){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    result = s_monitor_CodeRate (pDemod, &codeRate);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    /* Set SLV-T Bank : 0xB1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xB1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch(codeRate)
    {
    case SONY_DVBS_CODERATE_1_2:
    case SONY_DVBS_CODERATE_2_3:
    case SONY_DVBS_CODERATE_3_4:
    case SONY_DVBS_CODERATE_7_8:
        /*  slave     Bank      Addr      Bit      Signal name
         * --------------------------------------------------------------
         * <SLV-T>    B1h       3Eh       [0]      ISYND_IQTURN
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x3E, &data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    case SONY_DVBS_CODERATE_5_6:
        /*  slave     Bank      Addr      Bit      Signal name
         * --------------------------------------------------------------
         * <SLV-T>    B1h       5Dh       [0]      IBYTE_R56IQINV
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x5D, &data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    result = sony_demod_dvbs_s2_CheckIQInvert (pDemod, &isInverted);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (isInverted){
        *pSense = (data & 0x01) ? SONY_DEMOD_SAT_IQ_SENSE_NORMAL : SONY_DEMOD_SAT_IQ_SENSE_INV;
    } else {
        *pSense = (data & 0x01) ? SONY_DEMOD_SAT_IQ_SENSE_INV : SONY_DEMOD_SAT_IQ_SENSE_NORMAL;
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t s_monitor_CNR (sony_demod_t * pDemod,
                                    int32_t * pCNR)
{
    int32_t index = 0;
    int32_t minIndex = 0;
    int32_t maxIndex = 0;
    uint8_t data[3];
    uint32_t value = 0;

    SONY_TRACE_ENTER ("s_monitor_CNR");

    if ((!pDemod) || (!pCNR)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xA1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit     Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A1h       10h       [0]     ICPM_QUICKRDY
     * <SLV-T>    A1h       11h       [4:0]   ICPM_QUICKCNDT[12:8]
     * <SLV-T>    A1h       12h       [7:0]   ICPM_QUICKCNDT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if(data[0] & 0x01){
        value = ((uint32_t)(data[1] & 0x1F) << 8) | (uint32_t)(data[2] & 0xFF);

        minIndex = 0;
        maxIndex = sizeof(s_cn_data)/sizeof(s_cn_data[0]) - 1;

        if(value >= s_cn_data[minIndex].value){
            *pCNR = s_cn_data[minIndex].cnr_x1000;
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }
        if(value <= s_cn_data[maxIndex].value){
            *pCNR = s_cn_data[maxIndex].cnr_x1000;
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }

        while ((maxIndex - minIndex) > 1){

            index = (maxIndex + minIndex) / 2;

            if (value == s_cn_data[index].value){
                *pCNR = s_cn_data[index].cnr_x1000;
                SONY_TRACE_RETURN (SONY_RESULT_OK);
            } else if (value > s_cn_data[index].value){
                maxIndex = index;
            } else {
                minIndex = index;
            }

            if ((maxIndex - minIndex) <= 1){
                if (value == s_cn_data[maxIndex].value){
                    *pCNR = s_cn_data[maxIndex].cnr_x1000;
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                } else {
                    *pCNR = s_cn_data[minIndex].cnr_x1000;
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                }
            }
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Error */
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
}

static sony_result_t s_monitor_PER (sony_demod_t * pDemod,
                                    uint32_t * pPER)
{
    uint8_t data[3];
    uint32_t packetError = 0;
    uint32_t period = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER ("s_monitor_PER");

    if ((!pDemod) || (!pPER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xA0 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A0h       25h       [0]      IFBER_VALID
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x25, data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        /*  slave     Bank      Addr      Bit      Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h       2Ch       [1:0]    IFBER_PKTERR[17:16]
         * <SLV-T>    A0h       2Dh       [7:0]    IFBER_PKTERR[15:8]
         * <SLV-T>    A0h       2Eh       [7:0]    IFBER_PKTERR[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2C, data, 3) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        packetError = ((uint32_t)(data[0]  & 0x03) << 16) |
                      ((uint32_t)(data[1]  & 0xFF) <<  8) |
                       (uint32_t)(data[2]  & 0xFF);

        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h     BAh     [3:0]    8'h08      period       OFBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBA, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        period = (uint32_t)(1 << (data[0] & 0x0F));
        /*--------------------------------------------------------------------
          PER = packetError / (period * 8)
              = (packetError * 10^6) / (period * 8)
              = (packetError * 125000) / period
              = (packetError * 1000) * 125 / period
        --------------------------------------------------------------------*/
        tempDiv = period;
        if ((tempDiv == 0) || (packetError > (period * 8))){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (packetError * 1000) / tempDiv;
        tempR = (packetError * 1000) % tempDiv;

        tempR *= 125;
        tempQ = (tempQ * 125) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if ((tempDiv != 1) && (tempR >= (tempDiv/2))){
            *pPER = tempQ + 1;
        } else {
            *pPER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t s2_monitor_IQSense (sony_demod_t * pDemod,
                                         sony_demod_sat_iq_sense_t * pSense)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t isTSLock = 0;
    uint8_t isInverted = 0;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("s2_monitor_IQSense");

    if ((!pDemod) || (!pSense)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbs_s2_monitor_SyncStat (pDemod, &isTSLock);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (!isTSLock){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    /* Set SLV-T Bank : 0xAB */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xAB) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    ABh       23h       [0]      ICFRL_FIQINV
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x23, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    result = sony_demod_dvbs_s2_CheckIQInvert (pDemod, &isInverted);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    if (isInverted){
        *pSense = (data & 0x01) ? SONY_DEMOD_SAT_IQ_SENSE_NORMAL : SONY_DEMOD_SAT_IQ_SENSE_INV;
    } else {
        *pSense = (data & 0x01) ? SONY_DEMOD_SAT_IQ_SENSE_INV : SONY_DEMOD_SAT_IQ_SENSE_NORMAL;
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t s2_monitor_CNR (sony_demod_t * pDemod,
                                     int32_t * pCNR)
{
    int32_t index = 0;
    int32_t minIndex = 0;
    int32_t maxIndex = 0;
    uint8_t data[3];
    uint32_t value = 0;

    SONY_TRACE_ENTER ("s2_monitor_CNR");

    if ((!pDemod) || (!pCNR)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if ((pDemod->system != SONY_DTV_SYSTEM_DVBS2) &&
        (pDemod->system != SONY_DTV_SYSTEM_ANY)) {
        /* Not DVB-S2 or ANY */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Set SLV-T Bank : 0xA1 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit     Signal name
     * --------------------------------------------------------------
     * <SLV-T>    A1h       10h       [0]     ICPM_QUICKRDY
     * <SLV-T>    A1h       11h       [4:0]   ICPM_QUICKCNDT[12:8]
     * <SLV-T>    A1h       12h       [7:0]   ICPM_QUICKCNDT[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if(data[0] & 0x01){
        value = ((uint32_t)(data[1] & 0x1F) << 8) | (uint32_t)(data[2] & 0xFF);

        minIndex = 0;
        maxIndex = sizeof(s2_cn_data)/sizeof(s2_cn_data[0]) - 1;

        if(value >= s2_cn_data[minIndex].value){
            *pCNR = s2_cn_data[minIndex].cnr_x1000;
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }
        if(value <= s2_cn_data[maxIndex].value){
            *pCNR = s2_cn_data[maxIndex].cnr_x1000;
            SONY_TRACE_RETURN (SONY_RESULT_OK);
        }

        while ((maxIndex - minIndex) > 1){

            index = (maxIndex + minIndex) / 2;

            if (value == s2_cn_data[index].value){
                *pCNR = s2_cn_data[index].cnr_x1000;
                SONY_TRACE_RETURN (SONY_RESULT_OK);
            } else if (value > s2_cn_data[index].value){
                maxIndex = index;
            } else {
                minIndex = index;
            }

            if ((maxIndex - minIndex) <= 1){
                if (value == s2_cn_data[maxIndex].value){
                    *pCNR = s2_cn_data[maxIndex].cnr_x1000;
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                } else {
                    *pCNR = s2_cn_data[minIndex].cnr_x1000;
                    SONY_TRACE_RETURN (SONY_RESULT_OK);
                }
            }
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }
    /* Error */
    SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
}

static sony_result_t s2_monitor_PER (sony_demod_t * pDemod,
                                     uint32_t * pPER)
{
    uint8_t data[3];
    uint32_t packetError = 0;
    uint32_t packetCount = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER ("s2_monitor_PER");

    if ((!pDemod) || (!pPER)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0xB8 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xB8) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*  slave     Bank      Addr      Bit      Signal name
     * --------------------------------------------------------------
     * <SLV-T>    B8h       26h       [0]      IPERVALID
     * <SLV-T>    B8h       27h       [7:0]    IPERPKTERR[15:8]
     * <SLV-T>    B8h       28h       [7:0]    IPERPKTERR[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x26, data, 3) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (data[0] & 0x01){
        packetError = ((uint32_t)(data[1] & 0xFF) << 8) |
                       (uint32_t)(data[2] & 0xFF);

        /* Set SLV-T Bank : 0xA0 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /*  slave     Bank    Addr    Bit     Default    Setting       Signal name
         * ----------------------------------------------------------------------------------
         * <SLV-T>    A0h     BAh     [3:0]    8'h08      period       OFBER_MES[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xBA, data, 1) != SONY_RESULT_OK){
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        packetCount = (uint32_t)(1 << (data[0] & 0x0F));

        /*--------------------------------------------------------------------
          PER = packetError / packetCount
              = (packetError * 10^6) / packetCount
              = ((packetError * 10000) * 100) / packetCount
        --------------------------------------------------------------------*/
        tempDiv = packetCount;
        if ((tempDiv == 0) || (packetError > packetCount)){
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_HW_STATE);
        }

        tempQ = (packetError * 10000) / tempDiv;
        tempR = (packetError * 10000) % tempDiv;

        tempR *= 100;
        tempQ = (tempQ * 100) + (tempR / tempDiv);
        tempR = tempR % tempDiv;

        if ((tempDiv != 1) && (tempR >= (tempDiv/2))){
            *pPER = tempQ + 1;
        } else {
            *pPER = tempQ;
        }
    } else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_dvbs2_monitor_Rolloff (sony_demod_t * pDemod,
                                                uint8_t * pRolloff)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data=0x00;

    SONY_TRACE_ENTER ("sony_demod_dvbs2_monitor_Rolloff");

    if ((!pDemod) || (!pRolloff)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0xA0) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    if(pDemod->pI2c->ReadRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x49, &data, 1) != SONY_RESULT_OK){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    *pRolloff=(data & 0x03);
    SONY_TRACE_RETURN (result);
}
