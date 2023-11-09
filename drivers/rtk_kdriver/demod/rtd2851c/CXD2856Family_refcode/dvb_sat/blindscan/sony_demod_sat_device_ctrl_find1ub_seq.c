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
#include "sony_demod_dvbs_s2_blindscan_subseq_ss.h"
#include "sony_demod_dvbs_s2_blindscan_subseq_pm.h"
#include "sony_demod_dvbs_s2_blindscan_utility.h"
#include "sony_demod_dvbs_s2_blindscan_trace.h"
#include "sony_demod_sat_device_ctrl_find1ub_seq.h"

/*----------------------------------------------------------------------------
  Static Functions
----------------------------------------------------------------------------*/
static int32_t calcPower(int32_t ifagcdb_x100, uint32_t power);

/*----------------------------------------------------------------------------
  Functions
----------------------------------------------------------------------------*/
sony_result_t sony_demod_sat_device_ctrl_find1ub_seq_Initialize (sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq,
                                                                 sony_demod_t * pDemod,
                                                                 uint32_t minFreqKHz,
                                                                 uint32_t maxFreqKHz,
                                                                 uint8_t targetUB)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_INIT();
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_find1ub_seq_Initialize");

    if ((!pSeq) || (!pDemod)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    pSeq->isContinue = 1;
    pSeq->seqState = FIND1UB_SEQ_STATE_START;
    pSeq->minFreqKHz = minFreqKHz;
    pSeq->maxFreqKHz = maxFreqKHz;
    pSeq->minPowerFreqKHz = minFreqKHz - 5000;
    pSeq->maxPowerFreqKHz = maxFreqKHz + 5000;
    pSeq->targetUB = targetUB;
    pSeq->commandReq = 0;

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

    result = sony_demod_dvbs_s2_blindscan_subseq_ss_Initialize (&pSeq->subseqSS, &pSeq->commonParams);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    SONY_TRACE_RETURN (SONY_RESULT_OK);
}

sony_result_t sony_demod_sat_device_ctrl_find1ub_seq_Sequence (sony_demod_sat_device_ctrl_find1ub_seq_t * pSeq)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbs_s2_blindscan_power_t * pPower;
    SONY_TRACE_ENTER ("sony_demod_sat_device_ctrl_find1ub_seq_Sequence");

    if (!pSeq){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pSeq->subseqSS.isEnable){
        /* SS seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_ss_Sequence (&pSeq->subseqSS);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
    } else if (pSeq->subseqPM.isEnable){
        /* PM seq */
        result = sony_demod_dvbs_s2_blindscan_subseq_pm_Sequence(&pSeq->subseqPM);
        if (result != SONY_RESULT_OK){
            SONY_TRACE_RETURN(result);
        }
    } else {
        /* Main sequence */
        switch(pSeq->seqState)
        {
        case FIND1UB_SEQ_STATE_START:
            /* ODU_UbxSignal_ON */
            result = sony_singlecable_command_ODU_UbxSignal_ON (&pSeq->message, SONY_SINGLECABLE_ADDRESS_ALL_DEVICES);
            if (result != SONY_RESULT_OK) {return result;}
            pSeq->commandReq = 1;
            pSeq->commonParams.waitTime = SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_ODU_TX_WAIT;
            pSeq->seqState = FIND1UB_SEQ_STATE_SS_START;
            break;

        case FIND1UB_SEQ_STATE_SS_START:
            /* Get power spectrum */
            result = sony_demod_dvbs_s2_blindscan_subseq_ss_Start(&pSeq->subseqSS,
                                                                  pSeq->minPowerFreqKHz,
                                                                  pSeq->maxPowerFreqKHz,
                                                                  500,
                                                                  20000,
                                                                  1);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN(result);
            }
            pSeq->commonParams.waitTime = 0;
            pSeq->seqState = FIND1UB_SEQ_STATE_SS_FIN;
            break;

        case FIND1UB_SEQ_STATE_SS_FIN:
            SONY_DEMOD_DVBS_S2_BLINDSCAN_TRACE_POWERLIST("Power", pSeq->subseqSS.pPowerList);
            pPower = pSeq->subseqSS.pPowerList->pNext;
            pSeq->pmax_fn = 0;
            while(pPower){
                if(pPower->freqKHz > 0){
                    /* Valid data */
                    if((pSeq->pmax_fn == 0) || (pSeq->pmax_p1 < pPower->power)){
                        pSeq->pmax_fn = pPower->freqKHz;
                        pSeq->pmax_p1 = pPower->power;
                    }
                }
                pPower = pPower->pNext;
            }
            pSeq->loopCount = 0;
            /* Fall through */
        case FIND1UB_SEQ_STATE_TRIAL1:
            pPower = pSeq->subseqSS.pPowerList->pNext;
            pSeq->pmax_fn = 0;
            pSeq->pmax_pn = 0;
            while(pPower){
                if(pPower->freqKHz > 0){
                    /* Valid data */
                    if((pSeq->pmax_fn == 0) || (pSeq->pmax_pn < pPower->power)){
                        pSeq->pmax_fn = pPower->freqKHz;
                        pSeq->pmax_pn = pPower->power;
                    }
                }
                pPower = pPower->pNext;
            }
            /* ODU_UbxSignal_ON */
            result = sony_singlecable_command_ODU_UbxSignal_ON (&pSeq->message, SONY_SINGLECABLE_ADDRESS_ALL_DEVICES);
            if (result != SONY_RESULT_OK) {
                return result;
            }
            pSeq->commandReq = 1;
            pSeq->commonParams.waitTime = SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_ODU_TX_WAIT;
            pSeq->seqState = FIND1UB_SEQ_STATE_TUNING_ON;
            break;

        case FIND1UB_SEQ_STATE_TUNING_ON:
            pSeq->commonParams.tuneReq.isRequest = 1;
            pSeq->commonParams.tuneReq.frequencyKHz = pSeq->pmax_fn;
            pSeq->commonParams.tuneReq.symbolRateKSps = 45000;
            pSeq->commonParams.tuneReq.system = SONY_DTV_SYSTEM_DVBS;
            pSeq->commonParams.waitTime = 50;
            pSeq->seqState = FIND1UB_SEQ_STATE_AGC_ON;
            break;

        case FIND1UB_SEQ_STATE_AGC_ON:
            pSeq->commonParams.agcInfo.isRequest = 1;
            result = sony_demod_dvbs_s2_monitor_IFAGCOut(pSeq->commonParams.pDemod, &pSeq->commonParams.agcInfo.agcLevel);
            if (result != SONY_RESULT_OK) {
                return result;
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL2;
            break;

        case FIND1UB_SEQ_STATE_TRIAL2:
            pSeq->offsetKHz = pSeq->pmax_fn - pSeq->commonParams.tuneReq.frequencyKHz;
            if((pSeq->offsetKHz < -2000) || (pSeq->offsetKHz > 2000)){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
            }
            result = sony_demod_dvbs_s2_blindscan_CS_INIT(pSeq->commonParams.pDemod);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL3;
            break;

        case FIND1UB_SEQ_STATE_TRIAL3:
            pSeq->powerOn = pSeq->subseqPM.power;
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz - 500);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL4;
            break;

        case FIND1UB_SEQ_STATE_TRIAL4:
            if(pSeq->powerOn < pSeq->subseqPM.power){
                pSeq->powerOn = pSeq->subseqPM.power;
            }
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz + 500);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL5;
            break;

        case FIND1UB_SEQ_STATE_TRIAL5:
            if(pSeq->powerOn < pSeq->subseqPM.power){
                pSeq->powerOn = pSeq->subseqPM.power;
            }
            pSeq->powerOn = calcPower(pSeq->commonParams.agcInfo.agc_x100dB, (uint32_t)pSeq->powerOn);
            result = sony_singlecable_command_ODU_PowerOFF (&pSeq->message, SONY_SINGLECABLE_ADDRESS_ALL_DEVICES, pSeq->targetUB);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->commandReq = 1;
            pSeq->commonParams.waitTime = SONY_DEMOD_SAT_DEVICE_CTRL_FIND1UB_ODU_TX_WAIT;
            pSeq->seqState = FIND1UB_SEQ_STATE_TUNING_OFF;
            break;

        case FIND1UB_SEQ_STATE_TUNING_OFF:
            pSeq->commonParams.tuneReq.isRequest = 1;
            pSeq->commonParams.tuneReq.frequencyKHz = pSeq->pmax_fn;
            pSeq->commonParams.tuneReq.symbolRateKSps = 45000;
            pSeq->commonParams.tuneReq.system = SONY_DTV_SYSTEM_DVBS;
            pSeq->commonParams.waitTime = 50;
            pSeq->seqState = FIND1UB_SEQ_STATE_AGC_OFF;
            break;

        case FIND1UB_SEQ_STATE_AGC_OFF:
            pSeq->commonParams.agcInfo.isRequest = 1;
            result = sony_demod_dvbs_s2_monitor_IFAGCOut(pSeq->commonParams.pDemod, &pSeq->commonParams.agcInfo.agcLevel);
            if (result != SONY_RESULT_OK) {
                return result;
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL6;
            break;

        case FIND1UB_SEQ_STATE_TRIAL6:
            pSeq->offsetKHz = pSeq->pmax_fn - pSeq->commonParams.tuneReq.frequencyKHz;
            if((pSeq->offsetKHz < -2000) || (pSeq->offsetKHz > 2000)){
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
            }
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL7;
            break;

        case FIND1UB_SEQ_STATE_TRIAL7:
            pSeq->powerOff = pSeq->subseqPM.power;
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz - 500);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL8;
            break;

        case FIND1UB_SEQ_STATE_TRIAL8:
            if(pSeq->powerOff < pSeq->subseqPM.power){
                pSeq->powerOff = pSeq->subseqPM.power;
            }
            result = sony_demod_dvbs_s2_blindscan_subseq_pm_Start (&pSeq->subseqPM, pSeq->offsetKHz + 500);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL9;
            break;

        case FIND1UB_SEQ_STATE_TRIAL9:
            if(pSeq->powerOff < pSeq->subseqPM.power){
                pSeq->powerOff = pSeq->subseqPM.power;
            }
            pSeq->powerOff = calcPower(pSeq->commonParams.agcInfo.agc_x100dB, (uint32_t)pSeq->powerOff);
            if((pSeq->powerOn - pSeq->powerOff) > 1000){
                /* Detect user band frequency */
                pSeq->ubFreqKHz = pSeq->pmax_fn;

                /* Go to Finish State. */
                pSeq->ubNumber = 1;
                pSeq->seqState = FIND1UB_SEQ_STATE_FINISH;
            } else {
                /* Remove the data in +/- 2MHz area as spurious signal. */
                pPower = pSeq->subseqSS.pPowerList->pNext;
                while(pPower){
                    if((pPower->freqKHz > 0) && ((pSeq->pmax_fn - 2000) < (pPower->freqKHz)) && ((pPower->freqKHz) < (pSeq->pmax_fn + 2000))){
                        pPower->freqKHz = 0;
                    }
                    pPower = pPower->pNext;
                }
                /* Continue check */
                if((pSeq->pmax_p1 - pSeq->pmax_pn) > 3000){
                    pSeq->ubFreqKHz = 0;

                    /* Go to Finish State. */
                    pSeq->ubNumber = 1;
                    pSeq->seqState = FIND1UB_SEQ_STATE_FINISH;
                } else if(pSeq->loopCount++ > 30){
                    pSeq->ubFreqKHz = 0;

                    /* Go to Finish State. */
                    pSeq->ubNumber = 1;
                    pSeq->seqState = FIND1UB_SEQ_STATE_FINISH;
                } else {
                    /* Go to next trial */
                    pSeq->seqState = FIND1UB_SEQ_STATE_TRIAL1;
                }
            }
            break;

        case FIND1UB_SEQ_STATE_FINISH:
            /* Turning OFF the Power of all User bands as they were turned ON during start of algorithm. */

            result = sony_singlecable_command_ODU_PowerOFF (&pSeq->message,
                                                            SONY_SINGLECABLE_ADDRESS_ALL_DEVICES,
                                                            pSeq->ubNumber);
            if (result != SONY_RESULT_OK){
                SONY_TRACE_RETURN (result);
            }
            pSeq->commandReq = 1;

            if (pSeq->ubNumber == 8) {
                /* Last user band so exit this algorithm too. */
                pSeq->isContinue = 0;
            }
            /* Increment to Turn OFF the next userband in next cycle. */
            pSeq->ubNumber ++;
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
