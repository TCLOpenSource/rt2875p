/*------------------------------------------------------------------------------
  Copyright 2017 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
#include "sony_tuner_helene2.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_helene2_version =  SONY_HELENE2_VERSION;

/*------------------------------------------------------------------------------
 Static Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_helene2_terr_Initialize (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_helene2_terr_Tune (sony_tuner_terr_cable_t * pTuner,
                                                   uint32_t centerFreqKHz,
                                                   sony_dtv_system_t dtvSystem,
                                                   sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_helene2_terr_Sleep (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_helene2_terr_Shutdown (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_helene2_terr_ReadRFLevel (sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel);

static sony_result_t sony_tuner_helene2_sat_Initialize (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_helene2_sat_Tune (sony_tuner_sat_t * pTuner,
                                                  uint32_t centerFreqKHz,
                                                  sony_dtv_system_t dtvSystem,
                                                  uint32_t symbolRateKSps);

static sony_result_t sony_tuner_helene2_sat_Sleep (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_helene2_sat_Shutdown (sony_tuner_sat_t * pTuner);

static sony_result_t sony_tuner_helene2_sat_AGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                            uint32_t AGCLevel,
                                                            int32_t * pAGCdB);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_helene2_Create (sony_tuner_terr_cable_t * pTunerTerrCable,
                                         sony_tuner_sat_t *pTunerSat,
                                         sony_helene2_xtal_t xtalFreq,
                                         uint8_t i2cAddress,
                                         sony_i2c_t * pI2c,
                                         uint32_t configFlags,
                                         sony_helene2_t * pHelene2Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_Create");

    if ((!pI2c) || (!pHelene2Tuner) || (!pTunerTerrCable) || (!pTunerSat)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create the underlying HELENE2 reference driver. */
    result = sony_helene2_Create (pHelene2Tuner, xtalFreq, i2cAddress, pI2c, configFlags);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Populate / Initialise the sony_tuner_terr_cable_t structure */
    pTunerTerrCable->i2cAddress = i2cAddress;
    pTunerTerrCable->pI2c = pI2c;
    pTunerTerrCable->flags = configFlags;
    pTunerTerrCable->frequencyKHz = 0;
    pTunerTerrCable->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTunerTerrCable->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTunerTerrCable->Initialize = sony_tuner_helene2_terr_Initialize;
    pTunerTerrCable->Tune = sony_tuner_helene2_terr_Tune;
    pTunerTerrCable->Sleep = sony_tuner_helene2_terr_Sleep;
    pTunerTerrCable->Shutdown = sony_tuner_helene2_terr_Shutdown;
    pTunerTerrCable->ReadRFLevel = sony_tuner_helene2_terr_ReadRFLevel;
    pTunerTerrCable->CalcRFLevelFromAGC = NULL;
    pTunerTerrCable->user = pHelene2Tuner;

    /* Populate / Initialise the sony_tuner_sat_t structure */
    pTunerSat->i2cAddress = i2cAddress;
    pTunerSat->pI2c = pI2c;
    pTunerSat->flags = configFlags;
    pTunerSat->frequencyKHz = 0;
    pTunerSat->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTunerSat->symbolRateKSps = 0;
    pTunerSat->symbolRateKSpsForSpectrum = 45000;
    pTunerSat->Initialize = sony_tuner_helene2_sat_Initialize;
    pTunerSat->Tune = sony_tuner_helene2_sat_Tune;
    pTunerSat->Sleep = sony_tuner_helene2_sat_Sleep;
    pTunerSat->Shutdown = sony_tuner_helene2_sat_Shutdown;
    pTunerSat->AGCLevel2AGCdB = sony_tuner_helene2_sat_AGCLevel2AGCdB;
    pTunerSat->user = pHelene2Tuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_helene2_SetGPO (sony_tuner_terr_cable_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_Write_GPIO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_SetGPO (((sony_helene2_t *) pTuner->user), id, value);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_helene2_GetGPI1 (sony_tuner_terr_cable_t * pTuner, uint8_t * pValue)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_GetGPI1");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_GetGPI1 (((sony_helene2_t *) pTuner->user), pValue);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_helene2_RFFilterConfig (sony_tuner_terr_cable_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_RFFilterConfig");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_RFFilterConfig (((sony_helene2_t *) pTuner->user), coeff, offset);

    SONY_TRACE_RETURN (result);
}
/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_helene2_terr_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_terr_Initialize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_helene2_Initialize (((sony_helene2_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_terr_Tune (sony_tuner_terr_cable_t * pTuner,
                                                   uint32_t centerFreqKHz,
                                                   sony_dtv_system_t system,
                                                   sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_helene2_tv_system_t hSystem = SONY_HELENE2_TV_SYSTEM_UNKNOWN;

    SONY_TRACE_ENTER ("sony_tuner_helene2_terr_Tune");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

   /* Call into underlying driver. Convert system, bandwidth into dtv system. */
    switch (system) {
    case SONY_DTV_SYSTEM_DVBC:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBC_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */
        case SONY_DTV_BW_8_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBC_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth) {
        case SONY_DTV_BW_5_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT_5;
            break;
        case SONY_DTV_BW_6_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth) {
        case SONY_DTV_BW_1_7_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT2_1_7;
            break;
        case SONY_DTV_BW_5_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT2_5;
            break;
        case SONY_DTV_BW_6_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT2_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT2_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBT2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBC2_6;
            break;
        case SONY_DTV_BW_8_MHZ:
            hSystem = SONY_HELENE2_DTV_DVBC2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            hSystem = SONY_HELENE2_DTV_ISDBT_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            hSystem = SONY_HELENE2_DTV_ISDBT_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            hSystem = SONY_HELENE2_DTV_ISDBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBC:
        hSystem = SONY_HELENE2_DTV_DVBC_6; /* ISDB-C uses DVB-C 6MHz BW setting */
        break;

    case SONY_DTV_SYSTEM_J83B:
        switch (bandwidth) {
        case SONY_DTV_BW_J83B_5_06_5_36_MSPS:
            hSystem = SONY_HELENE2_DTV_DVBC_6; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            break;
        case SONY_DTV_BW_J83B_5_60_MSPS:
            hSystem = SONY_HELENE2_DTV_J83B_5_6; /* J.83B (5.6Msps used in Japan) uses special setting */
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    /* Intentional fall-through */
    case SONY_DTV_SYSTEM_UNKNOWN:
    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_terr_Tune(((sony_helene2_t *) pTuner->user), centerFreqKHz, hSystem);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    result = sony_helene2_terr_TuneEnd((sony_helene2_t *) pTuner->user);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = ((sony_helene2_t *) pTuner->user)->frequencykHz;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_terr_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_terr_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_helene2_Sleep (((sony_helene2_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_terr_Shutdown (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_terr_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_helene2_Sleep (((sony_helene2_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_terr_ReadRFLevel (sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_terr_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_terr_ReadRssi (((sony_helene2_t *) pTuner->user), pRFLevel);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    *pRFLevel *= 10; /* dB x 1000 value should be returned. */

    /* Add IFOUT value */
    switch (pTuner->system) {
    case SONY_DTV_SYSTEM_DVBT:
    case SONY_DTV_SYSTEM_DVBT2:
    case SONY_DTV_SYSTEM_DVBC2:
        *pRFLevel -= 4000; /* -4.0dBm */
        break;

    case SONY_DTV_SYSTEM_DVBC:
    case SONY_DTV_SYSTEM_ISDBC:
    case SONY_DTV_SYSTEM_J83B:
        *pRFLevel -= 1500; /* -1.5dBm */
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        *pRFLevel -= 4500; /* -4.5dBm */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_sat_Initialize (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_sat_Initialize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Tuner IC initialization is normally unnecessary because it is done in terrestrial side. */
#if 0
    result = sony_helene2_Initialize (((sony_helene2_t *) pTuner->user));
#endif

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_sat_Tune (sony_tuner_sat_t * pTuner,
                                                  uint32_t centerFreqKHz,
                                                  sony_dtv_system_t dtvSystem,
                                                  uint32_t symbolRateKSps)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_helene2_tv_system_t hSystem = SONY_HELENE2_STV_DVBS2;

    SONY_TRACE_ENTER ("sony_tuner_helene2_sat_Tune");

    if ((!pTuner) || (!pTuner->user)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    switch(dtvSystem)
    {
    case SONY_DTV_SYSTEM_DVBS:
        hSystem = SONY_HELENE2_STV_DVBS;
        break;

    case SONY_DTV_SYSTEM_DVBS2:
        hSystem = SONY_HELENE2_STV_DVBS2;
        break;

    case SONY_DTV_SYSTEM_ISDBS:
        hSystem = SONY_HELENE2_STV_ISDBS;
        break;

    case SONY_DTV_SYSTEM_ISDBS3:
        hSystem = SONY_HELENE2_STV_ISDBS3;
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_helene2_sat_Tune (((sony_helene2_t *) pTuner->user), centerFreqKHz, hSystem, symbolRateKSps);
    if (result != SONY_RESULT_OK){SONY_TRACE_RETURN (result);}

    pTuner->frequencyKHz = ((sony_helene2_t *) pTuner->user)->frequencykHz;
    pTuner->system = dtvSystem;
    pTuner->symbolRateKSps = symbolRateKSps;

    /* Tuner stabillization time */
    SONY_SLEEP (50);

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_sat_Sleep (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_sat_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_helene2_Sleep (((sony_helene2_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_sat_Shutdown (sony_tuner_sat_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_helene2_sat_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_helene2_Sleep (((sony_helene2_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->symbolRateKSps = 0;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_helene2_sat_AGCLevel2AGCdB (sony_tuner_sat_t * pTuner,
                                                            uint32_t AGCLevel,
                                                            int32_t * pAGCdB)
{
    int32_t tempA = 0;
    uint8_t isNegative = 0;
    uint32_t tempDiv = 0;
    uint32_t tempQ = 0;
    uint32_t tempR = 0;
    SONY_TRACE_ENTER("sony_tuner_helene2_sat_AGCLevel2AGCdB");

    if ((!pTuner) || (!pAGCdB)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*------------------------------------------------
      Gain_db      = AGCLevel * (-14   / 403) + 97
      Gain_db_x100 = AGCLevel * (-1400 / 403) + 9700
    -------------------------------------------------*/
    tempA = (int32_t)AGCLevel * (-1400);

    tempDiv = 403;
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
    *pAGCdB += 9700;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
