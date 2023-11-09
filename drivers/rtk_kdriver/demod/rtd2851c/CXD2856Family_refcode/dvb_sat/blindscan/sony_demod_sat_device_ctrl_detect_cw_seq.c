/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_common.h"
#include "sony_math.h"
#include "sony_demod.h"
#include "sony_demod_dvbs_s2.h"
#include "sony_demod_dvbs_s2_monitor.h"
#include "sony_demod_dvbs_s2_blindscan.h"
#include "sony_demod_dvbs_s2_blindscan_data.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"
#include "sony_demod_sat_device_ctrl_detect_cw_seq.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static int32_t calcPower(int32_t ifagcdb_x100, uint32_t power);
static const uint32_t stepKHz = 500;

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_sat_device_ctrl_detect_cw_seq_Initialize (sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq,
                                                                   sony_demod_t * pDemod,
                                                                   uint32_t freqKHz,
                                                                   uint32_t rangeKHz)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT();
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_detect_cw_seq_Initialize");

    if ((!pSeq) || (!pDemod)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isContinue = 1;
    pSeq->seqState = DETECT_CW_SEQ_STATE_START;
    pSeq->freqKHz = freqKHz;
    pSeq->rangeKHz = rangeKHz;
    pSeq->stepKHz = stepKHz;
    pSeq->currentOffsetFreqKHz = 0;
    pSeq->power = 0;

    pSeq->commonParams.pDemod = pDemod;
    pSeq->commonParams.detInfo.isDetect = 0;
    pSeq->commonParams.tuneReq.isRequest = 0;
    pSeq->commonParams.agcInfo.isRequest = 0;
    pSeq->commonParams.waitTime = 0;

    pSeq->commonParams.ckalFreqKHz = 64000; /* CKAL */
    pSeq->commonParams.ckahFreqKHz = 96000; /* CKAH */

    /* Initialize the data storage */
    result = sony_demod_dvbs_s2_blindscan_StorageInit (&(pSeq->commonParams.storage),
                                                       pSeq->powerArray,
                                                       sizeof(pSeq->powerArray)/sizeof(pSeq->powerArray[0]),
                                                       pSeq->dataArray,
                                                       sizeof(pSeq->dataArray)/sizeof(pSeq->dataArray[0]));
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* Initialize demodulator to start BlindScan. */
    result = sony_demod_dvbs_s2_blindscan_Initialize(pSeq->commonParams.pDemod);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    result = sony_demod_dvbs_s2_blindscan_subseq_pm_Initialize (&pSeq->subseqPM, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_detect_cw_seq_Sequence (sony_demod_sat_device_ctrl_detect_cw_seq_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_detect_cw_seq_Sequence");

    if (!pSeq){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pSeq->subseqPM.isEnable){
        /* PM seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence(&pSeq->subseqPM);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
    } else {
        /* Main sequence */
        switch(pSeq->seqState)
        {
        case DETECT_CW_SEQ_STATE_START:
            pSeq->commonParams.tuneReq.isRequest = 1;
            pSeq->commonParams.tuneReq.frequencyKHz = pSeq->freqKHz;
            pSeq->commonParams.tuneReq.symbolRateKSps = 45000;
            pSeq->commonParams.tuneReq.system = SONY_DTV_SYSTEM_DVBS;
            pSeq->commonParams.waitTime = 50;
            pSeq->seqState = DETECT_CW_SEQ_STATE_AGC_ON;
            break;

        case DETECT_CW_SEQ_STATE_AGC_ON:
            pSeq->commonParams.agcInfo.isRequest = 1;
            result = sony_demod_dvbs_s2_monitor_IFAGCOut(pSeq->commonParams.pDemod, &pSeq->commonParams.agcInfo.agcLevel);
            if (result != SONY_RESULT_OK) {
                return result;
            }
            pSeq->seqState = DETECT_CW_SEQ_STATE2;
            pSeq->commonParams.waitTime = 0;
            break;

        case DETECT_CW_SEQ_STATE2:
            pSeq->offsetFreqKHz = (int32_t)pSeq->freqKHz - (int32_t)pSeq->commonParams.tuneReq.frequencyKHz;
            result = sony_demod_dvbs_s2_blindscan_CS_INIT(pSeq->commonParams.pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->currentOffsetFreqKHz = (-1) * pSeq->rangeKHz;
            pSeq->seqState = DETECT_CW_SEQ_STATE3;
            pSeq->commonParams.waitTime = 0;
            break;

        case DETECT_CW_SEQ_STATE3:
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetFreqKHz + pSeq->currentOffsetFreqKHz);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = DETECT_CW_SEQ_STATE4;
            pSeq->commonParams.waitTime = 0;
            break;

        case DETECT_CW_SEQ_STATE4:
            if(pSeq->power < (int32_t)pSeq->subseqPM.power){
                pSeq->power = (int32_t)pSeq->subseqPM.power;
            }
            pSeq->currentOffsetFreqKHz += (int32_t)pSeq->stepKHz;
            if(pSeq->currentOffsetFreqKHz < (int32_t)pSeq->rangeKHz){
                /* Next trial */
                pSeq->seqState = DETECT_CW_SEQ_STATE3;
                pSeq->commonParams.waitTime = 0;
            } else {
                /* Reach to the end */
                pSeq->seqState = DETECT_CW_SEQ_STATE5;
                pSeq->commonParams.waitTime = 0;
            }
            break;

        case DETECT_CW_SEQ_STATE5:
            pSeq->power = calcPower(pSeq->commonParams.agcInfo.agc_x100dB, (uint32_t)pSeq->power);
            pSeq->isContinue = 0;
            break;

        default:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
        }
    }

    SONY_TRACE_RETURN (result);
}

static int32_t calcPower(int32_t ifagcdb_x100, uint32_t power)
{
    return (int32_t)(((int32_t)sony_math_log10 (power) - 421) * 10) - ifagcdb_x100;
}
