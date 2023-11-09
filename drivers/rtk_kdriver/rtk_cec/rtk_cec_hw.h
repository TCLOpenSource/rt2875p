/* drivers/media/platform/rtk-cec/exynos_hdmi_cec.h
 *
 * Copyright (c) 2010, 2014 Samsung Electronics
 *		http://www.samsung.com/
 *
 * Header file for interface of Samsung Exynos hdmi cec hardware
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RTK_CEC_HW_H__
#define __RTK_CEC_HW_H__ 
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/completion.h>
#include <linux/irqreturn.h>
#include <media/cec.h>
#include "rtk_cm_buff.h"

#define RX_RING_LENGTH      16
#define TX_RING_LENGTH      16

#define PATCH_RX_DATA_PERIOD 78
#define PATCH_RX_DROP_SAME_LA_DATA 0

extern unsigned int g_cec_debug_enable; // 0: close all, 1: open all, 2: smart parsing

typedef struct {
    unsigned char enable: 1;
    unsigned char state: 7;
    cm_buff *cmb;
    unsigned long timeout;
    struct delayed_work work;
} rtk_cec_xmit;

typedef struct {
    unsigned char enable: 1;
    unsigned char state: 7;
    cm_buff *cmb;
    struct delayed_work work;
} rtk_cec_rcv;

enum {
    IDEL,
    XMIT,
    RCV
};

enum {
    RCV_OK,
    RCV_FAIL,
};

#define CEC_MODE_OFF        0
#define CEC_MODE_ON         1
#define CEC_MODE_STANDBY    2

enum cec_state {
    STATE_IDLE,
    STATE_BUSY,
    STATE_DONE,
    STATE_NACK,
    STATE_ERROR
};


struct rtk_cec_private{
    struct {
        unsigned char init: 1;
        unsigned char enable: 1;
        unsigned char standby_mode: 1;
        unsigned char state_before_suspend:1;		
    } status;
    rtk_cec_xmit xmit;
    rtk_cec_rcv rcv;
    spinlock_t lock;

    struct cec_adapter	*adap;
    struct device		*dev;

    enum cec_state		rx_state;
    enum cec_state		tx_state;
    struct cec_msg		rx_msg;
    struct cec_msg		tx_msg;

};

#ifdef CONFIG_ARM64
extern u32 gic_irq_find_mapping(u32 hwirq);
#define IRQ_GET_KERNEL_IRQ_NUM(hwirq)   gic_irq_find_mapping(hwirq)
#else
#define IRQ_GET_KERNEL_IRQ_NUM(hwirq)   (hwirq)
#endif



int rtk_cec_hw_init(struct rtk_cec_private *p_this, unsigned int irq);
void rtk_cec_hw_uninit(struct rtk_cec_private *p_this, unsigned int irq);
int rtk_cec_hw_enable(struct rtk_cec_private *p_this, bool enable);
int rtk_cec_hw_set_rx_mask(struct rtk_cec_private *p_this, unsigned short rx_mask);
int rtk_cec_hw_set_retry_num(struct rtk_cec_private *p_this, unsigned long num);
int rtk_cec_hw_xmit_message(struct rtk_cec_private *p_this, struct cec_msg *msg);
irqreturn_t rtk_cec_irq_handler(int this_irq, void *dev_id);
void rtk_cec_dump_current_setting(char *buf, int in_standby);
int rtk_cec_sysfs_msg_out(struct cec_msg *msg);
int rtk_cec_hw_suspend(struct rtk_cec_private *p_this);
int rtk_cec_hw_resume(struct rtk_cec_private *p_this);

#endif
