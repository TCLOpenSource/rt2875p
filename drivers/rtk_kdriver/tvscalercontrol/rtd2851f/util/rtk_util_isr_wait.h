#ifndef __RTK_UTIL_ISR_WAIT_H__
#define __RTK_UTIL_ISR_WAIT_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifdef BUILD_QUICK_SHOW
#include <no_os/printk.h>
#include <no_os/wait.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#endif
/*
    Useful when you want to use interrupt to wait hardware toggles it's status.

    Ex: If you need to polling a register by while()

        void myfunc(void)
        {
            ...

            do {
                if(needToBreak)
                    break;
                else
                    read reg1;
            } while(1 == reg1.somebit);

            writeRegs(channel1);             // write registers immediately
        }

    If the reg1.somebit will be cleared by hardware after vsync which means you can exit the while at vsync back porch.
    But in a multitasking environment, you don't know when your task was context switched out.
    And you're afraid of infinite loop and blocked the CPU or sometimes you've to cancel the whole process, you'll write your codes like this:

        void myfunc(void)
        {
            ...

            loopcnt1 = 0xffff;
            while(--loopcnt1 > 0) {
                loopcnt2 = 0xffff;
                do {
                    if(needToBreak)
                        // need to break the whole process and return to begining (ex: timing disappeared suddenly)
                        break;
                    else
                        read reg1;

                    usleep_range(1000, 1100);       // not to block CPU, usleep_range() is more precise then others. msleep() is not reliable, actually msleep(<20ms) will be >= 20ms.
                } while((1 == reg1.somebit) && (--loopcnt2 > 0));

                if(0 != loopcnt2) {
                    // not timeout, check if we've enough time to write registers
                    if((uzu linecnt > 100) || (uzu linecnt < 1900))
                        break;
                }
            }

            if((0 != loopcnt1) && (0 != loopcnt2))
                // not timeout and enough time
                writeRegs(channel1);
            else
                error
        }

    But in a multitasking environment, you still don't know when your task will be context switched out and when you can get the CPU back.
    The most safe way is to stop the corrent task, then check reg1.somebit in interrupt.

    Ex:

        isr.c
            RUTIL_ISR_WAIT_EXPORT(wait_for_reg1_somebit);

            irqreturn_t my_isr(int irq, void *dev_id)
            {
                if(v active start ISR) {
                    ...

                    // v active start
                    if(RUTIL_ISR_WAIT(wait_for_reg1_somebit)) {
                        read reg1;
                        if(0 == reg1.somebit) {
                            // get ISR parameter
                            writeRegs( *((unsigned char *) RUTIL_ISR_WAIT_GET_PARAM(wait_for_reg1_somebit)) );

                            // finished, wake up the listener.
                            RUTIL_ISR_WAIT_WAKE_UP(wait_for_reg1_somebit, eRUTIL_ISR_WAIT_ISR_IS_FINISHED);
                        } else {
                            // not finished, wake up the listener still, let it decide what to do at next.
                            RUTIL_ISR_WAIT_WAKE_UP(wait_for_reg1_somebit, eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET);
                        }
                    }

                    ...
                }
            }


        myfunc.c
            static unsigned char myISR_Wait_Reg1_SomeBit_Channel;
            RUTIL_ISR_WAIT_DECLARE(wait_for_reg1_somebit, &myISR_Wait_Reg1_SomeBit_Channel);

            void myfunc(void)
            {
                ...

                unsigned char numOfFramesToWait = 20;
                while(numOfFramesToWait-- > 0) {
                    // Prepare parameter before calling RUTIL_ISR_WAIT_START()
                    // Because ISR will start to action after you called RUTIL_ISR_WAIT_START(), your parameter may be needed immediately.
                    myISR_Wait_Reg1_SomeBit_Channel = channel1;

                    // Ask ISR to start the work
                    RUTIL_ISR_WAIT_START(wait_for_reg1_somebit);

                    // start to wait for ISR wake up, wait enough period to ensure ISR has chance to handle it before RUTIL_ISR_WAIT_FOR() timeout.
                    if(RUTIL_ISR_WAIT_FOR(wait_for_reg1_somebit, 10 * v_sync_period_in_us)) {
                        // ISR waked up us, check if finished or not.
                        if(eRUTIL_ISR_WAIT_ISR_IS_FINISHED == RUTIL_ISR_WAIT_RESULT(wait_for_reg1_somebit)) {
                            // ISR report finished, it's done
                            break;
                        } else {
                            // ISR return but report not finished yet, can check if to cancel or not
                            if(needToBreak) {
                                RUTIL_ISR_WAIT_CANCEL(wait_for_reg1_somebit);
                                break;
                            }
                        }
                    } else {
                        // wait ISR until timed out, perhapes ISR function stocked? or hardware not issue ISR because of no timing input? Or timeout is too short?
                        // do your error handling here
                        rtd_printk(KERN_ERR, YOUR_TAG_NAME, "Error, no ISR happened");
                    }
                }

                RUTIL_ISR_WAIT_STOP(wait_for_reg1_somebit);

                if(eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET == RUTIL_ISR_WAIT_RESULT(wait_for_reg1_somebit)) {
                    // left while but ISR report not finished yet, which means failed and you've to do error handle here

                    // note if you called RUTIL_ISR_WAIT_CANCEL(), the RUTIL_ISR_WAIT_RESULT() will be eRUTIL_ISR_WAIT_ISR_IS_FINISHED
                    // so you won't enter here.
                    rtd_printk(KERN_ERR, YOUR_TAG_NAME, "Error, can't xxx");
                }
            }

            int my_module_init(void) {
                ...
                RUTIL_ISR_WAIT_INIT(wait_for_reg1_somebit);
                ...
            }

            module_init(my_module_init)

*/

#define RUTIL_ISR_WAIT_TAG              "ISR_WAIT"
typedef enum {
    // ISR report state
    eRUTIL_ISR_WAIT_ISR_IS_FINISHED,
    eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET,

    // listener report state
    eRUTIL_ISR_WAIT_ISR_NOT_FINISH_CANCELED = 30,
    eRUTIL_ISR_WAIT_ISR_TIMEOUT,
    eRUTIL_ISR_WAIT_LOOPCNT_TIMEOUT,
    eRUTIL_ISR_WAIT_SEMAPHORE_INTERRUPTED,
} E_RUTIL_ISR_WAIT_RESULTS;

//////////////////////////////////////////////////////////////////////////////////////
// for advanced ISR WAIT, less codes to write but less control
//////////////////////////////////////////////////////////////////////////////////////
#define RUTIL_ISR_WAIT_ADV_DECLARE(waitName, condName, pParam, condToCheck, executeIfCondTrue, executeIfCondFalse)  \
    RUTIL_ISR_WAIT_DECLARE(waitName, pParam);                                                                       \
    DEFINE_SEMAPHORE(sema_##waitName);                                                                              \
    void *pRUTIL_ISR_WAIT_Param_##waitName##_##condName = (void *) pParam;                                          \
    unsigned char RUTIL_ISR_WAIT_ISR_CondToCheck_##waitName##_##condName(void) {                                             \
        condToCheck;                                                                                                \
    }                                                                                                               \
    void RUTIL_ISR_WAIT_ISR_ExecWhenCondTrue_##waitName##_##condName(void *pUsrParam) {                             \
        executeIfCondTrue;                                                                                          \
    }                                                                                                               \
    void RUTIL_ISR_WAIT_ISR_ExecWhenCondFalse_##waitName##_##condName(void *pUsrParam) {                            \
        executeIfCondFalse;                                                                                         \
    }                                                                                                               \
    unsigned char (*pRUTIL_ISR_WAIT_FuncCondToCheck_##waitName)(void)                                                        \
        = RUTIL_ISR_WAIT_ISR_CondToCheck_##waitName##_##condName;                                                   \
                                                                                                                    \
    void (*pRUTIL_ISR_WAIT_FuncExecIfCondTrue_##waitName)(void *pUsrParam)                                          \
        = RUTIL_ISR_WAIT_ISR_ExecWhenCondTrue_##waitName##_##condName;                                              \
                                                                                                                    \
    void (*pRUTIL_ISR_WAIT_FuncExecIfCondFalse_##waitName)(void *pUsrParam)                                         \
        = RUTIL_ISR_WAIT_ISR_ExecWhenCondFalse_##waitName##_##condName

#define RUTIL_ISR_WAIT_ADV_EXPORT(waitName)                                                                         \
    RUTIL_ISR_WAIT_EXPORT(waitName);                                                                                \
    extern struct semaphore sema_##waitName;                                                                        \
    extern unsigned char (*pRUTIL_ISR_WAIT_FuncCondToCheck_##waitName)(void);                                                \
    extern void (*pRUTIL_ISR_WAIT_FuncExecIfCondTrue_##waitName)(void *pUsrParam);                                  \
    extern void (*pRUTIL_ISR_WAIT_FuncExecIfCondFalse_##waitName)(void *pUsrParam);

// for ISR part
#define RUTIL_ISR_WAIT_ADV_ISR(waitName)                                                                        \
    {                                                                                                           \
        if(RUTIL_ISR_WAIT(waitName)) {                                                                          \
            u32RUTIL_ISR_WAIT_ISR_Cnt_##waitName++;                                                             \
            if(pRUTIL_ISR_WAIT_FuncCondToCheck_##waitName()) {                                                  \
                pRUTIL_ISR_WAIT_FuncExecIfCondTrue_##waitName(RUTIL_ISR_WAIT_GET_PARAM(waitName));              \
                RUTIL_ISR_WAIT_WAKE_UP(waitName, eRUTIL_ISR_WAIT_ISR_IS_FINISHED);                              \
            } else {                                                                                            \
                pRUTIL_ISR_WAIT_FuncExecIfCondFalse_##waitName(RUTIL_ISR_WAIT_GET_PARAM(waitName));             \
                RUTIL_ISR_WAIT_WAKE_UP(waitName, eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET);                         \
            }                                                                                                   \
        }                                                                                                       \
    }

// for listener
#define RUTIL_ISR_WAIT_CS_WAIT_ISR_COND_TRUE(waitName, msg)                                                         \
    RUTIL_ISR_WAIT_CS_WAIT_BUSY_LOOP_DONE(msg                                                                       \
        ,                                                                                                           \
            while(!pRUTIL_ISR_WAIT_FuncCondToCheck_##waitName());                                                   \
        ,                                                                                                           \
            pRUTIL_ISR_WAIT_FuncExecIfCondTrue_##waitName(RUTIL_ISR_WAIT_GET_PARAM(waitName));                      \
    )

#define RUTIL_ISR_WAIT_CS_WAIT_BUSY_LOOP_DONE(msg, codeBlkWhile, codeBlkExecuteWhenDone)                            \
    {                                                                                                               \
        DEFINE_SPINLOCK(myLock);                                                                                    \
        unsigned long myFlags;                                                                                      \
                                                                                                                    \
        spin_lock_irqsave(&myLock, myFlags);                                                                        \
        codeBlkWhile;                                                                                               \
        RUTIL_TM_DUMP_LINE_INFO(0, msg);                                                                            \
        codeBlkExecuteWhenDone;                                                                                     \
        spin_unlock_irqrestore(&myLock, myFlags);                                                                   \
    }

#define RUTIL_ISR_WAIT_ADV_ADD_COND(waitName, condName, pParam, condToCheck, executeIfCondTrue, executeIfCondFalse) \
    void *pRUTIL_ISR_WAIT_Param_##waitName##_##condName = (void *) pParam;                                          \
    unsigned char RUTIL_ISR_WAIT_ISR_CondToCheck_##waitName##_##condName(void) {                                             \
        condToCheck;                                                                                                \
    }                                                                                                               \
    void RUTIL_ISR_WAIT_ISR_ExecWhenCondTrue_##waitName##_##condName(void *pUsrParam) {                             \
        executeIfCondTrue;                                                                                          \
    }                                                                                                               \
    void RUTIL_ISR_WAIT_ISR_ExecWhenCondFalse_##waitName##_##condName(void *pUsrParam) {                            \
        executeIfCondFalse;                                                                                         \
    }


#define RUTIL_ISR_WAIT_ADV_EXPORT_COND(waitName, condName)                                                          \
    extern void *pRUTIL_ISR_WAIT_Param_##waitName##_##condName;                                                     \
    extern unsigned char RUTIL_ISR_WAIT_ISR_CondToCheck_##waitName##_##condName(void);                                       \
    extern void RUTIL_ISR_WAIT_ISR_ExecWhenCondTrue_##waitName##_##condName(void *pUsrParam);                       \
    extern void RUTIL_ISR_WAIT_ISR_ExecWhenCondFalse_##waitName##_##condName(void *pUsrParam);                      \
    extern E_RUTIL_ISR_WAIT_RESULTS RUTIL_ISR_WAIT_FUNCTION_##waitName##_##condName(unsigned int loopCnt, unsigned long timeoutIn_us)

#define RUTIL_ISR_WAIT_ADV_SEL_COND(waitName, condName)                                                             \
    pRUTIL_ISR_WAIT_PARAM_##waitName = pRUTIL_ISR_WAIT_Param_##waitName##_##condName;                               \
    pRUTIL_ISR_WAIT_FuncCondToCheck_##waitName                                                                      \
        = RUTIL_ISR_WAIT_ISR_CondToCheck_##waitName##_##condName;                                                   \
                                                                                                                    \
    pRUTIL_ISR_WAIT_FuncExecIfCondTrue_##waitName                                                                   \
        = RUTIL_ISR_WAIT_ISR_ExecWhenCondTrue_##waitName##_##condName;                                              \
                                                                                                                    \
    pRUTIL_ISR_WAIT_FuncExecIfCondFalse_##waitName                                                                  \
        = RUTIL_ISR_WAIT_ISR_ExecWhenCondFalse_##waitName##_##condName

#define RUTIL_ISR_WAIT_ADV_WAIT(waitName, condName, loopCnt, timeOutIn_us)                                          \
    RUTIL_ISR_WAIT_FUNCTION_##waitName##_##condName(loopCnt, timeOutIn_us)

// provide some blk only w/o to write the whole function body
// it's your duty to make sure there is no race condition (never wait for semaphore)
//    so if you saw the error msg such as semaphore locked, there is a race condition when using waitName
#define RUTIL_ISR_WAIT_ADV_WAIT_FUNCTION(waitName, condName, codeBlkVariables, codeBlkPreCheck, codeBlkISR_Finish, codeBlkISR_NotFinish, codeBlkISR_TimeOut, codeBlkLoopCntTimeOut, codeBlkBeforeReturn)     \
    E_RUTIL_ISR_WAIT_RESULTS RUTIL_ISR_WAIT_FUNCTION_##waitName##_##condName(unsigned int loopCnt, unsigned long timeoutIn_us)    \
    {                                                                                                   \
        codeBlkVariables;                                                                               \
                                                                                                        \
        if(down_trylock(&sema_##waitName)) {                                                            \
            rtd_printk(KERN_ERR, RUTIL_ISR_WAIT_TAG, "ISR_WAIT: %s/%s semaphore locked before, wait",   \
                #waitName, #condName);                                                                  \
                                                                                                        \
            if(down_interruptible(&sema_##waitName)) {                                                  \
                rtd_printk(KERN_ERR, RUTIL_ISR_WAIT_TAG, "ISR_WAIT: %s/%s semaphore interrupted",       \
                    #waitName, #condName);                                                              \
                return eRUTIL_ISR_WAIT_SEMAPHORE_INTERRUPTED;                                           \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        RUTIL_ISR_WAIT_ADV_SEL_COND(waitName, condName);                                                \
        codeBlkPreCheck;                                                                                \
                                                                                                        \
        u32RUTIL_ISR_WAIT_ISR_Cnt_##waitName = 0;                                                       \
        while(loopCnt--) {                                                                              \
            RUTIL_ISR_WAIT_START(waitName);                                                             \
            if(RUTIL_ISR_WAIT_FOR(waitName, timeoutIn_us)) {                                            \
                if(eRUTIL_ISR_WAIT_ISR_IS_FINISHED == RUTIL_ISR_WAIT_RESULT(waitName)) {                \
                    codeBlkISR_Finish;                                                                  \
                    goto ISR_Wait_Return;                                                               \
                } else {                                                                                \
                    codeBlkISR_NotFinish;                                                               \
                }                                                                                       \
            } else {                                                                                    \
                RUTIL_ISR_WAIT_RESULT(waitName) = eRUTIL_ISR_WAIT_ISR_TIMEOUT;                          \
                codeBlkISR_TimeOut;                                                                     \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        if(eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET == RUTIL_ISR_WAIT_RESULT(waitName)) {                   \
            RUTIL_ISR_WAIT_RESULT(waitName) = eRUTIL_ISR_WAIT_LOOPCNT_TIMEOUT;                          \
            codeBlkLoopCntTimeOut;                                                                      \
        }                                                                                               \
                                                                                                        \
    ISR_Wait_Return:                                                                                    \
        RUTIL_ISR_WAIT_STOP(waitName);                                                                  \
        codeBlkBeforeReturn;                                                                            \
                                                                                                        \
        up(&sema_##waitName);                                                                           \
        return RUTIL_ISR_WAIT_RESULT(waitName);                                                         \
                                                                                                        \
    ISR_Wait_Cancel_Then_Return:                                                                        \
        __attribute__((unused));                                                                        \
        RUTIL_ISR_WAIT_CANCEL(waitName);                                                                \
        goto ISR_Wait_Return;                                                                           \
    }

/////////////////////////////////////////////////////////////////////////////
// for normal init, write more codes w/ fully control
/////////////////////////////////////////////////////////////////////////////
#define RUTIL_ISR_WAIT_DECLARE(waitName, pParam)                                \
    void *pRUTIL_ISR_WAIT_PARAM_##waitName = (void *) pParam;                   \
    wait_queue_head_t wqRUTIL_ISR_WAIT_##waitName;                              \
    bool bRUTIL_ISR_WAIT_start_to_wait_##waitName = false;                      \
    unsigned int u32RUTIL_ISR_WAIT_ISR_Cnt_##waitName;                          \
    extern unsigned int Scaler_get_input_v_period_in_us(unsigned char display); \
    extern unsigned int Scaler_get_uzu_v_period_in_us(unsigned char display);   \
    extern unsigned int Scaler_get_memcdtg_v_period_in_us(void);                \
    E_RUTIL_ISR_WAIT_RESULTS eRUTIL_ISR_WAIT_RESULTS_##waitName                 \
        = eRUTIL_ISR_WAIT_ISR_IS_FINISHED;                                      \
                                                                                \



#define RUTIL_ISR_WAIT_EXPORT(waitName)                                         \
    extern void *pRUTIL_ISR_WAIT_PARAM_##waitName;                              \
    extern wait_queue_head_t wqRUTIL_ISR_WAIT_##waitName;                       \
    extern bool bRUTIL_ISR_WAIT_start_to_wait_##waitName;                       \
    extern unsigned int u32RUTIL_ISR_WAIT_ISR_Cnt_##waitName;                   \
    extern E_RUTIL_ISR_WAIT_RESULTS eRUTIL_ISR_WAIT_RESULTS_##waitName;         \
    extern unsigned int Scaler_get_input_v_period_in_us(unsigned char display); \
    extern unsigned int Scaler_get_uzu_v_period_in_us(unsigned char display);   \
    extern unsigned int Scaler_get_memcdtg_v_period_in_us(void)

#define RUTIL_ISR_WAIT_INIT(waitName)        init_waitqueue_head(&wqRUTIL_ISR_WAIT_##waitName)

// for ISR
#define RUTIL_ISR_WAIT(waitName)             bRUTIL_ISR_WAIT_start_to_wait_##waitName

#define RUTIL_ISR_WAIT_GET_PARAM(waitName)   pRUTIL_ISR_WAIT_PARAM_##waitName

#define RUTIL_ISR_WAIT_WAKE_UP(waitName, eResult)        \
    eRUTIL_ISR_WAIT_RESULTS_##waitName = eResult;        \
    bRUTIL_ISR_WAIT_start_to_wait_##waitName = false;    \
    wake_up_interruptible_sync(&wqRUTIL_ISR_WAIT_##waitName)


// for listener
#define RUTIL_ISR_WAIT_START(waitName)                                          \
    eRUTIL_ISR_WAIT_RESULTS_##waitName = eRUTIL_ISR_WAIT_ISR_NOT_FINISHED_YET;  \
    bRUTIL_ISR_WAIT_start_to_wait_##waitName = true

#define RUTIL_ISR_WAIT_STOP(waitName)                                           \
    bRUTIL_ISR_WAIT_start_to_wait_##waitName = false

#define RUTIL_ISR_WAIT_FOR(waitName, timeOutIn_us)       \
    wait_event_interruptible_timeout(wqRUTIL_ISR_WAIT_##waitName, false == bRUTIL_ISR_WAIT_start_to_wait_##waitName, usecs_to_jiffies(timeOutIn_us))

#define RUTIL_ISR_WAIT_RESULT(waitName)     eRUTIL_ISR_WAIT_RESULTS_##waitName
#define RUTIL_ISR_WAIT_ISR_CNT(waitName)    u32RUTIL_ISR_WAIT_ISR_Cnt_##waitName

#define RUTIL_ISR_WAIT_CANCEL(waitName)                  \
    eRUTIL_ISR_WAIT_RESULTS_##waitName = eRUTIL_ISR_WAIT_ISR_NOT_FINISH_CANCELED

#ifdef __cplusplus
}
#endif

#endif  // __RTK_UTIL_ISR_WAIT_H__