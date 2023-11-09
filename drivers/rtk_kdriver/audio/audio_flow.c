/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#include <linux/slab.h>
#include "audio_flow.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/freezer.h>

#include "audio_inc.h"

enum{
    ENUM_MODULE_MAIN_AIN  = 0,
    ENUM_MODULE_SUB_AIN   = 1,
    ENUM_MODULE_MEMIN     = 2,
    ENUM_MODULE_MEMOUT    = 3,
    ENUM_MODULE_DEC       = 4,
    ENUM_MODULE_ENC       = 5,
    ENUM_MODULE_MAIN_PPAO = 6,
    ENUM_MODULE_SUB_PPAO  = 7,
    ENUM_MODULE_MAX
};

const char *ModuleName[] = {
    "Main-AIN",
    "Sub-AIN",
    "MEMIN",
    "MEMOUT",
    "DEC",
    "ENC",
    "MainPPAO",
    "SubPPAO"
};

#define DEADLOCK_TIMEOUT (2)

static struct task_struct *rtkaudio_flow_tsk;
static DEFINE_SEMAPHORE(Flow_sem);

static int CheckEventProcess(void* pData)
{
    FlowManager* pFlow = (FlowManager*)pData;
    /*THREAD* pThread = pFlow->pEventThread;*/
    Queue* pEventQueue = pFlow->pEventQueue;
    Queue* pUserQueue = pFlow->pUserQueue;

    /*while (pThread->IsAskToExit(pThread) == false)*/
    for (;;)
    {
        set_freezable();

        if (kthread_should_stop())
            break;

        if(pEventQueue->IsEmpty(pEventQueue))
        {
            /*osal_Sleep(50); [> sleep 50 ms <]*/
            msleep(50);
        }
        else
        {
            uint64_t eventID = (uint64_t)pEventQueue->GetQueue(pEventQueue);
            switch(eventID)
            {
                case EVENT_EOS:
                    DEBUG("[Flow] Get Event(%lld)\n", eventID);
#ifdef HANDLE_EOS_IN_FLOW
                    down(pFlow->m_eos_sem);
                    /*osal_SemWait(&pFlow->m_eos_sem, TIME_INFINITY);*/
                    if(pFlow->eos_callback_func)
                    {
                        DEBUG("[Flow] EOS Callback\n");
                        if(pFlow->eos_callback_func)
                            pFlow->eos_callback_func(pFlow->eos_callback_data);
                    }
                    /*osal_SemGive(&pFlow->m_eos_sem);*/
                    up(pFlow->m_eos_sem);
#else
                    pUserQueue->EnQueue(pUserQueue, (void*)EVENT_EOS);
#endif
                    break;
                default:
                    DEBUG("[Flow] Get Event(%lld)\n", eventID);
                    break;
            }
            pEventQueue->DeQueue(pEventQueue);
            /*osal_Sleep(50);*/
            msleep(50);
        }
    }

    return 0;
}

UINT32 Flow_DirectRun(FlowManager* pFlow)
{
    struct list_head* list;
    Base* pBaseObj;
    Base* pFlowBaseObj;
    UINT32 refClockPhyAddr;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;

    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;


    if(pFlow->extRefClock_phy)
    {
        /* For DTV source, use external reference clock */
        refClockPhyAddr = pFlow->extRefClock_phy;
    }
    else
    {
        /* use internal reference clock */
        ReferenceClock* refClock = pFlow->pRefClock;
        refClock->GetCoreAddress(refClock, &refClockPhyAddr, 0, 0);
    }

    get_ms = getpts();
    list_for_each_prev(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Run(pBaseObj);
        DEBUG("[FLOW] %s Run\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    pFlow->state = STATE_RUN;

    return S_OK;
}

UINT32 Flow_Run(FlowManager* pFlow)
{
    struct list_head* list;
    Base* pBaseObj;
    Base* pFlowBaseObj;
    UINT32 refClockPhyAddr;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;

    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;

    if(pFlow->extRefClock_phy)
    {
        /* For DTV source, use external reference clock */
        refClockPhyAddr = pFlow->extRefClock_phy;
    }
    else
    {
        /* use internal reference clock */
        ReferenceClock* refClock = pFlow->pRefClock;
        refClock->GetCoreAddress(refClock, &refClockPhyAddr, 0, 0);
    }

    get_ms = getpts();
    /* pass physical address to agent */
        list_for_each_prev(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
            pBaseObj = container_of(list, Base, flowList);
        pBaseObj->SetRefClock(pBaseObj, refClockPhyAddr);
        DEBUG("[FLOW] set RefClock for %s\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }

    get_ms = getpts();
    list_for_each_prev(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Pause(pBaseObj);
        DEBUG("[FLOW] %s Pause\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
                ERROR("[%s] flow connect timeout\n", __FUNCTION__);
                /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    get_ms = getpts();
    list_for_each_prev(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Run(pBaseObj);
        DEBUG("[FLOW] %s Run\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    pFlow->state = STATE_RUN;

    return S_OK;
}
UINT32  Flow_Stop(FlowManager* pFlow)
{
    struct list_head* list;
    Base* pBaseObj;
    Base* pFlowBaseObj;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;

    DEBUG("[%s] [%s]\n", __FUNCTION__, pFlowBaseObj->name);

    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;

    if(pFlow->state == STATE_RUN)
    {
        get_ms = getpts();
        list_for_each(list, &pFlowBaseObj->flowList)
        {
            if(list == &pFlowBaseObj->flowList) continue;
            pBaseObj = container_of(list, Base, flowList);
            pBaseObj->Pause(pBaseObj);
            DEBUG("[FLOW] %s Pause\n", pBaseObj->name);

            if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
            {
                ERROR("[%s] flow connect timeout\n", __FUNCTION__);
                /*system("echo fw@ check_lock > /dev/rtkaudio");*/
                break;
            }
        }
    }

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Stop(pBaseObj);
        DEBUG("[FLOW] %s Stop\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    pFlow->state = STATE_STOP;

    return S_OK;
}
UINT32  Flow_Pause(FlowManager* pFlow)
{
    struct list_head* list;
    Base* pBaseObj;
    Base* pFlowBaseObj;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;


    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Pause(pBaseObj);
        DEBUG("[FLOW] %s Pause\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    pFlow->state = STATE_PAUSE;

    return S_OK;
}
UINT32  Flow_Flush(FlowManager* pFlow)
{
    struct list_head* list;
    Base* pBaseObj;

    Base* pFlowBaseObj;
    pFlowBaseObj = pFlow->pBaseObj;

    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;

    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->Flush(pBaseObj);
        DEBUG("[FLOW] %s Flush\n", pBaseObj->name);
        /* Send Flush RPC to first filter, f/w will flush all connected agents */
        break;
    }

    return S_OK;
}
UINT32  Flow_Connect(FlowManager* pFlow, Base* pForeBase, Base* pBackBase)
{
    Base* pFlowBaseObj;
    struct list_head* list;
    Base* pBaseObj;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;

    // Check Exist Obj
    if(!list_empty(&pFlowBaseObj->flowList))
    {
        get_ms = getpts();
        list_for_each(list, &pFlowBaseObj->flowList)
        {
            if(list == &pFlowBaseObj->flowList) continue;
            pBaseObj = container_of(list, Base, flowList);
            if(pBaseObj == pBackBase)
            {
                ERROR("[FLOW] connect %s exit in flow (%llx = %llx)\n", pBackBase->name, (uint64_t)pBaseObj , (uint64_t)pBackBase);
                INFO("[FLOW] connect %s exit in flow (%llx = %llx)\n", pBackBase->name, (uint64_t)pBaseObj , (uint64_t)pBackBase);
              /*assert(pBaseObj != pBackBase);*/
            }

            if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
            {
                ERROR("[%s] flow connect timeout\n", __FUNCTION__);
                /*system("echo fw@ check_lock > /dev/rtkaudio");*/
                break;
            }
        }
    }

    /* Let pFlow->flowList be the head */
    if(list_empty(&pFlowBaseObj->flowList))
    {
        DEBUG("[FLOW] Init flow to be the head\n");
        list_add(&pForeBase->flowList, &pFlowBaseObj->flowList);
    }

    pForeBase->SetFlowEventQ(pForeBase, (void*)pFlow->pEventQueue, (void*)pFlow->pUserQueue);
    pBackBase->SetFlowEventQ(pBackBase, (void*)pFlow->pEventQueue, (void*)pFlow->pUserQueue);

    DEBUG("[FLOW] Connect %s to %s\n", pForeBase->name, pBackBase->name);
    return pBackBase->Connect(pForeBase, pBackBase);
}


UINT32  Flow_Remove(FlowManager* pFlow, Base* pBaseObj)
{
    Base* pTempBase = NULL;
    Base* pFlowBaseObj = NULL;
    int64_t get_ms = 0;
    struct list_head* list;
    pFlowBaseObj = pFlow->pBaseObj;

    DEBUG("[FLOW] %s Remove\n", pBaseObj->name);
    if(list_empty(&pFlowBaseObj->flowList))
    {
        DEBUG("[FLOW] connection is empty\n");
        return S_OK;
    }

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
       if(list == &pFlowBaseObj->flowList) continue;
       pTempBase = container_of(list, Base, flowList);
       DEBUG("[FLOW] (%llx)  find  Module(%s) (%llx) prev (%llx) next (%llx)\n", (uint64_t)&pFlowBaseObj->flowList, pTempBase->name, (uint64_t)&(pTempBase->flowList), (uint64_t)(pTempBase->flowList.prev), (uint64_t)(pTempBase->flowList.next));

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pTempBase = container_of(list, Base, flowList);
        if(pTempBase == pBaseObj) break;

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }

    if(pTempBase == pBaseObj)
    {
        DEBUG("[FLOW] Module(%s) exists in flow, disconnect it\n", pBaseObj->name);
    }
    else
    {
        DEBUG("[FLOW] Module(%s) is not in flow, disconnect it\n", pBaseObj->name);
    }
    pBaseObj->SetFlowEventQ(pBaseObj, NULL, NULL);

    return pBaseObj->Remove(pBaseObj);
}

UINT32  Flow_SetRate(FlowManager* pFlow, SINT32 rate)
{
    struct list_head* list;

    Base* pFlowBaseObj;
    int64_t get_ms = 0;
    Base* pBaseObj;
    pFlowBaseObj = pFlow->pBaseObj;
    if(list_empty(&pFlowBaseObj->flowList)) return S_OK;

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);
        pBaseObj->SetSeekingInfo(pBaseObj, rate, rate);
        DEBUG("[FLOW] %s Pause\n", pBaseObj->name);

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }

    return S_OK;
}

UINT32 Flow_Search(FlowManager* pFlow, Base* pSearchObj)
{
    UINT32 existInFlow = FALSE;
    Base* pFlowBaseObj;
    struct list_head* list;
    Base* pBaseObj;
    int64_t get_ms = 0;
    pFlowBaseObj = pFlow->pBaseObj;

    // Check Exist Obj
    if(list_empty(&pFlowBaseObj->flowList))
        return existInFlow;

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);

        if(pBaseObj == pSearchObj)
        {
            existInFlow = TRUE;
            break;
        }

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }

    return existInFlow;
}

/*
 * return bitfield value, indicate exist module
 *                     0, empty module
 * */
UINT32 Flow_ShowCurrentExitModule(FlowManager* pFlow, int forcePrint)
{
    struct list_head* list;
    Base* pBaseObj;
    Base* pFlowBaseObj;
    int64_t get_ms = 0;
    unsigned int ret = 0, i;
    pFlowBaseObj = pFlow->pBaseObj;

    if(list_empty(&pFlowBaseObj->flowList))
    {
        if(forcePrint == 1)
            pr_debug("[FLOW] No Find Module \n");
        else
            INFO("[FLOW] No Find Module \n");
        return ret;
    }

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pBaseObj = container_of(list, Base, flowList);

        if(forcePrint == 1)
            pr_debug("[FLOW] find %s \n", pBaseObj->name);
        else
            INFO("[FLOW] find %s \n", pBaseObj->name);
        for(i=0; i<ENUM_MODULE_MAX; i++){
            if(!strcmp(pBaseObj->name,ModuleName[i]))
                ret |= 0x1<<i;
        }

        if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
        {
            ERROR("[%s] flow connect timeout\n", __FUNCTION__);
            /*system("echo fw@ check_lock > /dev/rtkaudio");*/
            break;
        }
    }
    return ret;
}

UINT32 Flow_CheckExistModule(FlowManager* pFlow, Base* pBaseObj){

    struct list_head* list;
    Base* pFlowBaseObj, *pCurrBaseObj;
    int64_t get_ms = 0;
    /*unsigned int ret = 0, i;*/
    pFlowBaseObj = pFlow->pBaseObj;

    if(list_empty(&pFlowBaseObj->flowList))
    {
        INFO("[FLOW] No Find Module \n");
    }

    get_ms = getpts();
    list_for_each(list, &pFlowBaseObj->flowList)
    {
        if(list == &pFlowBaseObj->flowList) continue;
        pCurrBaseObj = container_of(list, Base, flowList);
        if(pCurrBaseObj == pBaseObj){
            INFO("[FLOW] Find Module\n");
            return 1;
        }
    }
    return 0;
}

UINT32 Flow_GetState(FlowManager* pFlow)
{
    return pFlow->state;
}

UINT32 Flow_SetExtRefClock(FlowManager* pFlow, UINT32 phyAddr)
{
    pFlow->extRefClock_phy = phyAddr;
    DEBUG("[FLOW][%s] physical address:%x\n", __FUNCTION__, phyAddr);
    return S_OK;
}
UINT32 Flow_SetEOSCallback(FlowManager* pFlow, pfnCallBack callback_func, SINT32 callback_data)
{
    down(pFlow->m_eos_sem);
    /*osal_SemWait(&pFlow->m_eos_sem, TIME_INFINITY);*/
    pFlow->eos_callback_func = callback_func;
    pFlow->eos_callback_data = callback_data;
    /*osal_SemGive(&pFlow->m_eos_sem);*/
    up(pFlow->m_eos_sem);
    return S_OK;
}

void delete_flow(FlowManager* pFlow)
{
    int64_t get_ms = 0;
    Base* pFlowBaseObj = NULL;
    if(pFlow == NULL) return;
    pFlowBaseObj = pFlow->pBaseObj;;

    if(!list_empty(&pFlowBaseObj->flowList))
    {
        struct list_head *list, *tmp_list;
        Base* pBaseObj;
        get_ms = getpts();
        list_for_each_safe(list, tmp_list, &pFlowBaseObj->flowList)
        {
            if(list == &pFlowBaseObj->flowList) continue;
            pBaseObj = container_of(list, Base, flowList);
            pBaseObj->SetFlowEventQ(pBaseObj, NULL, NULL);
            pFlow->Remove(pFlow, pBaseObj);

            if (((getpts() - get_ms)) / 90000 > DEADLOCK_TIMEOUT)
            {
                ERROR("[%s] flow connect timeout\n", __FUNCTION__);
                /*system("echo fw@ check_lock > /dev/rtkaudio");*/
                break;
            }
        }
        list_del_init(&pFlowBaseObj->flowList);
    }

    /*if(pFlow->pEventThread->IsRun(pFlow->pEventThread))*/
        /*pFlow->pEventThread->Exit(pFlow->pEventThread, TRUE);*/
    /*pFlow->pEventThread->Delete(pFlow->pEventThread);*/
    if (rtkaudio_flow_tsk) {
        int ret;
        ret = kthread_stop(rtkaudio_flow_tsk);
        if (!ret)
            ERROR("rtkaudio thread stopped\n");
    }

    pFlow->pEventQueue->Delete(pFlow->pEventQueue);
    pFlow->pUserQueue->Delete(pFlow->pUserQueue);

    /*osal_SemDestroy(&pFlow->m_eos_sem);*/
    if(pFlow->pRefClock != NULL)
    {
        pFlow->pRefClock->Delete(pFlow->pRefClock);
        pFlow->pRefClock = NULL;
    }

    pFlow->pBaseObj = NULL;
    kfree(pFlow);
    return delete_base(pFlowBaseObj);

}

FlowManager* new_flow(void)
{
    /* object init */
    char name[] = "FlowManager";
    FlowManager* pFlow = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
        return NULL;

    DEBUG("[%s] \n", __FUNCTION__);

    pFlow = (FlowManager*)kmalloc(sizeof(FlowManager), GFP_KERNEL);
    if(pFlow == NULL)
    {
      pObj->Delete(pObj);
      return NULL;
    }
    memset(pFlow, 0, sizeof(FlowManager));
    memcpy(pObj->name, name, sizeof(name));
    pObj->pDerivedObj = pFlow;

    pFlow->pBaseObj = pObj;

    /* setup member functions */

//======================================================

    pFlow->Delete     = delete_flow;
    pFlow->Run        =  Flow_Run;
    pFlow->Stop       =  Flow_Stop;
    pFlow->Pause      =  Flow_Pause;
    pFlow->Flush      =  Flow_Flush;
    pFlow->Connect    =  Flow_Connect;
    pFlow->Remove     =  Flow_Remove;
    pFlow->GetState   =  Flow_GetState;
    pFlow->SetRate    =  Flow_SetRate;
    pFlow->SetExtRefClock =  Flow_SetExtRefClock;
    pFlow->SetEOSCallback =  Flow_SetEOSCallback;
    pFlow->ShowCurrentExitModule = Flow_ShowCurrentExitModule;
    pFlow->CheckExistModule = Flow_CheckExistModule;
    pFlow->Search = Flow_Search;

    pFlow->pDerivedFlow = pFlow;
    INIT_LIST_HEAD(&pObj->flowList);

    pFlow->pRefClock = new_ReferenceClock();
    if(pFlow->pRefClock == NULL)
    {
        pFlow->Delete(pFlow); // ==> will kfree(pFlow);
        pFlow = NULL;
        return pFlow;
    }
    pFlow->state = STATE_STOP;

    pFlow->pEventQueue = new_queue();
    pFlow->pUserQueue = new_queue();
    /*pFlow->pEventThread = new_thread(CheckEventProcess, (void*)pFlow);*/
    rtkaudio_flow_tsk = kthread_create(CheckEventProcess, (void*)pFlow,
        "rtkaudio_flow_tsk");
    if (IS_ERR(rtkaudio_flow_tsk)) {
        rtkaudio_flow_tsk = NULL;
        return NULL;
    }
    wake_up_process(rtkaudio_flow_tsk);

    /*osal_SemCreate(1, 0, &pFlow->m_eos_sem);*/
    /*osal_SemGive(&pFlow->m_eos_sem);*/
    pFlow->eos_callback_func = NULL;
    pFlow->eos_callback_data = 0;
    pFlow->m_eos_sem         = &Flow_sem;

    return pFlow;
}
