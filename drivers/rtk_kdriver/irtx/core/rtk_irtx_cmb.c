#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "rtk_irtx.h"
#include "rtk_irtx_cmb.h"

void irtx_cmb_queue_head_init(IRTX_CMD_BUFF_HEAD *head)
{
    spin_lock_init(&head->lock);
    INIT_LIST_HEAD(&head->list);
    head->qlen = 0;
}

void irtx_cmb_queue_head(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb)
{
    unsigned long flags;
    spin_lock_irqsave(&head->lock, flags);

    list_add(&cmb->list, &head->list);
    head->qlen++;

    spin_unlock_irqrestore(&head->lock, flags);
}

void irtx_cmb_queue_tail(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb)
{
    unsigned long flags;
    spin_lock_irqsave(&head->lock, flags);

    list_add_tail(&cmb->list, &head->list);
    head->qlen++;

    spin_unlock_irqrestore(&head->lock, flags);
}

void irtx_cmb_dequeue_find(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb)
{
    unsigned long flags;
    IRTX_CMD_BUFF *cmb_temp = NULL;

    spin_lock_irqsave(&head->lock, flags);

    cmb_temp = list_entry(head->list.next, IRTX_CMD_BUFF, list);

    while (cmb_temp->list.next != &head->list)
    {
        if(cmb == cmb_temp)
        {
            list_del_init(&cmb->list);
            head->qlen--;
            break;
        }
        cmb_temp = list_entry(cmb_temp->list.next, IRTX_CMD_BUFF, list);
    }

    spin_unlock_irqrestore(&head->lock, flags);

}


IRTX_CMD_BUFF *irtx_cmb_dequeue(IRTX_CMD_BUFF_HEAD *head)
{
    IRTX_CMD_BUFF *cmb = NULL;
    unsigned long flags;
    spin_lock_irqsave(&head->lock, flags);

    if (!list_empty(&head->list))
    {
        cmb = list_entry(head->list.next, IRTX_CMD_BUFF, list);
        list_del_init(&cmb->list);
        head->qlen--;
    }

    spin_unlock_irqrestore(&head->lock, flags);

    return cmb;
}

IRTX_CMD_BUFF *irtx_cmb_dequeue_tail(IRTX_CMD_BUFF_HEAD *head)
{
    IRTX_CMD_BUFF *cmb = NULL;
    unsigned long flags;
    spin_lock_irqsave(&head->lock, flags);

    if (!list_empty(&head->list))
    {
        cmb = list_entry(head->list.prev, IRTX_CMD_BUFF, list);
        list_del_init(&cmb->list);
        head->qlen--;
    }

    spin_unlock_irqrestore(&head->lock, flags);

    return cmb;
}

unsigned int irtx_cmb_queue_len(const IRTX_CMD_BUFF_HEAD *head)
{
    return head->qlen;
}

void irtx_cmb_queue_purge(IRTX_CMD_BUFF_HEAD *head)
{
    IRTX_CMD_BUFF *cmb;
    unsigned long flags;
    spin_lock_irqsave(&head->lock, flags);

    while ((cmb = irtx_cmb_dequeue(head)) != NULL)
        irtx_kfree_cmb(cmb);

    spin_unlock_irqrestore(&head->lock, flags);
}

IRTX_CMD_BUFF *irtx_alloc_cmb(size_t size)
{
    IRTX_CMD_BUFF *cmb = (IRTX_CMD_BUFF *) kzalloc(sizeof(IRTX_CMD_BUFF) + size, GFP_KERNEL);
    if (cmb)
    {
        INIT_LIST_HEAD(&cmb->list);
        init_waitqueue_head(&cmb->wq);
        cmb->status = 0;
        cmb->flags = 0;
    }

    return cmb;
}

void irtx_kfree_cmb(IRTX_CMD_BUFF *cmb)
{
    if (cmb)
        kfree(cmb);
}
