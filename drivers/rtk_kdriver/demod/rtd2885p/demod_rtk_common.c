#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/io.h>
#include <asm/cacheflush.h>
#include "demod_rtk_common.h"
#include "comm.h"
#include "tv_osal.h"
//#include "ifd_analog.h"

//#include "demodrbus/sys_reg_reg.h"
//#include "demodrbus/ifd_108_reg.h"
#include "rbus/tv_sb1_ana_reg.h"
#include "rbus/dtv_frontend_reg.h"
#include "rbus/diseqc_reg.h"
//#include "rbus/img_demod_bus_reg.h"
//#include "rbus/demod_sd_reg.h"
//#include "demodcore/message_func_rtk.h"
#include "rbus/rtk_dtv_demod_8051_reg.h"
#include "rbus/dtv_demod_misc_reg.h"
#include "rbus/rtk_dtv_demod_sys_reg.h"
#include "rbus/hdic_register_bus_reg.h"
#include "rbus/stb_reg.h"
#include "rbus/topbist_reg.h"
#include "rbus/pll27x_reg_reg.h"
#include <rbus/tvsb5_reg.h>

#include <linux/dma-mapping.h>
#include <linux/pageremap.h>

#include <tp/tp_def.h>
#include <tp/tp_drv_api.h>
#include "rbus/efuse_reg.h"

//#include <rtk_kdriver/rtk_otp_region_api.h>


extern int rtk_otp_field_read_int_by_name(const char *name);

RTK_DEMOD_MODE m_realtek_TvSys_mode;

unsigned long *pgShareMemory = NULL;
unsigned long ShareMemoryPhysicalAddress;
unsigned char Atsc3RtkExtDemodFlag = 0;

#define DYNAMIC_MEM_ALLOCATE 0
#define TPO_DIVIDEND_CONSTANT 250*1000
#define TPO_DVISOR_CONSTANT   2
// Merlin4 Ver.A bring up
//#define MERLIN4_VER_A
//#define REALTEK_R_EXT_QFN


/*------------------------------------------------------------------------------
 * Chip Initialization
 *------------------------------------------------------------------------------*/
//#define rtd_outl(addr, value)   pli_writeReg32(addr, value)
//#define rtd_inl(addr)           pli_readReg32(addr)
//#define rtd_maskl(addr, mask, value)    rtd_outl(addr, ((rtd_inl(addr) & mask) | value))

#define _BIT(x)       (1UL <<x)
#ifndef _BIT0
#define _BIT0          _BIT(0 )
#define _BIT1          _BIT(1 )
#define _BIT2          _BIT(2 )
#define _BIT3          _BIT(3 )
#define _BIT4          _BIT(4 )
#define _BIT5          _BIT(5 )
#define _BIT6          _BIT(6 )
#define _BIT7          _BIT(7 )
#define _BIT8          _BIT(8 )
#define _BIT9          _BIT(9 )
#define _BIT10         _BIT(10)
#define _BIT11         _BIT(11)
#define _BIT12         _BIT(12)
#define _BIT13         _BIT(13)
#define _BIT14         _BIT(14)
#define _BIT15         _BIT(15)
#define _BIT16         _BIT(16)
#define _BIT17         _BIT(17)
#define _BIT18         _BIT(18)
#define _BIT19         _BIT(19)
#define _BIT20         _BIT(20)
#define _BIT21         _BIT(21)
#define _BIT22         _BIT(22)
#define _BIT23         _BIT(23)
#define _BIT24         _BIT(24)
#define _BIT25         _BIT(25)
#define _BIT26         _BIT(26)
#define _BIT27         _BIT(27)
#define _BIT28         _BIT(28)
#define _BIT29         _BIT(29)
#define _BIT30         _BIT(30)
#define _BIT31         _BIT(31)
#endif

//#define REG_RTK_DEMOD_8051_SRAM_WRITE_EN 0xB807223C//Merlin3/Mac5p Demod 8051

//static int DtvDemodInit = 0;
static int DtvDemodReset = 0;
static int pli_ready = 0;
U32BITS RbusAddr = 0;
unsigned char TunerInputMode = 0;//DVBS Demod default use differential+normal mode
unsigned char I2cFastSpeedFlag = 0;

int DtvGetChipVersion(void)
{
	return 0;
}


int DtvGetICVersion(void)
{
	/*	int ICVersion;

		ICVersion =   STB_SC_VerID_get_vercode(rtd_inl(STB_SC_VerID_reg));

		if (ICVersion == 0x65570000)
			return 1;  //Version A
		else
			return 0;   //Version Unknow
	*/
	return 0;
}


int TvFrontendPowerControl(unsigned char On)
{
	if (On) {
		CRT_CLK_OnOff(TVFRONTEND, CLK_ON, NULL);
		RTK_DEMOD_INFO("demod is power on.\n");
	} else {
		CRT_CLK_OnOff(TVALL, CLK_OFF, NULL);
		RTK_DEMOD_INFO("demod is power down.\n");
		pli_ready = 0;
	}

	return FUNCTION_SUCCESS;
}


int TvFrontendPowerControlIsdbt(unsigned char On)
{
	if (On) {
		CRT_CLK_OnOff(TVFRONTEND, CLK_ON, NULL);
		RTK_DEMOD_INFO("demod is power on.\n");
	} else {
		CRT_CLK_OnOff(TVALLISDBT, CLK_OFF, NULL);
		RTK_DEMOD_INFO("isdbt demod is power down.\n");
		pli_ready = 0;
	}

	return FUNCTION_SUCCESS;
}


int DtvDemodEnable(RTK_DEMOD_MODE mode)
{
	//DTV Demod misc on fisrt then Demod IP
	CRT_CLK_OnOff(DTVDEMOD, CLK_ON, NULL);

	switch (mode) {
	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
	case RTK_DEMOD_MODE_ISDB:
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
		RTK_DEMOD_INFO("DtvDemodEnable to DVBT DVBT2\n");
		break;

	case RTK_DEMOD_MODE_DTMB:
		CRT_CLK_OnOff(DTVHDIC, CLK_ON, NULL);
		RTK_DEMOD_INFO("DtvDemodEnable to DTMB\n");
		break;

	case RTK_DEMOD_MODE_DVBC:
	case RTK_DEMOD_MODE_ATSC:
	case RTK_DEMOD_MODE_QAM:
		CRT_CLK_OnOff(DTVRTK, CLK_ON, NULL);
		RTK_DEMOD_INFO("DtvDemodEnable to DVBC ISDBT ATSC OC\n");
		break;

	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		CRT_CLK_OnOff(DISEQC, CLK_ON, NULL);
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
		RTK_DEMOD_INFO("DtvDemodEnable to DVBSx\n");
		break;

	default:
		CRT_CLK_OnOff(DTVATB, CLK_ON, NULL);
		RTK_DEMOD_INFO("DtvDemodEnable mode = %d\n", mode);
		break;
	}

	return 0;
}


int DtvIsDemodClockEnable(RTK_DEMOD_MODE mode)
{
	U32BITS sysclk0 = rtd_inl(SYS_REG_SYS_CLKEN0_reg);
	U32BITS sysclk2 = rtd_inl(SYS_REG_SYS_CLKEN2_reg);
	U32BITS sysrst0 = rtd_inl(SYS_REG_SYS_SRST0_reg);
	U32BITS sysrst1 = rtd_inl(SYS_REG_SYS_SRST1_reg);
	U32BITS sysrst2 = rtd_inl(SYS_REG_SYS_SRST2_reg);
	U32BITS SysRstCheckCom = 0, SysRstCheckMode = 0;
	U32BITS SysClkCheckCom = 0, SysClkCheckMode = 0;

	SysClkCheckCom = SYS_REG_SYS_CLKEN0_get_clken_dtv_frontend(sysclk0)&SYS_REG_SYS_CLKEN0_get_clken_dtv_demod_mb(sysclk0)&SYS_REG_SYS_CLKEN2_get_clken_dtv_demod(sysclk2);
	SysRstCheckCom = SYS_REG_SYS_SRST0_get_rstn_dtv_frontend(sysrst0)&SYS_REG_SYS_SRST1_get_rstn_dtv_demod_mb(sysrst1)&SYS_REG_SYS_SRST2_get_rstn_dtv_demod(sysrst2);

	if (mode == RTK_DEMOD_MODE_DTMB) {
		SysClkCheckMode = SYS_REG_SYS_CLKEN0_get_clken_hdic(sysclk0);
		SysRstCheckMode = SYS_REG_SYS_SRST0_get_rstn_hdic_apb(sysrst0) & SYS_REG_SYS_SRST0_get_rstn_hdic_axi(sysrst0) & SYS_REG_SYS_SRST0_get_rstn_hdic(sysrst0);
	} else if ((mode == RTK_DEMOD_MODE_ATSC) || (mode == RTK_DEMOD_MODE_QAM) || (mode == RTK_DEMOD_MODE_DVBC)) {
		SysClkCheckMode = SYS_REG_SYS_CLKEN0_get_clken_rtk_demod(sysclk0);
		SysRstCheckMode = SYS_REG_SYS_SRST0_get_rstn_rtk_demod(sysrst0);
	} else if ((mode == RTK_DEMOD_MODE_DVBT) || (mode == RTK_DEMOD_MODE_DVBT2) || (mode == RTK_DEMOD_MODE_ISDB)) {
		SysClkCheckMode = SYS_REG_SYS_CLKEN0_get_clken_atb_demod(sysclk0) & SYS_REG_SYS_CLKEN0_get_clken_atb_demod_wrap(sysclk0);
		SysRstCheckMode = SYS_REG_SYS_SRST0_get_rstn_atb_demod(sysrst0) & SYS_REG_SYS_SRST0_get_rstn_atb_demod_wrap(sysrst0);
	} else if ((mode == RTK_DEMOD_MODE_DVBS) || (mode == RTK_DEMOD_MODE_DVBS2)) {
		SysClkCheckMode = SYS_REG_SYS_CLKEN0_get_clken_atb_demod(sysclk0) & SYS_REG_SYS_CLKEN0_get_clken_atb_demod_wrap(sysclk0);
		SysRstCheckMode = SYS_REG_SYS_SRST0_get_rstn_atb_demod(sysrst0) & SYS_REG_SYS_SRST0_get_rstn_atb_demod_wrap(sysrst0);
	} else {
		SysClkCheckMode = 0;
		SysRstCheckMode = 0;
		RTK_DEMOD_WARNING("UNKNOW DTV Standard %d\n", mode);
	}

	if (SysClkCheckCom & SysRstCheckCom & SysClkCheckMode &  SysRstCheckMode) {
		return 1;
	} else {
		RTK_DEMOD_WARNING("Check DTV Demod Clock Off in DtvIsDemodClockEnable; standard = %d, SYS_CLKEN0 = "PT_H32BITS", SYS_CLKEN2 = "PT_H32BITS", SYS_SRST0 ="PT_H32BITS", SYS_SRST2 ="PT_H32BITS"\n", mode, sysclk0, sysclk2, sysrst0, sysrst2);
		return 0;
	}
}

int DtvPGAControl(unsigned char PGAEnable, IFADCGAIN PGAGain)
{
	if (PGAGain < PGAFIX_0DB) {
		RTK_DEMOD_INFO("PGA Control : FIX PGA gain -%d dB\n", PGAGain);
		PGAGain = (-1) * PGAGain;
		//rtd_maskl(TV_SB1_ANA_IQADC_CTRL2_reg, ~(0x7 << 4), (PGAGain << 4));
		//rtd_maskl(TV_SB1_ANA_IQADC_CTRL5_reg, ~(0x7), (0x0));
	} else {
		RTK_DEMOD_INFO("PGA Control : FIX PGA gain %d dB\n", PGAGain);
		//rtd_maskl(TV_SB1_ANA_IQADC_CTRL2_reg, ~(0x7 << 4), (0x0 << 4));
		//rtd_maskl(TV_SB1_ANA_IQADC_CTRL5_reg, ~(0x7), (PGAGain));
	}
	return 0;
}


void  DtvDumpShareRegStatus(void)
{
	return;
}


void ExtAtsc3PowOffOn(unsigned char on)
{
	//Check IFADC CLK turn on
	if (on == 1) {
#ifndef REALTEK_R_EXT_QFN //MCM

		//rtd_maskl(PLL27X_REG_SYS_PLL_27X3_reg, 0x0FFFFFEFF, 0x00000100); //XTAL gating release
		//tv_osal_msleep(1);

		CRT_CLK_OnOff(DTVExtAtsc3, CLK_ON, NULL);

		// Merlin6 DEMOD_IF_AGC Pinmux set to GPI
		rtd_maskl(0xb8000844, 0x0FFFFFFF, 0xF0000000);       //<bit31:28>GPIO_61, Set pinmux  to MIS_GPIO_bit61
		rtd_outl(0xb801bd04, 0x80000000);                             //<bit31>GPIO_61=0x0: Configured as input pin, <bit0>write bit = 0x0
		RTK_DEMOD_INFO("ExtMcmAtsc3PowOffOn!! On = %u\n", on);
#else //QFN: Merlin6 GPIO_46 (0->1)
		rtd_maskl(0xb8000834, 0x0FFFFFFF, 0xF0000000);
		rtd_outl(0xb801bd1c, 0x00010000);
		rtd_outl(0xb801bd04, 0x00010001);
		mdelay(1);
		rtd_outl(0xb801bd1c, 0x00010001);
		udelay(10);
		RTK_DEMOD_INFO("ExtQfnAtsc3PowOffOn!! On = %u\n", on);
#endif

	} else {
#ifndef REALTEK_R_EXT_QFN //MCM
		CRT_CLK_OnOff(DTVExtAtsc3, CLK_OFF, NULL);
		RTK_DEMOD_INFO("ExtMcmAtsc3PowOffOn!! On = %u\n", on);
#else //QFN: Merlin6 GPIO_46 Pull Low
		rtd_maskl(0xb8000834, 0x0FFFFFFF, 0xF0000000);
		rtd_outl(0xb801bd1c, 0x00010000);
		rtd_outl(0xb801bd04, 0x00010001);
		RTK_DEMOD_INFO("ExtQfnAtsc3PowOffOn!! On = %u\n", on);
#endif
	}
}



void DemodAdcDefault(void)
{
	//past from script
	//Merlin5 ADC register default
	//rtd_outl(0xb8033100, 0x00010502);					//<bit16=1>IFD MBIAS power on,<bit11:10>REG_I_VCM_SEL=0x1(0.632V),<bit9:8>ADC2X_BG_SEL= 0x1(1.216V),  <bit7:0>REG_IFD_RESERVED=0x02(10u PGA IB20U select)
	//rtd_outl(0xb8033104, 0x00000140);					//<bit24>IQADC input cm power off, <bit9:8>REG_IPGA_BW=0x1(75M), <bit6:4>REG_IFCMP_ISEL=0x4(50uF)
	//rtd_outl(0xb8033118, 0x30200010);					//<bit30:28>IQ ADC calibration input swing=0x11, <bit21:20>REG_IQ_CAL_DIV=0x10, REG_CKOUT_SEL_Q: the default is "1:I_CKOUT" on AIF spec
	//rtd_outl(0xb8033120, 0x00040000);					//<bit18:16>REG_IQCAL_ISEL=0x100(50uA), IF ADC GAIN FIX EN <bit0> = 0(SIF mode)
	//rtd_outl(0xb8033124, 0x00000000);					//PGA/ADC Power Off
	//rtd_outl(0xb8033128, 0x00100022);					//<bit21:20>REG_QPGA_BW=0x1(75M), Default setting(10=20uA), IQ PGA CURRENT  select

	//IFADC+IFPGA power on
	//rtd_maskl(0xb8033104, 0xfefffcff, 0x00000100);	//REG_ADC2X_DIFPOW, IFADC input cm power set to off(b24)=0x0, IPGA_BW<bit9:8>=0x01(75M)
	rtd_maskl(0xb8033124, 0xfffffefe, 0x00000101);		//I_PGA<bit8> power on, <bit0>I_ADC power on
	rtd_maskl(0xb8033128, 0x8ff8ffff, 0x00020000);		//<bit30:28>REG_QPGA_INPUT_SEL=0x0(No input),<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)

	//IQADC+IQPGA power on
	//rtd_outl(0xb803312C, 0x00000400);					//default setting, <bit11:10>REG_Q_VCM_SEL=0x1(0.632V)
	//rtd_outl(0xb8033130, 0x44004000);					//default setting, <bit31:30>REG_QADC_I_VREF_SEL=0x1(1.0X),<bit27:26>REG_IADC_I_VREF_SEL=0x1(1.0X),<bit15:14>REG_QADC_LDO0_SEL=0x1(1.3V)
	//rtd_outl(0xb8033134, 0x11000091);					//default setting, <bit29:28>REG_QADC_VREF_SEL=0x1(1.4V), <bit25:24>REG_IADC_VREF_SEL=0x1(1.4V), <bit8:7>REG_IADC_LDO0_SEL=0x1(1.3V)
	//<bit6:4>REG_QADC_LDO_SEL=0x1(0.961V), <bit2:0>REG_IADC_LDO_SEL=0x1(0.961V)
	//Set IPGA/QPGA GAIN = 0dB
	rtd_maskl(0xb8033120, 0xfffffffe, 0x00000001);		//IF ADC GAIN FIX EN <bit0>= 1(Internal Demod IQ mode)
	rtd_maskl(0xb803312c, 0xffeecc00, 0x00000000);		//<bit20>Q single_end=0x0, <bit16>I single_end=0, <bit13:12>Q_CKIN_Sel=00(from PLL27x_IFADC), <bit9:8>I_CKIN_Sel=00: From PLL27X_IFADC, <bit7:4>reg_qpga_d=0x0(0dB), <bit3:0>reg_ipga_d=0x0(0dB)
}


void DemodAdcSetting(RTK_DEMOD_MODE mode)
{
	//ADC power on and set PGA input select
	rtd_maskl(0xb8033124, 0xffffeeee, 0x00000101);		//I_PGA<bit8> power on, <bit0>I_ADC power on
	rtd_maskl(0xb8033128, 0x8ff8ffff, 0x00020000);		//<bit30:28>REG_QPGA_INPUT_SEL=0x0(No input),<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)

	rtd_maskl(0xb803312c, 0xfffff3ff, 0x00000800);		//default setting, <bit11:10>REG_Q_VCM_SEL=0x2(0.656V)
	rtd_maskl(0xb8033130, 0x33ffffff, 0xCC000000);      //default setting, <bit31:30>REG_QADC_I_VREF_SEL=0x3(2X),<bit27:26>REG_IADC_I_VREF_SEL=0x3(2X)
	rtd_maskl(0xb8033134, 0xccffff88, 0x22000055);      //default setting, <bit29:28>REG_QADC_VREF_SEL=0x2(1.4V), <bit25:24>REG_IADC_VREF_SEL=0x2(1.4V)
	//<bit6:4>REG_QADC_LDO_SEL=0x5(0.908V), <bit2:0>REG_IADC_LDO_SEL=0x5(0.908V)

	IQAdcLdoSetting();                          //add IQADC_LDO setting from OTP setting
	QVcmSetting();				//q_vcm setting from OTP setting

	//PGA power on and set PGA gain 0dB
	rtd_maskl(0xb8033120, 0xfffffffe, 0x00000001);		//IF ADC GAIN FIX EN <bit0>= 1(Internal Demod IQ mode)


	switch (mode) {
	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		rtd_maskl(0xb8033104, 0xfefffcff, 0x00000200);	//REG_ADC2X_DIFPOW, IFADC input cm power set to off(b24)=0x0, IPGA_BW<bit9:8>=0x2(50M)
		rtd_maskl(0xb8033124, 0xffffeeee, 0x00001111);	//I_PGA<bit8> power on, I_QGA<bit12> power on, <bit0>I_ADC power on, <bit4>Q_ADC power on
		rtd_maskl(0xb8033128, 0x8e088fff, 0x10210000);		//IPGA_FIX_GAIN<bit14:12>=0x000(depend on REG_IPGA_D), Ipga input sel<bit18:16>=0x001(I input),
		if ((TunerInputMode == 0) || (TunerInputMode == 2)) {
			rtd_maskl(0xb803312c, 0xffeecc00, 0x00000000);	//Select to Differential, Q single_end<bit20>=0x0(disable), I single_end<bit16>=0(disable), Q_CKIN_Sel<bit13:12>=00, I_CKIN_Sel<bit9:8>=00: From PLL27X_IFADC, reg_qpga_d<bit7:4>=0x0(0dB), reg_ipga_d<bit3:0>=0x0(0dB)
			RTK_DEMOD_INFO("DemodAdcSetting to DVBSx(Differential + 0dB)\n");
		} else {
			rtd_maskl(0xb803312c, 0xffeecc00, 0x00110000);	//Select to Single End, Q single_end<bit20>=0x1(Enable), I single_end<bit16>=1(Enable), Q_CKIN_Sel<bit13:12>=00, I_CKIN_Sel<bit9:8>=00: From PLL27X_IFADC, reg_qpga_d<bit7:4>=0x0(0dB), reg_ipga_d<bit3:0>=0x0(0dB)
			RTK_DEMOD_INFO("DemodAdcSetting to DVBSx(Single End + 0dB)\n");
		}

		//rtd_maskl(0xB80004b8, 0xfffffdff, 0x00000000);  			//<bit9>IFADC ATV clk enable turn off
		//rtd_maskl(0xB8000500, 0xffffffef, 0x00000010);  			//<bit4>PLL27X_IFADC_DTV_CKO_EN setup
		//rtd_maskl(0xB80004b8, 0xfffffdff, 0x00000200);  			//<bit9>IFADC ATV clk enable turn on
		tv_osal_msleep(5);
		break;

	case RTK_DEMOD_MODE_ATV:
		RTK_DEMOD_INFO("DemodAdcSetting to ATV\n");
		rtd_maskl(0xb8033104, 0xfefffcff, 0x00000100);	//REG_ADC2X_DIFPOW, IFADC input cm power set to off(b24)=0x0, IPGA_BW<bit9:8>=0x01(75M)
		rtd_maskl(0xb8033124, 0xffffeeee, 0x00000101);		        //I_PGA<bit8> power on, <bit0>I_ADC power on
		rtd_maskl(0xb8033128, 0x8ff8ffff, 0x00020000);		        //<bit30:28>REG_QPGA_INPUT_SEL=0x0(No input),<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)
		rtd_maskl(0xb803312c, 0xffeecc00, 0x00000000);	            //Select to Differential, Q single_end<bit20>=0x0(disable), I single_end<bit16>=0(disable), Q_CKIN_Sel<bit13:12>=00, I_CKIN_Sel<bit9:8>=00: From PLL27X_IFADC, reg_qpga_d<bit7:4>=0x0(0dB), reg_ipga_d<bit3:0>=0x0(0dB)

		//ATV mode, set PLL27X_IFADC_DUTY and ATVADCDIV_RSTB toggle flow
		rtd_maskl(0xb80004b8, 0xfffffdff, 0x00000000);				//<bit9>PLL27X_IFADC_ATV_CKO_EN = 0x0(turn off)
		rtd_maskl(0xB80004dc, 0x9fffffff, 0x40000000); 				//<30:29>PLL27X_IFADC_DUTY setup
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00000000);				//<bit17>ATVADCDIV_RSTB = 0x0(active reset)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000);				//<bit17>ATVADCDIV_RSTB = 0x1(release reset)
		rtd_maskl(0xb80004b8, 0xfffffdff, 0x00000200);				//<bit9>PLL27X_IFADC_ATV_CKO_EN = 0x1(turn on)
		tv_osal_msleep(5);                                          //delay 5mS
		break;

	default:
		RTK_DEMOD_INFO("DemodAdcSetting to mode = %d\n", mode);
		rtd_maskl(0xb8033104, 0xfefffcff, 0x00000100);	//REG_ADC2X_DIFPOW, IFADC input cm power set to off(b24)=0x0, IPGA_BW<bit9:8>=0x01(75M)
		rtd_maskl(0xb8033124, 0xffffeeee, 0x00000101);		        //I_PGA<bit8> power on, <bit0>I_ADC power on
		rtd_maskl(0xb8033128, 0x8ff8ffff, 0x00020000);		        //<bit30:28>REG_QPGA_INPUT_SEL=0x0(No input),<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)
		rtd_maskl(0xb803312c, 0xffeecc00, 0x00000000);	            //Select to Differential, Q single_end<bit20>=0x0(disable), I single_end<bit16>=0(disable), Q_CKIN_Sel<bit13:12>=00, I_CKIN_Sel<bit9:8>=00: From PLL27X_IFADC, reg_qpga_d<bit7:4>=0x0(0dB), reg_ipga_d<bit3:0>=0x0(0dB)
		//DTV mode PLL27X_IFADC_CKO_EN toggle flow
		//rtd_maskl(0xB80004b8, 0xfffffdff, 0x00000000);  			//<bit9>IFADC ATV clk enable turn off
		//rtd_maskl(0xB8000500, 0xffffffef, 0x00000010);  			//<bit4>PLL27X_IFADC_DTV_CKO_EN setup
		//rtd_maskl(0xB80004b8, 0xfffffdff, 0x00000200);  			//<bit9>IFADC ATV clk enable turn on
		tv_osal_msleep(5);
		break;
	}
}


void DemodAdcCapcalSetting(RTK_DEMOD_MODE mode)
{
	switch (mode) {
	case RTK_DEMOD_MODE_DTMB:
	case RTK_DEMOD_MODE_ATV:
	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
		RTK_DEMOD_INFO("DemodAdcCapcalSetting: 0x00060006\n");
		rtd_outl(0xb803311c, 0x00060006);					//QADC_CAPCAL<2:0>=110 / IADC_CAPCAL<2:0>=110 for IDNL measurement
		break;
	case RTK_DEMOD_MODE_ISDB:
	case RTK_DEMOD_MODE_ATSC:
	case RTK_DEMOD_MODE_QAM:
	case RTK_DEMOD_MODE_DVBC:
	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		RTK_DEMOD_INFO("DemodAdcCapcalSetting: 0x00050005\n");
		rtd_outl(0xb803311c, 0x00050005);					//QADC_CAPCAL<2:0>=101 / IADC_CAPCAL<2:0>=101 for IDNL measurement
		break;
	default:
		RTK_DEMOD_INFO("DemodAdcCapcalSetting: 0x00040004\n");
		rtd_outl(0xb803311c, 0x00040004);					//QADC_CAPCAL<2:0>=100/ IADC_CAPCAL<2:0>=100(Default value) for IDNL measurement
		break;
	}
}


int IQAdcLdoSetting(void)
{
	int Iotp=0, Qotp=0;					
	int ifd_iadc_ldo_data = rtk_otp_field_read_int_by_name("hw_ifd_iadc_ldo");
	int ifd_qadc_ldo_data = rtk_otp_field_read_int_by_name("hw_ifd_qadc_ldo");

	if (ifd_iadc_ldo_data >= 0) {
	Iotp = (ifd_iadc_ldo_data >> 3) & 0x1; //check Iotp done flag

    if(Iotp==1){
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL7_reg, ~TV_SB1_ANA_IQADC_CTRL7_reg_iadc_ldo_sel_mask, ifd_iadc_ldo_data & 0x7);          //IADC_LDO_set
    }else{
			rtd_maskl(TV_SB1_ANA_IQADC_CTRL7_reg, ~TV_SB1_ANA_IQADC_CTRL7_reg_iadc_ldo_sel_mask, TV_SB1_ANA_IQADC_CTRL7_reg_iadc_ldo_sel(5));   //IADC_LDO_set default
    }
	}
	if (ifd_qadc_ldo_data >= 0) {
	Qotp = (ifd_qadc_ldo_data >> 3) & 0x1; //check Iotp done flag

    if(Qotp==1){
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL7_reg, ~TV_SB1_ANA_IQADC_CTRL7_reg_qadc_ldo_sel_mask, (ifd_qadc_ldo_data & 0x7) << 4);          //QADC_LDO_set
    }else{
			rtd_maskl(TV_SB1_ANA_IQADC_CTRL7_reg, ~TV_SB1_ANA_IQADC_CTRL7_reg_qadc_ldo_sel_mask, TV_SB1_ANA_IQADC_CTRL7_reg_qadc_ldo_sel(5));   //QADC_LDO_set default
    }
	}
	RTK_DEMOD_DBG("IADC_LDO OTP Done = %d, QADC_LDO OTP Done = %d, IADC_LDO = 0x%x, QADC_LDO = 0x%x\n", Iotp, Qotp, ifd_iadc_ldo_data & 0x7, ifd_qadc_ldo_data & 0x7);

	return FUNCTION_SUCCESS;
}


int QVcmSetting(void)
{
	int q_vcm_sel_ft_done = rtk_otp_field_read_int_by_name("hw_q_vcm_sel_ft_done");
	int	q_vcm_sel_ft      = rtk_otp_field_read_int_by_name("hw_q_vcm_sel_ft");

	if (q_vcm_sel_ft >= 0) {
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL5_reg, ~TV_SB1_ANA_IQADC_CTRL5_reg_q_vcm_sel_mask, TV_SB1_ANA_IQADC_CTRL5_reg_q_vcm_sel(q_vcm_sel_ft));
	}
    
	RTK_DEMOD_INFO("Q_VCM OTP Done = %d, Q_VCM_SEL = %d\n", q_vcm_sel_ft_done, q_vcm_sel_ft);
	return FUNCTION_SUCCESS;
}


int DtvDemodAdcPower(unsigned char on)
{
	return _TRUE;
}


int DtvDemodAdcPowerCheck(void)
{
	int ret = _TRUE;

	if ((rtd_inl(TV_SB1_ANA_IQADC_CTRL3_reg) & 0x1) != 0x1) {
		RTK_DEMOD_INFO("ADC Power Fail!!!!!!!!!\n");
		ret = _FALSE;
	}

	return ret;

}

int DtvIAdcIPgaPowerSetting(unsigned char On)
{
	CRT_CLK_OnOff(TVADC, CLK_ON, NULL);
	if (On) {
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL3_reg, ~TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_adc_mask,  TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_adc(1));
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL3_reg, ~TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_pga_mask,  TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_pga(1));
		RTK_DEMOD_INFO("DTV I_ADC & I_PGA Power On \n");
	} else {
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL3_reg, ~TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_adc_mask,  TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_adc(0));
		rtd_maskl(TV_SB1_ANA_IQADC_CTRL3_reg, ~TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_pga_mask,  TV_SB1_ANA_IQADC_CTRL3_reg_pow_i_pga(0));
		RTK_DEMOD_INFO("DTV I_ADC & I_PGA Power Off \n");
	}

	return FUNCTION_SUCCESS;
}


/*************************************************
* 1.CRT_IFADC : On                               *
* 2.IFD MBIAS / PLL_LDO_POW /PLL_POW_LDO11V : On *
* 3.ADC/PGA Power : Off                          *
*************************************************/
void IfdMbiasPowAndAdcPow(void)
{
	//Check IFADC CLK turn on
	CRT_CLK_OnOff(TVADC, CLK_ON, NULL);
	RTK_DEMOD_INFO("IFADC_CLK Enable!!\n");

	//MBIAS and PLL_LDO Power turn on
	rtd_maskl(0xb8033100, 0xfffeffff, 0x00010000);  //<bit16>REG_POW_MBIAS turn on (Must be frist turn on)
	tv_osal_usleep(50);
	rtd_maskl(0xb80004d4, 0xffffbfff, 0x00004000);  			//<bit14>PLL_POW_LDO11V turn on
	tv_osal_usleep(50);

	//turn Off ADC/PGA power
	rtd_maskl(0xb8033104, 0xfeffffff, 0x00000000);	//IQADC input cm power off <bit24> = 0
	rtd_outl(0xb8033124, 0x00000000);				//PGA/ADC Power Off
	tv_osal_usleep(50);

}


void DemodPllDefault(void)
{
	//past from script

	//Merlin5 register default setup
	rtd_outl(0xb80004c4, 0x00000000);				//DTV PLLDIF PLLDIF_DIVDIG_SEL = 000(2), PLLDIF_DIVADC_SEL = 00(divide by 8)

	//rtd_outl(0xb80004d0,0x0044000E);				//PLL27X, LFP Setting, M<14>+2=16(VCO=432M)
	//rtd_outl(0xb80004d4,0x00006021);				//<bit24:21>VDD11_PAD_SEL = 0x0
	//<bit5:4>PLL27X_PS_54M_DELAY=0x2(VCO=432M),<bit0>PLL27X_PS_DUTY_SEL =0x1(larger duty cycle)
	//rtd_outl(0xb80004d8,0x00003004);				//Default setting, <bit15:8>PLL_RESERVE=0x30, <bit2>PLL27X_VCORSTB=0x1(normal), <bit1>PLL27X_RSTB=0x0(Active Reset), <bit0>PLL27X_POW=0x0(Off).
	//rtd_outl(0xb80004dc,0x40010001);				//Default ATV mode, <bit30:29>PLL27X_IFADC_DUTY=0x2,<bit17:16>PLL27X_CK108M_DIV=0x1(DIV=4;VCO=432M) PLL27X setting,<bit0>PLL_CK_IFADC_MUX=0x1(ATV mode)

	rtd_outl(0xb8000514, 0x00000009);				//<bit3>PLL27x 108M clk to digital=0x1(enable), <bit0>write bit used
	rtd_outl(0xb8000500, 0x0000041c);				//<bit11:10>PLL27X_IFADC_CK108M_DIV=0x1(VCO=432M),<bit4>PLL27X_IFADC_DTV_CKO_EN=0x1(enable),<bit3:2>PLL27X_IFADC_ATV_CKO_SEL=0x3(135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV))
	rtd_outl(0xb8000504, 0x000e0104);				//PLL27X_IFADC VCO M,N, M<14>+2=16(VCO=432M), <bit9:6>PLL27X_IFADC_LF_RS=0x4
	rtd_outl(0xb8000508, 0x00000000);				//PLL27X_IFADC Poweroff and VCORSTB reset

	tv_osal_usleep(10);

	//PLL27X
	//rtd_maskl(0xb80004d8, 0xfffffff8, 0x00000004);	//<bit2>PLL27X_VCORSTB=0x1(normal), <bit1>PLL27X_RSTB=0x0(Active Reset), <bit0>PLL27X_POW=0x0(Off).
	//rtd_maskl(0xb80004dc, 0xfdfffffe, 0x02000000);	//Set PLL_L2H_CML_POW<bit25> = enable(0x1) and MUX to DTV Mode<bit0>=0x0

	//PLL27X_IFADC
	//rtd_maskl(0xb8000500, 0xfffffff3, 0x0000000C);	//<bit3:2>PLL27X_IFADC_ATV_CKO_SEL = 11 (135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV).
	//rtd_maskl(0xb8000508, 0xfffffff8, 0x00000007);	//IFADC, RSTB and VCORSTB Power on
}


void DemodPllSetting(RTK_DEMOD_MODE mode)
{


	//<23:20>PLL27X_LF_RS=0x5, <18:16>PLL27X_IP=0x5
	//rtd_maskl(0xb80004d0, 0xfff8ffff, 0x00050000); //PLL27X_IP
	//rtd_maskl(0xb80004d0, 0xff0fffff, 0x00500000); //PLL27X_LF_RS

	switch (mode) {
	case RTK_DEMOD_MODE_DTMB:
		//**PLL setting for DTMB_486M_121.5M Setting**//			//VCO and DIV setting : ADC CLK and System CLK setting(ADC=121.5M, Sys=121.5MM)
		rtd_maskl(0xb8000214, 0xFFFFB7FF, 0x00000800); 				//(B14,B11) = 01, Demod Sel = HDIC Demod(DTMB)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)
		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1
		//VCO = M<6:0>+2 = 16+2, N:00=1, 27 /1 *27=486MHz, PLL27X_IFADC_M<bit22:16> = 0x10(16), PLL27X_IFADC_N<bit25:24>=0x0(1),
		//<9:6>PLL27X_IFADC_LF_RS=0101, PLL27X_IFADC PUMP current<bit2:0>=0x5(0x101:15uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x00100000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x00000140); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000005); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000

		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000004); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 0100(/4), Adc_clk = 486 / 4 =121.5MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x00000080); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 1000(/4), sys_clk = 486 / 4 = 121.5 MHz
		//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit14>DIF432M_DIV=0x0(div2), <bit13>DIF432M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit10>LDPC_EN=0x0, <7:6>LDPC_DIV=0x0(div1.75), <bit2>PLL27X_IFADC_REF_SEL=0(XTAL)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00000000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00000000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000000); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x00000000); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL
		RTK_DEMOD_INFO("DemodPllSetting Pll Change to DTMB \n");
		break;


	case RTK_DEMOD_MODE_ATSC:
		//ADC CLK and System CLK setting(ADC=54M, Sys=432M)
		rtd_maskl(0xb8000214, 0xFFFFB7FF, 0x00004000); 				//(B14,B11) = 10, Demod Sel = RTK Demod(ATSC/DVBC/ISDBT/DTMB)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)

		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1

		//VCO = M<6:0>+2 = 14+2, N:00=1, 27 /1 *16=432MHz, PLL27X_IFADC_M<bit22:16> = 0xE(14), PLL27X_IFADC_N<bit25:24>=0x0(1),
		//<9:6>PLL27X_IFADC_LF_RS=0101, PLL27X_IFADC PUMP current<bit2:0>=0x5(101:15uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x000e0000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x00000140); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xffffffcf, 0x00000000); //PLL27X_IFADC_LF_CP
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000005); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000

		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000000); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 0000 = 0x0(/8), Adc_clk = 432 / 8=54MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x000000a0); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 1010 = 0xA(/1), sys_clk = 432 / 1 = 432MHz
		//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit14>DIF432M_DIV=0x0(div2), <bit13>DIF432M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit10>LDPC_EN=0x0, <7:6>LDPC_DIV=0x0(div1.75), <bit2>PLL27X_IFADC_REF_SEL=0(XTAL)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00000000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00000000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000000); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x00000000); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL
		RTK_DEMOD_INFO("DemodPllSetting to ATSC\n");
		break;

	case RTK_DEMOD_MODE_QAM:
		//ADC CLK and System CLK setting(ADC=25.2M, Sys=302.4M)
		rtd_maskl(0xb8000214, 0xFFFFB7FF, 0x00004000); 				//(B14,B11) = 10, Demod Sel = RTK Demod(ATSC/DVBC/ISDBT/DTMB)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)

		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1
		//VCO = M<6:0>+2 = 26+2, N:00=1, 27 /1 *28=756MHz, PLL27X_IFADC_M<bit22:16> = 0x1A(26), PLL27X_IFADC_N<bit25:24>=0x0(1),
		//<9:6>PLL27X_IFADC_LF_RS=1000, PLL27X_IFADC PUMP current<bit2:0>=0x4(100:12.5uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x001a0000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x00000200); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000004); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000

		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000008); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 1000 = 0x8(/30), Adc_clk = 756 / 30=25.2MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x00000010); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 0001 = 0x1(/2.5), sys_clk = 756 / 2.5 = 302.4MHz
		//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit14>DIF432M_DIV=0x0(div2), <bit13>DIF432M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit10>LDPC_EN=0x0, <7:6>LDPC_DIV=0x0(div1.75), <bit2>PLL27X_IFADC_REF_SEL=0(XTAL)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00000000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00000000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000000); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x00000000); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL
		RTK_DEMOD_INFO("DemodPllSetting to J83B\n");
		break;

	case RTK_DEMOD_MODE_DVBC:
		//ADC CLK and System CLK setting(ADC=36M, Sys=432M)
		rtd_maskl(0xb8000214, 0xFFFFB7FF, 0x00004000); 				//(B14,B11) = 10, Demod Sel = RTK Demod(ATSC/DVBC/ISDBT)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)

		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1

		//VCO = M<6:0>+2 = 18+2, N:00=1, 27 /1 *20=540MHz, PLL27X_IFADC_M<bit22:16> = 0x12(18), PLL27X_IFADC_N<bit25:24>=0x0(1),
		//<9:6>PLL27X_IFADC_LF_RS=0101, PLL27X_IFADC PUMP current<bit2:0>=0x5(101:15uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x00120000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x00000140); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000005); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000

		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000001); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 0001 = 0x1(/15), Adc_clk = 540 / 15=36MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x00000030); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 0011 = 0x3(/1.25), sys_clk = 864 / 1.25 = 432MHz
		//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit14>DIF432M_DIV=0x0(div2), <bit13>DIF432M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit10>LDPC_EN=0x0, <7:6>LDPC_DIV=0x0(div1.75), <bit2>PLL27X_IFADC_REF_SEL=0(XTAL)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00000000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00000000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000000); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x00000000); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL
		RTK_DEMOD_INFO("DemodPllSetting to DVBC\n");
		break;

	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
	case RTK_DEMOD_MODE_ISDB:
		//ADC CLK and System CLK setting(ADC=144M, Sys=518.4M)
		rtd_maskl(0xb8000214, 0xFFFDB7FF, 0x00020000);				//(B14,B11) = 00, Demod Sel = ATB Demod(DVB-T/T2/S/S2), <bit17>Dtv_demod_atb_clk_sel=0x1(1: ldpc from system clk)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)

		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1
		//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1),
		//<9:6>PLL27X_IFADC_LF_RS=0101, PLL27X_IFADC PUMP current<bit2:0>=0x5(101:15uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x00160000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x00000140); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xffffffcf, 0x00000000); //PLL27X_IFADC_LF_CP
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000005); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xffff00ff, 0x00003000); //<15:8>PLL_RESERVE=00110000

		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000007); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 0x7(/4.5)(33/67)), Adc_clk = 648 / 4.5 =144MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x00000030); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 0x3(/1.25)(40/60), sys_clk = 648 / 1.25 = 518.4 MHz

		//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit14>DIF432M_DIV=0x1(div3), <bit13>DIF432M_EN=0x1, <bit12>DTVDIG_EN=0x1,
		//<bit10>LDPC_EN=0x1, <7:6>LDPC_DIV=0x1(div1.25), <bit2>PLL27X_IFADC_REF_SEL=0(XTAL)
		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00004000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00002000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000400); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x00000040); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL
		RTK_DEMOD_INFO("DemodPllSetting to DVBT DVBT2 \n");
		break;

	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		//ADC CLK and System CLK setting(ADC=121.5M, Sys=486M)
		rtd_maskl(0xb8000214, 0xFFFDB7FF, 0x00020000);				//(B14,B11) = 00, Demod Sel = ATB Demod(DVB-T/T2/S/S2), <bit17>Dtv_demod_atb_clk_sel=0x1(1: ldpc from system clk)
		rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000000);				//PLL_L2H_CML_POW <bit25> = 1(on), PLL_CK_IFADC_MUX to DTV Mode, <bit0> = 0(DTV)

		rtd_maskl(0xb8000500, 0xFFFFFFEF, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1
		//VCO = M<6:0>+2 = 25+2, N:00=1, 27 /1 *27=729MHz, PLL27X_IFADC_M<bit22:16> = 0x19(25), PLL27X_IFADC_N<bit25:24>=0x0(1), <9:6>PLL27X_IFADC_LF_RS=0111, PLL27X_IFADC PUMP current<bit2:0>=0x4(100:12.5uF)
		rtd_maskl(0xb8000504, 0xff80ffff, 0x00190000); //PLL27X_IFADC_M
		rtd_maskl(0xb8000504, 0xfcffffff, 0x00000000); //PLL27X_IFADC_N
		rtd_maskl(0xb8000504, 0xfffffc3f, 0x000001c0); //PLL27X_IFADC_LF_RS
		rtd_maskl(0xb8000504, 0xffffffcf, 0x00000000); //PLL27X_IFADC_LF_CP
		rtd_maskl(0xb8000504, 0xfffffff8, 0x00000004); //PLL27X_IFADC_IP

		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000
		rtd_maskl(0xb80004c4, 0xfffffff0, 0x00000009); //ADC_CLK : PLLDIF_DIVADC_SEL<3:0> = 0x9(/6)(33/67)), Adc_clk = 729 / 6 =121.5MHz
		rtd_maskl(0xb80004c4, 0xffffff0f, 0x00000040); //System_CLK : PLLDIF_DIVDIG_SEL<bit7:4> = 0x4(/1.5)(50/50), sys_clk = 729 / 1.5 = 486 MHz

		rtd_maskl(0xb80004b8, 0xfffdffff, 0x00020000); //ATVADCDIV_RSTB
		rtd_maskl(0xb80004b8, 0xffffbfff, 0x00000000); //DIF432M_DIV
		rtd_maskl(0xb80004b8, 0xffffdfff, 0x00000000); //DIF432M_EN
		rtd_maskl(0xb80004b8, 0xffffefff, 0x00001000); //DTVDIG_EN: default
		rtd_maskl(0xb80004b8, 0xfffffbff, 0x00000400); //LDPC_EN
		rtd_maskl(0xb80004b8, 0xffffff3f, 0x000000c0); //LDPC_DIV
		rtd_maskl(0xb80004b8, 0xfffffffb, 0x00000000); //PLL27X_IFADC_REF_SEL: default
		RTK_DEMOD_INFO("DemodPllSetting to DVBSx\n");
		break;

	case RTK_DEMOD_MODE_ATV:
		//ATVIN_CLK_SEL, ATV Input clock select
		rtd_maskl(0xb8000204, 0xFFFCFFFF, 0x00000000);				//[17]ATVIN_CLK_SEL, [16]ATVIN_VD_CLK_SEL
		//ATV System CLK=108MHz, VCO=432MHz
		//rtd_outl(0xb80004d0, 0x0055000e);               			//VCO = M<6:0>14+2 = 16, N:00=1, 27 /1 *16=432MHz,[9:8]PLL27X_N = 0(1), [6:0]PLL27X_M = 0xe(14), <23:20>PLL27X_LF_RS=0x5, <18:16>PLL27X_IP=0x5
		//rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000
		//rtd_outl(0xb80004d4, 0x00036021);							//[18]PLL_XIXO_LOAD=0, [17:16]PLL_XIXO_DRIVE_SEL=0x3,[5:4]PLL27X_PS_54M_DELAY=0x2(VCO=432M)
		rtd_outl(0xb80004dc, 0x42010001);           				//[30:29]PLL27X_IFADC_DUTY=0x02(vco/4), [25]PLL_L2H_CML_POW=0x1, [17:16]PLL27X_CK108M_DIV=0x1(vco=432M),
		//[9]PLL27X_IFADC_DIV=0x0(divide 4), [0]PLL_CK_IFADC_MUX=0x1(ATV mode)
		rtd_outl(0xb8000514, 0x0000000b);							//[3]PLL27x 108M clk to digital=0x1, [1]PLL27X_PS_EN=0x1(PLL27x phase swallow enable), [0]write_bit(1 to set, 0 to clear)
		//rtd_maskl(0xb80004dc, 0xFDFFFFFE, 0x02000001);				//[25]PLL_L2H_CML_POW=0x1, [0]PLL_CK_IFADC_MUX=0x1(ATV mode)
		//rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000

		//PLL27X Power On
		//rtd_maskl(0xb80004d8, 0xFFFFFFF8, 0x00000000);   			//PLL27X_VCORSTB[2], 1:normal, 0:reset; PLL27X_RSTB[1], 1:normal, 0:reset; PLL27X_POW[0], 0:power down, 1:power on
		//rtd_maskl(0xb80004d8, 0xFFFFFFF8, 0x00000006);				//[2]PLL27X_VCORSTB = 0x1, [1]PLL27X_RSTB = 0x1, Reset release
		//rtd_maskl(0xb80004d8, 0xFFFFFFF8, 0x00000007);				//[0]PLL27X_POW =0x1, turn on PLL27X_POW

		//ATV ADC CLK=162MHz, VCO=648MHz, VCO and DIV setting//
		rtd_maskl(0xb8000500, 0xFFFFF3E3, 0x00000c1c);				//[11:10]PLL27X_IFADC_CK108M_DIV=0x3(VCO=648M),[4]PLL27X_IFADC_DTV_CKO_EN=0x1(enable),[3:2]PLL27X_IFADC_ATV_CKO_SEL=0x11(162M)
		rtd_maskl(0xb8000504, 0xFC80FC38, 0x00160145);				//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1), <9:6>PLL27X_IFADC_LF_RS=0101, <bit2:0>PLL27X_IFADC_IP=0x5(101:15uF)
		rtd_maskl(0xb80004d8, 0xFFFF00FF, 0x00003000);				//<15:8>PLL_RESERVE=00110000
		rtd_outl(0xb80004b8, 0x00024200);
		RTK_DEMOD_INFO("DemodPllSetting to ATV \n");
		break;

	default:
		RTK_DEMOD_INFO("DemodPllSetting wrong mode =%d \n", mode);
		break;
	}

	tv_osal_usleep(50);
	rtd_maskl(0xb8000508, 0xfffffff8, 0x00000000);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 0(Reset); <0>PLL27X_IFADC_POW turn off
	rtd_maskl(0xb8000508, 0xfffffffe, 0x00000001);  			//<0>PLL27X_IFADC_POW turn On
	rtd_maskl(0xb8000508, 0xfffffff9, 0x00000006);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 1(release)
	tv_osal_msleep(5);                                           //delay 10mS
}


int DtvDemodPllSettingCheck(RTK_DEMOD_MODE mode)
{
	int ret = _TRUE;

	switch (mode) {
	case RTK_DEMOD_MODE_DTMB:
	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
	case RTK_DEMOD_MODE_DVBC:
	case RTK_DEMOD_MODE_ATSC:
	case RTK_DEMOD_MODE_QAM:
	case RTK_DEMOD_MODE_ISDB:
	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		if ((rtd_inl(0xb80004dc) & 0x1) != 0x0) {
			RTK_DEMOD_INFO("DtvDemodPllSettingCheck Fail!!!!!!!!!\n");
			ret = _FALSE;
		}
		break;
	default:
		ret = _TRUE;
		break;
	}

	return ret;
}


int DtvDemodCRTSettingCheck(void)
{
	int ret = _TRUE;

	return ret;
}


void DtvDemodFrontendFilter(RTK_DEMOD_MODE mode)
{
//MERLIN5

	if (mode == RTK_DEMOD_MODE_DTMB) {
		RTK_DEMOD_INFO("Frontend Filter Change : load DTMB !\n");
		// down sample 4 filter setting
		rtd_outl(0xb8033900, 0x00001FFF);//tap0&1
		rtd_outl(0xb8033904, 0x1FFF0000);//tap2&3
		rtd_outl(0xb8033908, 0x00010002);//tap4&5
		rtd_outl(0xb803390C, 0x00020000);//tap6&7
		rtd_outl(0xb8033910, 0x1FFC1FFA);//tap8&9
		rtd_outl(0xb8033914, 0x1FFB0000);//tap10&11
		rtd_outl(0xb8033918, 0x0008000E);//tap12&13
		rtd_outl(0xb803391C, 0x000C0000);//tap14&15
		rtd_outl(0xb8033920, 0x1FEE1FE3);//tap16&17
		rtd_outl(0xb8033924, 0x1FE80000);//tap18&19
		rtd_outl(0xb8033928, 0x00220037);//tap20&21
		rtd_outl(0xb803392C, 0x002C1FFF);//tap22&23
		rtd_outl(0xb8033930, 0x1FC41FA0);//tap24&25
		rtd_outl(0xb8033934, 0x1FB30001);//tap26&27
		rtd_outl(0xb8033938, 0x0064009F);//tap28&29
		rtd_outl(0xb803393C, 0x007F1FFF);//tap30&31
		rtd_outl(0xb8033940, 0x1F5C1EFB);//tap32&33
		rtd_outl(0xb8033944, 0x1F300001);//tap34&35
		rtd_outl(0xb8033948, 0x010E01B3);//tap36&37
		rtd_outl(0xb803394C, 0x01611FFF);//tap38&39
		rtd_outl(0xb8033950, 0x1E1D1CD1);//tap40&41
		rtd_outl(0xb8033954, 0x1D400001);//tap42&43
		rtd_outl(0xb8033958, 0x04BB0A1D);//tap44&45
		rtd_outl(0xb803395C, 0x0E600FFF);//tap46&47
	} else if ((mode == RTK_DEMOD_MODE_DVBT) || (mode == RTK_DEMOD_MODE_DVBT2) || (mode == RTK_DEMOD_MODE_ISDB)) {
		RTK_DEMOD_INFO("Frontend Filter Change : load T/T2 !\n");
		//Set new down sample filter
		rtd_outl(0xb8033900, 0x1ffe1ffc);
		rtd_outl(0xb8033904, 0x00011fff);
		rtd_outl(0xb8033908, 0x00060003);
		rtd_outl(0xb803390c, 0x00070007);
		rtd_outl(0xb8033910, 0x00020006);
		rtd_outl(0xb8033914, 0x1ff81ffd);
		rtd_outl(0xb8033918, 0x1ff31ff4);
		rtd_outl(0xb803391c, 0x1ffb1ff5);
		rtd_outl(0xb8033920, 0x000d0004);
		rtd_outl(0xb8033924, 0x00170015);
		rtd_outl(0xb8033928, 0x000A0014);
		rtd_outl(0xb803392c, 0x1fed1ffc);
		rtd_outl(0xb8033930, 0x1fda1fe0);
		rtd_outl(0xb8033934, 0x1fec1fde);
		rtd_outl(0xb8033938, 0x001a0002);
		rtd_outl(0xb803393c, 0x003a002f);
		rtd_outl(0xb8033940, 0x00230037);
		rtd_outl(0xb8033944, 0x1fde0003);
		rtd_outl(0xb8033948, 0x1faa1fbd);
		rtd_outl(0xb803394c, 0x1fc51fac);
		rtd_outl(0xb8033950, 0x002a1ff3);
		rtd_outl(0xb8033954, 0x007e005d);
		rtd_outl(0xb8033958, 0x005f0080);
		rtd_outl(0xb803395c, 0x1fce001f);
		rtd_outl(0xb8033960, 0x1f4a1f80);
		rtd_outl(0xb8033964, 0x1f661f3e);
		rtd_outl(0xb8033968, 0x00381fbf);
		rtd_outl(0xb803396c, 0x011200b4);
		rtd_outl(0xb8033970, 0x01040133);
		rtd_outl(0xb8033974, 0x1fc30082);
		rtd_outl(0xb8033978, 0x1e351eed);
		rtd_outl(0xb803397c, 0x1dff1dd6);
		rtd_outl(0xb8033980, 0x00401ecd);
		rtd_outl(0xb8033984, 0x0480023b);
		rtd_outl(0xb8033988, 0x08ad06c2);
		rtd_outl(0xb803398c, 0x0a6909f5);
	}
}


int DemodFrontendPathSetting(RTK_DEMOD_MODE mode)
{

	switch (mode) {
	case RTK_DEMOD_MODE_DTMB:
		rtd_outl(0xb8033800, 0x10010010);							//<bit29:28>dtmb_data_valid_delay=0x1(delay data valid), <bit18:16>Dtv_frontend_mode=0x1, <bit8>dtv_idata_sel=0x0(from dn flt), <bit4>Dtv_dnflt_en=0x1(DN6)
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00040400);				//input fifo clr, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x1, FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x0, FIFO waddr & raddr clear
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000040);               //GPIO_61, Set pinmux DEMOD_IF_AGC
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_DTMB);
		RTK_DEMOD_INFO("FrontendPathSetting to DTMB\n");
		break;

	case RTK_DEMOD_MODE_ATSC:
		rtd_outl(0xb8033800, 0x10040000);							//<bit29:28>dtmb_data_valid_delay=0x1(delay data valid), <bit18:16>Dtv_frontend_mode=0x4, <bit8>dtv_idata_sel, <bit4>Dtv_dnflt_en for RTK Demod don't care
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00040400);				//input fifo clr, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x1, FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x0, FIFO waddr & raddr clear
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000040);               //GPIO_61, Set pinmux DEMOD_IF_AGC
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_ATSC);
		RTK_DEMOD_INFO("FrontendPathSetting to ATSC\n");
		break;

	case RTK_DEMOD_MODE_QAM:
		rtd_outl(0xb8033800, 0x10240000);							//<bit29:28>dtmb_data_valid_delay=0x1(delay data valid),<bit21>rtk_dtv_demod_sel=0x1(OpenCable), <bit18:16>Dtv_frontend_mode=0x4, <bit8>dtv_idata_sel, <bit4>Dtv_dnflt_en for RTK Demod don't care
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00040400);				//input fifo clr, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x1, FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x0, FIFO waddr & raddr clear
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000040);               //GPIO_61, Set pinmux DEMOD_IF_AGC
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_QAM);
		RTK_DEMOD_INFO("FrontendPathSetting to J83B\n");
		break;

	case RTK_DEMOD_MODE_DVBC:
		rtd_outl(0xb8033800, 0x10240000);							//<bit29:28>dtmb_data_valid_delay=0x1(delay data valid),<bit21>rtk_dtv_demod_sel=0x1(OpenCable), <bit18:16>Dtv_frontend_mode=0x4, <bit8>dtv_idata_sel, <bit4>Dtv_dnflt_en for RTK Demod don't care
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00040400);				//input fifo clr, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x1, FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x0, FIFO waddr & raddr clear
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000040);               //GPIO_61, Set pinmux DEMOD_IF_AGC
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_DVBC);
		RTK_DEMOD_INFO("FrontendPathSetting to DVBC\n");
		break;

	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
	case RTK_DEMOD_MODE_ISDB:
		rtd_outl(0xb8033800, 0x10010010);							//<bit29:28>dtmb_data_valid_delay=0x1(delay data valid), <bit18:16>Dtv_frontend_mode=0x1, <bit8>dtv_idata_sel=0x0(from dn flt), <bit4>Dtv_dnflt_en=0x1(DN6)
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00040400);				//input fifo clr, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x1, FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFBFBFF, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, iq_adc_fifo<bit10> = 0x0, FIFO waddr & raddr clear
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000040);           ///GPIO_61, Set pinmux DEMOD_IF_AGC
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_DVBT2);
		RTK_DEMOD_INFO("FrontendPathSetting to DVBT DVBT2\n");
		break;

	case RTK_DEMOD_MODE_DVBS:
	case RTK_DEMOD_MODE_DVBS2:
	case RTK_DEMOD_MODE_DVBS2X:
	case RTK_DEMOD_MODE_SPEC_AN:
		if (TunerInputMode <= 1) {
			rtd_outl(0xb8033800, 0x10020100);               //IQ Normal for Tuner Output
			RTK_DEMOD_INFO("DVBS spectrum normal\n");
		} else {
			rtd_outl(0xb8033800, 0x10020102);               //IQ Inversion for Tuner Output
			RTK_DEMOD_INFO("DVBS spectrum inversion\n");
		}
		rtd_maskl(0xb8033804, 0xFFFFFBFF, 0x00000400);	//input fifo clr, dtv_input_fifo_clr<bit10> = 0x1, Iadc Input FIFO waddr & raddr clear
		rtd_maskl(0xb8033804, 0xFFFFFBFF, 0x00000000);	//input fifo clr, dtv_input_fifo_clr<bit10> = 0x1, Iadc Input FIFO waddr & raddr clear
		rtd_maskl(0xb8000840, 0x0FFFFFFF, 0xA0000000);	//<31:28>gpio_19_ps, Set pinmux = 0xA(For DEMOD_IF_AGC)		
		DtvDemodFrontendFilter(RTK_DEMOD_MODE_DVBS);
		RTK_DEMOD_INFO("FrontendPathSetting to DVBSx\n");
		break;

	case RTK_DEMOD_MODE_ATV:
		RTK_DEMOD_INFO("FrontendPathSetting to ATV\n");
		rtd_maskl(0xb8000854, 0xFFFFFF0F, 0x00000030); 				//GPIO_61, Set pinmux ATV_IF_AGC
		break;

	default:
		RTK_DEMOD_INFO("FrontendPathSetting wrong mode =%d \n", mode);
		break;
	}

	return FUNCTION_SUCCESS;
}


/*------------------------------------------------------------------------------
 * FUNC : DTV demod memory mapping
 *------------------------------------------------------------------------------*/
/*
int DtvDemodMemRemapping(RTK_DEMOD_MODE mode)
{

	size_t DdrSize = 0;
	unsigned int AllocateSize = 0 ;

#if DYNAMIC_MEM_ALLOCATE
	unsigned char* nonCachedAddr;
#endif

	switch (mode) {
	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
		DdrSize = RTK_A_DVBT2_DDR_SIZE;
		RTK_DEMOD_INFO("DtvDemodMemRemapping (DVBTX) DDR_SIZE= 0x"PT_HEX_DUADDRESS"\n", DdrSize);
		break;
	case RTK_DEMOD_MODE_DTMB:
		DdrSize = RTK_H_DTMB_DDR_SIZE;
		RTK_DEMOD_INFO("DtvDemodMemRemapping (DTMB) DDR_SIZE= 0x"PT_HEX_DUADDRESS"\n", DdrSize);
		break;

	default:
		RTK_DEMOD_WARNING("TV MODE not support at DtvDemodMemRemapping! TV_MODE = %d \n", mode);
		//return FUNCTION_ERROR; for coverity
		break;

	}

	if (DdrSize) {


#if DYNAMIC_MEM_ALLOCATE

		if (pShareMemory)
			pli_freeContinuousMemoryMesg("DTV_DEMOD", pShareMemory);


		pShareMemory = (unsigned char*)pli_allocContinuousMemoryMesg(
						   "DTV_DEMOD",
						   DdrSize,
						   &nonCachedAddr,
						   &ShareMemoryPhysicalAddress);

		if (pShareMemory == NULL) {
			RTK_DEMOD_WARNING("DtvDemodMemRemapping failed, allocate share memory failed\n");
			return FUNCTION_ERROR;
		}


		RTK_DEMOD_INFO("allocate share memory(dynamic) - catch_addr=%p/%08lx, size=%08x\n",
					   pShareMemory,
					   ShareMemoryPhysicalAddress,
					   DdrSize);


#else

		AllocateSize = CravedoutMemoryQuery(&ShareMemoryPhysicalAddress);

		if (AllocateSize < DdrSize)
			RTK_DEMOD_WARNING("allocate share memory(fix) - allocate DDR size is not enough!!!\n");

		RTK_DEMOD_INFO("allocate share memory(fix) - Physical_addr=0x%08lx, size=0x%08x\n", ShareMemoryPhysicalAddress, AllocateSize);
#endif




	}


	switch (mode) {
	case RTK_DEMOD_MODE_DVBT:
	case RTK_DEMOD_MODE_DVBT2:
		rtd_outl(DTV_DEMOD_MISC_atb_wrapper_addr_offset_reg, ShareMemoryPhysicalAddress);
		break;
	case RTK_DEMOD_MODE_DTMB:
		rtd_outl(HDIC_REGISTER_BUS_hdic_mem_bus_ctrl4_reg, ShareMemoryPhysicalAddress);
		break;

	default:
		RTK_DEMOD_WARNING("TV MODE not support at DtvDemodMemRemapping! TV_MODE = %d \n", mode);
		return FUNCTION_ERROR;
		break;

	}


	return FUNCTION_SUCCESS;
}
*/


int DtvDemodTPOutEn(unsigned char enable)
{
	rtd_maskl(DTV_DEMOD_MISC_DEMOD_TP_OUT_EN_reg, ~DTV_DEMOD_MISC_DEMOD_TP_OUT_EN_demod_tp_out_enable_mask, DTV_DEMOD_MISC_DEMOD_TP_OUT_EN_demod_tp_out_enable(enable));
	RTK_DEMOD_INFO("DtvRDemod TP Output enable = %u !!!\n", enable);
	return FUNCTION_SUCCESS;
}


int DtvDemodInitialization(RTK_DEMOD_MODE mode, int force_rst)
{

	if (IsAtsc3McmPackage(RTK_DEMOD_MODE_ATSC3) == 1)
		ExtAtsc3PowOffOn(0);
	IfdMbiasPowAndAdcPow();                     //20181219 retune initial flow
	TvFrontendPowerControl(0);                    //Turn off TVALL
	DemodPllDefault();
	DemodPllSetting(mode);
	tv_osal_usleep(50);
	DemodAdcSetting(mode);
	//DemodAdcCapcalSetting(mode);
	tv_osal_usleep(50);
	rtd_maskl(TVSB5_TV_SB5_DCU1_ARB_CR1_reg, ~(0x00000001), 0x1);//TVSB5 do process command from cliend(demod)
	tv_osal_msleep(1);
	TvFrontendPowerControl(1);                    //Turn on TVFrontend
	tv_osal_usleep(50);

	//Frontend Path Setting
	DemodFrontendPathSetting(mode);


	DtvDemodEnable(mode);                       //Turn on DTVDemod
	//tv_osal_msleep(10);
	tv_osal_usleep(50);

	//RHAL_SetTPSource_By_Demod(0, 2, 0); //TP Path for Internal_Demod
	//RHAL_SetTPSource(0, 0, 0);

	Atsc3RtkExtDemodFlag = 0;

	DtvDemodReset = _BIT(mode);

	if (!DtvIsDemodClockEnable(mode)) {
		RTK_DEMOD_WARNING("init demod with demod clock disabled, please enable demod clock first\n");
	}

	m_realtek_TvSys_mode = mode;
	return CheckDemodReset(mode);
}

int ExtAtsc3DemodInitialization(RTK_DEMOD_MODE mode, int force_rst)
{

	if (IsAtsc3McmPackage(RTK_DEMOD_MODE_ATSC3) == 0) {
		RTK_DEMOD_INFO("ExtAtsc3DemodInitialization Error !! This is not MCM package!!\n");
		return FUNCTION_ERROR;
	}

	if ((mode != RTK_DEMOD_MODE_ATSC) && (mode != RTK_DEMOD_MODE_ATSC3)) {
		RTK_DEMOD_INFO("External MCM demod only can support ATSC and ASTC3 !! mode = %u\n", (unsigned int)mode);
		return FUNCTION_ERROR;
	}



	ExtAtsc3PowOffOn(0);

	TvFrontendPowerControl(0); //pull reset
	DtvIAdcIPgaPowerSetting(0);//turn off I_ADC, IPGA power
	tv_osal_usleep(10);

#if 0
//for ATS

	IfdMbiasPowAndAdcPow();                     //20181219 retune initial flow
	TvFrontendPowerControl(0);                    //Turn off TVALL
	DemodPllDefault();
	DemodPllSetting(RTK_DEMOD_MODE_ATSC);
	tv_osal_usleep(50);
	IQAdcLdoSetting();                          //add IQADC_LDO setting from OTP setting
	DemodAdcSetting(RTK_DEMOD_MODE_ATSC);
	tv_osal_usleep(50);
	rtd_maskl(TVSB5_TV_SB5_DCU1_ARB_CR1_reg, ~(0x00000001), 0x1);//TVSB5 do process command from cliend(demod)
	tv_osal_msleep(1);
	TvFrontendPowerControl(1);                    //Turn on TVFrontend
	tv_osal_usleep(50);

	//Frontend Path Setting
	DemodFrontendPathSetting(RTK_DEMOD_MODE_ATSC);
#endif

	ExtAtsc3PowOffOn(1);
	tv_osal_msleep(10);

	//RHAL_SetTPSource_By_Demod(0, 3, 0); //TP Path for External_Demod0 (MCM)
	//RHAL_SetTPSource(0, 0, 0);

	DtvDemodReset = _BIT(mode);
	m_realtek_TvSys_mode = mode;
	Atsc3RtkExtDemodFlag = 1;

	return CheckDemodReset(mode);
}



int GetDemodTvSysMode(TV_SYSTEM_TYPE * mode)
{
	if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DTMB)
		*mode = TV_SYS_TYPE_DTMB;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBT)
		*mode = TV_SYS_TYPE_DVBT;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBT2)
		*mode = TV_SYS_TYPE_DVBT2;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBC)
		*mode = TV_SYS_TYPE_DVBC;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_ATSC)
		*mode = TV_SYS_TYPE_ATSC;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_QAM)
		*mode = TV_SYS_TYPE_OPENCABLE;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_ISDB)
		*mode = TV_SYS_TYPE_ISDBT;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBS)
		*mode = TV_SYS_TYPE_DVBS;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBS2)
		*mode = TV_SYS_TYPE_DVBS2;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_DVBS2X)
		*mode = TV_SYS_TYPE_DVBS2;
	else if (m_realtek_TvSys_mode == RTK_DEMOD_MODE_SPEC_AN)
		*mode = TV_SYS_TYPE_UNKNOWN;

	return FUNCTION_SUCCESS;
}


int CheckDemodReset(RTK_DEMOD_MODE mode)
{
//	assert(mode < RTK_DEMOD_MODE_MAX);
	if (DtvDemodReset & _BIT(mode)) {
		RTK_DEMOD_INFO("CheckDemodReset: Demod %d has been reset (%x)\n", mode, DtvDemodReset);
		return 1;
	}
	return 0;
}


void ClearDemodReset(RTK_DEMOD_MODE mode)
{
	if (CheckDemodReset(mode)) {
		DtvDemodReset &= ~_BIT(mode);
		RTK_DEMOD_INFO("ClearDemodReset: reset flag of Demod %d has been cleared (%x)\n", mode, DtvDemodReset);
	}
}


int DemodTsOutClkCtrl(unsigned int TsClkKHz)
{
	//tpo_clk(MHz) = 250 / (pre + post + 2)
	//pre + post + 2 = 250 / tpo_clk(MHz)
	//pre + post + 2 = 250 *1000 / tpo_clk(KHz)
	//pre:0~511, post: 0~511
	unsigned int pre_div = 0, post_div = 0, result = 0;
	result = TPO_DIVIDEND_CONSTANT / TsClkKHz;
	if (result < TPO_DVISOR_CONSTANT)
		result = TPO_DVISOR_CONSTANT;
	result = result - TPO_DVISOR_CONSTANT;

	pre_div = (result + 1) / 2;
	if (pre_div > 511)
		pre_div = 511;

	post_div = result - pre_div;
	if (post_div > 511)
		post_div = 511;

	RHAL_TPOUT_SetClockSpeed(0, pre_div, post_div, 0);

	return FUNCTION_SUCCESS;
}



/*------------------------------------------------------------------------------
 * Analog TV API
 *------------------------------------------------------------------------------*/

int AtvDemodPowerControl(unsigned char On)
{
	if (On) {
		CRT_CLK_OnOff(ATVDEMOD, CLK_ON, NULL);
		RTK_DEMOD_INFO("Analog demod is power on.\n");
	} else {
		CRT_CLK_OnOff(ATVDEMOD, CLK_OFF, NULL);
		RTK_DEMOD_INFO("Analog demod is power off.\n");
	}

	return FUNCTION_SUCCESS;
}

int AtvDemodInitialization(RTK_DEMOD_MODE mode)
{
	RTK_DEMOD_INFO("AtvDemodInitialization : mode %d\n", mode);
	if (IsAtsc3McmPackage(RTK_DEMOD_MODE_ATSC3) == 1)
		ExtAtsc3PowOffOn(0);
	IfdMbiasPowAndAdcPow();                     //20181219 retune initial flow
	TvFrontendPowerControl(0);                    //Turn off TVALL
	DemodPllDefault();
	DemodPllSetting(mode);
	tv_osal_usleep(50);
	DemodAdcSetting(mode);
	//DemodAdcCapcalSetting(mode);
	tv_osal_usleep(50);
	TvFrontendPowerControl(1);                    //Turn on TVFrontend
	AtvDemodPowerControl(1);                    //Turn on ATVDemod
	tv_osal_usleep(50);
	DemodFrontendPathSetting(mode);
	Atsc3RtkExtDemodFlag = 0;

	return FUNCTION_SUCCESS;
}


/*------------------------------------------------------------------------------
 * DMA function
 *------------------------------------------------------------------------------*/

unsigned char* DemodDmaAddr;
unsigned char* DemodDmaNonCacheAddr;
U32BITS DemodDmaStartAddr, DemodDmaEndAddr;

int DtvDemodDMAAllocateMemory(void)
{
#if 0
	U32BITS phyAddr;
	//unsigned char* nonCachedAddr;

	if (DemodDmaAddr != 0)
		pli_freeContinuousMemoryMesg("DEMOD DMA", DemodDmaAddr);

	DemodDmaAddr = (unsigned char*)pli_allocContinuousMemoryMesg("DEMOD DMA", 0x800000, &DemodDmaNonCacheAddr, &phyAddr);
	DemodDmaStartAddr = phyAddr;
	DemodDmaEndAddr = phyAddr + 0x800000;
	RTK_DEMOD_INFO("DemodDmaAddr=%x, nonCachedAddr=%x, phyAddr=%x\n", DemodDmaAddr, DemodDmaNonCacheAddr, phyAddr);
	RTK_DEMOD_INFO("DemodDmaStartAddr=%x, DemodDmaEndAddr=%x\n", DemodDmaStartAddr, DemodDmaEndAddr);

	if (DemodDmaAddr == 0) return FUNCTION_ERROR;
	return FUNCTION_SUCCESS;
#endif
// ?? need to do 20150819
	return FUNCTION_ERROR;

}
int DtvDemodDMADump(BASE_INTERFACE_MODULE* pBaseInterface, unsigned char DeviceAddr, unsigned char mode)
{
#if 0
	char sFilePath[128];
	unsigned int iSize = 0;//, Width = 0, Height = 0;
	unsigned char *iStartAddr = 0;
	int iFileNum;
#ifdef CONFIG_DTV_DEMOD_USE_RBUSREG
	unsigned char i;
	U32BITS WritingAddr;

	WritingAddr = 0xE429;
	WritingAddr = WritingAddr * 4 + 0xb8040000;
	for (i = 0; i < 4; i++) {
		rtd_outl(WritingAddr + i * 4, (DemodDmaEndAddr >> (i * 8 + 3)) & 0xff);
	}
	WritingAddr = 0xE42D;
	WritingAddr = WritingAddr * 4 + 0xb8040000;
	for (i = 0; i < 4; i++) {
		rtd_outl(WritingAddr + i * 4, (DemodDmaStartAddr >> (i * 8 + 3)) & 0xff);
	}
	WritingAddr = 0xE438;
	WritingAddr = WritingAddr * 4 + 0xb8040000;
	if (mode == 0) //Dump ADC
		rtd_outl(WritingAddr, 0);
	else
		rtd_outl(WritingAddr, 1);

	//Enable DMA
	WritingAddr = 0xE432;
	WritingAddr = WritingAddr * 4 + 0xb8040000;
	rtd_outl(WritingAddr, 1);
#else
	unsigned char WritingBytes[10], WritingNum = 0;

	WritingBytes[0] = 0xE4;
	WritingBytes[1] = 0x29;
	WritingBytes[2] = (DemodDmaEndAddr >> 3) & 0xff;
	WritingBytes[3] = (DemodDmaEndAddr >> 11) & 0xff;
	WritingBytes[4] = (DemodDmaEndAddr >> 19) & 0xff;
	WritingBytes[5] = (DemodDmaEndAddr >> 27) & 0xff;
	WritingNum = 6;
	__realtek_i2c_write(pBaseInterface, DeviceAddr, &WritingBytes[0], WritingNum);
	WritingBytes[0] = 0xE4;
	WritingBytes[1] = 0x2D;
	WritingBytes[2] = (DemodDmaStartAddr >> 3) & 0xff;
	WritingBytes[3] = (DemodDmaStartAddr >> 11) & 0xff;
	WritingBytes[4] = (DemodDmaStartAddr >> 19) & 0xff;
	WritingBytes[5] = (DemodDmaStartAddr >> 27) & 0xff;
	WritingNum = 6;
	__realtek_i2c_write(pBaseInterface, DeviceAddr, &WritingBytes[0], WritingNum);

	if (mode == 0) { //Dump ADC
		WritingBytes[0] = 0xE4;
		WritingBytes[1] = 0x38;
		WritingBytes[2] = 0x0;
		WritingNum = 3;
	} else { //Dump DAGC
		WritingBytes[0] = 0xE4;
		WritingBytes[1] = 0x38;
		WritingBytes[2] = 0x1;
		WritingNum = 3;
	}
	__realtek_i2c_write(pBaseInterface, DeviceAddr, &WritingBytes[0], WritingNum);

	WritingBytes[0] = 0xE4;
	WritingBytes[1] = 0x32;
	WritingBytes[2] = 0x1; //Enable DMA
	WritingNum = 3;
	__realtek_i2c_write(pBaseInterface, DeviceAddr, &WritingBytes[0], WritingNum);

#endif

	iStartAddr = DemodDmaNonCacheAddr;
	iSize = 0x800000;

	system("mount -t vfat -o remount /dev/sda1 /mnt/usbmounts/sda1");
	sprintf(sFilePath, "%s/%s%s", "/mnt/usbmounts/sda1/", "DEMOD_DMA", ".bin"); //Dump to USB

	RTK_DEMOD_INFO("%s\n", sFilePath);

	FILE *pFile = fopen(sFilePath, "w");
	if (pFile == NULL) {
		RTK_DEMOD_INFO("file open fail...\n");
	} else {
		iFileNum = fileno(pFile);
	}

	if (pFile) {
		unsigned int iCnt = 0, iBufIdx = 0;
		unsigned int *iCurAddr = (unsigned int*)iStartAddr;
		unsigned int bufArray[128];
		unsigned int iBuf = 0;

		while (iCnt < iSize) {
			RTK_DEMOD_INFO(".");
			iBuf = *iCurAddr;
			bufArray[iBufIdx] = iBuf;
			iBufIdx ++;

			if (iBufIdx >= 128) {
				fwrite(bufArray, 128 * 4, 1, pFile);
				iBufIdx = 0;
			}

			iCnt += 4;
			iCurAddr += 1; //move pointer 4byte
		}
		RTK_DEMOD_INFO("\n");
		// last
		if (iBufIdx > 0) {
			fwrite(bufArray, iBufIdx * 4, 1, pFile);
			iBufIdx = 0;
		}
		RTK_DEMOD_INFO(".\n");
		if (0 != fflush(pFile))
			RTK_DEMOD_INFO("Flash to file fail\n");

		// Sync to device

		if (0 != fsync(iFileNum)) {
			RTK_DEMOD_INFO("fsync fail\n");
			if (fdatasync(iFileNum) == -1) {
				RTK_DEMOD_INFO("fdatasync error!!\n");
			}
		}

		fclose(pFile);
		RTK_DEMOD_INFO("write to file %s, size = %d\n", sFilePath, iSize);
	} else {
		RTK_DEMOD_INFO("write file Error!\n");
	}
#ifdef CONFIG_DTV_DEMOD_USE_RBUSREG
	//Disable DMA

	WritingAddr = 0xE432;
	WritingAddr = WritingAddr * 4 + 0xb8040000;
	rtd_outl(WritingAddr, 0);
#else
	WritingBytes[0] = 0xE4;
	WritingBytes[1] = 0x32;
	WritingBytes[2] = 0x0; //Disable DMA
	WritingNum = 3;
	__realtek_i2c_write(pBaseInterface, DeviceAddr, &WritingBytes[0], WritingNum);
#endif
#endif
	return FUNCTION_SUCCESS;

}


/*------------------------------------------------------------------------------
 * Base Interface API
 *------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 * FUNC : __realtek_wait_ms
 *
 * DESC : wait function for merlin2 baseinterface
 *
 * PARM : pBIF  : Private Data that wants contain in this base class
 *        ms    : time to wait in ms
 *
 * RET  : N/A
 *------------------------------------------------------------------------------*/
void __realtek_wait_ms(BASE_INTERFACE_MODULE* pBIF, unsigned long ms)
{
	tv_osal_msleep(ms);
}



/*------------------------------------------------------------------------------
 * FUNC : __rtd299s_i2c_write
 *
 * DESC : i2c write function for rtd299s base if
 *
 * PARM : pBaseInterface    : handle of rtd299s baseif
 *        DeviceAddr        : device address
 *        pWritingBytes     : data to be written
 *        ByteNum           : number of data to write
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
)
{
	COMM* pComm;
	I2cFastSpeedFlag = 0;
	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);
	return ENCODE_RET(pComm->SendWriteCommand(pComm, DeviceAddr, (unsigned char*) pWritingBytes, ByteNum, I2C_M_HIGH_SPEED));
}



/*------------------------------------------------------------------------------
 * FUNC : __rtd299s_i2c_read
 *
 * DESC : i2c read function for rtd299s base if
 *
 * PARM : pBaseInterface    : handle of rtd299s baseif
 *        DeviceAddr        : device address
 *        pReadingBytes     : read data buffer
 *        ByteNum           : number of data to read
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
)
{

	COMM* pComm ;
	I2cFastSpeedFlag = 0;
	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);
	return ENCODE_RET(pComm->SendReadCommand(pComm, DeviceAddr, pRegisterAddr, RegisterAddrSize, pReadingBytes, ByteNum, I2C_M_HIGH_SPEED));
}


/*------------------------------------------------------------------------------
 * FUNC : __rtd299s_i2c_write
 *
 * DESC : i2c write function for rtd299s base if
 *
 * PARM : pBaseInterface    : handle of rtd299s baseif
 *        DeviceAddr        : device address
 *        pWritingBytes     : data to be written
 *        ByteNum           : number of data to write
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_i2c_write_speedF(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
)
{
	COMM* pComm;
	I2cFastSpeedFlag = 1;
	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);
	return ENCODE_RET(pComm->SendWriteCommand(pComm, DeviceAddr, (unsigned char*) pWritingBytes, ByteNum, I2C_M_FAST_SPEED));
}



/*------------------------------------------------------------------------------
 * FUNC : __rtd299s_i2c_read
 *
 * DESC : i2c read function for rtd299s base if
 *
 * PARM : pBaseInterface    : handle of rtd299s baseif
 *        DeviceAddr        : device address
 *        pReadingBytes     : read data buffer
 *        ByteNum           : number of data to read
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_i2c_read_speedF(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
)
{

	COMM* pComm ;
	I2cFastSpeedFlag = 1;
	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);
	return ENCODE_RET(pComm->SendReadCommand(pComm, DeviceAddr, pRegisterAddr, RegisterAddrSize, pReadingBytes, ByteNum, I2C_M_FAST_SPEED));
}




/*------------------------------------------------------------------------------
 * FUNC : __realtek_rbus_i2c_write
 *
 * DESC : Rbus write function for realtek base if
 *
 * PARM : pBaseInterface    : handle of realtek baseif
 *        DeviceAddr        : device address
 *        pWritingBytes     : data to be written
 *        ByteNum           : number of data to write
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_rbus_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
)
{

#if 0
	unsigned char i = 0;
	unsigned char RbusData;
	U32BITS DemodAddrMsb, DemodAddrLsb;
	U32BITS DemodAddr;

	DemodAddrMsb = pWritingBytes[0];
	DemodAddrLsb = pWritingBytes[1];
	DemodAddr = DemodAddrMsb * 256 + DemodAddrLsb;

	if (DemodAddr >= 0x3000 && DemodAddr <= 0x3fff) {
		//Demod 8051 RAM Address to Rbus Mapping: 0x3000~0x3FFF(RAM 4K) = 0xB814C000~0xB814FFFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8140000;
	} else {
		//Demod/8051 Controll Signal Address to Rbus Mapping: 0xD000~0xDFFF = 0xB8154000~B8157FFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8150000;
	}

	//printk("===(Write)DemodAddrMsb = 0x%x, DemodAddrLsb = 0x%x===\n", DemodAddrMsb, DemodAddrLsb);
	//printk("===(Write)RbusAddr = 0x%x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	if (RbusAddr == 0xb814c000) { //8051 SRAM Start: 0x3000
		rtd_maskl(RTK_DTV_DEMOD_8051_GP_INT0_N_reg, ~RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en_mask, RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en(0x1));//Merlin4/Mac6p
		//rtd_maskl(REG_RTK_DEMOD_8051_SRAM_WRITE_EN, 0xFFFFFFFE, 0x00000001);//Merlin3/Mac5p
		//printk("[8051_SRAM_S]0xB815C004 = 0x%x\n", rtd_inl(0xB815C004));
	}

	//If ByteNum<2, Only record RbusAddr.
	if (ByteNum > 2) {
		for (i = 2; i < ByteNum; i++) {
			RbusData = pWritingBytes[i];
			//printk("RbusAddr = 0x%8x, RbusData = 0x%x, 0xB815C004 = 0x%x\n", RbusAddr + (i - 2) * 4, RbusData, rtd_inl(0xB815C004));

			rtd_outl(RbusAddr + (i - 2) * 4, RbusData);
			tv_osal_usleep(10);
		}
	}

	if ((RbusAddr + (i - 3) * 4) == 0xb814effc) { //0x3BFF (only use RAM 3K (3072) for ATSC)
		rtd_maskl(RTK_DTV_DEMOD_8051_GP_INT0_N_reg, ~RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en_mask, RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en(0x0));//Merlin4/Mac6p
		//rtd_maskl(REG_RTK_DEMOD_8051_SRAM_WRITE_EN, 0xFFFFFFFE, 0x00000000);//Merlin3/Mac5p
		//printk("[8051_SRAM_E]i = %u, 0xB815C004 = 0x%x\n", i, rtd_inl(0xB815C004));
	}
	return FUNCTION_SUCCESS;

#else
	unsigned char i = 0;
	unsigned char RbusData;
	unsigned char Rbus4BytesData[4];
	unsigned int RbusAddrTmp = 0;
	unsigned char *pRbusRegAddrData;
	unsigned char RbusRegAddrDataSize = 8;
	unsigned char RbusRegAddr[4], RbusRegAddrSize = 4;
	unsigned char ReadingByteNum = 4;
	U32BITS DemodAddrMsb, DemodAddrLsb;
	U32BITS DemodAddr;
	COMM* pComm;
	int ret = FUNCTION_SUCCESS;
	U32BITS I2CFlags = I2C_M_HIGH_SPEED;   //400KHz


	if (I2cFastSpeedFlag == 1)
		I2CFlags = I2C_M_FAST_SPEED; ///1MHz
	else
		I2CFlags = I2C_M_HIGH_SPEED;  //400KHz


	if (ByteNum <= 2)
		return FUNCTION_ERROR;

	RbusRegAddrDataSize = ((ByteNum - 2) * 4) + 4;
	pRbusRegAddrData = (unsigned char*) kmalloc(RbusRegAddrDataSize, GFP_KERNEL);

	if (pRbusRegAddrData == NULL) {
		RTK_DEMOD_INFO("__realtek_rbus_i2c_write kmalloc Failed\n");
		kfree(pRbusRegAddrData);
		return FUNCTION_ERROR;
	}


	if (Atsc3RtkExtDemodFlag != 0)
		pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);


	DemodAddrMsb = pWritingBytes[0];
	DemodAddrLsb = pWritingBytes[1];
	DemodAddr = DemodAddrMsb * 256 + DemodAddrLsb;

	if (DemodAddr >= 0x3000 && DemodAddr <= 0x3fff) {
		//Demod 8051 RAM Address to Rbus Mapping: 0x3000~0x3FFF(RAM 4K) = 0xB814C000~0xB814FFFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8140000;
	} else {
		//Demod/8051 Controll Signal Address to Rbus Mapping: 0xD000~0xDFFF = 0xB8154000~B8157FFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8150000;
	}

	//printk("===(Write)DemodAddrMsb = 0x%x, DemodAddrLsb = 0x%x===\n", DemodAddrMsb, DemodAddrLsb);
	//printk("===(Write)RbusAddr = 0x%x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	if (RbusAddr == 0xb814c000) { //8051 SRAM Start: 0x3000
		if (Atsc3RtkExtDemodFlag == 0) {
			rtd_maskl(RTK_DTV_DEMOD_8051_GP_INT0_N_reg, ~RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en_mask, RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en(0x1));//Merlin4/Mac6p
			//rtd_maskl(REG_RTK_DEMOD_8051_SRAM_WRITE_EN, 0xFFFFFFFE, 0x00000001);//Merlin3/Mac5p
			//printk("[8051_SRAM_S]0xB815C004 = 0x%x\n", rtd_inl(0xB815C004));
		} else {

			RbusAddrTmp =  RTK_DTV_DEMOD_8051_GP_INT0_N;
			RbusRegAddr[0] = RbusAddrTmp & 0xff;
			RbusRegAddr[1] = (RbusAddrTmp >> 8) & 0xff;
			RbusRegAddr[2] = (RbusAddrTmp >> 16) & 0xff;
			RbusRegAddr[3] = (RbusAddrTmp >> 24) & 0x1f;


			ret |= ENCODE_RET(pComm->SendReadCommand(pComm, DeviceAddr, RbusRegAddr, RbusRegAddrSize, Rbus4BytesData, ReadingByteNum, I2CFlags));

			Rbus4BytesData[0] &=  0x7F;
			Rbus4BytesData[0] |= 0x80;


			RbusAddrTmp =  RTK_DTV_DEMOD_8051_GP_INT0_N;
			*(pRbusRegAddrData) = RbusAddrTmp & 0xff;
			*(pRbusRegAddrData + 1) = (RbusAddrTmp >> 8) & 0xff;
			*(pRbusRegAddrData + 2) = (RbusAddrTmp >> 16) & 0xff;
			*(pRbusRegAddrData + 3) = (RbusAddrTmp >> 24) & 0x1f;
			*(pRbusRegAddrData + 4) = Rbus4BytesData[0];
			*(pRbusRegAddrData + 5) = Rbus4BytesData[1];
			*(pRbusRegAddrData + 6) = Rbus4BytesData[2];
			*(pRbusRegAddrData + 7) = Rbus4BytesData[3];
			RbusRegAddrDataSize = 8;
			ret |= ENCODE_RET(pComm->SendWriteCommand(pComm, DeviceAddr, pRbusRegAddrData, RbusRegAddrDataSize, I2CFlags));

		}

	}

	if (Atsc3RtkExtDemodFlag == 0) {
		for (i = 2; i < ByteNum; i++) {
			RbusData = pWritingBytes[i];
			//printk("RbusAddr = 0x%8x, RbusData = 0x%x, 0xB815C004 = 0x%x\n", RbusAddr + (i - 2) * 4, RbusData, rtd_inl(0xB815C004));
			rtd_outl(RbusAddr + (i - 2) * 4, RbusData);
			tv_osal_usleep(10);
		}
	} else {

		RbusRegAddrDataSize = ((ByteNum - 2) * 4) + 4;
		memset(pRbusRegAddrData, 0, RbusRegAddrDataSize);

		*(pRbusRegAddrData) = RbusAddr & 0xff;
		*(pRbusRegAddrData + 1) = (RbusAddr >> 8) & 0xff;
		*(pRbusRegAddrData + 2) = (RbusAddr >> 16) & 0xff;
		*(pRbusRegAddrData + 3) = (RbusAddr >> 24) & 0x1f;

		for (i = 2; i < ByteNum; i++) {
			*(pRbusRegAddrData + ((i - 1) * 4)) = pWritingBytes[i];
		}

		ret |= ENCODE_RET(pComm->SendWriteCommand(pComm, DeviceAddr, pRbusRegAddrData, RbusRegAddrDataSize, I2CFlags));

	}


	if ((RbusAddr + (i - 3) * 4) == 0xb814effc) { //0x3BFF (only use RAM 3K (3072) for ATSC)
		if (Atsc3RtkExtDemodFlag == 0) {
			rtd_maskl(RTK_DTV_DEMOD_8051_GP_INT0_N_reg, ~RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en_mask, RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en(0x0));//Merlin4/Mac6p
			//rtd_maskl(REG_RTK_DEMOD_8051_SRAM_WRITE_EN, 0xFFFFFFFE, 0x00000000);//Merlin3/Mac5p
			//printk("[8051_SRAM_E]i = %u, 0xB815C004 = 0x%x\n", i, rtd_inl(0xB815C004));
		} else {

			RbusAddrTmp =  RTK_DTV_DEMOD_8051_GP_INT0_N;
			RbusRegAddr[0] = RbusAddrTmp & 0xff;
			RbusRegAddr[1] = (RbusAddrTmp >> 8) & 0xff;
			RbusRegAddr[2] = (RbusAddrTmp >> 16) & 0xff;
			RbusRegAddr[3] = (RbusAddrTmp >> 24) & 0x1f;

			ret |= ENCODE_RET(pComm->SendReadCommand(pComm, DeviceAddr, RbusRegAddr, RbusRegAddrSize, Rbus4BytesData, ReadingByteNum, I2CFlags));

			Rbus4BytesData[0] &=  0x7F;

			RbusAddrTmp =  RTK_DTV_DEMOD_8051_GP_INT0_N;
			*(pRbusRegAddrData) = RbusAddrTmp & 0xff;
			*(pRbusRegAddrData + 1) = (RbusAddrTmp >> 8) & 0xff;
			*(pRbusRegAddrData + 2) = (RbusAddrTmp >> 16) & 0xff;
			*(pRbusRegAddrData + 3) = (RbusAddrTmp >> 24) & 0x1f;
			*(pRbusRegAddrData + 4) = Rbus4BytesData[0];
			*(pRbusRegAddrData + 5) = Rbus4BytesData[1];
			*(pRbusRegAddrData + 6) = Rbus4BytesData[2];
			*(pRbusRegAddrData + 7) = Rbus4BytesData[3];
			RbusRegAddrDataSize = 8;
			ret |= ENCODE_RET(pComm->SendWriteCommand(pComm, DeviceAddr, pRbusRegAddrData, RbusRegAddrDataSize, I2CFlags));

		}
	}

	kfree(pRbusRegAddrData);
	return ret;

#endif


}



/*------------------------------------------------------------------------------
 * FUNC : __realtek_rbus_i2c_read
 *
 * DESC : Rbus read function for realtek base if
 *
 * PARM : pBaseInterface    : handle of realtek baseif
 *        DeviceAddr        : device address
 *        pReadingBytes     : read data buffer
 *        ByteNum           : number of data to read
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_rbus_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
)
{

#if 0
	unsigned char i;
	unsigned char RbusData;
	//printk("===(Read)RbusAddr = 0x%8x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	for (i = 0; i < ByteNum; i++) {
		RbusData = rtd_inl(RbusAddr + (i * 4));
		pReadingBytes[i] = RbusData;
		tv_osal_usleep(10);

		//printk("===(Read)pReadingBytes[%u] = 0x%8x\n", i,  RbusData);
	}
	return FUNCTION_SUCCESS;

#else

	unsigned char i = 0;
	unsigned char RbusData;
	unsigned char* pReadingData;
	unsigned char RbusRegAddr[4], RbusRegAddrSize = 4;
	unsigned char ReadingByteNum = 4;
	U32BITS DemodAddrMsb, DemodAddrLsb;
	U32BITS DemodAddr;
	COMM* pComm ;
	int ret = FUNCTION_SUCCESS;
	U32BITS I2CFlags = I2C_M_HIGH_SPEED;   //400KHz


	if (I2cFastSpeedFlag == 1)
		I2CFlags = I2C_M_FAST_SPEED;   ///1MHz
	else
		I2CFlags = I2C_M_HIGH_SPEED;  //400KHz


	DemodAddrMsb = pRegisterAddr[0];
	DemodAddrLsb = pRegisterAddr[1];
	DemodAddr = DemodAddrMsb * 256 + DemodAddrLsb;

	if (DemodAddr >= 0x3000 && DemodAddr <= 0x3fff) {
		//Demod 8051 RAM Address to Rbus Mapping: 0x3000~0x3FFF(RAM 4K) = 0xB814C000~0xB814FFFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8140000;
	} else {
		//Demod/8051 Controll Signal Address to Rbus Mapping: 0xD000~0xDFFF = 0xB8154000~B8157FFC
		RbusAddr = (DemodAddr & 0x00003fff) * 4 + 0xb8150000;
	}

	//printk("===(Write)DemodAddrMsb = 0x%x, DemodAddrLsb = 0x%x===\n", DemodAddrMsb, DemodAddrLsb);
	//printk("===(Write)RbusAddr = 0x%x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	if (Atsc3RtkExtDemodFlag == 0) {
		for (i = 0; i < ByteNum; i++) {
			RbusData = rtd_inl(RbusAddr + (i * 4)) & 0xff;
			pReadingBytes[i] = RbusData;
			//printk("===(Read)pReadingBytes[%u] = 0x%8x\n", i,  RbusData);
			tv_osal_usleep(10);
		}
	} else {

		ReadingByteNum = ByteNum * 4;
		pReadingData = (unsigned char*) kmalloc(ReadingByteNum, GFP_KERNEL);

		if (pReadingData == NULL) {
			RTK_DEMOD_INFO("__realtek_rbus_i2c_read kmalloc Failed\n");
			kfree(pReadingData);
			return FUNCTION_ERROR;
		}

		pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void**)&pComm);
		RbusRegAddr[0] = RbusAddr & 0xff;
		RbusRegAddr[1] = (RbusAddr >> 8) & 0xff;
		RbusRegAddr[2] = (RbusAddr >> 16) & 0xff;
		RbusRegAddr[3] = (RbusAddr >> 24) & 0x1f;

		ret = ENCODE_RET(pComm->SendReadCommand(pComm, DeviceAddr, RbusRegAddr, RbusRegAddrSize, pReadingData, ReadingByteNum, I2CFlags));
		//RTK_DEMOD_INFO("[RD]DeviceAddr=0x%x, RbusRegAddr=0x%02x%02x%02x%02x, Rbus4BytesData=0x%02x%02x%02x%02x, ret=%d \n", DeviceAddr, RbusRegAddr[3], RbusRegAddr[2], RbusRegAddr[1], RbusRegAddr[0],Rbus4BytesData[3], Rbus4BytesData[2], Rbus4BytesData[1], Rbus4BytesData[0],ret);

		for (i = 0; i < ByteNum; i++) {
			pReadingBytes[i] = *(pReadingData + (i * 4));
		}

	}

	if (Atsc3RtkExtDemodFlag != 0)
		kfree(pReadingData);
	return ret;

#endif
}

/*------------------------------------------------------------------------------
 * FUNC : __realtek_a_rbus_i2c_write
 *
 * DESC : Rbus write function for realtek base if
 *
 * PARM : pBaseInterface    : handle of realtek baseif
 *        DeviceAddr        : device address
 *        pWritingBytes     : data to be written
 *        ByteNum           : number of data to write
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/

int __realtek_a_rbus_i2c_write(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	const unsigned char*        pWritingBytes,
	U32BITS               ByteNum
)
{
	unsigned char RbusData = 0;
	unsigned char DemodBaseAddr, DemodOffsetAddr;

	if (ByteNum > 3) {
		RTK_DEMOD_WARNING("__realtek_a_rbus_i2c_write fail : do not support ByteNum = "PT_U32BITS" >3\n", ByteNum);
		return FUNCTION_ERROR;
	}

	RbusAddr = 0xb8160000;
	DemodBaseAddr = pWritingBytes[0];
	DemodOffsetAddr = pWritingBytes[1];

	if (DemodBaseAddr == 0x66)
		RbusAddr = RbusAddr | (0x30 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x67)
		RbusAddr = RbusAddr | (0x31 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x68)
		RbusAddr = RbusAddr | (0x32 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x69)
		RbusAddr = RbusAddr | (0x33 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x70)
		RbusAddr = RbusAddr | (0x34 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x71)
		RbusAddr = RbusAddr | (0x35 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x72)
		RbusAddr = RbusAddr | (0x36 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x88)
		RbusAddr = RbusAddr | (0x37 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x8a)
		RbusAddr = RbusAddr | (0x38 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else if (DemodBaseAddr == 0x95)
		RbusAddr = RbusAddr | (0x39 << 10) | ((DemodOffsetAddr & 0xff) << 2);
	else
		RbusAddr = RbusAddr | ((DemodBaseAddr & 0x3f) << 10) | ((DemodOffsetAddr & 0xff) << 2);


	//printk("===(Write)DemodAddrMsb = 0x%x, DemodAddrLsb = 0x%x===\n", DemodAddrMsb, DemodAddrLsb);
	//printk("===(Write)RbusAddr = 0x%x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	//If ByteNum<2, Only record RbusAddr.

	if (ByteNum == 3) {
		RbusData = pWritingBytes[2];
		rtd_outl(RbusAddr, RbusData);
		tv_osal_usleep(10);
	}



	return FUNCTION_SUCCESS;

}



/*------------------------------------------------------------------------------
 * FUNC : __realtek_rbus_i2c_read
 *
 * DESC : Rbus read function for realtek base if
 *
 * PARM : pBaseInterface    : handle of realtek baseif
 *        DeviceAddr        : device address
 *        pReadingBytes     : read data buffer
 *        ByteNum           : number of data to read
 *
 * RET  : FUNCTION_SUCCESS / FUNCTION_ERROR
 *------------------------------------------------------------------------------*/
int __realtek_a_rbus_i2c_read(
	BASE_INTERFACE_MODULE*      pBaseInterface,
	unsigned char               DeviceAddr,
	unsigned char* pRegisterAddr,
	unsigned char RegisterAddrSize,
	unsigned char*              pReadingBytes,
	U32BITS               ByteNum
)
{
	unsigned char RbusData;

	if (ByteNum > 1) {
		RTK_DEMOD_WARNING("__realtek_a_rbus_i2c_read fail : do not support ByteNum = "PT_U32BITS">1\n", ByteNum);
		return FUNCTION_ERROR;
	}


	//printk("===(Read)RbusAddr = 0x%8x, ByteNum = 0x%x===\n", RbusAddr, ByteNum);

	RbusData = rtd_inl(RbusAddr);
	*pReadingBytes = RbusData;
	tv_osal_usleep(10);

	//printk("===(Read)pReadingBytes[%u] = 0x%8x\n", i,  RbusData);

	return FUNCTION_SUCCESS;
}


int DtvDemodMemRemappingInt(size_t size, unsigned long *phyAddr, unsigned int *StaticAllocateSize)
{
#if DYNAMIC_MEM_ALLOCATE
	unsigned char* nonCachedAddr = NULL;
	RTK_DEMOD_INFO("\033[1;32;31m""Dynamic allocate memory\n""\033[m");

	pgShareMemory = (unsigned long*)pli_allocContinuousMemoryMesg(
											"DTV_DEMOD",
											size,
											&nonCachedAddr,
											phyAddr);
	if(pgShareMemory == NULL){
		RTK_DEMOD_WARNING("Dynamic allocate memory failed!\n");
		return FUNCTION_ERROR;
	}
	RTK_DEMOD_INFO("pli_allocContinuousMemoryMesg dynamic allocate share memory - addr=%p/%08lx, size=%08x\n",
		pgShareMemory,
		*phyAddr,
		size);
#else
	RTK_DEMOD_INFO("\033[1;32;31m""Static allocate memory\n""\033[m");

	*StaticAllocateSize = CravedoutMemoryQuery(phyAddr);
	if( *StaticAllocateSize < size){
		RTK_DEMOD_WARNING("Static allocate memory failed!\n");
		return FUNCTION_ERROR;
	}
	RTK_DEMOD_INFO("CravedoutMemoryQueryStatic allocate share memory(fix) - Physical_addr=%08lx, size=%08x\n", *phyAddr, *StaticAllocateSize);
#endif
	return FUNCTION_SUCCESS;
}


void DtvDemodFreeAllocMem(void *ptr)
{
#if DYNAMIC_MEM_ALLOCATE
	if(ptr != NULL){
		RTK_DEMOD_INFO("Free dynamic allocate memory\n");
		pli_freeContinuousMemoryMesg("DTV_DEMOD", ptr);
		pgShareMemory = NULL;
	}
#endif
}


void*   pli_allocContinuousMemoryMesg(char *str, size_t size, unsigned char** nonCachedAddr, unsigned long *phyAddr)
{

	void *CachedAddr_local = NULL;
	void *nonCachedAddr_local = NULL;
	DUADDRESS phyAddr_local = 0;

	if (size <= 0)
		return 0;

	size &= 0x0fffffff;


	CachedAddr_local = dvr_malloc_uncached(size, &nonCachedAddr_local);


	if (!CachedAddr_local) {
		RTK_DEMOD_WARNING("[PLI] allocation failure... Allocate size = "PT_DEC_DUADDRESS"\n", size);
		return 0;
	}


	phyAddr_local = dvr_to_phys(CachedAddr_local);

	RTK_DEMOD_INFO("realtek_demod pli_allocContinuousMemoryMesg size=0x"PT_HEX_DUADDRESS" ptr=0x%p nonCachedAddr_local=0x%p phyAddr_local=0x"PT_HEX_DUADDRESS" \n", size, CachedAddr_local, nonCachedAddr_local, phyAddr_local);

	if (nonCachedAddr)
		*nonCachedAddr = (unsigned char*)nonCachedAddr_local;
	if (phyAddr)
		*phyAddr = (unsigned long) phyAddr_local;

	return CachedAddr_local;

}



unsigned int CravedoutMemoryQuery(unsigned long *pPhyAddr)
{

	unsigned int AllocateSize = 0;

	//*pPhyAddr = DEMOD_MEM_START_PHYS;
	//AllocateSize = DEMOD_MEM_LENGTH;

	AllocateSize = (unsigned int)carvedout_buf_query(CARVEDOUT_DEMOD, (void *)pPhyAddr);
	return AllocateSize;

}



void    pli_freeContinuousMemoryMesg(char *str, void *ptr)
{
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^  ERROR   need to modify
	kfree(ptr);
}

unsigned int IsAtsc3McmPackage(RTK_DEMOD_MODE mode)
{

	//check OTP data for MCM package
	unsigned int Ans = 0;
	/*
	unsigned char IsAtsc3Mcm = 0;

	if (mode == RTK_DEMOD_MODE_ATSC3) {
		if (rtk_otp_field_read_by_name("hw_mcm_atsc3", &IsAtsc3Mcm, 1) == true) {
			Ans = IsAtsc3Mcm;
		}
	}

	*/
	return Ans;

}

