/*
 *  Driver for 8250/16550-type serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __RTK_UART_H__
#define __RTK_UART_H__

#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/dmaengine.h>
#include "../../tty/serial/8250/8250.h"
#include "rtk_uart_dma.h"

#define UPF_NO_TXEN_TEST        ((__force upf_t) (1 << 15))

struct rtk_old_serial_port {
    unsigned int uart;
    unsigned int baud_base;
    unsigned int port;
    unsigned int irq;
    upf_t        flags;
    unsigned char hub6;
    unsigned char io_type;
    unsigned char __iomem *iomem_base;
    unsigned short iomem_reg_shift;
    unsigned long irqflags;
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
    unsigned char *dma_iomem_base;
    unsigned int dma_ring_buf_len;
    unsigned int dma_tx_thd;
    unsigned int dma_tx_empty_thd;
#endif
};

struct rtk_uart_port
{
	struct uart_8250_port up;
#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
	unsigned char       disable_printk;
	struct          delayed_work dwork;
#endif
#if defined(CONFIG_REALTEK_UART_DMA) || defined(CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA)
	struct uart_8250_rtk_dma *rtk_dma;
	struct uart_8250_rtk_dma_config rtk_dma_config;
#endif
};

static inline struct rtk_uart_port *up_to_rtk_uart_port(struct uart_8250_port *uart_8250)
{
	return container_of(uart_8250, struct rtk_uart_port, up);
}

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
extern int rtk_serial8250_request_dma(struct uart_8250_port *);
extern void rtk_serial8250_release_dma(struct uart_8250_port *);
extern void rtk_serial8250_tx_dma(struct uart_8250_port *);
extern void rtk_serial8250_dma_wait_for_xmitr(struct uart_8250_port *up);
extern void rtk_serial8250_dma_flush_buffer(struct uart_8250_port *up);
extern void rtk_serial8250_dma_console_write(struct uart_port *port,
        const unsigned char *buf, int count);
#endif

void wait_for_xmitr(struct uart_8250_port *up, int bits);

__attribute__((unused)) static inline int rtk_serial_in(struct uart_8250_port *up, int offset)
{
	return up->port.serial_in(&up->port, offset);
}


#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
#define _serial_out(up, offset, value)  \
        (up->port.serial_out(&(up)->port, (offset), (value)))
__attribute__((unused)) static void rtk_serial_out(struct uart_8250_port *up, int offset, int value)
{
    struct rtk_uart_port *rtk_up = up_to_rtk_uart_port(up);
    if (rtk_up->disable_printk && offset == UART_TX)
        return;
   _serial_out(up, offset, value);
}
#else
__attribute__((unused)) static inline void rtk_serial_out(struct uart_8250_port *up, int offset, int value)
{
    up->port.serial_out(&up->port, offset, value);
}
#endif


#if 0
#define DEBUG_INTR(fmt...)  rtd_pr_uart_info(fmt)
#else
#define DEBUG_INTR(fmt...)  do { } while (0)
#endif

#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
typedef int (*rtice_uart_callback_func)(unsigned char SBUF);
void rtk_serial8250_uart_regedist_rtice_callback(rtice_uart_callback_func func);
#endif
#endif
