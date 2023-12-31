/*Copyright (C) 2007-2013 Realtek Semiconductor Corporation.*/
#ifndef __RTK_CM_BUFF_H__
#define __RTK_CM_BUFF_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/wait.h>

#define RX_BUFFER_SIZE      64

typedef struct {
    struct list_head list;
    unsigned char status;

#define WAIT_XMIT       0xff
#define XMIT_OK         0x00
#define XMIT_FAIL       0x01
#define XMIT_TIMEOUT    0x02
#define XMIT_ABORT      0x03

    wait_queue_head_t wq;
    unsigned char *head;
    unsigned char *data;
    unsigned char *tail;
    unsigned char *end;
    unsigned char len;

} cm_buff;

typedef struct {
    spinlock_t lock;
    struct list_head list;
    unsigned int qlen;
} cm_buff_head;

void cmb_queue_head_init(cm_buff_head *head);
void cmb_queue_head(cm_buff_head *head, cm_buff *cmb);
void cmb_queue_tail(cm_buff_head *head, cm_buff *cmb);
void cmb_dequeue_find(cm_buff_head *head, cm_buff *cmb);
cm_buff *cmb_dequeue(cm_buff_head *head);
cm_buff *cmb_dequeue_tail(cm_buff_head *head);
void cmb_queue_purge(cm_buff_head *head);
unsigned int cmb_queue_len(const cm_buff_head *head);

cm_buff *alloc_cmb(size_t size);
void kfree_cmb(cm_buff *cmb);

void cmb_reserve(cm_buff *cmb, unsigned int len);
unsigned char *cmb_put(cm_buff *cmb, unsigned int len);
unsigned char *cmb_push(cm_buff *cmb, unsigned int len);
unsigned char *cmb_pull(cm_buff *cmb, unsigned int len);
void cmb_purge(cm_buff *cmb);
int cmb_tailroom(const cm_buff *cmb);

#endif /*__RTK_CEC_H__*/
