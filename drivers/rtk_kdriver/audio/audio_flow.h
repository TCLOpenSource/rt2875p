/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#ifndef _RTK_FLOW_H_
#define _RTK_FLOW_H_

#include <linux/fs.h>
#include "audio_inc.h"
#include "audio_util.h"

typedef void (*pfnCallBack)(SINT32 Data);

//typedef struct _FlowManager FlowManager;
typedef struct _FlowManager{
    void* pDerivedFlow;
    Base* pBaseObj;
    ReferenceClock *pRefClock;
    UINT32 state;
    UINT32 extRefClock_phy;
    Queue* pEventQueue;
    Queue* pUserQueue;
    struct semaphore* m_eos_sem;
    pfnCallBack eos_callback_func;
    SINT32 eos_callback_data;

    UINT32 (*Connect)(struct _FlowManager*, Base*, Base*);
    UINT32 (*Remove)(struct _FlowManager*, Base*);
    UINT32 (*Run)(struct _FlowManager*);
    UINT32 (*Stop)(struct _FlowManager*);
    UINT32 (*Pause)(struct _FlowManager*);
    UINT32 (*Flush)(struct _FlowManager*);
    UINT32 (*SetRate)(struct _FlowManager*, SINT32);
    UINT32 (*GetState)(struct _FlowManager*);
    UINT32 (*SetExtRefClock)(struct _FlowManager*, UINT32);
    UINT32 (*SetEOSCallback)(struct _FlowManager*, pfnCallBack, SINT32);
    UINT32 (*Search)(struct _FlowManager*, Base*);
    void   (*Delete)(struct _FlowManager*);
    UINT32 (*ShowCurrentExitModule)(struct _FlowManager*, int forcePrint);
    UINT32 (*CheckExistModule)(struct _FlowManager*, Base*);
}FlowManager;
FlowManager* new_flow(void);

#endif
