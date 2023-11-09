/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_tuner_sat.h"
#include "sony_tuner_horus3a.h"
#include "sony_horus3a.h"

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_horus3a_Initialize (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_horus3a_Tune (sony_tuner_sat_t * pTuner,
                                              uint32_t centerFreqKHz,
                                              sony_dtv_system_t dtvSystem,
                                              uint32_t symbolRateKSps);

static sony_result_t sony_tuner_horus3a_Sleep (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_horus3a_Shutdown (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_horus3a_AGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                        uint32_t AGCLevel,
                                                        int32_t * pAGCdB);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_horus3a_Create (sony_tuner_sat_t * pTuner,
                                         uint32_t xtalFreqMHz,
                                         uint8_t i2cAddress,
                                         sony_i2c_t * pI2c,
                                         uint32_t configFlags,
                                         sony_horus3a_t * pHorus3a)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_tuner_horus3a_Create");

    if ((!pI2c) || (!pHorus3a) || (!pTuner)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_horus3a_Create (pHorus3a, xtalFreqMHz, i2cAddress, pI2c, configFlags);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->frequencyKHz = 0;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->symbolRateKSps = 0;
    pTuner->symbolRateKSpsForSpectrum = 45000;
    pTuner->Initialize = sony_tuner_horus3a_Initialize;
    pTuner->Tune = sony_tuner_horus3a_Tune;
    pTuner->Sleep = sony_tuner_horus3a_Sleep;
    pTuner->Shutdown = sony_tuner_horus3a_Shutdown;
    pTuner->AGCLevel2AGCdB = sony_tuner_horus3a_AGCLevel2AGCdB;
    pTuner->user = pHorus3a;

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_tuner_horus3a_SetGPO (sony_tuner_sat_t * pTuner, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_horus3a_SetGPO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_horus3a_SetGPO (((sony_horus3a_t *) pTuner->user), value);

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_horus3a_Initialize (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_horus3a_t * pHorus3a = NULL;

    SONY_TRACE_ENTER ("sony_tuner_horus3a_Initialize");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pHorus3a = (sony_horus3a_t *)(pTuner->user);

    result = sony_horus3a_Initialize (pHorus3a);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->frequencyKHz = 0;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_horus3a_Tune (sony_tuner_sat_t * pTuner,
                                              uint32_t centerFreqKHz,
                                              sony_dtv_system_t dtvSystem,
                                              uint32_t symbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_horus3a_t * pHorus3a = NULL;
    sony_horus3a_tv_system_t satSystem = SONY_HORUS3A_STV_DVBS2;

    SONY_TRACE_ENTER ("sony_tuner_horus3a_Tune");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pHorus3a = (sony_horus3a_t *)(pTuner->user);

    switch(dtvSystem)
    {
    case SONY_DTV_SYSTEM_DVBS:
        satSystem = SONY_HORUS3A_STV_DVBS;
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        satSystem = SONY_HORUS3A_STV_DVBS2;
        break;

    case SONY_DTV_SYSTEM_ISDBS:
    case SONY_DTV_SYSTEM_ISDBS3:
        satSystem = SONY_HORUS3A_STV_ISDBS;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_horus3a_Tune (pHorus3a, centerFreqKHz, satSystem, symbolRateKSps);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->frequencyKHz = pHorus3a->frequencykHz;
    pTuner->system = dtvSystem;
    pTuner->symbolRateKSps = symbolRateKSps;

    /* Tuner stabillization time */
    SONY_SLEEP (50);

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_horus3a_Sleep (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_horus3a_t * pHorus3a = NULL;

    SONY_TRACE_ENTER ("sony_tuner_horus3a_Sleep");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pHorus3a = (sony_horus3a_t *)(pTuner->user);

    result = sony_horus3a_Sleep (pHorus3a);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->frequencyKHz = 0;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_horus3a_Shutdown (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_horus3a_t * pHorus3a = NULL;

    SONY_TRACE_ENTER ("sony_tuner_horus3a_Shutdown");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pHorus3a = (sony_horus3a_t *)(pTuner->user);

    result = sony_horus3a_Sleep (pHorus3a);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->frequencyKHz = 0;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_horus3a_AGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                        uint32_t AGCLevel,
                                                        int32_t * pAGCdB)
{
    int32_t tempA = 0;
    uint8_t isNegative = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER("sony_tuner_horus3a_AGCLevel2AGCdB");

    if ((!pTuner) || (!pAGCdB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*------------------------------------------------
      Gain_db      = (AGCLevel - 8192) * (-  33/1024) -   142
      Gain_db_x100 = (AGCLevel - 8192) * (-3300/1024) - 14200
                   = (AGCLevel - 8192) * (- 825/ 256) - 14200
    ------------------------------------------------*/
    tempA = (int32_t)AGCLevel - 8192;
    tempA *= -825;

    tempDiv = 256;
    if (tempA > 0){
        isNegative = 0;
        tempQ = (uint32_t)tempA / tempDiv;
        tempR = (uint32_t)tempA % tempDiv;
    } else {
        isNegative = 1;
        tempQ = (uint32_t)(tempA * (-1)) / tempDiv;
        tempR = (uint32_t)(tempA * (-1)) % tempDiv;
    }

    if (isNegative){
        if (tempR >= (tempDiv/2)){
            *pAGCdB = (int32_t)(tempQ + 1) * (int32_t)(-1);
        } else {
            *pAGCdB = (int32_t)tempQ * (int32_t)(-1);
        }
    } else {
        if (tempR >= (tempDiv/2)){
            *pAGCdB = (int32_t)(tempQ + 1);
        } else {
            *pAGCdB = (int32_t)tempQ;
        }
    }
    *pAGCdB -= 14200;

    if(*pAGCdB < 0){
        *pAGCdB = 0;
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
