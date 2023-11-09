/*Copyright (C) 2007-2013 Realtek Semiconductor Corporation.*/
#ifndef __RTK_IRTX_PRIV_H__
#define __RTK_IRTX_PRIV_H__

#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/completion.h>
#include "../core/rtk_irtx.h"
#include "../core/rtk_irtx_cmb.h"

extern u32 gic_irq_find_mapping(u32 hwirq);
#define IRQ_GET_KERNEL_IRQ_NUM(hwirq)       gic_irq_find_mapping(hwirq)

#define IRTX_TX_TIMEOUT      (HZ<<2)

#define ID_RTK_IRTX_CONTROLLER     0x6789
#define IRTX_MODEL_NAME             ("RTK HW")
#define IRTX_DRIVE_NAME              ("rtk_irtx")

typedef struct {
    unsigned char enable: 1;
    unsigned char state: 7;
    IRTX_CMD_BUFF *cmb;
    unsigned long timeout;
    struct delayed_work work;
} rtk_irtx_xmit;

enum {
    IDEL,
    XMIT,
};

#define IRTX_MODE_OFF        0
#define IRTX_MODE_ON         1

typedef struct {
    struct {
        unsigned char init: 1;
        unsigned char enable: 1;	
        unsigned char state_before_suspend:1;
    } status;

    rtk_irtx_xmit xmit;
    IRTX_CMD_BUFF_HEAD tx_queue;
    spinlock_t lock;

} rtk_irtx;

#endif
