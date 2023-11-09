/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
#include "sony_tuner_ascot3.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_ascot3_version =  SONY_ASCOT3_VERSION;

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot3_Initialize (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot3_Tune (sony_tuner_terr_cable_t * pTuner,
                                             uint32_t frequency,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_ascot3_Sleep (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot3_Shutdown (sony_tuner_terr_cable_t * pTuner);

static sony_result_t sony_tuner_ascot3_ReadRFLevel (sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_ascot3_Create (sony_tuner_terr_cable_t * pTuner,
                                        sony_ascot3_xtal_t xtalFreq,
                                        uint8_t i2cAddress,
                                        sony_i2c_t * pI2c,
                                        uint32_t configFlags,
                                        sony_ascot3_t * pAscot3Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Create");

    if ((!pI2c) || (!pAscot3Tuner) || (!pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create the underlying Ascot3 reference driver. */
    result = sony_ascot3_Create (pAscot3Tuner, xtalFreq, i2cAddress, pI2c, configFlags);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_ascot3_Initialize;
    pTuner->Tune = sony_tuner_ascot3_Tune;
    pTuner->Sleep = sony_tuner_ascot3_Sleep;
    pTuner->Shutdown = sony_tuner_ascot3_Shutdown;
    pTuner->ReadRFLevel = sony_tuner_ascot3_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pAscot3Tuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_2875_Create (sony_tuner_terr_cable_t * pTuner,
                                             uint8_t i2cAddress,
                                             sony_i2c_t * pI2c,
                                             uint32_t configFlags,
                                             sony_ascot3_t * pAscot3Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_2875_Create");

    if ((!pI2c) || (!pAscot3Tuner) || (!pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create the underlying Ascot3R reference driver. */
    result = sony_ascot3_2875_Create(pAscot3Tuner, i2cAddress, pI2c, configFlags);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_ascot3_Initialize;
    pTuner->Tune = sony_tuner_ascot3_Tune;
    pTuner->Sleep = sony_tuner_ascot3_Sleep;
    pTuner->Shutdown = sony_tuner_ascot3_Shutdown;
    pTuner->ReadRFLevel = sony_tuner_ascot3_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pAscot3Tuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_RFFilterConfig (sony_tuner_terr_cable_t * pTuner, uint8_t coeff, uint8_t offset)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_RFFilterConfig");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_RFFilterConfig (((sony_ascot3_t *) pTuner->user), coeff, offset);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_SetGPO (sony_tuner_terr_cable_t * pTuner, uint8_t id, uint8_t value)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Write_GPIO");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_SetGPO (((sony_ascot3_t *) pTuner->user), id, value);

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_ascot3_ReadRssi (sony_tuner_terr_cable_t * pTuner, int32_t * pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_ReadRssi");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_ReadRssi (((sony_ascot3_t *) pTuner->user), pRssi);

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_ascot3_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Initialize");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Initialize (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Tune (sony_tuner_terr_cable_t * pTuner,
                                             uint32_t frequencyKHz,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_ascot3_tv_system_t aSystem;

    SONY_TRACE_ENTER ("sony_tuner_ascot3_Tune");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Convert system, bandwidth into dtv system. */
    switch (system) {
    case SONY_DTV_SYSTEM_DVBC:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBC_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            /* 7MHZ BW setting is the same as 8MHz BW */
        case SONY_DTV_BW_8_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBC_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT:
        switch (bandwidth) {
        case SONY_DTV_BW_5_MHZ:
             aSystem = SONY_ASCOT3_DTV_DVBT_5;
            break;
        case SONY_DTV_BW_6_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_DVBT2:
        switch (bandwidth) {
        case SONY_DTV_BW_1_7_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT2_1_7;
            break;
        case SONY_DTV_BW_5_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT2_5;
            break;
        case SONY_DTV_BW_6_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT2_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT2_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBT2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);

        }
        break;

    case SONY_DTV_SYSTEM_DVBC2:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBC2_6;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = SONY_ASCOT3_DTV_DVBC2_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBT:
        switch (bandwidth) {
        case SONY_DTV_BW_6_MHZ:
            aSystem = SONY_ASCOT3_DTV_ISDBT_6;
            break;
        case SONY_DTV_BW_7_MHZ:
            aSystem = SONY_ASCOT3_DTV_ISDBT_7;
            break;
        case SONY_DTV_BW_8_MHZ:
            aSystem = SONY_ASCOT3_DTV_ISDBT_8;
            break;
        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }
        break;

    case SONY_DTV_SYSTEM_ISDBC:
        aSystem = SONY_ASCOT3_DTV_DVBC_6; /* ISDB-C uses DVB-C 6MHz BW setting */
        break;

    case SONY_DTV_SYSTEM_J83B:
        switch (bandwidth) {
        case SONY_DTV_BW_J83B_5_06_5_36_MSPS:
            aSystem = SONY_ASCOT3_DTV_DVBC_6; /* J.83B (5.057, 5.361Msps commonly used in US) uses DVB-C 6MHz BW setting */
            break;
        case SONY_DTV_BW_J83B_5_60_MSPS:
            aSystem = SONY_ASCOT3_DTV_J83B_5_6; /* J.83B (5.6Msps used in Japan) uses special setting */
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

    /* Call into underlying driver. */
    result = sony_ascot3_Tune(((sony_ascot3_t *) pTuner->user), frequencyKHz, aSystem);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Allow the tuner time to settle */
    SONY_SLEEP(50);

    result = sony_ascot3_TuneEnd((sony_ascot3_t *) pTuner->user);
    if (result != SONY_RESULT_OK) {
        pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
        pTuner->frequencyKHz = 0;
        pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
        SONY_TRACE_RETURN (result);
    }

    /* Assign current values. */
    pTuner->system = system;
    pTuner->frequencyKHz = ((sony_ascot3_t *) pTuner->user)->frequencykHz;
    pTuner->bandwidth = bandwidth;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Sleep");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Sleep (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_Shutdown (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */
    result = sony_ascot3_Sleep (((sony_ascot3_t *) pTuner->user));

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_ascot3_ReadRFLevel (sony_tuner_terr_cable_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_ascot3_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    result = sony_ascot3_ReadRssi (((sony_ascot3_t *) pTuner->user), pRFLevel);
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
