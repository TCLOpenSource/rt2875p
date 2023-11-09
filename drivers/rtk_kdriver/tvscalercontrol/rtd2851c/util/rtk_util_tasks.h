#ifndef __RTK_UTIL_TASKS__
#define __RTK_UTIL_TASKS__

#ifdef __cplusplus
extern "C" {
#endif

/*  Useful when you want to create a kernel thread, especially when you want to parallelize some flow.
    See this example:

        void my_func(void) {
            doing this....

            long_delay_func(true);  // this function take a long time to finish (ex: 60ms), and not related to rest jobs in "doing that...."

            doing that....          // "doing that...." takes 80ms
        }

    Time could be saved by parallelize long_delay_func() and "doing that....".

    There are at least two approaches here.

    1. Turn long_delay_func() to be a thread, use kthread_run() to execute it.
        Usually kthread_run() takes few ms to 2x ms. We don't want such kind of additional cost.
                                    ^^^^^^    ^^^^^

    2.Turn long_delay_func() to be a thread, create thread and let it wait on a event in advance, then wake it up when we want the thread to be executed.
        By doing so, wake_up() a thread takes 11~22 us. This is what we want.
                                              ^^^^^^^^

    So we choose approach #2, you can use this utility and modify your codes like this:

        myfunc.c
            static bool long_delay_func_thread_trueOrFlase;
            RTK_TSK_DECLARE(long_delay_func_thread, "LongDly_Func",  &long_delay_func_thread_trueOrFlase);

            // turn the function to be a thread
            static int long_delay_func_thread(void *pv) {
                bool trueOrFalse;
                ...
                while(1) {
                    // wait until someone called RTK_TSK_WAKE_UP(long_delay_func_thread) or RTK_TSK_DELETE(long_delay_func_thread)
                    RTK_TSK_WAIT_EVENT_FREEZABLE(long_delay_func_thread);

                    // report that we've waked up
                    RTK_TSK_REPORT_WAKED_UP(long_delay_func_thread);

                    // if waked up by RTK_TSK_DELETE(long_delay_func_thread)
                    if(kthread_should_stop()) {
                        RTK_TSK_ONE_ROUND_FINISH(long_delay_func_thread);
                        break;
                    }

                    // get parameter (you don't have to do this if no need to pass parameter.)
                    trueOrFalse = *((bool *) RTK_TSK_PARAM(long_delay_func_thread));            // you can use long_delay_func_trueOrFlase directly here.

                    // below codes takes a long time to finish
                    // rtd_printk(KERN_INFO, RTK_TSK_TAG_NAME, "Enter: %s", __FUNCTION__);
                    long_delay_func(trueOrFalse);                                               // or long_delay_func(long_delay_func_trueOrFlase)
                    // finished long delay codes

                    // report one round finished
                    RTK_TSK_ONE_ROUND_FINISH(long_delay_func_thread);
                }

                return 0;
            }

            void my_func(void) {
                doing this...

                // prepare paramter
                *((bool *) RTK_TSK_PARAM(long_delay_func_thread)) = true;       // you don't have to do this if no need to pass parameter.

                // wake up long_delay_func()
                // the 2nd parameter = true means we want to wait the last execution of long_delay_func_thread() finished, then start a new one.
                // the 3rd parameter indicate how many ms to wait for the last execution of long_delay_func_thread() finished. If usually it takes 60ms, use 120ms here is safe.
                // this happened if "doing that...." returned without waiting long_delay_func_thread() finished, then my_func() called again immediately.
                if(!RTK_TSK_WAKE_UP(long_delay_func_thread, true, 120)) {
                    pr_emerg("[ERROR] lastest long_delay_func_thread not finished\n");
                }

                doing that...

                // wait the long_delay_func() thread finished, there are 3 ways and up to you
            #if 1
                RTK_TSK_WAIT_FINISH(long_delay_func_thread);        // or RTK_TSK_WAIT_FINISH_TIMEOUT()
            #else
                while(RTK_TSK_IS_EXECUTING(long_delay_func_thread));
            #endif
            }

        mymodule.c
            RTK_TSK_EXPORT(long_delay_func_thread);

            int my_module_init(void) {
                ...
                RTK_TSK_CREATE(long_delay_func_thread);
                ...
            }

            void __exit my_module_exit(void) {
                ...
                RTK_TSK_DELETE(long_delay_func_thread);
                ...
            }

            module_init(my_module_init);
            module_exit(my_module_exit);

    3. Another use case is that you want to turn your thread with while(1) + msleep() to support wait timeout and wake up feature.

        Ex, your original codes looks like this:

            myThread.c
                int my_thread(void *pv) {
                    while(1) {
                        msleep(10);
                        ...
                    }
                }

        It could be:

            myThread.c

                RTK_TSK_WAIT_WAKEUP_DECLARE(MY_THREAD);

                int my_thread(void *pv) {
                    while(1) {
                        if(RTK_TSK_WAIT_EVENT_FREEZABLE_TIMEOUT(MY_THREAD, msecs_to_jiffies(20)) >= 1) {
                            // waked up by event or (event && timeout together)
                        } else {
                            // time out
                        }
                        RTK_TSK_REPORT_WAKED_UP(MY_THREAD);

                        ....

                        RTK_TSK_ONE_ROUND_FINISH(long_delay_func_thread);
                    }
                }

            other.c

                RTK_TSK_WAIT_WAKEUP_EXPORT(MY_THREAD);

                void some_func(void) {
                    ...

                    RTK_TSK_WAKE_UP(MY_THREAD, false, 0);       // don't check if the thread is running or not

                    ...
                }

*/

#define RTK_TSK_TAG_NAME        "RTK_UTIL_TASKS"

// bWaitLastExecutionFinished: if the thread was waked up before, wait until it finished, then wake it up again
// ulMilliSecToWait: if bWaitLastExecutionFinished == true, ulMilliSecToWait indicate how many ms to wait
// return value false means bWaitLastExecutionFinished == ture, but the thread is executing now and not finished after ulMilliSecToWait.
#define RTK_TSK_WAKE_UP(tsk_func, bWaitLastExecutionFinished, ulMilliSecToWait)     rtk_tsk_set_wakeup_flag_##tsk_func(true, bWaitLastExecutionFinished, ulMilliSecToWait)
#define RTK_TSK_IS_EXECUTING(tsk_func)                                              RTK_TSK_##tsk_func##_is_executing
#define RTK_TSK_WAIT_FINISH(tsk_func)                                                       wait_event_freezable(RTK_TSK_WAIT_QUEUE_finished_##tsk_func, false == RTK_TSK_##tsk_func##_is_executing)
#define RTK_TSK_WAIT_FINISH_TIMEOUT(tsk_func, jiffies)                              wait_event_freezable_timeout(RTK_TSK_WAIT_QUEUE_finished_##tsk_func, false == RTK_TSK_##tsk_func##_is_executing, jiffies)

#define RTK_TSK_WAIT_EVENT_FREEZABLE(tsk_func)                              wait_event_freezable(RTK_TSK_WAIT_QUEUE_##tsk_func, (true == rtk_tsk_get_wakeup_flag_##tsk_func()) || kthread_should_stop())
#define RTK_TSK_WAIT_EVENT_FREEZABLE_TIMEOUT(tsk_func, jiffies)     wait_event_freezable_timeout(RTK_TSK_WAIT_QUEUE_##tsk_func, (true == rtk_tsk_get_wakeup_flag_##tsk_func()) || kthread_should_stop(), jiffies)

#define RTK_TSK_REPORT_WAKED_UP(tsk_func)                   \
    rtk_tsk_set_wakeup_flag_##tsk_func(false, false, 0);    \
    RTK_TSK_##tsk_func##_is_executing = true;

#define RTK_TSK_PARAM(tsk_func)                             p_RTK_TSK_##tsk_func##_param
#define RTK_TSK_ONE_ROUND_FINISH(tsk_func)                  \
    RTK_TSK_##tsk_func##_is_executing = false;              \
    wake_up(&RTK_TSK_WAIT_QUEUE_finished_##tsk_func)

#define RTK_TSK_CREATE(tsk_func)                            rtk_tsk_create_##tsk_func()
#define RTK_TSK_DELETE(tsk_func)                            rtk_tsk_delete_##tsk_func()


// declare and export
#define RTK_TSK_WAIT_WAKEUP_EXPORT(tsk_func)                                \
    extern bool RTK_TSK_##tsk_func##_is_executing;                          \
    extern wait_queue_head_t RTK_TSK_WAIT_QUEUE_finished_##tsk_func;        \
    extern void rtk_tsk_set_wakeup_flag_##tsk_func(const bool bToWakeUp, const bool bWaitLastExecutionFinished, unsigned long ulMilliSecToWait);

#define RTK_TSK_EXPORT(tsk_func)                                            \
    RTK_TSK_WAIT_WAKEUP_EXPORT(tsk_func);                                   \
    extern void *p_RTK_TSK_##tsk_func##_param;                              \
    extern void rtk_tsk_create_##tsk_func(void);                            \
    extern void rtk_tsk_delete_##tsk_func(void);

// When there is no thread to create but you want to use the wait/wakeup features
#define RTK_TSK_WAIT_WAKEUP_DECLARE(tsk_func)                               \
    DECLARE_WAIT_QUEUE_HEAD(RTK_TSK_WAIT_QUEUE_##tsk_func);                 \
    DECLARE_WAIT_QUEUE_HEAD(RTK_TSK_WAIT_QUEUE_finished_##tsk_func);        \
    static DEFINE_SPINLOCK(RTK_TSK_SPINLOCK_##tsk_func);                    \
    static bool RTK_TSK_WAKE_UP_FLAG_##tsk_func = false;                    \
    bool RTK_TSK_##tsk_func##_is_executing = false;                         \
                                                                            \
    static bool rtk_tsk_get_wakeup_flag_##tsk_func(void) {                  \
        unsigned long flags;                                                \
        bool ret;                                                           \
                                                                            \
        spin_lock_irqsave(&RTK_TSK_SPINLOCK_##tsk_func, flags);             \
        ret = RTK_TSK_WAKE_UP_FLAG_##tsk_func;                              \
        spin_unlock_irqrestore(&RTK_TSK_SPINLOCK_##tsk_func, flags);        \
                                                                            \
        return ret;                                                         \
    }                                                                       \
                                                                            \
    bool rtk_tsk_set_wakeup_flag_##tsk_func(const bool bToWakeUp, const bool bWaitLastExecutionFinished, unsigned long ulMilliSecToWait) { \
        unsigned long flags;                                                        \
        bool ret = true;                                                            \
                                                                                    \
        if(bWaitLastExecutionFinished) {                                            \
            while(RTK_TSK_##tsk_func##_is_executing && (ulMilliSecToWait-- > 0)) {  \
                usleep_range(1000, 1100);                                           \
            };                                                                      \
                                                                                    \
            if(RTK_TSK_##tsk_func##_is_executing)                                   \
                ret = false;                                                        \
        }                                                                           \
                                                                                    \
        spin_lock_irqsave(&RTK_TSK_SPINLOCK_##tsk_func, flags);                     \
        RTK_TSK_WAKE_UP_FLAG_##tsk_func = bToWakeUp;                                \
        spin_unlock_irqrestore(&RTK_TSK_SPINLOCK_##tsk_func, flags);                \
                                                                                    \
        if(bToWakeUp) {                                                             \
            wake_up(&RTK_TSK_WAIT_QUEUE_##tsk_func);                                \
        }                                                                           \
                                                                                    \
        return ret;                                                                 \
    }


// The tsk_func is the function name of your thread.
// The tsk_name (thread name) is a meaningful C language string, whose length is restricted to 16 characters, including the terminating null byte ('\0').
// The p_param is the pointer to the parameter you want to pass to thread when wake it up.
#define RTK_TSK_DECLARE(tsk_func, tsk_name, p_param)                        \
    RTK_TSK_WAIT_WAKEUP_DECLARE(tsk_func);                                  \
    static int tsk_func(void *pv);                                          \
    static struct task_struct *p_RTK_TSK_##tsk_func = NULL;                 \
    static bool RTK_TSK_##tsk_func##_created = false;                       \
    void *p_RTK_TSK_##tsk_func##_param = (void *) p_param;                  \
                                                                            \
    void rtk_tsk_create_##tsk_func(void) {                                  \
        int err;                                                            \
                                                                            \
        if(RTK_TSK_##tsk_func##_created == false) {                         \
            p_RTK_TSK_##tsk_func = kthread_create(tsk_func, NULL, tsk_name);\
                                                                            \
            if(p_RTK_TSK_##tsk_func) {                                      \
                wake_up_process(p_RTK_TSK_##tsk_func);                      \
                RTK_TSK_##tsk_func##_created = true;                        \
            } else {                                                        \
                err = PTR_ERR(p_RTK_TSK_##tsk_func);                        \
                rtd_printk(KERN_WARNING, RTK_TSK_TAG_NAME, "Unable to start %s (err_id = %d).", #tsk_func, err); \
            }                                                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    void rtk_tsk_delete_##tsk_func(void) {                                  \
        int ret;                                                            \
        if(RTK_TSK_##tsk_func##_created) {                                  \
            ret = kthread_stop(p_RTK_TSK_##tsk_func);                       \
            if(!ret) {                                                      \
                p_RTK_TSK_##tsk_func = NULL;                                \
                RTK_TSK_##tsk_func##_created = false;                       \
                rtd_printk(KERN_WARNING, RTK_TSK_TAG_NAME, "%s thread stopped\n", #tsk_func); \
            }                                                               \
        }                                                                   \
    }


#ifdef __cplusplus
}
#endif

#endif  // __RTK_UTIL_TASKS__