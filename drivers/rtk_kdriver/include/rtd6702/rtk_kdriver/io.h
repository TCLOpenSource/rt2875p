/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012 by Chuck Chen <ycchen@realtek.com>
 *
 * IO mappings for rtd299o
 */

#ifndef _ASM_MACH_IO_H_
#define _ASM_MACH_IO_H_

#include <mach/iomap.h>
#include <asm/io.h>

#define __io(a)     __typesafe_io((a) & IO_SPACE_LIMIT)
#define __esb()     asm volatile("esb" : : : "memory")

__attribute__((__flatten__)) static inline unsigned char rtd_inb(unsigned long addr)
{
    unsigned char val;
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        val = __raw_readb((void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        val = __raw_readb((void __iomem *)addr);
    __esb();
    return val;
}
__attribute__((__flatten__)) static inline unsigned short rtd_inw(unsigned long addr)
{
    unsigned short val;
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        return __raw_readw((void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        return __raw_readw((void __iomem *)addr);
    __esb();
    return val;
}
__attribute__((__flatten__))  static inline unsigned int rtd_inl(unsigned long addr)
{
    unsigned int val;
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        val = __raw_readl((void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        val = __raw_readl((void __iomem *)addr);
    __esb();
    return val;
}
__attribute__((__flatten__)) static inline void rtd_outb(unsigned long addr, unsigned char val)
{
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        __raw_writeb(val,(void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        __raw_writeb(val,(void __iomem *)addr);
    __esb();
}
__attribute__((__flatten__)) static inline void rtd_outw(unsigned long addr, unsigned short val)
{
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        __raw_writew(val,(void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        __raw_writew(val,(void __iomem *)addr);
    __esb();
}
__attribute__((__flatten__)) static inline void rtd_outl(unsigned long addr, unsigned int val)
{
    if (addr < (RBUS_BASE_VIRT_OLD+RBUS_BASE_SIZE) && addr >= RBUS_BASE_VIRT_OLD)
        __raw_writel(val,(void __iomem *)(addr-RBUS_BASE_VIRT_OLD+RBUS_BASE_VIRT));
    else
        __raw_writel(val,(void __iomem *)addr);
    __esb();
}

static inline void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
	rtd_outl(reg_addr,(rtd_inl(reg_addr) & (~(((1<<(endBit-startBit+1))-1)<<startBit))) | (value<<startBit));
}

static inline unsigned int rtd_part_inl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit)
{
	return (rtd_inl(reg_addr)>>startBit)&((1<<(endBit-startBit+1))-1);
}


#define rtd_maskl(offset, andMask, orMask) rtd_outl(offset, ((rtd_inl(offset) & (andMask)) | (orMask)))
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))

// for serial port used
#include <mach/system.h>
#include <rbus/misc_reg.h>
#include <rbus/misc_uart_reg.h>
#include <rbus/wdog_reg.h>
#include <rbus/iso_misc_off_reg.h>
#include <mach/serial.h>

#ifdef CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA
#if (SYSTEM_CONSOLE == 0)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED, \
			0, 0, 0, 0},   /* ttyS0 */
#elif (SYSTEM_CONSOLE == 1)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED, \
			(char *)(MISC_UART_UR1_CTRL_reg), PAGE_SIZE, 0, 8},   /* ttyS0 */
#elif (SYSTEM_CONSOLE == 2)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(MISC_UART_U2RBR_THR_DLL_reg), 2, IRQF_SHARED, \
			0, 0, 0, 0},   /* ttyS0 */
#endif

#if (CONFIG_RTK_KDRV_SERIAL_8250_RUNTIME_UARTS == 2)
	#undef SERIAL_PORT_DFNS
	#if (SYSTEM_CONSOLE == 0)
		#define SERIAL_PORT_DFNS    \
      	{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED, \
      		0, 0, 0, 0}, /* ttyS0 */ \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED, \
				(char *)(MISC_UART_UR1_CTRL_reg), PAGE_SIZE, 0, 8},   /* ttyS1 */
	#elif (SYSTEM_CONSOLE == 1)
		#define SERIAL_PORT_DFNS    \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED, \
				(char *)(MISC_UART_UR1_CTRL_reg), PAGE_SIZE, 0, 8},   /* ttyS0 */ \
         { 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED, \
         	0, 0, 0, 0}, /* ttyS1 */
	#endif
#endif

#else
#if (SYSTEM_CONSOLE == 0)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS0 */
#elif (SYSTEM_CONSOLE == 1)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS0 */
#elif (SYSTEM_CONSOLE == 2)
	#define SERIAL_PORT_DFNS    \
		{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (char *)(MISC_UART_U2RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS0 */
#endif

#if (CONFIG_RTK_KDRV_SERIAL_8250_RUNTIME_UARTS == 2)
	#undef SERIAL_PORT_DFNS
#ifndef CONFIG_REALTEK_FPGA
	#if (SYSTEM_CONSOLE == 0)
		#define SERIAL_PORT_DFNS    \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS0 */ \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS1 */
	#elif (SYSTEM_CONSOLE == 1)
		#define SERIAL_PORT_DFNS    \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED}, 	/* ttyS1 */ \
			{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg), 2, IRQF_SHARED},   /* ttyS0 */
	#endif
#else
	#define SERIAL_PORT_DFNS    \
	{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED},       /* ttyS0 */ \
	{ 0, (UART_CLOCK>>4), 0, -1, UPF_SHARE_IRQ|UPF_BOOT_AUTOCONF, 0, UPIO_MEM32, (unsigned char *)(MISC_UART_U1RBR_THR_DLL_reg), 2, IRQF_SHARED},       /* ttyS1 */
#endif
#endif
#endif    
#endif	//_ASM_MACH_IO_H_
