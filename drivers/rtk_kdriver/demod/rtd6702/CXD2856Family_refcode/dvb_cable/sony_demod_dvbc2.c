/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/11/01
  Modification ID : d7aad0326b97a1da232f01f412ed67152f80f3b8
------------------------------------------------------------------------------*/
#include "sony_demod_dvbc2.h"
#include "sony_demod_dvbc2_monitor.h"
#include "sony_stdlib.h"

/*------------------------------------------------------------------------------
 Constant static data
------------------------------------------------------------------------------*/
static const uint8_t BNW_GI[] = { 2, 4, 2, 2 };         /* Constant array of values to be used for bnw, gi is index into constant array */
static const uint8_t DX_GI[] = { 24, 12, 24, 24 };      /* Constant array of values to be used for Dx, gi is index into constant array  */

static const uint8_t cas_aciflt1_c1[] = { 0x03, 0x0B, 0x08, 0x16 };
static const uint8_t cas_aciflt1_c2[] = { 0xE5, 0xE8, 0xF9, 0xF0 };
static const uint8_t cas_aciflt1_c3[] = { 0x06, 0x16, 0x10, 0x2B };
static const uint8_t cas_aciflt1_c4[] = { 0x2E, 0x0B, 0x04, 0xD8 };
static const uint8_t cas_aciflt1_c5[] = { 0x03, 0x0B, 0x08, 0x16 };

static const uint8_t cas_aciflt2_c1[] = { 0x01, 0x03, 0x14, 0x16 };
static const uint8_t cas_aciflt2_c2[] = { 0xEC, 0xF4, 0xE7, 0xF0 };
static const uint8_t cas_aciflt2_c3[] = { 0x02, 0x06, 0x28, 0x2C };
static const uint8_t cas_aciflt2_c4[] = { 0x30, 0x1E, 0xE5, 0xD8 };
static const uint8_t cas_aciflt2_c5[] = { 0x01, 0x03, 0x14, 0x16 };

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
/** @brief Configure the demodulator from Sleep to Active for DVB-C2 */
static sony_result_t SLtoAC2(sony_demod_t * pDemod);

/** @brief Configure the demodulator for use with a 24MHz Xtal. */
static sony_result_t SLtoAC2_24MHzXtalSetting (sony_demod_t * pDemod);

/** @brief Configure the demodulator for tuner optimisations and bandwidth specific settings. */
static sony_result_t SLtoAC2_BandSetting(sony_demod_t * pDemod);

/** @brief Configure the demodulator from Active DVB-C2 to Active to DVB-C2.  Used for changing channel parameters. */
static sony_result_t AC2toAC2(sony_demod_t * pDemod);

/** @brief Configure the demodulator from Active DVB-C2 to Sleep. */
static sony_result_t AC2toSL(sony_demod_t * pDemod);

/** @brief Read the data slice list from L1 Data */
static sony_result_t GetDataSliceIDList (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq);

/** @brief Read out the notch positions from L1 data */
static sony_result_t GetNotches (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq);

/** @brief Demodulator configuration settings 1 */
static sony_result_t DemodSetting1 (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq);

/** @brief Demodulator configuration settings 2 */
static sony_result_t DemodSetting2 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/** @brief Demodulator configuration settings 3.1 */
static sony_result_t DemodSetting3_1 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/** @brief Demodulator configuration settings 3.2 */
static sony_result_t DemodSetting3_2 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/** @brief Demodulator configuration settings 4 */
static sony_result_t DemodSetting4 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/** @brief Configure the demodulator to the calculated dependent static DS preset values */
static sony_result_t DemodPreset (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/** @brief Default the demodulator preset values */
static sony_result_t ReleasePreset (sony_demod_t * pDemod);

/** @brief Set the center frequency of the demodulator */
static sony_result_t SetDemodCenterFreq (sony_demod_t * pDemod, uint32_t centerFreqkHz);

/** @brief Change 1st trial of GI,TR,spectrum for the next acquisition. */
static sony_result_t Change1stTrial (sony_demod_t * pDemod);

/** @brief Check current lock state, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckGetTuneParamsDemodLock(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq);

/** @brief Check for valid L1 data, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckL1DataReady(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq, uint8_t * pL1DataReady);

/** @brief Select the next data slice, used when reading all PLP's from all data slices */
static sony_result_t SetNextDS(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq);

/** @brief Check for valid preset data, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckPresetReady1_1 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Check for valid preset data, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckPresetReady1_2 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Calculate the center frequency of a dependent static data slice */
static sony_result_t CalculateCenterFreq (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Check for valid preset data, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckPresetReady2 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Calculate the preset data for a dependent static data slice */
static sony_result_t CalculatePresetValue (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Check current lock state, used as part of a wait routine with timer part in integration layer */
static sony_result_t CheckTuneDemodLock(sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Read out the TRL nominal rate */
static sony_result_t GetTRLNominalRate(sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq);

/** @brief Calculate the tuning position in terms of carriers from the tune frequency and bandwidth */
static sony_result_t CalculateCarrierTunePosition(uint32_t *pTunePosition, uint32_t tuningFreqkHz, sony_dtv_bandwidth_t bandwidth);

/** @brief Get the PLP list, and calculate the C2 tuning frequency for the current DS.  Add tune parameters to array. */
static sony_result_t GetTuneParameters (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq);

/** @brief Determine if the current data slice is dependent static or not using notch positions. */
static sony_result_t DetermineDsType (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq, uint32_t dsTunePos, uint8_t * pIsDSDS);

/** @brief Calculate the notch overlap with the current data slice. */
static sony_result_t CalculateAccumulatedOverlap (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq, uint32_t dsTunePos, uint32_t * pAccumulatedOverlap);

/** @brief Calculate the next valid channel frequency based on the current C2 system bandwidth. */
static sony_result_t CalculateNextChannelFrequency (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq);
/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_demod_dvbc2_InitGetTuneParamsSeq (sony_demod_t * pDemod,
                                                     sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq,
                                                     uint32_t channelFrequencyKHz,
                                                     sony_dvbc2_tune_param_t * pTuneParamArray)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_InitGetTuneParamsSeq");

    if ((!pDemod) || (!pGetTuneParamsSeq) || (!pTuneParamArray)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Initialize scanData struct members here */
    pGetTuneParamsSeq->running = 1; /* TRUE */
    pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_1;
    pGetTuneParamsSeq->loopResult = SONY_RESULT_OK;
    pGetTuneParamsSeq->tuneParams = pTuneParamArray;
    pGetTuneParamsSeq->tuneParamIndex = 0;
    pGetTuneParamsSeq->frequencyKHz = channelFrequencyKHz;

    sony_memset(&pGetTuneParamsSeq->ds[0], 0x00, sizeof(pGetTuneParamsSeq->ds));
    pGetTuneParamsSeq->numDs = 0;
    pGetTuneParamsSeq->currentDsIndex = 0;
    sony_memset(&pGetTuneParamsSeq->notch[0], 0x00, sizeof(pGetTuneParamsSeq->notch));
    pGetTuneParamsSeq->numNotch = 0;
    pGetTuneParamsSeq->sf = 0;
    pGetTuneParamsSeq->gi = 0;
    pGetTuneParamsSeq->bnw = 0;
    pGetTuneParamsSeq->dx = 0;
    pGetTuneParamsSeq->nextChannelFrequency = 0;
    pGetTuneParamsSeq->systemTuningFrequency = 0;
    pGetTuneParamsSeq->minAccumulatedOverlap = 0xFFFFFFFF;

    pGetTuneParamsSeq->sleepTime = 0;
    pGetTuneParamsSeq->currentTime = 0;
    pGetTuneParamsSeq->loopStartTime = 0;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_InitTuneSeq (sony_demod_t * pDemod,
                                            sony_dvbc2_tune_param_t * pTuneParams,
                                            sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_InitTuneSeq");

    if ((!pDemod) || (!pTuneParams) || (!pTuneSeq)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Initialize tuneData struct members here */
    pTuneSeq->running = 1; /* TRUE */
    pTuneSeq->tuneRequired = 0; /* FALSE */
    pTuneSeq->rfFrequency = 0;
    pTuneSeq->lockResult = SONY_RESULT_OK;
    pTuneSeq->sleepTime = 0;
    pTuneSeq->currentTime = 0;
    pTuneSeq->loopStartTime = 0;

    pTuneSeq->pTuneData = pTuneParams;
    pTuneSeq->bandwidth = pTuneParams->bandwidth;
    pTuneSeq->tuneParamCenterFreq = pTuneParams->c2TuningFrequencyKHz;

    pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_INITIALIZE;
    pTuneSeq->filterSelect = 0;
    pTuneSeq->casDagcTarget = 0;
    pTuneSeq->crcg2BnFreqOfst = 0;
    sony_memset(&pTuneSeq->trcgNominalRate[0], 0x00, sizeof(pTuneSeq->trcgNominalRate));
    sony_memset(&pTuneSeq->ps[0], 0x00, sizeof(pTuneSeq->ps));
    pTuneSeq->centerFreq = 0;
    pTuneSeq->tunePosDSDS = 0;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_GetTuneParamsSeq (sony_demod_t * pDemod,
                                                 sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_GetTuneParamsSeq");

    if ((!pDemod) || (!pGetTuneParamsSeq)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pGetTuneParamsSeq->state) {
        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_1:
            result = ReleasePreset (pDemod);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = DemodSetting1 (pDemod, pGetTuneParamsSeq);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_2;
            /* Store start time for a looping state to enable timeout detection */
            pGetTuneParamsSeq->loopStartTime = pGetTuneParamsSeq->currentTime;
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_2:
            result = CheckGetTuneParamsDemodLock(pDemod, pGetTuneParamsSeq);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_3:
            result = Change1stTrial (pDemod);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_4;
            /* Store start time for a looping state to enable timeout detection */
            pGetTuneParamsSeq->loopStartTime = pGetTuneParamsSeq->currentTime;
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_4:
            {
                uint8_t l1DataReady = 0;

                /* Note: This function returns with registers frozen */
                result = CheckL1DataReady(pDemod, pGetTuneParamsSeq, &l1DataReady);
                if (result != SONY_RESULT_OK) {
                    SLVT_UnFreezeReg (pDemod);
                    SONY_TRACE_RETURN (result);
                }

                if (l1DataReady) {
                    result = GetDataSliceIDList (pDemod, pGetTuneParamsSeq);
                    if (result != SONY_RESULT_OK) {
                        SLVT_UnFreezeReg (pDemod);
                        SONY_TRACE_RETURN (result);
                    }

                    result = GetNotches (pDemod, pGetTuneParamsSeq);
                    if (result != SONY_RESULT_OK) {
                        SLVT_UnFreezeReg (pDemod);
                        SONY_TRACE_RETURN (result);
                    }

                    result = CalculateNextChannelFrequency (pDemod, pGetTuneParamsSeq);
                    if (result != SONY_RESULT_OK) {
                        SLVT_UnFreezeReg (pDemod);
                        SONY_TRACE_RETURN (result);
                    }

                    pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_5;
                }

                /* Unfreeze registers */
                SLVT_UnFreezeReg (pDemod);
            }
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_5:
            if (pGetTuneParamsSeq->currentDsIndex < pGetTuneParamsSeq->numDs){
                /* Set the next DS and pass on to the next state to read PLP data */
                result = SetNextDS (pDemod, pGetTuneParamsSeq);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

                pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_6;
                /* Store start time for a looping state to enable timeout detection */
                pGetTuneParamsSeq->loopStartTime = pGetTuneParamsSeq->currentTime;
            }
            else{
                /* All tune parameters have been detected for all DS */
                pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_7;
            }

            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_6:
            {
                uint8_t l1DataReady = 0;

                /* Note: This function returns with registers frozen */
                result = CheckL1DataReady(pDemod, pGetTuneParamsSeq, &l1DataReady);
                if (result != SONY_RESULT_OK) {
                    SLVT_UnFreezeReg (pDemod);
                    SONY_TRACE_RETURN (result);
                }

                if (l1DataReady) {
                    result = GetTuneParameters (pDemod, pGetTuneParamsSeq);
                    if (result != SONY_RESULT_OK) {
                        SLVT_UnFreezeReg (pDemod);
                        SONY_TRACE_RETURN (result);
                    }

                    /* we have now read from this DS so increment to the next and loop back */
                    pGetTuneParamsSeq->currentDsIndex++;
                    pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_5;
                }

                /* Unfreeze registers */
                SLVT_UnFreezeReg (pDemod);
            }
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_7:
            {
                uint8_t tuneParam = 0;

                /* Update C2 tuning frequency for dependent static data slices */
                for (tuneParam = 0 ; tuneParam < pGetTuneParamsSeq->tuneParamIndex ; tuneParam++) {
                    if (pGetTuneParamsSeq->tuneParams[tuneParam].isDependentStaticDS) {
                        pGetTuneParamsSeq->tuneParams[tuneParam].c2TuningFrequencyKHz = pGetTuneParamsSeq->systemTuningFrequency;
                    }
                }
            }

            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_FINALIZE;
            break;

        case SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_FINALIZE:
            pGetTuneParamsSeq->running = 0; /* FALSE */
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_TuneSeq (sony_demod_t * pDemod,
                                        sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_TuneSeq");

    if ((!pDemod) || (!pTuneSeq)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pTuneSeq->state) {
    case SONY_DEMOD_DVBC2_TUNE_INITIALIZE:
        /* Enable acquisition on the demodulator. */
        result = sony_demod_dvbc2_Tune (pDemod, pTuneSeq->pTuneData);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* If not in scan mode then release preset */
        if (!pDemod->scanMode) {
            result = ReleasePreset (pDemod);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        }

        if (!pTuneSeq->pTuneData->isDependentStaticDS) {
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_NON_DSDS_STAGE_1;
        }
        else {
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_1;
        }

        /* Set frequency for RF part */
        if (pTuneSeq->pTuneData->isDependentStaticDS) {
            /* Dependent static DS so always use C2 Tuning Freq */
            pTuneSeq->rfFrequency = pTuneSeq->pTuneData->c2TuningFrequencyKHz;
        }
        else {
            /* Normal DS, determine correct frequency to use from params */
            if (pTuneSeq->pTuneData->rfTuningFrequencyKHz == 0) {
                /* No RF tuning frequency provided so tune RF part to C2 tuning frequency */
                pTuneSeq->rfFrequency = pTuneSeq->pTuneData->c2TuningFrequencyKHz;
            }
            else {
                /* Tune RF part to provided tuning frequency */
                pTuneSeq->rfFrequency = pTuneSeq->pTuneData->rfTuningFrequencyKHz;
            }
        }

        pTuneSeq->tuneRequired = 1; /* TRUE */
        break;

    case SONY_DEMOD_DVBC2_TUNE_NON_DSDS_STAGE_1:
        result = DemodSetting2 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_TuneEnd (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_WAIT_LOCK;
        /* Store start time for a looping state to enable timeout detection */
        pTuneSeq->loopStartTime = pTuneSeq->currentTime;
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_1:
        result = DemodSetting3_1 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_2;
        /* Store start time for a looping state to enable timeout detection */
        pTuneSeq->loopStartTime = pTuneSeq->currentTime;
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_2:
        result = CheckPresetReady1_1 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_3:
        result = DemodSetting3_2 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_4;
        /* Store start time for a looping state to enable timeout detection */
        pTuneSeq->loopStartTime = pTuneSeq->currentTime;
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_4:
        result = CheckPresetReady1_2 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_5:
        result = Change1stTrial (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = CalculateCenterFreq (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = DemodSetting4 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = GetTRLNominalRate (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_6;
        /* Store start time for a looping state to enable timeout detection */
        pTuneSeq->loopStartTime = pTuneSeq->currentTime;
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_6:
        result = CheckPresetReady2 (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_7:
        result = Change1stTrial (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = CalculatePresetValue (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        /* Set frequency for RF part */
        if (pTuneSeq->pTuneData->rfTuningFrequencyKHz == 0) {
            /* No tuning frequency provided so tune RF part to calculated C2 tuning frequency DSDS */
            /* C2TuningFrequencyDSDS (MHz) = floor(TunePosDSDS / Tu(us)) */
            if (pTuneSeq->bandwidth == SONY_DTV_BW_6_MHZ) {
                /* Tu(us) = 4096 * (7 / 48) = 1792 / 3 */
                pTuneSeq->rfFrequency = (pTuneSeq->tunePosDSDS * 1000 * 3) / 1792;
            }
            else if (pTuneSeq->bandwidth == SONY_DTV_BW_8_MHZ) {
                /* Tu(us) = 4096 * (7 / 64) = 448 */
                pTuneSeq->rfFrequency = (pTuneSeq->tunePosDSDS * 1000) / 448;
            }
            else {
                SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
            }
        }
        else {
            /* Tune RF part to provided tuning frequency */
            pTuneSeq->rfFrequency = pTuneSeq->pTuneData->rfTuningFrequencyKHz;
        }

        pTuneSeq->tuneRequired = 1; /* TRUE */
        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_8;
        break;

    case SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_8:
        result = DemodPreset (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        result = sony_demod_TuneEnd (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_WAIT_LOCK;
        /* Store start time for a looping state to enable timeout detection */
        pTuneSeq->loopStartTime = pTuneSeq->currentTime;
        break;

    case SONY_DEMOD_DVBC2_TUNE_WAIT_LOCK:
        result = CheckTuneDemodLock (pDemod, pTuneSeq);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
        break;

    case SONY_DEMOD_DVBC2_TUNE_CHANGE_FIRST_TRIAL:
        result = Change1stTrial (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
        break;

    case SONY_DEMOD_DVBC2_TUNE_FINALIZE:
        pTuneSeq->tuneRequired = 0; /* FALSE */
        pTuneSeq->running = 0; /* FALSE */
        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_Tune (sony_demod_t * pDemod, sony_dvbc2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_Tune");

    if ((!pDemod) || (!pTuneParam)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state == SONY_DEMOD_STATE_ACTIVE) && (pDemod->system == SONY_DTV_SYSTEM_DVBC2)) {
        /* Demodulator Active and set to DVB-C2 mode */
        pDemod->bandwidth = pTuneParam->bandwidth;

        result = AC2toAC2 (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
    else if((pDemod->state == SONY_DEMOD_STATE_ACTIVE) && (pDemod->system != SONY_DTV_SYSTEM_DVBC2)){
        /* Demodulator Active but not DVB-C2 mode */
        result = sony_demod_Sleep (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        pDemod->system = SONY_DTV_SYSTEM_DVBC2;
        pDemod->bandwidth = pTuneParam->bandwidth;

        result = SLtoAC2 (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
    else if (pDemod->state == SONY_DEMOD_STATE_SLEEP) {
        /* Demodulator in Sleep mode */
        pDemod->system = SONY_DTV_SYSTEM_DVBC2;
        pDemod->bandwidth = pTuneParam->bandwidth;

        result = SLtoAC2 (pDemod);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }
    else {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Update demodulator state */
    pDemod->state = SONY_DEMOD_STATE_ACTIVE;

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_dvbc2_Sleep (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_dvbc2_Sleep");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if ((pDemod->state != SONY_DEMOD_STATE_SLEEP) && (pDemod->state != SONY_DEMOD_STATE_ACTIVE)) {
        /* This api is accepted in Sleep and Active states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    result = AC2toSL (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_dvbc2_CheckDemodLock (sony_demod_t * pDemod,
                                               sony_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_CheckDemodLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    *pLock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &sync, &tslock, &unlockDetected);

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (unlockDetected) {
        *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    if (sync >= 6) {
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCKED;
    }

    SONY_TRACE_RETURN (result);
}


sony_result_t sony_demod_dvbc2_CheckTSLock (sony_demod_t * pDemod,
                                            sony_demod_lock_result_t * pLock)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t sync = 0;
    uint8_t tslock = 0;
    uint8_t unlockDetected = 0;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_CheckTSLock");

    if ((!pDemod) || (!pLock)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    *pLock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;

    result = sony_demod_dvbc2_monitor_SyncStat (pDemod, &sync, &tslock, &unlockDetected);

    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (unlockDetected) {
        *pLock = SONY_DEMOD_LOCK_RESULT_UNLOCKED;
        SONY_TRACE_RETURN (SONY_RESULT_OK);
    }

    if ((sync >= 6) && tslock) {
        *pLock = SONY_DEMOD_LOCK_RESULT_LOCKED;
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_demod_dvbc2_SetPlpId (sony_demod_t * pDemod, uint8_t plpId)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_demod_dvbc2_SetPlpId");

    if (!pDemod) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE) {
        /* This api is accepted in Active state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* OREGD_FP_TGT_PLP_ID = p[k][m]
     * OREGD_L1C2_PRESET_CLEAR = H
     */

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit      Default     Name
     * ----------------------------------------------------------------------
     * <SLV-T>   50h     7Fh    [7:0]    8'h00       OREGD_FP_TGT_PLP_ID[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7F, plpId) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit      Default     Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     87h    [0]       8'h00      1'h01      OREGD_L1C2_PRESET_CLEAR
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t SLtoAC2(sony_demod_t * pDemod)
{
sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("SLtoAC2");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->dvbc2TLVOutput) {
        /* Configure TLV clock Mode and Frequency */
        result = sony_demod_SetTLVClockModeAndFreq (pDemod, SONY_DTV_SYSTEM_DVBC2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    } else
#endif
    {
        /* Configure TS clock Mode and Frequency */
        result = sony_demod_SetTSClockModeAndFreq (pDemod, SONY_DTV_SYSTEM_DVBC2);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
    }

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set demod mode */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x17, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set TS/TLV mode */
    {
        uint8_t data = 0;

#ifdef SONY_DEMOD_SUPPORT_TLV
        if (pDemod->dvbc2TLVOutput) {
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

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, 0x74) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x49, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x11 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x11) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* BBAGC TARGET level setting
     * Slave    Bank    Addr    Bit    default     Value          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   11h     6Ah    [7:0]     8'h50      8'h50      OREG_ITB_DAGC_TRGT[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x10 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_SONYSILICON) {
        /* ASCOT setting ON
         * Slave    Bank    Addr    Bit    default     Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   10h     A5h     [0]      8'h01      8'h01      OREG_ITB_GDEQ_EN
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA5, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }
    else {
        /* ASCOT setting OFF
         * Slave    Bank    Addr    Bit    default     Value          Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   10h     A5h     [0]      8'h01      8'h00      OREG_ITB_GDEQ_EN
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA5, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TSIF setting
     * slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   00h     CEh     [0]      8'h01      8'h01      ONOPARITY
     * <SLV-T>   00h     CFh     [0]      8'h01      8'h01      ONOPARITY_MANUAL_ON
     */
    {
        const uint8_t data[] = { 0x01, 0x01 };

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xCE, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   20h     C2h    [7:0]     8'h11      8'h00      2'b00,OREG_CAS_ACIFLT1_EN[1:0],2'b00,OREG_CAS_ACIFLT2_EN[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC2, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x25 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x25) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     6Ah    [1:0]     8'h02      8'h00      OREG_SP_ISCR_CNT_CORRECT_MODE[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     89h    [3:0]     8'h07      8'h0C      OREG_SP_BICM_DELAY_NOISSY[19:16]
     * <SLV-T>   25h     8Ah    [7:0]     8'h61      8'hD1      OREG_SP_BICM_DELAY_NOISSY[15:8]
     * <SLV-T>   25h     8Bh    [7:0]     8'h36      8'h40      OREG_SP_BICM_DELAY_NOISSY[7:0]
     */
    {
        static const uint8_t data[3] = { 0x0C, 0xD1, 0x40 };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x89, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     CBh    [2:0]     8'h05      8'h01      OREG_SP_RC_PLP_INIT,OREG_SP_RC_PLP,OREG_SP_RC_ISCR_INTERVAL_AUTO
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xCB, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     DCh    [7:0]     8'h2E      8'h7B      OREG_SP_BUFS_OFFSET_C[15:8]
     * <SLV-T>   25h     DDh    [7:0]     8'hE0      8'h00      OREG_SP_BUFS_OFFSET_C[7:0]
     * <SLV-T>   25h     DEh    [7:0]     8'h2E      8'h7B      OREG_SP_BUFS_OFFSET_D[15:8]
     * <SLV-T>   25h     DFh    [7:0]     8'hE0      8'h00      OREG_SP_BUFS_OFFSET_D[7:0]
     */
    {
        static const uint8_t data[4] = { 0x7B, 0x00, 0x7B, 0x00 };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xDC, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     E2h    [7:0]     8'h2F      8'h30      OREG_SP_RAM_STORED_COMMON[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE2, 0x30) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     E5h    [7:0]     8'h2F      8'h30      OREG_SP_RAM_STORED_DATA[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE5, 0x30) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x27 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     20h    [0]       8'h00      8'h01      OREG_FFT_FORCESCALE
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     35h    [0]       8'h00      8'h01      OREG_DFT_FORCE_SCALE
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     D9h    [5:0]     8'h19      8'h18      OREG_FCS_NTHETA[5:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD9, 0x18) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2A */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2A) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     38h    [3:0]     8'h04      8'h00      OREG_CAS_CCIFLT_EN_CW[1:0],OREG_CAS_CCIFLT_EN_CW2[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x38, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     78h    [2:0]     8'h01      8'h00      OREG_CRCG2_CCOP2_TYPE[2:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x78, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     86h    [7:0]     8'h08      8'h20      OREG_CSI_NP_LOWERBOUND2[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x86, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     88h    [7:0]     8'h14      8'h32      OREG_CSI_NP_BORDER[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x88, 0x32) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2B */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2B) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Bh     11h    [5:0]     8'h20      8'h00      OREG_CAS_CWFLT_ACTCOND[2:0],OREG_CAS_CW2FLT_ACTCOND[2:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Bh     2Bh    [4:0]     8'h00      8'h10      OREG_FITP_FORCE_FLT_SEL,2'b00,OREG_FITP_FLT_SEL[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2B, 0x10) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2D */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2D) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Dh     24h    [7:0]     8'h89      8'h01      OREG_LDPCD_PLP_MIN_OUTTIME_N[7:0]
     * <SLV-T>   2Dh     25h    [7:0]     8'h89      8'h01      OREG_LDPCD_PLP_MIN_OUTTIME_S[7:0]
     */
    {
        static const uint8_t data[2] = { 0x01, 0x01 };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0x11 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x11) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        /* DVB-C2 24MHz Xtal setting
         * Slave    Bank    Addr    Bit       Default    Value      Name
         * ---------------------------------------------------------------------------------
         * <SLV-T>   11h     33h    [7:0]     8'hC8      8'h00      OCTL_IFAGC_INITWAIT[7:0]
         * <SLV-T>   11h     34h    [7:0]     8'h02      8'h03      OCTL_IFAGC_MINWAIT[7:0]
         * <SLV-T>   11h     35h    [7:0]     8'h32      8'h3B      OCTL_IFAGC_MAXWAIT[7:0]
         */
        const uint8_t data[3] = {0x00, 0x03, 0x3B};
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x33, &data[0], sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set tuner and bandwidth specific settings */
    result = SLtoAC2_BandSetting (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable HiZ Setting 1 (TAGC, SAGC(Hi-Z), TSVALID, TSSYNC, TSCLK) */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80, 0x08, 0x1F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Disable HiZ Setting 2 */
    result = sony_demod_SetTSDataPinHiZ (pDemod, 0);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t SLtoAC2_BandSetting(sony_demod_t * pDemod)
{
    SONY_TRACE_ENTER ("SLtoAC2_BandSetting");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    switch (pDemod->bandwidth) {
    case SONY_DTV_BW_8_MHZ:
        {
            const uint8_t nominalRate[5] = {
                /* TRCG Nominal Rate [37:0] */
                0x15, 0x00, 0x00, 0x00, 0x00
            };
            /* <Timing Recovery setting>
             * Slave     Bank    Addr   Bit     Default   Value   Name
             * -------------------------------------------------------------------------------
             * <SLV-T>   20h     9Fh    [5:0]   8'h15     nomi1   OREG_TRCG_NOMINALRATE[37:32]
             * <SLV-T>   20h     A0h    [7:0]   8'h00     nomi2   OREG_TRCG_NOMINALRATE[31:24]
             * <SLV-T>   20h     A1h    [7:0]   8'h00     nomi3   OREG_TRCG_NOMINALRATE[23:16]
             * <SLV-T>   20h     A2h    [7:0]   8'h00     nomi4   OREG_TRCG_NOMINALRATE[15:8]
             * <SLV-T>   20h     A3h    [7:0]   8'h00     nomi5   OREG_TRCG_NOMINALRATE[7:0]
             * <SLV-T>   27h     7Ah    [3:0]   8'h00     8'h00   OREG_TRCG_LMTVAL0[3:0]
             */

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, nominalRate, 5) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Set SLV-T Bank : 0x27 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* Set SLV-T Bank : 0x10 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * Filter settings for Sony silicon tuners
         * slave    bank    addr     bit      default    name
         * ------------------------------------------------------------------
         * <SLV-T>   10h     A6h    [7:0]     8'h1E      OREG_ITB_COEF01[7:0]
         * <SLV-T>   10h     A7h    [7:0]     8'h1D      OREG_ITB_COEF02[7:0]
         * <SLV-T>   10h     A8h    [7:0]     8'h29      OREG_ITB_COEF11[7:0]
         * <SLV-T>   10h     A9h    [7:0]     8'hC9      OREG_ITB_COEF12[7:0]
         * <SLV-T>   10h     AAh    [7:0]     8'h2A      OREG_ITB_COEF21[7:0]
         * <SLV-T>   10h     ABh    [7:0]     8'hBA      OREG_ITB_COEF22[7:0]
         * <SLV-T>   10h     ACh    [7:0]     8'h29      OREG_ITB_COEF31[7:0]
         * <SLV-T>   10h     ADh    [7:0]     8'hAD      OREG_ITB_COEF32[7:0]
         * <SLV-T>   10h     AEh    [7:0]     8'h29      OREG_ITB_COEF41[7:0]
         * <SLV-T>   10h     AFh    [7:0]     8'hA4      OREG_ITB_COEF42[7:0]
         * <SLV-T>   10h     B0h    [7:0]     8'h29      OREG_ITB_COEF51[7:0]
         * <SLV-T>   10h     B1h    [7:0]     8'h9A      OREG_ITB_COEF52[7:0]
         * <SLV-T>   10h     B2h    [7:0]     8'h28      OREG_ITB_COEF61[7:0]
         * <SLV-T>   10h     B3h    [7:0]     8'h9E      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_SONYSILICON) {
            const uint8_t itbCoef[14] = {
            /*  COEF01 COEF02 COEF11 COEF12 COEF21 COEF22 COEF31 COEF32 COEF41 COEF42 COEF51 COEF52 COEF61 COEF62 */
                0x2F,  0xBA,  0x28,  0x9B,  0x28,  0x9D,  0x28,  0xA1,  0x29,  0xA5,  0x2A,  0xAC,  0x29,  0xB5
            };

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA6, itbCoef, 14) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        {
            /*
             * <IF freq setting>
             * Slave    bank    addr    bit     default     value           Name
             * ---------------------------------------------------------------------------------
             * <SLV-T>   10h     B6h    [7:0]     8'h1F     user defined    OREG_DNCNV_LOFRQ_T[23:16]
             * <SLV-T>   10h     B7h    [7:0]     8'h38     user defined    OREG_DNCNV_LOFRQ_T[15:8]
             * <SLV-T>   10h     B8h    [7:0]     8'h32     user defined    OREG_DNCNV_LOFRQ_T[7:0]
             */
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreqConfig.configDVBC2_8 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreqConfig.configDVBC2_8 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreqConfig.configDVBC2_8 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* System bandwidth setting
         * Slave    Bank    Addr    Bit      default    Value      Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   10h     D7h    [2:0]    8'h00      8'h00      OREG_CHANNEL_WIDTH[2:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set SLV-T Bank : 0x50 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        {
            const uint8_t data[] = { 0x14, 0xA0 };

            /* Slave    Bank    Addr    Bit       Default    Name
             * -------------------------------------------------------------------------------
             * <SLV-T>   50h     ECh    [7:0]     8'h11      OREG_L1SEQC2_TIMEOUT_CYCLE0[15:8]
             * <SLV-T>   50h     EDh    [7:0]     8'h9E      OREG_L1SEQC2_TIMEOUT_CYCLE0[7:0]
             * <SLV-T>   50h     EFh    [7:0]     8'h11      OREG_L1SEQC2_TIMEOUT_CYCLE1[15:8]
             * <SLV-T>   50h     F1h    [7:0]     8'h9E      OREG_L1SEQC2_TIMEOUT_CYCLE1[7:0]
             */
            if (pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xEC, &data[0], 2) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xEF, data[0]) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF1, data[1]) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }
        break;

    case SONY_DTV_BW_6_MHZ:
        {
            const uint8_t nominalRate[5] = {
                /* TRCG Nominal Rate [37:0] */
                0x1C, 0x00, 0x00, 0x00, 0x00
            };
            /* <Timing Recovery setting>
             * Slave     Bank    Addr   Bit     Default   Value   Name
             * -----------------------------------------------------------------------
             * <SLV-T>   20h     9Fh    [5:0]   8'h15     nomi1   OREG_TRCG_NOMINALRATE[37:32]
             * <SLV-T>   20h     A0h    [7:0]   8'h00     nomi2   OREG_TRCG_NOMINALRATE[31:24]
             * <SLV-T>   20h     A1h    [7:0]   8'h00     nomi3   OREG_TRCG_NOMINALRATE[23:16]
             * <SLV-T>   20h     A2h    [7:0]   8'h00     nomi4   OREG_TRCG_NOMINALRATE[15:8]
             * <SLV-T>   20h     A3h    [7:0]   8'h00     nomi5   OREG_TRCG_NOMINALRATE[7:0]
             * <SLV-T>   27h     7Ah    [3:0]   8'h00     8'h00   OREG_TRCG_LMTVAL0[3:0]
             */

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, nominalRate, 5) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Set SLV-T Bank : 0x27 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, 0x00) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* Set SLV-T Bank : 0x10 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x10) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /*
         * Filter settings for Sony silicon tuners
         * slave    bank    addr     bit      default    name
         * ------------------------------------------------------------------
         * <SLV-T>   10h     A6h    [7:0]     8'h1E      OREG_ITB_COEF01[7:0]
         * <SLV-T>   10h     A7h    [7:0]     8'h1D      OREG_ITB_COEF02[7:0]
         * <SLV-T>   10h     A8h    [7:0]     8'h29      OREG_ITB_COEF11[7:0]
         * <SLV-T>   10h     A9h    [7:0]     8'hC9      OREG_ITB_COEF12[7:0]
         * <SLV-T>   10h     AAh    [7:0]     8'h2A      OREG_ITB_COEF21[7:0]
         * <SLV-T>   10h     ABh    [7:0]     8'hBA      OREG_ITB_COEF22[7:0]
         * <SLV-T>   10h     ACh    [7:0]     8'h29      OREG_ITB_COEF31[7:0]
         * <SLV-T>   10h     ADh    [7:0]     8'hAD      OREG_ITB_COEF32[7:0]
         * <SLV-T>   10h     AEh    [7:0]     8'h29      OREG_ITB_COEF41[7:0]
         * <SLV-T>   10h     AFh    [7:0]     8'hA4      OREG_ITB_COEF42[7:0]
         * <SLV-T>   10h     B0h    [7:0]     8'h29      OREG_ITB_COEF51[7:0]
         * <SLV-T>   10h     B1h    [7:0]     8'h9A      OREG_ITB_COEF52[7:0]
         * <SLV-T>   10h     B2h    [7:0]     8'h28      OREG_ITB_COEF61[7:0]
         * <SLV-T>   10h     B3h    [7:0]     8'h9E      OREG_ITB_COEF62[7:0]
         */
        if (pDemod->tunerOptimize == SONY_DEMOD_TUNER_OPTIMIZE_SONYSILICON) {
            const uint8_t itbCoef[14] = {
            /*  COEF01 COEF02 COEF11 COEF12 COEF21 COEF22 COEF31 COEF32 COEF41 COEF42 COEF51 COEF52 COEF61 COEF62 */
                0x31,  0xA8,  0x29,  0x9B,  0x27,  0x9C,  0x28,  0x9E,  0x29,  0xA4,  0x29,  0xA2,  0x29,  0xA8
            };

            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xA6, itbCoef, 14) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        {
            /*
             * <IF freq setting>
             * Slave    bank    addr    bit     default     value           Name
             * ---------------------------------------------------------------------------------
             * <SLV-T>   10h     B6h    [7:0]     8'h1F     user defined    OREG_DNCNV_LOFRQ_T[23:16]
             * <SLV-T>   10h     B7h    [7:0]     8'h38     user defined    OREG_DNCNV_LOFRQ_T[15:8]
             * <SLV-T>   10h     B8h    [7:0]     8'h32     user defined    OREG_DNCNV_LOFRQ_T[7:0]
             */
            uint8_t data[3];
            data[0] = (uint8_t) ((pDemod->iffreqConfig.configDVBC2_6 >> 16) & 0xFF);
            data[1] = (uint8_t) ((pDemod->iffreqConfig.configDVBC2_6 >> 8) & 0xFF);
            data[2] = (uint8_t) (pDemod->iffreqConfig.configDVBC2_6 & 0xFF);
            if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xB6, data, sizeof (data)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        /* System bandwidth setting
         * Slave    Bank    Addr    Bit      default    Value      Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   10h     D7h    [2:0]    8'h00      8'h04      OREG_CHANNEL_WIDTH[2:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD7, 0x04) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Set SLV-T Bank : 0x50 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        {
            const uint8_t data[] = { 0x1B, 0x70 };

            /* Slave    Bank    Addr    Bit       Default    Name
             * -------------------------------------------------------------------------------
             * <SLV-T>   50h     ECh    [7:0]     8'h11      OREG_L1SEQC2_TIMEOUT_CYCLE0[15:8]
             * <SLV-T>   50h     EDh    [7:0]     8'h9E      OREG_L1SEQC2_TIMEOUT_CYCLE0[7:0]
             * <SLV-T>   50h     EFh    [7:0]     8'h11      OREG_L1SEQC2_TIMEOUT_CYCLE1[15:8]
             * <SLV-T>   50h     F1h    [7:0]     8'h9E      OREG_L1SEQC2_TIMEOUT_CYCLE1[7:0]
             */
            if (pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xEC, &data[0], 2) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xEF, data[0]) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            if (pDemod->pI2c->WriteOneRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF1, data[1]) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }
        }

        break;

    default:
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t AC2toAC2(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("AC2toAC2");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->dvbc2TLVOutput) {
        /* Set SLV-T Bank : 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TLV output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC0, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else
#endif
    {
        /* Set SLV-T Bank : 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TS output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set tuner and bandwidth specific settings */
    result = SLtoAC2_BandSetting (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t AC2toSL(sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("AC2toSL");

    if (!pDemod) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

#ifdef SONY_DEMOD_SUPPORT_TLV
    if (pDemod->dvbc2TLVOutput) {
        /* Set SLV-T Bank : 0x01 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TLV output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC0, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    } else
#endif
    {
        /* Set SLV-T Bank : 0x00 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Disable TS output */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC3, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
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

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   20h     C2h    [7:0]     8'h11      8'h11      2'b00,OREG_CAS_ACIFLT1_EN[1:0],2'b00,OREG_CAS_ACIFLT2_EN[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC2, 0x11) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x25 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x25) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     6Ah    [1:0]     8'h02      8'h02      OREG_SP_ISCR_CNT_CORRECT_MODE[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6A, 0x02) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     89h    [3:0]     8'h07      8'h07      OREG_SP_BICM_DELAY_NOISSY[19:16]
     * <SLV-T>   25h     8Ah    [7:0]     8'h61      8'h61      OREG_SP_BICM_DELAY_NOISSY[15:8]
     * <SLV-T>   25h     8Bh    [7:0]     8'h36      8'h36      OREG_SP_BICM_DELAY_NOISSY[7:0]
     */
    {
        static const uint8_t data[3] = { 0x07, 0x61, 0x36 };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x89, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     CBh    [2:0]     8'h05      8'h05      OREG_SP_RC_PLP_INIT,OREG_SP_RC_PLP,OREG_SP_RC_ISCR_INTERVAL_AUTO
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xCB, 0x05) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     DCh    [7:0]     8'h2E      8'h2E      OREG_SP_BUFS_OFFSET_C[15:8]
     * <SLV-T>   25h     DDh    [7:0]     8'hE0      8'hE0      OREG_SP_BUFS_OFFSET_C[7:0]
     * <SLV-T>   25h     DEh    [7:0]     8'h2E      8'h2E      OREG_SP_BUFS_OFFSET_D[15:8]
     * <SLV-T>   25h     DFh    [7:0]     8'hE0      8'hE0      OREG_SP_BUFS_OFFSET_D[7:0]
     */
    {
        static const uint8_t data[4] = { 0x2E, 0xE0, 0x2E, 0xE0 };
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xDC, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     E2h    [7:0]     8'h2F      8'h2F      OREG_SP_RAM_STORED_COMMON[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE2, 0x2F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   25h     E5h    [7:0]     8'h2F      8'h2F      OREG_SP_RAM_STORED_DATA[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE5, 0x2F) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x27 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     20h    [0]       8'h00      8'h00      OREG_FFT_FORCESCALE
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     35h    [0]       8'h00      8'h00      OREG_DFT_FORCE_SCALE
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x35, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   27h     D9h    [5:0]     8'h19      8'h19      OREG_FCS_NTHETA[5:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xD9, 0x19) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2A */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2A) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     38h    [3:0]     8'h04      8'h04      OREG_CAS_CCIFLT_EN_CW[1:0],OREG_CAS_CCIFLT_EN_CW2[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x38, 0x04) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     78h    [2:0]     8'h01      8'h01      OREG_CRCG2_CCOP2_TYPE[2:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x78, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     86h    [7:0]     8'h08      8'h08      OREG_CSI_NP_LOWERBOUND2[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x86, 0x08) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Ah     88h    [7:0]     8'h14      8'h14      OREG_CSI_NP_BORDER[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x88, 0x14) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2B */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2B) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Bh     11h    [5:0]     8'h20      8'h20      OREG_CAS_CWFLT_ACTCOND[2:0],OREG_CAS_CW2FLT_ACTCOND[2:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x11, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit     default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   2Bh     2Bh    [4:0]     8'h00      8'h00      OREG_FITP_FORCE_FLT_SEL,2'b00,OREG_FITP_FLT_SEL[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2B, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x2D */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2D) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    {
        const uint8_t data[] = { 0x89, 0x89 };

        /* Slave     Bank    Addr   Bit     default    Name
         * ----------------------------------------------------------------------------------
         * <SLV-T>   2Dh     24h    [7:0]     8'h89    OREG_LDPCD_PLP_MIN_OUTTIME_N[7:0]
         * <SLV-T>   2Dh     25h    [7:0]     8'h89    OREG_LDPCD_PLP_MIN_OUTTIME_S[7:0]
         */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* Need to release preset setting if dependent static DS is tuned before */
    result = ReleasePreset (pDemod);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-X Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x18, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x00 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x49, 0x33) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* TADC setting */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4B, 0x21) != SONY_RESULT_OK) {
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

    /* Set demodulator mode to default */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVX, 0x17, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t GetDataSliceIDList (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("GetDataSliceIDList");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* sf = IREG_START_FREQUENCY
     * gi = IREG_GUARD_INTERVAL
     * numDs = IREG_NUM_DSLICE
     * d[k] = IL1POST_PLP_IDk ,where k=0,1,..,numDs-1
     */

    /* NOTE: Registers are currently frozen from CheckL1DataReady */

    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* sf = IREG_START_FREQUENCY */
    {
        uint8_t rdata[3];

        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     24h     [7:0]        IREG_START_FREQUENCY[23:16]
         * <SLV-T>   52h     25h     [7:0]        IREG_START_FREQUENCY[15:8]
         * <SLV-T>   52h     26h     [7:0]        IREG_START_FREQUENCY[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        pGetTuneParamsSeq->sf = (((uint32_t) rdata[0] << 16) | (rdata[1] << 8) | (rdata[2]));
    }

    /* gi = IREG_GUARD_INTERVAL */
    {
        uint8_t rdata;

        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     29h     [1:0]        IREG_GUARD_INTERVAL[1:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x29, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        pGetTuneParamsSeq->gi = (rdata & 0x03);
        /* Now that we have the guard interval deduce other related values */
        pGetTuneParamsSeq->bnw = BNW_GI[pGetTuneParamsSeq->gi];
        pGetTuneParamsSeq->dx = DX_GI[pGetTuneParamsSeq->gi];
    }

    /* numDs = IREG_NUM_DSLICE */
    {
        uint8_t rdata;

        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     2Dh     [7:0]        IREG_NUM_DSLICE[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2D, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        pGetTuneParamsSeq->numDs = rdata;
    }

    /* d[k] = IL1POST_PLP_IDk ,where k=0,1,..,numDs-1 */
    {
        uint8_t i = 0;
        uint8_t i_max = 0;
        uint8_t rdata;

        /* Set SLV-T Bank : 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x22) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* 256 registers are split across two banks */
        if (pGetTuneParamsSeq->numDs > 128) {
            i_max = 128;
        }
        else {
            i_max = pGetTuneParamsSeq->numDs;
        }

        /* Get all DataSlice IDs uptil a maximum of 128 */
        for (i = 0; i < i_max; i++) {
            /* Slave     Bank    Addr         Bit        Name
             * ----------------------------------------------------------------------------------
             * <SLV-T>   22h     (80h + i)    [7:0]      IL1POST_PLP_IDx[7:0]     TS PLP_ID #1+i
             */
            if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80 + i, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            pGetTuneParamsSeq->ds[i] = rdata;
        }

        /* confirm if 2nd bank needs writing to  */
        if (pGetTuneParamsSeq->numDs > 128) {
            i_max = pGetTuneParamsSeq->numDs - 128;

            /* Set SLV-T Bank : 0x23 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x23) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Get remaining DataSlice IDs */
            for (i = 0; i < i_max; i++) {
                /* Slave     Bank    Addr         Bit        Name
                 * ----------------------------------------------------------------------------------
                 * <SLV-T>   23h     (10h + i)    [7:0]      IL1POST_PLP_IDx[7:0]     TS PLP_ID #129+i
                 */
                if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10 + i, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }

                /* validate index bounds */
                if ((128 + i) < SONY_DVBC2_DS_MAX) {
                    pGetTuneParamsSeq->ds[128 + i] = rdata;
                }
                else {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_RANGE);
                }
            }
        }
    }
    SONY_TRACE_RETURN (result);
}

static sony_result_t GetTuneParameters (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t plpIds[256];
    uint8_t numPlps = 0;
    uint32_t dsTunePos = 0;
    uint8_t isDSDS = 0;
    uint32_t accumulatedOverlap = 0;
    uint32_t c2TuningFrequencyKHz = 0;
    uint8_t plp = 0;

    SONY_TRACE_ENTER ("GetTuneParameters");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * tp[k] =   IREG_DSLICE_TUNE_POS
     * numPlp[k] = INUM_DATA_PLP_TS
     * p[k][m] = IL1POST_PLP_IDm,where m=0,1,..,numPlp[k]-1
     */

    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* tp[k] =  IREG_DSLICE_TUNE_POS */
    {
        uint8_t rdata[2];

        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     30h     [5:0]        IREG_DSLICE_TUNE_POS[13:8]
         * <SLV-T>   52h     31h     [7:0]        IREG_DSLICE_TUNE_POS[7:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        dsTunePos = (((rdata[0] & 0x3F) << 8) | rdata[1]);
    }

    /* numPlp[k] = INUM_DATA_PLP_TS */
    {
        /* Set SLV-T Bank : 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x22) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr    Bit        Name
         * ----------------------------------------------------------------------------------------
         * <SLV-T>   22h     7Fh     [7:0]      INUM_DATA_PLP_TS[7:0]    number of TS PLP (max 255)
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7F, &numPlps, sizeof (numPlps)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* p[k][m] = IL1POST_PLP_IDm,where m=0,1,..,numPlp[k]-1 */
    {
        uint8_t i = 0;
        uint8_t i_max = 0;
        uint8_t rdata;

        /* Set SLV-T Bank : 0x22 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x22) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* 256 registers are split across two banks */
        if (numPlps > 128) {
            i_max = 128;
        }
        else {
            i_max = numPlps;
        }

        /* Get all PLP IDs for this DataSlice up to a maximum of 128 */
        for (i = 0; i < i_max; i++) {
            /* Slave     Bank    Addr         Bit        Name
             * ----------------------------------------------------------------------------------------
             * <SLV-T>   22h     (80h + i)    [7:0]      IL1POST_PLP_IDx[7:0]     TS PLP_ID #1+i
             */
            if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x80 + i, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            plpIds[i] = rdata;
        }

        /* confirm if 2nd bank needs writing to  */
        if (numPlps > 128) {
            i_max = numPlps - 128;

            /* Set SLV-T Bank : 0x23 */
            if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x23) != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
            }

            /* Get remaining PLP IDs */
            for (i = 0; i < i_max; i++) {
                /* Slave     Bank    Addr         Bit        Name
                 * ----------------------------------------------------------------------------------------
                 * <SLV-T>   23h     (10h + i)    [7:0]      IL1POST_PLP_IDx[7:0]     TS PLP_ID #129+i
                 */
                if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x10 + i, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }

                plpIds[128 + i] = rdata;
            }
        }
    }

    /* Determine if data slice is dependent static or not */
    result = DetermineDsType (pDemod, pGetTuneParamsSeq, dsTunePos, &isDSDS);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Calculate C2 system tune frequency
     * C2_system_tuning_frequency = floor( ( sf + dsTunePos * Dx ) / Tu )
     */
    if (pDemod->bandwidth == SONY_DTV_BW_6_MHZ) {
        /* TU 6MHz = ((4096 / 48) * 7) = 1792 / 3 therefore x / TU = x * 3 / 1792 */
        c2TuningFrequencyKHz =  (pGetTuneParamsSeq->sf + (dsTunePos * pGetTuneParamsSeq->dx)) * 1000 * 3 / 1792;
    }
    else {
        /* TU 8MHz = ((4096 / 64) * 7 = 448 */
        c2TuningFrequencyKHz =   (pGetTuneParamsSeq->sf + (dsTunePos * pGetTuneParamsSeq->dx)) * 1000 / 448;
    }

    /* Calculate notch overlap with current data slice */
    result = CalculateAccumulatedOverlap (pDemod, pGetTuneParamsSeq, dsTunePos, &accumulatedOverlap);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* stf = tp[ argmink(accumulated overlap) ] */
    if (accumulatedOverlap < pGetTuneParamsSeq->minAccumulatedOverlap) {
        pGetTuneParamsSeq->minAccumulatedOverlap = accumulatedOverlap;
        pGetTuneParamsSeq->systemTuningFrequency = c2TuningFrequencyKHz;
    }

    /* Add tune params for each PLP in current DS */
    for (plp = 0 ; plp < numPlps ; plp++) {
        /* For DSDS cases the c2TuningFrequencyKHz parameter will be updated to the System
         * Tuning Frequency of the data slice with minimum notch overlap. */
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].c2TuningFrequencyKHz = c2TuningFrequencyKHz;
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].bandwidth = pDemod->bandwidth;
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].dataSliceId = pGetTuneParamsSeq->ds[pGetTuneParamsSeq->currentDsIndex];
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].dataPLPId = plpIds[plp];
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].isDependentStaticDS = isDSDS;
        pGetTuneParamsSeq->tuneParams[pGetTuneParamsSeq->tuneParamIndex].rfTuningFrequencyKHz = 0;
        pGetTuneParamsSeq->tuneParamIndex++;
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t DetermineDsType (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq, uint32_t dsTunePos, uint8_t * pIsDSDS)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t dsLowerRange = 0;
    uint32_t dsUpperRange = 0;
    uint8_t q = 0;
    uint32_t overlapStart = 0;
    uint32_t overlapEnd = 0;

    SONY_TRACE_ENTER ("DetermineDsType");

    if ((!pDemod) || (!pGetTuneParamsSeq) || (!pIsDSDS)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * If range{bns[q],bne[q]} and range{sf+tp[k]*Dx-1704,sf+tp[k]*Dx+1704} is overlapped greater than bnw*Dx-1, this DSLICE is dependent static DS.
     * dependent static DS id should be recorded as
     * ksa[i] = k,  where k=0,1,..,numDs-1 and q=0,1,..,numNotch-1
     */
    dsLowerRange = pGetTuneParamsSeq->sf + dsTunePos * pGetTuneParamsSeq->dx - SONY_DVBC2_OFDM_CAR_CENTER;
    dsUpperRange = dsLowerRange + (2 * SONY_DVBC2_OFDM_CAR_CENTER);

    *pIsDSDS = 0; /* DSlice is noted as Normal */

    for (q = 0; q < pGetTuneParamsSeq->numNotch; q++) {
        /* Check for overlap */

        /* overlapStart = max(notchStart, dsLowerRange) */
        if (pGetTuneParamsSeq->notch[q].notchStart >= dsLowerRange) {
            overlapStart = pGetTuneParamsSeq->notch[q].notchStart;
        }
        else {
            overlapStart = dsLowerRange;
        }

        /* overlapEnd = min(notchEnd, dsUpperRange) */
        if (pGetTuneParamsSeq->notch[q].notchEnd >= dsUpperRange) {
            overlapEnd = dsUpperRange;
        }
        else {
            overlapEnd = pGetTuneParamsSeq->notch[q].notchEnd;
        }

        if (overlapStart < overlapEnd) {
            if ((overlapEnd - overlapStart) > (uint32_t) (pGetTuneParamsSeq->bnw * pGetTuneParamsSeq->dx - 1)) {
                *pIsDSDS = 1; /* DSlice is noted as Dependent Static DS so break out of notch loop */
                break;
            }
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t GetNotches (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("GetNotches");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * numNotch = IREG_NUM_NOTCH
     * ns[q]  = IREG_NOTCH_START_q
     * nw[q] = IREG_NOTCH_WIDTH_q, where q=0,1,..,numNotch-1
     *
     * Assume bnw=2(GI128), 4(GI64), and Dx=24(GI128), 12(GI64).
     * Calculate bns, bne.
     * bns[q] = sf+ns[q]*Dx+1
     * bne[q] = sf+(ns[q]+nw[q])*Dx-1
     */


    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* numNotch = IREG_NUM_NOTCH */
    {
        uint8_t rdata;

        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     2Eh     [3:0]        IREG_NUM_NOTCH[3:0]
         */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x2E, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        pGetTuneParamsSeq->numNotch = (rdata & 0x0F);
    }

    /*
     * ns[q]  = IREG_NOTCH_START_q
     * nw[q] = IREG_NOTCH_WIDTH_q, where q=0,1,..,numNotch-1
     * Calculate bns, bne.
     * bns[q] = sf+ns[q]*Dx+1
     * bne[q] = sf+(ns[q]+nw[q])*Dx-1
     */
    if (pGetTuneParamsSeq->numNotch > 0) {
        /* Slave     Bank    Addr    Bit          Name
         * ------------------------------------------------------------------
         * <SLV-T>   52h     4Dh     [5:0]        IREG_NOTCH_START_0[13:8]
         * <SLV-T>   52h     4Eh     [7:0]        IREG_NOTCH_START_0[7:0]
         * <SLV-T>   52h     4Fh     [0]          IREG_NOTCH_WIDTH_0[8]
         * <SLV-T>   52h     50h     [7:0]        IREG_NOTCH_WIDTH_0[7:0]
         * <SLV-T>   52h     51h     [7:0]        IREG_RESERVED_3_0[7:0]
         *  (snip)
         * A maximum of 15 notches, each notch using 5 registers of data (inc IREG_RESERVED_x)
         */
        uint8_t rdata[SONY_DVBC2_NUM_NOTCH_MAX * SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH];   /* 15 notches x 5 registers per notch */
        uint8_t i = 0;

        if (pGetTuneParamsSeq->numNotch > SONY_DVBC2_NUM_NOTCH_MAX) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Bulk Read */
        if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x4D, rdata, (pGetTuneParamsSeq->numNotch * SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* sort through read data 5 bytes at a time (IREG_RESERVED_X is ignored */
        for (i = 0; i < pGetTuneParamsSeq->numNotch; i++) {
            /*
             * ns[q]  = IREG_NOTCH_START_q
             * nw[q] = IREG_NOTCH_WIDTH_q, where q=0,1,..,numNotch-1
             */
            uint16_t ns; /* Notch start (in carriers/Dx) relative to the start of the C2 system */
            uint16_t nw; /* Notch width (in carriers/Dx) relative to the start of the C2 system */

            ns = (((uint16_t) rdata[(SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)] << 8) | (rdata[1 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)]));
            nw = (((uint16_t) rdata[2 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)] << 8) | (rdata[3 + (SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH * i)]));

            /*
             * Calculate bns, bne.
             * bns[q] = sf+ns[q]*Dx+1
             * bne[q] = sf+(ns[q]+nw[q])*Dx-1
             */
            pGetTuneParamsSeq->notch[i].notchStart = pGetTuneParamsSeq->sf + ns * pGetTuneParamsSeq->dx + 1;
            pGetTuneParamsSeq->notch[i].notchEnd = pGetTuneParamsSeq->sf + (ns + nw) * pGetTuneParamsSeq->dx - 1;
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t CalculateAccumulatedOverlap (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq, uint32_t dsTunePos, uint32_t * pAccumulatedOverlap)
{
    uint32_t dsLowerRange = 0;
    uint32_t dsUpperRange = 0;
    uint8_t q = 0;
    uint32_t overlapStart = 0;
    uint32_t overlapEnd = 0;

    SONY_TRACE_ENTER ("CalculateAccumulatedOverlap");

    if ((!pDemod) || (!pGetTuneParamsSeq) ||(!pAccumulatedOverlap)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* For each q, accumulate overlap between range{bns[q], bne[q]} and range{sf+tp*Dx-1704, sf+tp*Dx+1704} */
    dsLowerRange = pGetTuneParamsSeq->sf + dsTunePos * pGetTuneParamsSeq->dx - SONY_DVBC2_OFDM_CAR_CENTER;
    dsUpperRange = dsLowerRange + (2 * SONY_DVBC2_OFDM_CAR_CENTER);
    *pAccumulatedOverlap = 0;

    for (q = 0; q < pGetTuneParamsSeq->numNotch; q++) {
        /* overlapStart = max(notchStart, dsLowerRange) */
        if (pGetTuneParamsSeq->notch[q].notchStart >= dsLowerRange) {
            overlapStart = pGetTuneParamsSeq->notch[q].notchStart;
        }
        else {
            overlapStart = dsLowerRange;
        }

        /* overlapEnd = min(notchEnd, dsUpperRange) */
        if (pGetTuneParamsSeq->notch[q].notchEnd >= dsUpperRange) {
            overlapEnd = dsUpperRange;
        }
        else {
            overlapEnd = pGetTuneParamsSeq->notch[q].notchEnd;
        }

        if (overlapStart < overlapEnd) {
            *pAccumulatedOverlap += overlapEnd - overlapStart;
        }
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t CalculateNextChannelFrequency (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t rdata[2];

    SONY_TRACE_ENTER ("CalculateNextChannelFrequency");

    if ((!pDemod) || (!pGetTuneParamsSeq)) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Read signal bandwidth */
    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr    Bit          Name
     * ------------------------------------------------------------------
     * <SLV-T>   52h     27h     [7:0]        IREG_C2_BANDWIDTH[15:8]
     * <SLV-T>   52h     28h     [7:0]        IREG_C2_BANDWIDTH[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x27, rdata, sizeof (rdata)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Calculate step size in KHz based on actual bandwidth of found DVB-C2 signal */
    if (pDemod->bandwidth == SONY_DTV_BW_6_MHZ) {
        /* TU 6MHz = ((4096 / 48) * 7) = 1792 / 3 therefore x / TU = x * 3 / 1792 */
        pGetTuneParamsSeq->nextChannelFrequency = ((pGetTuneParamsSeq->sf + ((((uint32_t) rdata[0] << 8) | (rdata[1])) * pGetTuneParamsSeq->dx + 1) +
                                                  SONY_DVBC2_OFDM_CAR_CENTER) * 1000 * 3 / 1792);
    }
    else {
        /* TU 8MHz = ((4096 / 64) * 7 = 448 */
        pGetTuneParamsSeq->nextChannelFrequency = ((pGetTuneParamsSeq->sf + ((((uint32_t) rdata[0] << 8) | (rdata[1])) * pGetTuneParamsSeq->dx + 1) +
                                                  SONY_DVBC2_OFDM_CAR_CENTER) * 1000 / 448);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t DemodSetting1 (sony_demod_t * pDemod, sony_demod_dvbc2_get_tune_params_seq_t * pGetTuneParamsSeq)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("DemodSetting1");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * OREG_CENTER_FREQ = floor( ( [RF Freq(MHz)] * [Tu(us)] + 24/2 ) / 24 ) * 24
     * (Tu(us) = 4096*7/64 (8MHz), 4096*7/48 (6MHz) )
     * OREG_FP_TUNE_MODE = 0
     * OHOST_RESET = H
     */
    result = SetDemodCenterFreq (pDemod, pGetTuneParamsSeq->frequencyKHz);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Dh    [1:0]     8'h00      8'h00      OREGD_FP_TUNE_MODE[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7D, 0x00) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (sony_demod_SoftReset (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t DemodSetting2 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t oregCenterFreq = 0;
    uint8_t data[6];

    SONY_TRACE_ENTER ("DemodSetting2");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     *  1. demod setting for tune
     *    OREG_CENTER_FREQ  = floor (([C2 Tuning Freq (MHz)] * [Tu(us)] + 12/2) / 12) * 12
     *    OREGD_FP_TUNE_MODE = 2
     *    OREGD_FP_TGT_DSLICE_ID = d[k]
     *    OREGD_FP_TGT_PLP_ID = p[k][m]
     *    OREGD_L1C2_PRESET_CLEAR = H
     */

    /* Calculate the tune position in carriers */
    result = CalculateCarrierTunePosition(&oregCenterFreq, pTuneSeq->pTuneData->c2TuningFrequencyKHz, pTuneSeq->bandwidth);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* Slave     Bank    Addr   Bit       Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Ah    [7:0]     8'h03      OREG_CENTER_FREQ[23:16]
     * <SLV-T>   50h     7Bh    [7:0]     8'h59      OREG_CENTER_FREQ[15:8]
     * <SLV-T>   50h     7Ch    [7:0]     8'h88      OREG_CENTER_FREQ[7:0]
     */
    data[0] = (uint8_t) ((oregCenterFreq >> 16) & 0xFF);
    data[1] = (uint8_t) ((oregCenterFreq >> 8) & 0xFF);
    data[2] = (uint8_t) ((oregCenterFreq) & 0xFF);

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Dh    [1:0]     8'h00      8'h02      OREGD_FP_TUNE_MODE[1:0]
     */
    data[3] = 0x02;

    /* Slave     Bank    Addr    Bit      Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Eh    [7:0]     8'h00      OREGD_FP_TGT_DSLICE_ID[7:0]
     * <SLV-T>   50h     7Fh    [7:0]     8'h00      OREGD_FP_TGT_PLP_ID[7:0]
     */
    data[4] = pTuneSeq->pTuneData->dataSliceId;
    data[5] = pTuneSeq->pTuneData->dataPLPId;

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, data, sizeof (data)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     87h     [0]      8'h00      8'h01      OREGD_L1C2_PRESET_CLEAR
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Reset & Hi-Z disable performed in sony_demod_TuneEnd */

    SONY_TRACE_RETURN (result);
}

static sony_result_t DemodSetting3_1 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t oregCenterFreq = 0;
    uint8_t data[7];

    SONY_TRACE_ENTER ("DemodSetting3_1");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * OREG_CENTER_FREQ  = floor (([C2 Tuning Freq (MHz)] * [Tu(us)] + 12/2) / 12) * 12
     * OREGD_FP_TUNE_MODE = 2
     * OREGD_FP_TGT_DSLICE_ID = d[ksa]
     * OREGD_FP_TGT_PLP_ID = p[ksa][m]
     */

    /* Calculate the tune position in carriers */
    result = CalculateCarrierTunePosition(&oregCenterFreq, pTuneSeq->pTuneData->c2TuningFrequencyKHz, pTuneSeq->bandwidth);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* Slave     Bank    Addr   Bit       Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Ah    [7:0]     8'h03      OREG_CENTER_FREQ[23:16]
     * <SLV-T>   50h     7Bh    [7:0]     8'h59      OREG_CENTER_FREQ[15:8]
     * <SLV-T>   50h     7Ch    [7:0]     8'h88      OREG_CENTER_FREQ[7:0]
     */
    data[0] = (uint8_t) ((oregCenterFreq >> 16) & 0xFF);
    data[1] = (uint8_t) ((oregCenterFreq >> 8) & 0xFF);
    data[2] = (uint8_t) ((oregCenterFreq) & 0xFF);

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Dh    [1:0]     8'h00      8'h02      OREGD_FP_TUNE_MODE[1:0]
     */
    data[3] = 0x02;

    /* Slave     Bank    Addr    Bit      Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Eh    [7:0]     8'h00      OREGD_FP_TGT_DSLICE_ID[7:0]
     * <SLV-T>   50h     7Fh    [7:0]     8'h00      OREGD_FP_TGT_PLP_ID[7:0]
     */
    data[4] = pTuneSeq->pTuneData->dataSliceId;
    data[5] = pTuneSeq->pTuneData->dataPLPId;

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, data, 6) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* OREG_L1C2_CENTER_FREQ_SUB = OREG_CENTER_FREQ
     * OREGD_FP_NEXT_TUNE_MODE = 2
     * OREGD_FP_NEXT_DSLICE_ID = d[ksa]
     * OREGD_L1C2_NEXT_PLP_ID = p[ksa][m]
     * OREGD_L1C2_PRESET_CLEAR =H
     * OHOST_RESET=H
     */

    /* Slave     Bank    Addr    Bit      Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     81h    [7:0]     8'h03       OREG_L1C2_CENTER_FREQ_SUB[23:16]
     * <SLV-T>   50h     82h    [7:0]     8'h59       OREG_L1C2_CENTER_FREQ_SUB[15:8]
     * <SLV-T>   50h     83h    [7:0]     8'h88       OREG_L1C2_CENTER_FREQ_SUB[7:0]
     */
    data[0] = (uint8_t) ((oregCenterFreq >> 16) & 0xFF);
    data[1] = (uint8_t) ((oregCenterFreq >> 8) & 0xFF);
    data[2] = (uint8_t) ((oregCenterFreq) & 0xFF);

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     84h    [1:0]     8'h02      8'h02      OREGD_FP_NEXT_TUNE_MODE[1:0]
     */
    data[3] = 0x02;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     85h    [7:0]     8'h00       OREGD_FP_NEXT_DSLICE_ID
     * <SLV-T>   50h     86h    [7:0]     8'h00       OREGD_L1C2_NEXT_PLP_ID
     */
    data[4] = pTuneSeq->pTuneData->dataSliceId;
    data[5] = pTuneSeq->pTuneData->dataPLPId;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     87h    [0]       8'h00       OREGD_L1C2_PRESET_CLEAR
     */
    data[6] = 0x01;

    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, data, 7) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * OHOST_RESET=H
     */
    if (sony_demod_SoftReset (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t DemodSetting3_2 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t l1CenterFreqSub = 0;
    uint8_t data[7];
    uint32_t sf = 0;
    uint32_t tp = 0;
    uint8_t dx = 0;

    SONY_TRACE_ENTER ("DemodSetting3_2");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * OREG_L1C2_CENTER_FREQ_SUB = sf + tp * Dx
     * Where :
     *   sf = IREG_START_FREQUENCY
     *   tp = IREG_DSLICE_TUNE_POS
     *   Dx = 24(GI128), 12(GI64)
     *
     * OREGD_FP_NEXT_TUNE_MODE = 2
     * OREGD_FP_NEXT_DSLICE_ID = d[ksa]
     * OREGD_L1C2_NEXT_PLP_ID = p[ksa][m]
     * OREGD_L1C2_PRESET_CLEAR =H
     */

    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* sf = IREG_START_FREQUENCY */
    /* Slave     Bank    Addr    Bit          Name
     * ------------------------------------------------------------------
     * <SLV-T>   52h     24h     [7:0]        IREG_START_FREQUENCY[23:16]
     * <SLV-T>   52h     25h     [7:0]        IREG_START_FREQUENCY[15:8]
     * <SLV-T>   52h     26h     [7:0]        IREG_START_FREQUENCY[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x24, data, 3) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    sf = (((uint32_t) data[0] << 16) | (data[1] << 8) | (data[2]));

    /* Slave     Bank    Addr    Bit          Name
     * ------------------------------------------------------------------
     * <SLV-T>   52h     30h     [5:0]        IREG_DSLICE_TUNE_POS[13:8]
     * <SLV-T>   52h     31h     [7:0]        IREG_DSLICE_TUNE_POS[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x30, data, 2) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    tp = (((data[0] & 0x3F) << 8) | data[1]);

    if (pTuneSeq->ps[1] & 0x10) {
        dx = DX_GI[(uint8_t)SONY_DVBC2_GI_1_64];
    }
    else {
        dx = DX_GI[(uint8_t)SONY_DVBC2_GI_1_128];
    }

    /* OREG_L1C2_CENTER_FREQ_SUB = sf + tp * Dx */
    l1CenterFreqSub = sf + tp * dx;

    /* Store for use in the second RF tune */
    pTuneSeq->tunePosDSDS = l1CenterFreqSub;

    /* OREG_L1C2_CENTER_FREQ_SUB = sf + tp * Dx
     * OREGD_FP_NEXT_TUNE_MODE = 2
     * OREGD_FP_NEXT_DSLICE_ID = d[ksa]
     * OREGD_L1C2_NEXT_PLP_ID = p[ksa][m]
     * OREGD_L1C2_PRESET_CLEAR =H
     */

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr    Bit      Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     81h    [7:0]     8'h03       OREG_L1C2_CENTER_FREQ_SUB[23:16]
     * <SLV-T>   50h     82h    [7:0]     8'h59       OREG_L1C2_CENTER_FREQ_SUB[15:8]
     * <SLV-T>   50h     83h    [7:0]     8'h88       OREG_L1C2_CENTER_FREQ_SUB[7:0]
     */
    data[0] = (uint8_t) ((l1CenterFreqSub >> 16) & 0xFF);
    data[1] = (uint8_t) ((l1CenterFreqSub >> 8) & 0xFF);
    data[2] = (uint8_t) ((l1CenterFreqSub) & 0xFF);

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     84h    [1:0]     8'h02      8'h02      OREGD_FP_NEXT_TUNE_MODE[1:0]
     */
    data[3] = 0x02;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     85h    [7:0]     8'h00       OREGD_FP_NEXT_DSLICE_ID
     * <SLV-T>   50h     86h    [7:0]     8'h00       OREGD_L1C2_NEXT_PLP_ID
     */
    data[4] = pTuneSeq->pTuneData->dataSliceId;
    data[5] = pTuneSeq->pTuneData->dataPLPId;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     87h    [0]       8'h00       OREGD_L1C2_PRESET_CLEAR
     */
    data[6] = 0x01;

    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, data, 7) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t DemodSetting4 (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    uint8_t data[7];
    uint32_t oregL1C2CenterFreqSub = 0;

    SONY_TRACE_ENTER ("DemodSetting4");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * OREG_L1C2_CENTER_FREQ_SUB = center_freq
     * OREGD_FP_NEXT_TUNE_MODE = 2
     * OREGD_FP_NEXT_DSLICE_ID = d[ksa]
     * OREGD_L1C2_NEXT_PLP_ID = p[ksa][m]
     * OREGD_L1C2_PRESET_CLEAR =H
     */

    oregL1C2CenterFreqSub = pTuneSeq->centerFreq;

    /* Sanity check that value does not overflow 24bits */
    if (oregL1C2CenterFreqSub >= 0xFFFFFF) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Slave     Bank    Addr    Bit      Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     81h    [7:0]     8'h03       OREG_L1C2_CENTER_FREQ_SUB[23:16]
     * <SLV-T>   50h     82h    [7:0]     8'h59       OREG_L1C2_CENTER_FREQ_SUB[15:8]
     * <SLV-T>   50h     83h    [7:0]     8'h88       OREG_L1C2_CENTER_FREQ_SUB[7:0]
     */
    data[0] = (uint8_t) ((oregL1C2CenterFreqSub >> 16) & 0xFF);
    data[1] = (uint8_t) ((oregL1C2CenterFreqSub >> 8) & 0xFF);
    data[2] = (uint8_t) ((oregL1C2CenterFreqSub) & 0xFF);

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     84h    [1:0]     8'h02      8'h02      OREGD_FP_NEXT_TUNE_MODE[1:0]
     */
    data[3] = 0x02;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     85h    [7:0]     8'h00       OREGD_FP_NEXT_DSLICE_ID
     * <SLV-T>   50h     86h    [7:0]     8'h00       OREGD_L1C2_NEXT_PLP_ID
     */
    data[4] = pTuneSeq->pTuneData->dataSliceId;
    data[5] = pTuneSeq->pTuneData->dataPLPId;

    /* Slave     Bank    Addr    Bit      Default    Value      Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     87h    [0]       8'h00       OREGD_L1C2_PRESET_CLEAR
     */
    data[6] = 0x01;

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x81, data, sizeof (data)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t DemodPreset (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t oregCenterFreq = 0;

    SONY_TRACE_ENTER ("DemodPreset");

    if ((!pDemod) || (!pTuneSeq)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /*
     * 1. demod setting for tune
     *  OREG_CENTER_FREQ  = center_freq
     *  OREGD_FP_TUNE_MODE = 2
     *  OREGD_FP_TGT_DSLICE_ID = d[ksa]
     *  OREGD_FP_TGT_PLP_ID = p[ksa][m]
     *  OREG_L1C2_PRESET_CLEAR = H
     */

    oregCenterFreq = pTuneSeq->centerFreq;

    {
        uint8_t data[6];
        /* Sanity check that value does not overflow 24bits */
        if (oregCenterFreq >= 0xFFFFFF) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
        }

        /* Slave     Bank    Addr   Bit       Default    Name
         * --------------------------------------------------------------------------------
         * <SLV-T>   50h     7Ah    [7:0]     8'h03      OREG_CENTER_FREQ[23:16]
         * <SLV-T>   50h     7Bh    [7:0]     8'h59      OREG_CENTER_FREQ[15:8]
         * <SLV-T>   50h     7Ch    [7:0]     8'h88      OREG_CENTER_FREQ[7:0]
         */
        data[0] = (uint8_t) ((oregCenterFreq >> 16) & 0xFF);
        data[1] = (uint8_t) ((oregCenterFreq >> 8) & 0xFF);
        data[2] = (uint8_t) ((oregCenterFreq) & 0xFF);

        /* Slave     Bank    Addr    Bit      Default    Value      Name
         * --------------------------------------------------------------------------------
         * <SLV-T>   50h     7Dh    [1:0]     8'h00      8'h02      OREGD_FP_TUNE_MODE[1:0]
         */
        data[3] = 0x02;

        /* Slave     Bank    Addr    Bit      Default    Name
         * --------------------------------------------------------------------------------
         * <SLV-T>   50h     7Eh    [7:0]     8'h00      OREGD_FP_TGT_DSLICE_ID[7:0]
         * <SLV-T>   50h     7Fh    [7:0]     8'h00      OREGD_FP_TGT_PLP_ID[7:0]
         */
        data[4] = pTuneSeq->pTuneData->dataSliceId;
        data[5] = pTuneSeq->pTuneData->dataPLPId;

        /* Set SLV-T Bank : 0x50 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Burst write data */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr    Bit      Default    Value      Name
         * --------------------------------------------------------------------------------
         * <SLV-T>   50h     87h     [0]      8'h00      8'h01      OREGD_L1C2_PRESET_CLEAR
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, 0x01) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * 2. Special settings for dependent static DS
     *  2.1 Decrease target value of CAS DAGC
     *   OREG_CAS_DAGC_TRGT = casDagcTarget
     */
    {
        uint8_t data[2];

        data[0] = (uint8_t) ((pTuneSeq->casDagcTarget >> 8) & 0x0F);
        data[1] = (uint8_t) ((pTuneSeq->casDagcTarget) & 0xFF);

        /* Set SLV-T Bank : 0x27 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * --------------------------------------------------------------------------------
         * <SLV-T>   27h     F4h    [3:0]     8'h02       OREG_CAS_DAGC_TRGT[11:8]
         * <SLV-T>   27h     F5h    [7:0]     8'h80       OREG_CAS_DAGC_TRGT[7:0]
         */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF4, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * 2.2 Change GICORs smoothing coefficient
     *  OREG_GICOR_SMT_ALPHA_ACQ = 4h0A
     */
    {
        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   27h     51h    [7:4]     8'h4A       OREG_GICOR_SMT_ALPHA_ACQ[3:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x51, 0xA0, 0xF0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * 2.3 Increase gain of TRL
     *  OREG_TRCG_GAIN = 4h0F
     * 2.4 Enlarge TRL acquisition time
     *  OREG_TRCG_GAINCT0 = 6h3F
     *  OREG_TRCG_GAINCT1 = 6h3F
     *  OREG_TRCG_GAINCT2 = 6h3F
     */
    {
        /* Slave     Bank    Addr   Bit       Default    Value   Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   27h     73h    [5:0]     8'h07      4'h0F   OREG_TRCG_GAIN[5:0]
         * <SLV-T>   27h     74h    [5:0]     8'h3F      6'h3F   OREG_TRCG_GAINCT0[5:0]
         * <SLV-T>   27h     75h    [5:0]     8'h3F      6'h3F   OREG_TRCG_GAINCT1[5:0]
         * <SLV-T>   27h     76h    [5:0]     8'h19      6'h3F   OREG_TRCG_GAINCT2[5:0]
         */
        uint8_t data[4] = {0x0F, 0x3F, 0x3F, 0x3F};

        if (pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x73, &data[0], 4) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }


    /* 2.5 Write TRL nominalrate
     *  OREG_TRCG_NOMINALRATE = trcgNominalRate
     */
    {
        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default    Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [6:0]     8'h11      OREG_TRCG_NOMINALRATE[38:32]
         * <SLV-T>   20h     A0h    [7:0]     8'hF0      OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]     8'h00      OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]     8'h00      OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]     8'h00      OREG_TRCG_NOMINALRATE[7:0]
         */
        if (pDemod->pI2c-> WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, pTuneSeq->trcgNominalRate, sizeof (pTuneSeq->trcgNominalRate)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* 2.6 Force SYR to Contra-Raptor mode
     *  OREG_SYR_ACTMODE = 3'h03
     */
    {
        /* Set SLV-T Bank : 0x27 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
        /* Slave     Bank    Addr   Bit       Default   Value   Name
         * --------------------------------------------------------------------------
         * <SLV-T>   27h     C9h    [2:0]     8'h07     3'h03   OREG_SYR_ACTMODE[2:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC9, 0x03) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* 2.7 Write CAS filter coefficients
     *  OREG_CAS_ACIFLT1_EN = 2h1
     *  OREG_CAS_ACIFLT2_EN = 2h1
     */
    {
        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default    Value      Name
         * ----------------------------------------------------------------------------------------------------------------------
         * <SLV-T>   20h     C2h    [7:0]     8'h11      8'h11      2'b00,OREG_CAS_ACIFLT1_EN[1:0],2'b00,OREG_CAS_ACIFLT2_EN[1:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC2, 0x11) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * OREG_CAS_ACIFLT1_C1=cas_aciflt1_c1
     * OREG_CAS_ACIFLT1_C2=cas_aciflt1_c2
     * OREG_CAS_ACIFLT1_C3=cas_aciflt1_c3
     * OREG_CAS_ACIFLT1_C4=cas_aciflt1_c4
     * OREG_CAS_ACIFLT1_C5=cas_aciflt1_c5
     * OREG_CAS_ACIFLT2_C1=cas_aciflt2_c1
     * OREG_CAS_ACIFLT2_C2=cas_aciflt2_c2
     * OREG_CAS_ACIFLT2_C3=cas_aciflt2_c3
     * OREG_CAS_ACIFLT2_C4=cas_aciflt2_c4
     * OREG_CAS_ACIFLT2_C5=cas_aciflt2_c5
     */
    {
        uint8_t data[10];

        /* Set SLV-T Bank : 0x2A */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2A) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * -----------------------------------------------------------------------
         * <SLV-T>   2Ah     20h    [7:0]     8'h16       OREG_CAS_ACIFLT1_C1[7:0]
         * <SLV-T>   2Ah     21h    [7:0]     8'hf0       OREG_CAS_ACIFLT1_C2[7:0]
         * <SLV-T>   2Ah     22h    [7:0]     8'h2B       OREG_CAS_ACIFLT1_C3[7:0]
         * <SLV-T>   2Ah     23h    [7:0]     8'hD8       OREG_CAS_ACIFLT1_C4[7:0]
         * <SLV-T>   2Ah     24h    [7:0]     8'h16       OREG_CAS_ACIFLT1_C5[7:0]
         * <SLV-T>   2Ah     25h    [7:0]     8'h16       OREG_CAS_ACIFLT2_C1[7:0]
         * <SLV-T>   2Ah     26h    [7:0]     8'hF0       OREG_CAS_ACIFLT2_C2[7:0]
         * <SLV-T>   2Ah     27h    [7:0]     8'h2C       OREG_CAS_ACIFLT2_C3[7:0]
         * <SLV-T>   2Ah     28h    [7:0]     8'hD8       OREG_CAS_ACIFLT2_C4[7:0]
         * <SLV-T>   2Ah     29h    [7:0]     8'h16       OREG_CAS_ACIFLT2_C5[7:0]
         */
        data[0] = cas_aciflt1_c1[pTuneSeq->filterSelect];
        data[1] = cas_aciflt1_c2[pTuneSeq->filterSelect];
        data[2] = cas_aciflt1_c3[pTuneSeq->filterSelect];
        data[3] = cas_aciflt1_c4[pTuneSeq->filterSelect];
        data[4] = cas_aciflt1_c5[pTuneSeq->filterSelect];
        data[5] = cas_aciflt2_c1[pTuneSeq->filterSelect];
        data[6] = cas_aciflt2_c2[pTuneSeq->filterSelect];
        data[7] = cas_aciflt2_c3[pTuneSeq->filterSelect];
        data[8] = cas_aciflt2_c4[pTuneSeq->filterSelect];
        data[9] = cas_aciflt2_c5[pTuneSeq->filterSelect];

        /* Burst write data */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * 3. Write PRESET values recorded already in ps[0-22]
     *
     * Slave     Bank    Addr   Bit     ps[i]   Name
     * ----------------------------------------------------------------------------------------------
     * <SLV-T>   50h     14h    [0]     0       IREG_L1C2_PRESET_READY (*)
     * <SLV-T>   50h     15h    [4:3]   1       IREG_L1C2_GI64,IREG_L1C2_TR(*),IREG_L1C2_FEC_HEADER_TYPE_NEXT,IREG_L1C2_DSLICE_TI_DEPTH_NEXT[1:0] (**)
     * <SLV-T>   50h     16h    [1:0]   2       IREG_L1C2_DS_START_NEXT[9:8] (**)
     * <SLV-T>   50h     17h    [7:0]   3       IREG_L1C2_DS_START_NEXT[7:0] (**)
     * <SLV-T>   50h     18h    [1:0]   4       IREG_L1C2_DS_END_NEXT[9:8] (**)
     * <SLV-T>   50h     19h    [7:0]   5       IREG_L1C2_DS_END_NEXT[7:0] (**)
     * <SLV-T>   50h     1Ah    [5:4]   6       IREG_L1C2_NUM_NOTCH_NEXT[1:0] (**),IREG_L1C2_NPSYM[3:0] (*)
     * <SLV-T>   50h     1Bh    [1:0]   7       IREG_L1C2_NOTCH_START_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Ch    [7:0]   8       IREG_L1C2_NOTCH_START_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Dh    [1:0]   9       IREG_L1C2_NOTCH_END_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Eh    [7:0]   10      IREG_L1C2_NOTCH_END_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Fh    [1:0]   11      IREG_L1C2_NOTCH_START_1_NEXT[9:8] (**)
     * <SLV-T>   50h     20h    [7:0]   12      IREG_L1C2_NOTCH_START_1_NEXT[7:0] (**)
     * <SLV-T>   50h     21h    [1:0]   13      IREG_L1C2_NOTCH_END_1_NEXT[9:8] (**)
     * <SLV-T>   50h     22h    [7:0]   14      IREG_L1C2_NOTCH_END_1_NEXT[7:0] (**)
     * <SLV-T>   50h     23h    [1:0]   15      IREG_L1C2_NOTCH_START_2_NEXT[9:8] (**)
     * <SLV-T>   50h     24h    [7:0]   16      IREG_L1C2_NOTCH_START_2_NEXT[7:0] (**)
     * <SLV-T>   50h     25h    [1:0]   17      IREG_L1C2_NOTCH_END_2_NEXT[9:8] (**)
     * <SLV-T>   50h     26h    [7:0]   18      IREG_L1C2_NOTCH_END_2_NEXT[7:0] (**)
     * <SLV-T>   50h     27h    [4:0]   19      IREG_L1C2_LEFT_EP_NEXT,IREG_L1C2_RIGHT_EP_NEXT,IREG_L1C2_IS_BNOTCH_0_NEXT,IREG_L1C2_IS_BNOTCH_1_NEXT,IREG_L1C2_IS_BNOTCH_2_NEXT (**)
     * <SLV-T>   50h     28h    [6:0]   20      IREG_L1C2_COMMON_EN_NEXT,IREG_L1C2_PLP_SIS_MIS_NEXT,IREG_L1C2_PLP_PAYLOAD_TYPE_NEXT[4:0] (**)
     * <SLV-T>   50h     29h    [7:0]   21      IREG_L1C2_PLP_ID0_NEXT[7:0] (**)
     * <SLV-T>   50h     2Ah    [7:0]   22      IREG_L1C2_PLP_ID1_NEXT[7:0] (**)
     *
     * 4. Write CRCG offset value, and enable PRESET mode
     *    OREG_CRCG2_BN_FREQ_OFST = crcg2BnFreqOfst
     *    OREG_C2_DECODE_MODE = 2
     *    OREG_L1C2_PRESET_ON = H
     *    OREG_L1SEQC2_PRSTMODE = H
     */
    {
        uint8_t data[4];

        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------------------------
         * <SLV-T>   50h     55h    [4:0]     8'h00       OREG_L1C2_GI64,OREG_L1C2_TR,OREG_L1C2_FEC_HEADER_TYPE,OREG_L1C2_DSLICE_TI_DEPTH[1:0]
         * <SLV-T>   50h     56h    [1:0]     8'h00       OREG_L1C2_DS_START[9:8]
         * <SLV-T>   50h     57h    [7:0]     8'h00       OREG_L1C2_DS_START[7:0]
         * <SLV-T>   50h     58h    [1:0]     8'h00       OREG_L1C2_DS_END[9:8]
         * <SLV-T>   50h     59h    [7:0]     8'h00       OREG_L1C2_DS_END[7:0]
         * <SLV-T>   50h     5Ah    [5:0]     8'h00       OREG_L1C2_NUM_NOTCH[1:0],OREG_L1C2_NPSYM[3:0]
         * <SLV-T>   50h     5Bh    [1:0]     8'h00       OREG_L1C2_NOTCH_START_0[9:8]
         * <SLV-T>   50h     5Ch    [7:0]     8'h00       OREG_L1C2_NOTCH_START_0[7:0]
         * <SLV-T>   50h     5Dh    [1:0]     8'h00       OREG_L1C2_NOTCH_END_0[9:8]
         * <SLV-T>   50h     5Eh    [7:0]     8'h00       OREG_L1C2_NOTCH_END_0[7:0]
         * <SLV-T>   50h     5Fh    [1:0]     8'h00       OREG_L1C2_NOTCH_START_1[9:8]
         * <SLV-T>   50h     60h    [7:0]     8'h00       OREG_L1C2_NOTCH_START_1[7:0]
         * <SLV-T>   50h     61h    [1:0]     8'h00       OREG_L1C2_NOTCH_END_1[9:8]
         * <SLV-T>   50h     62h    [7:0]     8'h00       OREG_L1C2_NOTCH_END_1[7:0]
         * <SLV-T>   50h     63h    [1:0]     8'h00       OREG_L1C2_NOTCH_START_2[9:8]
         * <SLV-T>   50h     64h    [7:0]     8'h00       OREG_L1C2_NOTCH_START_2[7:0]
         * <SLV-T>   50h     65h    [1:0]     8'h00       OREG_L1C2_NOTCH_END_2[9:8]
         * <SLV-T>   50h     66h    [7:0]     8'h00       OREG_L1C2_NOTCH_END_2[7:0]
         * <SLV-T>   50h     67h    [4:0]     8'h00       OREG_L1C2_LEFT_EP,OREG_L1C2_RIGHT_EP,OREG_L1C2_IS_BNOTCH_0,OREG_L1C2_IS_BNOTCH_1,OREG_L1C2_IS_BNOTCH_2
         * <SLV-T>   50h     68h    [6:0]     8'h00       OREG_L1C2_COMMON_EN,OREG_L1C2_PLP_SIS_MIS,OREG_L1C2_PLP_PAYLOAD_TYPE[4:0]
         * <SLV-T>   50h     69h    [7:0]     8'h00       OREG_L1C2_PLP_ID0[7:0]
         * <SLV-T>   50h     6Ah    [7:0]     8'h00       OREG_L1C2_PLP_ID1[7:0]
         */

        /* Set SLV-T Bank : 0x50 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Burst write preset data */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x55, &pTuneSeq->ps[1], SONY_DEMOD_DVBC2_PS_SIZE - 1) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------------------------
         * <SLV-T>   50h     6Bh    [7:0]     8'h00       OREG_CRCG2_BN_FREQ_OFST[15:8]
         * <SLV-T>   50h     6Ch    [7:0]     8'h00       OREG_CRCG2_BN_FREQ_OFST[7:0]
         */
        data[0] = (uint8_t) ((pTuneSeq->crcg2BnFreqOfst >> 8) & 0xFF);
        data[1] = (uint8_t) ((pTuneSeq->crcg2BnFreqOfst) & 0xFF);

        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------------------------
         * <SLV-T>   50h     6Dh    [1:0]     8'h00       OREG_C2_DECODE_MODE[1:0]
         */
        data[2] = 0x02;

        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------------------------
         * <SLV-T>   50h     6Eh    [0]       8'h00       OREG_L1C2_PRESET_ON
         */
        data[3] = 0x01;

        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6B, data, sizeof(data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * --------------------------------------------------------------------
         * <SLV-T>   50h     6Fh    [6]       1'b0        OREG_L1SEQC2_PRSTMODE
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6F, 0x40, 0x40) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* 5. Start demodulator sequencer by SW reset
     *  OHOST_RESET = H6.
     * 6. Hi-Z setting disable (TS)
     */

    /* Reset & Hi-Z diaable performed in tune end */

    SONY_TRACE_RETURN (result);
}

static sony_result_t ReleasePreset (sony_demod_t * pDemod)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("ReleasePreset");

    if (!pDemod){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * 1. Recover initial values
     *  OREG_CAS_DAGC_TRGT = 12'h280
     */
    {
        static const uint8_t data[2] = { 0x02, 0x80 };

        /* Set SLV-T Bank : 0x27 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default    Value   Name
         * -----------------------------------------------------------------------
         * <SLV-T>   27h     F4h    [3:0]     8'h02      4'h02   OREG_CAS_DAGC_TRGT[11:8]
         * <SLV-T>   27h     F5h    [7:0]     8'h80      8'h80   OREG_CAS_DAGC_TRGT[7:0]
         */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xF4, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* OREG_GICOR_SMT_ALPHA_ACQ = 4h4 */
    {
        /* Slave     Bank    Addr   Bit       Default   Value   Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   27h     51h    [7:0]     8'h4A     4'h04   OREG_GICOR_SMT_ALPHA_ACQ[3:0]
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x51, 0x40, 0xF0) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * OREG_TRCG_GAIN = 4'b0111
     * OREG_TRCG_GAINCT0 = 6'h3F
     * OREG_TRCG_GAINCT1 = 6'h3F
     * OREG_TRCG_GAINCT2 = 6'h19
     */
    {
        /* Slave     Bank    Addr   Bit       Default    Value   Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   27h     73h    [5:0]     8'h07      4'h07   OREG_TRCG_GAIN[5:0]
         * <SLV-T>   27h     74h    [5:0]     8'h3F      6'h3F   OREG_TRCG_GAINCT0[5:0]
         * <SLV-T>   27h     75h    [5:0]     8'h3F      6'h3F   OREG_TRCG_GAINCT1[5:0]
         * <SLV-T>   27h     76h    [5:0]     8'h19      6'h19   OREG_TRCG_GAINCT2[5:0]
         */
        uint8_t data[4] = {0x07, 0x3F, 0x3F, 0x19};

        if (pDemod->pI2c->WriteRegister(pDemod->pI2c, pDemod->i2cAddressSLVT, 0x73, &data[0], 4) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /* OREG_TRCG_NOMINALRATE */
    {
        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* <Timing Recovery setting>
         * Slave     Bank    Addr   Bit     Default   Name
         * -----------------------------------------------------------------------
         * <SLV-T>   20h     9Fh    [6:0]   8'h11     OREG_TRCG_NOMINALRATE[38:32]
         * <SLV-T>   20h     A0h    [7:0]   8'hF0     OREG_TRCG_NOMINALRATE[31:24]
         * <SLV-T>   20h     A1h    [7:0]   8'h00     OREG_TRCG_NOMINALRATE[23:16]
         * <SLV-T>   20h     A2h    [7:0]   8'h00     OREG_TRCG_NOMINALRATE[15:8]
         * <SLV-T>   20h     A3h    [7:0]   8'h00     OREG_TRCG_NOMINALRATE[7:0]
         */

        switch (pDemod->bandwidth) {
        case SONY_DTV_BW_8_MHZ:
            {
                const uint8_t nominalRate[5] = {
                    /* TRCG Nominal Rate [38:0] */
                    0x15, 0x00, 0x00, 0x00, 0x00
                };

                if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, nominalRate, 5) != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }
            }
            break;

        case SONY_DTV_BW_6_MHZ:
            {
                const uint8_t nominalRate[5] = {
                    /* TRCG Nominal Rate [38:0] */
                    0x1C, 0x00, 0x00, 0x00, 0x00
                };

                if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x9F, nominalRate, 5) != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
                }
            }
            break;

            default:
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }

    /*
     * OREG_SYR_ACTMODE = 3'h7
     */
    {
        /* Set SLV-T Bank : 0x27 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x27) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default   Value   Name
         * ----------------------------------------------------------------------------
         * <SLV-T>   27h     C9h    [2:0]     8'h07    3'h07    OREG_SYR_ACTMODE[2:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC9, 0x07) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * OREG_CAS_ACIFLT1_EN = 2h1
     * OREG_CAS_ACIFLT2_EN = 2h1
     */
    {
        /* Set SLV-T Bank : 0x20 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default    Value   Name
         * -------------------------------------------------------------------------------------------------------------------
         * <SLV-T>   20h     C2h    [7:0]     8'h11      8'h11   2'b00,OREG_CAS_ACIFLT1_EN[1:0],2'b00,OREG_CAS_ACIFLT2_EN[1:0]
         */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC2, 0x11) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * OREG_CAS_ACIFLT1_C1 = 8'h16
     * OREG_CAS_ACIFLT1_C2 = 8'hf0
     * OREG_CAS_ACIFLT1_C3 = 8'h2b
     * OREG_CAS_ACIFLT1_C4 = 8'hd8
     * OREG_CAS_ACIFLT1_C5 = 8'h16
     * OREG_CAS_ACIFLT2_C1 = 8'h16
     * OREG_CAS_ACIFLT2_C2 = 8'hf0
     * OREG_CAS_ACIFLT2_C3 = 8'h2c
     * OREG_CAS_ACIFLT2_C4 = 8'hd8
     * OREG_CAS_ACIFLT2_C5 = 8'h16
     */
    {
        const uint8_t data[10] = {0x16, 0xF0, 0x2B, 0xD8, 0x16, 0x16, 0xF0, 0x2C, 0xD8, 0x16};

        /* Set SLV-T Bank : 0x2A */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x2A) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }


        /* Slave     Bank    Addr   Bit       Default     Name
         * -----------------------------------------------------------------------
         * <SLV-T>   2Ah     20h    [7:0]     8'h16       OREG_CAS_ACIFLT1_C1[7:0]
         * <SLV-T>   2Ah     21h    [7:0]     8'hf0       OREG_CAS_ACIFLT1_C2[7:0]
         * <SLV-T>   2Ah     22h    [7:0]     8'h2B       OREG_CAS_ACIFLT1_C3[7:0]
         * <SLV-T>   2Ah     23h    [7:0]     8'hD8       OREG_CAS_ACIFLT1_C4[7:0]
         * <SLV-T>   2Ah     24h    [7:0]     8'h16       OREG_CAS_ACIFLT1_C5[7:0]
         * <SLV-T>   2Ah     25h    [7:0]     8'h16       OREG_CAS_ACIFLT2_C1[7:0]
         * <SLV-T>   2Ah     26h    [7:0]     8'hF0       OREG_CAS_ACIFLT2_C2[7:0]
         * <SLV-T>   2Ah     27h    [7:0]     8'h2C       OREG_CAS_ACIFLT2_C3[7:0]
         * <SLV-T>   2Ah     28h    [7:0]     8'hD8       OREG_CAS_ACIFLT2_C4[7:0]
         * <SLV-T>   2Ah     29h    [7:0]     8'h16       OREG_CAS_ACIFLT2_C5[7:0]
         */

        /* Burst write data */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x20, data, sizeof (data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    /*
     * 2. Set CRCGs offset value to 0, and disable PRESET mode
     *   OREG_CRCG2_BN_FREQ_OFST = 0
     *   OREG_C2_DECODE_MODE = 0
     *   OREG_L1C2_PRESET_ON = L
     *   OREG_L1SEQC2_PRSTMODE = L
     */
    {
        const uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};

        /* Set SLV-T Bank : 0x50 */
        if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * ----------------------------------------------------------------------------------------------
         * <SLV-T>   50h     6Bh    [7:0]     8'h00       OREG_CRCG2_BN_FREQ_OFST[15:8]
         * <SLV-T>   50h     6Ch    [7:0]     8'h00       OREG_CRCG2_BN_FREQ_OFST[7:0]
         * <SLV-T>   50h     6Dh    [1:0]     8'h00       OREG_C2_DECODE_MODE[1:0]
         * <SLV-T>   50h     6Eh    [0]       8'h00       OREG_L1C2_PRESET_ON
         */
        if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6B, data, sizeof(data)) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }

        /* Slave     Bank    Addr   Bit       Default     Name
         * --------------------------------------------------------------------
         * <SLV-T>   50h     6Fh    [6]       1'h00       OREG_L1SEQC2_PRSTMODE
         */
        if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6F, 0x00, 0x40) != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t SetDemodCenterFreq (sony_demod_t * pDemod, uint32_t centerFreqkHz)
{
    uint8_t data[3];
    uint32_t oregCenterFreqCarriers = 0;
    uint32_t div = 0;

    SONY_TRACE_ENTER ("SetDemodCenterFreq");

    if (!pDemod){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    /*
     * OREG_CENTER_FREQ = floor( ( [RF Freq(MHz)] * [Tu(us)] + 24/2 ) / 24 ) * 24
     * (Tu(us) = 4096*7/64 (8MHz), 4096*7/48 (6MHz) )
     */

    if (pDemod->bandwidth == SONY_DTV_BW_6_MHZ) {
        /* TU 6MHz = ((4096 / 48) * 7) = 1792 / 3 */
        div = 3 * 1000 * 24;
        oregCenterFreqCarriers = (((centerFreqkHz * 1792) + (div / 2)) / div) * 24;
    }
    else {
        /* TU 8MHz = ((4096 / 64) * 7 = 448 */
        div = 1000 * 24;
        oregCenterFreqCarriers = (((centerFreqkHz * 448) + (div / 2)) / div) * 24;
    }

    /* Sanity check that value does not overflow 24bits */
    if (oregCenterFreqCarriers >= 0xFFFFFF) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Slave     Bank    Addr   Bit       Default    Name
     * --------------------------------------------------------------------------------
     * <SLV-T>   50h     7Ah    [7:0]     8'h03      OREG_CENTER_FREQ[23:16]
     * <SLV-T>   50h     7Bh    [7:0]     8'h59      OREG_CENTER_FREQ[15:8]
     * <SLV-T>   50h     7Ch    [7:0]     8'h88      OREG_CENTER_FREQ[7:0]
     */
    data[0] = (uint8_t) ((oregCenterFreqCarriers >> 16) & 0xFF);
    data[1] = (uint8_t) ((oregCenterFreqCarriers >> 8) & 0xFF);
    data[2] = (uint8_t) ((oregCenterFreqCarriers) & 0xFF);

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Burst write data */
    if (pDemod->pI2c->WriteRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7A, data, sizeof (data)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t Change1stTrial (sony_demod_t * pDemod)
{
    uint8_t data = 0x00;
    uint8_t rdata = 0x00;

    SONY_TRACE_ENTER ("Change1stTrial");

    if (!pDemod){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * Change 1st trial of GI,TR,spectrum for the next acquisition.
     * 1. Read these registers
     * IREG_CRCG2_SINVP1
     * IREG_L1C2_GI64
     * IREG_L1C2_TR
     *
     * 2. Write these registers
     * OREG_L1SEQC2_GI64_INIT = IREG_L1C2_GI64
     * OREG_L1SEQC2_TR_INIT = IREG_L1C2_TR
     * OREG_L1SEQC2_SPINV_INIT = IREG_CRCG2_SINVP1
     */

    /* Set SLV-T Bank : 0x28 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x28) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    /*
     * Slave    Bank    Addr    Bit      Name                 Meaning
     * ---------------------------------------------------------------------------------
     * <SLV-T>   28h     E6h     [0]     IREG_CRCG2_SINVP1    0:not invert,   1:invert
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xE6, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    data = (rdata & 0x01);

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave     Bank    Addr   Bit      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     15h    [4:3]    IREG_L1C2_GI64, IREG_L1C2_TR
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x15, &rdata, sizeof (rdata)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    data |= ((rdata & 0x18) >> 2);

    /* Slave     Bank    Addr   Bit      Default     Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     6Fh    [6:0]    8'h20       OREG_L1SEQC2_GI64_INIT,OREG_L1SEQC2_TR_INIT,OREG_L1SEQC2_SPINV_INIT
     */
    if (sony_i2c_SetRegisterBits (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x6F, data, 0x07) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

static sony_result_t CheckGetTuneParamsDemodLock(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq)
{
    sony_demod_lock_result_t lockStatus = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_result_t result;

    SONY_TRACE_ENTER("CheckGetTuneParamsDemodLock");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = sony_demod_dvbc2_CheckDemodLock(pDemod, &lockStatus);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    switch(lockStatus){
        case SONY_DEMOD_LOCK_RESULT_LOCKED:
            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_STAGE_3;
            pGetTuneParamsSeq->loopResult = SONY_RESULT_OK;
            break;

        case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
            /* Demod lock failed */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_FINALIZE;
            pGetTuneParamsSeq->loopResult = SONY_RESULT_ERROR_UNLOCK;
            break;

        case SONY_DEMOD_LOCK_RESULT_NOTDETECT:
            if (pGetTuneParamsSeq->currentTime - pGetTuneParamsSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK) {
                /* Continue waiting (state is not changed.) */
                pGetTuneParamsSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK_INTERVAL;
            }
            else {
                /* Demod lock timeout */
                /* Finalize this iteration of the scan routine and provide an unlock callback */
                pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_FINALIZE;
                pGetTuneParamsSeq->loopResult = SONY_RESULT_ERROR_TIMEOUT;
            }
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t CheckL1DataReady(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq, uint8_t * pL1DataReady)
{
    uint8_t r_data_en = 0;
    uint8_t preset_ready_mon = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CheckL1DataReady");

    if ((!pDemod) || (!pGetTuneParamsSeq) || (!pL1DataReady)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Default return value to false */
    *pL1DataReady = 0;

    /* wait IREG_L1P2R_DATA_EN=H && IREG_L1C2_PRESET_READY_MON[0]=H */
    /* Freeze registers */
    if (SLVT_FreezeReg (pDemod) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Set SLV-T Bank : 0x52 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x52) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /*
     * Slave    Bank    Addr    Bit        Name                         Meaning
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
     * Slave    Bank    Addr    Bit        Name                         Meaning
     * --------------------------------------------------------------------------------------
     * <SLV-T>   50h     C4h     [0]       IREG_L1C2_PRESET_READY_MON   0:invalid, 1:valid
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0xC4, &preset_ready_mon, 1) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if ((r_data_en & 0x01) && (preset_ready_mon & 0x01)){
        /* L1 data is ready */
        *pL1DataReady = 1;
        pGetTuneParamsSeq->loopResult = SONY_RESULT_OK;
    }
    else
    {
        /* L1 data not yet ready */
        if (pGetTuneParamsSeq->currentTime - pGetTuneParamsSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_L1_DATA) {
            /* Continue waiting (state is not changed.) */
            pGetTuneParamsSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_L1_DATA_INTERVAL;
        }
        else {
            /* Wait L1 Data timeout */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pGetTuneParamsSeq->state = SONY_DEMOD_DVBC2_GET_TUNE_PARAMS_FINALIZE;
            pGetTuneParamsSeq->loopResult = SONY_RESULT_ERROR_TIMEOUT;
        }
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t SetNextDS(sony_demod_t *pDemod, sony_demod_dvbc2_get_tune_params_seq_t *pGetTuneParamsSeq)
{
    SONY_TRACE_ENTER("SetNextDS");

    if ((!pDemod) || (!pGetTuneParamsSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * OREGD_FP_TUNE_MODE = 1
     * OREGD_FP_TGT_DSLICE_ID = d[k]
     * OREGD_L1C2_PRESET_CLEAR = H
     */

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit       Default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     7Dh    [1:0]     8'h00      2'h01      OREGD_FP_TUNE_MODE[1:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7D, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit       Default     Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     7Eh    [7:0]     8'h00       OREGD_FP_TGT_DSLICE_ID[7:0]
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x7E, pGetTuneParamsSeq->ds[pGetTuneParamsSeq->currentDsIndex]) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit       Default    Value      Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     87h    [0]       8'h00      1'h01      OREGD_L1C2_PRESET_CLEAR
     */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x87, 0x01) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

static sony_result_t CheckPresetReady1_1 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    uint8_t syncStat;
    uint8_t tsLock;
    uint8_t earlyUnlock;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CheckPresetReady1_1");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * IREG_L1C2_PRESET_READY
     * IREG_L1C2_GI64
     * Read these registers.
     * Wait until IREG_L1C2_PRESET_READY == H,
     * then record the above mentioned values to pTuneSeq->ps[]
     */

    result = sony_demod_dvbc2_monitor_SyncStat(pDemod, &syncStat, &tsLock, &earlyUnlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (earlyUnlock){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_UNLOCK);
    }

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank     Addr   Bit          Name
     * ------------------------------------------------------------
     * <SLV-T>   50h     14h    [0]          IREG_L1C2_PRESET_READY
     * <SLV-T>   50h     15h    [4]          IREG_L1C2_GI64
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x14, &pTuneSeq->ps[0], 2) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pTuneSeq->ps[0] & 0x01) {
        /* Data is valid */
        /* Move onto the next state and return loop OK */
        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_3;
    }
    else {
        /* Preset Data not valid yet */
        if (pTuneSeq->currentTime - pTuneSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_PRESET_READY) {
            /* Continue waiting (state is not changed.) */
            pTuneSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_PRESET_READY_INTERVAL;
            result = SONY_RESULT_OK;
        }
        else {
            /* Preset ready timeout */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
            result = SONY_RESULT_ERROR_TIMEOUT;
        }
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t CheckPresetReady1_2 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    uint8_t syncStat;
    uint8_t tsLock;
    uint8_t earlyUnlock;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CheckPresetReady1_2");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /*
     * IREG_L1C2_PRESET_READY
     * IREG_L1C2_GI64
     * IREG_L1C2_DS_START_NEXT
     * IREG_L1C2_DS_END_NEXT
     * IREG_L1C2_NUM_NOTCH_NEXT
     * IREG_L1C2_NOTCH_START_0_NEXT
     * IREG_L1C2_NOTCH_END_0_NEXT
     * IREG_L1C2_NOTCH_START_1_NEXT
     * IREG_L1C2_NOTCH_END_1_NEXT
     * IREG_L1C2_NOTCH_START_2_NEXT
     * IREG_L1C2_NOTCH_END_2_NEXT
     * IREG_L1C2_IS_BNOTCH_0_NEXT
     * IREG_L1C2_IS_BNOTCH_1_NEXT
     * IREG_L1C2_IS_BNOTCH_2_NEXT
     * Read these registers.
     * Wait until IREG_L1C2_PRESET_READY == H,
     * then record the above mentioned values to pTuneSeq->ps[]
     */

    result = sony_demod_dvbc2_monitor_SyncStat(pDemod, &syncStat, &tsLock, &earlyUnlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (earlyUnlock){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_UNLOCK);
    }

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank     Addr   Bit          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     14h    [0]          IREG_L1C2_PRESET_READY (*)
     * <SLV-T>   50h     15h    [4:3]        IREG_L1C2_GI64,IREG_L1C2_TR(*)
     * <SLV-T>   50h     15h    [2:0]        IREG_L1C2_FEC_HEADER_TYPE_NEXT,IREG_L1C2_DSLICE_TI_DEPTH_NEXT[1:0] (**)
     * <SLV-T>   50h     16h    [1:0]        IREG_L1C2_DS_START_NEXT[9:8] (**)
     * <SLV-T>   50h     17h    [7:0]        IREG_L1C2_DS_START_NEXT[7:0] (**)
     * <SLV-T>   50h     18h    [1:0]        IREG_L1C2_DS_END_NEXT[9:8] (**)
     * <SLV-T>   50h     19h    [7:0]        IREG_L1C2_DS_END_NEXT[7:0] (**)
     * <SLV-T>   50h     1Ah    [5:4]        IREG_L1C2_NUM_NOTCH_NEXT[1:0] (**)
     * <SLV-T>   50h     1Ah    [3:0]        IREG_L1C2_NPSYM[3:0] (*)
     * <SLV-T>   50h     1Bh    [1:0]        IREG_L1C2_NOTCH_START_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Ch    [7:0]        IREG_L1C2_NOTCH_START_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Dh    [1:0]        IREG_L1C2_NOTCH_END_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Eh    [7:0]        IREG_L1C2_NOTCH_END_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Fh    [1:0]        IREG_L1C2_NOTCH_START_1_NEXT[9:8] (**)
     * <SLV-T>   50h     20h    [7:0]        IREG_L1C2_NOTCH_START_1_NEXT[7:0] (**)
     * <SLV-T>   50h     21h    [1:0]        IREG_L1C2_NOTCH_END_1_NEXT[9:8] (**)
     * <SLV-T>   50h     22h    [7:0]        IREG_L1C2_NOTCH_END_1_NEXT[7:0] (**)
     * <SLV-T>   50h     23h    [1:0]        IREG_L1C2_NOTCH_START_2_NEXT[9:8] (**)
     * <SLV-T>   50h     24h    [7:0]        IREG_L1C2_NOTCH_START_2_NEXT[7:0] (**)
     * <SLV-T>   50h     25h    [1:0]        IREG_L1C2_NOTCH_END_2_NEXT[9:8] (**)
     * <SLV-T>   50h     26h    [7:0]        IREG_L1C2_NOTCH_END_2_NEXT[7:0] (**)
     * <SLV-T>   50h     27h    [4:0]        IREG_L1C2_LEFT_EP_NEXT,IREG_L1C2_RIGHT_EP_NEXT,IREG_L1C2_IS_BNOTCH_0_NEXT,IREG_L1C2_IS_BNOTCH_1_NEXT,IREG_L1C2_IS_BNOTCH_2_NEXT (**)
     * <SLV-T>   50h     28h    [6:0]        IREG_L1C2_COMMON_EN_NEXT,IREG_L1C2_PLP_SIS_MIS_NEXT,IREG_L1C2_PLP_PAYLOAD_TYPE_NEXT[4:0] (**)
     * <SLV-T>   50h     29h    [7:0]        IREG_L1C2_PLP_ID0_NEXT[7:0] (**)
     * <SLV-T>   50h     2Ah    [7:0]        IREG_L1C2_PLP_ID1_NEXT[7:0] (**)
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x14, &pTuneSeq->ps[0], SONY_DEMOD_DVBC2_PS_SIZE) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pTuneSeq->ps[0] & 0x01) {
        /* Data is valid */
        /* Move onto the next state and return loop OK */
        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_5;
    }
    else {
        /* Preset Data not valid yet */
        if (pTuneSeq->currentTime - pTuneSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_PRESET_READY) {
            /* Continue waiting (state is not changed.) */
            pTuneSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_PRESET_READY_INTERVAL;
            result = SONY_RESULT_OK;
        }
        else {
            /* Preset ready timeout */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
            result = SONY_RESULT_ERROR_TIMEOUT;
        }
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t CalculateCenterFreq (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    uint32_t left_bbn_width = 0;
    uint32_t right_bbn_width = 0;
    uint32_t ds_start_next = 0;
    uint8_t isBNotchNextBit = 0x04;
    uint8_t i = 0;
    uint8_t Dx;

    SONY_TRACE_ENTER("CalculateCenterFreq");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* Get DS Start Next from preset information */
    ds_start_next = ((((uint32_t) pTuneSeq->ps[2] & 0x03) << 8) | (pTuneSeq->ps[3]));

    /* Get GI from IREG_L1C2_GI64 (stored in pTuneSeq->ps[1]) and use to calculate Dx */
    if (pTuneSeq->ps[1] & 0x10) {
        Dx = DX_GI[(uint8_t)SONY_DVBC2_GI_1_64];
    }
    else {
        Dx = DX_GI[(uint8_t)SONY_DVBC2_GI_1_128];
    }

    /* Calculate CENTER_FREQ */
    {
        /* DVB-C2 specification allows upto three notches in one tuning position */
        for (i = 0; i < 3; i++) {
            /* IREG_L1C2_IS_BNOTCH_x_NEXT = H */
            if (pTuneSeq->ps[19] & isBNotchNextBit) {
                uint32_t notch_start_next = ((((uint32_t) pTuneSeq->ps[7 + (i * 4)] & 0x03) << 8) | (pTuneSeq->ps[8 + (i * 4)]));
                uint32_t notch_end_next = ((((uint32_t) pTuneSeq->ps[9 + (i * 4)] & 0x03) << 8) | (pTuneSeq->ps[10 + (i * 4)]));

                if (ds_start_next > notch_start_next) {
                    /* left_bbn_width = ceil (4 * IREG_L1C2_NOTCH_END_x_NEXT / Dx) */
                    left_bbn_width = ((4 * notch_end_next) + (Dx - 1)) / Dx;
                }

                if (ds_start_next < notch_start_next) {
                    /* right_bbn_width = ceil ((3408 - 4 * IREG_L1C2_NOTCH_START_x_NEXT) / Dx ) */
                    right_bbn_width = ((SONY_DVBC2_OFDM_CAR_MAX - 4 * notch_start_next) + (Dx - 1)) / Dx;
                }
            }

            /* Change bitmask to check for IREG_L1C2_IS_BNOTCH_x_NEXT in next loop iteration */
            isBNotchNextBit >>= 1;
        }

        /* center_freq = tune_pos_dsds + ( left_bbn_width - ceil( (left_bbn_width + right_bbn_width) / 2 ) ) * Dx */
        pTuneSeq->centerFreq = pTuneSeq->tunePosDSDS + (left_bbn_width - ((left_bbn_width + right_bbn_width + 1) / 2)) * Dx;
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t CheckPresetReady2 (sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    uint8_t syncStat;
    uint8_t tsLock;
    uint8_t earlyUnlock;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CheckPresetReady2");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    /*
     * Get PRESET information
     * ps[0]  = IREG_L1C2_PRESET_READY (*)
     * ps[1]  = IREG_L1C2_GI64, IREG_L1C2_TR, IREG_L1C2_FEC_HEADER_TYPE_NEXT, IREG_L1C2_DSLICE_TI_DEPTH_NEXT[1:0]
     * ps[2]  = IREG_L1C2_DS_START_NEXT[9:8]
     * ps[3]  = IREG_L1C2_DS_START_NEXT[7:0]
     * ps[4]  = IREG_L1C2_DS_END_NEXT[9:8]
     * ps[5]  = IREG_L1C2_DS_END_NEXT[7:0]
     * ps[6]  = IREG_L1C2_NUM_NOTCH_NEXT[1:0], IREG_L1C2_NPSYM[3:0]
     * ps[7]  = IREG_L1C2_NOTCH_START_0_NEXT[9:8]
     * ps[8]  = IREG_L1C2_NOTCH_START_0_NEXT[7:0]
     * ps[9]  = IREG_L1C2_NOTCH_END_0_NEXT[9:8]
     * ps[10] = IREG_L1C2_NOTCH_END_0_NEXT[7:0]
     * ps[11] = IREG_L1C2_NOTCH_START_1_NEXT[9:8]
     * ps[12] = IREG_L1C2_NOTCH_START_1_NEXT[7:0]
     * ps[13] = IREG_L1C2_NOTCH_END_1_NEXT[9:8]
     * ps[14] = IREG_L1C2_NOTCH_END_1_NEXT[7:0]
     * ps[15] = IREG_L1C2_NOTCH_START_2_NEXT[9:8]
     * ps[16] = IREG_L1C2_NOTCH_START_2_NEXT[7:0]
     * ps[17] = IREG_L1C2_NOTCH_END_2_NEXT[9:8]
     * ps[18] = IREG_L1C2_NOTCH_END_2_NEXT[7:0]
     * ps[19] = IREG_L1C2_LEFT_EP_NEXT,IREG_L1C2_RIGHT_EP_NEXT,IREG_L1C2_IS_BNOTCH_0_NEXT,IREG_L1C2_IS_BNOTCH_1_NEXT,IREG_L1C2_IS_BNOTCH_2_NEXT
     * ps[20] = IREG_L1C2_COMMON_EN_NEXT,IREG_L1C2_PLP_SIS_MIS_NEXT,IREG_L1C2_PLP_PAYLOAD_TYPE_NEXT[4:0]
     * ps[21] = IREG_L1C2_PLP_ID0_NEXT[7:0]
     * ps[22] = IREG_L1C2_PLP_ID1_NEXT[7:0]

     * Read these registers.
     * Wait until IREG_L1C2_PRESET_READY == H, then record the above mentioned values other than IREG_L1C2_PRESET_READY to ps[0-23].
     */

    result = sony_demod_dvbc2_monitor_SyncStat(pDemod, &syncStat, &tsLock, &earlyUnlock);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    if (earlyUnlock){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_UNLOCK);
    }

    /* Set SLV-T Bank : 0x50 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x50) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr    Bit          Name
     * ----------------------------------------------------------------------------------
     * <SLV-T>   50h     14h    [0]          IREG_L1C2_PRESET_READY (*)
     * <SLV-T>   50h     15h    [4:3]        IREG_L1C2_GI64,IREG_L1C2_TR(*)
     * <SLV-T>   50h     15h    [2:0]        IREG_L1C2_FEC_HEADER_TYPE_NEXT,IREG_L1C2_DSLICE_TI_DEPTH_NEXT[1:0] (**)
     * <SLV-T>   50h     16h    [1:0]        IREG_L1C2_DS_START_NEXT[9:8] (**)
     * <SLV-T>   50h     17h    [7:0]        IREG_L1C2_DS_START_NEXT[7:0] (**)
     * <SLV-T>   50h     18h    [1:0]        IREG_L1C2_DS_END_NEXT[9:8] (**)
     * <SLV-T>   50h     19h    [7:0]        IREG_L1C2_DS_END_NEXT[7:0] (**)
     * <SLV-T>   50h     1Ah    [5:4]        IREG_L1C2_NUM_NOTCH_NEXT[1:0] (**)
     * <SLV-T>   50h     1Ah    [3:0]        IREG_L1C2_NPSYM[3:0] (*)
     * <SLV-T>   50h     1Bh    [1:0]        IREG_L1C2_NOTCH_START_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Ch    [7:0]        IREG_L1C2_NOTCH_START_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Dh    [1:0]        IREG_L1C2_NOTCH_END_0_NEXT[9:8] (**)
     * <SLV-T>   50h     1Eh    [7:0]        IREG_L1C2_NOTCH_END_0_NEXT[7:0] (**)
     * <SLV-T>   50h     1Fh    [1:0]        IREG_L1C2_NOTCH_START_1_NEXT[9:8] (**)
     * <SLV-T>   50h     20h    [7:0]        IREG_L1C2_NOTCH_START_1_NEXT[7:0] (**)
     * <SLV-T>   50h     21h    [1:0]        IREG_L1C2_NOTCH_END_1_NEXT[9:8] (**)
     * <SLV-T>   50h     22h    [7:0]        IREG_L1C2_NOTCH_END_1_NEXT[7:0] (**)
     * <SLV-T>   50h     23h    [1:0]        IREG_L1C2_NOTCH_START_2_NEXT[9:8] (**)
     * <SLV-T>   50h     24h    [7:0]        IREG_L1C2_NOTCH_START_2_NEXT[7:0] (**)
     * <SLV-T>   50h     25h    [1:0]        IREG_L1C2_NOTCH_END_2_NEXT[9:8] (**)
     * <SLV-T>   50h     26h    [7:0]        IREG_L1C2_NOTCH_END_2_NEXT[7:0] (**)
     * <SLV-T>   50h     27h    [4:0]        IREG_L1C2_LEFT_EP_NEXT,IREG_L1C2_RIGHT_EP_NEXT,IREG_L1C2_IS_BNOTCH_0_NEXT,IREG_L1C2_IS_BNOTCH_1_NEXT,IREG_L1C2_IS_BNOTCH_2_NEXT (**)
     * <SLV-T>   50h     28h    [6:0]        IREG_L1C2_COMMON_EN_NEXT,IREG_L1C2_PLP_SIS_MIS_NEXT,IREG_L1C2_PLP_PAYLOAD_TYPE_NEXT[4:0] (**)
     * <SLV-T>   50h     29h    [7:0]        IREG_L1C2_PLP_ID0_NEXT[7:0] (**)
     * <SLV-T>   50h     2Ah    [7:0]        IREG_L1C2_PLP_ID1_NEXT[7:0] (**)
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x14, &pTuneSeq->ps[0], SONY_DEMOD_DVBC2_PS_SIZE) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    if (pTuneSeq->ps[0] & 0x01) {
        /* Data is valid */
        /* Move onto the next state and return loop OK */
        pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_7;
    }
    else {
        /* Preset Data not valid yet */
        if (pTuneSeq->currentTime - pTuneSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_PRESET_READY) {
            /* Continue waiting (state is not changed.) */
            pTuneSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_PRESET_READY_INTERVAL;
            result = SONY_RESULT_OK;
        }
        else {
            /* Preset ready timeout */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
            result = SONY_RESULT_ERROR_TIMEOUT;
        }
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t CalculatePresetValue (sony_demod_t * pDemod, sony_demod_dvbc2_tune_seq_t * pTuneSeq)
{
    uint32_t number_of_useful_carrier;
    uint32_t bn_width = 0;  /* variable to store BroadbandNotch width */
    uint8_t isBNotchNextBit = 0x04;
    uint8_t i = 0;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CalculatePresetValue");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* 1. Calculate CRCG offset
     * 2. Calculate CAS DAGC TARGET value */

    /* crcg2BnFreqOfst = (tune_pos_dsds - center_freq ) * 2^4 */
    pTuneSeq->crcg2BnFreqOfst = (int16_t) (((int32_t)pTuneSeq->tunePosDSDS - (int32_t)pTuneSeq->centerFreq) * (1 << 4));

    /* DVB-C2 specification allows upto three notches in one tuning position */
    for (i = 0; i < 3; i++) {
        /* IREG_L1C2_IS_BNOTCH_x_NEXT = H */
        if (pTuneSeq->ps[19] & isBNotchNextBit) {
            uint32_t notch_start_next = ((((uint32_t) pTuneSeq->ps[7 + (i * 4)] & 0x03) << 8) | (pTuneSeq->ps[8 + (i * 4)]));
            uint32_t notch_end_next = ((((uint32_t) pTuneSeq->ps[9 + (i * 4)] & 0x03) << 8) | (pTuneSeq->ps[10 + (i * 4)]));

            /* bn_width += (IREG_L1C2_NOTCH_END_[n]_NEXT - IREG_L1C2_NOTCH_START_[n]_NEXT) * 4 - 1; */
            bn_width += (((notch_end_next - notch_start_next) * 4) - 1);

            /* Care for the edge of BroadbandNotch */

            /* Left edge
             * If(IREG_L1C2_NOTCH_START_[n]_NEXT ==0 && IREG_L1C2_LEFT_EP_NEXT==L){
             *   bn_width += 1;
             * }
             */
            if ((notch_start_next == 0) && (!(pTuneSeq->ps[19] & 0x10))) {
                bn_width += 1;
            }

            /* Right edge
             * If(IREG_L1C2_NOTCH_END_[n]_NEXT ==852 &&
             * IREG_L1C2_RIGHT_EP_NEXT==L){
             *  bn_width += 1;
             * }
             */
            if ((notch_end_next == 852) && (!(pTuneSeq->ps[19] & 0x08))) {
                bn_width += 1;
            }
        }

        /* Change bitmask to check for IREG_L1C2_IS_BNOTCH_x_NEXT in next loop iteration */
        isBNotchNextBit >>= 1;
    }

    /* number_of_useful_carrier = 3409 - bn_width */
    number_of_useful_carrier = 3409 - bn_width;

    /* casDagcTarget = ( number_of_useful_carrier * 13 + 20000 + 50 ) / 100 (truncate) */
    pTuneSeq->casDagcTarget = (uint16_t) (number_of_useful_carrier * 13 + 20000 + 50) / 100;

    /* 3. Select CAS filter coefficients.
     *    Select CAS filter coefficients by number_of_useful_carrier
     * 0 < number_of_useful_carrier < 888
     * select SONY_DEMOD_DVBC2_FILTER1 (0MHz < Bandwidth of DS < 2MHz, (for 8MHz)
     *                 0MHz < Bandwidth of DS < 1.5MHz, (for 6MHz))
     * 888 < number_of_useful_carrier < 1776
     *      select SONY_DEMOD_DVBC2_FILTER2 (2MHz < Bandwidth of DS < 4MHz, (for 8MHz)
     *                    1.5MHz < Bandwidth of DS < 3MHz, (for 6MHz))
     * 1776 < number_of_useful_carrier < 2664
     *      select SONY_DEMOD_DVBC2_FILTER3 (4MHz < Bandwidth of DS < 6MHz, (for 8MHz)
     *                      3MHz < Bandwidth of DS < 4.5MHz, (for 6MHz))
     * 2664 <= number of useful carrier
     *      select SONY_DEMOD_DVBC2_FILTER4 (4MHz < Bandwidth of DS < 6MHz, (for 8MHz)
     *                      3MHz < Bandwidth of DS < 4.5MHz, (for 6MHz))
     */
    if (number_of_useful_carrier < 888) {
        pTuneSeq->filterSelect = SONY_DEMOD_DVBC2_FILTER1;
    }
    else if (number_of_useful_carrier < 1776) {
        pTuneSeq->filterSelect = SONY_DEMOD_DVBC2_FILTER2;
    }
    else if (number_of_useful_carrier < 2664) {
        pTuneSeq->filterSelect = SONY_DEMOD_DVBC2_FILTER3;
    }
    else {
        pTuneSeq->filterSelect = SONY_DEMOD_DVBC2_FILTER4;
    }

    SONY_TRACE_RETURN (result);
}

static sony_result_t CheckTuneDemodLock(sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    sony_demod_lock_result_t lockStatus = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("CheckTuneDemodLock");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->scanMode) {
        /* Tune part of scan process only requires demod lock */
        result = sony_demod_dvbc2_CheckDemodLock(pDemod, &lockStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
    }
    else {
        /* for non-scan tune, full TS lock is required */
        result = sony_demod_dvbc2_CheckTSLock(pDemod, &lockStatus);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
    }

    switch(lockStatus){
        case SONY_DEMOD_LOCK_RESULT_LOCKED:
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_CHANGE_FIRST_TRIAL;
            result = SONY_RESULT_OK;
            break;

        case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
            /* Demod lock failed */
            /* Finalize this iteration of the scan routine and provide an unlock callback */
            pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
            result = SONY_RESULT_ERROR_UNLOCK;
            break;

        case SONY_DEMOD_LOCK_RESULT_NOTDETECT:
            if (pTuneSeq->currentTime - pTuneSeq->loopStartTime < SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK) {
                /* Continue waiting (state is not changed.) */
                pTuneSeq->sleepTime = SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK_INTERVAL;
            }
            else {
                /* Demod lock timeout */
                /* Finalize this iteration of the scan routine and provide an unlock callback */
                pTuneSeq->state = SONY_DEMOD_DVBC2_TUNE_FINALIZE;
                result = SONY_RESULT_ERROR_TIMEOUT;
            }
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_HW_STATE);
    }

    SONY_TRACE_RETURN(result);
}

static sony_result_t GetTRLNominalRate(sony_demod_t *pDemod, sony_demod_dvbc2_tune_seq_t *pTuneSeq)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER("GetTRLNominalRate");

    if ((!pDemod) || (!pTuneSeq)){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* 1. Get TRL nominal rate
     * trcgNominalRate = IREG_TRL_CTLVAL_S
     */

    /* Set SLV-T Bank : 0x20 */
    if (pDemod->pI2c->WriteOneRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x00, 0x20) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }

    /* Slave    Bank    Addr     Bit        Name
     * ---------------------------------------------------------------
     * <SLV-T>   20h     52h     [6:0]      IREG_TRL_CTLVAL_S[38:32]
     * <SLV-T>   20h     53h     [7:0]      IREG_TRL_CTLVAL_S[31:24]
     * <SLV-T>   20h     54h     [7:0]      IREG_TRL_CTLVAL_S[23:16]
     * <SLV-T>   20h     55h     [7:0]      IREG_TRL_CTLVAL_S[15:8]
     * <SLV-T>   20h     56h     [7:0]      IREG_TRL_CTLVAL_S[7:0]
     */
    if (pDemod->pI2c->ReadRegister (pDemod->pI2c, pDemod->i2cAddressSLVT, 0x52, pTuneSeq->trcgNominalRate, sizeof (pTuneSeq->trcgNominalRate)) != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_I2C);
    }
    SONY_TRACE_RETURN(result);
}

static sony_result_t CalculateCarrierTunePosition(uint32_t *pTunePosition, uint32_t tuningFreqkHz, sony_dtv_bandwidth_t bandwidth)
{
    SONY_TRACE_ENTER("CalculateCarrierTunePosition");

    if (!pTunePosition){
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }

    /* OREG_CENTER_FREQ  = floor (([Tuning Freq (MHz)] * [Tu(us)] + 12/2) / 12) * 12   */
    if (bandwidth == SONY_DTV_BW_6_MHZ) {
        /* Tu(us) = 4096 * (7 / 48) = 1792 / 3 */
        *pTunePosition = ((tuningFreqkHz * 1792) / 3) / 1000;
    }
    else if (bandwidth == SONY_DTV_BW_8_MHZ) {
        /* Tu(us) = 4096 * (7 / 64) = 448 */
        *pTunePosition = (tuningFreqkHz * 448) / 1000;
    }
    else {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Round value to nearest valid carrier */
    *pTunePosition = ((*pTunePosition + 6) / 12) * 12;

    /* Sanity check that value does not overflow 24bits */
    if (*pTunePosition >= 0xFFFFFF) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}
