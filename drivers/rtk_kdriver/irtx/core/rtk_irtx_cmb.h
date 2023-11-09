#ifndef __RTK_IRTX_CMB_H__
#define __RTK_IRTX_CMB_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/wait.h>

typedef struct irtx_cmd
{
    unsigned int timeout_ms;
    unsigned int timing_unit;/*ns*/
    unsigned int pwm_en;
    unsigned int pwm_duty;
    unsigned int pwm_freq;
    unsigned int timing_info_len;
    unsigned int timing_info[0];/*bit31 represents high or low level, bit30-0 represent timing length*/
} IRTX_CMD;

#define IRTX_GET_TIMING_LEVEL(x)           (((x) >> 31) & 0x1)
#define IRTX_GET_TIMING_LENGTH(x)           ((x) & 0x7FFFFFFF)
#define IRTX_EACH_TIMING_INFO_LEN               (sizeof(((IRTX_CMD *)0)->timing_info[0]))



typedef struct
{
    struct list_head list;
    unsigned char status;
#define WAIT_XMIT       0xff
#define XMIT_OK         0x00
#define XMIT_FAIL       0x01
#define XMIT_TIMEOUT    0x02
#define XMIT_ABORT      0x03

    unsigned char flags;
#define NONBLOCK        0x80
#define TX_MSG            0x08
    wait_queue_head_t wq;

    unsigned int cur_timing_info_index;
    IRTX_CMD cmd;
} IRTX_CMD_BUFF;

typedef struct
{
    spinlock_t lock;
    struct list_head list;
    unsigned int qlen;
} IRTX_CMD_BUFF_HEAD;

void irtx_cmb_queue_head_init(IRTX_CMD_BUFF_HEAD *head);
void irtx_cmb_queue_head(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb);
void irtx_cmb_queue_tail(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb);
void irtx_cmb_dequeue_find(IRTX_CMD_BUFF_HEAD *head, IRTX_CMD_BUFF *cmb);
IRTX_CMD_BUFF *irtx_cmb_dequeue(IRTX_CMD_BUFF_HEAD *head);
IRTX_CMD_BUFF *irtx_cmb_dequeue_tail(IRTX_CMD_BUFF_HEAD *head);
void irtx_cmb_queue_purge(IRTX_CMD_BUFF_HEAD *head);
unsigned int irtx_cmb_queue_len(const IRTX_CMD_BUFF_HEAD *head);

IRTX_CMD_BUFF *irtx_alloc_cmb(size_t size);
void irtx_kfree_cmb(IRTX_CMD_BUFF *cmb);

#endif
