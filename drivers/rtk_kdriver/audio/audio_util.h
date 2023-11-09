/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#ifndef _RTK_UTIL_
#define _RTK_UTIL_

#include <linux/fs.h>
#include "audio_inc.h"

typedef struct queue_t{
    struct list_head queue_list;
    void* queue_data;
} queue_t;

//typedef struct _QUEUE Queue;
typedef struct _QUEUE{
    struct semaphore *m_sem;
    struct list_head queue_list;

    void    (*Delete)(struct _QUEUE*);
    bool    (*IsEmpty)(struct _QUEUE*);
    UINT32  (*EnQueue)(struct _QUEUE*, void*);
    UINT32  (*DeQueue)(struct _QUEUE*);
    void*   (*GetQueue)(struct _QUEUE*);
    UINT32  (*GetQueueCount)(struct _QUEUE*);
} Queue;
Queue* new_queue(void);
#endif
