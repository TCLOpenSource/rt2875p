/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#include "audio_base.h"
#include <audio_rpc.h>
#include <linux/slab.h>


#define DUMMY_htonl(_X_)  htonl(_X_)
#define DUMMY_htons(_X_)  htons(_X_)
#define DUMMY_ntohs(_X_)  ntohs(_X_)
#define DUMMY_ntohl(_X_)  ntohl(_X_)

static UINT32 ResetInRingBuf(Base* pBaseObj, UINT32 pinID)
{
    UINT32 res;
    AUDIO_RPC_RINGBUFFER_HEADER header;

    if(pBaseObj->inPin == NULL && pinID == UNINIT_PINID) return S_FALSE;

    /* return OK for system-only module */
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    memset(&header, 0, sizeof(AUDIO_RPC_RINGBUFFER_HEADER));
    header.instanceID = pBaseObj->GetAgentID(pBaseObj);
    header.pinID      = pinID;
    header.readIdx    = 0;

    res = RTKAUDIO_RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(&header);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

static UINT32 ResetInPin(Base* pBaseObj)
{
    DEBUG("[%s][%s]\n", __FUNCTION__, pBaseObj->name);
    if(pBaseObj->inPin != NULL)
    {
        ResetInRingBuf(pBaseObj, pBaseObj->GetInPinID(pBaseObj));
        pBaseObj->inPin = NULL;
    }
    return S_OK;
}

static UINT32 InitInRingBuf(Base* pBaseObj, UINT32 pinID)
{
    SINT32 i;
    UINT32 pAddr;
    UINT32 res;
    RINGBUFFER_HEADER *vAddr;
    AUDIO_RPC_RINGBUFFER_HEADER header;

    if(pBaseObj->inPin == NULL || pinID  == UNINIT_PINID) return S_FALSE;

    /* return OK for system-only module */
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    DEBUG("[%s][%s]\n", __FUNCTION__, pBaseObj->name);

    header.instanceID = pBaseObj->GetAgentID(pBaseObj);
    header.pinID      = pinID;
    header.readIdx    = 0;
    header.listSize   = pBaseObj->inPin->GetListSize(pBaseObj->inPin);

    pBaseObj->inPin->GetBufHeader(pBaseObj->inPin, (void**)&vAddr, &pAddr);
    header.pRingBufferHeaderList[0] = (unsigned long)pAddr;

    if(header.listSize > 1)
    {
        MultiPin* pMObj = (MultiPin*)pBaseObj->inPin->pDerivedObj;
        for(i = 1; i < header.listSize; i++)
        {
            pMObj->pPinObjArr[i-1]->GetBufHeader(pMObj->pPinObjArr[i-1], (void**)&vAddr, &pAddr);
            header.pRingBufferHeaderList[i] = (unsigned long)pAddr;
        }
    }

    res = RTKAUDIO_RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(&header);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

static UINT32 SetInPin(Base* pBaseObj, Pin* pPinObj)
{
    if(pPinObj == NULL) return S_FALSE;
    if(pBaseObj->inPin != NULL)
    {
        ResetInPin(pBaseObj);
    }

    INFO("[%s][%s]\n", __FUNCTION__, pBaseObj->name);
    pBaseObj->inPin = pPinObj;

    if(pBaseObj->state == STATE_RUN)
    {
        /*Avoid InitInRingBuf after Quick Show*/
        INFO("[QuickShow][%s] return to avoid InitInRingBuf\n", __FUNCTION__);
        return S_OK;
    }

    if(InitInRingBuf(pBaseObj, pBaseObj->GetInPinID(pBaseObj)) != S_OK)
    {
        ResetInPin(pBaseObj);
        return S_FALSE;
    }

    return S_OK;
}

UINT32 Run(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID = pBaseObj->GetAgentID(pBaseObj);

    pBaseObj->state = STATE_RUN;

    /* return OK for system-only module */
    if(instanceID == UNDEFINED_AGENT_ID) return S_OK;

    res = RTKAUDIO_RPC_TOAGENT_RUN_SVC(&instanceID);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 Pause(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID = pBaseObj->GetAgentID(pBaseObj);

    pBaseObj->state = STATE_PAUSE;

    /* return OK for system-only module */
    if(instanceID == UNDEFINED_AGENT_ID) return S_OK;

    res = RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(&instanceID);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 Stop(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID = pBaseObj->GetAgentID(pBaseObj);

    pBaseObj->state = STATE_STOP;

    /* return OK for system-only module */
    if(instanceID == UNDEFINED_AGENT_ID) return S_OK;

    res = RTKAUDIO_RPC_TOAGENT_STOP_SVC(&instanceID);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 Flush(Base* pBaseObj)
{
    UINT32 res;
    AUDIO_RPC_SENDIO sendIO;

    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    sendIO.pinID      = pBaseObj->GetInPinID(pBaseObj);
    sendIO.instanceID = pBaseObj->GetAgentID(pBaseObj);

    res = RTKAUDIO_RPC_TOAGENT_FLUSH_SVC(&sendIO);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 SetRefClock(Base* pBaseObj, UINT32 refClockPhyAddr)
{
    UINT32 res;
    AUDIO_RPC_REFCLOCK rpc_refclock;

    /* return OK for system-only module */
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    rpc_refclock.instanceID  = pBaseObj->GetAgentID(pBaseObj);
    rpc_refclock.pRefClockID = pBaseObj->GetInPinID(pBaseObj);
    rpc_refclock.pRefClock   = refClockPhyAddr; // pass physical address to agent
    res = RTKAUDIO_RPC_TOAGENT_SETREFCLOCK_SVC(&rpc_refclock);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

Pin* GetOutPin(Base* pBaseObj)
{
    if(pBaseObj->outPin == NULL) return NULL;

    return pBaseObj->outPin;
}
UINT32 GetInPinID(Base* pBaseObj)
{
    return pBaseObj->inPinID;
}
UINT32 GetOutPinID(Base* pBaseObj)
{
    return pBaseObj->outPinID;
}
UINT32 GetState(Base* pBaseObj)
{
    return pBaseObj->state;
}
UINT32 GetAgentID(Base* pBaseObj)
{
    return pBaseObj->instanceID;
}
UINT32 InitOutRingBuf(Base* pBaseObj)
{
    SINT32 i;
    UINT32 pAddr;
    UINT32 res;
    RINGBUFFER_HEADER *vAddr;
    AUDIO_RPC_RINGBUFFER_HEADER header;

    if(pBaseObj->outPin == NULL || pBaseObj->GetOutPinID(pBaseObj) == UNINIT_PINID) return S_FALSE;

    /* return OK for system-only module */
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    header.instanceID = pBaseObj->GetAgentID(pBaseObj);
    header.pinID      = pBaseObj->GetOutPinID(pBaseObj);
    header.readIdx    = -1;
    header.listSize   = pBaseObj->outPin->GetListSize(pBaseObj->outPin);

    pBaseObj->outPin->GetBufHeader(pBaseObj->outPin, (void**)&vAddr, &pAddr);
    header.pRingBufferHeaderList[0] = (unsigned long)pAddr;
    if(header.listSize > 1)
    {
        MultiPin* pMObj = (MultiPin*)pBaseObj->outPin->pDerivedObj;
        for(i = 1; i < header.listSize; i++)
        {
            pMObj->pPinObjArr[i-1]->GetBufHeader(pMObj->pPinObjArr[i-1], (void**)&vAddr, &pAddr);
            if (DUMMY_ntohl(vAddr->numOfReadPtr) > 1) {
                vAddr->numOfReadPtr = DUMMY_htonl(1);
            }
            header.pRingBufferHeaderList[i] = (unsigned long)pAddr;
        }
    }

    res = RTKAUDIO_RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(&header);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 ResetOutRingBuf(Base* pBaseObj)
{
    UINT32 res;
    AUDIO_RPC_RINGBUFFER_HEADER header;

    if(pBaseObj->outPin == NULL && pBaseObj->GetOutPinID(pBaseObj) == UNINIT_PINID) return S_FALSE;

    /* return OK for system-only module */
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    memset(&header, 0, sizeof(AUDIO_RPC_RINGBUFFER_HEADER));
    header.instanceID = pBaseObj->GetAgentID(pBaseObj);
    header.pinID      = pBaseObj->GetOutPinID(pBaseObj);
    header.readIdx    = -1;

    res = RTKAUDIO_RPC_TOAGENT_INITRINGBUFFER_HEADER_SVC(&header);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 Connect(Base* pForeBaseObj, Base* pBaseObj)
{
    UINT32 res;
    AUDIO_RPC_CONNECTION connection;

    if(pForeBaseObj->GetOutPinID(pForeBaseObj) == UNINIT_PINID) return S_FALSE;
    if(pForeBaseObj->GetOutPin(pForeBaseObj)   == NULL) return S_FALSE;
    if(pBaseObj->GetInPinID(pBaseObj) == UNINIT_PINID) return S_FALSE;

    /* setup inPin and init input ring buffer */
    if(SetInPin(pBaseObj, pForeBaseObj->GetOutPin(pForeBaseObj)) != S_OK)
    {
        ERROR("[%s] SetInPin error!!\n", pBaseObj->name);
        return S_FALSE;
    }

    DEBUG("[%s][%s] fore is %s\n", pBaseObj->name, __FUNCTION__, pForeBaseObj->name);
    if(strncmp(pForeBaseObj->name, "AIN-", 4) == 0)
        ERROR("[%s:%d] it's %s, does not need to connect to flow manager\n", __FUNCTION__, __LINE__, pForeBaseObj->name);
    else
        list_add(&pBaseObj->flowList, &pForeBaseObj->flowList);

    /* return OK if connect to system-only module */
    if(pForeBaseObj->GetAgentID(pForeBaseObj) == UNDEFINED_AGENT_ID) return S_OK;
    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    /* connect */
    connection.desInstanceID = pBaseObj->GetAgentID(pBaseObj);
    connection.srcInstanceID = pForeBaseObj->GetAgentID(pForeBaseObj);
    connection.srcPinID      = pForeBaseObj->GetOutPinID(pForeBaseObj);
    connection.desPinID      = pBaseObj->GetInPinID(pBaseObj);

    if(pBaseObj->state == STATE_RUN)
    {
        /*Avoid RPC after Quick Show*/
        INFO("[QuickShow][%s] return to avoid RPC\n", __FUNCTION__);
        return S_OK;
    }

    res = RTKAUDIO_RPC_TOAGENT_CONNECT_SVC(&connection);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);

    return res;

}

static UINT32 Disconnect(Base* pForeBaseObj, Base* pBackBaseObj)
{
    if(pForeBaseObj)
    {
        if(pForeBaseObj->GetOutPinID(pForeBaseObj) == UNINIT_PINID) return S_FALSE;
        if(pForeBaseObj->GetOutPin(pForeBaseObj)   == NULL) return S_FALSE;
    }
    if(pBackBaseObj)
        if(pBackBaseObj->GetInPinID(pBackBaseObj) == UNINIT_PINID) return S_FALSE;

    /* return OK if connect to system-only module */
    if(pForeBaseObj)
        if(pForeBaseObj->GetAgentID(pForeBaseObj) == UNDEFINED_AGENT_ID) return S_OK;
    if(pBackBaseObj)
        if(pBackBaseObj->GetAgentID(pBackBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    /*disconnect */
    if( (pForeBaseObj != NULL)  && (pBackBaseObj != NULL))
    {
        UINT32 res;
        AUDIO_RPC_CONNECTION connection;
        connection.desInstanceID = pBackBaseObj->GetAgentID(pBackBaseObj);
        connection.srcInstanceID = pForeBaseObj->GetAgentID(pForeBaseObj);
        connection.srcPinID      = pForeBaseObj->GetOutPinID(pForeBaseObj);
        connection.desPinID      = pBackBaseObj->GetInPinID(pBackBaseObj);
        res = RTKAUDIO_RPC_TOAGENT_DISCONNECT_SVC(&connection);
        if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pForeBaseObj->name,__FUNCTION__);
        else DEBUG("[%s][%s] RPC return = S_OK\n",pForeBaseObj->name,__FUNCTION__);
    }
    else
    {
        DEBUG("[%s] no send RPC ForeBase = %llx pBackBaseObj = %llx \n", __FUNCTION__,(uint64_t)pForeBaseObj, (uint64_t)pBackBaseObj);
    }

    return S_OK;
}


UINT32 Remove(Base* pBaseObj)
{
    Base* pPrevObj = NULL;
    Base* pNextObj = NULL;

    if(list_empty(&pBaseObj->flowList))
    {
        DEBUG("[%s] Module(%s) is empty \n", __FUNCTION__, pBaseObj->name);
    }
    else
    {
        pNextObj = container_of(pBaseObj->flowList.next, Base, flowList);
        DEBUG("[%s] Module(%s) next is %s \n", __FUNCTION__, pBaseObj->name, pNextObj->name);
        if(pNextObj->GetAgentID(pNextObj) == UNDEFINED_AGENT_ID)
        {
            DEBUG("[%s] Module(%s) do not find next moduel id = %x \n", __FUNCTION__, pBaseObj->name, pNextObj->GetAgentID(pNextObj));
            pNextObj = NULL; // not realy fw moduel
        }
        pPrevObj = container_of(pBaseObj->flowList.prev, Base, flowList);
        DEBUG("[%s] Module(%s) prev is %s \n", __FUNCTION__, pBaseObj->name, pPrevObj->name);
        if(pPrevObj->GetAgentID(pPrevObj) == UNDEFINED_AGENT_ID)
        {
            DEBUG("[%s] Module(%s) do not find prev moduel id = %x \n", __FUNCTION__, pBaseObj->name, pPrevObj->GetAgentID(pPrevObj));
            pPrevObj = NULL; // not realy fw moduel
        }
    }

    // disconnect output
    if(pNextObj)
        Disconnect(pBaseObj, pNextObj);

    // disconnect input
    if(pPrevObj)
        Disconnect(pPrevObj, pBaseObj);

    // remove from filter
    list_del_init(&pBaseObj->flowList);
    if(pNextObj)
        ResetInPin(pNextObj);
    ResetInPin(pBaseObj);

    return S_OK;
}

UINT32 SetSeekingInfo(Base* pBaseObj, SINT32 Speed, SINT32 Skip)
{
    UINT32 res;
    AUDIO_RPC_SEEK  seekInfo;

    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_OK;

    seekInfo.instanceID = pBaseObj->GetAgentID(pBaseObj);
    seekInfo.skip  = Skip;
    seekInfo.speed = Speed;
    res = RTKAUDIO_RPC_TOAGENT_SETSEEKING_SVC(&seekInfo);
    if(res != S_OK) ERROR("[%s][%s] RPC return != S_OK\n",pBaseObj->name,__FUNCTION__);
    else DEBUG("[%s][%s] RPC return = S_OK\n",pBaseObj->name,__FUNCTION__);
    return res;
}

UINT32 SetFlowEventQ(Base* pBaseObj, void* pQueue, void* pUserQueue)
{
    pBaseObj->flowEventQ = pQueue;
    pBaseObj->flowUserQ  = pUserQueue;
    return S_OK;
}
UINT32 PrivateInfo(Base* pBaseObj, UINT32 infoId, UINT8* data, UINT32 length)
{
    return S_OK; /* virtual function */
}
UINT32 SwitchFocus(Base* pBaseObj, void* data)
{
    return S_OK; /* virtual function */
}

void delete_base(Base* pBaseObj)
{
    DEBUG("[%s][%s]\n",pBaseObj->name,__FUNCTION__);
    if(pBaseObj == NULL) return;

    if(pBaseObj->GetAgentID(pBaseObj) != UNDEFINED_AGENT_ID)
    {

        if(pBaseObj->Stop)
            pBaseObj->Stop(pBaseObj);
        if(pBaseObj->Remove)
            pBaseObj->Remove(pBaseObj);
        DestroyAgent(&pBaseObj->instanceID);
    }

    if(pBaseObj->outPin != NULL)
    {
        pBaseObj->outPin->Delete(pBaseObj->outPin);
        pBaseObj->outPin = NULL;
    }

    delete_from_module_list(pBaseObj);
    kfree(pBaseObj);
    return;
}

Base* new_base(void)
{
    Base* pObj = NULL;
    char name[] = "BASE";
    pObj = (Base*)kmalloc(sizeof(Base), GFP_KERNEL);
    if(pObj == NULL)
    {
        return NULL;
    }
    memset(pObj, 0, sizeof(Base));
    memcpy(pObj->name, name, sizeof(name));
    pObj->pDerivedObj = pObj;
    pObj->instanceID  = UNDEFINED_AGENT_ID;
    pObj->inPin       = NULL;
    pObj->outPin      = NULL;
    pObj->inPinID     = UNINIT_PINID;
    pObj->outPinID    = UNINIT_PINID;
    pObj->state       = STATE_STOP;
    pObj->flowEventQ  = NULL;
    pObj->flowUserQ   = NULL;
    INIT_LIST_HEAD(&pObj->list);
    INIT_LIST_HEAD(&pObj->flowList);

    pObj->Delete          = delete_base;
    pObj->Run             = Run;
    pObj->Pause           = Pause;
    pObj->Stop            = Stop;
    pObj->Flush           = Flush;
    pObj->SetRefClock     = SetRefClock;
    pObj->GetOutPin       = GetOutPin;
    pObj->GetInPinID      = GetInPinID;
    pObj->GetOutPinID     = GetOutPinID;
    pObj->GetState        = GetState;
    pObj->GetAgentID      = GetAgentID;
    pObj->InitOutRingBuf  = InitOutRingBuf;
    pObj->ResetOutRingBuf = ResetOutRingBuf;
    pObj->PrivateInfo     = PrivateInfo;
    pObj->SwitchFocus     = SwitchFocus;
    pObj->SetSeekingInfo  = SetSeekingInfo;
    pObj->Connect         = Connect;
    pObj->Disconnect      = Disconnect;
    pObj->Remove          = Remove;
    pObj->SetFlowEventQ   = SetFlowEventQ;
    add_to_module_list(pObj);
    return pObj;
}

