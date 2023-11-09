#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <rbus/stb_reg.h>
#include <rbus/emcu_reg.h>

#include "rtk_kdv_emcu.h"
#include <rtk_kdriver/rtk_emcu_export.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <asm/system_misc.h>
#include <asm/suspend.h>


#include <mach/platform.h>
#ifdef CONFIG_RTK_KDRV_MIO
#include <rbus/iso_spi_synchronizer_reg.h>
#endif
#include <rtk_kdriver/rtk-kdrv-common.h>

int g_rtk_load_8051 = 0;

#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
extern int ir_suspend_protocol(void);
extern int venus_ir_input_early_init(void);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_CEC)
extern int pm_rtd_cec_suspend(void);
#endif

extern platform_info_t platform_info;
extern int rtk_SetIOPinDirection(unsigned long long pininfo, unsigned int value);
extern int emcu_chk_event(void);
extern int rtk_emcu_smem_set_ir(char *ibuf);
extern void emcu_set_boot_cmd_irda(void);

/* general reg setting */

/* 8051 magic event */
#define MAGIC_8051                  0x2379beef  /* 8051 is runing but counting RTC only. setting by 8051 only.*/
#define WAKE_8051                   0x9021affe  /* send message to 8051,let 8051 wakeup and handle standby status.. */
#define EVENT1_8051                 0x9021affc  /* send event1 status to 8051 */
#define CHECK_8051                   0           /* set magic reg of 8051 to zero then check 8051 is runing. */

#define GET_MAGIC_PARM()            rtd_inl(REG_MAGIC_51)
#define SET_MAGIC_PARM(num)         rtd_outl(REG_MAGIC_51,num)

#define STM_NOEMAL      1
#define STM_WAKEUP      2

static void pm_load_st_gpio(void)
{
    unsigned long long value;
    char *buf, *token;
    buf = platform_info.pcb_st_enum_parameter;
    while(1)
    {
        token = strchr(buf,',');
        if (token){
            token = token + 1;
            value = simple_strtoull(token, 0, 0);
            rtk_SetIOPinDirection(value,0);
        }
        else
            break;

        buf = token;
    }
}

static void calculate_DDR_checksum(void)
{
	//SCPU text
	unsigned int *data_ptr = phys_to_virt(SCPU_TEXT_START);
	unsigned int *end_ptr =  phys_to_virt(SCPU_TEXT_END);
	unsigned int sumValue=0;
	while(data_ptr<end_ptr)
	{
		sumValue+=*data_ptr;
		data_ptr++;
	}
	rtd_pr_emcu_warn("DDR_checksum S(%x)\n", sumValue);

	//ACPU text
	data_ptr = phys_to_virt(ACPU_TEXT_START);
	end_ptr =  phys_to_virt(ACPU_TEXT_END);
	sumValue=0;
	while(data_ptr<end_ptr)
	{
		sumValue+=*data_ptr;
		data_ptr++;
	}
	rtd_pr_emcu_warn("DDR_checksum A(%x)\n", sumValue);

	//VCPU text
	data_ptr = phys_to_virt(VCPU_TEXT_STRT);
	end_ptr =  phys_to_virt(VCPU_TEXT_END);
	sumValue=0;
	while(data_ptr<end_ptr)
	{
		sumValue+=*data_ptr;
		data_ptr++;
	}
	rtd_pr_emcu_warn("DDR_checksum V(%x)\n", sumValue);
}


/** NOTICE,
 * wakup_8051 is modified as instead of timer shudown main area,
 * but wait for STANDBYWFIL2 for sure all SCPU activities idle.
 * An additional option will be set for emcu fw, to countdown this waiting.
 * - if timeout, SOC will reboot. (AC reboot)
 * - if option=0, means no reboot. (Keep platform for debugging.)
 **/
extern void  rtk_ledserial_restore_setting( void );
extern unsigned char GetUartRegBakup_inited( void);
extern int emcu_enter_poweroff(int mode);
void wakeup_8051(int flag)
{
    rtd_pr_emcu_info("@@!!!!!enter pm_wakeup_emcu\n");

    //rtd_maskl(PINMUX_ST_STB_TOP_1_reg, 0x0f0fffff, 0x40400000); // switch RTD285o uart0
    /* ================================================
     * refine enable EMCU core flow as DIC's recommend
     * CLKEN=1 > CLKEN=0 > RSTN=1 > CLKEN=1
     * ================================================*/
    if(new_protocol_enable == 1){
        emcu_enter_poweroff(0);
    }else{
        rtd_outl(REG_MAGIC_51, 0);
        rtd_outl(STB_ST_SRST1_reg, _BIT9);          // rst off

        if(flag == SUSPEND_RAM){
           rtd_outl(REG_MAGIC_51, WAKE_8051);
        }
        else{
            rtd_outl(REG_MAGIC_51, EVENT1_8051);
	    }

        rtd_outl(STB_ST_CLKEN1_reg, _BIT9 | _BIT0); // clk enable
        udelay(50);                                 // wait for reset ready
        rtd_outl(STB_ST_SRST1_reg, _BIT9 | _BIT0);  // rst on

        udelay(500);
    }
    #ifdef CONFIG_CUSTOMER_TV030
        if(GetUartRegBakup_inited())
            rtk_ledserial_restore_setting();
        rtd_outl(STB_ST_CLKMUX_reg, rtd_inl(STB_ST_CLKMUX_reg)|_BIT0);
    #endif
    rtd_pr_emcu_alert("%s(%u)ecpu reset release done\n",__func__,__LINE__);
}


void rtkpm_get_param(void){
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
    ir_suspend_protocol();
#ifndef CONFIG_CUSTOMER_TV030
    emcu_set_boot_cmd_irda();
#endif
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_CEC)
    if(new_protocol_enable == 0){
        cec_flag = CHK_CEC(GET_EVENT_MSG());
    }
    if(cec_flag) {
        rtd_pr_emcu_info("\n==>initial CEC suspend block.\n");
        pm_rtd_cec_suspend();
    }
#endif
}

#if defined (CONFIG_RTK_KDRV_EMCU_DCOFF_TIMEOUT)
static int emcu_dcoff_timeout = CONFIG_RTK_KDRV_EMCU_DCOFF_TIMEOUT;

void set_emcu_dcoff_timeout(void)
{
    rtd_outl(RTD_SHARE_MEM_BASE+(4*IDX_REG_DC_OFF_TIMEOUT), emcu_dcoff_timeout);
}

#else
#define set_emcu_dcoff_timeout() do {} while(0)
#endif //#if defined (RTK_KDRV_EMCU_DCOFF_TIMEOUT)

#ifdef CONFIG_TV030_PLATFORM
extern int ir_protocol;
int emcu_customer_setting(u32 ctl)
{
	/*
	This func sync with http://mm2.sdlc.rd.realtek.com/gerrit/#/c/88476/
	commit by Gene
	 */
    unsigned int share_tmp = rtd_inl(RTD_SHARE_MEM_BASE);
	unsigned int share_ir_tmp = rtd_inl(RTD_SHARE_MEM_BASE+4*(0xe));
	unsigned int share_led_tmp = rtd_inl(RTD_SHARE_MEM_BASE+4*(0x1c));
	rtd_pr_emcu_info("irda protocol=0x%x\n",ir_protocol);

	if((share_led_tmp == 0x0) || (share_led_tmp == 0xab054f00)) //(share_led_tmp == 0xf2a0d5))
	{
	    rtd_pr_emcu_info("long press power key,led cannot work when shutdown flow\n");
		rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x4c000001);
	}

	if((share_tmp == 0x0) || (share_ir_tmp == 0x0)) {
	    rtd_pr_emcu_alert("\n%s(%u)workaround for sharememory all NULL!!!\n",__func__,__LINE__);
        rtd_pr_emcu_info("pm_wake_ workaround for sharememory all NULL. warning!!!!!\n");
        if(ir_protocol == 8){
        rtd_outl(RTD_SHARE_MEM_BASE             ,0x2c0001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1), 0x10000a);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x2), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x3), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x4), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x5), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x6), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x7), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x8), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x9), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xa), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xb), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xc), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xd), 0x8);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xe), 0xab054f00);//0xf2a0d5);//power
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xf), 0x80f7f00);//0xfef010);//netflix
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x10), 0xf300cf00);//0xf300cf);//0
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x11), 0x7308cf00);//0xf310ce);//1
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x12), 0xb304cf00);//0xf320cd);//2
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x13), 0x330ccf00);//0xf330cc);//3
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x14), 0xd302cf00);//0xf340cb);//4
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x15), 0x530acf00);//0xf350ca);//5
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x16), 0x9306cf00);//0xf360c9);//6
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x17), 0x130ecf00);//0xf370c8);//7
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x18), 0xe301cf00);//0xf380c7);//8
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x19), 0x6309cf00);//0xf390c6);//9
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1a), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1b), 0x3);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x4c000001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1d), 0x401c04);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1e), 0x400804);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1f), 0x0);
        }
		else if(ir_protocol == 18){
		rtd_outl(RTD_SHARE_MEM_BASE             ,0x2c0001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1), 0x10000a);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x2), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x3), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x4), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x5), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x6), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x7), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x8), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x9), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xa), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xb), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xc), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xd), 0x12);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xe), 0x2ad5b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xf), 0xef10b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x10), 0xaf50b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x11), 0x31ceb916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x12), 0x32cdb916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x13), 0x33ccb916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x14), 0x34cbb916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x15), 0x35cab916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x16), 0x36c9b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x17), 0x37c8b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x18), 0x38c7b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x19), 0x39c6b916);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1a), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1b), 0x3);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x4c000001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1d), 0x401c04);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1e), 0x400804);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1f), 0x0);
		}
    }

    if(share_tmp == 0x10110000) {
	    rtd_pr_emcu_alert("\n%s(%u)DC off, share memory may not set correctly by ap!!!\n",__func__,__LINE__);
        rtd_pr_emcu_info("pm_wake_ DC share memory may not set correctly by ap, using DC off default. warning!!!!!\n");
#if 1 // original AP default value
        rtd_outl(RTD_SHARE_MEM_BASE             ,0x2c0001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1), 0x10000a);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x2), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x3), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x4), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x5), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x6), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x7), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x8), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x9), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xa), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xb), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xc), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xd), 0x8);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xe), 0xab054f00);//0xf2a0d5);//power
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xf), 0x80f7f00);//0xfef010);//netflix
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x10), 0xf300cf00);//0xf300cf);//0
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x11), 0x7308cf00);//0xf310ce);//1
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x12), 0xb304cf00);//0xf320cd);//2
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x13), 0x330ccf00);//0xf330cc);//3
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x14), 0xd302cf00);//0xf340cb);//4
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x15), 0x530acf00);//0xf350ca);//5
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x16), 0x9306cf00);//0xf360c9);//6
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x17), 0x130ecf00);//0xf370c8);//7
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x18), 0xe301cf00);//0xf380c7);//8
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x19), 0x6309cf00);//0xf390c6);//9
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1a), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1b), 0x3);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x4c000001);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1d), 0x401c04);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1e), 0x400804);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1f), 0x0);
#endif
    }

    if(share_tmp == 0x10000000) {
	    rtd_pr_emcu_alert("\n%s(%u)STR off, share memory may not set correctly by ap!!!\n",__func__,__LINE__);
        rtd_pr_emcu_info("pm_wake_ STR share memory may not set correctly by ap, using STR off default. warning!!!!!\n");
#if 1 // original AP default value
        rtd_outl(RTD_SHARE_MEM_BASE		, 0x101c0011);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1), 0x10000a);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x2), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x3), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x4), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x5), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x6), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x7), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x8), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x9), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xa), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xb), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xc), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xd), 0x8);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xe), 0xab054f00);//0xf2a0d5);//power
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xf), 0x80f7f00);//0xfef010);//netflix
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x10), 0xf300cf00);//0xf300cf);//0
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x11), 0x7308cf00);//0xf310ce);//1
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x12), 0xb304cf00);//0xf320cd);//2
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x13), 0x330ccf00);//0xf330cc);//3
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x14), 0xd302cf00);//0xf340cb);//4
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x15), 0x530acf00);//0xf350ca);//5
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x16), 0x9306cf00);//0xf360c9);//6
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x17), 0x130ecf00);//0xf370c8);//7
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x18), 0xe301cf00);//0xf380c7);//8
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x19), 0x6309cf00);//0xf390c6);//9
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1a), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1b), 0x3);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x80000000);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1d), 0x401c04);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1e), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1f), 0x0);
#else
        rtd_outl(RTD_SHARE_MEM_BASE		, 0x111c2011);   // 10sec wakeup
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1), 0x16000a);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x2), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x3), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x4), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x5), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x6), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x7), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x8), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x9), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xa), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xb), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xc), 0xa);	   // 10sec wakeup
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xd), 0x8);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xe), 0xf2a0d5);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0xf), 0xfef010);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x10), 0xf300cf);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x11), 0xf310ce);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x12), 0xf320cd);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x13), 0xf330cc);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x14), 0xf340cb);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x15), 0xf350ca);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x16), 0xf360c9);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x17), 0xf370c8);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x18), 0xf380c7);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x19), 0xf390c6);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1a), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1b), 0x3);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1c), 0x80000000);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1d), 0x400600);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1e), 0x0);
        rtd_outl(RTD_SHARE_MEM_BASE+4*(0x1f), 0x0);
#endif
    }
    return 0;
}

#else
int emcu_customer_setting(u32 ctl)
{
    /* for TV001 */
    return 0;
}
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_WATCHDOG)
extern int watchdog_enable (unsigned char On);
#endif
int rtk_pm_load_8051(int flag)
{
	int ret;

    rtd_pr_emcu_info("\n%s(%u)2013/07/09 11:30: %d, %pS\n",__func__,__LINE__, flag, __builtin_return_address(0));
    local_irq_disable();

    if(new_protocol_enable == 1){
#if 0  //just for debug,open uart1
        //pinmux
        rtd_maskl(0xb8060218, 0x0f0fffff, 0x60600000);
        rtd_maskl(0xb8060248, 0x0ffffff8, 0x00000002);	
#endif
    }
/*MA6PBU-1437 disable watchdog due to 8051 not tick watchdog*/
#if IS_ENABLED(CONFIG_RTK_KDRV_WATCHDOG)
        rtd_pr_emcu_info("Suspend action...MUST disable watchdog\n");
        watchdog_enable(0);
#endif

    if(flag == SUSPEND_BOOTCODE)
    {
        rtd_pr_emcu_info("bootcode call suspend.\n");
    }else if((flag == SUSPEND_RAM )||
             (flag == SUSPEND_WAKEUP))
    {
        unsigned int stm_value;
        unsigned int pm_event_msg;
        if(new_protocol_enable == 1){
            if(flag == SUSPEND_WAKEUP){
                stm_value = STM_WAKEUP;
            }else{
                //for after add rtc_wakeup, stm area only used BIT_28 & BIT_29
			    stm_value =	stm_flag&0x03;
                if(!stm_value)
                    stm_value = STM_NOEMAL;
            }
		    ret = send_packet(EMCU_STR, stm_value, NULL, NULL, 0, 0);
		    if(ret != EMCU_CMD_DONE){
                EMCU_ERR("STM set fialed=%d\n",ret);
		    }
		    stm_flag = stm_value;
        }else{
		    pm_event_msg = GET_EVENT_MSG();
		    if(flag == SUSPEND_WAKEUP){
			    stm_value = STM_WAKEUP;
		    }else{
		        //for after add rtc_wakeup, stm area only used BIT_28 & BIT_29
                stm_value = (pm_event_msg>>loc_stm) &0x03;
			    if(!stm_value)
        		    stm_value = STM_NOEMAL;
		    }
            //for after add rtc_wakeup, stm area only used BIT_28 & BIT_29
            pm_event_msg = (pm_event_msg&0xcfffffff)|(stm_value<<loc_stm);
            SET_EVENT_MSG(pm_event_msg);
            rtd_pr_emcu_info("Suspend to RAM action...(0x%08x)\n",pm_event_msg);
      }
    }else{
#if IS_ENABLED(CONFIG_RTK_KDRV_WATCHDOG)
    /*Jira MAC5P-2215, disable watchdog when enter emcu_on mode*/
    watchdog_enable(0);
#endif
        rtd_pr_emcu_info("system call suspend.(disable watchdog)\n");
    }

    if(flag != SUSPEND_WAKEUP)
    {
        pm_load_st_gpio();
    }
    rtd_pr_emcu_info("load bin file from DDR\n");
    rtkpm_get_param();

/* Calculate DDR checksum for make sure STR flow DDR data is not corrupt */
    if(((rtd_inl(STB_WDOG_DATA14_reg) & 0xFFFF) == 0xfafe) && (flag == SUSPEND_RAM))
        calculate_DDR_checksum();

    set_emcu_dcoff_timeout();

    /* The customer setting must set to function below please!!! */
    if(new_protocol_enable == 0){
        emcu_customer_setting(0);
    }

    if(emcu_chk_event())
        BUG_ON(1);
    else {
		if((GET_WK_SOR() & 0x000000ff) != 0x09) { // reset 8051 if not wov mode
			wakeup_8051(flag);
		}
		else {	// enter power off if wov mode
			if(rtd_inl(STB_WDOG_DATA9_reg) != 0x23797970) {
				rtd_pr_emcu_info("It should accept power off command when wov here\n");
				rtd_outl(STB_WDOG_DATA9_reg, 0x23797970);
			}
			rtd_pr_emcu_info("Enter Power off when wov mode\n");
		}
	}
#ifdef CONFIG_RTK_KDRV_MIO
    //MIO ignore the command from Main when power off in Main
    rtd_pr_emcu_info("MIO gnore the command from Main when power off in Main\n");
    //rtd_outl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg,rtd_inl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg)&(~(0x1 << ISO_SPI_SYNCHRONIZER_SPI_ctrl_main_iso_wfifo_en_shift)));
    // Disable MIO side auto update
    rtd_outl(ISO_SPI_SYNCHRONIZER_sd_mio_SPI_ctrl_reg, 0x00000010);
    udelay(50);
    rtd_outl(ISO_SPI_SYNCHRONIZER_SPI_ctrl_reg, 0x0011);
#endif
    return 0;
}
EXPORT_SYMBOL(rtk_pm_load_8051);

#if 1 //defined (CONFIG_TV051)
void ac_on_power_down_analog(void);

#else
void ac_on_power_down__analog(void)
{
}
#endif

#ifndef CONFIG_RTK_KDRV_EMCU_MODULE
static int rtk_pm_load_8051_code(char *s)
{
    if((strlen(s) < 1) || rtk_emcu_smem_set_ir(s+1)){
        rtd_pr_emcu_alert("\n%s(%u)There are no IR key setting!!!\n",__func__,__LINE__);
        //BUG_ON(1);
    }

    ac_on_power_down_analog();
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
    venus_ir_input_early_init();
#endif
    g_rtk_load_8051 = 1;
    return 0;
}
__setup("POWERDOWN",rtk_pm_load_8051_code);
#else
int rtk_pm_load_8051_code(void){
	char strings[32] = {0};
	char *p = strings;
	if(rtk_parse_commandline_equal("POWERDOWN", p, sizeof(strings))){
		if(rtk_emcu_smem_set_ir(p)){
			rtd_pr_emcu_alert("\n%s(%u)There are no IR key setting!!!\n",__func__,__LINE__);
		}
	}

	if(rtk_parse_commandline("POWERDOWN")){
		ac_on_power_down_analog();
		g_rtk_load_8051 = 1;
	}
	return 0;
}
EXPORT_SYMBOL(rtk_pm_load_8051_code);
#endif
/* end of file */
