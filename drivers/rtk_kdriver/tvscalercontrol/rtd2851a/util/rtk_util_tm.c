#include <rtk_dc_mt.h>                      // usage: rtd_printk
#include <linux/vmalloc.h>
#include <util/rtk_util_tm.h>
#include <tvscalercontrol/io/ioregdrv.h>    // for IoReg_Read
#include <rbus/dma_vgip_reg.h>
#include <rbus/vgip_reg.h>

#define tm_printk(fmt)          //rtd_printk(KERN_INFO, "TM_INTERNAL", fmt)

static unsigned int gLatestTimeStampToDumpLineInfo = 0;
static bool gbLineInfoTitleNotDumpedYet = true;
void rtk_util_tm_dump_line_info(const unsigned int cnt, const char *comment)
{
    unsigned int currentTime = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
    unsigned int timeElapsed;

    if(gLatestTimeStampToDumpLineInfo == 0) {
        timeElapsed = 0;
    } else {
        timeElapsed = ((currentTime - gLatestTimeStampToDumpLineInfo) * 1000) / 90;     // convert to us
    }

    if(gbLineInfoTitleNotDumpedYet) {
        rtd_printk(KERN_INFO, RUTIL_TM_LINE_INFO_TAG, " , comment,"
                                                        " cnt,"
                                                        " currentTime(us),"
                                                        " VO line cnt (Vstart,end,total),"
                                                        " VGIP line cnt ch1(dma),"
                                                        " MTG line cnt,"
                                                        " UZU line cnt (Vstart,end,total),"
                                                        " MEMC line cnt (Vstart,end,total),"
                                                        " frame cnt (dtg input, output)(uzu input, output)(memc input, output)-(dtg multiple, remove)(uzu multiple, remove)(memc multiple, remove),"
                                                        " i2d frame idx (phase err),"
                                                        " i2d measure (8 frames)");

      gbLineInfoTitleNotDumpedYet = false;
    }

    rtd_printk(KERN_INFO, RUTIL_TM_LINE_INFO_TAG, " ,%s,"
                                                " %4d,"
                                                " %8d,"
                                                " %4d(%2d,%4d,%4d),"
                                                " %4d(%4d),"
                                                " %4d,"
                                                " %4d(%2d,%4d,%4d),"
                                                " %4d(%2d,%4d,%4d),"
                                                " (%1d,%1d)(%1d,%1d)(%1d,%1d)-(%1d,%1d)(%1d,%1d)(%1d,%1d),"
                                                " %1d(%4d),"
                                                " %4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d",
        comment,
        cnt,
        (currentTime * 1000) / 90,

        // VO
        VODMA_VODMA_LINE_ST_get_line_cnt(IoReg_Read32(VODMA_VODMA_LINE_ST_reg)),
            VODMA_VODMA_V1VGIP_VACT1_get_v_st(IoReg_Read32(VODMA_VODMA_V1VGIP_VACT1_reg)),
            VODMA_VODMA_V1VGIP_VACT1_get_v_end(IoReg_Read32(VODMA_VODMA_V1VGIP_VACT1_reg)),
            VODMA_VODMA_V1SGEN_get_v_thr(IoReg_Read32(VODMA_VODMA_V1SGEN_reg)),

        // VGIP
        VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg)),
            DMA_VGIP_DMA_VGIP_LC_get_dma_line_cnt(IoReg_Read32(DMA_VGIP_DMA_VGIP_LC_reg)),

        // MTG
        PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg)),

        // UZU after delay
        PPOVERLAY_new_meas2_linecnt_real_get_uzudtg_dly_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg)),
            PPOVERLAY_MAIN_DEN_V_Start_End_get_mv_den_sta(IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg)),
            PPOVERLAY_MAIN_DEN_V_Start_End_get_mv_den_end(IoReg_Read32(PPOVERLAY_MAIN_DEN_V_Start_End_reg)),
            PPOVERLAY_uzudtg_DV_TOTAL_get_uzudtg_dv_total(IoReg_Read32(PPOVERLAY_uzudtg_DV_TOTAL_reg)),

        // MEMC
        PPOVERLAY_new_meas1_linecnt_real_get_memcdtg_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg)),
            PPOVERLAY_memcdtg_DV_DEN_Start_End_get_memcdtg_dv_den_sta(IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg)),
            PPOVERLAY_memcdtg_DV_DEN_Start_End_get_memcdtg_dv_den_end(IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg)),
            PPOVERLAY_memcdtg_DV_TOTAL_get_memcdtg_dv_total(IoReg_Read32(PPOVERLAY_memcdtg_DV_TOTAL_reg)),

        // frame cnt dtg
        PPOVERLAY_DTG_FRAME_CNT3_get_dtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT3_reg)),
            PPOVERLAY_DTG_FRAME_CNT2_get_dtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT2_reg)),

        // frame cnt uzu
        PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
            PPOVERLAY_DTG_FRAME_CNT1_get_uzudtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),

        // frame cut memc
        PPOVERLAY_DTG_FRAME_CNT1_memcdtg_input_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),
            PPOVERLAY_DTG_FRAME_CNT1_memcdtg_output_frame_cnt(IoReg_Read32(PPOVERLAY_DTG_FRAME_CNT1_reg)),

        // multiple/remove dtg
        PPOVERLAY_DTG_M_multiple_vsync_get_dtg_m_multiple_vsync(IoReg_Read32(PPOVERLAY_DTG_M_multiple_vsync_reg)),
            PPOVERLAY_DTG_M_Remove_input_vsync_get_remove_half_ivs_mode2(IoReg_Read32(PPOVERLAY_DTG_M_Remove_input_vsync_reg)),

        // multiple/remove uzu
        PPOVERLAY_uzudtg_fractional_fsync_get_uzudtg_multiple_vsync(IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg)),
            PPOVERLAY_uzudtg_fractional_fsync_get_uzudtg_remove_half_ivs_mode(IoReg_Read32(PPOVERLAY_uzudtg_fractional_fsync_reg)),

        // multiple/remove memc
        PPOVERLAY_memcdtg_fractional_fsync_get_memcdtg_multiple_vsync(IoReg_Read32(PPOVERLAY_memcdtg_fractional_fsync_reg)),
            PPOVERLAY_memcdtg_fractional_fsync_get_memcdtg_remove_half_ivs_mode(IoReg_Read32(PPOVERLAY_memcdtg_fractional_fsync_reg)),

        // i2d idx and phase err
        PPOVERLAY_I2D_MEAS_phase_get_i2d_frame_index(IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg)),
            PPOVERLAY_I2D_MEAS_phase_get_i2d_phase_err(IoReg_Read32(PPOVERLAY_I2D_MEAS_phase_reg)),
            PPOVERLAY_I2D_MEAS_0_get_i2d_tune_res0(IoReg_Read32(PPOVERLAY_I2D_MEAS_0_reg)),
            PPOVERLAY_I2D_MEAS_0_get_i2d_tune_res1(IoReg_Read32(PPOVERLAY_I2D_MEAS_0_reg)),
            PPOVERLAY_I2D_MEAS_1_get_i2d_tune_res2(IoReg_Read32(PPOVERLAY_I2D_MEAS_1_reg)),
            PPOVERLAY_I2D_MEAS_1_get_i2d_tune_res3(IoReg_Read32(PPOVERLAY_I2D_MEAS_1_reg)),
            PPOVERLAY_I2D_MEAS_2_get_i2d_tune_res4(IoReg_Read32(PPOVERLAY_I2D_MEAS_2_reg)),
            PPOVERLAY_I2D_MEAS_2_get_i2d_tune_res5(IoReg_Read32(PPOVERLAY_I2D_MEAS_2_reg)),
            PPOVERLAY_I2D_MEAS_3_get_i2d_tune_res6(IoReg_Read32(PPOVERLAY_I2D_MEAS_3_reg)),
            PPOVERLAY_I2D_MEAS_3_get_i2d_tune_res7(IoReg_Read32(PPOVERLAY_I2D_MEAS_3_reg))
    );

    gLatestTimeStampToDumpLineInfo = currentTime;
}


// for how to dump the TM result
#define TM_LOG_TITLE_1          ",Topic #,"                                             \
                                    "Round ID,"                                         \
                                    "Execution Flow,"                                   \
                                    "# V CSW (context switched by ourself),"            \
                                    "# IV CSW,"

#define TM_LOG_FMT_1            ",%3d,"                 \
                                    "%3d,"              \
                                    "%4d,"              \
                                    "%6lu,"             \
                                    "%6lu,"

#ifdef CONFIG_SCHED_INFO
    #define TM_LOG_TITLE_2      TM_LOG_TITLE_1          \
                                    "run_delay (us),"

    #define TM_LOG_FMT_2        TM_LOG_FMT_1            \
                                    "%7lu,"
#else
    #define TM_LOG_TITLE_2      TM_LOG_TITLE_1
    #define TM_LOG_FMT_2        TM_LOG_FMT_1
#endif

#define TM_LOG_TITLE            TM_LOG_TITLE_2                          \
                                    "Kthread ID,"                       \
                                    "Duration (us) in topic,"           \
                                    "Duration (us) in thread,"          \
                                    "Time Stamp (us)),"                 \
                                    "Comment,"                          \
                                    "Function Name,"                    \
                                    "Line Number,"                      \
                                    "Log overhead (us)"

#define TM_LOG_FMT              TM_LOG_FMT_2                            \
                                    " %4d,"                             \
                                    "%7d,"                              \
                                    "%7d,"                              \
                                    "%8d,"                              \
                                    " %s,"                              \
                                    " %s,"                              \
                                    "%8d,"                              \
                                    "%2d"


stRUTIL_TM_TOPICS gstRUTIL_TM_Topics[eRUtilTM_Topic_Max];      // preset to all zero

bool rtk_util_tm_init_topic(const unsigned char *nameOfTopic, const E_RUTIL_TM_TOPICS eTopic, const unsigned char numOfThreads, const unsigned short numOfSteps)
{
    unsigned short maxSteps;
    unsigned char maxThreads;

    if(NULL != gstRUTIL_TM_Topics[eTopic].pstStepLogHead) {
        // initialized before
        return false;
    }

#if CONFIG_RUTIL_TM_LOG_START_END_ONLY
    maxSteps = 2;
    maxThreads = ( (numOfThreads >= 2) ? 2 : numOfThreads );
#else
    maxSteps = numOfSteps;
    maxThreads = numOfThreads;
#endif

    // allocate space
    gstRUTIL_TM_Topics[eTopic].pstStepLogHead = (stRUTIL_TM_STEP_LOG *) vzalloc(sizeof(stRUTIL_TM_STEP_LOG) * maxSteps);

    if(NULL == gstRUTIL_TM_Topics[eTopic].pstStepLogHead) {
        return false;
    } else {
        gstRUTIL_TM_Topics[eTopic].pstThreadInfo = (stRUTIL_TM_TOPIC_THREAD_INFO *) vzalloc(sizeof(stRUTIL_TM_TOPIC_THREAD_INFO) * maxThreads);

        if(NULL == gstRUTIL_TM_Topics[eTopic].pstThreadInfo) {
            vfree(gstRUTIL_TM_Topics[eTopic].pstStepLogHead);
            gstRUTIL_TM_Topics[eTopic].pstStepLogHead = NULL;
            return false;
        }

        gstRUTIL_TM_Topics[eTopic].nameOfTopic = nameOfTopic;
        gstRUTIL_TM_Topics[eTopic].numOfThreads = maxThreads;
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent = gstRUTIL_TM_Topics[eTopic].pstStepLogHead;
        gstRUTIL_TM_Topics[eTopic].pstStepLogMax = &(gstRUTIL_TM_Topics[eTopic].pstStepLogHead[maxSteps]);

        gstRUTIL_TM_Topics[eTopic].go = false;
        gstRUTIL_TM_Topics[eTopic].runID = 0;

        gstRUTIL_TM_Topics[eTopic].totalElapsedInUS = 0;
        gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS = 0;

        return true;
    }
}

void rtk_util_tm_module_init(void)
{
    unsigned int i;

    for(i=0; i<eRUtilTM_Topic_Max; i++) {
        gstRUTIL_TM_Topics[i].pstStepLogHead = NULL;
        gstRUTIL_TM_Topics[i].go = false;
    }
}

static bool rtk_util_tm_exit_topic(const E_RUTIL_TM_TOPICS eTopic)
{
    if(NULL == gstRUTIL_TM_Topics[eTopic].pstStepLogHead) {
        return false;
    } else {
        if(NULL != gstRUTIL_TM_Topics[eTopic].pstThreadInfo) {
            vfree(gstRUTIL_TM_Topics[eTopic].pstThreadInfo);
        }

        vfree(gstRUTIL_TM_Topics[eTopic].pstStepLogHead);

        gstRUTIL_TM_Topics[eTopic].numOfThreads = 0;
        gstRUTIL_TM_Topics[eTopic].pstThreadInfo = NULL;

        gstRUTIL_TM_Topics[eTopic].pstStepLogHead = NULL;
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent = NULL;
        gstRUTIL_TM_Topics[eTopic].pstStepLogMax = NULL;

        gstRUTIL_TM_Topics[eTopic].go = false;
        gstRUTIL_TM_Topics[eTopic].runID = 0;
        return true;
    }
}

bool rtk_util_tm_module_exit(void)
{
    unsigned int i;
    bool ret = true;

    for(i=0; i<eRUtilTM_Topic_Max; i++) {
        if(rtk_util_tm_exit_topic(i)) {
            // do nothing
        } else {
            // failed
            ret = false;
        }
    }

    return ret;
}

bool rtk_util_tm_start(const E_RUTIL_TM_TOPICS eTopic)
{
    if(NULL == gstRUTIL_TM_Topics[eTopic].pstStepLogHead) {
        tm_printk("start return false");
        return false;
    } else {
        gstRUTIL_TM_Topics[eTopic].go = true;

    #if 1
        // Prepare for next rtk_util_tm_start() and rtk_util_tm_dump_line_info()
        // If you put those codes to utk_util_tm_start(), you'll miss one case:
        //    The code flow called rtk_util_tm_start() twice before called rtk_util_tm_end_and_dump()
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent = gstRUTIL_TM_Topics[eTopic].pstStepLogHead;
        gstRUTIL_TM_Topics[eTopic].runID++;
        memset(gstRUTIL_TM_Topics[eTopic].pstThreadInfo, 0, sizeof(stRUTIL_TM_TOPIC_THREAD_INFO) * gstRUTIL_TM_Topics[eTopic].numOfThreads);
    #endif

        tm_printk("start return true");
        return true;
    }
}

static bool gbTMTitleNotDumpedYet = true;
static bool gbTM_SummaryTitleNotDumpedYet = true;
bool rtk_util_tm_end_and_dump(const E_RUTIL_TM_TOPICS eTopic, const bool bDumpToConsole)
{
    unsigned char threadID;
    unsigned int latestTimeStampInThread = 0;
    unsigned int latestTimeStampOfExecutionFlow = 0;
    unsigned long long latestRunDelayInThread = 0;
    stRUTIL_TM_STEP_LOG *pstStepLog;

    unsigned long currentRunDelayInUS = 0;
    unsigned short execute_steps = 0;

    // stop RUTIL_TM_LOG_INTERNAL()
    gstRUTIL_TM_Topics[eTopic].go = false;

    if((NULL == gstRUTIL_TM_Topics[eTopic].pstStepLogHead)
       || (gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent == gstRUTIL_TM_Topics[eTopic].pstStepLogHead)) {

        // not initialized or not start or nothing logged
        tm_printk("end return false");
        return false;
    } else {
        // dump title, later you can find all [TM] and import to excel then do pivot tables analysis
        if(gbTMTitleNotDumpedYet) {
            if(bDumpToConsole) {
                rtd_printk(KERN_INFO, RUTIL_TM_INFO_TAG, TM_LOG_TITLE);
            }

            gbTMTitleNotDumpedYet = false;
        }

        // point to head of step logs
        pstStepLog = gstRUTIL_TM_Topics[eTopic].pstStepLogHead;
        latestTimeStampOfExecutionFlow = pstStepLog->timeStamp;

        // start to dump time elapsed info of each step
        while(pstStepLog != gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent) {
            // find the corresponding thread records
            for(threadID = 0; threadID < gstRUTIL_TM_Topics[eTopic].numOfThreads; threadID++) {
                if(gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].tid == 0) {
                    // not found this tid before, add a new one
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].tid               = pstStepLog->tid;
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestTimeStamp   = pstStepLog->timeStamp;        // we want the "duration in thread" of 1st time_measure_log() of each thread to be zero
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestRunDelay    = pstStepLog->run_delay;        // we want the "run_delay in thread" of 1st time_measure_log() of each thread to be zero

                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNVSCW       = pstStepLog->nvcsw;
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNIVSCW      = pstStepLog->nivcsw;

                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNVCSW        = 0;
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNIVCSW       = 0;
                    break;
                } else if (gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].tid == pstStepLog->tid) {
                    // found
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNVCSW        += pstStepLog->nvcsw - gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNVSCW;
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNVSCW       = pstStepLog->nvcsw;

                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNIVCSW       += pstStepLog->nivcsw - gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNIVSCW;
                    gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestNIVSCW      = pstStepLog->nivcsw;
                    break;
                }
            }

            if(threadID >= gstRUTIL_TM_Topics[eTopic].numOfThreads) {
                if(bDumpToConsole) {
                    rtd_printk(KERN_INFO, RUTIL_TM_INFO_TAG, "Exceed the max threads you specified for topic %d, please increase it.", eTopic);
                }
            }

            latestTimeStampInThread = gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestTimeStamp;
            latestRunDelayInThread  = gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestRunDelay;

            currentRunDelayInUS = (unsigned long) (pstStepLog->run_delay - latestRunDelayInThread) / 1000;                  // convert to unsigned long then ns to us, maybe data loss
            gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalRunDelayInUS += currentRunDelayInUS;

            if(bDumpToConsole) {
                // 1. us time representation. if you want to use by ms, please remove "* 1000"
                // 2. to save those logs to xxxx.csv, then read from excel, seperator are ',' and ':'
                rtd_printk(KERN_INFO, RUTIL_TM_INFO_TAG, TM_LOG_FMT
                    , eTopic
                    , gstRUTIL_TM_Topics[eTopic].runID
                    , execute_steps++
                    , pstStepLog->nvcsw
                    , pstStepLog->nivcsw
                    , currentRunDelayInUS
                    , pstStepLog->tid
                    , ((pstStepLog->timeStamp - latestTimeStampOfExecutionFlow) * 1000)/90                                      // period in execution flow
                    , ((pstStepLog->timeStamp - latestTimeStampInThread) * 1000)/90                                             // period in thread
                    , ((pstStepLog->timeStamp * 1000) / 90) & 0xFFFFF                                                           // we don't want all digits for easy to read
                    , pstStepLog->comment
                    , pstStepLog->funcName
                    , pstStepLog->linenum
                    , ((pstStepLog->logStart - pstStepLog->timeStamp) * 1000)/90                                                // time measure overhead
                );
            }

            // update latest time stamps
            latestTimeStampOfExecutionFlow = pstStepLog->logStart;                                                          // do not count in RUTIL_TM_LOG_INTERNAL() overhead
            gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestTimeStamp = pstStepLog->logStart;                   // do not count in RUTIL_TM_LOG_INTERNAL() overhead
            gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].latestRunDelay = pstStepLog->run_delay;

            pstStepLog++;
        }   // end of while, dump each step

        // dump topic total time, Total Elapsed, Run_Delay, (Total Elapsed - Run_Delay)
        pstStepLog--;       // move to latest log

        gstRUTIL_TM_Topics[eTopic].totalElapsedInUS = ((pstStepLog->timeStamp - gstRUTIL_TM_Topics[eTopic].pstStepLogHead->timeStamp) * 1000) / 90;

        gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS = 0;
        gstRUTIL_TM_Topics[eTopic].totalNVCSW        = 0;
        gstRUTIL_TM_Topics[eTopic].totalNIVCSW       = 0;
        for(threadID = 0; threadID < gstRUTIL_TM_Topics[eTopic].numOfThreads; threadID++) {
            gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS += gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalRunDelayInUS;
            gstRUTIL_TM_Topics[eTopic].totalNVCSW        += gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNVCSW;
            gstRUTIL_TM_Topics[eTopic].totalNIVCSW       += gstRUTIL_TM_Topics[eTopic].pstThreadInfo[threadID].totalNIVCSW;
        }

        // always dump summary
        if(gbTM_SummaryTitleNotDumpedYet) {
            rtd_printk(KERN_INFO, RUTIL_TM_INFO_SUMMARY_TAG, " , NameOfTopic, TopicID, Round, NVCSW, NIVCSW, Run_Delay,  Actual,  Elapsed");
            gbTM_SummaryTitleNotDumpedYet = false;
        }

        if(1) {     //(bDumpToConsole) {
            rtd_printk(KERN_INFO, RUTIL_TM_INFO_SUMMARY_TAG, " , [%s], %3d, %3d, %8d, %8d, %8d, %8d, %8d",
                gstRUTIL_TM_Topics[eTopic].nameOfTopic,
                eTopic,
                gstRUTIL_TM_Topics[eTopic].runID,
                gstRUTIL_TM_Topics[eTopic].totalNVCSW,
                gstRUTIL_TM_Topics[eTopic].totalNIVCSW,
                gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS,
                gstRUTIL_TM_Topics[eTopic].totalElapsedInUS - gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS,
                gstRUTIL_TM_Topics[eTopic].totalElapsedInUS);
        }

        tm_printk("end return true");

    #if 0
        // Prepare for next rtk_util_tm_start() and rtk_util_tm_dump_line_info()
        // If you put those codes to utk_util_tm_start(), you'll miss one case:
        //    The code flow called rtk_util_tm_start() twice before called rtk_util_tm_end_and_dump()
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent = gstRUTIL_TM_Topics[eTopic].pstStepLogHead;
        gstRUTIL_TM_Topics[eTopic].runID++;
        memset(gstRUTIL_TM_Topics[eTopic].pstThreadInfo, 0, sizeof(stRUTIL_TM_TOPIC_THREAD_INFO) * gstRUTIL_TM_Topics[eTopic].numOfThreads);
    #endif

        gLatestTimeStampToDumpLineInfo = 0;

        return true;
    }   // end of dump info
}

unsigned long rtk_util_tm_get_result_in_us(const E_RUTIL_TM_TOPICS eTopic, const E_RUTIL_TM_RESULTS eWhichResult)
{
     tm_printk("result");
     if(NULL == gstRUTIL_TM_Topics[eTopic].pstStepLogHead) {
        tm_printk("result return false");
        return 0xFFFFFFFF;      // return a very big value to notice upper layer app it's wrong.
    } else {
        switch (eWhichResult) {
            case eRUtilTM_Results_totalElapsed:
                return gstRUTIL_TM_Topics[eTopic].totalElapsedInUS;

            case eRUtilTM_Results_TotalRunDelay:
                return gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS;

            case eRUtilTM_Results_TotalActual:
                return (gstRUTIL_TM_Topics[eTopic].totalElapsedInUS - gstRUTIL_TM_Topics[eTopic].totalRunDelayInUS);

            case eRUtilTM_Results_TotalNVCSW:
                return gstRUTIL_TM_Topics[eTopic].totalNVCSW;

            case eRUtilTM_Results_TotalNIVCSW:
                return gstRUTIL_TM_Topics[eTopic].totalNIVCSW;

            default:
                return 0;
        }
    }
}

bool rtk_util_tm_verify_result_in_us(const E_RUTIL_TM_TOPICS eTopic, const char *nameOfTeamToHelp, const E_RUTIL_TM_RESULTS eWhichResult, const unsigned long maxAllowed, const char *nameOfFunction, const unsigned int lineOfCode)
{
    if(rtk_util_tm_get_result_in_us(eTopic, eWhichResult) >= maxAllowed) {
        rtd_printk(KERN_ERR, RUTIL_TM_INFO_SUMMARY_TAG, "[%s][%d] exceed time limit, max %ld, now %ld, ask help from %s",
            nameOfFunction, lineOfCode,
            maxAllowed, RUTIL_TM_GET_RESULT_IN_us(eTopic, eWhichResult),
            nameOfTeamToHelp);

        return true;
    } else {
        return false;
    }
}
