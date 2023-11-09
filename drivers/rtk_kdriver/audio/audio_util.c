/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

/*#include <Util.h>*/
#include <linux/slab.h>
#include "audio_util.h"
#include "linux/list.h"

#define THREAD_RUN        (0x1)
#define THREAD_ASK_EXIT   (0x2)

#define SET_RUN(flag)            SET_FLAG(flag, THREAD_RUN)
#define IS_RUN(flag)             (HAS_FLAG(flag, THREAD_RUN)!= 0)
#define RESET_RUN(flag)          RESET_FLAG(flag, THREAD_RUN)

#define SET_ASK_EXIT(flag)       SET_FLAG(flag, THREAD_ASK_EXIT)
#define IS_ASK_EXIT(flag)        (HAS_FLAG(flag, THREAD_ASK_EXIT)!= 0)
#define RESET_ASK_EXIT(flag)     RESET_FLAG(flag, THREAD_ASK_EXIT)

static DEFINE_SEMAPHORE(Queue_sem);

bool IsEmpty(Queue* pQueue)
{
    return list_empty(&pQueue->queue_list);
}
UINT32 GetQueueCount(Queue* pQueue)
{
    UINT32 queue_count = 0;
    struct list_head *tmp_list;
    list_for_each(tmp_list, &pQueue->queue_list)
    {
        queue_count++;
    }
    return queue_count;
}
UINT32 EnQueue(Queue* pQueue, void* data)
{
    queue_t* queue_node = (queue_t*)kmalloc(sizeof(queue_t), GFP_KERNEL);
    if(queue_node == NULL)
        return S_FALSE;
    queue_node->queue_data = data;

    down(pQueue->m_sem);
    /*osal_SemWait(&pQueue->m_sem, TIME_INFINITY);*/
    list_add_tail(&queue_node->queue_list, &pQueue->queue_list);
    /*osal_SemGive(&pQueue->m_sem);*/
    up(pQueue->m_sem);

    return S_OK;
}
UINT32 DeQueue(Queue* pQueue)
{
    queue_t* queue_node;
    if(list_empty(&pQueue->queue_list)) return S_OK;

    down(pQueue->m_sem);
    /*osal_SemWait(&pQueue->m_sem, TIME_INFINITY);*/
    queue_node = container_of(pQueue->queue_list.next, queue_t, queue_list);
    list_del_init(&queue_node->queue_list);
    /*osal_SemGive(&pQueue->m_sem);*/
    up(pQueue->m_sem);

    kfree(queue_node);
    return S_OK;
}
void* GetQueue(Queue* pQueue)
{
    queue_t* queue_node = NULL;
    if(list_empty(&pQueue->queue_list)) return NULL;

    down(pQueue->m_sem);
    /*osal_SemWait(&pQueue->m_sem, TIME_INFINITY);*/
    queue_node = container_of(pQueue->queue_list.next, queue_t, queue_list);
    /*osal_SemGive(&pQueue->m_sem);*/
    up(pQueue->m_sem);

    if(queue_node != NULL) return queue_node->queue_data;
    else return NULL;
}
void del_queue(Queue* pQueue)
{
    while(!pQueue->IsEmpty(pQueue))
    {
        pQueue->DeQueue(pQueue);
    }
    /*osal_SemDestroy(&pQueue->m_sem);*/
    kfree((void*)pQueue);
    return;
}

Queue* new_queue(void)
{
    Queue* pQueue = (Queue*)kmalloc(sizeof(Queue), GFP_KERNEL);
    if(pQueue == NULL) return NULL;

    pQueue->Delete        = del_queue;
    pQueue->IsEmpty       = IsEmpty;
    pQueue->EnQueue       = EnQueue;
    pQueue->DeQueue       = DeQueue;
    pQueue->GetQueue      = GetQueue;
    pQueue->GetQueueCount = GetQueueCount;

    INIT_LIST_HEAD(&pQueue->queue_list);
    pQueue->m_sem         = &Queue_sem;
    /*osal_SemCreate(1, 0, &pQueue->m_sem);*/
    /*osal_SemGive(&pQueue->m_sem);*/

    return pQueue;
}
