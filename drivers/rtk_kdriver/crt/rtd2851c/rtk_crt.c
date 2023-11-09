/*
 *  linux/arch/arm/mach-rtd299s/rtk_crt.c
 *
 *  Copyright (C) 2015 Realtek
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef BUILD_QUICK_SHOW
#include <linux/delay.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/hardirq.h>
#include <linux/uaccess.h>
#include <mach/system.h>
#else
#include <sysdefs.h>
#include <vsprintf.h>
#include <malloc.h>
#include <no_os/export.h>
#endif

#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/sb2_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/plletn_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/usb2_top_reg.h>
#include <rbus/tvsb5_reg.h>
#include <mach/system.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_crt.h>
#include <tvscalercontrol/io/ioregdrv.h>

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
#include <tvscalercontrol/vdc/video.h>
#include <rbus/vdtop_reg.h>
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_MIO)
#include <rbus/iso_mio_standby_reg.h>
#endif

char moduleName[32];
char lastModuleName[32];

#ifndef BUILD_QUICK_SHOW
/* rtk_crt char device variable */
int crt_major 		= CRT_MAJOR;
int crt_minor 		= 0;
int crt_nr_devs 	= CRT_NR_DEVS;
dev_t crt_devnum;
static struct class *crt_class;
static struct kobject *crt_kobj;
struct cdev crt_cdev;

static unsigned int register_addr, register_val = 0;
static unsigned int register_password = 0;
static struct device *crt_device;
static struct platform_device *crt_platform_devs;

#define func_trace()    rtd_pr_crt_err("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__)

unsigned long rtd_hwsem_user[1];

/* Notice: this API is semantically diff from hw_semaphore_try_lock.
* return 1: get lock
* return 0: not get lock.
*/
int rtd_hwsem_1_trylock(void)
{
	int ret;
	ret =  (*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_4_reg));
#if !defined (RTD_SWSEM_DEBUG_DISABLE)
	if (ret == 1)
		rtd_hwsem_user[0] = (unsigned long)__builtin_return_address(0);
#endif //#if !defined (RTD_SWSEM_DEBUG_DISABLE)

	return ret;
}


void rtd_hwsem_1_lock(void)
{
	while (!rtd_hwsem_1_trylock()) {
		udelay(1);
	}
	rtd_hwsem_user[0] = (unsigned long)__builtin_return_address(0);
}

void rtd_hwsem_1_unlock(void)
{
	*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_4_reg) = 0;
#if !defined (RTD_SWSEM_DEBUG_DISABLE)
	rtd_hwsem_user[0] = 0;
#endif //#if !defined (RTD_SWSEM_DEBUG_DISABLE)
	dsb(sy);

}

#if !defined (SB2_SHADOW_1)
#error "need define a dummy register for SW semaphore"
#endif
#define RD_SWSEM_REG          SB2_SHADOW_1_reg

DEFINE_SPINLOCK(pll27x3_lock);
DEFINE_SPINLOCK(avdin_lock);

unsigned int rtd_swsem_user[32];
int rtd_swsem_debug;  //sysctl used

int rtd_swsem_debug_func(void)
{
	int i;
	for(i=0; i<32; i++) {
		rtd_pr_crt_emerg( "user: 0x%x \n", rtd_swsem_user[i]);
	}

	return 0;
}


/**
   return 0: unlocked.
   return 1: get lock
**/
inline int rtd_crt_swsem_trylock(int bit)
{
          int swsem;
          int ret;

          if (!rtd_hwsem_1_trylock())
                    return 0;

          swsem = rtd_inl(RD_SWSEM_REG) & bit;
          if (swsem != 0) {
                    ret = 0;
                    goto bad;
          }

          rtd_setbits(RD_SWSEM_REG, bit); // lock
#if !defined (RTD_SWSEM_DEBUG_DISABLE)
	  rtd_swsem_user[__builtin_ctz(bit)] = (unsigned long)__builtin_return_address(0);
#endif //#if !defined (RTD_SWSEM_DEBUG_DISABLE)

          ret = 1;
bad:
          rtd_hwsem_1_unlock();

          return ret;
}

void rtd_crt_swsem_lock(int bit)
{
          int retry = 0;

          while (!rtd_crt_swsem_trylock(bit)) {
#if !defined (RTD_SWSEM_DEBUG_DISABLE)		  
		  if (retry++ == 0x100000) {
			  rtd_swsem_debug_func();
		  }
#endif //#if !defined (RTD_SWSEM_DEBUG_DISABLE)
          }
	  rtd_swsem_user[__builtin_ctz(bit)] = (unsigned long)__builtin_return_address(0);
}

void rtd_crt_swsem_unlock(int bit)
{
          rtd_hwsem_1_lock();

          rtd_clearbits(RD_SWSEM_REG, bit); //
  
#if !defined (RTD_SWSEM_DEBUG_DISABLE)
	  rtd_swsem_user[__builtin_ctz(bit)] = 0;
#endif //#if !defined (RTD_SWSEM_DEBUG_DISABLE)

          rtd_hwsem_1_unlock();
}



#if 0
inline void vdc_hw_semaphore_lock(void)
{
	unsigned char ucTime = 0;
	/* 0xb801a624 */
	while (!*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_1_reg)) {
		if (ucTime == 0) {
			ucTime ++;
			rtd_pr_crt_err("#@# %s wait SB2_HD_SEM_NEW_1_reg ready\n", moduleName);
			rtd_pr_crt_err("#@# last %s used SB2_HD_SEM_NEW_1_reg semaphore\n", lastModuleName);
		}
//		udelay(20);
	}
}

inline unsigned char vdc_hw_semaphore_try_lock(void)
{
	/* 0xb801a624 */
	if (!*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_1_reg)) {
		rtd_pr_crt_err("#@# %s SB2_HD_SEM_NEW_1_reg try lock, return FAIL\n", moduleName);
		return 0;	// _FAIL
	}
	return 1;	// _SUCCESS
}


inline void vdc_hw_semaphore_unlock(void)
{
	/* 0xb801a624 */
	*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_1_reg) = 0;
//	udelay(10);
}
#endif //#if 0
#endif  /* !BUILD_QUICK_SHOW */
void hw_semaphore_try_lock(void)
{
#if 0
	while (!*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_5_reg)) {
		rtd_pr_crt_err("#@# %s wait SB2_HD_SEM_NEW_5_reg ready\n", moduleName);
		rtd_pr_crt_err("#@# last %s used SB2_HD_SEM_NEW_5_reg semaphore\n", lastModuleName);
		mdelay(20);
	}
#endif
}

EXPORT_SYMBOL(hw_semaphore_try_lock);

void hw_semaphore_unlock(void)
{
#if 0
	*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_5_reg) = 0;
	udelay(10);
#endif
}

EXPORT_SYMBOL(hw_semaphore_unlock);

#if 0
static void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
	unsigned int X,A,result;
	X=(1<<(endBit-startBit+1))-1;
	A=rtd_inl(reg_addr);
	result = (A & (~(X<<startBit))) | (value<<startBit);
	rtd_outl(reg_addr,result);
}
#endif



unsigned int pll_info_bush(void)
{
/*
BUSH		27/(PDIV+1)*(Ncode+3+Fcode/2048)/(DIV+1)
0xb8000430	[29:28]	DIV
			[5:4]	PDIV
0xb8000434	[26:16]	F
			[15:8]	N

*/
	unsigned int value_DIV = (rtd_inl(PLL_REG_SYS_PLL_BUSH1_reg) >> 28) & 0x3;
	unsigned int value_PDIV = (rtd_inl(PLL_REG_SYS_PLL_BUSH1_reg) >> 4) & 0x3;
	unsigned int value_f =  (rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg) >> 16) & 0x7ff;
	unsigned int value_n = (rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg) >> 8) & 0xff;

	return 27*(value_n+3+(value_f>>11))/(value_PDIV+1)/(1<<value_DIV)/2;

}
EXPORT_SYMBOL(pll_info_bush);

unsigned int pll_info_disp(void)
{

/*
//DISP		27*(M+3+F/2048)/(N+2)/(2^O)
0xb80006a0	[26:16]	F
0xb80006c0	[14:12]	N
		[27:20]	M
0xb80006c8	[13:12]	O

*/
	unsigned int dclk;
	unsigned int value_f =  (rtd_inl(PLL27X_REG_PLL_SSC0_reg) >> 16) & 0x7ff;
	unsigned int value_n = (rtd_inl(PLL27X_REG_SYS_PLL_DISP1_reg) >> 12) & 0x7;
	unsigned int value_m = (rtd_inl(PLL27X_REG_SYS_PLL_DISP1_reg) >> 20) & 0xff;
	unsigned int value_o = (rtd_inl(PLL27X_REG_SYS_PLL_DISP3_reg) >> 12) & 0x3;

	unsigned int is_bypass_n = PLL27X_REG_SYS_PLL_DISP2_get_dpll_x_bpsin(rtd_inl(PLL27X_REG_SYS_PLL_DISP2_reg));

	if (is_bypass_n) {
		dclk = ((27*(value_m+3+ (value_f >>11))) / (1 << value_o));
	} else {
		dclk = ((27*(value_m+3+ (value_f >>11))) / ((value_n+2)*(1 << value_o)));
	}

	return dclk;
}
EXPORT_SYMBOL(pll_info_disp);

int APLL_ADC_clock(enum CRT_CLK clken)
{
	if (CLK_ON == clken) {
		if (((rtd_inl(SYS_REG_SYS_SRST1_reg)&SYS_REG_SYS_SRST1_rstn_apll_adc_mask) != 0) &&
			((rtd_inl(SYS_REG_SYS_CLKEN1_reg)&SYS_REG_SYS_CLKEN1_clken_apll_adc_mask) != 0))
			return CRT_SUCCESS;
	}

	hw_semaphore_try_lock();
	if (CLK_ON == clken) {
		if (((rtd_inl(SYS_REG_SYS_SRST1_reg)&SYS_REG_SYS_SRST1_rstn_apll_adc_mask) == 0) ||
			((rtd_inl(SYS_REG_SYS_CLKEN1_reg)&SYS_REG_SYS_CLKEN1_clken_apll_adc_mask) == 0)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_apll_adc_mask | SYS_REG_SYS_SRST1_write_data_mask);
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_apll_adc_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
		}
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

#if IS_ENABLED(CONFIG_RTK_KDRV_USB)
int USB_CriticalRegister(enum CRT_CLK clken, void *private_data) {
#define USB_PHY_DELAY	100
	long usb_version = (long)private_data;

	if (!usb_version)
		return CRT_FAIL;

	hw_semaphore_try_lock();

	if (CLK_ON == clken) {
		/*---------------------------------------------------
		* shutdown sequence :
		*  1. mac clock off
		*  2. not release mac reset
		*  3. not release phy reset
		*---------------------------------------------------*/

		/*---------------------------------------------------
		* init sequence :
		*  1. phy reset release
		*  2. mac clock on
		*  3. mac clock off
		*  4. mac reset release
		*  5. mac clock on
		*---------------------------------------------------*/

		switch (usb_version) {
		case 2:
			// disable interrupt
			rtd_outl(SYS_REG_INT_CTRL_SCPU_reg,
						SYS_REG_INT_CTRL_SCPU_usb2_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(0));
			
			rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg,
						SYS_REG_INT_CTRL_SCPU_2_usb2_ex_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(0));
			udelay(USB_PHY_DELAY);

			// clock off: USB2_MAC
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						SYS_REG_SYS_CLKEN2_clken_usb2_dwc_mask |
						SYS_REG_SYS_CLKEN2_clken_usb2_ex_dwc_mask |
						SYS_REG_SYS_CLKEN2_write_data(0));
			udelay(USB_PHY_DELAY);

			// USB2_MAC Reset
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						SYS_REG_SYS_SRST2_rstn_usb2_dwc_mask |
						SYS_REG_SYS_SRST2_rstn_usb2_ex_dwc_mask |
						SYS_REG_SYS_SRST2_write_data(0));
			udelay(USB_PHY_DELAY);

			// USB Phy Reset
			rtd_outl(SYS_REG_SYS_SRST3_reg,
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p3_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p2_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p1_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p0_mask |
						SYS_REG_SYS_SRST3_write_data(0));
			udelay(USB_PHY_DELAY);


			// Release USB Phy Reset
			rtd_outl(SYS_REG_SYS_SRST3_reg,
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p3_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p2_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p1_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p0_mask |
						SYS_REG_SYS_SRST3_write_data(1));
			udelay(USB_PHY_DELAY);

			// clock on : USB2_MAC
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						SYS_REG_SYS_CLKEN2_clken_usb2_dwc_mask |
						SYS_REG_SYS_CLKEN2_clken_usb2_ex_dwc_mask |
						SYS_REG_SYS_CLKEN2_write_data(1));
			udelay(USB_PHY_DELAY);

			// clock off : USB2_MAC
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						SYS_REG_SYS_CLKEN2_clken_usb2_dwc_mask |
						SYS_REG_SYS_CLKEN2_clken_usb2_ex_dwc_mask |
						SYS_REG_SYS_CLKEN2_write_data(0));
			udelay(USB_PHY_DELAY);

			// Release USB2_MAC Reset
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						SYS_REG_SYS_SRST2_rstn_usb2_dwc_mask |
						SYS_REG_SYS_SRST2_rstn_usb2_ex_dwc_mask |
						SYS_REG_SYS_SRST2_write_data(1));
			udelay(USB_PHY_DELAY);

			// clock on : USB2_MAC
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						SYS_REG_SYS_CLKEN2_clken_usb2_dwc_mask |
						SYS_REG_SYS_CLKEN2_clken_usb2_ex_dwc_mask |
						SYS_REG_SYS_CLKEN2_write_data(1));
			udelay(USB_PHY_DELAY);

			// enable interrupt
			rtd_outl(SYS_REG_INT_CTRL_SCPU_reg,
						SYS_REG_INT_CTRL_SCPU_usb2_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(1));
			
			rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg,
						SYS_REG_INT_CTRL_SCPU_2_usb2_ex_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(1));
			udelay(USB_PHY_DELAY);

			break;
		default:
			return CRT_FAIL;
		}
	} else {
		switch (usb_version) {
		/*---------------------------------------------------
		* shutdown sequence :
		*  1. mac clock off
		*  2. not release mac reset
		*  3. not release phy reset
		*---------------------------------------------------*/
		case 2:
			// disable interrupt
			rtd_outl(SYS_REG_INT_CTRL_SCPU_reg,
						SYS_REG_INT_CTRL_SCPU_usb2_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(0));
			
			rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg,
						SYS_REG_INT_CTRL_SCPU_2_usb2_ex_int_scpu_routing_en_mask |
						SYS_REG_INT_CTRL_SCPU_write_data(0));
			udelay(USB_PHY_DELAY);

			/* Shudown sequence */
			// clock off : USB2_MAC
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						SYS_REG_SYS_CLKEN2_clken_usb2_dwc_mask |
						SYS_REG_SYS_CLKEN2_clken_usb2_ex_dwc_mask |
						SYS_REG_SYS_CLKEN2_write_data(0));
			udelay(USB_PHY_DELAY);

			// not release mac reset
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						SYS_REG_SYS_SRST2_rstn_usb2_dwc_mask |
						SYS_REG_SYS_SRST2_rstn_usb2_ex_dwc_mask |
						SYS_REG_SYS_SRST2_write_data(0));
			udelay(USB_PHY_DELAY);

			// not release phy reset
			rtd_outl(SYS_REG_SYS_SRST3_reg,
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p3_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p2_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p1_mask |
						SYS_REG_SYS_SRST3_rstn_usb2_phy_p0_mask |
						SYS_REG_SYS_SRST3_write_data(0));
			udelay(USB_PHY_DELAY);
			break;
		default:
			return CRT_FAIL;
		}
	}
#undef USB_PHY_DELAY
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
#endif	//CONFIG_USB

#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
int IrDA_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* HW mode */
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_irda_shift));   //clken_irda:0
		rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_irda_shift));    //RSTN_irda:0
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_irda_shift) | STB_ST_CLKEN1_write_data_mask);   	//clken_irda:1
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_irda_shift));   //clken_irda:0
		rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_irda_shift) | STB_ST_SRST1_write_data_mask);    	//RSTN_irda:1
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_irda_shift) | STB_ST_CLKEN1_write_data_mask);   	//clken_irda:1
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_iso_misc_shift) | STB_ST_CLKEN0_write_data_mask); // ISO_MISC clock
		rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_iso_misc_shift) | STB_ST_SRST0_write_data_mask);  // ISO_MISC reset

		/* RAW mode */
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_irda_off_shift));   //clken_irda_off:0
		rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_irda_off_shift));    //RSTN_irda_off:0
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_irda_off_shift) | STB_ST_CLKEN0_write_data_mask);   	//clken_irda_off:1
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_irda_off_shift));   //clken_irda_off:0
		rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_irda_off_shift) | STB_ST_SRST0_write_data_mask);    	//RSTN_irda_off:1
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_irda_off_shift) | STB_ST_CLKEN0_write_data_mask);   	//clken_irda_off:1
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_iso_misc_off_shift) | STB_ST_CLKEN0_write_data_mask); // ISO_MISC_OFF clock
		rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_iso_misc_off_shift) | STB_ST_SRST0_write_data_mask);  // ISO_MISC_OFF reset
	} else {
		/* HW mode */
		rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_irda_shift) | STB_ST_SRST1_write_data_mask);  // ISO_MISC_IRDA reset
		/* RAW mode */
		rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_irda_off_shift) | STB_ST_SRST0_write_data_mask);  // ISO_MISC_OFF reset
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_I2C)
int I2C_CriticalRegister(enum CRT_CLK clken , void *private_data)
{
	int i2c_phy = *(int *)private_data;

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		if (i2c_phy & BIT(0)) {
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c0_off_shift));
			rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_i2c0_off_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c0_off_shift)| BIT(STB_ST_CLKEN0_write_data_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c0_off_shift));
			rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_i2c0_off_shift)| BIT(STB_ST_SRST0_write_data_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c0_off_shift)| BIT(STB_ST_CLKEN0_write_data_shift));
		}
		if (i2c_phy & BIT(1)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c1_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c1_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c1_shift)| BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c1_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c1_shift)| BIT(SYS_REG_SYS_SRST2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c1_shift)| BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
		}
		if (i2c_phy & BIT(2)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c2_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c2_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c2_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c2_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c2_shift) | BIT(SYS_REG_SYS_SRST2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c2_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
		}/*
		if (i2c_phy & BIT(3)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c3_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c3_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c3_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c3_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c3_shift) | BIT(SYS_REG_SYS_SRST2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c3_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
		}
		if (i2c_phy & BIT(4)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c4_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c4_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c4_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c4_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_i2c4_shift) | BIT(SYS_REG_SYS_SRST2_write_data_shift));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c4_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
		}
		if (i2c_phy & BIT(5)) {
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c5_off_shift));
			rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_i2c5_off_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c5_off_shift) | BIT(STB_ST_CLKEN0_write_data_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c5_off_shift));
			rtd_outl(STB_ST_SRST0_reg, BIT(STB_ST_SRST0_rstn_i2c5_off_shift) | BIT(STB_ST_SRST0_write_data_shift));
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c5_off_shift) | BIT(STB_ST_CLKEN0_write_data_shift));
		}*/

	} else {
		if (i2c_phy & BIT(0))
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c0_off_shift));

		if (i2c_phy & BIT(1))
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c1_shift));

		if (i2c_phy & BIT(2))
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c2_shift));
/*
		if (i2c_phy & BIT(3))
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c3_shift));

		if (i2c_phy & BIT(4))
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_i2c4_shift));

		if (i2c_phy & BIT(5))
			rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_i2c5_off_shift));
*/
	}

	hw_semaphore_unlock();

	return CRT_SUCCESS;
}
#endif




int UART_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	int systemConsole = 0;

	if (private_data) {
		systemConsole = *(int *)private_data;
	} else {
		rtd_pr_crt_warn(" UART_CriticalRegister without parivate_data parameter\n");
	}

	hw_semaphore_try_lock();
	switch (systemConsole) {
	case 1:
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_uart1_shift));
		rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_uart1_mask);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_uart1_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_uart1_shift));
		rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_uart1_mask | SYS_REG_SYS_SRST2_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_uart1_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);
		rtd_clearbits(SYS_REG_SYS_CLKSEL_reg, BIT(0));	 // set uart1 to 98MHz
		rtd_maskl(PLL27X_REG_SYS_PLL_AUD3_reg, 0xfffffff2, BIT(3));
		break;
	case 2:
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_uart2_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);
		break;
	case 3:
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(6));
		break;
	default:
		hw_semaphore_unlock();
		return CRT_FAIL;

	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int SE_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {

		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_se_shift));
		udelay(10);

		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
        //no need on rtd287x  
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se2_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(10);

		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se_shift));
        //no need on rtd287x
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se2_shift));
		udelay(10);

		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_se_shift));
		udelay(10);

		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_se_shift) | SYS_REG_SYS_SRST1_write_data_mask);
		udelay(10);

		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
        //no need on rtd287x
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se2_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(10);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se_shift));
        //no need on rtd287x
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_se2_shift));
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_se_shift));
		udelay(10);
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int NAND_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	struct NF_CLK_INFO *p_nf_clk = (struct NF_CLK_INFO *)private_data;

	hw_semaphore_try_lock();

	if (p_nf_clk == NULL) {
		//setting nf clk on/off
		if (clken == CLK_ON) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_nf_mask | SYS_REG_SYS_CLKEN2_write_data_mask);
		} else {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_nf_mask);
		}
	} else {
		if (p_nf_clk->clk_set_or_get == NF_CLK_SET) {
			//setting nf_clksel
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_nf_mask);
			rtd_outl(SYS_REG_SYS_CLKDIV_reg, (rtd_inl(SYS_REG_SYS_CLKDIV_reg) & (~SYS_REG_SYS_CLKDIV_nf_clksel_mask)) | SYS_REG_SYS_CLKDIV_nf_clksel(p_nf_clk->nf_clksel));
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_nf_mask | SYS_REG_SYS_CLKEN2_write_data_mask);
		} else {
			//getting current nf clk info
			p_nf_clk->clk_is_on = SYS_REG_SYS_CLKEN2_get_clken_nf(rtd_inl(SYS_REG_SYS_CLKEN2_reg));
			if (p_nf_clk->clk_is_on == 1) {
				//getting current nf_clksel info
				p_nf_clk->nf_clksel = SYS_REG_SYS_CLKDIV_get_nf_clksel(rtd_inl(SYS_REG_SYS_CLKDIV_reg));
			}
		}
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int MD_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_md_shift));
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_md_shift));
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_md_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_md_shift));
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_md_shift) | SYS_REG_SYS_SRST1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_md_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_md_shift));
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
int DOLBY_HDR_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_hdr1_comp_shift)|BIT(SYS_REG_SYS_CLKEN1_clken_hdr1_dm_shift)| SYS_REG_SYS_CLKEN1_write_data_mask);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_hdr1_comp_shift)|BIT(SYS_REG_SYS_CLKEN1_clken_hdr1_dm_shift));
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
int GPU_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {

		//disable GPU SRST0 RESET (bit usage changed)
		//rtd_setbits(SYS_REG_SYS_SRST0_reg, BIT(19));
		//udelay(10);

		//reg = SysReadHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_SRST0_OFFSET) | (
		//SysWriteHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_SRST0_OFFSET, reg);
		//OSWaitus(100);
		//enable GPU CLK
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_3d_gde_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(10);

		//reg = SysReadHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_CLK_OFFSET) | (_B
		//SysWriteHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_CLK_OFFSET, reg);
		//OSWaitus(100);

		//disable GPU RESET
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_3d_gde_shift) | SYS_REG_SYS_SRST1_write_data_mask);
		udelay(10);
		//reg = SysReadHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_RESET_OFFSET) |
		//SysWriteHWReg((void *)SYS_REG_BASE, SYS_REG_GPU_RESET_OFFSET, reg);
		//OSWaitus(100);
        //rtd_pr_crt_err("func=%s line=%d SYS_CLKEN1_reg=%x SYS_SRST1_reg=%x \n",__FUNCTION__,__LINE__,rtd_inl(SYS_REG_SYS_CLKEN1_reg),rtd_inl(SYS_REG_SYS_SRST1_reg));
		rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, BIT(SYS_REG_INT_CTRL_SCPU_gpu_int_scpu_routing_en_shift) | SYS_REG_INT_CTRL_SCPU_write_data_mask);
		
	} else if (clken == CLK_OFF) {
		//do nothing
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int PWM_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	int pwm_type = *(int *)private_data;
	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		if (pwm_type == 0) {
			/*iso*/
 			rtd_outl(STB_ST_CLKEN0_reg,
 						BIT(STB_ST_CLKEN0_clken_pwm_shift) | STB_ST_CLKEN0_write_data_mask);
			rtd_outl(STB_ST_CLKEN0_reg,
						BIT(STB_ST_CLKEN0_clken_pwm_shift));
			rtd_outl(STB_ST_SRST0_reg,
						BIT(STB_ST_SRST0_rstn_pwm_shift));
			rtd_outl(STB_ST_SRST0_reg,
						BIT(STB_ST_SRST0_rstn_pwm_shift) | STB_ST_SRST0_write_data_mask);
			rtd_outl(STB_ST_CLKEN0_reg,
						BIT(STB_ST_CLKEN0_clken_pwm_shift) | STB_ST_CLKEN0_write_data_mask);
		} else {
			/*misc*/
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						BIT(SYS_REG_SYS_CLKEN2_clken_pwm_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						BIT(SYS_REG_SYS_CLKEN2_clken_pwm_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						BIT(SYS_REG_SYS_SRST2_rstn_pwm_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						BIT(SYS_REG_SYS_SRST2_rstn_pwm_shift) | SYS_REG_SYS_SRST2_write_data_mask);
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						BIT(SYS_REG_SYS_CLKEN2_clken_pwm_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);
		}

	} else {
		if (pwm_type == 0) {
			rtd_outl(STB_ST_CLKEN0_reg,
						  BIT(STB_ST_CLKEN0_clken_pwm_shift));
			rtd_outl(STB_ST_SRST0_reg,
						  BIT(STB_ST_SRST0_rstn_pwm_shift));
		} else {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg,
						  BIT(SYS_REG_SYS_CLKEN2_clken_pwm_shift));
			rtd_outl(SYS_REG_SYS_SRST2_reg,
						  BIT(SYS_REG_SYS_SRST2_rstn_pwm_shift));
		}
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
#ifdef CONFIG_RTK_KDRV_MIO
int PCMCIA_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/*pcmcia rbus wrapper clock */
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_pcmcia_shift));   /* disable clock */
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_pcmcia_shift));  /* assert pcmcia */
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_pcmcia_shift) | BIT(SYS_REG_SYS_SRST2_write_data_shift));    /* deassert pcmcia */
		udelay(10);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_pcmcia_shift) | BIT(SYS_REG_SYS_CLKEN2_write_data_shift));
		/* PCMCIA mio clock */
		rtd_outl(ISO_MIO_STANDBY_SYS_CLKEN_reg, BIT(ISO_MIO_STANDBY_SYS_CLKEN_clken_pcmcia_wrapper_shift)); /* disable clock */
		udelay(10);
		rtd_outl(ISO_MIO_STANDBY_SYS_SRST_reg, BIT(ISO_MIO_STANDBY_SYS_SRST_rstn_pcmcia_wrapper_shift));    /* assert pcmcia */
		udelay(10);
		rtd_outl(ISO_MIO_STANDBY_SYS_SRST_reg, BIT(ISO_MIO_STANDBY_SYS_SRST_rstn_pcmcia_wrapper_shift) | BIT(ISO_MIO_STANDBY_SYS_SRST_write_data_shift));   /* deassert pcmcia */
		udelay(10);
		rtd_outl(ISO_MIO_STANDBY_SYS_CLKEN_reg, BIT(ISO_MIO_STANDBY_SYS_CLKEN_clken_pcmcia_wrapper_shift) | BIT(ISO_MIO_STANDBY_SYS_CLKEN_write_data_shift));
	} else {
		/*pcmcia rbus wrapper clock */
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_pcmcia_shift));   /* disable clock  */
		/* PCMCIA mio clock */
		rtd_outl(ISO_MIO_STANDBY_SYS_CLKEN_reg, BIT(ISO_MIO_STANDBY_SYS_CLKEN_clken_pcmcia_wrapper_shift)); /* disable clock  */
	}
		hw_semaphore_unlock();
		return CRT_SUCCESS;
}
#else
int PCMCIA_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_pc1_shift));	/* disable clock */
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST5_reg, BIT(SYS_REG_SYS_SRST5_rstn_pc1_shift));	/* assert pcmcia */
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST5_reg, BIT(SYS_REG_SYS_SRST5_rstn_pc1_shift) | BIT(SYS_REG_SYS_SRST5_write_data_shift)); 	/* deassert pcmcia */
		udelay(10);
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_pc1_shift) | BIT(SYS_REG_SYS_CLKEN5_write_data_shift));
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_pc1_shift));	/* disable clock  */
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}
#endif
int CEC_CriticalRegister(enum CRT_CLK clken)
{

    hw_semaphore_try_lock();
    if (clken == CLK_ON) {
        rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_cec_shift));
        udelay(10);
        rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_cec_shift));
        udelay(10);
        rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_cec_shift) | BIT(STB_ST_SRST1_write_data_shift));
        udelay(10);
        rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_cec_shift) |BIT(STB_ST_CLKEN1_write_data_shift));
        udelay(10);
    } else {

    }
    hw_semaphore_unlock();
    return CRT_SUCCESS;
}

int IRTX_CriticalRegister(enum CRT_CLK clken)
{

    hw_semaphore_try_lock();
    if (clken == CLK_ON) {
        rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_irtx_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_irtx_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_irtx_shift) | BIT(SYS_REG_SYS_SRST2_write_data_shift));
        udelay(10);
        rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_irtx_shift) |BIT(SYS_REG_SYS_SRST2_write_data_shift));
        udelay(10);
    } else {
        rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_irtx_shift));
        rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_irtx_shift));
    }
    hw_semaphore_unlock();
    return CRT_SUCCESS;
}



int LSADC_CriticalRegister(enum CRT_CLK clken)
{

    hw_semaphore_try_lock();
    if (clken == CLK_ON) {
        rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_lsadc_shift));
        udelay(10);
        rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_lsadc_shift));
        udelay(10);
        rtd_outl(STB_ST_SRST1_reg, BIT(STB_ST_SRST1_rstn_lsadc_shift) | BIT(STB_ST_SRST1_write_data_shift));
        udelay(10);
        rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_lsadc_shift) |BIT(STB_ST_CLKEN1_write_data_shift));
        udelay(10);
    } else {
        rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_lsadc_shift));  /* disable clock  */
    }
    hw_semaphore_unlock();
    return CRT_SUCCESS;
}


int GPIO_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
#if (0)
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_gpio_mask | SYS_REG_SYS_CLKEN2_write_data(1));        
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_gpio_mask | SYS_REG_SYS_CLKEN2_write_data(0));		   		   		           
		rtd_outl(SYS_REG_SYS_SRST2_reg,  SYS_REG_SYS_SRST2_rstn_gpio_mask | SYS_REG_SYS_CLKEN2_write_data(0));   // do reset		
		rtd_outl(SYS_REG_SYS_SRST2_reg,  SYS_REG_SYS_SRST2_rstn_gpio_mask | SYS_REG_SYS_CLKEN2_write_data(1));   // release reset		
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_gpio_mask | SYS_REG_SYS_CLKEN2_write_data(1));		   		           
#endif
		rtd_outl(STB_ST_CLKEN0_reg, STB_ST_CLKEN0_clken_iso_misc_mask | STB_ST_CLKEN0_write_data(1)); // ISO_MISC clock		
		rtd_outl(STB_ST_SRST0_reg,  STB_ST_SRST0_rstn_iso_misc_mask | STB_ST_SRST0_get_write_data(1));  // release ISO_MISC reset
	} else {

	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int EMMC_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {

		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_emmc_shift));
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_emmc_shift));
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_emmc_shift) | SYS_REG_SYS_SRST2_rstn_emmc_mask);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_emmc_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);

	} else {
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_emmc_shift));
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_emmc_shift));
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int SDIO_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {

		rtd_outl(SYS_REG_SYS_CLKEN3_reg, BIT(SYS_REG_SYS_CLKEN3_clken_sd_shift));
		rtd_outl(SYS_REG_SYS_SRST3_reg, BIT(SYS_REG_SYS_SRST3_rstn_sd_shift));
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST3_reg, BIT(SYS_REG_SYS_SRST3_rstn_sd_shift) | SYS_REG_SYS_SRST3_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN3_reg, BIT(SYS_REG_SYS_CLKEN3_clken_sd_shift) | SYS_REG_SYS_CLKEN3_write_data_mask);

		/* enable interrupt */
		rtd_setbits(SYS_REG_INT_CTRL_SCPU_reg, BIT(SYS_REG_INT_CTRL_SCPU_sd_int_scpu_routing_en_shift));
		udelay(10);

	} else {
		rtd_outl(SYS_REG_SYS_CLKEN3_reg, BIT(SYS_REG_SYS_CLKEN3_clken_sd_shift));
		rtd_outl(SYS_REG_SYS_SRST3_reg, BIT(SYS_REG_SYS_SRST3_rstn_sd_shift));

		/* disable interrupt */
		rtd_clearbits(SYS_REG_INT_CTRL_SCPU_reg, BIT(SYS_REG_INT_CTRL_SCPU_sd_int_scpu_routing_en_shift));
		rtd_clearbits(SYS_REG_INT_CTRL_KCPU_reg, BIT(SYS_REG_INT_CTRL_KCPU_sd_int_kcpu_routing_en_shift));
		udelay(10);
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int SMC_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_sc_shift));   /* Disable Clock */
		udelay(100);
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_sc_shift));   /*HW Reset*/
		udelay(100);
		rtd_outl(SYS_REG_SYS_SRST2_reg, BIT(SYS_REG_SYS_SRST2_rstn_sc_shift) | SYS_REG_SYS_SRST2_write_data_mask);     /* HW Reset*/
		udelay(100);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_sc_shift) | SYS_REG_SYS_CLKEN2_write_data_mask);     /* Enable Clock*/
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, BIT(SYS_REG_SYS_CLKEN2_clken_sc_shift));   /* Disable Clock */
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int SMC_MIO_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_sc1_shift));   /* Disable Clock */
		udelay(100);
		rtd_outl(SYS_REG_SYS_SRST5_reg, BIT(SYS_REG_SYS_SRST5_rstn_sc1_shift));   /*HW Reset*/
		udelay(100);
		rtd_outl(SYS_REG_SYS_SRST5_reg, BIT(SYS_REG_SYS_SRST5_rstn_sc1_shift) | SYS_REG_SYS_SRST5_write_data_mask);     /* HW Reset*/
		udelay(100);
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_sc1_shift) | SYS_REG_SYS_CLKEN5_write_data_mask);     /* Enable Clock*/
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN5_reg, BIT(SYS_REG_SYS_CLKEN5_clken_sc1_shift));   /* Disable Clock */
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int CP_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* CLKEN = "0" -> RSTN = "0" -> RSTN = "1" -> CLKEN = "1" */
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_cp_shift));
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_cp_shift));
		udelay(10);
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_cp_shift) | SYS_REG_SYS_SRST1_write_data_mask);
		udelay(10);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_cp_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(10);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_cp_shift));
		udelay(10);
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int TP_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {

		/*  External Reset TP (0: reset ; 1: normal) */
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tp_mask);
		udelay(100);

		/* enable TP clock */
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tp_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(100);

		/*  External Reset TP */
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tp_mask | SYS_REG_SYS_SRST1_write_data_mask);
		udelay(100);

	} else {
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tp_mask);
		udelay(100);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tp_mask);
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}



int TPOUT_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* enable TPOUT clock */
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tp_div_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(100);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tp_div_mask);
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}



int MHL_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* Module Clock & Reset Enable */
		rtd_outl(SYS_REG_SYS_SRST1_reg,
				  (SYS_REG_SYS_SRST1_rstn_hdmi_mask |
				   SYS_REG_SYS_SRST1_rstn_vodma1_mask |
				   SYS_REG_SYS_SRST1_write_data_mask));

		/* STANDBY Module Clock & Reset Enable */
		rtd_outl(STB_ST_SRST1_reg,
					(STB_ST_SRST1_rstn_cbus_mask |
					STB_ST_SRST1_rstn_alu_mask |
					STB_ST_SRST1_write_data_mask));

		rtd_outl(STB_ST_CLKEN1_reg,
					(STB_ST_CLKEN1_clken_cbus_mask |
					STB_ST_CLKEN1_clken_alu_mask |
					STB_ST_CLKEN1_write_data_mask));

		rtd_maskl(STB_ALU_IE_reg, ~STB_ALU_IE_stie_cbus_mask, STB_ALU_IE_stie_cbus_mask);  /* enable STB_ALU_IE interrupt enable*/
		rtd_outl(STB_ALU_IP_reg, STB_ALU_IP_stip_cbus_mask);  /* clear pending flag of cbus*/
	} else {
		/* do nothing */
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int HDMI_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* HDMI & APLL &cbus & off measure */
		rtd_outl(SYS_REG_SYS_SRST1_reg,
					(SYS_REG_SYS_SRST1_rstn_hdmi_mask |
					SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
					SYS_REG_SYS_SRST1_rstn_offms_mask |
					/*SYS_REG_SYS_SRST1_rstn_cbus_mask |*/
					SYS_REG_SYS_SRST1_write_data_mask));

		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
					(SYS_REG_SYS_CLKEN1_clken_hdmi_mask |
					SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
					SYS_REG_SYS_CLKEN1_clken_offms_mask |
					/*SYS_REG_SYS_CLKEN1_clken_cbus_mask |*/
					SYS_REG_SYS_CLKEN1_write_data_mask));
		/* DDC */
		//rtd_outl(SYS_REG_SYS_SRST3_reg, SYS_REG_SYS_SRST3_rstn_ddc_mask | SYS_REG_SYS_SRST3_write_data_mask);
		//rtd_outl(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_clken_ddc_mask | SYS_REG_SYS_CLKEN3_write_data_mask);
	} else {
		/* HDMI & APLL &cbus & off measure */
		rtd_outl(SYS_REG_SYS_SRST1_reg,
					(SYS_REG_SYS_SRST1_rstn_hdmi_mask |
					SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
					SYS_REG_SYS_SRST1_rstn_offms_mask));
					//SYS_REG_SYS_SRST1_rstn_cbus_mask));

		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
					(SYS_REG_SYS_CLKEN1_clken_hdmi_mask |
					SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
					SYS_REG_SYS_CLKEN1_clken_offms_mask));
					//SYS_REG_SYS_CLKEN1_clken_cbus_mask));
		/* DDC */
		//rtd_outl(SYS_REG_SYS_SRST3_reg, SYS_REG_SYS_SRST3_rstn_ddc_mask);
		//rtd_outl(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_clken_ddc_mask);
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}


int GDMA_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	int src_type = -1;

	if (private_data)
		src_type = *(int *)private_data;
	else
		rtd_pr_crt_warn(" GDMA_CriticalRegister without parivate_data parameter\n");


	hw_semaphore_try_lock();

	if (clken == CLK_ON) {

		if (src_type == 0) {	/* enable/disable clken_disp_gdma */
			rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg) |
					 SYS_REG_SYS_DISPCLKSEL_clken_disp_gdma(1));
		}
	} else {
		if (src_type == 0) {
			rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg, rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg) & ~SYS_REG_SYS_DISPCLKSEL_clken_disp_gdma_mask);
		}
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}
int VBI_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_vbis0_shift) | SYS_REG_SYS_SRST1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_vbis0_shift) | SYS_REG_SYS_CLKEN1_write_data_mask);
		udelay(10);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, BIT(SYS_REG_SYS_CLKEN1_clken_vbis0_shift));
		rtd_outl(SYS_REG_SYS_SRST1_reg, BIT(SYS_REG_SYS_SRST1_rstn_vbis0_shift));
		udelay(10);
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int ETH_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	int wol_mode = *(int *)private_data;
	int count = 0;

	hw_semaphore_try_lock();

	if (clken == CLK_ON && wol_mode == 0) {
		/* Enable SCPU ETN interrupt and disable KCPU ETN interrupt */
		rtd_outl(SYS_REG_INT_CTRL_KCPU_reg, SYS_REG_INT_CTRL_KCPU_etn_int_kcpu_routing_en_mask);
		rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_etn_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data_mask);
	}
	else if(clken == CLK_ON && wol_mode == 1) {
		/* GETN_RSTN_MASK bit[30]=0 */
		rtd_outl(STB_ST_CTRL0_reg,STB_ST_CTRL0_xtal_en_status_mask|STB_ST_CTRL0_etn_clk_sel_rbus_mask|STB_ST_CTRL0_etn_ldo_en_mask|STB_ST_CTRL0_rstn_mt_mask);
		/* GETN RBUS & GETN clock */
		rtd_outl(STB_ST_CLKEN0_reg, STB_ST_CLKEN0_clken_etn_rbus_mask | STB_ST_CLKEN0_clken_etn_mask | STB_ST_CLKEN0_write_data_mask); // CLKEN_GETN_RBUS bit[11]=1,CLKEN_GETN bit[8]=1
		rtd_outl(STB_ST_SRST0_reg, STB_ST_SRST0_rstn_etn_mask | STB_ST_SRST0_write_data_mask); // RSTN_GETN_bit[8]=1

		rtd_outl(PLLETN_GETN_CLKEN_reg, PLLETN_GETN_CLKEN_clken_getn_mac_mask | PLLETN_GETN_CLKEN_clken_getn_sys_mask);  //CLKEN_GETN_MAC bit[2] = 0, CLKEN_GETN_SYS[1] = 0
		rtd_outl(STB_ST_CLKEN0_reg, STB_ST_CLKEN0_clken_etn_rbus_mask | STB_ST_CLKEN0_clken_etn_mask); // CLKEN_GETN_RBUS bit[11]=0,CLKEN_GETN bit[8]=0
		rtd_outl(STB_ST_SRST0_reg, STB_ST_SRST0_rstn_etn_mask); //RSTN_GETN bit[8] = 0
		rtd_outl(STB_ST_SRST0_reg, STB_ST_SRST0_rstn_etn_mask | STB_ST_SRST0_write_data_mask);  // RSTN_GETN bit[8] = 1
		rtd_outl(STB_ST_CLKEN0_reg, STB_ST_CLKEN0_clken_etn_rbus_mask | STB_ST_CLKEN0_clken_etn_mask | STB_ST_CLKEN0_write_data_mask); // CLKEN_GETN_RBUS bit[11]=1,CLKEN_GETN bit[8]=1

		/* AFE Power State */
		rtd_outl(PLLETN_ETN_phy_info_reg_reg,PLLETN_ETN_phy_info_reg_afe_pow_state_mask
		                    | PLLETN_ETN_phy_info_reg_por_dvth_cen(1)
		                    | PLLETN_ETN_phy_info_reg_por_avth_cen(1));
		/* PLL ETN */
		rtd_outl(PLLETN_GETN_RST_reg, PLLETN_GETN_RST_rstn_getn_sys_mask | PLLETN_GETN_RST_rstn_getn_mac_mask | PLLETN_GETN_RST_rstn_getn_phy_mask | PLLETN_GETN_RST_write_data_mask);  //RSTN_GETN_PHY bit[3] = 1, RSTN_GETN_MAC bit[2] = 1, RSTN_GETN_SYS[1] = 1
		rtd_outl(PLLETN_GETN_CLKEN_reg, PLLETN_GETN_CLKEN_clken_getn_mac_mask | PLLETN_GETN_CLKEN_clken_getn_sys_mask | PLLETN_GETN_CLKEN_write_data_mask);  //CLKEN_GETN_MAC bit[2] = 1, CLKEN_GETN_SYS[1] = 1
		
		/* Check init_autoload_done = 1 */
		while(!(rtd_inl(PLLETN_INIT_AUTOLOAD_DONE_reg) & PLLETN_INIT_AUTOLOAD_DONE_init_autoload_done_mask) && count < 500) {    // wait plletn init done
			mdelay(1);
			count++;
		}
		if(count == 500) {
			rtd_pr_crt_warn("[ETH] initdone failed!(%08x)\n", rtd_inl(PLLETN_INIT_AUTOLOAD_DONE_reg));
			return CRT_FAIL;
		}
	}
	else if(clken == CLK_OFF && wol_mode == 1) {
		rtd_setbits(STB_ST_CTRL0_reg, BIT(STB_ST_CTRL0_etn_rstn_mask_shift));
		rtd_outl(PLLETN_GETN_CLKEN_reg, BIT(PLLETN_GETN_CLKEN_clken_getn_sys_shift));
		rtd_outl(PLLETN_GETN_RST_reg, BIT(PLLETN_GETN_RST_rstn_getn_sys_shift));
		rtd_outl(STB_ST_CLKEN0_reg, BIT(STB_ST_CLKEN0_clken_etn_rbus_shift));
	}
	else {
		/* disable interrupt */
		rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_etn_int_scpu_routing_en_mask);
	}

	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int DiseqC_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//---------------------------------------------
		// Enable Demod Clock / Reset
		// Note : DiseqC has dependency issue with DEMOD
		//        so we have to enable demod first
		//---------------------------------------------
		hw_semaphore_unlock();
		//DDEMOD_CriticalRegister(CLK_ON);
		hw_semaphore_try_lock();
		//-------------------------
		rtd_outl(SYS_REG_SYS_SRST0_reg,SYS_REG_SYS_SRST0_rstn_diseqc(1)|
                                       SYS_REG_SYS_SRST0_write_data(1));
		udelay(50);
		rtd_outl(SYS_REG_SYS_CLKEN0_reg,SYS_REG_SYS_CLKEN0_clken_diseqc(1)|
                                        SYS_REG_SYS_CLKEN0_get_write_data(1));
		udelay(10);
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_diseqc_mask);
		udelay(50);
		rtd_outl(SYS_REG_SYS_SRST0_reg,  SYS_REG_SYS_SRST0_rstn_diseqc_mask);
		udelay(10);
	}

	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int WOV_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();

	if (clken == CLK_ON) {
		/* HW mode */
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_wov_shift));   //CLKEN_WOV:0
		rtd_outl(STB_ST_SRST1_reg,  BIT(STB_ST_SRST1_rstn_wov_shift));     //RSTN_WOV:0
		rtd_outl(STB_ST_SRST1_reg,  BIT(STB_ST_SRST1_rstn_wov_shift) | STB_ST_SRST1_write_data_mask);    	//RSTN_WOV:1
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_wov_shift) | STB_ST_CLKEN1_write_data_mask);   	//CLKEN_WOV:1
	} else {
		rtd_outl(STB_ST_CLKEN1_reg, BIT(STB_ST_CLKEN1_clken_wov_shift));   //CLKEN_WOV:0
		rtd_outl(STB_ST_SRST1_reg,  BIT(STB_ST_SRST1_rstn_wov_shift));     //RSTN_WOV:0
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}


int TV_All_CriticalRegister(enum CRT_CLK clken)
{
	/* 
	//from script   Demod(A+D) CLK / Reast set to 0
    //ATV turn off	
    rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00800000);							//RSTN_IF_DEMOD<bit23> set to 0(active low)
    rtd_outl(0xb8000108, 0x00400800);							//RSTN_IF_DEMOD_logic<bit22> set to 0(active low)
    rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00800000);							//CLKEN_IF_DEMOD<bit23> set to 0(CLK off)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod IP reset
    rtd_outl(0xb8000100, 0x01182000);							//Demod IP Reset<bit24, bit20, bit19, bit13> set to 0(active low)
    rtd_outl(0xb8000108, 0x00000800);							//Demod IP Reset<bit11> set to 0(active low)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod Wrapper reset
    rtd_outl(0xb8000100, 0x80e00000);							//Demod IP Reset<bit31, bit23, bit22, bit21> set to 0(active low)
    rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00000002);							//RSTN_dtv_demod_mb<bit2> set to 0(active low)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod turn off
    rtd_outl(0xb8000110, 0x81d82000);							//Demod CLK<bit31, bit24, bit23, bit22, bit20, bit19, bit13> set to 0
    rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x00000800);							//CLKEN_DTV_DEMOD<bit11> set to 0
    ScalerTimer_DelayXms(1);	
	*/
	
	hw_semaphore_try_lock();        //DTV Demod turn off(CLK/RESET), Demod IP off frist then wrapper 
    if (clken == CLK_OFF) {
        //ATV_IFD RESET and CLK OFF
        rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_if_demod(1)|SYS_REG_SYS_SRST1_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_if_demod_logic(1)|SYS_REG_SYS_SRST1_write_data(0));
        rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod(1)|SYS_REG_SYS_CLKEN1_write_data(0));

        //DTV_IP reset
        rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod(1)|SYS_REG_SYS_SRST0_rstn_hdic(1)|SYS_REG_SYS_SRST0_rstn_dtv_frontend(1)|SYS_REG_SYS_SRST0_rstn_rtk_demod(1)|SYS_REG_SYS_SRST0_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_dtv_demod(1)|SYS_REG_SYS_SRST2_write_data(0));
        mdelay(1);
        //DTV_warpper reset
        rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_diseqc(1)|SYS_REG_SYS_SRST0_rstn_atb_demod_wrap(1)|SYS_REG_SYS_SRST0_rstn_hdic_apb(1)|SYS_REG_SYS_SRST0_rstn_hdic_axi(1)|SYS_REG_SYS_SRST0_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_dtv_demod_mb(1)|SYS_REG_SYS_SRST1_write_data(0));
        mdelay(1);
        //turn off clk
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_diseqc(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1)|SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1)|SYS_REG_SYS_CLKEN0_clken_hdic(1)|SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1)|SYS_REG_SYS_CLKEN0_clken_rtk_demod(1)|SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1)|SYS_REG_SYS_CLKEN2_write_data(0));
		udelay(10);
	} else {
		//please turn on by tv standard...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;

}


int TV_AllISDBT_CriticalRegister(enum CRT_CLK clken)
{
	/* 
	//from script   Demod(A+D) CLK / Reast set to 0
    //ATV turn off	
    rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00800000);							//RSTN_IF_DEMOD<bit23> set to 0(active low)
    rtd_outl(0xb8000108, 0x00400800);							//RSTN_IF_DEMOD_logic<bit22> set to 0(active low)
    rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00800000);							//CLKEN_IF_DEMOD<bit23> set to 0(CLK off)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod IP reset
    rtd_outl(0xb8000100, 0x01182000);							//Demod IP Reset<bit24, bit20, bit19, bit13> set to 0(active low)
    rtd_outl(0xb8000108, 0x00000800);							//Demod IP Reset<bit11> set to 0(active low)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod Wrapper reset
    rtd_outl(0xb8000100, 0x80e00000);							//Demod IP Reset<bit31, bit23, bit22, bit21> set to 0(active low)
    rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00000002);							//RSTN_dtv_demod_mb<bit2> set to 0(active low)
    ScalerTimer_DelayXms(1);									//driver delay 1mS

    //DTV Demod turn off
    rtd_outl(0xb8000110, 0x81d82000);							//Demod CLK<bit31, bit24, bit23, bit22, bit20, bit19, bit13> set to 0
    rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x00000800);							//CLKEN_DTV_DEMOD<bit11> set to 0
    ScalerTimer_DelayXms(1);	
	*/
	
	hw_semaphore_try_lock();        //DTV Demod turn off(CLK/RESET), Demod IP off frist then wrapper 
    if (clken == CLK_OFF) {
		rtd_maskl(TVSB5_TV_SB5_DCU1_ARB_CR1_reg, ~(0x00000002 | 0x00000001), 0x0);//TVSB5 don't process command from cliend(demod/demoddbg)
		mdelay(1);
        //ATV_IFD RESET and CLK OFF
        rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_if_demod(1)|SYS_REG_SYS_SRST1_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_if_demod_logic(1)|SYS_REG_SYS_SRST1_write_data(0));
        rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod(1)|SYS_REG_SYS_CLKEN1_write_data(0));

        //turn off clk
        rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_diseqc(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1)|SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1)|SYS_REG_SYS_CLKEN0_clken_hdic(1)|SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1)|SYS_REG_SYS_CLKEN0_clken_rtk_demod(1)|SYS_REG_SYS_CLKEN0_write_data(0));
        rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1)|SYS_REG_SYS_CLKEN2_write_data(0));
        mdelay(1);
        //DTV_IP reset
        rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod(1)|SYS_REG_SYS_SRST0_rstn_hdic(1)|SYS_REG_SYS_SRST0_rstn_dtv_frontend(1)|SYS_REG_SYS_SRST0_rstn_rtk_demod(1)|SYS_REG_SYS_SRST0_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_dtv_demod(1)|SYS_REG_SYS_SRST2_write_data(0));
        mdelay(1);
        //DTV_warpper reset
        rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_diseqc(1)|SYS_REG_SYS_SRST0_rstn_atb_demod_wrap(1)|SYS_REG_SYS_SRST0_rstn_hdic_apb(1)|SYS_REG_SYS_SRST0_rstn_hdic_axi(1)|SYS_REG_SYS_SRST0_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_dtv_demod_mb(1)|SYS_REG_SYS_SRST1_write_data(0));
        mdelay(1);
        //turn off clk
        rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_diseqc(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod(1)|SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1)|SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1)|SYS_REG_SYS_CLKEN0_clken_hdic(1)|SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1)|SYS_REG_SYS_CLKEN0_clken_rtk_demod(1)|SYS_REG_SYS_CLKEN0_write_data(0));
        rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1)|SYS_REG_SYS_CLKEN2_write_data(0));
        udelay(10);
	} else {
		//please turn on by tv standard...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;

}


int TV_Adc_CriticalRegister(enum CRT_CLK clken)
{
	/*
	//from script
    Turn on:    IFADC_CLK do not toggle 
	rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00008001);				//<bit15>CLKEN_IFADC, set to 1
	rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00008001);				//<bit15>RSTN_IFADC, set to 1(Release)
	*/

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_ifadc(1) | SYS_REG_SYS_CLKEN1_write_data(1));
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_ifadc(1) | SYS_REG_SYS_SRST1_write_data(1));
		udelay(10);
	} else{
	    rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_ifadc(1) | SYS_REG_SYS_CLKEN1_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_ifadc(1) | SYS_REG_SYS_SRST1_write_data(0));
		udelay(10);
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int TV_Frontend_CriticalRegister(enum CRT_CLK clken)
{
	/* 
	//from script
	//IFADC, frontend CLK / RESET flow
	//rtd_outl(0xb8000110, 0x00480001);				//<bit22>Clken_dtv_demod_mb, <bit19>CLKEN_dtv_frontend,  set to 1
	//rtd_outl(0xb8000110, 0x00480000);				//<bit22>Clken_dtv_demod_mb, <bit19>CLKEN_dtv_frontend, set to 0
	rtd_outl(0xb8000100, 0x00080001);				//<bit19>RSTN_dtv_frontend,  set to 1(release)
	rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00000003);				//<bit1>RSTN_dtv_demod_mb,  set to 1(release)
	rtd_outl(0xb8000110, 0x00480001);				//<bit22>Clken_dtv_demod_mb, <bit19>CLKEN_dtv_frontend,  set to 1
	ScalerTimer_DelayXms(10);
	*/
	
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1) |SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1) |SYS_REG_SYS_CLKEN0_write_data(1));	
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1) |SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1) |SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_dtv_frontend(1) |SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_dtv_demod_mb(1) |SYS_REG_SYS_SRST1_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dtv_demod_mb(1) |SYS_REG_SYS_CLKEN0_clken_dtv_frontend(1) |SYS_REG_SYS_CLKEN0_write_data(1));

		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int DTV_ExtAtsc3Demod_CriticalRegister(enum CRT_CLK clken)
{
	
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
               //rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_atsc_demod(1)|SYS_REG_SYS_SRST4_write_data(0));
		//rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_atsc_demod(1)|SYS_REG_SYS_SRST4_write_data(1));
		udelay(10);
	} else {
		//rtd_outl(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_atsc_demod(1)|SYS_REG_SYS_SRST4_write_data(0));
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int DTV_Demod_CriticalRegister(enum CRT_CLK clken)
{
	/* 
	//from script
	//DTV_DEMOD CLK / RESET flow
	//rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x00000801);				//<bit11>CLKEN_DTV_DEMOD, set to 1
	//rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x00000800);				//<bit11>CLKEN_DTV_DEMOD, set to 0
	rtd_outl(0xb8000108, 0x00000801);				//<bit11>RSTN_DTV_DEMOD, set to 1(release)
	rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x00000801);				//<bit11>CLKEN_DTV_DEMOD, set to 1
	ScalerTimer_DelayXms(10);
	*/
	
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1) |SYS_REG_SYS_CLKEN2_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1) |SYS_REG_SYS_CLKEN2_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_dtv_demod(1)|SYS_REG_SYS_SRST2_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_dtv_demod(1) |SYS_REG_SYS_CLKEN2_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int DTV_Atb_CriticalRegister(enum CRT_CLK clken)
{
	/*
	//from script
    //DTV_ATB_Demod CLK/Reset Enable,  Demod wrapper frist then Demod IP 						
    //rtd_outl(0xb8000110, 0x00800001);							//<bit23>clken_atb_demod_wrap, <bit0>write bit set to 1
    //rtd_outl(0xb8000110, 0x00800000);							//<bit23>clken_atb_demod_wrap, <bit0>write bit set to 0
    rtd_outl(0xb8000100, 0x00800001);							//<bit23>RSTN_atb_demod_wrap, <bit0>write bit set to 1(release)
    rtd_outl(0xb8000110, 0x00800001);							//<bit23>clken_atb_demod_wrap, <bit0>write bit set to 1
    ScalerTimer_DelayXms(1);									//delay 10uS
    //rtd_outl(0xb8000110, 0x01000001);							//<bit24>clken_atb_demod, <bit0>write bit set to 1
    //rtd_outl(0xb8000110, 0x01000000);							//<bit24>clken_atb_demod, <bit0>write bit set to 0
    rtd_outl(0xb8000100, 0x01000001);							//<bit24>RSTN_atb_demod, <bit0>write bit set to 1(release)
    rtd_outl(0xb8000110, 0x01000001);							//<bit24>clken_atb_demod, <bit0>write bit set to 1
    ScalerTimer_DelayXms(1);									//delay 10uS
	*/

	hw_semaphore_try_lock();    //Demod wrapper turn frist then tuner DEMOD IP
	if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod_wrap(1) |SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
        //rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod(1) |SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int DTV_AtbWoWrap_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();    //Demod wrapper turn frist then tuner DEMOD IP
	if (clken == CLK_ON) {
/*		
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod_wrap(1) |SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod_wrap(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
        //rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(0));
*/
		//Mac8p reuse Atb's DFE, need to enable Atb's demod, except for wrapper
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_atb_demod(1) |SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_atb_demod(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int DTV_Hdic_CriticalRegister(enum CRT_CLK clken)
{
	/*
	//from script
    //DTV_HDIC_Demod CLK/Reset Enable,  Demod wrapper frist then Demod IP 	
    //rtd_outl(0xb8000110, 0x00100001);							//<bit20>CLKEN_hdic, set to 1
    //rtd_outl(0xb8000110, 0x00100000);							//<bit20>CLKEN_hdic, set to 0
    rtd_outl(0xb8000100, 0x00600001);							//<bit21>RSTN_hdic_axi, <bit22>RSTN_hdic_apb set to 1(release)
    ScalerTimer_DelayXms(1);									//udelay(10);
    rtd_outl(0xb8000100, 0x00100001);               			//<bit20>RSTN_hdic set to 1(release)
    ScalerTimer_DelayXms(1);									//udelay(10);
    rtd_outl(0xb8000110, 0x00100001);							//<bit20>CLKEN_hdic, set to 1
    ScalerTimer_DelayXms(10);
	*/

	hw_semaphore_try_lock();  //Demod wrapper turn frist then tuner DEMOD IP
    if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_hdic(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_hdic(1) |SYS_REG_SYS_CLKEN0_write_data(0));
        rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_hdic_axi(1)|SYS_REG_SYS_SRST0_rstn_hdic_apb(1)|SYS_REG_SYS_SRST0_write_data(1));
        udelay(10);
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_hdic(1)|SYS_REG_SYS_SRST0_write_data(1));
        udelay(10);
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_hdic(1) |SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int DTV_Rtk_CriticalRegister(enum CRT_CLK clken)
{
	/*
	//from script
	//RTK Demod CLK / RESET flow
	//rtd_outl(0xb8000110, 0x00002001);				//<bit13>CLKEN_rtk_demod, set to 1
	//rtd_outl(0xb8000110, 0x00002000);				//<bit13>CLKEN_rtk_demod, set to 0
	rtd_outl(0xb8000100, 0x00002001);				//<bit13>RSTN_rtk_demod, set to 1(release)
	rtd_outl(0xb8000110, 0x00002001);				//<bit13>CLKEN_rtk_demod, set to 1
	*/

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_rtk_demod(1) | SYS_REG_SYS_CLKEN0_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_rtk_demod(1) | SYS_REG_SYS_CLKEN0_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_rtk_demod(1) | SYS_REG_SYS_SRST0_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_rtk_demod(1) | SYS_REG_SYS_CLKEN0_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int ATV_CriticalRegister(enum CRT_CLK clken)
{
/*  //from script
    //AtvDemodPowerControl(1) turn on AtvDemod
    //rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00800001);							//<bit23>CLKEN_IF_DEMOD, <bit0>write bit(1)
    //rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00800000);							//<bit23>CLKEN_IF_DEMOD, <bit0>write bit(0)
    rtd_outl(SYS_REG_SYS_SRST1_reg, 0x00800001);							//<bit23>RSTN_IF_DEMOD, <bit0>write bit(1) Reset release 
    rtd_outl(SYS_REG_SYS_CLKEN1_reg, 0x00800001);							//<bit23>CLKEN_IF_DEMOD, <bit0>write bit(1)
    ScalerTimer_DelayXms(1);									//Driver delay 10uS
    //rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x04000001);							//<bit26>CLKEN_ATVIN_IFD, <bit0>write bit(1)
    //rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x04000000);							//<bit26>CLKEN_ATVIN_IFD, <bit0>write bit(0)
    rtd_outl(0xb8000108, 0x0c400001);							//<bit27>RSTN_ATVIN2, <bit26>RSTN_ATVIN1, <bit22>RSTN_IF_DEMOD_logic, <bit0>write bit(1) Reset release 
    rtd_outl(SYS_REG_SYS_CLKEN2_reg, 0x04000001);							//<bit26>CLKEN_ATVIN_IFD, <bit0>write bit(1)
    ScalerTimer_DelayXms(1);									//Driver delay 10uS
*/
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod(1) | SYS_REG_SYS_CLKEN1_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod(1) | SYS_REG_SYS_CLKEN1_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_if_demod(1) | SYS_REG_SYS_SRST1_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod(1) | SYS_REG_SYS_CLKEN1_write_data(1));
		udelay(10);
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_ifd(1) | SYS_REG_SYS_CLKEN2_write_data(1));
		//rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_ifd(1) | SYS_REG_SYS_CLKEN2_write_data(0));
		rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_atvin2(1) | SYS_REG_SYS_SRST2_rstn_atvin(1) | SYS_REG_SYS_SRST2_rstn_if_demod_logic(1) | SYS_REG_SYS_SRST2_write_data(1));
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_ifd(1) | SYS_REG_SYS_CLKEN2_write_data(1));
		udelay(10);
	} else {
		// no need...
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int ADC_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
			//SYS_REG_SYS_SRST1_rstn_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_SRST1_write_data_mask); 	/* Reset release in APLL_ADC&I/M domain */
		udelay(100);//wait apll stable
		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
			SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
			//SYS_REG_SYS_CLKEN1_clken_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_CLKEN1_write_data_mask); /* APLL_ADC  & I/M domain CRT on */
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_apll_adc_mask); /* APLL_ADC CRT off */
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_apll_adc_mask); 	/* APLL_ADC CRT off */
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int Auto_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
			//SYS_REG_SYS_SRST1_rstn_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_SRST1_write_data_mask); 	/* Reset release in APLL_ADC&I/M domain */
		udelay(100);//wait apll stable
		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
			SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
			//SYS_REG_SYS_CLKEN1_clken_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_CLKEN1_write_data_mask); /* APLL_ADC  & I/M domain CRT on */
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int Auto2_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
			//SYS_REG_SYS_SRST1_rstn_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_SRST1_write_data_mask); 	/* Reset release in APLL_ADC&I/M domain */
		udelay(100);//wait apll stable
		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
			SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
			//SYS_REG_SYS_CLKEN1_clken_dispi_mask | seems change to SYS_REG_SYS_SRST3 0x1800010C
			SYS_REG_SYS_CLKEN1_write_data_mask); /* APLL_ADC  & I/M domain CRT on */
	} else {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_apll_adc_mask); /* APLL_ADC CRT off */
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_apll_adc_mask); 	/* Reset  in I/M domain */
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}



int VO_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vodma1_mask);
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vodma1_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vodma1_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vodma1_mask);
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vodma1_mask | SYS_REG_SYS_SRST1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vodma1_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
	} else {

	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;
}

int VFEDEV_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		// Enable offline measure clock
		// 0xb8000104
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_offms_mask | SYS_REG_SYS_SRST1_write_data_mask);

		// 0xb8000114
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int TVE1_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {

#if 1//for ma3 LGE project

		//check[22]CLKEN_VDEC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_vdec_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec_mask | SYS_REG_SYS_SRST1_write_data_mask); //[22]RSTN_VDEC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vdec_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[22]CLKEN_VDEC
		}

		//check [11]RSTN_APLL_ADC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_apll_adc_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_apll_adc_mask | SYS_REG_SYS_SRST1_write_data_mask); //[11]RSTN_APLL_ADC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_apll_adc_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[11]CLKEN_APLL_ADC
		}

		//check [27]RSTN_ATVIN2[26]RSTN_ATVIN
		if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_atvin_vd_mask)) {
			rtd_outl(SYS_REG_SYS_SRST2_reg,
				SYS_REG_SYS_SRST2_rstn_atvin_mask |
				SYS_REG_SYS_SRST2_rstn_atvin2_mask |
				SYS_REG_SYS_SRST2_write_data_mask); //[27]RSTN_ATVIN2[26]RSTN_ATVIN
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_vd_mask | SYS_REG_SYS_CLKEN2_write_data_mask); //[27]CLKEN_ATVIN_VD
		}

		//check [26]CLKEN_ATVIN_IFD
		if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_atvin_ifd_mask)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_ifd_mask | SYS_REG_SYS_CLKEN2_write_data_mask); //[26]CLKEN_ATVIN_IFD
		}

		//check [15]CLKEN_IFADC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_ifadc_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_ifadc_mask | SYS_REG_SYS_SRST1_write_data_mask); //[15]RSTN_IFADC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_ifadc_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[15]CLKEN_IFADC
		}

		//check [23]CLKEN_IF_DEMOD
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_if_demod_mask)) {
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[23]CLKEN_IF_DEMOD
		}

		rtd_maskl(SYS_REG_SYS_CLKSEL_reg, 0xFFFCFFFF, 0x00020000); //[17]ATVIN_CLK_SEL] = 0x1, 0x0: IFD, 0x1: VD [16]ATVIN_VD_CLK_SEL = 0x0, 0x0: 108MHz 0x1: 27MHz

#else
		rtd_maskl(CRT_SYS_SRST1_VADDR, 0xFF9FF7FF, 0x00600800); //[22]RSTN_VDEC[21]RSTN_VDEC2[11]RSTN_APLL_ADC
		rtd_maskl(CRT_SYS_SRST2_VADDR, 0xF3FFFFFF, 0x0C000000); //[27]RSTN_ATVIN2[26]RSTN_ATVIN
		rtd_maskl(CRT_SYS_CLKEN1_VADDR, 0xFFBFF7FF, 0x00400800); //[22]CLKEN_VDEC[11]CLKEN_APLL_ADC
		rtd_maskl(CRT_SYS_CLKEN2_VADDR, 0xFFBFFFFF, 0x00400000); //[27]CLKEN_ATVIN_VD
		rtd_maskl(CRT_SYS_CLKSEL_VADDR, 0xFFFCFFFF, 0x00020000); //[17]ATVIN_CLK_SEL] = 0x1, 0x0: IFD, 0x1: VD [16]ATVIN_VD_CLK_SEL = 0x0, 0x0: 108MHz 0x1: 27MHz

		rtd_maskl(CRT_SYS_SRST1_VADDR, 0xFffF7fFF, 0x00008000); //[15]RSTN_IFADC
		rtd_maskl(CRT_SYS_CLKEN2_VADDR, 0xFbfFFfFF, 0x04000000); //[26]CLKEN_ATVIN_IFD

		rtd_maskl(CRT_SYS_SRST2_VADDR, 0xF3fFFfFF, 0x0c000000); //[26]RSTN_ATVIN ,[27]RSTN_ATVIN2
		rtd_maskl(CRT_SYS_CLKEN1_VADDR, 0xFF7fFFFF, 0x00800000); //[23]CLKEN_IF_DEMOD
		rtd_maskl(CRT_SYS_CLKEN1_VADDR, 0xFFff7FFF, 0x00008000); //[15]CLKEN_IFADC
		//not to disable ATV @ Crixus 20141106
		//rtd_maskl(CRT_SYS_CLKSEL_VADDR,0xFFFCFFFF,0x00000000);
#endif

#if 0//Pll27x setting in bootcode
		//initialize clk 27x domain for VD
		////PLL27x Clock : 108M
		//	rtd_maskl(PLL27X_REG_SYS_PLL_27X1_reg,0xFFFFFFE0,0x0000000E);//432MHzPLL27X_M[6:0] = 0x0E
		rtd_maskl(PLL27X_REG_SYS_PLL_27X1_reg, 0xFFFFFFE0, 0x0000000A); //324MHzPLL27X_M[6:0] = 0x0A

		rtd_maskl(PLL27X_REG_SYS_PLL_27X2_reg, 0xFFFFFFFF, 0x00000000); // do nothing
		//	rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg,0xFDFC0FFF,0x0201F000);//[25]PLL_L2H_CML_POW [17:16]PLL27X_CK108M_DIV = 0x1, divide by 4 (use when VCO=432MHz) [15:14]PLL27X_YPPADC_CKO_SEL = 0x3,YPPADC clock outpu 11:108M [13]PLL27X_YPPADC_CKO_EN[12]PLL27X_YPPADC_CK54M_EN
		rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg, 0xFDFC0FFF, 0x0202F000); //[25]PLL_L2H_CML_POW [17:16]PLL27X_CK108M_DIV = 0x2, divide by 4 (use when VCO=432MHz) [15:14]PLL27X_YPPADC_CKO_SEL = 0x3,YPPADC clock outpu 11:108M [13]PLL27X_YPPADC_CKO_EN[12]PLL27X_YPPADC_CK54M_EN

		rtd_maskl(PLL27X_REG_SYS_PLL_27X3_reg, 0x00000000, 0x00000000); //Power off and rst PLL27x
		rtd_maskl(PLL27X_REG_SYS_PLL_27X3_reg, 0x00000000, 0x00000017); //Power on PLL27x [4]PLL27X_PS_EN[3]PLL27X_PS_DIV2[2]PLL27X_VCORSTB[1]PLL27X_RSTB[0]PLL27X_POW
		mdelay(5);
#endif

		//tve reset
		/*rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tve_mask | SYS_REG_SYS_SRST1_write_data_mask); //[26]RSTN_TVE*/
		//tve clock enable - digital
		/*rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tve_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[26]CLKEN_TVE*/
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int TVE2_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/*rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tve_mask | SYS_REG_SYS_CLKEN1_write_data_mask);*/
	} else {
		/*rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tve_mask);*/
	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int TVE3_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//tve clock disable - digital
		/*rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tve_mask); //[26]CLKEN_TVE*/
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int TVE4_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/*rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tve_mask | SYS_REG_SYS_SRST1_write_data_mask);*/
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int TVE5_CriticalRegister(enum CRT_CLK clken)
{

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//check[22]CLKEN_VDEC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_vdec_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec_mask | SYS_REG_SYS_SRST1_write_data_mask); //[22]RSTN_VDEC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vdec_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[22]CLKEN_VDEC
		}

		//check [11]RSTN_APLL_ADC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_apll_adc_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_apll_adc_mask | SYS_REG_SYS_SRST1_write_data_mask); //[11]RSTN_APLL_ADC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_apll_adc_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[11]CLKEN_APLL_ADC
		}

		//check [27]RSTN_ATVIN2[26]RSTN_ATVIN
		if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_atvin_vd_mask)) {
			rtd_outl(SYS_REG_SYS_SRST2_reg,
				SYS_REG_SYS_SRST2_rstn_atvin_mask |
				SYS_REG_SYS_SRST2_rstn_atvin2_mask |
				SYS_REG_SYS_SRST2_write_data_mask); //[27]RSTN_ATVIN2[26]RSTN_ATVIN
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_vd_mask | SYS_REG_SYS_CLKEN2_write_data_mask); //[27]CLKEN_ATVIN_VD
		}

		//check [26]CLKEN_ATVIN_IFD
		if (!(rtd_inl(SYS_REG_SYS_CLKEN2_reg) & SYS_REG_SYS_CLKEN2_clken_atvin_ifd_mask)) {
			rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_atvin_ifd_mask | SYS_REG_SYS_CLKEN2_write_data_mask); //[26]CLKEN_ATVIN_IFD
		}

		//check [15]CLKEN_IFADC
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_ifadc_mask)) {
			rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_ifadc_mask | SYS_REG_SYS_SRST1_write_data_mask); //[15]RSTN_IFADC
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_ifadc_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[15]CLKEN_IFADC
		}

		//check [23]CLKEN_IF_DEMOD
		if (!(rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_if_demod_mask)) {
			rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_if_demod_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[23]CLKEN_IF_DEMOD
		}

		rtd_maskl(SYS_REG_SYS_CLKSEL_reg, 0xFFFCFFFF, 0x00020000); //[17]ATVIN_CLK_SEL] = 0x1, 0x0: IFD, 0x1: VD [16]ATVIN_VD_CLK_SEL = 0x0, 0x0: 108MHz 0x1: 27MHz

#if 0//Pll27x setting in bootcode
		//initialize clk 27x domain for VD
		////PLL27x Clock : 108M
		//	rtd_maskl(PLL27X_REG_SYS_PLL_27X1_reg,0xFFFFFFE0,0x0000000E);//432MHzPLL27X_M[6:0] = 0x0E
		rtd_maskl(PLL27X_REG_SYS_PLL_27X1_reg, 0xFFFFFFE0, 0x0000000A); //432MHzPLL27X_M[6:0] = 0x0A
		rtd_maskl(PLL27X_REG_SYS_PLL_27X2_reg, 0xFFFFFFFF, 0x00000000); // do nothing
		//	rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg,0xFDFC0FFF,0x0201F000);//[25]PLL_L2H_CML_POW [17:16]PLL27X_CK108M_DIV = 0x1, divide by 4 (use when VCO=432MHz) [15:14]PLL27X_YPPADC_CKO_SEL = 0x3,YPPADC clock outpu 11:108M [13]PLL27X_YPPADC_CKO_EN[12]PLL27X_YPPADC_CK54M_EN
		rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg, 0xFDFC0FFF, 0x0202F000); //[25]PLL_L2H_CML_POW [17:16]PLL27X_CK108M_DIV = 0x2, divide by 4 (use when VCO=432MHz) [15:14]PLL27X_YPPADC_CKO_SEL = 0x3,YPPADC clock outpu 11:108M [13]PLL27X_YPPADC_CKO_EN[12]PLL27X_YPPADC_CK54M_EN
		rtd_maskl(PLL27X_REG_SYS_PLL_27X3_reg, 0x00000000, 0x00000000); //Power off and rst PLL27x
		rtd_maskl(PLL27X_REG_SYS_PLL_27X3_reg, 0x00000000, 0x00000017); //Power on PLL27x [4]PLL27X_PS_EN[3]PLL27X_PS_DIV2[2]PLL27X_VCORSTB[1]PLL27X_RSTB[0]PLL27X_POW
		//ScalerTimer_DelayXms(5);
		mdelay(5);
#endif
		//tve reset
		/*rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_tve_mask | SYS_REG_SYS_SRST1_write_data_mask); //[26]RSTN_TVE*/
		//tve clock enable - digital
		/*rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tve_mask | SYS_REG_SYS_CLKEN1_write_data_mask); //[26]CLKEN_TVE*/

	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

void  Check_PLL27X_Settings(void){

	pll27x_reg_sys_pll_27x3_RBUS sys_pll_27x3_reg;	/* 0_04d8 */
	pll27x_reg_sys_pll_27x4_RBUS sys_pll_27x4_reg;	/* 0_04dc */
	pll27x_reg_sys_pll_27x5_RBUS sys_pll_27x5_reg;	/* 0_0514 */
	
	sys_pll_27x3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X3_reg);
	sys_pll_27x4_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X4_reg);
	sys_pll_27x5_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X5_reg);
	
	if (sys_pll_27x3_reg.pll27x_pow != 1)
		sys_pll_27x3_reg.pll27x_pow = 1;

	if (sys_pll_27x3_reg.pll27x_rstb != 1)
		sys_pll_27x3_reg.pll27x_rstb = 1;

	if (sys_pll_27x3_reg.pll27x_vcorstb != 1)
		sys_pll_27x3_reg.pll27x_vcorstb = 1;

	/*0_0514:[1]PLL27X_PS_EN*/
	if (sys_pll_27x5_reg.pll27x_ps_en != 1){ //0_04d8 -> 0_0514
		rtd_outl(PLL27X_REG_SYS_PLL_27X5_reg,
		PLL27X_REG_SYS_PLL_27X5_pll27x_ps_en_mask |
		PLL27X_REG_SYS_PLL_27X5_write_data_mask);
	}

	if (sys_pll_27x4_reg.pll_l2h_cml_pow != 1)
		sys_pll_27x4_reg.pll_l2h_cml_pow = 1;

#if 0 // FIXME: no defined in mac8p
	/*0_0514:[4]PLL27X_YPPADC_CKO_EN*/
	if (sys_pll_27x5_reg.pll27x_yppadc_cko_en != 1) {//0_04dc -> 0_0514
		rtd_outl(PLL27X_REG_SYS_PLL_27X5_reg,
		PLL27X_REG_SYS_PLL_27X5_pll27x_yppadc_cko_en_mask |
		PLL27X_REG_SYS_PLL_27X5_write_data_mask);
	}

	if (sys_pll_27x4_reg.pll27x_yppadc_cko_sel != 3)
		sys_pll_27x4_reg.pll27x_yppadc_cko_sel = 3;
#endif

	rtd_outl(PLL27X_REG_SYS_PLL_27X3_reg, sys_pll_27x3_reg.regValue);
	rtd_outl(PLL27X_REG_SYS_PLL_27X4_reg, sys_pll_27x4_reg.regValue);
}

EXPORT_SYMBOL(Check_PLL27X_Settings);

int VDC1_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/*004DC:[25]PLL_L2H_CML_POW*/
		pll27x_reg_sys_pll_27x4_RBUS sys_pll_27x4_reg; 
		sys_pll_27x4_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X4_reg);
		sys_pll_27x4_reg.pll_l2h_cml_pow = 1;
		rtd_outl(PLL27X_REG_SYS_PLL_27X4_reg, sys_pll_27x4_reg.regValue);

#if 0  // FIXME : PLL27X_REG_SYS_PLL_27X5_pll27x_ifadc_cko_en_mask is missing on ML5
		/*0_0514:[5]PLL27X_YPPADC_CK54M_EN, [6]PLL27X_IFADC_CKO_EN*/
		rtd_outl(PLL27X_REG_SYS_PLL_27X5_reg,
		PLL27X_REG_SYS_PLL_27X5_pll27x_yppadc_cko_en_mask |
		PLL27X_REG_SYS_PLL_27X5_pll27x_ifadc_cko_en_mask |   
		PLL27X_REG_SYS_PLL_27X5_write_data_mask);
#endif

		/* 0_0104: [22]RSTN_VDEC[21]RSTN_VDEC2[15]RSTN_IFADC for vdac[11]RSTN_APLL_ADC */
		/* 0_0108: [27]RSTN_ATVIN2[26]RSTN_ATVIN */
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_vdec_mask |
			SYS_REG_SYS_SRST1_rstn_vdec2_mask |
			SYS_REG_SYS_SRST1_rstn_ifadc_mask |
			SYS_REG_SYS_SRST1_rstn_apll_adc_mask |
			SYS_REG_SYS_SRST1_write_data_mask);
		rtd_outl(SYS_REG_SYS_SRST2_reg,
			SYS_REG_SYS_SRST2_rstn_atvin_mask |
			SYS_REG_SYS_SRST2_rstn_atvin2_mask |
			SYS_REG_SYS_SRST2_write_data_mask);

		/* 0_0114: [22]CLKEN_VDEC[15]CLKEN_IFADC for vdac[11]CLKEN_APLL_ADC */
		/* 0_0118: [27]CLKEN_ATVIN_VD */
		rtd_outl(SYS_REG_SYS_CLKEN1_reg,
			SYS_REG_SYS_CLKEN1_clken_vdec_mask |
			SYS_REG_SYS_CLKEN1_clken_ifadc_mask |
			SYS_REG_SYS_CLKEN1_clken_apll_adc_mask |
			SYS_REG_SYS_CLKEN1_write_data_mask);
		rtd_outl(SYS_REG_SYS_CLKEN2_reg,
			SYS_REG_SYS_CLKEN2_clken_atvin_vd_mask |
			SYS_REG_SYS_CLKEN2_write_data_mask);
#if 0
		/*vd reset*/
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_vdec_mask |
			SYS_REG_SYS_SRST1_rstn_vdec2_mask);
		mdelay(5);
		rtd_outl(SYS_REG_SYS_SRST1_reg,
			SYS_REG_SYS_SRST1_rstn_vdec_mask |
			SYS_REG_SYS_SRST1_rstn_vdec2_mask |
			SYS_REG_SYS_SRST1_write_data_mask);
#endif
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int VDC2_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_vdec_mask);
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int VDC3_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	pll27x_reg_sys_pll_27x3_RBUS sys_pll_27x3_reg;	/* 0_04d8 */
	pll27x_reg_sys_pll_27x4_RBUS sys_pll_27x4_reg;	/* 0_04dc */

	unsigned char ucRet = true;

	//hw_semaphore_try_lock();
	if (clken == CLK_ON) {

#if 0
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKSEL_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKSEL_reg));

		rtd_pr_crt_emerg( "SYS_REG_SYS_SRST1_reg=%x\n", rtd_inl(SYS_REG_SYS_SRST1_reg));
		rtd_pr_crt_emerg( "0xb8000108=%x\n", rtd_inl(0xb8000108));
		rtd_pr_crt_emerg( "0xb800010c=%x\n", rtd_inl(0xb800010c));
		rtd_pr_crt_emerg( "0xb8000110=%x\n", rtd_inl(0xb8000110));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN1_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN1_reg));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN2_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN2_reg));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN3_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN3_reg));

		rtd_pr_crt_emerg( "PLL27X_REG_SYS_PLL_27X2_reg=%x\n", rtd_inl(PLL27X_REG_SYS_PLL_27X2_reg));
		rtd_pr_crt_emerg( "0xb80004d8=%x\n", rtd_inl(0xb80004d8));
#endif

		sys_pll_27x3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X3_reg);
		sys_pll_27x4_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X4_reg);

		if (sys_pll_27x3_reg.pll27x_pow == 1 &&
			sys_pll_27x3_reg.pll27x_rstb == 1 &&
			sys_pll_27x3_reg.pll27x_vcorstb == 1 &&
			/* sys_pll_27x5_reg.pll27x_ps_en == 1 && */ //0_04d8 -> 0_0514
			/* sys_pll_27x5_reg.pll27x_yppadc_cko_en == 1 && */ //0_04dc -> 0_0514
			/* sys_pll_27x4_reg.pll27x_yppadc_cko_sel == 3 && */
			sys_pll_27x4_reg.pll_l2h_cml_pow == 1) {

			ucRet = true;

			//hw_semaphore_unlock();
			return CRT_SUCCESS;
		} else {
			ucRet = false;

		}
	} else {

	}

	*(int *)private_data = ucRet;

	//hw_semaphore_unlock();

	return CRT_SUCCESS;
}
int VDC4_CriticalRegister(enum CRT_CLK clken, void *private_data)
{

#if 0//Pll27x setting in bootcode
	pll27x_reg_sys_pll_27x1_RBUS sys_pll_27x1_reg;	/* 0_04d0 */
	pll27x_reg_sys_pll_27x2_RBUS sys_pll_27x2_reg;	/* 0_04d4 */
	pll27x_reg_sys_pll_27x3_RBUS sys_pll_27x3_reg;	/* 0_04d8 */
	pll27x_reg_sys_pll_27x4_RBUS sys_pll_27x4_reg;	/* 0_04dc */

	int freq = *(int *)private_data;

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		sys_pll_27x1_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X1_reg);
		sys_pll_27x2_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X2_reg);
		sys_pll_27x3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X3_reg);
		sys_pll_27x4_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X4_reg);

		switch (freq) {
		case VDC_PLL27x_CLK_540M:
			sys_pll_27x1_reg.pll27x_m = 0x12;
			sys_pll_27x2_reg.pll27x_ps_54m_delay = 0x3;
			break;
		default:
		case VDC_PLL27x_CLK_432M:
			sys_pll_27x1_reg.pll27x_m = 0x0e;
			sys_pll_27x2_reg.pll27x_ps_54m_delay = 0x2;
			break;
		case VDC_PLL27x_CLK_324M:
			sys_pll_27x1_reg.pll27x_m = 0x0a;
			sys_pll_27x2_reg.pll27x_ps_54m_delay = 0x0;
			break;
		case VDC_PLL27x_CLK_648M:
			sys_pll_27x1_reg.pll27x_m = 0x16;
			sys_pll_27x2_reg.pll27x_ps_54m_delay = 0x1;
			break;
		}
		sys_pll_27x1_reg.pll27x_n = 0;
		sys_pll_27x5_reg.pll27x_yppadc_cko_en = 1;//0_04dc -> 0_0514
		sys_pll_27x4_reg.pll27x_yppadc_cko_sel = 3; /* 108M */
		sys_pll_27x4_reg.pll27x_ck108m_div = freq;
		sys_pll_27x4_reg.pll_l2h_cml_pow = 1;
		rtd_outl(PLL27X_REG_SYS_PLL_27X1_reg, sys_pll_27x1_reg.regValue);
		rtd_outl(PLL27X_REG_SYS_PLL_27X2_reg, sys_pll_27x2_reg.regValue);
		rtd_outl(PLL27X_REG_SYS_PLL_27X4_reg, sys_pll_27x4_reg.regValue);
#if 0 //frank@0813 mark below code avoid VDC register Rbustimeout
		/* 0_04d8: [4]PLL27X_PS_EN[2]PLL27X_VCORSTB[1]PLL27X_RSTB[0]PLL27X_POW
			=> set 0, reset first */
		sys_pll_27x3_reg.pll27x_pow = 0;
		sys_pll_27x3_reg.pll27x_rstb = 0;
		sys_pll_27x3_reg.pll27x_vcorstb = 0;
		sys_pll_27x5_reg.pll27x_ps_en = 0; //0_04d8 -> 0_0514
		rtdf_outl(PLL27X_REG_SYS_PLL_27X3_reg, sys_pll_27x3_reg.regValue);
#endif
		/* power on pll 27x */
		sys_pll_27x3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X3_reg);
		/* 0_04d8: [4]PLL27X_PS_EN[2]PLL27X_VCORSTB[1]PLL27X_RSTB[0]PLL27X_POW
			=> set 1, Power on! */
		sys_pll_27x3_reg.pll27x_pow = 1;
		sys_pll_27x3_reg.pll27x_rstb = 1;
		sys_pll_27x3_reg.pll27x_vcorstb = 1;
		sys_pll_27x5_reg.pll27x_ps_en = 1; //0_04d8 -> 0_0514
		rtd_outl(PLL27X_REG_SYS_PLL_27X3_reg, sys_pll_27x3_reg.regValue);

	} else {

	}
	hw_semaphore_unlock();
#endif
	return CRT_SUCCESS;
}
int VDC5_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	unsigned int regValue;

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X1_reg);
		*((unsigned int *)private_data) = regValue;
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)

int VDC6_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	//pll27x_reg_sys_pll_27x3_RBUS sys_pll_27x3_reg;	/* 0_04d8 */
	vdtop_misc_control_RBUS misc_control_reg;	/* 9764 */
	unsigned int clkReg = *(unsigned int *)private_data;

	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		//sys_pll_27x3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_27X3_reg);
		//sys_pll_27x3_reg.pll27x_ps_div2 = 0;

		misc_control_reg.regValue = rtd_inl(VDTOP_MISC_CONTROL_reg);
		switch (clkReg) {
		case VDC_PLL27x_CLK_648M:
			//sys_pll_27x3_reg.pll27x_ps_div2 = 1;
			misc_control_reg.swallow_freq_sel = 2;
			break;
		default:
		case VDC_PLL27x_CLK_540M:
			misc_control_reg.swallow_freq_sel = 1;
			break;
		case VDC_PLL27x_CLK_432M:
			misc_control_reg.swallow_freq_sel = 0;
			break;
		case VDC_PLL27x_CLK_324M:
			misc_control_reg.swallow_freq_sel = 2;
			break;
		}
		rtd_outl(VDTOP_MISC_CONTROL_reg, misc_control_reg.regValue);
		//rtd_outl(PLL27X_REG_SYS_PLL_27X3_reg, sys_pll_27x3_reg.regValue);
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}
int VDC7_CriticalRegister(enum CRT_CLK clken)
{
	/* b800_0104[21] */
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/* sw reset for video decoder */
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec2_mask);
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}
int VDC8_CriticalRegister(enum CRT_CLK clken)
{
	/* b800_0104[21] */
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec2_mask | SYS_REG_SYS_SRST1_write_data_mask);
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}


int VDC9_CriticalRegister(enum CRT_CLK clken, void *private_data)
{
	sys_reg_sys_srst1_RBUS sys_srst1_reg;		/* 0_0104 */
	unsigned char ucRet = true;

	//hw_semaphore_try_lock();
	if (clken == CLK_ON) {

		sys_srst1_reg.regValue = rtd_inl(SYS_REG_SYS_SRST1_reg);

#if 0
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKSEL_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKSEL_reg));

		rtd_pr_crt_emerg( "SYS_REG_SYS_SRST1_reg=%x\n", rtd_inl(SYS_REG_SYS_SRST1_reg));
		rtd_pr_crt_emerg( "0xb8000108=%x\n", rtd_inl(0xb8000108));
		rtd_pr_crt_emerg( "0xb800010c=%x\n", rtd_inl(0xb800010c));
		rtd_pr_crt_emerg( "0xb8000110=%x\n", rtd_inl(0xb8000110));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN1_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN1_reg));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN2_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN2_reg));
		rtd_pr_crt_emerg( "SYS_REG_SYS_CLKEN3_reg=%x\n", rtd_inl(SYS_REG_SYS_CLKEN3_reg));

		rtd_pr_crt_emerg( "PLL27X_REG_SYS_PLL_27X2_reg=%x\n", rtd_inl(PLL27X_REG_SYS_PLL_27X2_reg));
		rtd_pr_crt_emerg( "0xb80004d8=%x\n", rtd_inl(0xb80004d8));
#endif


		/* 0_0104: [22]RSTN_VDEC[21]RSTN_VDEC2[15]RSTN_IFADC for vdac[11]RSTN_APLL_ADC */
		if (sys_srst1_reg.rstn_vdec == 1 &&
			sys_srst1_reg.rstn_vdec2 == 1 &&
			sys_srst1_reg.rstn_ifadc == 1 &&
			sys_srst1_reg.rstn_apll_adc == 1) {

			ucRet = true;

			//hw_semaphore_unlock();
			return CRT_SUCCESS;
		}
		// VD clock is not setted, set it.
		else {
			ucRet = false;
		}
	} else {

	}

	*(int *)private_data = ucRet;

	//hw_semaphore_unlock();

	return CRT_SUCCESS;
}

int VDC10_CriticalRegister(enum CRT_CLK clken)
{	/* softreset for video decoder */
//	int flag_pll27x3;
	unsigned long flag_avdin;

	/* b800_0104[21] */

	// Set AVD_SW_Reset_used = 1
//	RTD_SWSEM_27x3_LOCK(flag_pll27x3);  //to gurantee PLL27x3 clock source.
	RTD_SWSEM_AVDIN_LOCK(flag_avdin); //to avoid downstream access AVDIN clock domain.

	hw_semaphore_try_lock();

	// Disable reset bit
	rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec2_mask);
	mdelay(1);
	// Enable Reset bit
	rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_vdec2_mask | SYS_REG_SYS_SRST1_write_data_mask);

	hw_semaphore_unlock();

	//Clear AVD_SW_Reset_used = 0
	RTD_SWSEM_AVDIN_UNLOCK(flag_avdin); //to allow downstream access AVDIN clock domain.
//	RTD_SWSEM_27x3_UNLOCK(flag_pll27x3);  //free for PLL27x3 clock source operation.

	return CRT_SUCCESS;
}


int VDCInput2_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/* 0_0514[5] set 1 to enable 54M */ //0_04dc -> 0_0514
	#if 0//merlin6 spec removed
		rtd_outl(PLL27X_REG_SYS_PLL_27X5_reg,
		PLL27X_REG_SYS_PLL_27X5_pll27x_yppadc_ck54m_en_mask |
		PLL27X_REG_SYS_PLL_27X5_write_data_mask);
	#endif
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}
int VDCInput3_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		/* 0_0514[5] set 0 to enable 54M */ //0_04dc -> 0_0514
	#if 0//merlin6 spec removed
		rtd_outl(PLL27X_REG_SYS_PLL_27X5_reg,
		PLL27X_REG_SYS_PLL_27X5_pll27x_yppadc_ck54m_en_mask); 
	#endif
	} else {

	}
	hw_semaphore_unlock();

	return CRT_SUCCESS;
}

#endif

int Thermal_CriticalRegister(enum CRT_CLK clken)
{
	hw_semaphore_try_lock();
	if (clken == CLK_ON) {
		rtd_outl(SYS_REG_SYS_SRST2_reg, SYS_REG_SYS_SRST2_rstn_tm_mask | SYS_REG_SYS_SRST2_write_data_mask);//release thermal rst
		rtd_outl(SYS_REG_SYS_CLKEN2_reg, SYS_REG_SYS_CLKEN2_clken_tm_mask | SYS_REG_SYS_CLKEN2_write_data_mask);//enable thermal clk
	} else {
	}
	hw_semaphore_unlock();
	return CRT_SUCCESS;

}

void CRT_CLK_OnOff(enum CRT_MODULE module_ip, enum CRT_CLK clken, void *private_data)
{
	CRT_CLK_ModuleName(module_ip);

	switch (module_ip) {
#if IS_ENABLED(CONFIG_RTK_KDRV_USB)
	case USB:
		if (USB_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("USB CRT setting fail\n");
		}
		break;
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
	case IrDA:
		if (IrDA_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("IrDA CRT setting fail\n");
		}
		break;
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_I2C)
	case I2C:
		if (I2C_CriticalRegister(clken , private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("I2C CRT setting fail\n");
		}
		break;
#endif
	case UART:
		if (UART_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("UART CRT setting fail\n");
		}
		break;

	case SE:
		if (SE_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("SE CRT setting fail\n");
		}
		break;

	case NAND:
		if (NAND_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("NAND CRT setting fail\n");
		}
		break;

	case MD:
		if (MD_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("MD CRT setting fail\n");
		}
		break;
	case PCMCIA:
		if (PCMCIA_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("PCMCIA CRT setting fail\n");
		}
		break;
	case CEC:
		if (CEC_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("CEC CRT setting fail\n");
		}
		break;
	case IRTX:
            if (IRTX_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("IRTX CRT setting fail\n");
		}
		break;
        case LSADC:
            if (LSADC_CriticalRegister(clken) == CRT_FAIL) {
                rtd_pr_crt_warn("LSADC CRT setting fail\n");
            }
            break;
			
	case DISEQC:
		if (DiseqC_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DiseqC CRT setting fail\n");
		}
		break;
    case WOV:
        if (WOV_CriticalRegister(clken) == CRT_FAIL) {
            rtd_pr_crt_warn("WOV CRT setting fail\n");
        }
        break;
	case GPIO:
		if (GPIO_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("GPIO CRT setting fail\n");
		}
		break;

	case PWM:
		if (PWM_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("PWM CRT setting fail\n");
		}
		break;

	case EMMC:
		if (EMMC_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("EMMC CRT setting fail\n");
		}
		break;

	case SDIO:
		if (SDIO_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("SDIO CRT setting fail\n");
		}
		break;

	case SMC:
		if (SMC_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("SMC CRT setting fail\n");
		}
		break;
	case SMC_MIO:
		if (SMC_MIO_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("SMC_MIO CRT setting fail\n");
		}
		break;

	case CP:
		if (CP_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("CP CRT setting fail\n");
		}
		break;

	case TP:
		if (TP_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TP CRT setting fail\n");
		}
		break;

        case TPOUT:
                if (TPOUT_CriticalRegister(clken) == CRT_FAIL) {
                        rtd_pr_crt_warn("TPOUT CRT setting fail\n");
                }
                break;

	case MHL:
		if (MHL_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("MHL CRT setting fail\n");
		}
		break;

	case HDMI:
		if (HDMI_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("HDMI CRT setting fail\n");
		}
		break;

	case GDMA:
		if (GDMA_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("GDMA CRT setting fail\n");
		}
		break;
	case VBI:
		if (VBI_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VBI CRT setting fail\n");
		}
		break;
	case ETH:
		if (ETH_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("ETH CRT setting fail\n");
		}
		break;
	case TVALL:
		if (TV_All_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVALL CRT setting fail\n");
		}
		break;
	case TVADC:
		if (TV_Adc_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVADC CRT setting fail\n");
		}
		break;
	case TVFRONTEND:
		if (TV_Frontend_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVFRONTEND CRT setting fail\n");
		}
		break;
	case DTVDEMOD:
		if (DTV_Demod_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVDEMOD CRT setting fail\n");
		}
		break;
       	case DTVExtAtsc3:
		if (DTV_ExtAtsc3Demod_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVExtAtsc3 CRT setting fail\n");
		}
		break;

	case DTVATB:
		if (DTV_Atb_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVATB CRT setting fail\n");
		}
		break;
	case DTVHDIC:
		if (DTV_Hdic_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVHDIC CRT setting fail\n");
		}
		break;
	case DTVRTK:
		if (DTV_Rtk_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVRTK CRT setting fail\n");
		}
		break;
	case TVALLISDBT:
		if (TV_AllISDBT_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVALLISDBT CRT setting fail\n");
		}
		break;
	case DTVATBwoWrap:
		if (DTV_AtbWoWrap_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("DTVATBwoWrap CRT setting fail\n");
		}
		break;
	case ATVDEMOD:
		if (ATV_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("ATVDEMOD CRT setting fail\n");
		}
		break;
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
	case SCALER_ADC_CLK_1:
		if (ADC_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("ADC CRT setting fail\n");
		}
		break;

	case SCALER_VFEDEV_1:
		if (VFEDEV_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VFEDEV CRT setting fail\n");
		}
		break;
	
	case SCALER_AUTO_CLK_1:
		if (Auto_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("Auto 1 CRT setting fail\n");
		}
		break;
		
	case SCALER_AUTO_CLK_2:
		if (Auto2_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("Auto 2 CRT setting fail\n");
		}
		break;

	case SCALER_VO_1:
		if (VO_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VO CRT setting fail\n");
		}
		break;
	case SCALER_TVE_1:
		if (TVE1_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVE 1 CRT setting fail\n");
		}
		break;
	case SCALER_TVE_2:
		if (TVE2_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVE 2 CRT setting fail\n");
		}
		break;
	case SCALER_TVE_3:
		if (TVE3_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVE 3 CRT setting fail\n");
		}
		break;
	case SCALER_TVE_4:
		if (TVE4_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVE 4 CRT setting fail\n");
		}
		break;
	case SCALER_TVE_5:
		if (TVE5_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("TVE 5 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_1:
		if (VDC1_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 1 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_2:
		if (VDC2_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 2 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_3:
		if (VDC3_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 3 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_4:
		if (VDC4_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 4 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_5:
		if (VDC5_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 5 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_6:
		if (VDC6_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 6 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_7:
		if (VDC7_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 7 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_8:
		if (VDC8_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 8 CRT setting fail\n");
		}
		break;

	case SCALER_VDC_9:
		if (VDC9_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 9 CRT setting fail\n");
		}
		break;

	case SCALER_VDC_10:
		if (VDC10_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC 10 CRT setting fail\n");
		}
		break;

	case SCALER_VDC_INPUT_2:
		if (VDCInput2_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC_INPUT 2 CRT setting fail\n");
		}
		break;
	case SCALER_VDC_INPUT_3:
		if (VDCInput3_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("VDC INPUT 3 CRT setting fail\n");
		}
		break;
#endif
	case GPU:
		if (GPU_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("GPU CRT setting fail\n");
		}
		break;
	case DOLBY_HDR:
		if (DOLBY_HDR_CriticalRegister(clken, private_data) == CRT_FAIL) {
			rtd_pr_crt_warn("GPU CRT setting fail\n");
		}
		break;		
	case THERMAL_SENSOR:
		if (Thermal_CriticalRegister(clken) == CRT_FAIL) {
			rtd_pr_crt_warn("Thermal CRT setting fail\n");
		}
		break;
	default:
		/* not in module list call BUS() to stop system */
		BUG();
		break;
	}

}
EXPORT_SYMBOL(CRT_CLK_OnOff);

char * CRT_CLK_ModuleName(enum CRT_MODULE module_ip)
{

	/*coverity CID 15369, strings shoud have null terminated*/
        snprintf(lastModuleName, sizeof(lastModuleName), "%s", moduleName);
	memset(moduleName, 0, sizeof(moduleName));
	switch (module_ip) {

	case USB:
		snprintf(moduleName, sizeof(moduleName), "%s", "USB");
		break;

	case IrDA:
		snprintf(moduleName, sizeof(moduleName), "%s", "IrDA");
		break;

	case I2C:
		snprintf(moduleName, sizeof(moduleName), "%s", "I2C");
		break;

	case UART:
		snprintf(moduleName, sizeof(moduleName), "%s", "UART");
		break;

	case SE:
		snprintf(moduleName, sizeof(moduleName), "%s", "SE");
		break;

	case NAND:
		snprintf(moduleName, sizeof(moduleName), "%s", "NAND");
		break;

	case MD:
		snprintf(moduleName, sizeof(moduleName), "%s", "MD");
		break;

	case PCMCIA:
		snprintf(moduleName, sizeof(moduleName), "%s", "PCMCIA");
		break;

	case GPIO:
		snprintf(moduleName, sizeof(moduleName), "%s", "GPIO");
		break;
		
    case CEC:
            snprintf(moduleName, sizeof(moduleName), "%s", "CEC");
            break;  
            
	case IRTX:
            snprintf(moduleName, sizeof(moduleName), "%s", "IRTX");
            break;  
            
        case LSADC:
            snprintf(moduleName, sizeof(moduleName), "%s", "LSADC");
            break;  

	case PWM:
		snprintf(moduleName, sizeof(moduleName), "%s", "PWM");
		break;

	case EMMC:
		snprintf(moduleName, sizeof(moduleName), "%s", "EMMC");
		break;

	case SDIO:
		snprintf(moduleName, sizeof(moduleName), "%s", "SDIO");
		break;

	case SMC:
		snprintf(moduleName, sizeof(moduleName), "%s", "SMC");
		break;

	case CP:
		snprintf(moduleName, sizeof(moduleName), "%s", "CP");
		break;

	case TP:
		snprintf(moduleName, sizeof(moduleName), "%s", "TP");
		break;

	case GDMA:
		snprintf(moduleName, sizeof(moduleName), "%s", "GDMA");
		break;

	case VBI:
		snprintf(moduleName, sizeof(moduleName), "%s", "VBI");
		break;

	case ETH:
		snprintf(moduleName, sizeof(moduleName), "%s", "ETH");
		break;

	case TVALL:
		snprintf(moduleName, sizeof(moduleName), "%s", "TVALL");
		break;
	case TVADC:
		snprintf(moduleName, sizeof(moduleName), "%s", "TVADC");
		break;
	case TVFRONTEND:
		snprintf(moduleName, sizeof(moduleName), "%s", "TVFRONTEND");
		break;
	case DTVDEMOD:
		snprintf(moduleName, sizeof(moduleName), "%s", "DTVDEMOD");
		break;
       case DTVExtAtsc3:
		snprintf(moduleName, sizeof(moduleName), "%s", "DTVExtAtsc3");
		break;        
	case DTVATB:
		snprintf(moduleName, sizeof(moduleName), "%s", "DTVATB");
		break;
	case DTVHDIC:
		snprintf(moduleName, sizeof(moduleName), "%s", "DTVHDIC");
		break;
	case DTVRTK:
		snprintf(moduleName, sizeof(moduleName), "%s", "DTVRTK");
		break;
	case ATVDEMOD:
		snprintf(moduleName, sizeof(moduleName), "%s", "ATVDEMOD");
		break;

	//====scaler=========
	case SCALER_ADC_CLK_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_ADC_CLK_1");
		break;


	case SCALER_AUTO_CLK_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_AUTO_CLK_1");
		break;


	case SCALER_AUTO_CLK_2:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_AUTO_CLK_2");
		break;

	case SCALER_VFEDEV_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VFEDEV_1");
		break;


	case SCALER_TVE_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_TVE_1");
		break;

	case SCALER_TVE_2:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_TVE_2");
                break;
	case SCALER_TVE_3:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_TVE_3");
		break;

	case SCALER_TVE_4:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_TVE_4");
		break;

	case SCALER_TVE_5:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_TVE_5");
		break;

	case SCALER_VDC_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_1");
		break;

	case SCALER_VDC_2:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_2");
		break;

	case SCALER_VDC_3:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_3");
		break;

	case SCALER_VDC_4:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_4");
		break;

	case SCALER_VDC_5:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_5");
		break;

	case SCALER_VDC_6:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_6");
		break;

	case SCALER_VDC_7:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_7");
		break;

	case SCALER_VDC_8:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_8");
		break;

	case SCALER_VDC_9:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_9");
		break;

	case SCALER_VDC_10:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_10");
		break;

	case SCALER_VDC_INPUT_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_INPUT_1");
		break;

	case SCALER_VDC_INPUT_2:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_INPUT_2");
		break;


	case SCALER_VDC_INPUT_3:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VDC_INPUT_3");
		break;

	case SCALER_VO_1:
		snprintf(moduleName, sizeof(moduleName), "%s", "SCALER_VO_1");
		break;

	case THERMAL_SENSOR:
		snprintf(moduleName, sizeof(moduleName), "%s", "Thermal");
		break;


	default:
		/* not in module list call BUS() to stop system */
		snprintf(moduleName,sizeof(moduleName),"HEX: %x",module_ip);
		break;
	}

	return moduleName;
}

#ifndef BUILD_QUICK_SHOW
/* reg info : bits-width, regName */
typedef struct reg_info {
	int bit_width;
	char regName[32];
} regInfo;

regInfo crt_sys_srst_0_reg[] = {
	{1, "RSTN_Ib_sb1"},
	{1, "RSTN_Ib_sb2"},
	{1, "RSTN_Ib_sb3"},
	{1, "RSTN_Ib_ve"},
	{1, "RSTN_Ib_ve2"},
	{1, "RSTN_Ib_vp9"},
	{1, "RSTN_Ib_tvsb1"},
	{1, "RSTN_ND"},
	{1, "RSTN_AKL"},
	{1, "RSTN_DSC"},
	{1, "RSTN_SCE"},
	{1, "RSTN_KT"},
	{1, "RSTN_Ib_gpu"},
	{1, "RSTN_KCPU"},
	{1, "RSTN_VDE2"},
	{1, "RSTN_VE2_P2"},
	{1, "RSTN_VE2"},
	{1, "RSTN_VCPU2"},
	{1, "RSTN_VP9"},
	{1, "RSTN_ACPU"},
	{1, "RSTN_Ib_tvsb2"},
	{1, "RSTN_Ib_tvsb3"},
	{1, "RSTN_Ib_tvsb4"},
	{1, "RSTN_Ib_tvsb5"},
	{1, "RSTN_SCPU_WRAP"},
	{1, "RSTN_VDE"},
	{1, "RSTN_VE_P2"},
	{1, "RSTN_VE"},
	{1, "RSTN_VCPU"},
	{1, "RSTN_AE"},
	{1, "RSTN_SCPU"},
	{1, "RSTN_PLLREG"},
};

regInfo crt_sys_srst_1_reg[] = {
	{1, "RSTN_TVSB1"},
	{1, "RSTN_TVSB2"},
	{1, "RSTN_TVSB3"},
	{1, "RSTN_TVSB4"},
	{1, "RSTN_TVSB5"},
	{1, "RSTN_TVE"},
	{1, "RSTN_DCPHY"},
	{1, "RSTN_DCU"},
	{1, "RSTN_IF_DEMOD"},
	{1, "RSTN_VDEC"},
	{1, "RSTN_VDEC2"},
	{1, "RSTN_VBIS0"},
	{1, "RSTN_AUDIO2_DAC"},
	{1, "RSTN_AUDIO_ADC"},
	{1, "RSTN_AUDIO_DAC"},
	{1, "RSTN_AUDIO"},
	{1, "RSTN_IFADC"},
	{1, "RSTN_CBUS"},
	{1, "RSTN_OFFMS"},
	{1, "RSTN_ME"},
	{1, "RSTN_APLL_ADC"},
	{1, "RSTN_HDMI"},
	{1, "RSTN_HDMITX"},
	{1, "RSTN_TP"},
	{1, "RSTN_CP"},
	{1, "RSTN_MD"},
	{1, "RSTN_SE"},
	{1, "Reserved"},
	{1, "RSTN_3D_GDE"},
	{1, "RSTN_VODMA"},
	{1, "RSTN_DISPIM"},
	{1, "RSTN_DISP"},
};

regInfo crt_sys_srst_2_reg[] = {
	{2, "Reserved"},
	{1, "Reserved"},
	{1, "RSTN_TM"},
	{1, "RSTN_ATVIN2"},
	{1, "RSTN_ATVIN"},
	{2, "Dummy"},
	{1, "RSTN_EMMC"},
	{1, "Reserved"},
	{1, "RSTN_BISTREG"},
	{2, "Dummy"},
	{1, "RSTN_USB"},
	{1, "RSTN_OTG"},
	{1, "Reserved"},
	{2, "Dummy"},
	{1, "RSTN_IRTX"},
	{1, "RSTN_PADMUX"},
	{1, "RSTN_DTV_DEMOD"},
	{1, "RSTN_MISC"},
	{1, "RSTN_PWM"},
	{1, "RSTN_UART1"},
	{1, "RSTN_UART2"},
	{1, "RSTN_SC1"},
	{1, "RSTN_GPIO"},
	{1, "RSTN_I2C1"},
	{1, "RSTN_I2C2"},
	{1, "RSTN_NF"},
	{1, "RSTN_SC"},
	{1, "RSTN_PC"},
};

regInfo crt_sys_srst_3_reg[] = {
	{6, "Reserved"},
	{1, "RSTN_DCPHY2"},
	{1, "RSTN_DCU2"},
	{6, "Reserved"},
	{1, "RSTN_DDC"},
	{1, "RSTN_SD"},
	{3, "Reserved"},
	{1, "RSTN_HDMITX_PLL"},
	{4, "Reserved"},
	{1, "RSTN_USB2_PHY_P3"},
	{1, "RSTN_USB2_PHY_P2"},
	{1, "RSTN_USB2_PHY_P1"},
	{1, "RSTN_USB2_PHY_P0"},
	{2, "Reserved"},
	{1, "RSTN_USB3_MDIO_P0"},
	{1, "RSTN_USB3_PHY_P0"},
};

regInfo crt_sys_clken_0_reg[] = {
	{2, "Reserved"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "CLKEN_AKL"},
	{1, "CLKEN_DSC"},
	{1, "CLKEN_SCE"},
	{1, "CLKEN_KT"},
	{1, "Reserved"},
	{1, "CLKEN_KCPU"},
	{1, "Reserved"},
	{1, "CLKEN_VE2_P2"},
	{1, "CLKEN_VE2"},
	{1, "CLKEN_VCPU2"},
	{1, "CLKEN_VP9"},
	{1, "CLKEN_ACPU"},
	{1, "Reserved"},
	{1, "Reserved"},
	{3, "Reserved"},
	{1, "Reserved"},
	{1, "CLKEN_VE_P2"},
	{1, "CLKEN_VE"},
	{1, "CLKEN_VCPU"},
	{1, "CLKEN_SCPU_WRAP"},
	{1, "CLKEN_SCPU"},
	{1, "CLKEN_PLLREG"},
};

regInfo crt_sys_clken_1_reg[] = {
	{1, "CLKEN_TVSB1"},
	{1, "CLKEN_TVSB2"},
	{1, "CLKEN_TVSB3"},
	{1, "CLKEN_TVSB4"},
	{1, "CLKEN_TVSB5"},
	{1, "CLKEN_TVE"},
	{1, "CLKEN_DCPHY"},
	{1, "CLKEN_DCU"},
	{1, "CLKEN_IF_DEMOD"},
	{1, "CLKEN_VDEC"},
	{1, "CLKEN_SE2"},
	{1, "CLKEN_VBIS0"},
	{2, "Dummy"},
	{1, "CLKEN_AUDIO_PRE512FS"},
	{1, "CLKEN_AUDIO"},
	{1, "CLKEN_IFADC"},
	{1, "CLKEN_CBUS"},
	{1, "CLKEN_OFFMS"},
	{1, "CLKEN_ME"},
	{1, "CLKEN_APLL_ADC"},
	{1, "CLKEN_HDMI"},
	{1, "CLKEN_HDMITX"},
	{1, "CLKEN_TP"},
	{1, "CLKEN_CP"},
	{1, "CLKEN_MD"},
	{1, "CLKEN_SE"},
	{1, "CLKEN_TP_DIV"},
	{1, "CLKEN_3D_GDE"},
	{1, "CLKEN_VODMA"},
	{1, "CLKEN_DISPIM"},
	{1, "CLKEN_DISP"},
};

regInfo crt_sys_clken_2_reg[] = {
	{3, "Reserved"},
	{1, "CLKEN_TM"},
	{1, "CLKEN_ATVIN_VD"},
	{1, "CLKEN_ATVIN_IFD"},
	{2, "Dummy"},
	{1, "CLKEN_EMMC"},
	{1, "CLKEN_EFUSE"},
	{1, "CLKEN_BISTREG"},
	{1, "Reserved"},
	{1, "Reserved"},
	{1, "CLKEN_USB"},
	{1, "CLKEN_OTG"},
	{1, "Reserved"},
	{1, "CLKEN_AUDIO2_512FS"},
	{1, "Dummy"},
	{1, "CLKEN_IRTX"},
	{1, "CLKEN_PADMUX"},
	{1, "CLKEN_DTV_DEMOD"},
	{1, "CLKEN_MISC"},
	{1, "CLKEN_PWM"},
	{1, "CLKEN_UART1"},
	{1, "CLKEN_UART2"},
	{1, "CLKEN_SC1"},
	{1, "CLKEN_GPIO"},
	{1, "CLKEN_I2C1"},
	{1, "CLKEN_I2C2"},
	{1, "CLKEN_NF"},
	{1, "CLKEN_SC"},
	{1, "CLKEN_PC"},
};

regInfo crt_sys_clken_3_reg[] = {
	{6, "Reserved"},
	{1, "CLKEN_DCPHY2"},
	{1, "CLKEN_DCU2"},
	{6, "Reserved"},
	{1, "CLKEN_DDC"},
	{1, "CLKEN_SD"},
	{8, "Reserved"},
	{1, "Clken_audio2_dma_r_512fs"},
	{1, "Clken_audio_dma_r_512fs"},
	{1, "Clken_audio_da1_256fs"},
	{1, "Clken_audio_da1_128fs"},
	{1, "Clken_audio_da0_256fs"},
	{1, "Clken_audio_da0_128fs"},
	{1, "Clken_audio_ad_256fs"},
	{1, "Clken_audio_ad_128fs"},
};

#define SYS_REG_SYS_SRST_0_SIZE 	(sizeof(crt_sys_srst_0_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_SRST_1_SIZE 	(sizeof(crt_sys_srst_1_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_SRST_2_SIZE 	(sizeof(crt_sys_srst_2_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_SRST_3_SIZE 	(sizeof(crt_sys_srst_3_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_CLKEN_0_SIZE 	(sizeof(crt_sys_clken_0_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_CLKEN_1_SIZE 	(sizeof(crt_sys_clken_1_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_CLKEN_2_SIZE 	(sizeof(crt_sys_clken_2_reg) / sizeof(struct reg_info))
#define SYS_REG_SYS_CLKEN_3_SIZE 	(sizeof(crt_sys_clken_3_reg) / sizeof(struct reg_info))

unsigned int getBitMask(unsigned int bit, unsigned int width)
{
	unsigned int retMask = 0;
	int i;

	for (i = bit; i <= (bit + width - 1) ; i++) {
		retMask += (1 << i);
	}

	rtd_pr_crt_debug("bit %02d, width = %d,  Mask = 0x%08X \n", bit, width,  retMask);
	return retMask;
}

#if 0
static ssize_t crt_reg_show(unsigned regAddr, regInfo *regData, unsigned int regSize, char *buf)
{
	char regInfoString[1024];
	char *pregInfoString = regInfoString;
	int regInfoStringCount, index, bitCount = 32;
	unsigned int regVal, bitmask, lowbit;

	hw_semaphore_try_lock();
	regVal = rtd_inl(regAddr);
	hw_semaphore_unlock();
	regInfoStringCount = sprintf(pregInfoString, "===== (0x%08X) = (0x%08X) ===== \n", regAddr, regVal);
	pregInfoString += regInfoStringCount;

	for (index = 0; index < regSize; index++) {
		if ((regData + index)->bit_width > 1) {
			lowbit = bitCount - (regData + index)->bit_width;
			bitmask = getBitMask(lowbit, (regData + index)->bit_width);
			regInfoStringCount = sprintf(pregInfoString, "%s\t[%d:%d] = 0x%X\n", (regData + index)->regName , --bitCount, lowbit, ((regVal & bitmask) >> lowbit));
			pregInfoString += regInfoStringCount;
			bitCount = lowbit;
		} else {
			bitCount--;
			bitmask = getBitMask(bitCount, (regData + index)->bit_width);
			regInfoStringCount = sprintf(pregInfoString, "%s\t[%d] = 0x%X\n", (regData + index)->regName, bitCount, ((regVal & bitmask) >> bitCount));
			pregInfoString += regInfoStringCount;
		}
	}

	return sprintf(buf, "%s\n", regInfoString);
}
#endif

static ssize_t crt_sys_srst_0_show(struct kobject *kobj, struct kobj_attribute *attr,
								   char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_SRST0_reg, crt_sys_srst_0_reg, SYS_REG_SYS_SRST_0_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_srst_1_show(struct kobject *kobj, struct kobj_attribute *attr,
								   char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_SRST1_reg, crt_sys_srst_1_reg, SYS_REG_SYS_SRST_1_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_srst_2_show(struct kobject *kobj, struct kobj_attribute *attr,
								   char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_SRST2_reg, crt_sys_srst_2_reg, SYS_REG_SYS_SRST_2_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}


static ssize_t crt_sys_srst_3_show(struct kobject *kobj, struct kobj_attribute *attr,
								   char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_SRST3_reg, crt_sys_srst_3_reg, SYS_REG_SYS_SRST_3_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_clken_0_show(struct kobject *kobj, struct kobj_attribute *attr,
									char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_CLKEN0_reg, crt_sys_clken_0_reg, SYS_REG_SYS_CLKEN_0_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_clken_1_show(struct kobject *kobj, struct kobj_attribute *attr,
									char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_CLKEN1_reg, crt_sys_clken_1_reg, SYS_REG_SYS_CLKEN_1_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_clken_2_show(struct kobject *kobj, struct kobj_attribute *attr,
									char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_CLKEN2_reg, crt_sys_clken_2_reg, SYS_REG_SYS_CLKEN_2_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_sys_clken_3_show(struct kobject *kobj, struct kobj_attribute *attr,
									char *buf)
{
	char regInfoString[1024] = {0};

	//fix me later by ben
	//crt_reg_show(SYS_REG_SYS_CLKEN3_reg, crt_sys_clken_3_reg, SYS_REG_SYS_CLKEN_3_SIZE, regInfoString);

	return sprintf(buf, "%s\n", regInfoString);
}

static ssize_t crt_show(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
#if 0	//fix me later by ben.

	char regInfoString[1024];
	char *pregInfoString = regInfoString;
	int regInfoStringCount;
	unsigned int regVal, regIndex;

	hw_semaphore_try_lock();
	/* SYS_REG_SYS_SRST register */
	for (regIndex = SYS_REG_SYS_SRST0_reg; regIndex <= SYS_REG_SYS_SRST3_reg ; regIndex += 4) {
		regVal = rtd_inl(regIndex);
		regInfoStringCount = sprintf(pregInfoString, "===== RSTN (0x%08X) = (0x%08X) ===== \n", regIndex, regVal);
		pregInfoString += regInfoStringCount;
	}

	/* SYS_REG_SYS_CLKEN register */
	for (regIndex = SYS_REG_SYS_CLKEN0_reg; regIndex <= SYS_REG_SYS_CLKEN3_reg ; regIndex += 4) {
		regVal = rtd_inl(regIndex);
		regInfoStringCount = sprintf(pregInfoString, "===== CLK_EN (0x%08X) = 0x%08X ===== \n", regIndex, regVal);
		pregInfoString += regInfoStringCount;
	}
	hw_semaphore_unlock();

	return sprintf(buf, "%s\n", regInfoString);
#endif
	return 0;
}

static void crt_register_help(void)
{
	rtd_pr_crt_err("password on  : echo 0x1234abcd > /sys/crt/register\n");
	rtd_pr_crt_err("password off : echo 0xabcd1234 > /sys/crt/register\n");
	rtd_pr_crt_err("set value command : echo register_addr register_val > /sys/crt/register\n");
	rtd_pr_crt_err("command example : echo 0xb8060158 0x1234 > /sys/crt/register\n");
	rtd_pr_crt_err("setbit command    : echo setbit 0xb8060158 0xFF > /sys/crt/register\n");
	rtd_pr_crt_err("clearbit command  : echo clearbit 0xb8060158 0xF > /sys/crt/register\n");
	rtd_pr_crt_err("setzero command   : echo setzero 0xb8060158 > /sys/crt/register\n");

}

static ssize_t crt_register_show(struct kobject *kobj, struct kobj_attribute *attr,
								 char *buf)
{
	if (register_password == 0) {
		return sprintf(buf, "enter register address access password\n");
	} else if ((register_addr < 0xb8000000)) {
		return sprintf(buf, "wrong register address 0x%08X\n", register_addr);
	}

	return sprintf(buf, "reg addr = 0x%08X , val = 0x%08X\n", register_addr, rtd_inl(register_addr));
}

typedef enum {
	CRT_COMMAND_SET_ZERO = 1,
	CRT_COMMAND_SETBIT,
	CRT_COMMAND_CLEARBIT
} CRT_COMMAND;
static ssize_t crt_register_store(struct kobject *kobj, struct kobj_attribute *attr,
								  const char *buf, size_t count)
{
	int ret = 0, command = 0;
	char cmd_string[32] = "";

	if ((buf == NULL) || (strlen(buf) == 0)) {
		goto Fail;
	}

	sscanf(buf, "%s", cmd_string);
	if (strlen(cmd_string) == 0) {
		goto Fail;
	}

	if (strcmp(cmd_string,"help")==0) {
		crt_register_help();
		return count;
	}
	else if (strcmp(cmd_string,"setzero")==0) {
		command = CRT_COMMAND_SET_ZERO;
	}
	else if (strcmp(cmd_string,"setbit")==0) {
		command = CRT_COMMAND_SETBIT;
	}
	else if (strcmp(cmd_string,"clearbit")==0) {
		command = CRT_COMMAND_CLEARBIT;
	}

	if (command > 0) {
		ret = sscanf(buf, "%*s 0x%08X 0x%08X", &register_addr, &register_val);
	}
	else {
		ret = sscanf(buf, "0x%08X 0x%08X", &register_addr, &register_val);
	}

	if (ret <= 0) {
		goto Fail;
	}

	if (register_addr == 0x1234abcd)
		register_password = 1;

	if (register_addr == 0xabcd1234)
		register_password = 0;

	if ((register_password == 1) && (command > 0)) {
		switch (command) {
		case CRT_COMMAND_SET_ZERO:
			rtd_outl(register_addr, 0);
			break;
		case CRT_COMMAND_SETBIT :
			rtd_setbits(register_addr, register_val);
			break;
		case CRT_COMMAND_CLEARBIT :
			rtd_clearbits(register_addr, register_val);
			break;
		default :
			break;
		}
		register_val = 0;
	}
	else if ((register_password == 1) && (register_val != 0)) {
		rtd_outl(register_addr, register_val);
		register_val = 0;
	}

	return count;

Fail:
	rtd_pr_crt_err("error command! Please input command like below:\n\n");
	crt_register_help();
	return count;
}


static struct kobj_attribute crt_status_attribute =
	__ATTR(crt_status, 0444, crt_show, NULL);
static struct kobj_attribute crt_sys_srst_0_attribute =
	__ATTR(crt_sys_srst_0, 0444, crt_sys_srst_0_show, NULL);
static struct kobj_attribute crt_sys_srst_1_attribute =
	__ATTR(crt_sys_srst_1, 0444, crt_sys_srst_1_show, NULL);
static struct kobj_attribute crt_sys_srst_2_attribute =
	__ATTR(crt_sys_srst_2, 0444, crt_sys_srst_2_show, NULL);
static struct kobj_attribute crt_sys_srst_3_attribute =
	__ATTR(crt_sys_srst_3, 0444, crt_sys_srst_3_show, NULL);
static struct kobj_attribute crt_sys_clken_0_attribute =
	__ATTR(crt_sys_clken_0, 0444, crt_sys_clken_0_show, NULL);
static struct kobj_attribute crt_sys_clken_1_attribute =
	__ATTR(crt_sys_clken_1, 0444, crt_sys_clken_1_show, NULL);
static struct kobj_attribute crt_sys_clken_2_attribute =
	__ATTR(crt_sys_clken_2, 0444, crt_sys_clken_2_show, NULL);
static struct kobj_attribute crt_sys_clken_3_attribute =
	__ATTR(crt_sys_clken_3, 0444, crt_sys_clken_3_show, NULL);
static struct kobj_attribute crt_register =
/*	__ATTR(register, 0666, crt_register_show, crt_register_store); // fix compile error */
__ATTR(register, 0444, crt_register_show, crt_register_store);


static struct attribute *attrs[] = {
	&crt_status_attribute.attr,
	&crt_sys_srst_0_attribute.attr,
	&crt_sys_srst_1_attribute.attr,
	&crt_sys_srst_2_attribute.attr,
	&crt_sys_srst_3_attribute.attr,
	&crt_sys_clken_0_attribute.attr,
	&crt_sys_clken_1_attribute.attr,
	&crt_sys_clken_2_attribute.attr,
	&crt_sys_clken_3_attribute.attr,
	&crt_register.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

/* CRT char device file operation session */
long crt_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int retval = 0;
	/* no user space ioctl  */
#if 0
	unsigned long flags;
	crt_clk_onoff_cmd CLK_OnOff_CMD;

	switch (cmd) {
	case CRT_IOC_CLK_ONOFF:
		retval = copy_from_user((void *)&CLK_OnOff_CMD, (const void __user *)arg, sizeof(crt_clk_onoff_cmd)) ?  -EFAULT : 0;
		if (CLK_OnOff_CMD.private_data == NULL) {
			CRT_CLK_OnOff(CLK_OnOff_CMD.module_ip, CLK_OnOff_CMD.clken, NULL);
		} else {
			CRT_CLK_OnOff(CLK_OnOff_CMD.module_ip, CLK_OnOff_CMD.clken, CLK_OnOff_CMD.private_data);
		}

		rtd_pr_crt_debug("CRT : module %s, CLK %s\n", CRT_CLK_ModuleName(CLK_OnOff_CMD.module_ip), (CLK_OnOff_CMD.clken) ? "OFF" : "ON");
		break;
	default:
		rtd_pr_crt_err("CRT : wrong ioctl cmd\n");
	}
#endif
	return retval;
}


int crt_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int crt_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* CRT file operation */
struct file_operations crt_fops = {
read:
	NULL,
write:
	NULL,
unlocked_ioctl:
	crt_ioctl,
open:
	crt_open,
release:
	crt_release,
};

/* set /dev/crt mode to 0666  */
static char *crt_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}


#ifdef CONFIG_PM
static int crt_suspend(struct platform_device *dev, pm_message_t state)
{
	rtd_pr_crt_debug("crt_suspend\n");
	return 0;
}
static int crt_resume(struct platform_device *dev)
{
	rtd_pr_crt_debug("crt_resume\n");
	return 0;
}
#endif

static struct platform_driver crt_platform_driver = {
#ifdef CONFIG_PM
	.suspend    = crt_suspend,
	.resume     = crt_resume,
#endif
	. driver = {
		.name       = "crt",
		.bus        = &platform_bus_type,
	} ,
} ;


static int __init crt_module_init(void)
{
	int retval;

	crt_kobj = kobject_create_and_add("crt", NULL);
	if (!crt_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(crt_kobj, &attr_group);
	if (retval)
		kobject_put(crt_kobj);


	/* register device to get major and minor number */
	if (crt_major) {
		crt_devnum = MKDEV(crt_major, crt_minor);
		retval = register_chrdev_region(crt_devnum, crt_nr_devs, "crt");
	} else {
		retval = alloc_chrdev_region(&crt_devnum, crt_minor, crt_nr_devs, "crt");
		crt_major = MAJOR(crt_devnum);
	}
	if (retval < 0) {
		rtd_pr_crt_warn("CRT : can't get major %d\n", crt_major);
		return retval;
	}

	/* create device node by udev API */
	crt_class = class_create(THIS_MODULE, "crt");
	if (IS_ERR(crt_class)) {
		return PTR_ERR(crt_class);
	}
	crt_class->devnode = crt_devnode;
	crt_device = device_create(crt_class, NULL, crt_devnum, NULL, "crt");
	rtd_pr_crt_info("(N)CRT module init, major number = %d, device name = %s \n", crt_major, dev_name(crt_device));

	/* cdev API to register file operation */
	cdev_init(&crt_cdev, &crt_fops);
	crt_cdev.owner = THIS_MODULE;
	crt_cdev.ops = &crt_fops;
	retval = cdev_add(&crt_cdev, crt_devnum , 1);
	if (retval) {
		rtd_pr_crt_err("Error %d adding char_reg_setup_cdev", retval);
	}

	crt_platform_devs = platform_device_register_simple("crt", -1, NULL, 0);
	/* rtd_pr_crt_info("CRT platform device name %s\n", dev_name(&crt_platform_devs->dev)); */

	if (platform_driver_register(&crt_platform_driver) != 0) {
		platform_device_unregister(crt_platform_devs);
		crt_platform_devs = NULL;
		rtd_pr_crt_warn("crt platform driver register fail\n");
		return retval;
	}

	memset(moduleName, 0, sizeof(moduleName));
	memset(lastModuleName, 0, sizeof(lastModuleName));

	return retval;
}

static void __exit crt_module_exit(void)
{
	if (crt_platform_devs)
		platform_device_unregister(crt_platform_devs);

	platform_driver_unregister(&crt_platform_driver);

	kobject_put(crt_kobj);
	device_destroy(crt_class, crt_devnum);
	class_destroy(crt_class);
	unregister_chrdev_region(crt_devnum, crt_nr_devs);

}

module_init(crt_module_init);
module_exit(crt_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Tseng <mark.tseng@realtek.com>");
#endif /* !BUILD_QUICK_SHOW */
