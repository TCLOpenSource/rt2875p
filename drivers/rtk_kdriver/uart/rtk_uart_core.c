/*
 *  Universal/legacy driver for 8250/16550-type serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2001 Russell King.
 *
 *  Supports: ISA-compatible 8250/16550 ports
 *        PNP 8250/16550 ports
 *        early_serial_setup() ports
 *        userspace-configurable "phantom" ports
 *        "rtk_serial8250" platform devices
 *        rtk_serial8250_register_8250_port() ports
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/ratelimit.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/nmi.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/irq.h>
#ifdef CONFIG_SPARC
#include <linux/sunserialcore.h>
#endif

#include <asm/irq.h>
#include "rtk_uart.h"
#include <rbus/iso_misc_off_uart_reg.h>
#include <rbus/stb_reg.h>
#include <mach/rtk_platform.h>
//#include <mach/io.h>
#include <rtk_kdriver/io.h>
#include <mach/system.h>
#include <linux/version.h>
#include <rbus/sys_reg_reg.h>
#include <linux/of_irq.h>
#include <linux/version.h>
#include <rtd_log/rtd_module_log.h>

extern void rtk_serial8250_tx_chars(struct uart_8250_port *up);
extern void rtk_serial8250_init_port(struct uart_8250_port *up);
extern void rtk_serial8250_set_defaults(struct uart_8250_port *up);
extern void rtk_serial8250_console_write(struct uart_8250_port *up, const char *s,
                              unsigned int count);
extern void rtk_serial8250_unregister_port(int line);
extern int rtk_serial8250_console_setup(struct uart_port *port, char *options, bool probe);
extern int rtice_uart_handler(unsigned char SBUF, unsigned int dir);

/**** Stan add to ignore the printk between kernel init and busybox. BEGIN ****/
#ifdef CONFIG_REALTEK_DISABLE_PRINTK_WHEN_BOOT
int KERNEL_PRINT_ENABLE = 1;
#endif
/**** Stan add to ignore the printk between kernel init and busybox. END ****/

// Add to ignore all printk. BEGIN
#ifdef CONFIG_REALTEK_IGNORE_ALL_PRINTK
int KERNEL_PRINTALL_ENABLE = 1;
#endif

void __iomem * get_rbus_mapped_addr(unsigned long in)
{
	if (in)
		return (void __iomem *)GET_MAPPED_RBUS_ADDR(in);

	return NULL;
}

#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)

static int rtice_enable=1;
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
int rtice_port = 1;
#else
int rtice_port = 0;
#endif
int rtice_early_disable_tx_int_flag = 0;
int rtice_early_disable_tx_int = 0;



void rtk_serial8250_set_rtice_enable(int en)
{
	rtice_enable = en;
}
EXPORT_SYMBOL(rtk_serial8250_set_rtice_enable);

int rtk_serial8250_get_rtice_enable(void)
{
	return rtice_enable;
}
EXPORT_SYMBOL(rtk_serial8250_get_rtice_enable);

void rtk_serial8250_set_rtice_port(int port)
{
	rtice_port = port;
}
EXPORT_SYMBOL(rtk_serial8250_set_rtice_port);

int rtk_serial8250_get_rtice_port(void)
{
	return rtice_port;
}
EXPORT_SYMBOL(rtk_serial8250_get_rtice_port);

#endif

/*
 * Configuration:
 *   share_irqs - whether we pass IRQF_SHARED to request_irq().  This option
 *                is unsafe when used on edge-triggered interrupts.
 */
static unsigned int share_irqs = SERIAL8250_SHARE_IRQS;

static unsigned int nr_uarts = CONFIG_RTK_KDRV_SERIAL_8250_RUNTIME_UARTS;

static struct uart_driver rtk_serial8250_reg;

static unsigned int skip_txen_test; /* force skip of txen test at init time */

#define PASS_LIMIT  512

#include <asm/serial.h>
/*
 * SERIAL_PORT_DFNS tells us about built-in ports that have no
 * standard enumeration mechanism.   Platforms that can find all
 * serial ports via mechanisms like ACPI or PCI need not supply it.
 */
#ifndef SERIAL_PORT_DFNS
#define SERIAL_PORT_DFNS
#endif

static const struct rtk_old_serial_port old_serial_port[] = {
    SERIAL_PORT_DFNS /* defined in asm/serial.h */
};


#define UART_NR CONFIG_RTK_KDRV_SERIAL_8250_NR_UARTS

struct rtk_uart_port rtk_serial8250_ports[UART_NR];

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_RSA

#define PORT_RSA_MAX 4
static unsigned long probe_rsa[PORT_RSA_MAX];
static unsigned int probe_rsa_count;
#endif /* CONFIG_RTK_KDRV_SERIAL_8250_RSA  */


struct irq_info {
    struct          hlist_node node;
    int         irq;
    spinlock_t      lock;   /* Protects list not the hash */
    struct list_head    *head;
};

#define NR_IRQ_HASH     32  /* Can be adjusted later */
static struct hlist_head irq_lists[NR_IRQ_HASH];
static DEFINE_MUTEX(hash_mutex);    /* Used to walk the hash */

static unsigned int rtk_serial8250_get_clock(void *iobase)
{
	unsigned int uart_clock = 27000000;
	//rtd_pr_uart_err("%p, %p\n", iobase, GET_MAPPED_RBUS_ADDR(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg));
	if(iobase == (void *)GET_MAPPED_RBUS_ADDR(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg)) {
		unsigned int val = rtd_inl(STB_ST_CLKMUX_reg);
		if((val & 0x1) == 0)
			uart_clock = 108000000;
	} else if(iobase == (void *)GET_MAPPED_RBUS_ADDR(MISC_UART_U1RBR_THR_DLL_reg)) {
		unsigned int val = rtd_inl(SYS_REG_SYS_CLKSEL_reg);
		if((val & 0x1) == 0)
			uart_clock = 108000000;
	}
	
	return uart_clock;
}

/*
 * This is the serial driver's interrupt routine.
 *
 * Arjan thinks the old way was overly complex, so it got simplified.
 * Alan disagrees, saying that need the complexity to handle the weird
 * nature of ISA shared interrupts.  (This is a special exception.)
 *
 * In order to handle ISA shared interrupts properly, we need to check
 * that all ports have been serviced, and therefore the ISA interrupt
 * line has been de-asserted.
 *
 * This means we need to loop through all ports. checking that they
 * don't have an interrupt pending.
 */
static irqreturn_t rtk_serial8250_interrupt(int irq, void *dev_id)
{
    struct irq_info *i = dev_id;
    struct list_head *l, *end = NULL;
    int pass_counter = 0, handled = 0;

#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
    struct uart_8250_port *up_rtice;
#endif


    DEBUG_INTR("rtk_serial8250_interrupt(%d)...", irq);


#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
    up_rtice = &rtk_serial8250_ports[rtice_port].up;
    if (rtice_early_disable_tx_int == 1) {
        up_rtice->ier &= ~UART_IER_THRI;
        rtk_serial_out(up_rtice, UART_IER, up_rtice->ier);
    }
    if(rtice_early_disable_tx_int_flag == 1) {
        up_rtice->ier |= UART_IER_THRI;
        rtk_serial_out(up_rtice, UART_IER, up_rtice->ier);
        rtice_early_disable_tx_int_flag = 0;
    }
#endif


    spin_lock(&i->lock);

    l = i->head;
    do {
        struct uart_8250_port *up;
        struct uart_port *port;

        up = list_entry(l, struct uart_8250_port, list);
        port = &up->port;

        if (port->handle_irq(port)) {
            handled = 1;
            end = NULL;
        } else if (end == NULL)
            end = l;

        l = l->next;

        if (l == i->head && pass_counter++ > PASS_LIMIT) {
            /* If we hit this, we're dead. */
            printk_ratelimited(KERN_ERR
                               "rtk_serial8250: too much work for irq%d\n", irq);
            break;
        }
    } while (l != end);

    spin_unlock(&i->lock);

    DEBUG_INTR("end.\n");

    return IRQ_RETVAL(handled);
}

/*
 * To support ISA shared interrupts, we need to have one interrupt
 * handler that ensures that the IRQ line has been deasserted
 * before returning.  Failing to do this will result in the IRQ
 * line being stuck active, and, since ISA irqs are edge triggered,
 * no more IRQs will be seen.
 */
static void serial_do_unlink(struct irq_info *i, struct uart_8250_port *up)
{
    spin_lock_irq(&i->lock);

    if (!list_empty(i->head)) {
        if (i->head == &up->list)
            i->head = i->head->next;
        list_del(&up->list);
    } else {
        BUG_ON(i->head != &up->list);
        i->head = NULL;
    }
    spin_unlock_irq(&i->lock);
    /* List empty so throw away the hash node */
    if (i->head == NULL) {
        hlist_del(&i->node);
        kfree(i);
    }
}

static int serial_link_irq_chain(struct uart_8250_port *up)
{
    struct hlist_head *h;
    struct hlist_node *n;
    struct irq_info *i;
    int ret, irq_flags = up->port.flags & UPF_SHARE_IRQ ? IRQF_SHARED : 0;

    mutex_lock(&hash_mutex);

    h = &irq_lists[up->port.irq % NR_IRQ_HASH];

    hlist_for_each(n, h) {
        i = hlist_entry(n, struct irq_info, node);
        if (i->irq == up->port.irq)
            break;
    }

    if (n == NULL) {
        i = kzalloc(sizeof(struct irq_info), GFP_KERNEL);
        if (i == NULL) {
            mutex_unlock(&hash_mutex);
            return -ENOMEM;
        }
        spin_lock_init(&i->lock);
        i->irq = up->port.irq;
        hlist_add_head(&i->node, h);
    }
    mutex_unlock(&hash_mutex);

    spin_lock_irq(&i->lock);

    if (i->head) {
        list_add(&up->list, i->head);
        spin_unlock_irq(&i->lock);

        ret = 0;
    } else {
        INIT_LIST_HEAD(&up->list);
        i->head = &up->list;
        spin_unlock_irq(&i->lock);
        irq_flags |= up->port.irqflags;
	rtd_pr_uart_info("up->port.irq:%d\n",up->port.irq);
        ret = request_irq(up->port.irq, rtk_serial8250_interrupt,
                          irq_flags, "serial", i);
        if (ret < 0)
            serial_do_unlink(i, up);
    }

    return ret;
}

static void serial_unlink_irq_chain(struct uart_8250_port *up)
{
    /*
     * yes, some broken gcc emit "warning: 'i' may be used uninitialized"
     * but no, we are not going to take a patch that assigns NULL below.
     */
    struct irq_info *i;
    struct hlist_node *n;
    struct hlist_head *h;

    mutex_lock(&hash_mutex);

    h = &irq_lists[up->port.irq % NR_IRQ_HASH];

    hlist_for_each(n, h) {
        i = hlist_entry(n, struct irq_info, node);
        if (i->irq == up->port.irq)
            break;
    }

    BUG_ON(n == NULL);
    BUG_ON(i->head == NULL);

    if (list_empty(i->head))
        free_irq(up->port.irq, i);

    serial_do_unlink(i, up);
    mutex_unlock(&hash_mutex);
}

/*
 * This function is used to handle ports that do not have an
 * interrupt.  This doesn't work very well for 16450's, but gives
 * barely passable results for a 16550A.  (Although at the expense
 * of much CPU overhead).
 */
    // RTK_FIXME, commit : c1eba5bcb64  refine timer
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void rtk_serial8250_timeout(struct timer_list *t)
#else
static void rtk_serial8250_timeout(unsigned long data)
#endif
{
    // RTK_FIXME, commit : c1eba5bcb64  remove .data
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    struct uart_8250_port *up = from_timer(up, t, timer);
#else
    struct uart_8250_port *up = (struct uart_8250_port *)data;
#endif

    up->port.handle_irq(&up->port);
    mod_timer(&up->timer, jiffies + uart_poll_timeout(&up->port));
}

    // RTK_FIXME, commit : c1eba5bcb64  remove .data
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void rtk_serial8250_backup_timeout(struct timer_list *t)
#else
static void rtk_serial8250_backup_timeout(unsigned long data)
#endif
{
    // RTK_FIXME, commit : c1eba5bcb64  remove .data
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    struct uart_8250_port *up = from_timer(up, t, timer);
#else
    struct uart_8250_port *up = (struct uart_8250_port *)data;
#endif
    unsigned int iir, ier = 0, lsr;
    unsigned long flags;

    spin_lock_irqsave(&up->port.lock, flags);

    /*
     * Must disable interrupts or else we risk racing with the interrupt
     * based handler.
     */
    if (up->port.irq) {
        ier = rtk_serial_in(up, UART_IER);
        rtk_serial_out(up, UART_IER, 0);
    }

    iir = rtk_serial_in(up, UART_IIR);

    /*
     * This should be a safe test for anyone who doesn't trust the
     * IIR bits on their UART, but it's specifically designed for
     * the "Diva" UART used on the management processor on many HP
     * ia64 and parisc boxes.
     */
    lsr = rtk_serial_in(up, UART_LSR);
    up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
    if ((iir & UART_IIR_NO_INT) && (up->ier & UART_IER_THRI) &&
        (!uart_circ_empty(&up->port.state->xmit) || up->port.x_char) &&
        (lsr & UART_LSR_THRE)) {
        iir &= ~(UART_IIR_ID | UART_IIR_NO_INT);
        iir |= UART_IIR_THRI;
    }

    if (!(iir & UART_IIR_NO_INT)) {

#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
        if(rtice_early_disable_tx_int == 0)
            rtk_serial8250_tx_chars(up);
#else
        rtk_serial8250_tx_chars(up);
#endif

    }

    if (up->port.irq)
        rtk_serial_out(up, UART_IER, ier);

    spin_unlock_irqrestore(&up->port.lock, flags);

    /* Standard timer interval plus 0.2s to keep the port running */
    mod_timer(&up->timer,
              jiffies + uart_poll_timeout(&up->port) + HZ / 5);
}

static int univ8250_setup_irq(struct uart_8250_port *up)
{
    struct uart_port *port = &up->port;
    int retval = 0;

    /*
     * The above check will only give an accurate result the first time
     * the port is opened so this value needs to be preserved.
     */
    if (up->bugs & UART_BUG_THRE) {
        rtd_pr_uart_debug("%s%d - using backup timer\n", CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME, serial_index(port));

        // RTK_FIXME, commit : c1eba5bcb64  refine timer
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        timer_setup(&up->timer, rtk_serial8250_backup_timeout, 0);
#else
        up->timer.data = (unsigned long)up;
        up->timer.function = rtk_serial8250_backup_timeout;
#endif
        mod_timer(&up->timer, jiffies +
                  uart_poll_timeout(port) + HZ / 5);
    }

    /*
     * If the "interrupt" for this port doesn't correspond with any
     * hardware interrupt, we use a timer-based system.  The original
     * driver used to do this with IRQ0.
     */
    if (!port->irq) {
        // RTK_FIXME, commit : c1eba5bcb64  remove .data
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
        up->timer.data = (unsigned long)up;
#endif
        mod_timer(&up->timer, jiffies + uart_poll_timeout(port));
    } else
        retval = serial_link_irq_chain(up);

    return retval;
}

static void univ8250_release_irq(struct uart_8250_port *up)
{
    struct uart_port *port = &up->port;

    del_timer_sync(&up->timer);
        // RTK_FIXME, commit : c1eba5bcb64  refine timer
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
    timer_setup(&up->timer, rtk_serial8250_timeout, 0);
#else
    up->timer.function = rtk_serial8250_timeout;
#endif
    if (port->irq)
        serial_unlink_irq_chain(up);
}

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_RSA
static int rtk_serial8250_request_rsa_resource(struct uart_8250_port *up)
{
    unsigned long start = UART_RSA_BASE << up->port.regshift;
    unsigned int size = 8 << up->port.regshift;
    struct uart_port *port = &up->port;
    int ret = -EINVAL;

    switch (port->iotype) {
        case UPIO_HUB6:
        case UPIO_PORT:
            start += port->iobase;
            if (request_region(start, size, "serial-rsa"))
                ret = 0;
            else
                ret = -EBUSY;
            break;
    }

    return ret;
}

static void rtk_serial8250_release_rsa_resource(struct uart_8250_port *up)
{
    unsigned long offset = UART_RSA_BASE << up->port.regshift;
    unsigned int size = 8 << up->port.regshift;
    struct uart_port *port = &up->port;

    switch (port->iotype) {
        case UPIO_HUB6:
        case UPIO_PORT:
            release_region(port->iobase + offset, size);
            break;
    }
}
#endif

static const struct uart_ops *base_ops;
static struct uart_ops univ8250_port_ops;

static const struct uart_8250_ops univ8250_driver_ops = {
    .setup_irq  = univ8250_setup_irq,
    .release_irq    = univ8250_release_irq,
};



/**
 * rtk_serial8250_get_port - retrieve struct uart_8250_port
 * @line: serial line number
 *
 * This function retrieves struct uart_8250_port for the specific line.
 * This struct *must* *not* be used to perform a 8250 or serial core operation
 * which is not accessible otherwise. Its only purpose is to make the struct
 * accessible to the runtime-pm callbacks for context suspend/restore.
 * The lock assumption made here is none because runtime-pm suspend/resume
 * callbacks should not be invoked if there is any operation performed on the
 * port.
 */
struct uart_8250_port *rtk_serial8250_get_port(int line)
{
    return &rtk_serial8250_ports[line].up;
}
EXPORT_SYMBOL_GPL(rtk_serial8250_get_port);

// RTK_FIXME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void (*rtk_serial8250_isa_config)(int port, struct uart_port *up,
                                     u32 *capabilities);
#else
static void (*rtk_serial8250_isa_config)(int port, struct uart_port *up,
                                     unsigned short *capabilities);
#endif

// RTK_FIXME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void rtk_serial8250_set_isa_configurator(
    void (*v)(int port, struct uart_port *up, u32 *capabilities))
#else
void rtk_serial8250_set_isa_configurator(
    void (*v)(int port, struct uart_port *up, unsigned short *capabilities))
#endif
{
    rtk_serial8250_isa_config = v;
}
EXPORT_SYMBOL(rtk_serial8250_set_isa_configurator);

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_RSA

static void univ8250_config_port(struct uart_port *port, int flags)
{
    struct uart_8250_port *up = up_to_u8250p(port);

    up->probe &= ~UART_PROBE_RSA;
    if (port->type == PORT_RSA) {
        if (rtk_serial8250_request_rsa_resource(up) == 0)
            up->probe |= UART_PROBE_RSA;
    } else if (flags & UART_CONFIG_TYPE) {
        int i;

        for (i = 0; i < probe_rsa_count; i++) {
            if (probe_rsa[i] == up->port.iobase) {
                if (rtk_serial8250_request_rsa_resource(up) == 0)
                    up->probe |= UART_PROBE_RSA;
                break;
            }
        }
    }

    base_ops->config_port(port, flags);

    if (port->type != PORT_RSA && up->probe & UART_PROBE_RSA)
        rtk_serial8250_release_rsa_resource(up);
}

static int univ8250_request_port(struct uart_port *port)
{
    struct uart_8250_port *up = up_to_u8250p(port);
    int ret;

    ret = base_ops->request_port(port);
    if (ret == 0 && port->type == PORT_RSA) {
        ret = rtk_serial8250_request_rsa_resource(up);
        if (ret < 0)
            base_ops->release_port(port);
    }

    return ret;
}

static void univ8250_release_port(struct uart_port *port)
{
    struct uart_8250_port *up = up_to_u8250p(port);

    if (port->type == PORT_RSA)
        rtk_serial8250_release_rsa_resource(up);
    base_ops->release_port(port);
}

static void univ8250_rsa_support(struct uart_ops *ops)
{
    ops->config_port  = univ8250_config_port;
    ops->request_port = univ8250_request_port;
    ops->release_port = univ8250_release_port;
}

#else
#define univ8250_rsa_support(x)     do { } while (0)
#endif /* CONFIG_RTK_KDRV_SERIAL_8250_RSA */
static void rtk_serial8250_isa_init_ports(void)
{
    struct rtk_uart_port *rtk_up;
    static int first = 1;
    int i, irqflag = 0;

    if (!first)
        return;
    first = 0;

    if (nr_uarts > UART_NR)
        nr_uarts = UART_NR;

    for (i = 0; i < nr_uarts; i++) {
        struct uart_8250_port *up = &rtk_serial8250_ports[i].up;
        struct uart_port *port = &up->port;


        port->line = i;
        rtk_serial8250_init_port(up);
        if (!base_ops)
            base_ops = port->ops;
        port->ops = &univ8250_port_ops;

        // RTK_FIXME, commit : c1eba5bcb64  refine timer
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
        timer_setup(&up->timer, rtk_serial8250_timeout, 0);
#else
        init_timer(&up->timer);
        up->timer.function = rtk_serial8250_timeout;
#endif

        up->ops = &univ8250_driver_ops;

        /*
         * ALPHA_KLUDGE_MCR needs to be killed.
         */
        up->mcr_mask = ~ALPHA_KLUDGE_MCR;
        up->mcr_force = ALPHA_KLUDGE_MCR;
    }

    /* chain base port ops to support Remote Supervisor Adapter */
    univ8250_port_ops = *base_ops;
    univ8250_rsa_support(&univ8250_port_ops);

    if (share_irqs)
        irqflag = IRQF_SHARED;

    for (i = 0, rtk_up = rtk_serial8250_ports;
         i < ARRAY_SIZE(old_serial_port) && i < nr_uarts;
         i++, rtk_up++) {
        struct uart_8250_port *up = &rtk_up->up;
        struct uart_port *port = &up->port;

        port->iobase   = old_serial_port[i].port;
        port->irq      = irq_canonicalize(old_serial_port[i].irq);
        port->irqflags = old_serial_port[i].irqflags;
        port->uartclk  = rtk_serial8250_get_clock(get_rbus_mapped_addr((unsigned long)old_serial_port[i].iomem_base));
        port->flags    = old_serial_port[i].flags;
        port->hub6     = old_serial_port[i].hub6;
        port->membase  = (unsigned char __iomem *)get_rbus_mapped_addr((unsigned long)old_serial_port[i].iomem_base);
        port->iotype   = old_serial_port[i].io_type;
        port->regshift = old_serial_port[i].iomem_reg_shift;
        rtk_serial8250_set_defaults(up);

        port->irqflags |= irqflag;
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
        rtk_up->rtk_dma_config.dma_membase = (unsigned char __iomem *)get_rbus_mapped_addr((unsigned long)old_serial_port[i].dma_iomem_base);
        rtk_up->rtk_dma_config.dma_membase_ofst = (old_serial_port[i].dma_iomem_base - old_serial_port[i].iomem_base) >> old_serial_port[i].iomem_reg_shift;
        rtk_up->rtk_dma_config.dma_ring_buf_len = old_serial_port[i].dma_ring_buf_len;
        rtk_up->rtk_dma_config.dma_tx_thd = old_serial_port[i].dma_tx_thd;
        rtk_up->rtk_dma_config.dma_tx_empty_thd = old_serial_port[i].dma_tx_empty_thd;
        rtk_up->rtk_dma = NULL;
        /*
        rtd_pr_uart_warn(KERN_WARNING "DMA CONFIG:%d, %08x, %08x, %08x %08x, %08x %08x\n",
            i, (unsigned int)get_rbus_mapped_addr((unsigned long)old_serial_port[i].iomem_base),
            (unsigned long)rtk_up->rtk_dma_config.dma_membase,
            rtk_up->rtk_dma_config.dma_membase_ofst,
            rtk_up->rtk_dma_config.dma_ring_buf_len,
            rtk_up->rtk_dma_config.dma_tx_thd,
            rtk_up->rtk_dma_config.dma_tx_empty_thd
            );
        */
#endif
        if (rtk_serial8250_isa_config != NULL)
            rtk_serial8250_isa_config(i, &up->port, &up->capabilities);
    }
}

static void rtk_serial8250_register_ports(struct uart_driver *drv, struct device *dev)
{
    int i;

    for (i = 0; i < nr_uarts; i++) {
        struct uart_8250_port *up = &rtk_serial8250_ports[i].up;

        if (up->port.type == PORT_8250_CIR)
            continue;

        if (up->port.dev)
            continue;

        up->port.dev = dev;

        if (skip_txen_test)
            up->port.flags |= UPF_NO_TXEN_TEST;

        uart_add_one_port(drv, &up->port);
    }
}

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_CONSOLE

static void univ8250_console_write(struct console *co, const char *s,
                                   unsigned int count)
{
    struct uart_8250_port *up = &rtk_serial8250_ports[co->index].up;

    rtk_serial8250_console_write(up, s, count);
}

static int univ8250_console_setup(struct console *co, char *options)
{
    struct uart_port *port;

    /*
     * Check whether an invalid uart number has been specified, and
     * if so, search for the first available port that does have
     * console support.
     */
    if (co->index >= nr_uarts)
        co->index = 0;
    port = &rtk_serial8250_ports[co->index].up.port;
    /* link port to console */
    port->cons = co;
#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)    
    rtice_port = co->index;
#endif
    return rtk_serial8250_console_setup(port, options, false);
}


static int rtk_uart_parse_earlycon(char *p, unsigned char *iotype, resource_size_t *addr,
			char **options)
{
	if (strncmp(p, "mmio,", 5) == 0) {
		*iotype = UPIO_MEM;
		p += 5;
	} else if (strncmp(p, "mmio32,", 7) == 0) {
		*iotype = UPIO_MEM32;
		p += 7;
	}else if (strncmp(p, "0x", 2) == 0) {
		*iotype = UPIO_MEM;
	} else {
		return -EINVAL;
	}

	/*
	 * Before you replace it with kstrtoull(), think about options separator
	 * (',') it will not tolerate
	 */
	*addr = simple_strtoull(p, NULL, 0);
	p = strchr(p, ',');
	if (p)
		p++;

	*options = p;
	return 0;
}

/**
 *  univ8250_console_match - non-standard console matching
 *  @co:      registering console
 *  @name:    name from console command line
 *  @idx:     index from console command line
 *  @options: ptr to option string from console command line
 *
 *  Only attempts to match console command lines of the form:
 *      console=uart[8250],io|mmio|mmio32,<addr>[,<options>]
 *      console=uart[8250],0x<addr>[,<options>]
 *  This form is used to register an initial earlycon boot console and
 *  replace it with the rtk_serial8250_console at 8250 driver init.
 *
 *  Performs console setup for a match (as required by interface)
 *  If no <options> are specified, then assume the h/w is already setup.
 *
 *  Returns 0 if console matches; otherwise non-zero to use default matching
 */
static int univ8250_console_match(struct console *co, char *name, int idx,
                                  char *options)
{
    char match[] = CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME;  /* 8250-specific earlycon name */
    unsigned char iotype;
    phys_addr_t  addr;

    int i;

    if (strncmp(name, match, strlen(CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME)) != 0)
        return -ENODEV;
    if (rtk_uart_parse_earlycon(options, &iotype, (void *)&addr, &options))
        return -ENODEV;
    /* try to match the port specified on the command line */
    for (i = 0; i < nr_uarts; i++) {
        struct uart_port *port = &rtk_serial8250_ports[i].up.port;

        if (port->iotype != iotype)
            continue;
        if ((iotype == UPIO_MEM || iotype == UPIO_MEM32) &&
            (port->mapbase != addr))
            continue;
        if (iotype == UPIO_PORT && port->iobase != addr)
            continue;

        co->index = i;
        port->cons = co;
        return rtk_serial8250_console_setup(port, options, true);
    }

    return -ENODEV;
}

static struct console univ8250_console = {
    .name       = CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME,
    .write      = univ8250_console_write,
    .device     = uart_console_device,
    .setup      = univ8250_console_setup,
    .match      = univ8250_console_match,
    .flags      = CON_PRINTBUFFER | CON_ANYTIME,
    .index      = -1,
    .data       = &rtk_serial8250_reg,
};

static int univ8250_console_init(void)
{
    if (nr_uarts == 0)
        return -ENODEV;

    rtk_serial8250_isa_init_ports();
    register_console(&univ8250_console);
    return 0;
}
#if !defined(CONFIG_RTK_KDRV_SERIAL_8250_MODULE)
console_initcall(univ8250_console_init);
#else
static void __exit univ8250_console_exit(void)
{
    if (nr_uarts == 0)
        return;
    unregister_console(&univ8250_console);
}
#endif


#define rtk_serial8250_CONSOLE  &univ8250_console
#else
#define rtk_serial8250_CONSOLE  NULL
#endif

static struct uart_driver rtk_serial8250_reg = {
    .owner          = THIS_MODULE,
    .driver_name        = "rtk_serial",
    .dev_name       = CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME,
    .major          = TTY_MAJOR,
    .minor          = 68,
    .cons           = rtk_serial8250_CONSOLE,
};

/*
 * early_serial_setup - early registration for 8250 ports
 *
 * Setup an 8250 port structure prior to console initialisation.  Use
 * after console initialisation will cause undefined behaviour.
 */
int __init rtk_early_serial_setup(struct uart_port *port)
{
    struct uart_port *p;

    if (port->line >= ARRAY_SIZE(rtk_serial8250_ports) || nr_uarts == 0)
        return -ENODEV;

    rtk_serial8250_isa_init_ports();
    p = &rtk_serial8250_ports[port->line].up.port;
    p->iobase       = port->iobase;
    p->membase      = port->membase;
    p->irq          = port->irq;
    p->irqflags     = port->irqflags;
    p->uartclk      = port->uartclk;
    p->fifosize     = port->fifosize;
    p->regshift     = port->regshift;
    p->iotype       = port->iotype;
    p->flags        = port->flags;
    p->mapbase      = port->mapbase;
    p->mapsize      = port->mapsize;
    p->private_data = port->private_data;
    p->type     = port->type;
    p->line     = port->line;

    rtk_serial8250_set_defaults(up_to_u8250p(p));

    if (port->serial_in)
        p->serial_in = port->serial_in;
    if (port->serial_out)
        p->serial_out = port->serial_out;
    if (port->handle_irq)
        p->handle_irq = port->handle_irq;

    return 0;
}

/**
 *  rtk_serial8250_suspend_port - suspend one serial port
 *  @line:  serial line number
 *
 *  Suspend one serial port.
 */
void rtk_serial8250_suspend_port(int line)
{
    struct uart_8250_port *up = &rtk_serial8250_ports[line].up;
    struct uart_port *port = &up->port;

    if (!console_suspend_enabled && uart_console(port) &&
        port->type != PORT_8250) {
        unsigned char canary = 0xa5;
        rtk_serial_out(up, UART_SCR, canary);
        if (rtk_serial_in(up, UART_SCR) == canary)
            up->canary = canary;
    }

    uart_suspend_port(&rtk_serial8250_reg, port);
}

/**
 *  rtk_serial8250_resume_port - resume one serial port
 *  @line:  serial line number
 *
 *  Resume one serial port.
 */

extern void uart_hwsem_lock(const char *name);
extern void uart_hwsem_unlock(void);
#define Get_HD_SEM_NEW_1()   uart_hwsem_lock(__func__)
#define Set_HD_SEM_NEW_1()   uart_hwsem_unlock()

void rtk_serial8250_resume_port(int line)
{
    struct uart_8250_port *up = &rtk_serial8250_ports[line].up;
    struct uart_port *port = &up->port;

    up->canary = 0;

    if (up->capabilities & UART_NATSEMI) {
        /* Ensure it's still in high speed mode */
        Get_HD_SEM_NEW_1();
        serial_port_out(port, UART_LCR, 0xE0);

        ns16550a_goto_highspeed(up);

        serial_port_out(port, UART_LCR, 0);
        Set_HD_SEM_NEW_1();
        port->uartclk = 921600 * 16;
    }
    uart_resume_port(&rtk_serial8250_reg, port);
}

/*
 * Register a set of serial devices attached to a platform device.  The
 * list is terminated with a zero flags entry, which means we expect
 * all entries to have at least UPF_BOOT_AUTOCONF set.
 */
#if defined(CONFIG_OF) || defined(CONFIG_ARM64)
static int rtk_serial8250_probe(struct platform_device *pdev)
{

    int ret;
#if defined(CONFIG_RTK_KDRV_SERIAL_8250_CONSOLE) && defined(CONFIG_RTK_KDRV_SERIAL_8250_MODULE)
    univ8250_console_init();
#endif
#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
    rtd_pr_uart_info("[RTICE]rtice_enable : %d, rtice_port : %d\n",
            rtk_serial8250_get_rtice_enable(), rtice_port);
#endif

    rtd_pr_uart_info("Realtek Serial: 8250/16550 driver, %d ports, IRQ sharing %sabled\n",
            nr_uarts, share_irqs ? "en" : "dis");

    rtk_serial8250_isa_init_ports();

    {
        // workaround for 64bit serial irq number
        int i;
        for (i = 0; i < nr_uarts; i++) {
            struct uart_8250_port *up = &rtk_serial8250_ports[i].up;
            struct uart_port *port = &up->port;
	    struct device_node *np = pdev->dev.of_node;
	    port->irq = irq_of_parse_and_map(np, i);
	    rtd_pr_uart_info("%s,  virq=%d\n", __func__, port->irq);
        }

    }

    rtk_serial8250_reg.nr = UART_NR;
    ret = uart_register_driver(&rtk_serial8250_reg);
    if (ret) {
        dev_err(&pdev->dev, "Failed to initialize rtk_serial8250\n");
        goto probe_out;
    }

    rtk_serial8250_register_ports(&rtk_serial8250_reg, &pdev->dev);
probe_out:
    return ret;
}
#else
static int rtk_serial8250_probe(struct platform_device *dev)
{
    struct plat_rtk_serial8250_port *p = dev_get_platdata(&dev->dev);
    struct uart_8250_port uart;
    int ret, i, irqflag = 0;

    memset(&uart, 0, sizeof(uart));

    if (share_irqs)
        irqflag = IRQF_SHARED;

    for (i = 0; p && p->flags != 0; p++, i++) {
        uart.port.iobase    = p->iobase;
        uart.port.membase   = p->membase;
        uart.port.irq       = p->irq;
        uart.port.irqflags  = p->irqflags;
        uart.port.uartclk   = p->uartclk;
        uart.port.regshift  = p->regshift;
        uart.port.iotype    = p->iotype;
        uart.port.flags     = p->flags;
        uart.port.mapbase   = p->mapbase;
        uart.port.hub6      = p->hub6;
        uart.port.private_data  = p->private_data;
        uart.port.type      = p->type;
        uart.port.serial_in = p->serial_in;
        uart.port.serial_out    = p->serial_out;
        uart.port.handle_irq    = p->handle_irq;
        uart.port.handle_break  = p->handle_break;
        uart.port.set_termios   = p->set_termios;
        uart.port.pm        = p->pm;
        uart.port.dev       = &dev->dev;
        uart.port.irqflags  |= irqflag;
        ret = rtk_serial8250_register_8250_port(&uart);
        if (ret < 0) {
            dev_err(&dev->dev, "unable to register port at index %d "
                    "(IO%lx MEM%llx IRQ%d): %d\n", i,
                    p->iobase, (unsigned long long)p->mapbase,
                    p->irq, ret);
        }
    }
    return 0;
}
#endif


/*
 * Remove serial ports registered against a platform device.
 */
static int rtk_serial8250_remove(struct platform_device *dev)
{
    int i;
#if defined(CONFIG_RTK_KDRV_SERIAL_8250_CONSOLE) && defined(CONFIG_RTK_KDRV_SERIAL_8250_MODULE)
    univ8250_console_exit();
#endif	
    for (i = 0; i < nr_uarts; i++) {
        struct uart_8250_port *up = &rtk_serial8250_ports[i].up;

        if (up->port.dev == &dev->dev)
            rtk_serial8250_unregister_port(i);
    }
    return 0;
}

static struct platform_device *g_rtk_serial8250_dev = 0;

static int rtk_serial8250_suspend(struct platform_device *dev, pm_message_t state)
{
    int i;

    for (i = 0; i < UART_NR; i++) {
	 struct rtk_uart_port *rtk_up = &rtk_serial8250_ports[i];
        struct uart_8250_port *up = &rtk_up->up;	
	 
        if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev) {
            g_rtk_serial8250_dev = dev;
            uart_suspend_port(&rtk_serial8250_reg, &up->port);
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
            if(rtk_up->rtk_dma) {
                rtk_serial8250_release_dma(up);
                rtk_up->rtk_dma = NULL;
            }
#endif

#ifndef CONFIG_REALTEK_FPGA
            if (up->port.membase == (unsigned char *)GET_MAPPED_RBUS_ADDR(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg)) {
                rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, BIT(SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_shift));  //iso_misc_off_int_scpu_routing disable
            }
#endif  //!CONFIG_REALTEK_FPGA
        }
    }

    return 0;
}

static int rtk_serial8250_resume(struct platform_device *dev)
{
    int i;
    __maybe_unused int retval;

    g_rtk_serial8250_dev = 0;
    for (i = 0; i < UART_NR; i++) {
	 struct rtk_uart_port *rtk_up = &rtk_serial8250_ports[i];
        struct uart_8250_port *up = &rtk_up->up;
        if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev) {
            rtk_serial8250_resume_port(i);
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
            if(rtk_up->rtk_dma_config.dma_membase != NULL && !rtk_up->rtk_dma) {
                retval = rtk_serial8250_request_dma(up);
                if (retval) {
                    rtd_pr_uart_warn("%s%d - failed to request RTK DMA\n",
                                       	CONFIG_RTK_KDRV_SERIAL_8250_DRV_NAME, serial_index(&(up->port)));
                    rtk_up->rtk_dma = NULL;
                }
            }
#endif

#ifndef CONFIG_REALTEK_FPGA

            // enable uart global interrupt
            if (up->port.membase == (unsigned char *)GET_MAPPED_RBUS_ADDR(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg)) {
#if defined(SYS_REG_INT_CTRL_KCPU_reg) && defined(SYS_REG_INT_CTRL_KCPU_standby_iso_misc_off_int_kcpu_routing_en_shift)
                rtd_outl(SYS_REG_INT_CTRL_KCPU_reg, BIT(SYS_REG_INT_CTRL_KCPU_standby_iso_misc_off_int_kcpu_routing_en_shift));  //iso_misc_off_int_kcpu_routing disable
#endif
                rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, BIT(SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_shift) | BIT(SYS_REG_INT_CTRL_SCPU_write_data_shift)); //iso_misc_off_int_scpu_routing enable
            }

#endif  //!CONFIG_REALTEK_FPGA
        }
    }

    return 0;
}


#if defined(CONFIG_OF) || defined(CONFIG_ARM64)
static const struct of_device_id rtk_uart_of_match[] = {
    {
        .compatible = "realtek,rtd29xx-uart",
    },
    {},
};
MODULE_DEVICE_TABLE(of, rtk_uart_of_match);
#endif


static struct platform_driver rtk_serial8250_isa_driver = {
    .probe      = rtk_serial8250_probe,
    .remove     = rtk_serial8250_remove,
    .suspend    = rtk_serial8250_suspend,
    .resume     = rtk_serial8250_resume,
    .driver     = {
        .name   = "rtk_serial8250",

#if defined(CONFIG_OF) || defined(CONFIG_ARM64)
        .of_match_table = rtk_uart_of_match,
#endif
    },
};

void rtk_serial8250_emergency_resume(void)
{

    if(g_rtk_serial8250_dev && console_suspend_enabled)
        rtk_serial8250_resume(g_rtk_serial8250_dev);
    g_rtk_serial8250_dev = 0;
}
EXPORT_SYMBOL(rtk_serial8250_emergency_resume);

/*
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the table in include/asm/serial.h
 */
static struct platform_device *rtk_serial8250_isa_devs;


static int rtk_uart_match_port(const struct uart_port *port1, const struct uart_port *port2)
{
	if (port1->iotype != port2->iotype)
		return 0;
      
	switch (port1->iotype) {
	case UPIO_MEM:
	case UPIO_MEM16:
	case UPIO_MEM32:
	case UPIO_MEM32BE:
		return (port1->mapbase == port2->mapbase);
	}
	return 0;
}


/*
 * rtk_serial8250_register_8250_port and rtk_serial8250_unregister_port allows for
 * 16x50 serial ports to be configured at run-time, to support PCMCIA
 * modems and PCI multiport cards.
 */
static DEFINE_MUTEX(serial_mutex);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct uart_8250_port *rtk_serial8250_find_match_or_unused(const struct uart_port *port)
#else
static struct uart_8250_port *rtk_serial8250_find_match_or_unused(struct uart_port *port)
#endif
{
    int i;

    /*
     * First, find a port entry which matches.
     */
    for (i = 0; i < nr_uarts; i++)
        if (rtk_uart_match_port(&rtk_serial8250_ports[i].up.port, port))
            return &rtk_serial8250_ports[i].up;

    /* try line number first if still available */
    i = port->line;
    if (i < nr_uarts && rtk_serial8250_ports[i].up.port.type == PORT_UNKNOWN &&
        rtk_serial8250_ports[i].up.port.iobase == 0)
        return &rtk_serial8250_ports[i].up;
    /*
     * We didn't find a matching entry, so look for the first
     * free entry.  We look for one which hasn't been previously
     * used (indicated by zero iobase).
     */
    for (i = 0; i < nr_uarts; i++)
        if (rtk_serial8250_ports[i].up.port.type == PORT_UNKNOWN &&
            rtk_serial8250_ports[i].up.port.iobase == 0)
            return &rtk_serial8250_ports[i].up;

    /*
     * That also failed.  Last resort is to find any entry which
     * doesn't have a real port associated with it.
     */
    for (i = 0; i < nr_uarts; i++)
        if (rtk_serial8250_ports[i].up.port.type == PORT_UNKNOWN)
            return &rtk_serial8250_ports[i].up;

    return NULL;
}

/**
 *  rtk_serial8250_register_8250_port - register a serial port
 *  @up: serial port template
 *
 *  Configure the serial port specified by the request. If the
 *  port exists and is in use, it is hung up and unregistered
 *  first.
 *
 *  The port is then probed and if necessary the IRQ is autodetected
 *  If this fails an error is returned.
 *
 *  On success the port is ready to use and the line number is returned.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
int rtk_serial8250_register_8250_port(const struct uart_8250_port *up)
#else
int rtk_serial8250_register_8250_port(struct uart_8250_port *up)
#endif
{
    struct uart_8250_port *uart;
    int ret = -ENOSPC;

    if (up->port.uartclk == 0)
        return -EINVAL;

    mutex_lock(&serial_mutex);

    uart = rtk_serial8250_find_match_or_unused(&up->port);
    if (uart && uart->port.type != PORT_8250_CIR) {
        if (uart->port.dev)
            uart_remove_one_port(&rtk_serial8250_reg, &uart->port);

        uart->port.iobase       = up->port.iobase;
        uart->port.membase      = up->port.membase;
        uart->port.irq          = up->port.irq;
        uart->port.irqflags     = up->port.irqflags;
        uart->port.uartclk      = up->port.uartclk;
        uart->port.fifosize     = up->port.fifosize;
        uart->port.regshift     = up->port.regshift;
        uart->port.iotype       = up->port.iotype;
        uart->port.flags        = up->port.flags | UPF_BOOT_AUTOCONF;
        uart->bugs      = up->bugs;
        uart->port.mapbase      = up->port.mapbase;
        uart->port.mapsize      = up->port.mapsize;
        uart->port.private_data = up->port.private_data;
        uart->tx_loadsz     = up->tx_loadsz;
        uart->capabilities  = up->capabilities;
        uart->port.throttle = up->port.throttle;
        uart->port.unthrottle   = up->port.unthrottle;
        uart->port.rs485_config = up->port.rs485_config;
        uart->port.rs485    = up->port.rs485;
        uart->dma       = up->dma;

        /* Take tx_loadsz from fifosize if it wasn't set separately */
        if (uart->port.fifosize && !uart->tx_loadsz)
            uart->tx_loadsz = uart->port.fifosize;

        if (up->port.dev)
            uart->port.dev = up->port.dev;

        if (skip_txen_test)
            uart->port.flags |= UPF_NO_TXEN_TEST;

        if (up->port.flags & UPF_FIXED_TYPE)
            uart->port.type = up->port.type;

        rtk_serial8250_set_defaults(uart);

        /* Possibly override default I/O functions.  */
        if (up->port.serial_in)
            uart->port.serial_in = up->port.serial_in;
        if (up->port.serial_out)
            uart->port.serial_out = up->port.serial_out;
        if (up->port.handle_irq)
            uart->port.handle_irq = up->port.handle_irq;
        /*  Possibly override set_termios call */
        if (up->port.set_termios)
            uart->port.set_termios = up->port.set_termios;
        if (up->port.set_mctrl)
            uart->port.set_mctrl = up->port.set_mctrl;
        if (up->port.startup)
            uart->port.startup = up->port.startup;
        if (up->port.shutdown)
            uart->port.shutdown = up->port.shutdown;
        if (up->port.pm)
            uart->port.pm = up->port.pm;
        if (up->port.handle_break)
            uart->port.handle_break = up->port.handle_break;
        if (up->dl_read)
            uart->dl_read = up->dl_read;
        if (up->dl_write)
            uart->dl_write = up->dl_write;

        if (uart->port.type != PORT_8250_CIR) {
            if (rtk_serial8250_isa_config != NULL)
                rtk_serial8250_isa_config(0, &uart->port,
                                      &uart->capabilities);

            ret = uart_add_one_port(&rtk_serial8250_reg,
                                    &uart->port);
            if (ret == 0)
                ret = uart->port.line;
        } else {
            dev_info(uart->port.dev,
                     "skipping CIR port at 0x%lx / 0x%llx, IRQ %d\n",
                     uart->port.iobase,
                     (unsigned long long)uart->port.mapbase,
                     uart->port.irq);

            ret = 0;
        }
    }
    mutex_unlock(&serial_mutex);

    return ret;
}
EXPORT_SYMBOL(rtk_serial8250_register_8250_port);

/**
 *  rtk_serial8250_unregister_port - remove a 16x50 serial port at runtime
 *  @line: serial line number
 *
 *  Remove one serial port.  This may not be called from interrupt
 *  context.  We hand the port back to the our control.
 */
void rtk_serial8250_unregister_port(int line)
{
    struct uart_8250_port *uart = &rtk_serial8250_ports[line].up;

    mutex_lock(&serial_mutex);
    uart_remove_one_port(&rtk_serial8250_reg, &uart->port);
    if (rtk_serial8250_isa_devs) {
        uart->port.flags &= ~UPF_BOOT_AUTOCONF;
        if (skip_txen_test)
            uart->port.flags |= UPF_NO_TXEN_TEST;
        uart->port.type = PORT_UNKNOWN;
        uart->port.dev = &rtk_serial8250_isa_devs->dev;
        uart->capabilities = 0;
        uart_add_one_port(&rtk_serial8250_reg, &uart->port);
    } else {
        uart->port.dev = NULL;
    }
    mutex_unlock(&serial_mutex);
}
EXPORT_SYMBOL(rtk_serial8250_unregister_port);


#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)

void early_disable_printk(void)
{
    rtice_early_disable_tx_int = 1;
}
EXPORT_SYMBOL(early_disable_printk);

void early_enable_printk(void)
{
    rtice_early_disable_tx_int = 0;
    //add flag to avoid unneccessary re-writing tx register
    rtice_early_disable_tx_int_flag = 1;
}
EXPORT_SYMBOL(early_enable_printk);

void disable_printk(void)
{
    struct uart_8250_port *up;
    struct rtk_uart_port *rtk_up;
    if (rtice_port < UART_NR) {
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
        unsigned long flags;
#endif
	 rtk_up = &rtk_serial8250_ports[rtice_port];
        up = &rtk_up->up;
        rtk_up->disable_printk = 1;
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
        spin_lock_irqsave(&up->port.lock, flags);
        if(rtk_up->rtk_dma) {
            struct uart_port *port = &up->port;
            int dma_reg_ofst = rtk_up->rtk_dma_config.dma_membase_ofst;
            serial_port_out(port, dma_reg_ofst + UART_DMA_INT_EN, UR1_UR_TX_THD_EN(0));
            rtk_serial8250_dma_flush_buffer(up);
        }
        spin_unlock_irqrestore(&up->port.lock, flags);

#endif
    }
}
EXPORT_SYMBOL(disable_printk);
void enable_printk(void)
{
    struct uart_8250_port *up;
    struct rtk_uart_port *rtk_up;
    if (rtice_port < UART_NR) {
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
        unsigned long flags;
#endif    
        rtk_up = &rtk_serial8250_ports[rtice_port];
        up = &rtk_up->up;
        rtk_up->disable_printk = 0;
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
        spin_lock_irqsave(&up->port.lock, flags);
        if(rtk_up->rtk_dma) {
            struct uart_port *port = &up->port;
            int dma_reg_ofst = rtk_up->rtk_dma_config.dma_membase_ofst;
            rtk_serial8250_dma_flush_buffer(up);
            serial_port_out(port, dma_reg_ofst + UART_DMA_INT_EN, UR1_UR_TX_THD_EN(1));
        }
        spin_unlock_irqrestore(&up->port.lock, flags);
#endif
        uart_write_wakeup(&up->port);   // wait up tty...
    }
}
EXPORT_SYMBOL(enable_printk);

void rtice_putchar(char ch)
{
    struct uart_8250_port *up;
    if (rtice_port >= UART_NR)
        return;
    up = &rtk_serial8250_ports[rtice_port].up;
    wait_for_xmitr(up, UART_LSR_THRE);
    _serial_out(up, UART_TX, ch);
}

EXPORT_SYMBOL(rtice_putchar);


#endif



#if defined(CONFIG_OF) || defined(CONFIG_ARM64)
module_platform_driver(rtk_serial8250_isa_driver);
#else


static int __init rtk_serial8250_init(void)
{
    int ret;

    if (nr_uarts == 0)
        return -ENODEV;

    rtk_serial8250_isa_init_ports();

    rtd_pr_uart_info("Serial: 8250/16550 driver, "
           "%d ports, IRQ sharing %sabled\n", nr_uarts,
           share_irqs ? "en" : "dis");

#if IS_ENABLED(CONFIG_REALTEK_RTICE) || IS_ENABLED(CONFIG_RTK_KDRV_RTICE)
    rtd_pr_uart_info("[RTICE]rtice_enable : %d, rtice_port : %d\n", rtk_serial8250_get_rtice_enable(), rtice_port);
#endif


#ifdef CONFIG_SPARC
    ret = sunserial_register_minors(&rtk_serial8250_reg, UART_NR);
#else
    rtk_serial8250_reg.nr = UART_NR;
    ret = uart_register_driver(&rtk_serial8250_reg);
#endif
    if (ret)
        goto out;

    ret = rtk_serial8250_pnp_init();
    if (ret)
        goto unreg_uart_drv;

    rtk_serial8250_isa_devs = platform_device_alloc("rtk_serial8250",
                          PLAT8250_DEV_LEGACY);
    if (!rtk_serial8250_isa_devs) {
        ret = -ENOMEM;
        goto unreg_pnp;
    }

    ret = platform_device_add(rtk_serial8250_isa_devs);
    if (ret)
        goto put_dev;

    rtk_serial8250_register_ports(&rtk_serial8250_reg, &rtk_serial8250_isa_devs->dev);

    ret = platform_driver_register(&rtk_serial8250_isa_driver);
    if (ret == 0)
        goto out;

    platform_device_del(rtk_serial8250_isa_devs);
put_dev:
    platform_device_put(rtk_serial8250_isa_devs);
unreg_pnp:
    rtk_serial8250_pnp_exit();
unreg_uart_drv:
#ifdef CONFIG_SPARC
    sunserial_unregister_minors(&rtk_serial8250_reg, UART_NR);
#else
    uart_unregister_driver(&rtk_serial8250_reg);
#endif
out:
    return ret;
}

static void __exit rtk_serial8250_exit(void)
{
    struct platform_device *isa_dev = rtk_serial8250_isa_devs;

    /*
     * This tells rtk_serial8250_unregister_port() not to re-register
     * the ports (thereby making rtk_serial8250_isa_driver permanently
     * in use.)
     */
    rtk_serial8250_isa_devs = NULL;

    platform_driver_unregister(&rtk_serial8250_isa_driver);
    platform_device_unregister(isa_dev);

    rtk_serial8250_pnp_exit();

#ifdef CONFIG_SPARC
    sunserial_unregister_minors(&rtk_serial8250_reg, UART_NR);
#else
    uart_unregister_driver(&rtk_serial8250_reg);
#endif
}

module_init(rtk_serial8250_init);
module_exit(rtk_serial8250_exit);

#endif
EXPORT_SYMBOL(rtk_serial8250_suspend_port);
EXPORT_SYMBOL(rtk_serial8250_resume_port);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic 8250/16x50 serial driver");

module_param(share_irqs, uint, 0644);
MODULE_PARM_DESC(share_irqs, "Share IRQs with other non-8250/16x50 devices"
                 " (unsafe)");

module_param(nr_uarts, uint, 0644);
MODULE_PARM_DESC(nr_uarts, "Maximum number of UARTs supported. (1-" __MODULE_STRING(CONFIG_RTK_KDRV_SERIAL_8250_NR_UARTS) ")");

module_param(skip_txen_test, uint, 0644);
MODULE_PARM_DESC(skip_txen_test, "Skip checking for the TXEN bug at init time");

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_RSA
module_param_array(probe_rsa, ulong, &probe_rsa_count, 0444);
MODULE_PARM_DESC(probe_rsa, "Probe I/O ports for RSA");
#endif
MODULE_ALIAS_CHARDEV_MAJOR(TTY_MAJOR);

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_DEPRECATED_OPTIONS
#ifndef MODULE
/* This module was renamed to 8250_core in 3.7.  Keep the old "8250" name
 * working as well for the module options so we don't break people.  We
 * need to keep the names identical and the convenient macros will happily
 * refuse to let us do that by failing the build with redefinition errors
 * of global variables.  So we stick them inside a dummy function to avoid
 * those conflicts.  The options still get parsed, and the redefined
 * MODULE_PARAM_PREFIX lets us keep the "8250." syntax alive.
 *
 * This is hacky.  I'm sorry.
 */
static void __used s8250_options(void)
{
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "8250_core."

    module_param_cb(share_irqs, &param_ops_uint, &share_irqs, 0644);
    module_param_cb(nr_uarts, &param_ops_uint, &nr_uarts, 0644);
    module_param_cb(skip_txen_test, &param_ops_uint, &skip_txen_test, 0644);
#ifdef CONFIG_RTK_KDRV_SERIAL_8250_RSA
    __module_param_call(MODULE_PARAM_PREFIX, probe_rsa,
                        &param_array_ops, .arr = &__param_arr_probe_rsa,
                        0444, -1, 0);
#endif
}
#else
MODULE_ALIAS("8250_core");
#endif
#endif
