KBUILD_CFLAGS += -Werror
obj-$(CONFIG_RTK_KDRV_SERIAL_8250) += rtk_uart.o
rtk_uart-y := rtk_uart_core.o rtk_uart_port.o
rtk_uart-$(CONFIG_RTK_KDRV_SERIAL_8250_UART_DMA) += rtk_uart_dma.o
rtk_uart-$(CONFIG_RTK_KDRV_SERIAL_8250_CONSOLE) += rtk_uart_early.o
