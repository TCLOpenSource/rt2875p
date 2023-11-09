#ifndef __RTK_UTIL_TM_H__
#define __RTK_UTIL_TM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* This module was used to profile (measure the time elapsed) during some specific code flow (we called this: topic).
   You can use this module to profile different topics across threads.

   PS: There are at least 3 approaches to do this:
       1. Find a profiling tool
       2. instrument-functions: https://jasonblog.github.io/note/debug/gccg++_-instrument-functions_can_shu.html
       3. rtk_util_tm way: We can do anything we like, and you'll be more familier to trace the code flow by yourself.

   WARNING: This module is not thread safe. (Be free to upgrade it. I think you don't have to change any interface to support thread safe.)
            If you saw some steps has un-usual time elapsed, it's usually caused by RUTIL_TM_LOG() called not finished by one thread but another thread entered this macro.
            You've to watch and avoid this problem by yourself.

   For example, we want to profile the unmute latency from VO ready to really unmute:

   1. You've to declare there is a new topic to profile by adding an enum to E_RUTIL_TM_TOPICS.
      Such as eRUtilTM_VSC_Unmute.

      Be remember if you added an enum for a new topic but w/o initialize the topic, the function calls (such as RUTIL_TM_LOG()) won't take effects.

   2. You've to initial the TM module by specifying how many threads in this topic and how many steps you want to measure the elapsed time.
      Usually you'll do this in module init.

        int my_module_init(void) {
            ...
            RUTIL_TM_MODULE_INIT();
            RUTIL_TM_INIT_TOPIC("VSC Unmute", eRUtilTM_VSC_Unmute, 3, 300);     // there are 3 threads in this topic, total we'll add no more than 300 steps to measure the time elapsed.
                                                                                // you'll be warned when called RUTIL_TM_END_AND_DUMP() if the topic that you want to profile exceed 3 threads and 300 steps
            ...
        }

        void __exit my_module_exit(void) {
            ...
            RUTIL_TM_MODULE_EXIT();                                             // release the memory used by this module.
            ...
        }

        module_init(my_module_init) ;
        module_exit(my_module_exit) ;

    3. Specify when to start/stop_and_dump measure the time elapsed, and when to measure each steps.
       Ex:

        // thread 1, RPC
        unsigned int rpcVoReady(unsigned long para1, unsigned long para2)
        {
            ...
            up(&VO_InfoSemaphore);

            RUTIL_TM_START(eRUtilTM_VSC_Unmute, "VO Ready");                    // if playback by VO such as Netflix, we can measure the unmute latency from here cause the VO info was confirmed.
        }

        // thread 2, you've two choices to insert RUTIL_TM_LOG(), it's up to you to pick one of them.
    #if 1
        unsigned char rtk_run_scaler(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType, KADP_VSC_OUTPUT_MODE_T outputMode)
        {
            "do some init"

            RUTIL_TM_LOG_BLK(eRUtilTM_VSC_Unmute, "Setup_IMD"),
                Setup_IMD();
            );

            "do rest"
        }
    #else
        unsigned char rtk_run_scaler(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType, KADP_VSC_OUTPUT_MODE_T outputMode)
        {
            "do some init"

            RUTIL_TM_LOG(eRUtilTM_VSC_Unmute, "Before Setup_IMD");              // The macro was designed to minimize the impact to system performance and try not count the overhead of RUTIL_TM_LOG() into time elapsed.

            Setup_IMD();

            RUTIL_TM_LOG(eRUtilTM_VSC_Unmute, "After Setup_IMD");

            "do rest"
        }
    #endif

        // thread 3,
        void scalerdisplay_force_bg_enable(unsigned int channel, unsigned char enable)
        {
            ...
            wakeup_mute_off_callback(SLR_MAIN_DISPLAY);

            // This is dump one line summary w/ RUTIL_TM_INFO_SUMMARY_TAG even you set CONFIG_RUTIL_TM_LOG_DUMP_TO_CONSOLE to false
            RUTIL_TM_END_AND_DUMP(eRUtilTM_VSC_Unmute, "Unmuted", CONFIG_RUTIL_TM_LOG_DUMP_TO_CONSOLE);           // unmuted, end and dump information

            // The measured result will be kept until you called RUTIL_TM_END_AND_DUMP() again
            // You can use this macro to exam if the measured time exceeded the limit or not.
            // It will dump message w/ RUTIL_TM_INFO_SUMMARY_TAG if the measured time exceeded the limit.
            RUTIL_TM_VERIFY_RESULT_IN_us(eRUtilTM_VSC_Unmute, "Name of team to call help if exceeded the limit", eRUtilTM_Results_TotalActual, 450*1000); // 450 ms = 450*1000 us

            // Or you can use this macro to get the measured result
            if(RUTIL_TM_GET_RESULT_IN_us(eRUtilTM_VSC_Unmute, eRUtilTM_Results_totalElapsed) > 450*1000) {
                rtd_printk(KERN_ERR, RUTIL_TM_INFO_SUMMARY_TAG, "Your error msg.");
            }
        }

    4. How to read the log of TM profiling result
        Please study TM_LOG_TITLE/TM_LOG_FMT in rtk_util_tm.c to realize how to analysis the log dumped.
        We recomment that you store the log to usb (rtd_kernel_usb.log), you may get all lines contains
            [RUTIL_TM_INFO_TAG] or [RUTIL_TM_INFO_SUMMARY_TAG] in rtd_kernel_usb.log and import to excel to do futhur analysis.

    5. How to read the log of RUTIL_TM_DUMP_LINE_INFO()
        Please study rtk_util_tm_dump_line_info() in rtk_util_tm.c to realize how to analysis the log dumped.
        We recomment that you store the log to usb (rtd_kernel_usb.log), you may get all lines contains
            [RUTIL_TM_LINE_INFO_TAG] in rtd_kernel_usb.log and import to excel to do futhur analysis.

    6. The TM module was config not to impact performance by deault.
       You can switch those configs:

       #define CONFIG_RUTIL_TM_LOG_ENABLE
            Default is 1.
            If you set to 0, all TM Macros won't take effects.

       #define CONFIG_RUTIL_TM_LOG_START_END_ONLY
            Default is 1 to save code size, the TM module won't measure the time elapsed of each steps but only start and end.
            Set to 0 if you want every steps you logged with RUTIL_TM_LOG() takes effects.
            If CONFIG_RUTIL_TM_LOG_ENABLE is 0, the TM module won't log anything and this config has no effects.

       #define CONFIG_RUTIL_TM_LOG_DUMP_TO_CONSOLE
            Default is 0.
            Actually this define has no effect to TM module itself.
            But you can use this definition when you called RUTIL_TM_END_AND_DUMP() as the 3rd parameter to unify the behavior of whole system.

       #define CONFIG_RUTIL_TM_DUMP_LINE_INFO
            Default is 0.
            There is a useful macro RUTIL_TM_DUMP_LINE_INFO()
            It will dump line information including VO, MTG (DTG-M), UZU (M-disp/memc input timing), MEMC (to panel) and other information.
            Refer to the function body for the detail.
            Please note dump this will impact the system performance so default it's off and the RUTIL_TM_DUMP_LINE_INFO() won't take effects.
*/

// it's not recomment to include files in header file, but this is a debug module and we want to save time when adding RUTIL_TM_LOG() to many files.
// User just include rtk_util_tm.h is enough.
#ifndef BUILD_QUICK_SHOW
#include <linux/sched.h>
#else
#include <sysdefs.h>
#endif
#include <rbus/timer_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/ppoverlay_reg.h>

#define RUTIL_TM_INFO_TAG             "TM"
#define RUTIL_TM_INFO_SUMMARY_TAG     "TM_SUMMARY"
#define RUTIL_TM_LINE_INFO_TAG        "TM_LINE_INFO"

typedef enum {
    eRUtilTM_Results_totalElapsed,               // the time elapsed from RUTIL_TM_START() to RUTIL_TM_END_AND_DUMP(), we use 90KHz clock to measure the period.
    eRUtilTM_Results_TotalRunDelay,              // the time that we context switched out (no matter by ourself or by the scheduler) which means we're not using the CPU, refer to https://cloud.tencent.com/developer/article/1635841
    eRUtilTM_Results_TotalActual,                // the actual time that we're using CPU. (Actual = Elapsed - RunDelay)
    eRUtilTM_Results_TotalNVCSW,                 // context switch counts caused by ourself
    eRUtilTM_Results_TotalNIVCSW,                // context switch counts caused by scheduler
} E_RUTIL_TM_RESULTS;

typedef enum {
    eRUtilTM_VSC_Unmute,
    eRUtilTM_Topic_SetIMD_PQ,

    // add your own topic before this line
    eRUtilTM_Topic_Max,
} E_RUTIL_TM_TOPICS;

#define CONFIG_RUTIL_TM_LOG_ENABLE           0
#define CONFIG_RUTIL_TM_LOG_START_END_ONLY   1
#define CONFIG_RUTIL_TM_LOG_DUMP_TO_CONSOLE  0
#define CONFIG_RUTIL_TM_DUMP_LINE_INFO       0

/******************************************************************
 * Do not modify from here
 ******************************************************************/
#if CONFIG_RUTIL_TM_DUMP_LINE_INFO
    #define RUTIL_TM_DUMP_LINE_INFO(cnt, comment)                   rtk_util_tm_dump_line_info(cnt, comment)
#else
    #define RUTIL_TM_DUMP_LINE_INFO(cnt, comment)
#endif

#if CONFIG_RUTIL_TM_LOG_ENABLE
  #if CONFIG_RUTIL_TM_LOG_START_END_ONLY
    #define RUTIL_TM_LOG(eTopic, comment)
    #define RUTIL_TM_LOG_BLK(eTopic, comment, blkOfCodes)            blkOfCodes
  #else
    #define RUTIL_TM_LOG(eTopic, comment)                            RUTIL_TM_LOG_INTERNAL(eTopic, comment)
    #define RUTIL_TM_LOG_BLK(eTopic, comment, blkOfCodes)            \
        RUTIL_TM_LOG_INTERNAL(eTopic, comment" Start");              \
        blkOfCodes;                                                  \
        RUTIL_TM_LOG_INTERNAL(eTopic, comment" End");
  #endif

    #define RUTIL_TM_MODULE_INIT()                                   rtk_util_tm_module_init()
    #define RUTIL_TM_MODULE_EXIT()                                   rtk_util_tm_module_exit()

    #define RUTIL_TM_INIT_TOPIC(nameOfTopic, eTopic, numOfThreads, numOfSteps)       rtk_util_tm_init_topic(nameOfTopic, eTopic, numOfThreads, numOfSteps)

    // use RUTIL_TM_LOG_INTERNAL() to ensure rtk_util_tm_start/end_and_dump will always has time information,
    // without influrence of RUTIL_TM_CONFIG_LOG_DETAIL_STEPS
    // but if CONFIG_RUTIL_TM_LOG_ENABLE = 0, rtk_util_tm_start/end_and_dump has no effects.
    #define RUTIL_TM_START(eTopic, comment)                                 \
                    {                                                       \
                        rtk_util_tm_start(eTopic);                          \
                        RUTIL_TM_LOG_INTERNAL(eTopic, comment);             \
                    }

    #define RUTIL_TM_END_AND_DUMP(eTopic, comment, bDumpToConsole)          \
                    {                                                       \
                        RUTIL_TM_LOG_INTERNAL(eTopic, comment);             \
                        rtk_util_tm_end_and_dump(eTopic, bDumpToConsole);   \
                    }

    #define RUTIL_TM_GET_RESULT_IN_us(eTopic, eResult)                               rtk_util_tm_get_result_in_us(eTopic, eResult)
    #define RUTIL_TM_VERIFY_RESULT_IN_us(eTopic, nameOfTeamToHelp, eResult, max)     rtk_util_tm_verify_result_in_us(eTopic, nameOfTeamToHelp, eResult, max, __FUNCTION__, __LINE__)
#else
    // turn off everything
    #define RUTIL_TM_LOG(eTopic, comment)
    #define RUTIL_TM_LOG_BLK(eTopic, comment, blkOfCodes)                   blkOfCodes

    #define RUTIL_TM_MODULE_INIT()
    #define RUTIL_TM_MODULE_EXIT()

    #define RUTIL_TM_INIT_TOPIC(nameOfTopic, eTopic, numOfThreads, numOfSteps)

    #define RUTIL_TM_START(eTopic, comment)
    #define RUTIL_TM_END_AND_DUMP(eTopic, comment, bDumpToConsole)
    #define RUTIL_TM_GET_RESULT_IN_us(eTopic, eResult)     (0UL)
    #define RUTIL_TM_VERIFY_RESULT_IN_us(eTopic, nameOfTeamToHelp, eResult, max)
#endif

// define the logged info of each step
typedef struct {
    unsigned int        timeStamp;
    unsigned int        logStart;

    const char          *comment;           // the string that you want to note

    unsigned int        linenum;            // the linenum when you called time_measure_log()
    const char          *funcName;          // the function name when you called time_measure_log()

    unsigned int        tid;                // kernel thread id (in fact, it's pid)

    unsigned long		nvcsw;              // the number of voluntary context switches (Number of Voluntary Context Switches) -> context switched by ourself
	unsigned long		nivcsw;             // the number of involuntary context switches (Number of InVoluntary Context Switches) -> context switched not by ourself

#ifdef CONFIG_THREAD_INFO_IN_TASK
    unsigned int        cpu;                // you can get what CPU we're using now w/ current->thread_info.cpu
#endif

    // scheduling info, please check CONFIG_SCHED_INFO
    //unsigned long      pcount;            // # of times we have run on this CPU, you can observe pcount though (nvscw + nivcsw)
    unsigned long long   run_delay;         // Time spent waiting on a runqueue, the unit is nano second instead of jiffies, see https://unix.stackexchange.com/questions/418773/measure-units-in-proc-pid-schedstat
    //unsigned long long last_arrival;      // When did we last run on a CPU? (useless, all 0)
    //unsigned long long last_queued;       // When were we last queued to run? (useless, all 0)
} stRUTIL_TM_STEP_LOG;

typedef struct {
    unsigned int        tid;                // kthread id (in fact, it's pid)
    unsigned int        latestTimeStamp;    // latest time stamp
    unsigned long long  latestRunDelay;
    unsigned long       totalRunDelayInUS;  // unit is us

    unsigned long       latestNVSCW;
    unsigned long       latestNIVSCW;

    unsigned long       totalNVCSW;
    unsigned long       totalNIVCSW;
} stRUTIL_TM_TOPIC_THREAD_INFO;

// struct for each topic
typedef struct {
    const unsigned char *nameOfTopic;

    unsigned char numOfThreads;
    stRUTIL_TM_TOPIC_THREAD_INFO *pstThreadInfo;

    stRUTIL_TM_STEP_LOG *pstStepLogHead;
    stRUTIL_TM_STEP_LOG *pstStepLogCurrent;         // point to the empty record to store new info w/ RUTIL_TM_LOG()

    stRUTIL_TM_STEP_LOG *pstStepLogMax;             // point to the end+1 of the empty records

    bool go;                                        // start to log each step or not. used in RUTIL_TM_LOG()
    unsigned int runID;                             // runID, won't be cleared except you called rtk_util_tm_exit()

    unsigned int totalElapsedInUS;                  // total elapsed time from start to end
    unsigned int totalRunDelayInUS;                 // total run delay from start to end

    unsigned int totalNVCSW;
    unsigned int totalNIVCSW;
} stRUTIL_TM_TOPICS;     // store info of each topic


// globals
extern stRUTIL_TM_TOPICS gstRUTIL_TM_Topics[eRUtilTM_Topic_Max];

/* ***************************************************************************************************************
 *
 * Call this macro to dump scaler related line info.
 *
 * It will dump line count info of VO, MTG (DTG-M), UZU (M-disp/memc input timing), MEMC (to panel).
 * And will dump other information.
 *
 * See the comments in this function to understand how to read the output.
 *
 * Note: this will impact system performance, use CONFIG_RUTIL_TM_DUMP_LINE_INFO (default off) to control it.
 *
 * @param cnt     --> any number that you want to print.
 * @param comment --> the comment to dump, ease to grep.
 * ****************************************************************************************************************/
extern void rtk_util_tm_dump_line_info(const unsigned int cnt, const char *comment);

/* ***************************************************************************************************************
 * This function will initialize TM module.
 * ****************************************************************************************************************/
extern void rtk_util_tm_module_init(void);

/* ***************************************************************************************************************
 * This function will initialize each topic.
 *
 * @param eTopic --> the topic (code flow) you specified w/ E_RUTIL_TM_TOPICS
 * @param numOfThreads --> how many possible threads within this topic (code flow)
 * @param numOfSteps --> how many possible steps you want to measure the time elapsed
 * @return false --> failed to allocate memory or already initialized without exit.
 * ****************************************************************************************************************/
extern bool rtk_util_tm_init_topic(const unsigned char *nameOfTopic, const E_RUTIL_TM_TOPICS eTopic, const unsigned char numOfThreads, const unsigned short numOfSteps);

/* ***************************************************************************************************************
 * This function will de-initialize TM module including free the memory.
 *
 * @return false --> if not initialized or initialized failed before.
 * ****************************************************************************************************************/
extern bool rtk_util_tm_module_exit(void);

/* ***************************************************************************************************************
 * Start to log time elapsed on eTopic code flow.
 *
 * @param eTopic --> the topic (code flow) you specified w/ E_RUTIL_TM_TOPICS
 * @return false --> not initialized or initialized failed before.
 * ****************************************************************************************************************/
extern bool rtk_util_tm_start(const E_RUTIL_TM_TOPICS eTopic);

/* ***************************************************************************************************************
 * Stop logging the time elapsed on eTopic code flow and dump the logged information.
 *
 * Please study TM_LOG_TITLE/TM_LOG_FMT in rtk_util_tm.c to realize how to analysis the log dumped.
 * We recomment that you store the log to usb (rtd_kernel_usb.log), you may get all lines contains
 *     [RUTIL_TM_INFO_TAG] in rtd_kernel_usb.log and import to excel to do futhur analysis.
 *
 * Reset the state machine. So that you can call rtk_util_tm_start() again to start next measure of the whole topic (code flow).
 *
 * @param eTopic --> the topic (code flow) you specified w/ E_RUTIL_TM_TOPICS
 * @param bDumpToConsole --> true if you want to print the summary to console
 * @return false --> not initialized or initialized failed before.
 * ****************************************************************************************************************/
extern bool rtk_util_tm_end_and_dump(const E_RUTIL_TM_TOPICS eTopic, const bool bDumpToConsole);

/* ***************************************************************************************************************
 * Get the TM measured result. Please make sure you called this function after rtk_util_tm_end_and_dump() and before rtk_util_tm_start()
 *
 * @param eTopic --> the topic (code flow) you specified w/ E_RUTIL_TM_TOPICS
 * @param eWhichResult --> select what result you want.
 * @return The result you specified, 0xFFFFFFFF if eTopic not initialized or unknown eTopic. The unit is micro seconds.
 * ****************************************************************************************************************/
extern unsigned long rtk_util_tm_get_result_in_us(const E_RUTIL_TM_TOPICS eTopic, const E_RUTIL_TM_RESULTS eWhichResult);

/* ***************************************************************************************************************
 * Verify the TM measured result. Please make sure you called this function after rtk_util_tm_end_and_dump() and before rtk_util_tm_start().
 * This function will compare the measured result and maxAllowed, always dump KERN_ERR if exceed the maxAllowed including what team should you to call help.
 *
 * @param eTopic --> the topic (code flow) you specified w/ E_RUTIL_TM_TOPICS
 * @param nameOfTeamToHelp --> a string contain the name of team to call help if the measured result exceed the maxAllowed
 * @param eWhichResult --> select what result you want.
 * @param maxAllowed --> the max allowed value.
 * @param nameOfFunction --> __FUNCTION__
 * @param lineOfCode --> __LINE__
 * @return true --> measured result exceed the maxAllowed.
 * ****************************************************************************************************************/
extern bool rtk_util_tm_verify_result_in_us(const E_RUTIL_TM_TOPICS eTopic, const char *nameOfTeamToHelp, const E_RUTIL_TM_RESULTS eWhichResult, const unsigned long maxAllowed, const char *nameOfFunction, const unsigned int lineOfCode);

/* ***************************************************************************************************************
 * Call RUTIL_TM_LOG_INTERNAL() and give a text through @param comment, the TM module will
 * record the time by system 90K clock and the text.
 *
 * At the end, you can call rtk_util_tm_end_and_dump() to dump all time elapsed
 * between each RUTIL_TM_LOG_INTERNAL() that you called.
 *
 * We use macro here to reduce the impact (slow down) to the original code flow.
 *
 * Also, you can notice that we won't count in the overhead of RUTIL_TM_LOG_INTERNAL()
 *
 * @param eTopic --> the topic (code flow) you want to log
 * @param comment --> given a text notice
 * @return none
 * ****************************************************************************************************************/

#ifdef CONFIG_SCHED_INFO
    #define time_measure_log_sched_info(eTopic)                                                             \
    {                                                                                                       \
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->run_delay    = current->sched_info.run_delay;         \
    }
#else
    #define time_measure_log_sched_info(eTopic)                                                             \
    {                                                                                                       \
        gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->run_delay    = 0;                                     \
    }
#endif

// rtd_printk(KERN_INFO, "TM", "log: %s, %d, %d", comments, gstRUTIL_TM_Topics[eTopic].go, gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent != gstRUTIL_TM_Topics[eTopic].pstStepLogMax);
#define RUTIL_TM_LOG_INTERNAL(eTopic, comments)                                                                     \
{                                                                                                                   \
    if(gstRUTIL_TM_Topics[eTopic].go) {                                                                             \
        if(gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent != gstRUTIL_TM_Topics[eTopic].pstStepLogMax) {              \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->timeStamp = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);       \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->comment = comments;                                       \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->linenum  = __LINE__;                                      \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->funcName = __FUNCTION__;                                  \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->tid      = current->pid;                                  \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->nvcsw    = current->nvcsw;                                \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->nivcsw   = current->nivcsw;                               \
                                                                                                                    \
            time_measure_log_sched_info(eTopic);                                                                    \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent->logStart = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);        \
                                                                                                                    \
            gstRUTIL_TM_Topics[eTopic].pstStepLogCurrent++;                                                         \
        }                                                                                                           \
    }                                                                                                               \
}

#ifdef __cplusplus
}
#endif

#endif  // #ifndef __RTK_UTIL_TM_H__
