#include <rtd_log/rtd_module_log.h>
#include "rtk_ice-aux.h"

#include "rtk_ice-debug.h"
#include <linux/delay.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <rtk_kdriver/io.h>
#include <linux/string.h>
#include "rbus/iso_misc_off_uart_reg.h"

#ifdef CONFIG_ARCH_RTK2851A
#include "rbus/pinmux_main_reg.h"
#else
#include "rbus/pinmux_reg.h"
#endif

#ifndef  BOOLEAN
#define BOOLEAN int
#define FALSE 0
#define TRUE 1
#endif
#ifdef DEBUG_TM_USED
static unsigned int s_tm_last = 0;
void rtice_tm_start(void)
{
	s_tm_last = rtd_inl(SCPU_CLK90K_LO_reg);
}

unsigned int rtice_tm_end(void)
{
	unsigned int eTm = rtd_inl(SCPU_CLK90K_LO_reg) - s_tm_last;
	return eTm * 1000 / 90090;

}
#endif

void uart1_enable(void)
{
	static BOOLEAN f_enabled = FALSE;

#ifdef CONFIG_ARCH_RTK2885M
	pinmux_st_pin_mux_ctrl1_RBUS mux_ctrl1_reg;
	pinmux_st_gpio_st_cfg_0_RBUS st_gpio_st_cfg_0_reg;
#endif

	iso_misc_off_uart_u0rbr_thr_dll_RBUS uart_u0rbr_thr_reg;
	iso_misc_off_uart_u0iir_fcr_RBUS uart_u0iir_fcr_reg;
	iso_misc_off_uart_u0lcr_RBUS uart_u0lcr_reg;
	
	if(f_enabled == TRUE)
		return;
#ifdef CONFIG_ARCH_RTK2885M
    /*pin mux*/
    mux_ctrl1_reg.regValue = rtd_inl(PINMUX_ST_Pin_Mux_Ctrl1_reg);
    mux_ctrl1_reg.uart0_rxdi_sel = 0x1;//src1:input mux
    rtd_outl(PINMUX_ST_Pin_Mux_Ctrl1_reg,mux_ctrl1_reg.regValue);
    st_gpio_st_cfg_0_reg.regValue = rtd_inl(PINMUX_ST_GPIO_ST_CFG_0_reg);
    st_gpio_st_cfg_0_reg.st_gpio_00_ps = 0x6;//uart0_rxdi_src1
    st_gpio_st_cfg_0_reg.st_gpio_01_ps = 0x6;//uart0_tx
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_0_reg,st_gpio_st_cfg_0_reg.regValue);
#endif

	/*uart settings*/
	uart_u0lcr_reg.regValue = 0;
	uart_u0lcr_reg.dlab = 1;/*DLAB = 1*/
	uart_u0lcr_reg.wls = 3;/*data len:8bit*/
	rtd_outl(ISO_MISC_OFF_UART_U0LCR_reg,uart_u0lcr_reg.regValue);
	/**
	  *UART1_DLL(Value): Timing Setting/DLL(DLAB=1)
	  *eg.115200; RX/TX FIFO data(DLAB=0)  27M/(16 *115200)
	**/
	uart_u0rbr_thr_reg.regValue = 0;
	uart_u0rbr_thr_reg.dll = 0xf;/*0x0f ---115200   0x1d ---57600*/
	rtd_outl(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg,uart_u0rbr_thr_reg.regValue);
	/*DLAB --> 0*/
	uart_u0lcr_reg.dlab = 0;
	rtd_outl(ISO_MISC_OFF_UART_U0LCR_reg,uart_u0lcr_reg.regValue);
	/**
	  *UART1_FCR(FIFO CTRL): FIFO level: 11  for 14bytes ,
	  *Transmitter FIFO reset,Receiver FIFO reset.
	**/
	uart_u0iir_fcr_reg.regValue = 0;
	uart_u0iir_fcr_reg.fifo16 = 0x3;/*fifo enable*/
	uart_u0iir_fcr_reg.iid = 0x6;
	rtd_outl(ISO_MISC_OFF_UART_U0IIR_FCR_reg,uart_u0iir_fcr_reg.regValue);
	/*UART1_FCR(FIFO CTRL): FIFO Enable:FIFO En;*/
	uart_u0iir_fcr_reg.iid = 0x1;
	rtd_outl(ISO_MISC_OFF_UART_U0IIR_FCR_reg,uart_u0iir_fcr_reg.regValue);
	/*DLAB --> 0*/
	uart_u0lcr_reg.dlab = 0;
	rtd_outl(ISO_MISC_OFF_UART_U0LCR_reg,uart_u0lcr_reg.regValue);
	/*finish*/
	f_enabled = TRUE;
}

void uart1_printer(unsigned char *buff, unsigned int len)
{
	iso_misc_off_uart_u0rbr_thr_dll_RBUS uart_u0rbr_thr_reg;
	iso_misc_off_uart_u0lsr_RBUS uart_u0lsr_reg;
	
	while (len) {
		int i = 0,cnt = 11;
		int tlen = len > 14 ? 14 : len;

		for(i = 0;i < tlen;i++){
			uart_u0rbr_thr_reg.dll = *(buff++);
			len--;
			rtd_outl(ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg, uart_u0rbr_thr_reg.regValue);
			/*wait empty of tr holding register*/
			cnt = 100;
			do{
				udelay(2);
				uart_u0lsr_reg.regValue = rtd_inl(ISO_MISC_OFF_UART_U0LSR_reg);
				if(cnt <= 0 || uart_u0lsr_reg.thre == 1)
					break;
			}while(1);
		}
		/*wait empty of tr fifo*/
		cnt = 11;
		do{
			uart_u0lsr_reg.regValue = rtd_inl(ISO_MISC_OFF_UART_U0LSR_reg);
			cnt--;
			if(cnt <= 0 || uart_u0lsr_reg.temt  == 1)
				break;
			else
				udelay(100);
		}while(1);
	}
}

void rtice_print_string(unsigned char *format, ...)
{
	unsigned char tmp[256];
	va_list argptr;
	
	va_start(argptr, format);
	vsprintf(tmp, format, argptr);
	va_end(argptr);

#ifdef DEBUG_RTICE
	uart1_enable();
	uart1_printer(tmp, strlen(tmp) <= 256 ? strlen(tmp) : 256);
#else
	 rtd_pr_rtice_debug("%s\n", tmp);
#endif
}

void rtice_print_array(unsigned char*fmt,unsigned char*pData,unsigned int len)
{
#ifdef DEBUG_RTICE_CMD
	int i;
	unsigned char tmp[512];
	unsigned char *ptmp = tmp;
	uart1_enable();
	memset(tmp, 0x0, 512);
	ptmp += sprintf(ptmp, "[DEBUG_CMD] :len = %d, %s { ", len, fmt);
	for (i = 0; i < len; i++) {
		ptmp += sprintf(ptmp, "%02x, ", *pData++);
		if (ptmp + 7 - tmp >= sizeof(tmp)) {
			ptmp += sprintf(ptmp, "...");
			break;
		}
	}
	ptmp += sprintf(ptmp, "} \r\n");
	uart1_printer(tmp, strlen(tmp));
#else
	int i;
	for (i = 0; i < len; i++)
		rtd_pr_rtice_debug("%02x ", *pData++);
	rtd_pr_rtice_debug("\n");
#endif /*  */
}

unsigned char rtice_checksum(unsigned char *p_data, unsigned int len)
{
	unsigned char checksum = 0;
	while (len--)
		checksum += *(p_data++);
	return (unsigned char)-checksum;
}
