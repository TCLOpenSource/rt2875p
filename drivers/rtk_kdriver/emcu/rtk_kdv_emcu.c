/*
 *  Copyright (C) 2017 Realtek
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#ifdef  CONFIG_CUSTOMER_TV030
#include <rbus/stb_reg.h>
#endif
#include <rbus/emcu_reg.h>
#include <mach/platform.h>
#include <mach/rtk_platform.h>
#include <mach/pcbMgr.h>
#include <rtk_kdriver/rtk_emcu_export.h>
#include <rtk_kdriver/rtk-kdrv-common.h>
#include "rtk_kdv_emcu.h"
#include "ir_input.h"
#include "irrc_common.h"
#include <rbus/stb_reg.h>
#include <linux/suspend.h>
#include <asm/suspend.h>
#include "rtk_kdv_emcu_isr.h"

/***************************************************************************
Information:
1. sample code porting from AP\system\src\Platform_Lib\miscMgr\wakeup.cpp
	GoogleTV/kernel/android/nougat/device/realtek/frameworks/native/libs/Platform_Lib/miscMgr/wakeup.cpp
2. Device node allocate at /dev/emcu_kdv
3. Device attribute allocate at /sys/devices/virtual/misc/emcu_kdv
4. To check pcb param on target over cmd "ls /sys/realtek_boards/pcb_enum"
5. There are two method to enable Auto Parse.
	One is modify config of kernel, another is key in command at bootcode.
	bootcode command is "env set mmc_param_1 emcu_auto=1;env save"


****************************************************************************/
/************************************************************************
 * External variable
 ************************************************************************/
extern int rtk_pm_load_8051(int flag);
extern int g_rtk_load_8051;

/************************************************************************
 * Local Function
 ************************************************************************/
static void emcu_show_sharemem(void);

/************************************************************************
 * Local definition
 ************************************************************************/
#ifdef CONFIG_EMCU_AUTO_PARSER
#define AUTO_PCB_PARAM
#endif

int new_protocol_enable=0;
static unsigned int irda_arry_ext[PARAM_LEN_IRDA+1] = {0}; //13 irda max num + 1 irda protocol
unsigned int irda_valid_num=0;
static unsigned int keypad_arry_ext[PARAM_LEN_KEYPAD] = {0};
unsigned int keypad_valid_num=0;
static unsigned int power_pin_arry_ext[PARAM_LEN_PWEN] = {0};
unsigned int power_pin_valid_num=0;
unsigned int stm_flag=0;
unsigned int cec_flag=0;

/* API for device driver to check if IP should keep alive while suspend */
/* for suspend event; emcu_suspend_event */
#define CEC_EVENT_SHT       0
#define PPS_EVENT_SHT       1
#define IRDA_EVENT_SHT      2
#define LSADC_EVENT_SHT     3
#define GPIO_EVENT_SHT      4
#define WOV_EVENT_SHT       5
#define NET_EVENT_SHT       6  //6,7,8
#define LED_FLASH_SHT       9

#define CEC_SSP_EVENT       BIT(CEC_EVENT_SHT)
#define PPS_SSP_EVENT       BIT(PPS_EVENT_SHT)
#define IRDA_SSP_EVENT      BIT(IRDA_EVENT_SHT)
#define LSADC_SSP_EVENT     BIT(LSADC_EVENT_SHT)
#define GPIO_SSP_EVENT      BIT(GPIO_EVENT_SHT)
#define WOV_SSP_EVENT       BIT(WOV_EVENT_SHT)
#define WOL_SSP_EVENT       BIT(NET_EVENT_SHT+0)
#define WOW_SSP_EVENT       BIT(NET_EVENT_SHT+1)
#define WIFI_SSP_EVENT      BIT(NET_EVENT_SHT+2)
#define LED_FLASH_EVENT     BIT(LED_FLASH_SHT)
/**********************************************/

#define PARSE_USER			0
#define PARSE_AUTO			1
#define PARSE_IN_CASE			2

#define EMCU_IRDA_ALERT			BIT(1)
#define EMCU_POWER_EN_ALERT		BIT(2)
#define EMCU_KEYPAD_ALERT		BIT(3)
#define EMCU_WOWLAN_ALERT		BIT(4)
#define EMCU_WIFI_DG_ALERT		BIT(5)

#define PR_TYPE_HEX     (16)
#define PR_TYPE_DEC     (10)
#define PR_TYPE_PCB     (0xff)

#define EMCU_KEYPAD_PWR_PIN_NAME    "PIN_KEYPAD_PWR"

#ifdef  CONFIG_CUSTOMER_TV030
#define REG_MAGIC_51                0xb8060110
#endif
#ifdef CONFIG_MULTI_IR_KEY
unsigned int reno_gpi_cnt;
#endif

/************************************************************************
 * Local variable
 ************************************************************************/
static unsigned int emcu_suspend_event;
static unsigned int emcu_auto_event;
static unsigned int emcu_test_ctl;
static unsigned int local_sharemem_sup[32] = {0};
static unsigned int local_sharemem_res[32] = {0};
static unsigned char emcu_ac_on_alt = 0;
static unsigned int pw_arry_ext[PARAM_LEN_PWEN] = {0};

#define EMCU_PARSE_EN   BIT(0)
#define EMCU_CHK_SKIP   BIT(1)

#ifdef CONFIG_RTK_KDRV_EMCU_MODULE
static int rtk_emcu_ac_alert(void);
extern int rtk_pm_load_8051_code(void);
#endif

/************************************************************************
 * #define
 ************************************************************************/
u32 emcu_CEC_event(void)
{
    if(emcu_suspend_event & CEC_SSP_EVENT){
        return CEC_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_PPS_event(void)
{
    if(emcu_suspend_event & PPS_SSP_EVENT){
        return PPS_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_IRDA_event(void)
{
    if(emcu_suspend_event & IRDA_SSP_EVENT){
        return IRDA_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_LSADC_event(void)
{
    if(emcu_suspend_event & LSADC_SSP_EVENT){
        return LSADC_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_GPIO_event(void)
{
    if(emcu_suspend_event & GPIO_SSP_EVENT){
        return GPIO_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_WOV_event(void)
{
    if(emcu_suspend_event & WOV_SSP_EVENT){
        return WOV_SSP_EVENT;
    }
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
#define EMCU_NET_EVENT  (WOL_SSP_EVENT|WOW_SSP_EVENT|WIFI_SSP_EVENT)
u32 emcu_NET_event(void)
{
    if(emcu_suspend_event & EMCU_NET_EVENT){
        return (emcu_suspend_event & EMCU_NET_EVENT);
    }
    return 0;
}

/************************************************************************
 *
 ************************************************************************/
u32 emcu_LED_FALSH_event(void)
{
    if(emcu_suspend_event & LED_FLASH_EVENT){
        return (emcu_suspend_event & LED_FLASH_EVENT);
    }
    return 0;
}
EXPORT_SYMBOL(emcu_LED_FALSH_event);

/************************************************************************
 *
 ************************************************************************/
u32 emcu_WOL_event(void)
{
    if(emcu_suspend_event & WOL_SSP_EVENT){
        return (emcu_suspend_event & WOL_SSP_EVENT);
    }
    return 0;
}
EXPORT_SYMBOL(emcu_WOL_event);

/************************************************************************
 *
 ************************************************************************/
u32 emcu_WOW_event(void)
{
    if(emcu_suspend_event & WOW_SSP_EVENT){
        return (emcu_suspend_event & WOW_SSP_EVENT);
    }
    return 0;
}
EXPORT_SYMBOL(emcu_WOW_event);


int emcu_set_RTC(int value)
{
	int ret = 0;
	EMCU_DEBUG("emcu_set_RTC=%d\n",value);
	if(new_protocol_enable == 1){
		ret = send_packet(EMCU_RTC, value, NULL, NULL, 0, 0);
		if(ret != EMCU_CMD_DONE){
			EMCU_ERR("RTC set fialed=%d\n",ret);
		}
	}else{
		if(value){
			//inform 8051 do rtc_wakeup
			rtd_setbits(RTD_SHARE_MEM_BASE,_BIT30);
		} else {
			rtd_clearbits(RTD_SHARE_MEM_BASE,_BIT30);
		}
	}
	return ret;
}
EXPORT_SYMBOL(emcu_set_RTC);

void emcu_set_ddc(unsigned int value)
{
    int ret = -1;
    if(new_protocol_enable == 1)
    {
        ret = send_packet(EMCU_DDC, 0, &value, NULL, 1, 0);
        if(ret != EMCU_CMD_DONE){
            EMCU_ERR("%s failed=%d\n", __func__, ret);
        }
    }
    else
    {
        EMCU_ERR("%s, unsupport!\n", __func__);
    }
}
EXPORT_SYMBOL(emcu_set_ddc);


/************************************************************************
 * This func is going to provide pm.c to call.
 ************************************************************************/
char* power_pin_tlb[PARAM_LEN_PWEN] = {
	POWER_PIN_TLB
};

/************************************************************************
 * External variable
 ************************************************************************/


/************************************************************************
 * Return format
 * [7:0] = address (0: for GPIO 0~30, 4: for GPIO 31~53)
 * [15:8] = gpio_no (base on address, Range is 0~30 )
 * note bit0 of reg is write enable bit
 * example:
 *          gpio 00  ==> 0x0100
 *          gpio 10  ==> 0x0b00
 *          gpio 30  ==> 0x1f00
 *          gpio 31  ==> 0x0104
 *          gpio 47  ==> 0x1104
 ************************************************************************/
static
unsigned int get_gpio_info(unsigned char GPIO_NUM)
{
#if 1
    /* Start from 2851a, report GPIO number directly to 8051 */
    return GPIO_NUM;
#else
	unsigned int loop = 0;
	unsigned int bitOffset = 0;
	unsigned int regAddr = 0;

	while(loop < 2){    //iso gpio have 2 set only
		if(GPIO_NUM <= (30+(31*loop))){
			bitOffset = (GPIO_NUM - (31*loop))+1;
			regAddr = loop*4;
			return ((bitOffset<<SHT_GPIO_BIT) | regAddr );
		}
		loop++;
	}
	return 0xff;
#endif
}

/************************************************************************
************************************************************************/
unsigned char emcu_pin_type_convert(unsigned int ptype)
{
	unsigned char emcu_ptype = 0;
	switch(ptype){
		case PCB_PIN_TYPE_ISO_GPIO:
			emcu_ptype = EMCU_PINTYPE_ISO_GPIO;
			break;
		case PCB_PIN_TYPE_ISO_GPI:
			emcu_ptype = EMCU_PINTYPE_ISO_GPI;
			break;
		case PCB_PIN_TYPE_ISO_GPIO_MIO:
			emcu_ptype = EMCU_PINTYPE_ISO_GPIO_MIO;
			break;
#ifdef CONFIG_RTK_KDRV_MIO
		case PCB_PIN_TYPE_MIO_GPIO:
			emcu_ptype = EMCU_PINTYPE_MIO_GPIO;
			break;
#endif
		default:
			EMCU_ERR("%s, pin type:%d not support!\n", __func__, ptype);
			break;
	}
	return emcu_ptype;
}

/************************************************************************
 * This function handle GPIO type iopad always.
 * Return format
 * [7:0] = address (0: for GPIO 0~30, 4: for GPIO 31~61)
 * [15:8] = gpio_no (base on address, Range is 0~30 )
 * [19:16] = invert.
 * [23:20] = iso_gpio type(0x4)
 * [31:24] = special event flag.
 *           ex. 0x57: re-key; 0x56: ddr power pin
 ************************************************************************/
static
unsigned int powerMgr_interpret_gpio_value(unsigned long long param)
{
	unsigned int ptype;
	unsigned int index;
	unsigned int invert;
	unsigned int para_tmp;
	unsigned int event_type;
	unsigned char emcu_ptype;

	event_type = ((param >> IDX_SHIFT) & 0xff);
	ptype 	= GET_PIN_TYPE(param);
	index	= GET_PIN_INDEX(param);
	invert	= GET_GPIO_INVERT(param);
	para_tmp = get_gpio_info(index);
	emcu_ptype = emcu_pin_type_convert(ptype);
	if(para_tmp == 0xff)
		return para_tmp;
	para_tmp |= ((invert&0xF)<<SHT_GPIO_IVR)|((emcu_ptype&0xF)<<SHT_GPIO_TYP)|((event_type&0xFF)<<24);

	EMCU_DEBUG("index=0x%x\n",index);
	EMCU_DEBUG("invert=0x%x\n",invert);
	EMCU_DEBUG("emcu_ptype=0x%x\n",emcu_ptype);
	EMCU_DEBUG("para_tmp=0x%x\n",para_tmp);

	return para_tmp;
}

/************************************************************************
 * @type   : transfer data type in Dec od Hex
 * @i_cnt  : max param count, if beyond, report error
 * @*o_cnt : pointer for report how many param get
 * @param  : pointer of arry that data parsed.
 * @buf    : input data buffer
 * @count  : length of input data
 ************************************************************************/
static
int emcu_get_param_single(int type,
		unsigned long long* param,
		const char *buf, size_t count)
{
	int ret = -1;
	unsigned long long tmp_long;

	if (kstrtoull(buf, type, &tmp_long)){
		goto FINISH_OUT;
	}else{
		*param = tmp_long;
	}
	ret = 0;

FINISH_OUT:
	if(ret)
		EMCU_DEBUG("get param fail\n");

	return ret;
}

/************************************************************************
 *
 ************************************************************************/
static
unsigned long long emcu_get_power_type_idx(char* pin_name)
{
    unsigned long long ret_idx = 0;

#ifdef CONFIG_ARCH_RTK6702
    if(!pin_name)
        return 0;
    if (!strncmp(pin_name, EMCU_POWER_PIN_NAME1, strlen(EMCU_POWER_PIN_NAME1))){
        ret_idx = (IDX_POWER_PIN) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_POWER_PIN_NAME2, strlen(EMCU_POWER_PIN_NAME2))){
        ret_idx = (IDX_POWER_PIN|0x1ULL) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_POWER_PIN_NAME3, strlen(EMCU_POWER_PIN_NAME3))){
        ret_idx = (IDX_POWER_PIN|0x2ULL) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_POWER_PIN_NAME4, strlen(EMCU_POWER_PIN_NAME4))){
        ret_idx = (IDX_POWER_PIN|0x3ULL) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_DDR_POWER_NAME2, strlen(EMCU_DDR_POWER_NAME2))){
        ret_idx = (IDX_DDR_POWER|0x1ULL) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_DDR_POWER_NAME1, strlen(EMCU_DDR_POWER_NAME1))){
        ret_idx = (IDX_DDR_POWER) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_WOL_POWER_NAME, strlen(EMCU_WOL_POWER_NAME))){
        ret_idx = (IDX_WOL_POWER) << IDX_SHIFT;
    }else if (!strncmp(pin_name, EMCU_WOV_POWER_NAME, strlen(EMCU_WOV_POWER_NAME))){
        ret_idx = (IDX_WOV_POWER) << IDX_SHIFT;
    }
#else
    int i;
    int power_cnt = 0;
    int ddr_cnt = 0;
    char *pin_name_type[PARAM_LEN_PWEN]= {
        "PIN_POWER",      //0 power pin
        "PIN_DDR_POWER",  //1 DDR power pin
        "PIN_WLAN",       //2 wifi power pin
        "PIN_WOV",        //3 wov power pin
        "PIN_AMP",        //4 amp power pin
        "PIN_WOL",        //5 wol power pin
        "PIN_LED",        //6 led flash pin
        "PIN_A0V9",       //7 control HDRX0V9/USB0V9/PIF OV9 power
    };
    char *power_pin_name[8]= {
        "PIN_POWER_EN",
        "PIN_POWER_EN1",
        "PIN_POWER_EN2",
        "PIN_POWER_EN3",
        "PIN_POWER_EN4",
        "PIN_POWER_EN5",
    };
	char *ddr_pin_name[5]= {
        "PIN_DDR_POWER_EN",
        "PIN_DDR_POWER_EN1",
        "PIN_DDR_POWER_EN2",
        "PIN_DDR_POWER_EN3",
    };

    if(!pin_name){
         return 0;
    }
    for(i=0; i<PARAM_LEN_PWEN; i++){
        if(!pin_name_type[i]){
            break;
        }
        if(strncmp(pin_name, pin_name_type[i], 7)==0){
            switch (i){
                case 0:
                    for(power_cnt=0; power_cnt<8; power_cnt++){
                        if(!power_pin_name[power_cnt]){
                            break;
                        }
                        if(strcmp(pin_name, power_pin_name[power_cnt]) == 0){
                            if(IDX_POWER_PIN != 0x0ULL){
                                ret_idx = (IDX_POWER_PIN | power_cnt) << IDX_SHIFT;
                            }else{
                                ret_idx = IDX_POWER_PIN << IDX_SHIFT;
                            }
                            EMCU_ALERT("power%d power pin hit\n", power_cnt);
                            break;
                        }
                    }
                    break;
                case 1:
                    for(ddr_cnt=0; ddr_cnt<5; ddr_cnt++){
                        if(!ddr_pin_name[ddr_cnt]){
                            break;
                        }
                        if(strcmp(pin_name, ddr_pin_name[ddr_cnt]) == 0){
                            if(IDX_DDR_POWER != 0x56ULL){
                                ret_idx = (IDX_DDR_POWER | ddr_cnt) << IDX_SHIFT;
                            }else{
                                ret_idx = IDX_DDR_POWER << IDX_SHIFT;
                            }
                            EMCU_ALERT("DDR%d power pin hit\n", ddr_cnt);
                            break;
                        }
                    }
                    break;
                case 2:
                    ret_idx = IDX_WIFI_POWER << IDX_SHIFT;
                    EMCU_ALERT("wifi power pin hit\n");
                    break;
                case 3:
                    ret_idx = IDX_WOV_POWER << IDX_SHIFT;
                    EMCU_ALERT("wov power pin hit\n");
                    break;
                case 4:
                    ret_idx = 0x0ULL << IDX_SHIFT;
                    EMCU_ALERT("AMP pin hit\n");
                    break;
                case 5:
                    if(emcu_WOL_event() == 0){
                        ret_idx = IDX_WOL_POWER << IDX_SHIFT;
                        EMCU_ALERT("wol pin hit\n");
                    }else{
                        ret_idx = 0xFAFA;  //invalid value
                    }
                    break;
                case 6:
                    if(emcu_LED_FALSH_event() != 0){
                        ret_idx = IDX_LED_FLASH_POWER << IDX_SHIFT;
                        EMCU_ALERT("led flash mode hit\n");
                    }else{
                        ret_idx = 0xFAFA;  //invalid value
                    }
                    break;
                case 7:
                    if(strcmp(pin_name, "PIN_A0V9_EN1") == 0){
                        if(get_product_type() == PRODUCT_TYPE_DIAS){
                            ret_idx = 0xFAFA;  //invalid value, dias don't need power down power
                        }else{
                            ret_idx = 0x0ULL << IDX_SHIFT;
                            EMCU_ALERT("PIN_A0V9_EN1 hit\n");
                        }
                    }else if(strcmp(pin_name, "PIN_A0V9_EN2") == 0){
                        ret_idx = 0x0ULL << IDX_SHIFT;
                        EMCU_ALERT("PIN_A0V9_EN2 hit\n");
                    }else{
                        ret_idx = 0xFAFA;  //invalid value, dias don't need power down power
                    }
                    break;
                default:
                    EMCU_ALERT("power pin type %d wrong\n", i);
            }
            break;
       }
    }
#endif
    return ret_idx;
}


/************************************************************************
 * @type   : transfer data type in Dec od Hex
 * @i_cnt  : max param count, if beyond, report error
 * @*o_cnt : pointer for report how many param get
 * @param  : pointer of arry that data parsed.
 * @buf    : input data buffer
 * @count  : length of input data
 * return  : return -1 if
 *					1. There is no any parameter was found.
 *					2. Number of found parameters big then i_cnt.
 *
 ************************************************************************/
static
int emcu_get_param_multi(int type,
		int i_cnt,
		int* o_cnt,
		unsigned long long* param,
		const char *buf, size_t count)
{
	int ret = -1;
	int i;
	char last_char;
	char *opt = NULL;
	char *buftmp = NULL;
	char *buforg = NULL;
	unsigned long long tmp_long;

	buforg = buftmp = kmalloc(count+1, GFP_KERNEL);
	if(!buftmp){
		goto FINISH_OUT;
	}
	memset(buftmp,0,count+1);
	strncpy(buftmp, buf, count);

	i = strlen(buftmp)-1;
	last_char = buftmp[i];

	if(!isdigit(last_char)&& !isalpha(last_char) ){
		buftmp[i] = 0;
	}

	i = 0;
	while ((opt = strsep(&buftmp, ",")) != NULL) {
		if(i && (type == PR_TYPE_PCB)){
			/* parse PCB parameter */
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
			if(pcb_mgr_get_enum_info_byname(opt, &tmp_long) == 0){
				param[i] = tmp_long;
				param[i] |= emcu_get_power_type_idx(opt);
				EMCU_DEBUG("%s[0x%llx] got.\n",opt,param[i]);
				i++;
			}else{
				EMCU_INFO("%s can't be found.\n",opt);
			}
#else
			rtd_pr_emcu_alert("%s(%d)need CONFIG_REALTEK_PCBMGR support\n",__func__,__LINE__);
#endif
		}else{
			/* translate digit number HEX/DEC */
			if (kstrtoull(opt, type, &tmp_long)){
				goto FINISH_OUT;
			}else{
				param[i] = tmp_long;
				EMCU_DEBUG("param[%d]=0x%llx\n",i,param[i]);
				i++;
			}
		}
	}
	*o_cnt = i;
	if((*o_cnt == 0) || ((*o_cnt) > i_cnt)) {
		goto FINISH_OUT;
	}
	ret = 0;

FINISH_OUT:
	if(buforg)
		kfree(buforg);
	if(ret)
		EMCU_DEBUG("get param fail\n");

	return ret;
}

/* new protocol start */
static unsigned short emcu_sum_zero_calc(unsigned int header, unsigned short *data, int len)
{
    int i = 0;
    unsigned short ret = 0, *sp = (unsigned short *)&header;

    EMCU_ALERT("HEADER=%08x\n", header);
    ret = *sp + *(sp + 1);
    for (i = 0;i < len;i++)
        ret += data[i];

    // 2's complement
    ret = ~ret + 1;

    EMCU_ALERT("checksum=%04x\n", ret);

    return ret;
}

static int make_request(unsigned char class,
        unsigned short field,
        unsigned int *data,
        int len,
        bool read)
{
    int ret = 0, i = 0;
    unsigned int header = 0;
    unsigned int checksum = 0;

    // Class Code only uses 7-bit field
    class &= 0x7F;

    // Header preparation
    if (len > EMCU_DAT_LEN_MAX) {
        return -EMCU_LEN_EXCEED;
	}
	
    if (len && !data) {
        return -EMCU_EMPTY_DATA;
	}
	
    // clear share memory context
	for(i=0;i<(RTD_SHARE_MEM_LEN/2);i++){
		rtd_outl((RTD_SHARE_MEM_BASE + (4*i)),0);
    }

    // data copy
    for (i = 0;i < len;i++) {
        rtd_outl(RTD_SHARE_MEM_BASE+(4*(i+1)),data[i]);
	}
	
    // cal header
    header = SET_REQ_HEADER(class, field, len + 1, read);
    header |= CMD_FLAG_MASK;

    // Checksum
    checksum = emcu_sum_zero_calc(header, (unsigned short *)data, (len<<1));
    rtd_outl((RTD_SHARE_MEM_BASE + ((len+1)<<2)), checksum);

    // must write header in last step to avoid triggerring receiver with polling method
    rtd_outl((RTD_SHARE_MEM_BASE), header);

    return ret;
}

int send_packet(unsigned char class,
        unsigned short field,
        unsigned int *data,
        unsigned int *ret_dat,
        int len,
        bool read)
{
    int ret = 0, ret_len = 0, i;
    int timeout = 100; // count
    unsigned int reg;
    unsigned short checksum;
#if 1
    unsigned int temp[16];
#endif

    // init
    ret = make_request(class, field, data, len, read);
    if (ret)
        return ret;

#if 1
    // debug
    for(i=0;i<(RTD_SHARE_MEM_LEN/2);i++){
        temp[i] = rtd_inl((RTD_SHARE_MEM_BASE + (4*i)));
    }

    EMCU_ALERT(
            "mem[00]=0x%08x mem[01]=0x%08x mem[02]=0x%08x mem[03]=0x%08x\n"
            "mem[04]=0x%08x mem[05]=0x%08x mem[06]=0x%08x mem[07]=0x%08x\n"
            "mem[08]=0x%08x mem[09]=0x%08x mem[0a]=0x%08x mem[0b]=0x%08x\n"
            "mem[0c]=0x%08x mem[0d]=0x%08x mem[0e]=0x%08x mem[0f]=0x%08x\n",
            temp[ 0],temp[ 1],temp[ 2],temp[ 3],
            temp[ 4],temp[ 5],temp[ 6],temp[ 7],
            temp[ 8],temp[ 9],temp[10],temp[11],
            temp[12],temp[13],temp[14],temp[15]);
#endif
    // trigger receiver if interrupt supports.
#define EMCU_INT_REG        EMCU_CPU_INT2_reg
#define EMCU_INT_TRIGGER    (EMCU_CPU_INT2_int_sm_mask|EMCU_CPU_INT2_write_data_mask)
    rtd_outl(EMCU_INT_REG, EMCU_INT_TRIGGER);

    // check cmd flag first
    timeout = 1000;
    while(timeout--){
        reg = rtd_inl((RTD_SHARE_MEM_BASE));
        if (!(reg & CMD_FLAG_MASK))
            break;
        mdelay(5);
    }
    if ((reg & CMD_FLAG_MASK))
        return -EMCU_CMD_TIMEOUT;

    // wait response from receiver
    timeout = 1000;
    while(timeout--){
        reg = rtd_inl((RTD_SHARE_MEM_BASE));
        if ((reg & RES_FLAG_MASK))
            break;
        mdelay(5);
    }
    if (!(reg & RES_FLAG_MASK))
        return -EMCU_RES_TIMEOUT;

    // check errors bit first
    reg = rtd_inl((RTD_SHARE_MEM_BASE));
    if (reg & ERROR_BITS_MASK)
        return -EMCU_RES_ERROR;

    // make sure class code & read bit correct
    if (!(GET_CLASS_CODE(reg) == class) || !(GET_READ_BIT(reg) == read)) {
        return -EMCU_RES_MISMATCH;
	}
	
	if(read != 0) {
		// return data collection
		ret_len = GET_PACKET_LEN(reg) - 1;
		if (ret_len && !ret_dat)
			return -EMCU_EMPTY_DATA;

		for (i = 0;i < ret_len;i++)
			ret_dat[i] = rtd_inl((RTD_SHARE_MEM_BASE + 4*(i+1)));
		checksum = rtd_inw((RTD_SHARE_MEM_BASE + 4*(i+1)));
#if 1
		for(i=0;i<(RTD_SHARE_MEM_LEN/2);i++){
			temp[i] = rtd_inl((RTD_SHARE_MEM_BASE + (4*i)));
		}
		EMCU_ALERT("read mode response sharemem\n"
				"mem[00]=0x%08x mem[01]=0x%08x mem[02]=0x%08x mem[03]=0x%08x\n"
				"mem[04]=0x%08x mem[05]=0x%08x mem[06]=0x%08x mem[07]=0x%08x\n"
				"mem[08]=0x%08x mem[09]=0x%08x mem[0a]=0x%08x mem[0b]=0x%08x\n"
				"mem[0c]=0x%08x mem[0d]=0x%08x mem[0e]=0x%08x mem[0f]=0x%08x\n",
				temp[ 0],temp[ 1],temp[ 2],temp[ 3],
				temp[ 4],temp[ 5],temp[ 6],temp[ 7],
				temp[ 8],temp[ 9],temp[10],temp[11],
				temp[12],temp[13],temp[14],temp[15]);
#endif
		EMCU_ALERT("8051 checksum=%04x\n", checksum);

		if (checksum != emcu_sum_zero_calc(reg, (unsigned short *)ret_dat, (ret_len<<1)))
			return -EMCU_RES_CHK_FAIL;

		return ret_len;
	}
    EMCU_ALERT("Mission Completed\n");

    return ret;
}
EXPORT_SYMBOL(send_packet);

/************************************************************************
 * FUNC: clear assignee operation, not clear all share memory
 *
 * example:
 *	cat emcu_clr_pm_task
 *	echo 1 > emcu_clr_pm_task
 *
 ************************************************************************/
static
ssize_t emcu_clr_pm_task_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
#if 1
    return snprintf(buf, 512,
			"please key in \"echo param > /sys/devices/virtual/misc/emcu_kdv/emcu_clr_pm_task\"\n"
			"  nibble: 0:KEYPAD; 1:CEC; 2:PPS; 3:WUT; 4:IRDA; 5:PWEN; 6:WOW_nible; 7:STM_nible\n"
			"  bit:    24:WOW; 25:MHL; 26:REV; 27:UART; 28|29:STM; 30:RTC; 31:REV\n");

#else
    return snprintf(buf, 128, "0x%x\n",(unsigned int)GET_EVENT_MSG());
#endif
}

static
ssize_t emcu_clr_pm_task_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int msg;
	unsigned long long loc;
	size_t ret = count;

	const char* str[]={
		"KEYPAD","CEC","PPS","WUT",
		"IRDA","PWEN","WOW","STM" };

	if(emcu_get_param_single(16,(unsigned long long*)&loc,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	/*
	 The input value between 0~7 is meaning group func.
	 The input value between 20~31 is meaning bit func. Make sure which func you want to clear.
	 bit[24:31] These area are bit operation.
	*/
	if(new_protocol_enable == 0){
		if(((loc > 7) && (loc < 20)) || (loc > 31)){
			EMCU_DEBUG("input mismatch\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(loc < (sizeof(str) / sizeof(str[0])))
			EMCU_INFO("\n%sYou like to clear %s task.\n",__func__,str[loc]);

		msg = GET_EVENT_MSG();

		if(loc < 8){
			msg &= ~(0xf<<(loc*4));
		}else{
			if((loc == 28) || (loc == 29) ){
				/* bit[29:28] are area for STM */
				msg &= ~(0x3<<loc);
			}else{
				msg &= ~(0x1<<loc);
			}
		}

		SET_EVENT_MSG(msg);

		EMCU_INFO("pm_event_msg=0x%x\n",msg);
	}else{
		EMCU_ALERT("new_protocol not support to clr pm task\n");
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_clr_pm_task, S_IRUGO | S_IWUSR,
		emcu_clr_pm_task_dev_show,emcu_clr_pm_task_dev_store);

/************************************************************************
 * FUNC: claer all share memory immediately
 * example:
 *	cat emcu_clr_share_mem
 *	echo 1 > emcu_clr_share_mem
 *
 ************************************************************************/
static
ssize_t emcu_clr_share_mem_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{

	int i;
	char buf_tmp[512]={0};

	EMCU_INFO("%s(%u)\n", __func__, __LINE__);
#define EMCU_TEST_ENABLE
#ifdef  EMCU_TEST_ENABLE
	for(i=0;i<RTD_SHARE_MEM_LEN;i++){
		rtd_outl(RTD_SHARE_MEM_BASE+(4*i),0xffffffff);
	}
	EMCU_INFO("for test only, fill 0xffffffff to reg0~31\n");
#endif

	for(i=0;i<RTD_SHARE_MEM_LEN;i++){
		if(i)
            snprintf(buf_tmp+strlen(buf_tmp),1,",");
        snprintf(buf_tmp+strlen(buf_tmp),8,"%08x",rtd_inl(RTD_SHARE_MEM_BASE+(4*i)));
	}
        emcu_suspend_event = 0;
	emcu_auto_event = 0;
    return snprintf(buf, 128, "%s\n",buf_tmp);
}

static
ssize_t emcu_clr_share_mem_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int i;
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val == 1){
		/* clear share mem */
		for(i=0;i<RTD_SHARE_MEM_LEN;i++){
			rtd_outl(RTD_SHARE_MEM_BASE+(4*i),0);
		}
		/* clear power pin array */
		for(i=0;i<PARAM_LEN_PWEN;i++){
			pw_arry_ext[i] = 0;
		}
                emcu_suspend_event = 0;
		emcu_auto_event = 0;
#ifdef CONFIG_MULTI_IR_KEY
        reno_gpi_cnt = 0;
#endif
	}else{
		ret = -EINVAL;
		EMCU_DEBUG("input mismatch\n");
	}

FINISH_OUT:
	return ret;


}

DEVICE_ATTR(emcu_clr_share_mem, S_IRUGO | S_IWUSR,
		emcu_clr_share_mem_dev_show,emcu_clr_share_mem_dev_store);

/************************************************************************
  * FUNC: display share memory value
 * example:
 *	cat emcu_show_share_mem
 *
 ************************************************************************/
static
ssize_t emcu_show_share_mem_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int i;
	unsigned int share_tmp[32];

	for(i=0;i<RTD_SHARE_MEM_LEN;i++){
		share_tmp[i] = rtd_inl(RTD_SHARE_MEM_BASE+(4*i));
		//EMCU_INFO("mem[%02d]=0x%08x\n",i,share_mem[i]);
	}

	EMCU_ALERT( "The recorded POWER pin info is below.\n");
	for(i=0; i<PARAM_LEN_PWEN; i++){
		EMCU_ALERT(	"pow[%d]=0x%08x\n",	i, pw_arry_ext[i]);
	}
	return snprintf(buf, 1024,
			"mem[00]=0x%08x mem[01]=0x%08x mem[02]=0x%08x mem[03]=0x%08x\n"
			"mem[04]=0x%08x mem[05]=0x%08x mem[06]=0x%08x mem[07]=0x%08x\n"
			"mem[08]=0x%08x mem[09]=0x%08x mem[0a]=0x%08x mem[0b]=0x%08x\n"
			"mem[0c]=0x%08x mem[0d]=0x%08x mem[0e]=0x%08x mem[0f]=0x%08x\n"
			"mem[10]=0x%08x mem[11]=0x%08x mem[12]=0x%08x mem[13]=0x%08x\n"
			"mem[14]=0x%08x mem[15]=0x%08x mem[16]=0x%08x mem[17]=0x%08x\n"
			"mem[18]=0x%08x mem[19]=0x%08x mem[1a]=0x%08x mem[1b]=0x%08x\n"
			"mem[1c]=0x%08x mem[1d]=0x%08x mem[1e]=0x%08x mem[1f]=0x%08x\n",
			share_tmp[ 0],share_tmp[ 1],share_tmp[ 2],share_tmp[ 3],
			share_tmp[ 4],share_tmp[ 5],share_tmp[ 6],share_tmp[ 7],
			share_tmp[ 8],share_tmp[ 9],share_tmp[10],share_tmp[11],
			share_tmp[12],share_tmp[13],share_tmp[14],share_tmp[15],
			share_tmp[16],share_tmp[17],share_tmp[18],share_tmp[19],
			share_tmp[20],share_tmp[21],share_tmp[22],share_tmp[23],
			share_tmp[24],share_tmp[25],share_tmp[26],share_tmp[27],
			share_tmp[28],share_tmp[29],share_tmp[30],share_tmp[31]);
}

static
ssize_t emcu_show_share_mem_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	EMCU_INFO("\n(%s)no operation!\n",__func__);

	return count;
}

DEVICE_ATTR(emcu_show_share_mem, S_IRUGO | S_IWUSR,
		emcu_show_share_mem_dev_show,
		emcu_show_share_mem_dev_store);

/************************************************************************
 * FUNC: enable weke on WIFI dongle rekey func,
 *       emcu will check this func with emcu_set_WOL simultaneously.
 * example:
 *	cat emcu_set_WOWLAN
 *	echo 1 > emcu_set_WOWLAN
 ************************************************************************/
static
ssize_t emcu_set_wifi_dg_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
			"please key in \"echo param > /sys/devices/virtual/misc/emcu_kdv/emcu_set_wifi_dg\"\n"
			"        only one param which format match PCB param.");

}

/************************************************************************
 *
 ************************************************************************/
static
int powerMgr_wifi_dg_parser(const char *buf, size_t count,unsigned int ctl)
{
	unsigned int rekey;
	unsigned int pm_event_msg;
	int ret = 0;
	unsigned long long in_val;
	unsigned int para_tmp;
	unsigned char kptype;
	int param_cnt;
	unsigned long long param[2];

	if(ctl == PARSE_AUTO){
		if(!(emcu_test_ctl & EMCU_PARSE_EN)){
			EMCU_DEBUG("%s(%d)auto parser have not support\n",__func__,__LINE__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(emcu_get_param_multi(PR_TYPE_PCB,2,&param_cnt,param,
				EMCU_WIFI_REKEY_PIN_NAME,
				sizeof(EMCU_WIFI_REKEY_PIN_NAME)))
		{
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(param_cnt != 2){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		in_val = param[1];
		EMCU_DEBUG("pin value: 0x%llx \n",param[1]);

	}else if(ctl == PARSE_USER){
		if(emcu_get_param_single(16,&in_val,buf,count)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}else{
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	EMCU_DEBUG("WIFI dongle GPIO = 0x%llx\n",in_val);
	EMCU_DEBUG("GET_PIN_TYPE:%x\n",GET_PIN_TYPE(in_val));

	in_val |= IO_TPYE_REKEY;	//add re-key flag
	kptype = GET_PIN_TYPE(in_val);
	if( (kptype == PCB_PIN_TYPE_ISO_GPIO) ||
		(kptype == PCB_PIN_TYPE_ISO_GPI)
		|| (kptype == PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
		|| (kptype == PCB_PIN_TYPE_MIO_GPIO)
#endif
	)
	{
		if(new_protocol_enable == 1){
			para_tmp = powerMgr_interpret_gpio_value(in_val);
			if(para_tmp == 0xff)
				goto INVAILD;

			rekey = 1;
			ret = send_packet(EMCU_WOW, rekey, &para_tmp, NULL, 1, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("wifi_dg set fialed=%d\n",ret);
			}	
		}else{
			pm_event_msg = GET_EVENT_MSG();
			para_tmp = powerMgr_interpret_gpio_value(in_val);
			if(para_tmp == 0xff)
				goto INVAILD;
/*
WIFI dongle event Format define below:
[7:0] = address (0: 0~31, 4:32~)
[15:8] = gpio_no
[19:16] = invert.
[23:20] = iso_gpio type(0x4)
[31:24] = 0x57 (opcode for WIFI dongle rekey func)
*/
			SET_WIFI_DG_PARM(para_tmp);

			pm_event_msg |= ((unsigned int)0x1 << loc_wow); // bit0:WOW enable
#ifdef EMCU_WIFI_POWER_NAME
			pm_event_msg |= ((unsigned int)0x8 << loc_wifi); // bit3:albert WIFI module
#endif
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("pm_event_msg=0x%x\n",pm_event_msg);
		}
	}else{
INVAILD:
		EMCU_DEBUG("Has not vaild pin assigne!\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;
}

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_set_wifi_dg_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_wifi_dg_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_wifi_dg_parser(buf,count,PARSE_USER);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_WIFI_DG_ALERT;
                emcu_suspend_event |= WIFI_SSP_EVENT;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_set_wifi_dg, S_IRUGO | S_IWUSR,
		emcu_set_wifi_dg_dev_show,emcu_set_wifi_dg_dev_store);

/************************************************************************
 * FUNC: enable weke on LAN func
 * example:
 *	cat emcu_set_WOWLAN
 *	echo 1 > emcu_set_WOWLAN
 ************************************************************************/
static
ssize_t emcu_set_WOL_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 128, "0x%x\n",(unsigned int)GET_EVENT_MSG());
#if 0
    return snprintf(buf, 256,
			"please key in \"echo param > /sys/devices/virtual/misc/emcu_kdv/emcu_enable_wowlan\"\n"
			"        param maust as 0~15");
#endif
}

static
ssize_t emcu_set_WOL_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{

	unsigned long long in_val;
	unsigned int enable=0;
	int res=0;
	unsigned int pm_event_msg;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(in_val >1){
		EMCU_DEBUG("\nparameter big then 1. Ignore setting.\n");
        ret = -EINVAL;
        goto FINISH_OUT;
	}
	if(new_protocol_enable == 1){
		if(in_val == 0){
			enable =0;
		}else if(in_val == 1){
			enable = 1;
		}
		res = send_packet(EMCU_WOL, enable, NULL, NULL, 0, 0);
		if(res != EMCU_CMD_DONE){
			EMCU_ERR("EMCU_WOL set fialed=%d\n",res);
		}		
	}else{
		pm_event_msg = GET_EVENT_MSG();
		pm_event_msg &= ~((unsigned int)0x1 << loc_wow);
		pm_event_msg |= ((unsigned int)in_val << loc_wow); // bit0:WOW enable
#ifdef EMCU_WIFI_POWER_NAME
		if(in_val)
			pm_event_msg |= ((unsigned int)0x8 << loc_wifi); // bit3:albert WIFI module
#endif
		SET_EVENT_MSG(pm_event_msg);
		EMCU_INFO("pm_event_msg=0x%x\n",pm_event_msg);
	}
	if(in_val){
		emcu_suspend_event |= WOL_SSP_EVENT;
	}else{
		emcu_suspend_event &= ~WOL_SSP_EVENT;
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_set_WOL, S_IRUGO | S_IWUSR,
		emcu_set_WOL_dev_show,emcu_set_WOL_dev_store);


/************************************************************************
 * FUNC: enable weke on wide LAN func
 * example:
 *	cat emcu_set_WOWLAN
 *	echo 1 > emcu_set_WOWLAN
 ************************************************************************/
static
ssize_t emcu_set_WOWLAN_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
			"please key in \"echo param > /sys/devices/virtual/misc/emcu_kdv/emcu_enable_wowlan\"\n"
			"        param maust be pcb_param about GPIO of extern WOW_Module");

}

/************************************************************************
 *
 ************************************************************************/
static
int powerMgr_wowlan_parser(const char *buf, size_t count,unsigned int ctl)
{
	unsigned int pm_event_msg;
	unsigned char valid_num;
	unsigned int para_tmp;
	int ret = 0;
	unsigned long long in_val;
	unsigned char kptype;
	unsigned long long param[2];
	int param_cnt;
	/*
	 Note:
		Currently, wake on wide LAN use extern module
		and send messange by GPIO to inform EMCU.
		So, It's need to know which GPIO pin used.
	*/
	if(ctl == PARSE_AUTO){
		if(!(emcu_test_ctl & EMCU_PARSE_EN)){
			EMCU_DEBUG("%s(%d)auto parser have not support\n",__func__,__LINE__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(emcu_get_param_multi(PR_TYPE_PCB,2,&param_cnt,param,
				EMCU_WOWLAN_PIN_NAME,
				sizeof(EMCU_WOWLAN_PIN_NAME)))
		{
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		EMCU_DEBUG("pin cnt: %d \n",param_cnt-1);
		if(param_cnt != 2){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		in_val = param[1];
		EMCU_DEBUG("pin value: 0x%llx \n",param[1]);

	}else if(ctl == PARSE_USER){
		if(emcu_get_param_single(16,&in_val,buf,count)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}else{
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	EMCU_DEBUG("WOWLAN GPIO = 0x%llx\n",in_val);
	EMCU_DEBUG("GET_PIN_TYPE:%x\n",GET_PIN_TYPE(in_val));

		kptype = GET_PIN_TYPE(in_val);
		if( (kptype == PCB_PIN_TYPE_ISO_GPIO) ||
			(kptype == PCB_PIN_TYPE_ISO_GPI)
			|| (kptype == PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
			|| (kptype == PCB_PIN_TYPE_MIO_GPIO)
#endif
		){
			if(new_protocol_enable == 1){
				if(in_val == 0){
					EMCU_NOTICE("WOWLAN is diabled\n");
					goto FINISH_OUT;
				}
				in_val |= IO_WOW_PIN;
				if(keypad_valid_num == PARAM_LEN_KEYPAD){
					EMCU_WARNING("Force insert WOWLAN PIn to last field of keypad\n");
					/*
					 Force insert WOWLAN PIn to last field of keypad.
					 It's will cause to lost last original keypad pcb enum.
					 */
					keypad_valid_num -= 1;
				}
				keypad_arry_ext[keypad_valid_num] = powerMgr_interpret_gpio_value(in_val);
				if(keypad_arry_ext[keypad_valid_num] == 0xff)
					goto INVAILD;

				keypad_valid_num = keypad_valid_num+1;
				emcu_suspend_event |= WOW_SSP_EVENT;
			}else{
				pm_event_msg = GET_EVENT_MSG();
				in_val |= IO_WOW_PIN;
				para_tmp = powerMgr_interpret_gpio_value(in_val);
				if(para_tmp == 0xff)
					goto INVAILD;
				/*
				 To check if there are keypad parameter already.
				 if not, enable keypad function and to set one parameter
				 */
				valid_num = ((pm_event_msg >> loc_keypad) & 0xf);

				/* wowlan info locate at last keypad field always */
				if(emcu_auto_event & EMCU_WOWLAN_ALERT){
					SET_KEYPAD_PARM(valid_num-1,para_tmp);
				}else{
					if(valid_num == PARAM_LEN_KEYPAD){
						/*
						 Force insert WOWLAN PIn to last field of keypad.
						 It's will cause to lost last original keypad pcb enum.
						 */
						valid_num -= 1;
					}
					SET_KEYPAD_PARM(valid_num,para_tmp);
					valid_num += 1;
				}
				EMCU_DEBUG("valid_num:%u\n",valid_num);

				SET_EVENT_IDX(pm_event_msg,loc_keypad,valid_num);
#ifdef EMCU_WIFI_POWER_NAME
				pm_event_msg |= ((unsigned int)0x8 << loc_wifi); // bit3:albert WIFI module
#endif
				SET_EVENT_MSG(pm_event_msg);
				EMCU_INFO("pm_event_msg=0x%x\n",pm_event_msg);
				emcu_suspend_event |= WOW_SSP_EVENT;
			}
	}
	else{
INVAILD:
		EMCU_DEBUG("Has not vaild pin assigne!\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}
FINISH_OUT:
	return ret;

}

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_set_WOWLAN_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_wowlan_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_wowlan_parser(buf,count,PARSE_USER);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_WOWLAN_ALERT;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_set_WOWLAN, S_IRUGO | S_IWUSR,
		emcu_set_WOWLAN_dev_show,emcu_set_WOWLAN_dev_store);

/************************************************************************
 * To reference ./drivers/rtk_kdriver/include/rtk_kdriver/pcbMgr.h for PIN define
 ************************************************************************/

static
void powerMgr_setup_keypad_multi(unsigned int num,unsigned long long* param)
{
	unsigned int pm_event_msg;
	unsigned char valid_num_old = 0;
	unsigned int para_old = 0;
	unsigned int para_tmp;
	unsigned char kptype;
	unsigned char kpidx;
	unsigned char param1;
	unsigned char param2;
	unsigned char i;
	unsigned char valid_flag;
	unsigned char valid_num;

	if(num>PARAM_LEN_KEYPAD)
		num = PARAM_LEN_KEYPAD;

	if(new_protocol_enable == 1){
		if(keypad_valid_num == PARAM_LEN_KEYPAD){
			keypad_valid_num = keypad_valid_num -1; //keypad num can't over array.
			EMCU_INFO("%s %d keypad set over array, force use latset bit\n", __func__, __LINE__);
		}
		valid_num = keypad_valid_num;
		for(i=0;i<num;i++){
			EMCU_DEBUG("keypad param%u=0x%llx\n",i,*(param+i));
			valid_flag = 0;
			kptype = GET_PIN_TYPE(*(param+i));
			kpidx = GET_PIN_INDEX(*(param+i));
			EMCU_DEBUG("kptype=%u\n",kptype);
			EMCU_DEBUG("kpidx=%u\n",kpidx);
			switch(kptype){
			case PCB_PIN_TYPE_ISO_GPIO:
			case PCB_PIN_TYPE_ISO_GPI:
			case PCB_PIN_TYPE_ISO_GPIO_MIO:
#ifdef CONFIG_RTK_KDRV_MIO
			case PCB_PIN_TYPE_MIO_GPIO:
#endif
				EMCU_DEBUG("PCB_PIN_TYPE_ISO_GPIO\n");
				keypad_arry_ext[valid_num] = powerMgr_interpret_gpio_value(*(param+i));
				if(keypad_arry_ext[valid_num] == 0xff)
					break;
				valid_flag =1;
				break;
			case PCB_PIN_TYPE_LSADC:
				EMCU_DEBUG("PCB_PIN_TYPE_LSADC\n");
				//kpidx  = lsadc set
				param1 = GET_LSADC_HITVALUE(*(param+i));    //get hit value
				param2 = GET_LSADC_TOLERANCE(*(param+i));   //get tolerance
				keypad_arry_ext[valid_num] = (param1<<8)|param2;
				keypad_arry_ext[valid_num]|=(kpidx&0xf)<<16;
				keypad_arry_ext[valid_num]|=(kptype&0xf)<<20;
				valid_flag =1;
				break;
			case PCB_PIN_TYPE_ISO_UNIPWM:
			case PCB_PIN_TYPE_ISO_PWM:
			default:
				EMCU_DEBUG("unknow pin type\n");
				break;
			}
			if(valid_flag){
				EMCU_DEBUG("KEYPAD_PARM%u=0x%08x\n",valid_num,keypad_arry_ext[valid_num]);
				valid_num++;
				if(valid_num == PARAM_LEN_KEYPAD){
					EMCU_INFO("%s %d keypad set over array\n", __func__, __LINE__);
					break;
				}
			}
		}
		keypad_valid_num = valid_num;
	} else{
		/* To check if wake on wide lan info has been setted. */
		valid_num_old = ((GET_EVENT_MSG() >> loc_keypad) & 0xf);
		if( (emcu_auto_event & EMCU_WOWLAN_ALERT) &&
			(valid_num_old) )
		{
			para_old = GET_KEYPAD_PARM(valid_num_old-1);
			EMCU_DEBUG("last keypad param%d=0x%x\n",(valid_num_old-1),para_old);
			if(num == PARAM_LEN_KEYPAD){
				/*It's need to reserve one field for WOWLAN. */
				num -= 1;
			}
		}

		for(i=0;i<PARAM_LEN_KEYPAD;i++){
			SET_KEYPAD_PARM(i,0);
		}

		valid_num = 0;
		for(i=0;i<num;i++){
			EMCU_DEBUG("keypad param%u=0x%llx\n",i,*(param+i));
			valid_flag = 0;
			kptype = GET_PIN_TYPE(*(param+i));
			kpidx = GET_PIN_INDEX(*(param+i));
			EMCU_DEBUG("kptype=%u\n",kptype);
			EMCU_DEBUG("kpidx=%u\n",kpidx);
			switch(kptype){
			case PCB_PIN_TYPE_ISO_GPIO:
			case PCB_PIN_TYPE_ISO_GPI:
			case PCB_PIN_TYPE_ISO_GPIO_MIO:
#ifdef CONFIG_RTK_KDRV_MIO
			case PCB_PIN_TYPE_MIO_GPIO:
#endif
				EMCU_DEBUG("PCB_PIN_TYPE_ISO_GPIO\n");
				para_tmp = powerMgr_interpret_gpio_value(*(param+i));
				if(para_tmp == 0xff)
					break;
				valid_flag =1;
				break;
			case PCB_PIN_TYPE_LSADC:
				EMCU_DEBUG("PCB_PIN_TYPE_LSADC\n");
				//kpidx  = lsadc set
				param1 = GET_LSADC_HITVALUE(*(param+i));    //get hit value
				param2 = GET_LSADC_TOLERANCE(*(param+i));   //get tolerance
				para_tmp = (param1<<8)|param2;
				para_tmp|=(kpidx&0xf)<<16;
				para_tmp|=(kptype&0xf)<<20;
				valid_flag =1;
				break;
			case PCB_PIN_TYPE_ISO_UNIPWM:
			case PCB_PIN_TYPE_ISO_PWM:
			default:
				EMCU_DEBUG("unknow pin type\n");
				break;
			}

			if(valid_flag){
				EMCU_DEBUG("KEYPAD_PARM%u=0x%08x\n",valid_num,para_tmp);
				SET_KEYPAD_PARM(valid_num,para_tmp);
				valid_num++;
			}
		}

		if( (emcu_auto_event & EMCU_WOWLAN_ALERT) &&
			(valid_num_old) )
		{
			SET_KEYPAD_PARM(valid_num,para_old);
			valid_num++;
			EMCU_DEBUG("To add WOWLAN gpio [0x%08x]\n",para_old);
		}

		pm_event_msg = GET_EVENT_MSG();
		EMCU_DEBUG("Valid keypad paramteter have %u set.\n",valid_num);
		SET_EVENT_IDX(pm_event_msg,loc_keypad,valid_num);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
	}
	return;
}

/************************************************************************
 * FUNC: enable and setting keypad parameter
 * example:
 *	cat emcu_setup_keypad_multi
 *	echo key_num,param1,param2,...,... > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_keypad_multi
 *      echo 6,12345678aabbcdd,aabbccddeeff3456,100011204,222,444,100011204 > emcu_setup_keypad_multi
 * Note:
 *	1. the max number of keypad are 10
 *	2. get pcb info at "/sys/realtek_boards/pcb_enum/"
 *	2. get pcb info by "cat /sys/realtek_boards/pcb_enum/PIN_XXXX"
 *
 ************************************************************************/
static
ssize_t emcu_setup_keypad_multi_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    unsigned int key_ary[PARAM_LEN_KEYPAD];
    unsigned int i;
    unsigned int key_cnt;
    unsigned int str_len;
    char tmp_buf[128] = {0};

	if(new_protocol_enable == 1){
		key_cnt = keypad_valid_num;
	    EMCU_DEBUG("%s(%d)key_cnt=%d\n",__func__,__LINE__,key_cnt);

	    if(key_cnt > PARAM_LEN_KEYPAD)
	        key_cnt = PARAM_LEN_KEYPAD;

	    str_len = 0;
	    for(i=0;i<key_cnt;i++){
	        EMCU_DEBUG("%s(%d)key_ary[%d] = 0x%08x\n",__func__,__LINE__,i,keypad_arry_ext[i]);
	        snprintf(tmp_buf+str_len,
	            sizeof(tmp_buf)-str_len,
	            "0x%08x%s",keypad_arry_ext[i],((i+1)<key_cnt)?",":" ");
	        str_len = strlen(tmp_buf);
	        EMCU_DEBUG("%s(%d)%s\n",
	            __func__,__LINE__,tmp_buf);
	    }
	}else{
    	key_cnt = (GET_EVENT_MSG() & (0xf << loc_keypad)) >> loc_keypad;
    	EMCU_DEBUG("%s(%d)key_cnt=%d\n",__func__,__LINE__,key_cnt);

	    if(key_cnt > PARAM_LEN_KEYPAD)
	        key_cnt = PARAM_LEN_KEYPAD;

	    str_len = 0;
	    for(i=0;i<key_cnt;i++){
	        key_ary[i] = GET_KEYPAD_PARM(i);
	        EMCU_DEBUG("%s(%d)key_ary[%d] = 0x%08x\n",__func__,__LINE__,i,key_ary[i]);

	        snprintf(tmp_buf+str_len,
	            sizeof(tmp_buf)-str_len,
	            "0x%08x%s",key_ary[i],((i+1)<key_cnt)?",":" ");
	        str_len = strlen(tmp_buf);
	        EMCU_DEBUG("%s(%d)%s\n",
	            __func__,__LINE__,tmp_buf);
	    }
	}
    return snprintf(buf, 128, "%s\n",tmp_buf);
}


/************************************************************************
 To reference ./drivers/rtk_kdriver/include/rtk_kdriver/pcbMgr.h for PIN define
  PIN_KEYPAD_N	N =0~9
 ************************************************************************/
static
int powerMgr_keypad_parser(const char *buf, size_t count,unsigned int ctl)
{
	int ret = 0;
	int i;
	int param_cnt;
	unsigned long long param[11];
	const char* buf_pt;
	int  pmode;
	int cnt_tmp;
	unsigned long long tmp_long;

	if(ctl != PARSE_AUTO){
		pmode = PR_TYPE_HEX;
		buf_pt = buf;
		cnt_tmp = count;
	}else{
        cnt_tmp = 0;
        if(pcb_mgr_get_enum_info_byname(EMCU_KEYPAD_PWR_PIN_NAME, &tmp_long)==0){
            cnt_tmp++;
            param[cnt_tmp] = tmp_long;
            EMCU_INFO("%s(%d)param[%d]=0x%llx(%s)\n",
                    __func__,__LINE__,cnt_tmp,tmp_long,EMCU_KEYPAD_PWR_PIN_NAME);
        }

        if(cnt_tmp == 0){
            EMCU_INFO("No keypad pin defined\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}
        param[0] = param_cnt = cnt_tmp+1;
        goto AUTO_PARSE;
	}

	if(emcu_get_param_multi(pmode,11,&param_cnt,param,buf_pt,cnt_tmp)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

AUTO_PARSE:
	if(ctl != PARSE_AUTO){
		if(param[0] != (param_cnt-1)){
			EMCU_DEBUG("input mismatch %d %d\n", param[0], param_cnt);
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}else{
		param[0] = (param_cnt-1);
		for(i=0;i<param_cnt;i++){
			EMCU_DEBUG("pin%d: 0x%llx \n",i+1,param[i+1]);
		}
	}

	powerMgr_setup_keypad_multi((unsigned int)param[0],&param[1]);

FINISH_OUT:
	return ret;
}

/************************************************************************

 ************************************************************************/
static
ssize_t emcu_setup_keypad_multi_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_keypad_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_keypad_parser(buf,count,PARSE_USER);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_KEYPAD_ALERT;
		emcu_suspend_event |= LSADC_SSP_EVENT;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_keypad_multi, S_IRUGO | S_IWUSR,
		emcu_setup_keypad_multi_dev_show,
		emcu_setup_keypad_multi_dev_store);

/************************************************************************
 * FUNC: enable and setting keypad parameter
 * example:
 *	cat emcu_setup_cec
 *	echo 1 > emcu_setup_cec
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_cec_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 128,
		"%x\n",GET_EVENT_MSG());
}

static
ssize_t emcu_setup_cec_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int pm_event_msg;
	int res;
	ssize_t ret = count;
	unsigned long long in_val;
#ifdef EMCU_WIFI_POWER_NAME
	unsigned int cec_p=3;
#else
	unsigned int cec_p=15;
#endif
	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val >cec_p){
		EMCU_DEBUG("\nparameter big then 15 or 7. Ignore setting.\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		if(new_protocol_enable == 1){
			res = send_packet(EMCU_CEC, (in_val&0xF), NULL, NULL, 0, 0);
			if(res != EMCU_CMD_DONE){
				EMCU_ERR("cec set fialed=%d\n",res);
			}
		}else{
#ifdef EMCU_WIFI_POWER_NAME
			// bit0~2: for CEC event
			pm_event_msg = GET_EVENT_MSG() & ~(0x3<<loc_cec);
			pm_event_msg |= ((unsigned int)in_val << loc_cec);
#else
			pm_event_msg = GET_EVENT_MSG();
			SET_EVENT_IDX(pm_event_msg,loc_cec,(unsigned int)in_val);
#endif
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("pm_event_msg=0x%x\n",pm_event_msg);
		}
		cec_flag=(unsigned int)in_val;
		emcu_suspend_event |= CEC_SSP_EVENT;
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_cec, S_IRUGO | S_IWUSR,
		emcu_setup_cec_dev_show,emcu_setup_cec_dev_store);


/************************************************************************
 * FUNC: enable mhl func
 * example:
 *	cat emcu_setup_mhl
 *	echo 1 > emcu_setup_mhl
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_mhl_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo 1 > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_mhl\"\n");
}

static
ssize_t emcu_setup_mhl_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int res;
	unsigned int enable; 
	unsigned int pm_event_msg;
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(new_protocol_enable == 1){
		if(in_val == 1){
			enable = 1;
		}else{
		    enable = 0;
		}
		res = send_packet(EMCU_MHL, enable&0x1, NULL, NULL, 0, 0);
		if(res != EMCU_CMD_DONE){
			EMCU_ERR("MHL set fialed=%d\n",res);
		}
		EMCU_INFO("MHL ENABLE=%d\n",in_val);
	}else{
		if(in_val == 1){
			pm_event_msg = GET_EVENT_MSG();
			pm_event_msg |= ((unsigned int)0x1 << loc_mhl); // bit1:MHL enable
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("CBUS pm_event_msg=0x%x\n",pm_event_msg);
		}else{
			ret = -EINVAL;
			EMCU_DEBUG("input mismatch\n");
		}
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_mhl, S_IRUGO | S_IWUSR,
		emcu_setup_mhl_dev_show,emcu_setup_mhl_dev_store);

/************************************************************************
 * FUNC: enable ewbs func
 * example:
 *	cat emcu_setup_ewbs
 *	echo 1 > emcu_setup_ewbs
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_ewbs_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo 1 > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_ewbs\"\n");
}

static
ssize_t emcu_setup_ewbs_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int pm_event_msg;
	int res;
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(in_val == 1){
		if(new_protocol_enable == 1){
			res = send_packet(EMCU_EWBS, in_val, NULL, NULL, 0, 0);
			if(res != EMCU_CMD_DONE){
				EMCU_ERR("ewbs set fialed=%d\n",res);
			}	
		}else{
			pm_event_msg = GET_EVENT_MSG();
			pm_event_msg |= ((unsigned int)0x1 << loc_ewbs); // bit2:EWBS enable
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("CBUS pm_event_msg=0x%x\n",pm_event_msg);
		}
	}else{
		ret = -EINVAL;
		EMCU_DEBUG("input mismatch\n");
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_ewbs, S_IRUGO | S_IWUSR,
		emcu_setup_ewbs_dev_show,emcu_setup_ewbs_dev_store);


/************************************************************************
 * FUNC: enable uart func
 * example:
 *	cat emcu_setup_uart
 *	echo 1 > emcu_setup_uart
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_uart_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo 1 > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_uart\"\n");
}

static
ssize_t emcu_setup_uart_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int pm_event_msg;
	int res;
	unsigned long long in_val;
	size_t ret = count;


	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val == 1){
		if(new_protocol_enable == 1){
			res = send_packet(EMCU_UART, in_val, NULL, NULL, 0, 0);
			if(res != EMCU_CMD_DONE){
				EMCU_ERR("uart set fialed=%d\n",res);
			}	
		}else{
			pm_event_msg = GET_EVENT_MSG();
			pm_event_msg |= ((unsigned int)0x1 << loc_uart); // bit3:UART0 disable
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("UART pm_event_msg=0x%x\n",pm_event_msg);
		}
	}else{
		ret = -EINVAL;
		EMCU_DEBUG("input mismatch\n");
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_uart, S_IRUGO | S_IWUSR,
		emcu_setup_uart_dev_show,emcu_setup_uart_dev_store);

/************************************************************************
 * FUNC: enable wake on Voice
 * example:
 *	cat emcu_setup_wov
 *	echo 1 > emcu_setup_woc
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_wov_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 128,
		"%x\n",GET_EVENT_MSG());
}

static
ssize_t emcu_setup_wov_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int res;
	unsigned int enable=0;
	unsigned int pm_event_msg;
	ssize_t ret = count;
	unsigned long long in_val;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(new_protocol_enable == 1){
		if(in_val == 0){
			enable =0;
		}else if(in_val == 1){
			enable = 1;
		}
		res = send_packet(EMCU_WOV, enable, NULL, NULL, 0, 0);
		if(res != EMCU_CMD_DONE){
			EMCU_ERR("WOV set fialed=%d\n",res);
		}		
	}else{
		pm_event_msg = GET_EVENT_MSG();

	    /* loc_wov[bit3] is for WOV */
	    pm_event_msg = (pm_event_msg & 0x7fffffff);
		if(in_val == 0){
			SET_EVENT_MSG(pm_event_msg);
		}else if(in_val == 1){
			SET_EVENT_MSG(pm_event_msg|0x80000000);
		}else{
		    EMCU_INFO("param(0x%llx) no support\n",in_val);
		}
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_wov, S_IRUGO | S_IWUSR,
		emcu_setup_wov_dev_show,emcu_setup_wov_dev_store);

/************************************************************************
 * FUNC: enable pervious power on source func
 * example:
 *	cat emcu_setup_ppsource
 *	echo 1 > emcu_setup_ppsource
 * Note:
 *
 ************************************************************************/
static
ssize_t emcu_setup_ppsource_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo param > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_ppsource\"\n"
		"        note: support VGA1 only, please setting param=3");
}

static
ssize_t emcu_setup_ppsource_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned int param[EMCU_DAT_LEN_MAX];
	unsigned int ret_data[16] = {0};
	unsigned int clock_mode;
	unsigned int record_tme;
	unsigned int cur_time=0;
	int res;
	unsigned int pm_event_msg;
	unsigned long long in_val;
	size_t ret = count;
	char str[][10]={"HDMI1","HDMI2","HDMI3",
			"VGA1","VGA2","VGA3",
			"AV1","AV2","AV3"};

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(new_protocol_enable == 1){
		if(in_val < 10){
			EMCU_INFO("previously source is %s\n",str[in_val]);
			clock_mode =2;
			record_tme=1;
			res = send_packet(EMCU_TIMER, ((record_tme&0x1)<<2)| (clock_mode&0x3), NULL, ret_data, 0, 1);
			if(res<0){
				EMCU_ERR("cur_time get fialed=%d\n",res);
			}else if(res == 1){
				cur_time = ret_data[0];
			}else{
				EMCU_INFO("not set cur time\n");
				cur_time = 0;
			}
			
			EMCU_INFO("%s ret=%d\n",__func__, res);
			param[0] = (cur_time<<8 & 0xFFFFFF00) | (in_val & 0x000000FF);
			res = send_packet(EMCU_PPS, PARAM_LEN_PPS, param, NULL, 1, 0);
			if(res != EMCU_CMD_DONE){
				EMCU_ERR("PPS set fialed=%d\n",res);
			}
			emcu_suspend_event |= PPS_SSP_EVENT;

		}else{
			ret = -EINVAL;
			EMCU_DEBUG("unknow previously source.\n");
		}
	}else{
		if(in_val < 10){
			EMCU_INFO("previously source is %s\n",str[in_val]);
			SET_PPS_PARM((unsigned int)in_val);
			pm_event_msg = GET_EVENT_MSG();
			SET_EVENT_IDX(pm_event_msg,loc_pps,PARAM_LEN_PPS);
			SET_EVENT_MSG(pm_event_msg);
			EMCU_INFO("pm_event_msg=0x%x\n",pm_event_msg);
	                emcu_suspend_event |= PPS_SSP_EVENT;
		}else{
			ret = -EINVAL;
			EMCU_DEBUG("unknow previously source.\n");
		}
	}
FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_ppsource, S_IRUGO | S_IWUSR,
		emcu_setup_ppsource_dev_show,emcu_setup_ppsource_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_wakeup_timer(unsigned int days,unsigned int hours,unsigned int mins)
{
	unsigned int clock_mode;
	unsigned int param[EMCU_DAT_LEN_MAX];
	int ret;
	unsigned int pm_event_msg;
	unsigned int total_mins;

	if(hours > 23)
		hours = 23;
	if(mins > 59)
		mins = 59;

	total_mins = (days * 24 *60) + (hours *60) + mins;

	EMCU_DEBUG("sleeping time is %ddays %dhours %dmins\n",days,hours,mins);
	EMCU_DEBUG("total sleeping time is %u minute\n",total_mins);

	if(new_protocol_enable == 1){
		clock_mode = 1;
		param[0] = total_mins;

		ret = send_packet(EMCU_TIMER, clock_mode, param, NULL, 1, 0);
		if(ret != EMCU_CMD_DONE){
				EMCU_ERR("wakeup_timer set fialed=%d\n",ret);
		}
	}else{
		SET_WUT_PARM(total_mins);

		EMCU_DEBUG("IDX_REG_WUT adr =0x%x\n",
			RTD_SHARE_MEM_BASE+4*IDX_REG_WUT);
		EMCU_DEBUG("IDX_REG_WUT val =0x%x\n",
			rtd_inl(RTD_SHARE_MEM_BASE+4*IDX_REG_WUT));
		pm_event_msg = GET_EVENT_MSG();
		SET_EVENT_IDX(pm_event_msg,loc_wut,PARAM_LEN_WUT);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		EMCU_DEBUG("msg adr =0x%x\n",PM_EVENT_MSG);
		EMCU_DEBUG("msg val =0x%x\n",rtd_inl(PM_EVENT_MSG));
	}
}


/************************************************************************
 * FUNC: set wake up in timers function, minimum is minute
 * example:
 *	cat emcu_setup_wakeup_timer
 *	echo days,hours,mins > emcu_setup_wakeup_timer
 * Note: format days,hours,mins in DEC
 ************************************************************************/
static
ssize_t emcu_setup_wakeup_timer_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo days,hours,mins > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_wakeup_timer\"\n"
		"        Noteice: days\"hours\"mins are DEC format\n");
}

static
ssize_t emcu_setup_wakeup_timer_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{

	int param_cnt;
	unsigned long long param[3];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_DEC,3,&param_cnt,param,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 3){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_setup_wakeup_timer(
		(unsigned int)param[0],
		(unsigned int)param[1],
		(unsigned int)param[2]);

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_wakeup_timer, S_IRUGO | S_IWUSR,
		emcu_setup_wakeup_timer_dev_show,emcu_setup_wakeup_timer_dev_store);


/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_wakeup_timer1(int days, int hours, int mins, int sec)
{
	unsigned int clock_mode;
	unsigned int param[EMCU_DAT_LEN_MAX];
	int ret;
	unsigned int pm_event_msg;
	unsigned int total_secs;

	if(hours > 23)
		hours = 23;

	if(mins > 59)
		mins = 59;

	if(sec > 60)
		sec = 60;

	total_secs = (days*24*60*60) + (hours *60*60) + (mins *60) + sec;

	EMCU_DEBUG("sleeping time is %ddays %dhours %dmins, %dsec\n",days,hours,mins, sec);
	EMCU_DEBUG("total sleeping time is %u sec\n",total_secs);

	if(new_protocol_enable == 1){
		clock_mode = 0;
		param[0] = total_secs;

		ret = send_packet(EMCU_TIMER, clock_mode, param, NULL, 1, 0);
		if(ret != EMCU_CMD_DONE){
				EMCU_ERR("wakeup_timer set fialed=%d\n",ret);
		}
	}else{
		SET_WUT_PARM(total_secs);

		EMCU_DEBUG("IDX_REG_WUT adr =0x%x\n", RTD_SHARE_MEM_BASE+4*IDX_REG_WUT);
		EMCU_DEBUG("IDX_REG_WUT val =0x%x\n", rtd_inl(RTD_SHARE_MEM_BASE+4*IDX_REG_WUT));

		// Read back from index register
		pm_event_msg = GET_EVENT_MSG();

		// set index for WUT to infor emcu using sec paramter
		SET_EVENT_IDX(pm_event_msg,loc_wut,0x2);

		// Write to Index register
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		EMCU_DEBUG("msg adr =0x%x\n",PM_EVENT_MSG);
		EMCU_DEBUG("msg val =0x%x\n",rtd_inl(PM_EVENT_MSG));
	}
}


/************************************************************************
 * FUNC: set wake up in timers function, minimum is sec
 * example:
 *	cat emcu_setup_wakeup_timer1
 *	echo days,hours,mins,sec > emcu_setup_wakeup_timer1
 * Note: format days,hours,mins,sec  in DEC
 ************************************************************************/
static
ssize_t emcu_setup_wakeup_timer1_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo days,hours,mins,sec > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_wakeup_timer1\"\n"
		"        Noteice: days\"hours\"mins\"sec are DEC format\n");
}

static
ssize_t emcu_setup_wakeup_timer1_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[4];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_DEC,4,&param_cnt,param,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 4){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_setup_wakeup_timer1(
		(unsigned int)param[0],
		(unsigned int)param[1],
		(unsigned int)param[2],
		(unsigned int)param[3]);
FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_setup_wakeup_timer1, S_IRUGO | S_IWUSR,
		emcu_setup_wakeup_timer1_dev_show,emcu_setup_wakeup_timer1_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_wakeup_min( unsigned int reason,
                                unsigned int mins)
{
	unsigned int clock_mode;
	unsigned int param[EMCU_DAT_LEN_MAX];
	int ret;
	unsigned int pm_event_msg;
	unsigned int s_reason;

	/* the argument reason is useless, It's fixed as '1' at min case. */
	s_reason = 1;

	EMCU_DEBUG("sleeping time is %u minute\n",mins);
	if(new_protocol_enable == 1){
		clock_mode = 1;
		param[0] = mins;

		ret = send_packet(EMCU_TIMER, clock_mode, param, NULL, 1, 0);
		if(ret != EMCU_CMD_DONE){
				EMCU_ERR("wakeup_timer set fialed=%d\n",ret);
		}
	}else{
		EMCU_DEBUG("sleeping reason is %u\n",s_reason);
		SET_WUT_PARM(mins);

		pm_event_msg = GET_EVENT_MSG();
		SET_EVENT_IDX(pm_event_msg,loc_wut,s_reason);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		EMCU_DEBUG("msg adr =0x%x\n",PM_EVENT_MSG);
		EMCU_DEBUG("msg val =0x%x\n",rtd_inl(PM_EVENT_MSG));
	}
}


/************************************************************************
 * FUNC: set wake up in timers and reason function, minimum is min
 * example:
 *	cat emcu_setup_wakeup_min
 *	echo reason,mins > emcu_setup_wakeup_min
 * Note: format reason,mins  in DEC
 ************************************************************************/
static
ssize_t emcu_setup_wakeup_min_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo reason,mins > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_wakeup_min\"\n"
		"        Noteice: reason\"mins are DEC format\n");
}

static
ssize_t emcu_setup_wakeup_min_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{

	int param_cnt;
	unsigned long long param[2];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_DEC,2,&param_cnt,param,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 2){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_setup_wakeup_min(
		(unsigned int)param[0],
		(unsigned int)param[1]);

FINISH_OUT:
	return ret;


}

DEVICE_ATTR(emcu_setup_wakeup_min, S_IRUGO | S_IWUSR,
		emcu_setup_wakeup_min_dev_show,emcu_setup_wakeup_min_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_wakeup_sec( unsigned int reason,
                                unsigned int sec)
{
	unsigned int param[EMCU_DAT_LEN_MAX];
	unsigned int clock_mode;
	int ret;
	unsigned int pm_event_msg;
	unsigned int s_reason;

	/* the argument reason is useless, It's fixed as '2' at sec case. */
	s_reason = 2;
	EMCU_DEBUG("sleeping time is %u seconds\n",sec);

	if(new_protocol_enable == 1){
		clock_mode = 0;
		param[0] = sec;

		ret = send_packet(EMCU_TIMER, clock_mode, param, NULL, 1, 0);
		if(ret != EMCU_CMD_DONE){
				EMCU_ERR("wakeup_timer set fialed=%d\n",ret);
		}
	}else{
		EMCU_DEBUG("sleeping reason is %u\n",s_reason);
		SET_WUT_PARM_SEC(sec);

		pm_event_msg = GET_EVENT_MSG();
		SET_EVENT_IDX(pm_event_msg,loc_wut_sec,s_reason);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		EMCU_DEBUG("msg adr =0x%x\n",PM_EVENT_MSG);
		EMCU_DEBUG("msg val =0x%x\n",rtd_inl(PM_EVENT_MSG));
	}
}


/************************************************************************
 * FUNC: set wake up in timers and reason function, minimum is sec
 * example:
 *	cat emcu_setup_wakeup_sec
 *	echo reason,mins > emcu_setup_wakeup_sec
 * Note: format reason,mins  in DEC
 ************************************************************************/
static
ssize_t emcu_setup_wakeup_sec_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo reason,sec > /sys/devices/virtual/misc/emcu_kdv/emcu_setup_wakeup_sec_\"\n"
		"        Noteice: reason\"sec are DEC format\n");
}

static
ssize_t emcu_setup_wakeup_sec_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{

	int param_cnt;
	unsigned long long param[2];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_DEC,2,&param_cnt,param,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 2){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_setup_wakeup_sec(
		(unsigned int)param[0],
		(unsigned int)param[1]);

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_wakeup_sec, S_IRUGO | S_IWUSR,
		emcu_setup_wakeup_sec_dev_show,emcu_setup_wakeup_sec_dev_store);


/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_irda(unsigned int protocol ,
                         unsigned int key_num,
                         unsigned int * key_array )
{
	unsigned int i;
	unsigned int pm_event_msg;
	EMCU_ALERT("IrDA Protocol is %u key_num:%u\n",protocol,key_num);

	if(new_protocol_enable == 1){
		if(!key_num)
			return;
		//param[0] = protocol;
		
		irda_arry_ext[0] = protocol;
		if(key_num > (PARAM_LEN_IRDA-1)){
			/*
			 The PARAM_LEN_IRDA include procotol and lot of set of key codes,
			 So, the number of irda key are (PARAM_LEN_IRDA-1)
			 */
			key_num = (PARAM_LEN_IRDA-1);
		}
		/* To set physical key code */
		for(i=0 ;i<key_num; i++){
			EMCU_ALERT("IR_KEY%u=0x%x\n",1+i,*(key_array+i));
			//param[i+1] = *(key_array+i);
			irda_arry_ext[i+1] = *(key_array+i);
		}
		irda_valid_num = i;
		
		EMCU_ALERT("IrDA Protocol is %u irda_num:%u\n",protocol,irda_valid_num);
	}else{
		/* initialize all data */
		for(i=0 ;i<PARAM_LEN_IRDA; i++){
			SET_IRDA_PARM(i,0);
		}

		if(!key_num)
			return;

		/* To set protocol */
		SET_IRDA_PARM(0,protocol);
		if(key_num > (PARAM_LEN_IRDA-1)){
			/*
	 		 The PARAM_LEN_IRDA include procotol and lot of set of key codes,
			 So, the number of irda key are (PARAM_LEN_IRDA-1)
			 */
			key_num = (PARAM_LEN_IRDA-1);
		}

		/* To set physical key code */
		for(i=0 ;i<key_num; i++){
			EMCU_DEBUG("IR_KEY%u=0x%x\n",1+i,*(key_array+i));
			SET_IRDA_PARM(1+i,*(key_array+i));
		}

		pm_event_msg = GET_EVENT_MSG();
		SET_EVENT_IDX(pm_event_msg,loc_irda,key_num);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
	}
}

/************************************************************************
 *
 ************************************************************************/
#define MAX_IRKEY_NUM	(PARAM_LEN_IRDA-1)
#define MAX_IR_PARAM_CNT (MAX_IRKEY_NUM+2)

static
int powerMgr_irda_parser(const char *buf, size_t count,unsigned int ctl)
{
	int ret = 0;
	unsigned long long param[18];
	unsigned int ir_key[18];
	uint32_t user_key[18];
	int i;
	int param_cnt;
	const char* buf_pt;
	int  pmode;
	int  pmax_cnt;
	size_t cnt_tmp;

	/*
	 Because the max number of irda is 15 before.
	 The AP maybe send up to 15 set of irda key.
	 So, to reserve 18 set for safety reason.
	 */

	if(ctl == PARSE_USER){
		pmode = PR_TYPE_HEX;
		buf_pt = buf;
		cnt_tmp = count;
		/* string format of user mode:
			"protocol,key_num,keycode1,keycode2,.. > emcu_setup_irda" */
		pmax_cnt = MAX_IRKEY_NUM+2;
	}else if(ctl == PARSE_IN_CASE){
		pmode = PR_TYPE_HEX;
		buf_pt = buf;
		cnt_tmp = count;
		/* string format of key code mode:
			"key_num,keycode1,keycode2,.. > emcu_setup_irda_in_code" */
		pmax_cnt = MAX_IRKEY_NUM+1;
	}else if(ctl == PARSE_AUTO){
		if(!(emcu_test_ctl & EMCU_PARSE_EN)){
			EMCU_DEBUG("%s(%d)auto parser have not support\n",__func__,__LINE__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		pmode = PR_TYPE_PCB;
		buf_pt = EMCU_IRDA_PIN_NAME;
		cnt_tmp = sizeof(EMCU_IRDA_PIN_NAME);
		/* string format of auto mode:
			"3,PIN_IRDA_UKEY_0,PIN_IRDA_UKEY_1,PIN_IRDA_UKEY_2" */
		pmax_cnt = 4;

	}else{
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(emcu_get_param_multi(pmode,pmax_cnt,&param_cnt,param,buf_pt,cnt_tmp)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(ctl == PARSE_USER){
		if(param[1] != (param_cnt-2))
		{
			EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[1]);
			EMCU_DEBUG("input mismatch\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		for(i=0;i<param[1];i++){
			ir_key[i] = (unsigned int)param[i+2];
		}
	}else if(ctl == PARSE_IN_CASE){
		if(param[0] != (param_cnt-1))
		{
			EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
			EMCU_DEBUG("input mismatch\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		for(i=0;i<param[0];i++){
			user_key[i] = (uint32_t)param[i+1];
			EMCU_INFO("user keycode%d=0x%08x\n",i,user_key[i]);
		}
		param[1] = param[0];
		param[0] = 0;

	}else if(ctl == PARSE_AUTO){
#ifndef CONFIG_RTK_KDRV_INPUT_DEVICE
		EMCU_ALERT("irda keycode transfer function not ready\n");
		ret = -EINVAL;
		goto FINISH_OUT;
#else
		int j;

		if(param_cnt < 2){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		EMCU_DEBUG("pin cnt: %d \n",param_cnt-1);
		for(i=0;i<(param_cnt-1);i++){
			EMCU_DEBUG("pin%d: 0x%llx \n",i+1,param[i+1]);
			for(j=0;j<6;j++){
				user_key[(i*6)+j] = (param[i+1] >> (10*j))  & 0x3ff;
				EMCU_DEBUG("  ukey%d=0x%03x\n",(i*6)+j,user_key[(i*6)+j]);
			}

		}

		EMCU_DEBUG("===================================\n");
		param[0] = 0;
		param[1] = 0;
		for(i=0;i<18;i++){
			if(user_key[i] == 0){
				for(j=(i+1);j<18;j++){
					if(user_key[j]){
						user_key[i] = user_key[j];
						user_key[j] = 0;
						break;
					}
				}
			}
			EMCU_DEBUG("  ukey%d=0x%03x\n",i,user_key[i]);
			if(user_key[i]){
				param[1]+=1;
				if(param[1] >= MAX_IRKEY_NUM)
					break;
			}
		}
		EMCU_DEBUG("total ukey is %llu\n",param[1]);
#endif
	}else{
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(ctl != PARSE_USER){

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
		venus_ir_keylist_to_scanlist(user_key, (uint32_t)param[1], ir_key, (uint32_t)param[1]);
		for(i=0;i<(param[1]);i++){
			EMCU_ALERT("irda keycode%d=0x%08x\n",i,ir_key[i]);
		}
#else
		EMCU_ALERT("irda keycode transfer function not ready\n");
		ret = -EINVAL;
		goto FINISH_OUT;
#endif
	}
	powerMgr_setup_irda(
			(unsigned int)param[0],(unsigned int)param[1],ir_key);

FINISH_OUT:
	return ret;
}

#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
extern unsigned int ir_get_standby_protocol(void);
extern unsigned int ir_proto_get_boot_powerup_keylist(unsigned int proto,
								unsigned int *key_array, unsigned int arr_len);
#endif

void emcu_set_boot_cmd_irda(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
	int i;
	unsigned int pm_event_msg;
	unsigned int irda_cnt = 0;
	unsigned int keylist_len = 0;
	unsigned int vaild_irda_cnt = 0;
	unsigned int user_key[PARAM_LEN_IRDA] = {0} ;
	unsigned int ir_key[PARAM_LEN_IRDA] = {0} ;
	unsigned int  proto_standby_id;

	proto_standby_id = ir_get_standby_protocol();
	keylist_len = ir_proto_get_boot_powerup_keylist(proto_standby_id,user_key,PARAM_LEN_IRDA);

	EMCU_ALERT("standby irda protocol: %d keylist len: %d\n",
			proto_standby_id,keylist_len);
	if(new_protocol_enable == 1){
		irda_cnt = irda_valid_num;
		if(irda_cnt >= PARAM_LEN_IRDA){
			EMCU_ALERT("irda count are up to limit(%d)\n",irda_cnt);
			return ;
		}
		vaild_irda_cnt = (PARAM_LEN_IRDA-irda_cnt);
		EMCU_ALERT("vaild_irda_cnt1:%d\n",vaild_irda_cnt);

		vaild_irda_cnt = venus_ir_keylist_to_scanlist(
				user_key, keylist_len,
				ir_key, vaild_irda_cnt);
		EMCU_ALERT("vaild_irda_cnt2:%d\n",vaild_irda_cnt);

		
		/* To set protocol */
		//irda_param[0] = proto_standby_id;
		irda_arry_ext[0] = proto_standby_id;
		for(i=0; i<vaild_irda_cnt; i++){
			EMCU_ALERT("irda keycode%d=0x%08x\n",i,ir_key[i]);
			//irda_param[i+1] = ir_key[i];
			irda_arry_ext[irda_cnt+i+1] = ir_key[i];
		}
		irda_valid_num = irda_cnt + vaild_irda_cnt;
		
		EMCU_ALERT("vaild_irda_cnt3:%d\n",irda_valid_num);
	}else{
		pm_event_msg = GET_EVENT_MSG();
		irda_cnt = CHK_IRDA(pm_event_msg);

		if(irda_cnt >= PARAM_LEN_IRDA){
			EMCU_ALERT("irda count are up to limit(%d)\n",irda_cnt);
		}else{
			vaild_irda_cnt = (PARAM_LEN_IRDA-irda_cnt);
			EMCU_ALERT("vaild_irda_cnt1:%d\n",vaild_irda_cnt);
			
			vaild_irda_cnt = venus_ir_keylist_to_scanlist(
					user_key, keylist_len,
					ir_key, vaild_irda_cnt);
			EMCU_ALERT("vaild_irda_cnt2:%d\n",vaild_irda_cnt);
			
			/* To set protocol */
			SET_IRDA_PARM(0,proto_standby_id);
			for(i=0;i<(vaild_irda_cnt);i++){
				EMCU_ALERT("irda keycode%d=0x%08x\n",i,ir_key[i]);
				SET_IRDA_PARM(irda_cnt+1+i,ir_key[i]);
			}
		}
		irda_cnt = (irda_cnt+vaild_irda_cnt);
		EMCU_ALERT("irda_cnt:%d\n",irda_cnt);
		SET_EVENT_IDX(pm_event_msg,loc_irda,irda_cnt);
		SET_EVENT_MSG(pm_event_msg);
	}
#else
	EMCU_ALERT("No rtk irda driver\n");
	return;
#endif
}

/************************************************************************
 * FUNC: set IRDA key to wakeup
 * example:
 *	cat emcu_setup_irda
 *	echo protocol,key_num,keycode1,keycode2,.. > emcu_setup_irda

 ************************************************************************/
static
ssize_t emcu_setup_irda_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo protocol,key_num,keycode1,keycode2,... > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_setup_irda\"\n");
}

static
ssize_t emcu_setup_irda_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_irda_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_irda_parser(buf,count,PARSE_USER);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_IRDA_ALERT;
                emcu_suspend_event |= IRDA_SSP_EVENT;
	}

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_setup_irda, S_IRUGO | S_IWUSR,
		emcu_setup_irda_dev_show,emcu_setup_irda_dev_store);

/************************************************************************
 * FUNC: set IRDA key to wakeup, parameter in name
 * example:
 *	cat emcu_setup_irda_in_code
 *	echo key_num,keyName1,keyName2,.. > emcu_setup_irda_in_code

 ************************************************************************/
static
ssize_t emcu_setup_irda_in_code_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo key_num,keycode1,keycode2,... > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_setup_irda_in_code\"\n");
}


static
ssize_t emcu_setup_irda_in_code_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_irda_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_irda_parser(buf,count,PARSE_IN_CASE);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_IRDA_ALERT;
                emcu_suspend_event |= IRDA_SSP_EVENT;
	}

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_setup_irda_in_code, S_IRUGO | S_IWUSR,
		emcu_setup_irda_in_code_dev_show,
		emcu_setup_irda_in_code_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_setup_power_en(unsigned int pw_num,
                             unsigned long long * pw_array )
{
	unsigned int i;
	unsigned int para_tmp;
	unsigned int valid_num;
	unsigned char kptype;
	unsigned int pm_event_msg;

	EMCU_DEBUG("\n(%s)2021/11/25 21:10\n",__func__);
	EMCU_DEBUG("Power enable line have %u set.\n",pw_num);

	if(pw_num>PARAM_LEN_PWEN)
		pw_num = PARAM_LEN_PWEN;

	if(new_protocol_enable == 1){
		valid_num = 0;
		for(i=0 ;i<pw_num; i++){
			EMCU_DEBUG("en_power para%u=0x%llx\n",i,*(pw_array+i));
			EMCU_DEBUG("GET_PIN_TYPE:%x\n",GET_PIN_TYPE(*(pw_array+i)));

			kptype = GET_PIN_TYPE(*(pw_array+i));
			if( (kptype == PCB_PIN_TYPE_ISO_GPIO) ||
				(kptype == PCB_PIN_TYPE_ISO_GPI)
				|| (kptype == PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
				|| (kptype == PCB_PIN_TYPE_MIO_GPIO)
#endif
			)
			{
				para_tmp = powerMgr_interpret_gpio_value(*(pw_array+i));
				if(para_tmp == 0xff)
					continue;
				power_pin_arry_ext[valid_num]=para_tmp;
				valid_num++;
			}
		}
		power_pin_valid_num=valid_num;
	}else{
		for(i=0 ;i<PARAM_LEN_PWEN; i++)
			pw_arry_ext[i] = 0;

		valid_num = 0;
		for(i=0 ;i<pw_num; i++){
			EMCU_DEBUG("en_power para%u=0x%llx\n",i,*(pw_array+i));
			EMCU_DEBUG("GET_PIN_TYPE:%x\n",GET_PIN_TYPE(*(pw_array+i)));

			kptype = GET_PIN_TYPE(*(pw_array+i));
			if( (kptype == PCB_PIN_TYPE_ISO_GPIO) ||
				(kptype == PCB_PIN_TYPE_ISO_GPI)
				|| (kptype == PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
				|| (kptype == PCB_PIN_TYPE_MIO_GPIO)
#endif
			)
			{
				para_tmp = powerMgr_interpret_gpio_value(*(pw_array+i));
				if(para_tmp == 0xff)
					continue;
				pw_arry_ext[valid_num] = para_tmp;
				valid_num++;
			}
		}

		pm_event_msg = GET_EVENT_MSG();
		SET_EVENT_IDX(pm_event_msg,loc_pwen,valid_num);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
	}
}

int powerMgr_power_en_parser(const char *buf, size_t count,unsigned int ctl)
{
    int i;
    int ret = 0;
    int param_cnt;
    unsigned long long param[PARAM_LEN_PWEN+1] = {0};
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
    unsigned long long  rtk_tmp_gpio;
    unsigned long long ret_value;
#endif
    const char* buf_pt = NULL;
    int  pmode;
    size_t cnt_tmp;

    if(ctl == PARSE_USER){
        pmode = PR_TYPE_HEX;
        buf_pt = buf;
        cnt_tmp = count;
    }else if(ctl == PARSE_IN_CASE){
        pmode = PR_TYPE_PCB;
        buf_pt = buf;
        cnt_tmp = count;
    }else if(ctl == PARSE_AUTO){
        cnt_tmp = 0;
        for(i=0;i<PARAM_LEN_PWEN;i++){
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
            if(pcb_mgr_get_enum_info_byname(power_pin_tlb[i], &rtk_tmp_gpio)==0){
                ret_value = emcu_get_power_type_idx(power_pin_tlb[i]);
                if(ret_value == 0xFAFA)
                    continue;
                rtk_tmp_gpio |= ret_value;
                cnt_tmp++;
                param[cnt_tmp] = rtk_tmp_gpio;
                EMCU_DEBUG("%s(%d)param[%d]=0x%llx(%s)\n",
                        __func__,__LINE__,cnt_tmp,rtk_tmp_gpio,power_pin_tlb[i]);
            }
#else
			rtd_pr_emcu_alert("%s(%d)need CONFIG_REALTEK_PCBMGR support\n",__func__,__LINE__);
#endif
        }

        if(cnt_tmp == 0){
            EMCU_WARNING("No power pin defined\n");
            ret = -EINVAL;
            goto FINISH_OUT;
        }
        param[0] = param_cnt = cnt_tmp+1;
        goto AUTO_PARSE;

    }else{
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(emcu_get_param_multi(pmode,PARAM_LEN_PWEN+1,&param_cnt,param,buf_pt,cnt_tmp)){
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(ctl != PARSE_AUTO){
        /*
         * The number of parameter should same with parameter input over user.
         * It's to make sure parameter input correctly.
         */
        if(param[0] != (param_cnt-1))
        {
            EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
            EMCU_DEBUG("input mismatch\n");
            ret = -EINVAL;
            goto FINISH_OUT;
        }
    }else{
AUTO_PARSE:
        /*
         * The number of parameter may small then max number,
         * because pab paremeter have not to define.
         */
        if(param_cnt < 2)
        {
            EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
            EMCU_DEBUG("input mismatch\n");
            ret = -EINVAL;
            goto FINISH_OUT;
        }
        EMCU_DEBUG("pin cnt: %d \n",param_cnt-1);
        for(i=0;i<(param_cnt-1);i++){
            EMCU_DEBUG("pin%d: 0x%llx \n",i+1,param[i+1]);
        }
    }
    powerMgr_setup_power_en((unsigned int)(param_cnt-1),&param[1]);
    emcu_auto_event |= EMCU_POWER_EN_ALERT;

FINISH_OUT:
	return ret;
}

/************************************************************************
 * FUNC: set power pin information to EMCU
 * example:
 *	cat emcu_setup_power_en
 *	echo pw_num,pw_num,pw_num,.. > emcu_setup_power_en
 * note: max number of power pin are 3
 ************************************************************************/
static
ssize_t emcu_setup_power_en_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo pw_num,power_pin1,power_pin2,... > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_setup_power_en\"\n");
}

static
ssize_t emcu_setup_power_en_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_power_en_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_power_en_parser(buf,count,PARSE_USER);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_power_en, S_IRUGO | S_IWUSR,
		emcu_setup_power_en_dev_show,emcu_setup_power_en_dev_store);


/************************************************************************
 * FUNC: set power pin information in name to EMCU
 * example:
 *	cat setup_power_en_in_name
 *	echo pw_num,pw_name1,pw_name1,.. > setup_power_en_in_name
 * note: max number of power pin are 3,
 	To get pcb param by cmd below
 	ex. ls /sys/realtek_boards/pcb_enum
 ************************************************************************/
static
ssize_t emcu_setup_power_en_in_name_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo pw_num,pw_name1,pw_name2,... > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_setup_power_en\"\n");
}

static
ssize_t emcu_setup_power_en_in_name_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	size_t ret = 0;

	if(emcu_test_ctl & EMCU_PARSE_EN){
		ret = powerMgr_power_en_parser(buf,count,PARSE_AUTO);
	}else{
		ret = powerMgr_power_en_parser(buf,count,PARSE_IN_CASE);
	}

	if(ret){
		ret = -EINVAL;
		goto FINISH_OUT;
	}else{
		ret = count;
		emcu_auto_event |= EMCU_POWER_EN_ALERT;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_setup_power_en_in_name, S_IRUGO | S_IWUSR,
		emcu_setup_power_en_in_name_dev_show,
		emcu_setup_power_en_in_name_dev_store);

/************************************************************************
 * FUNC: get how long sleep from emcu
 * example:
 *	cat emcu_get_sleep_dur
 *
 ************************************************************************/
static
ssize_t emcu_get_sleep_dur_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	unsigned int dur_h;
	unsigned int dur_l;
	unsigned int dur_ary[3];
	int res;
	unsigned int ret_data[4] = {0};

	dur_h = dur_l = 0;
	if(new_protocol_enable == 1){
		res = send_packet(EMCU_WAKE_UP_SOURCE, 0, NULL, ret_data, 0, 1);
	 	if(res < 0){
			EMCU_ERR("get_sleep_dur fialed=%d\n", res);
		} 
		if(res>0){
			dur_h = ret_data[2];  //dur
			dur_l = ret_data[3];  //SysTickSec
		}
		
		EMCU_ERR("get_sleep_dur data_len=%d dur_h=%u dur_l=%u\n", res, dur_h, dur_l);
	}else{
		GET_TIME_DUR(dur_h,dur_l);
	}
	dur_ary[0] = dur_l;                    //min
	dur_ary[1] = (dur_h >> 24) & 0xff;     //sec
	dur_ary[2] = dur_h & 0xffffff;         //ms
	EMCU_INFO("\nsleep duration: %umin %usec %ums\n",
		dur_ary[0],dur_ary[1],dur_ary[2]);

    return snprintf(buf, 128, "%u,%u,%u\n",dur_ary[0],dur_ary[1],dur_ary[2]);
}

static
ssize_t emcu_get_sleep_dur_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	EMCU_INFO("\n(%s)no operation!\n",__func__);

	return count;
}

DEVICE_ATTR(emcu_get_sleep_dur, S_IRUGO | S_IWUSR,
		emcu_get_sleep_dur_dev_show,emcu_get_sleep_dur_dev_store);

/************************************************************************
 *
 ************************************************************************/
int powerMgr_set_wakeup_source_undef(void)
{
    EMCU_INFO("%s(%d)\n",__func__,__LINE__);
    if(new_protocol_enable == 0){
        local_sharemem_res[IDX_REG_WK_SOR] = WKSOR_UNDEF;
        local_sharemem_res[IDX_REG_WK_STS] = WKSOR_UNDEF;
    }else{
        EMCU_INFO("%s(%d) new protocol don't need set source undef\n",__func__,__LINE__);
    }
    return 0;
}
EXPORT_SYMBOL(powerMgr_set_wakeup_source_undef);

static
ssize_t emcu_set_wakeup_source_undef_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned long long in_val;
    size_t ret = count;

    if(emcu_get_param_single(16,&in_val,buf,count)){
        ret = -EINVAL;
        goto FINISH_OUT;
    }
    if(in_val == 1){
        powerMgr_set_wakeup_source_undef();
    }else{
FINISH_OUT:
        EMCU_INFO("\n(%s)unsupported paramerter!\n",__func__);
    }
    return count;
}

static
ssize_t emcu_set_wakeup_source_undef_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  1 > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_set_wakeup_source_undef\"\n");
}
DEVICE_ATTR(emcu_set_wakeup_source_undef, S_IRUGO | S_IWUSR,
        emcu_set_wakeup_source_undef_dev_show,emcu_set_wakeup_source_undef_dev_store);

/************************************************************************
 * FUNC: get wake up source  from emcu
 * example:
 *	cat emcu_get_wakeup_source
 ************************************************************************/
int powerMgr_get_wakeup_source(unsigned int* row, unsigned int* status)
{
	unsigned int ret_data[EMCU_DAT_LEN_MAX]= {0};
    int res = 0;
    unsigned int tmp1 = 0;
    unsigned int tmp2 = 0;
    const char* str[]={"UNDEF","KEYPAD","WUT","IRDA","CEC","PPS","WOW","HML","RTC","WOV","WKSOR_EWBS"};
    WAKE_UP_T reason;

    if((row == NULL ) && (status == NULL)){
        return -ENOMEM;
    }
	if(new_protocol_enable == 1){
		res = send_packet(EMCU_WAKE_UP_SOURCE, 0, NULL,ret_data, 0, 1);
		if(res < 0){
			EMCU_ERR("get_wakeup_source fialed=%d\n", res);
		} 
		EMCU_ERR("get_wakeup_source data_len=%d\n", res);
		if(res!=0){
			tmp2 = ret_data[0];  //status
			tmp1 = ret_data[1];  //source
		}
	}else{
	    tmp1 = local_sharemem_res[IDX_REG_WK_SOR];
	    tmp2 = local_sharemem_res[IDX_REG_WK_STS];
	}
    if(row)
        *row = tmp1;
    if(status)
        *status = tmp2;

    reason = (WAKE_UP_T)(tmp1 & 0xff);
    if((reason) >= WKSOR_END){
        res = -EINVAL;
        goto ERR_OUT;
    }

    EMCU_INFO("\n%s(%d)wakeup source:%s; row:0x%08x; status:0x%08x\n",
        __func__,__LINE__,str[reason],tmp1,tmp2);

ERR_OUT:
    return res;

}
EXPORT_SYMBOL(powerMgr_get_wakeup_source);

static
ssize_t emcu_get_wakeup_source_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	unsigned int ir_key = 0;
	unsigned int wk_source = 0;

	powerMgr_get_wakeup_source(&wk_source,&ir_key);

    return snprintf(buf, 128, "0x%x\n",wk_source);
}

static
ssize_t emcu_get_wakeup_source_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{

	EMCU_INFO("\n(%s)no operation!\n",__func__);
	return count;
}

DEVICE_ATTR(emcu_get_wakeup_source, S_IRUGO | S_IWUSR,
		emcu_get_wakeup_source_dev_show,emcu_get_wakeup_source_dev_store);

/************************************************************************
 * FUNC: get wake up status
 * example:
 *	cat emcu_get_wakeup_status
 ************************************************************************/
static
ssize_t emcu_get_wakeup_status_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	unsigned int wk_status=0;
	int res;
	unsigned int ret_data[EMCU_DAT_LEN_MAX]= {0};

	if(new_protocol_enable == 1){
		res = send_packet(EMCU_WAKE_UP_SOURCE, 1, NULL, ret_data, 0, 1);
		if(res < 0){
			EMCU_ERR("get_wakeup_status fialed=%d\n", res);
		} 
		if(res!=0)
			wk_status = ret_data[0];
	}else{
		wk_status = GET_WK_STS();
	}
	EMCU_INFO("%s(%u)wakeup status = 0x%x\n", __func__, __LINE__,wk_status);
    return snprintf(buf, 128, "0x%x\n",wk_status);
}

static
ssize_t emcu_get_wakeup_status_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	EMCU_INFO("\n(%s)no operation!\n",__func__);
	return count;
}

DEVICE_ATTR(emcu_get_wakeup_status, S_IRUGO | S_IWUSR,
		emcu_get_wakeup_status_dev_show,emcu_get_wakeup_status_dev_store);

/************************************************************************
 *  * FUNC: get wake up status(keycode)
 *   * example:
 *    *	cat emcu_get_wakeup_status_keycode
 *     ************************************************************************/
static
ssize_t emcu_get_wakeup_status_keycode_dev_show(struct device *dev,
			struct device_attribute *attr,
				char *buf)
{
	unsigned int wk_status=0;
	unsigned int ret_data[EMCU_DAT_LEN_MAX]= {0};
	unsigned int  keycode = 0;
	int res;

	if(new_protocol_enable == 1){
		res = send_packet(EMCU_WAKE_UP_SOURCE, 1, NULL, ret_data, 0, 1);
		if(res < 0){
			EMCU_ERR("get_wakeup_status fialed=%d\n", res);
		}
		if(res!=0)
			wk_status = ret_data[0];
	}else{
		wk_status = GET_WK_STS();
	}

	keycode = venus_ir_scancode_to_keycode(wk_status);
	EMCU_INFO("%s(%u)wakeup status = 0x%x keycode = %x\n", __func__, __LINE__,wk_status, keycode);
	return snprintf(buf, 128, "0x%x\n",keycode);
}

static
ssize_t emcu_get_wakeup_status_keycode_dev_store(struct device *dev,
			struct device_attribute *attr,
				const char *buf, size_t count)
{
	EMCU_INFO("\n(%s)no operation!\n",__func__);
	return count;
}
DEVICE_ATTR(emcu_get_wakeup_status_keycode, S_IRUGO | S_IWUSR,
				emcu_get_wakeup_status_keycode_dev_show,emcu_get_wakeup_status_keycode_dev_store);


/************************************************************************
 *  * FUNC: set led
 *   * example:
 *   *	cat emcu_set_led
 *   *  set 1 > emcu_set_led_flash
 *     ************************************************************************/
static ssize_t emcu_set_led_flash_dev_show(struct device *dev,
			struct device_attribute *attr,
				char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  1 > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_set_led_flash\"\n");
}

static ssize_t emcu_set_led_flash_dev_store(struct device *dev,
			struct device_attribute *attr,
				const char *buf, size_t count)
{

	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val){
		emcu_suspend_event |= LED_FLASH_EVENT;
		EMCU_INFO("\n(%s)led flash event set!\n",__func__);
	}else{
		emcu_suspend_event &= ~LED_FLASH_EVENT;
	}
FINISH_OUT:
	return ret;

}
DEVICE_ATTR(emcu_set_led_flash, S_IRUGO | S_IWUSR,
				emcu_set_led_flash_dev_show, emcu_set_led_flash_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_set_cur_time(int hours,int mins,int sec)
{
	unsigned int cur_time;
	unsigned int time_param[EMCU_DAT_LEN_MAX];
	unsigned int time_mod;
	int ret;
	unsigned int record_tme;

	if(hours > 23)
		hours = 23;
	if(mins > 59)
		mins = 59;
	if(sec > 59)
		sec = 59;
	if(new_protocol_enable == 1){
		cur_time = (hours<<16)|(mins<<8)|(sec&0xFF);
		time_param[0]=cur_time;
		time_mod=2;
		record_tme=1;
		ret = send_packet(EMCU_TIMER, ((record_tme&0x1)<<2)|(time_mod&0x3), time_param, NULL, 1, 0);
		if(ret != EMCU_CMD_DONE){
			EMCU_ERR("cur_time set fialed=%d\n",ret);
		}	
	}else{
		cur_time = (hours<<24)|(mins<<16)|(sec<<8);
		SET_CUR_TIME(cur_time);
	}
	EMCU_DEBUG("\nrecord current time : %dhour, %dmin, %dsec\n",
		hours,mins,sec);

}

/************************************************************************
 * FUNC: set current time to emcu
 * example:
 *	cat emcu_set_cur_time
 *	echo  hours,mins,sec > emcu_set_cur_time
 ************************************************************************/
static
ssize_t emcu_set_cur_time_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  hours,mins,sec > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_cur_time\"\n"
		"        Noteice: \"hours,mins,sec\" are DEC format\n");
}

static
ssize_t emcu_set_cur_time_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[3];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_DEC,3,&param_cnt,param,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(param_cnt != 3){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_set_cur_time((int)param[0],(int)param[1],(int)param[2]);

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_set_cur_time, S_IRUGO | S_IWUSR,
		emcu_set_cur_time_dev_show,emcu_set_cur_time_dev_store);

/************************************************************************
 * FUNC: get recorded time from emcu
 * example:
 *	cat emcu_get_rec_time
 *
 ************************************************************************/
static
ssize_t emcu_get_rec_time_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	unsigned int cur_time;
	unsigned int ret_data[EMCU_DAT_LEN_MAX];
	unsigned int time_mod;
	int ret;
	unsigned int record_tme;

	if(new_protocol_enable == 1){
		time_mod=2;
		record_tme=1;
		ret = send_packet(EMCU_TIMER, ((record_tme&0x1)<<2)|(time_mod&0x3), NULL, ret_data, 0, 1);
		if(ret<0){
			EMCU_ERR("cur_time get fialed=%d\n",ret);
		}
		if(ret == 1){
			cur_time = ret_data[0];
		}else{
			EMCU_INFO("not set cur time\n");
			cur_time = 0;
		}
	}else{
		cur_time = GET_CUR_TIME();
	}
	cur_time = (cur_time>>8);

	EMCU_INFO("\nrecorded time : %uhour, %umin, %usec\n",
		(cur_time>>16)& 0xff,
		(cur_time>> 8)& 0xff,
		(cur_time>> 0)& 0xff);

    return snprintf(buf, 128, "%x\n",cur_time);
}

static
ssize_t emcu_get_rec_time_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	EMCU_INFO("\n(%s)no operation!\n",__func__);
	return count;
}

DEVICE_ATTR(emcu_get_rec_time, S_IRUGO | S_IWUSR,
		emcu_get_rec_time_dev_show,emcu_get_rec_time_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_set_sharemem(unsigned char addr,unsigned int value)
{
	if(addr>31){
		EMCU_DEBUG("The address is to much.\n");
	}else{
		rtd_outl(RTD_SHARE_MEM_BASE+(4*addr),value);

		EMCU_DEBUG("\nthe share_memory %u = 0x%08x",
			addr,rtd_inl(RTD_SHARE_MEM_BASE+4*addr));
	}
}

/************************************************************************
 * FUNC: set expect value to certent share_mem
 * example:
 *	cat emcu_set_sharemem
 *	echo addr,value > emcu_set_sharemem
 *
 ************************************************************************/
static
ssize_t emcu_set_sharemem_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  addr,value > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_sharemem\"\n");
}

static
ssize_t emcu_set_sharemem_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[2];
	size_t ret = count;

	if(emcu_get_param_multi(PR_TYPE_HEX,2,&param_cnt,param,buf,count)){
		EMCU_INFO("\n(%s)get param fail\n",__func__);
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 2){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	powerMgr_set_sharemem((unsigned char)param[0],(unsigned int)param[1]);

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_set_sharemem, S_IRUGO | S_IWUSR,
		emcu_set_sharemem_dev_show,emcu_set_sharemem_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
int powerMgr_set_sharemem_mask(
	unsigned char addr,unsigned int mask,unsigned int value)
{
	int ret = 0;
	if(addr>31){
		EMCU_DEBUG("The address is to much.\n");
		ret = -EINVAL;;
	}else{
		unsigned int reginfo;

		reginfo = rtd_inl(RTD_SHARE_MEM_BASE+(4*addr));
		reginfo = ((reginfo & mask)|(value & ~mask));
		rtd_outl(RTD_SHARE_MEM_BASE+(4*addr),reginfo);

		EMCU_DEBUG("\nthe share_memory %u = 0x%08x",
			addr,rtd_inl(RTD_SHARE_MEM_BASE+4*addr));
	}
	return ret;
}

/************************************************************************
 * FUNC: set expect value to certent share_mem with mask
 * example:
 *	cat emcu_set_sharemem_mask
 *	echo addr,mask,value > emcu_set_sharemem_mask
 *
 ************************************************************************/
static
ssize_t emcu_set_sharemem_mask_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  addr,mask,value > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_sharemem_mask\"\n");
}

static
ssize_t emcu_set_sharemem_mask_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[3];
	size_t ret = 0;

	if(emcu_get_param_multi(PR_TYPE_HEX,3,&param_cnt,param,buf,count)){
		EMCU_INFO("\n(%s)get param fail\n",__func__);
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 3){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	ret = powerMgr_set_sharemem_mask(
				(unsigned char)param[0],
				(unsigned int) param[1],
				(unsigned int) param[2]);

	if(ret == 0)
		ret = count;

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(emcu_set_sharemem_mask, S_IRUGO | S_IWUSR,
		emcu_set_sharemem_mask_dev_show,emcu_set_sharemem_mask_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
int powerMgr_set_STM(unsigned int value)
{
	int ret;
	unsigned int pm_event_msg;
	unsigned int stm_tmp;

	if(value >3){
		EMCU_DEBUG("\nparameter big then 3. Ignore setting.\n");
		return -1;
	}else{
		if(new_protocol_enable == 1){
			ret = send_packet(EMCU_STR, value, NULL, NULL, 0, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("STM set fialed=%d\n",ret);
			}		
		}else{
			pm_event_msg = GET_EVENT_MSG();
			stm_tmp = (((pm_event_msg >> loc_stm) &0xf) & (~0x03));
			SET_EVENT_IDX(pm_event_msg,loc_stm,(value|stm_tmp));
			SET_EVENT_MSG(pm_event_msg);
			EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		}
		/*
		 To make sure power pin of DDR has been remove form share mem.
		 Call power_en related func here.
		 */
		if(!(emcu_auto_event & EMCU_POWER_EN_ALERT)){
			if(powerMgr_power_en_parser(NULL,0,PARSE_AUTO)){
				EMCU_WARNING("%s(%d)power_en auto parse fail!\n",__func__,__LINE__);
			}
		}
		stm_flag = value;

		return 0;
	}
}

/************************************************************************
 * FUNC: enable suspend to RAM
 * example:
 *	cat emcu_set_STM
 *	echo 1 > emcu_set_STM
 ************************************************************************/
static
ssize_t emcu_set_STM_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  smt_val > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_STM\"\n"
		"        notice: smt_val is \"1\" now\n");
}

static
ssize_t emcu_set_STM_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(powerMgr_set_STM((unsigned int)in_val)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_set_STM, S_IRUGO | S_IWUSR,
		emcu_set_STM_dev_show,emcu_set_STM_dev_store);


/************************************************************************
 *
 ************************************************************************/
static
void powerMgr_set_RTC(unsigned int in_val)
{
	int ret;
	unsigned int pm_event_msg;
	unsigned int stm_tmp;

	if(new_protocol_enable == 1){
		ret = send_packet(EMCU_RTC, in_val, NULL, NULL, 0, 0);
		if(ret != EMCU_CMD_DONE){
			EMCU_ERR("RTC set fialed=%d\n",ret);
		}		 
	}else{
		pm_event_msg = GET_EVENT_MSG();
		stm_tmp = (((pm_event_msg >> loc_rtc) &0xf) | 0x4); //use bit2, index bit30
		SET_EVENT_IDX(pm_event_msg,loc_rtc,stm_tmp);
		SET_EVENT_MSG(pm_event_msg);
		EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
	}
}

/************************************************************************
 * FUNC: enable RTC wake up
 * example:
 *	cat emcu_set_RTC
 *	echo 1 > emcu_set_RTC
 ************************************************************************/
static
ssize_t emcu_set_RTC_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo 1 > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_RTC\"\n");
}

static
ssize_t emcu_set_RTC_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val > 1){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	powerMgr_set_RTC(in_val);

FINISH_OUT:
	return count;

}

DEVICE_ATTR(emcu_set_RTC, S_IRUGO | S_IWUSR,
		emcu_set_RTC_dev_show,emcu_set_RTC_dev_store);


/************************************************************************
 *
 ************************************************************************/
/* #define INTERNAL_WOW_SUPPORT */
#ifdef INTERNAL_WOW_SUPPORT
static
void powerMgr_set_WOW(unsigned int value,
                      unsigned char *ptr_sip,
                      unsigned char *ptr_hip,
                      unsigned char *ptr_smac,
                      unsigned char *ptr_hmac)
{
	unsigned int ip_param[EMCU_DAT_LEN_MAX];
	int ret;
	unsigned int pm_event_msg;

	EMCU_DEBUG("value=%x\n",value);
	EMCU_DEBUG("server IP : %02x:%02x:%02x:%02x (%u:%u:%u:%u)\n",
		*(ptr_sip+0),*(ptr_sip+1),*(ptr_sip+2),*(ptr_sip+3),
		*(ptr_sip+0),*(ptr_sip+1),*(ptr_sip+2),*(ptr_sip+3));
	EMCU_DEBUG("host   IP : %02x:%02x:%02x:%02x (%u:%u:%u:%u)\n",
		*(ptr_hip+0),*(ptr_hip+1),*(ptr_hip+2),*(ptr_hip+3),
		*(ptr_hip+0),*(ptr_hip+1),*(ptr_hip+2),*(ptr_hip+3));
	EMCU_DEBUG("server MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		*(ptr_smac+0),*(ptr_smac+1),*(ptr_smac+2),
		*(ptr_smac+3),*(ptr_smac+4),*(ptr_smac+5));
	EMCU_DEBUG("host   MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		*(ptr_hmac+0),*(ptr_hmac+1),*(ptr_hmac+2),
		*(ptr_hmac+3),*(ptr_hmac+4),*(ptr_hmac+5));

	if(value >1){
		EMCU_DEBUG("\nparameter big then 1. Ignore setting.\n");
	}
	else{
		if(new_protocol_enable == 1){
			ip_param[0]= (*(ptr_sip+0)<<24)|(*(ptr_sip+1)<<16)|(*(ptr_sip+2)<<8)|(*(ptr_sip+3))))
			ip_param[1] (*(ptr_hip+0)<<24)|(*(ptr_hip+1)<<16)|(*(ptr_hip+2)<<8)|(*(ptr_hip+3))))
			ip_param[2]= (*(ptr_smac+0)<<24)|(*(ptr_smac+1)<<16)|(*(ptr_smac+2)<<8)|(*(ptr_smac+3))))
			ip_param[3]= (*(ptr_hmac+0)<<24)|(*(ptr_hmac+1)<<16)|(*(ptr_hmac+2)<<8)|(*(ptr_hmac+3))))

			ret = send_packet(EMCU_WOW, 0, &ip_param, NULL, 4, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("WOW set fialed=%d\n",ret);
			}
		}else{
			SET_SVER_IP(ptr_sip);
			SET_SVER_MAC(ptr_smac);
			SET_HOST_IP(ptr_hip);
			SET_HOST_MAC(ptr_hmac);

			pm_event_msg = GET_EVENT_MSG();
			pm_event_msg |= ((unsigned int)0x1 << loc_wow); // bit0:WOW enable
			SET_EVENT_MSG(pm_event_msg);

			EMCU_DEBUG("pm_event_msg=0x%x\n",pm_event_msg);
		}
	}
}

/************************************************************************
 * FUNC: set WOW parameter
 * example:
 *	cat emcu_set_WOW
 *	echo value,sip,hip,smac,hmac > emcu_set_WOW
 *      echo 4,168.32.1.12,172.55.20.5,33:44:55:66:77:88,12:34:56:78:aa:bb > /sys/devices/virtual/misc/emcu_kdv/emcu_set_WOW
 *
 ************************************************************************/
static
ssize_t emcu_set_WOW_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  value,sip,hip,smac,hmac > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_WOW\"\n"
		"        notice: sip/hip in format \"xxx.xxx.xxx.xxx\"\n"
		"                smac/hmac in format \"xx:xx:xx:xx:xx:xx\"\n\n"
		"example  \"echo 4,168.32.1.12,172.55.20.5,33:44:55:66:77:88,12:34:56:78:aa:bb > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_WOW\"\n");
}

static
ssize_t emcu_set_WOW_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int i,j;
	int len;
	char *opt = NULL;
	char *buftmp;
	char *buforg;
	char *IP_buf;
	char *ip_buf;
	unsigned long long tmp_long;
	unsigned long long param[1];
	unsigned char ip_tmp[4][6];
	size_t ret = count;

	buforg = buftmp = kmalloc(count, GFP_KERNEL);
	ip_buf = kmalloc(64, GFP_KERNEL);
	if(!buftmp || !ip_buf){
		EMCU_DEBUG( "buftmp is NULL\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	memset(buftmp,0,count);
	strcpy(buftmp, buf);

	i = 0;
	while ((opt = strsep(&buftmp, ",")) != NULL) {
		len = strlen(opt);
		if(i){
			unsigned int base;
			unsigned int max_cnt;
			char *delim;
			if((i==1) || (i==2)){
				base = 10;
				max_cnt = 4;
				delim =  ".";
			}else{
				base = 16;
				max_cnt = 6;
				delim =  ":";
			}

			j = 0;
			memset(ip_buf,0,64);
			IP_buf = ip_buf;
			strcpy(ip_buf, opt);
			while ((opt = strsep(&IP_buf, delim)) != NULL) {
				if (kstrtoull(opt, base, &tmp_long)){
					EMCU_DEBUG("get param fail 1\n");
					ret = -EINVAL;
					goto FINISH_OUT;
				}else{
					ip_tmp[i-1][j] = (unsigned char)tmp_long;
				}
				j++;
			}
			if(j != max_cnt){
				EMCU_DEBUG("get param fail 2\n");
				ret = -EINVAL;
				goto FINISH_OUT;
			}
#if 0
			if(max_cnt == 4)
				EMCU_INFO("IP[%d]: %u.%u.%u.%u\n",
					i,ip_tmp[i-1][0],ip_tmp[i-1][1],ip_tmp[i-1][2],ip_tmp[i-1][3]);
			else
				EMCU_INFO("MAC[%d]: %x:%x:%x:%x:%x:%x\n",
					i,
					ip_tmp[i-1][0],ip_tmp[i-1][1],ip_tmp[i-1][2],
					ip_tmp[i-1][3],ip_tmp[i-1][4],ip_tmp[i-1][5]);
#endif
		}else{
			if (kstrtoull(opt, 16, &tmp_long)){
				EMCU_DEBUG("get param fail 3\n");
				ret = -EINVAL;
				goto FINISH_OUT;
			}else{
				param[i] = tmp_long;
			}
			EMCU_INFO("param[%d]=0x%llx\n",i,param[i]);
		}
		i++;
	}

	if(i != 5){
		EMCU_DEBUG("param count mismatch\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	powerMgr_set_WOW(param[0],
			(unsigned char*)&ip_tmp[0],
			(unsigned char*)&ip_tmp[1],
			(unsigned char*)&ip_tmp[2],
			(unsigned char*)&ip_tmp[3]);

FINISH_OUT:
	if(buforg)
		kfree(buforg);
	if(ip_buf)
		kfree(ip_buf);

	return ret;
}

DEVICE_ATTR(emcu_set_WOW, S_IRUGO | S_IWUSR,
		emcu_set_WOW_dev_show,emcu_set_WOW_dev_store);

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_set_WOW_chksum_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,
		"please key in \"echo  chksum > "
		"/sys/devices/virtual/misc/emcu_kdv/emcu_set_WOW_chksum\"\n");
}

static
ssize_t emcu_set_WOW_chksum_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long long in_val;
	size_t ret = count;

	if(emcu_get_param_single(16,&in_val,buf,count)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	SET_WOW_CHKSUM((unsigned int)in_val);
        emcu_suspend_event |= WOW_SSP_EVENT;

FINISH_OUT:
	return ret;

}

DEVICE_ATTR(emcu_set_WOW_chksum, S_IRUGO | S_IWUSR,
		emcu_set_WOW_chksum_dev_show,emcu_set_WOW_chksum_dev_store);
#endif	//#ifdef INTERNAL_WOW_SUPPORT

#ifdef CONFIG_MULTI_IR_KEY

/************************************************************************
 * set GPO func, for setting power enable pin used.
 * max number of power enable pin are 4 sets.
 ************************************************************************/
static
ssize_t emcu_reno_gpo_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  cnt,GPO1,GPO2,GPO3,GPO4 > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_gpo\"\n");
}

static
ssize_t emcu_reno_gpo_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int tmp;
    unsigned int reg_val;
    unsigned int index;
    unsigned int invert;
    unsigned int param_cnt;
    unsigned long long param[5] = {0};
    size_t ret = 0;
    int i;

    if(emcu_get_param_multi(PR_TYPE_HEX,5,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 5){
        ret = -EINVAL;
        goto FINISH_OUT;
    }
    if(param[0] != (param_cnt-1)){
        EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
        EMCU_DEBUG("input mismatch\n");
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    reg_val = 0;
    param_cnt = 0;
    for(i=0;i<param[0];i++){
        if(GET_PIN_TYPE(param[i+1]) != PCB_PIN_TYPE_ISO_GPIO){
            EMCU_ALERT("%s(%d)unvaild gpo pcb param 0x%llx\n",
                    __func__,__LINE__,param[i+1]);
            continue;
        }
        param_cnt++;
        index   = GET_PIN_INDEX(param[i+1]);
        invert  = GET_GPIO_INVERT(param[i+1]);
        EMCU_DEBUG("%s(%d)index=0x%x,invert=0x%x\n",__func__,__LINE__,index,invert);
        tmp = index|(((invert)?1:0)<<7);
        EMCU_DEBUG("%s(%d)reg_val=0x%x,tmp=0x%x\n",__func__,__LINE__,reg_val,tmp);
        reg_val = (reg_val << 8) | tmp;
        EMCU_DEBUG("%s(%d)reg_val=0x%x,tmp=0x%x\n",__func__,__LINE__,reg_val,tmp);
    }
    EMCU_DEBUG("%s(%d)reg_val=0x%x\n",__func__,__LINE__,reg_val);
    rtd_outl(IDX_REG_GPO,reg_val);

    pm_event_msg = GET_EVENT_MSG() & ~RENO_GPO_MASK;
    pm_event_msg |= (param_cnt << SHT_RENO_GPO);
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= GPIO_SSP_EVENT;

    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_gpo, S_IRUGO | S_IWUSR,
        emcu_reno_gpo_dev_show,emcu_reno_gpo_dev_store);


/************************************************************************
 * set LSADC func, for setting keypad used.
 * max number of power enable pin are 2 sets.
 ************************************************************************/
static
ssize_t emcu_reno_lsadc_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  cnt,LSADC1,LSADC2 > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_lsadc\"\n");
}

static
ssize_t emcu_reno_lsadc_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int tmp;
    unsigned int reg_val;
    unsigned int kpidx;
    unsigned int hit_val;
    unsigned int tolerance;
    unsigned int param_cnt;
    unsigned long long param[3] = {0};
    size_t ret = 0;
    int i;

    if(emcu_get_param_multi(PR_TYPE_HEX,3,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 3){
        ret = -EINVAL;
        goto FINISH_OUT;
    }
    if(param[0] != (param_cnt-1)){
        EMCU_DEBUG("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
        EMCU_DEBUG("input mismatch\n");
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    reg_val = 0;
    param_cnt = 0;
    for(i=0;i<param[0];i++){
        if(GET_PIN_TYPE(param[i+1]) != PCB_PIN_TYPE_LSADC){
            EMCU_ALERT("%s(%d)unvaild lsadc pcb param 0x%llx\n",
                __func__,__LINE__,param[i+1]);
            continue;
        }
        param_cnt++;
        tolerance = GET_LSADC_TOLERANCE(param[i+1]) & 0x1f; //tolerance value (0~31)
        kpidx  = GET_PIN_INDEX(param[i+1]) & 0x7;           //lsadc set (0~7)
        hit_val = GET_LSADC_HITVALUE(param[i+1]) & 0xff;    //hit value (0~255)
        tmp = (hit_val<<8) | (kpidx<<5) | (tolerance);
        reg_val = (reg_val << 16) | tmp;
    }
    rtd_outl(IDX_REG_LSADC,reg_val);

    pm_event_msg = GET_EVENT_MSG() & ~RENO_LSADC_MASK;
    pm_event_msg |= (param_cnt << SHT_RENO_LSADC);
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= LSADC_SSP_EVENT;
    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_lsadc, S_IRUGO | S_IWUSR,
        emcu_reno_lsadc_dev_show,emcu_reno_lsadc_dev_store);

/************************************************************************
 * set wake on LAN func.
 * param1 : enable/disable;1:eanble/other:disable
 * param2 : PCB parameter if need
 ************************************************************************/
static
ssize_t emcu_reno_wol_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  eanble_flag,pcb_param > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_wol\"\n");
}

static
ssize_t emcu_reno_wol_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int tmp;
    unsigned int reg_val;
    unsigned int index;
    unsigned int invert;
    unsigned int param_cnt;
    unsigned long long param[2] = {0};
    size_t ret = 0;


    if(emcu_get_param_multi(PR_TYPE_HEX,2,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 2){
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    pm_event_msg = GET_EVENT_MSG();

    pm_event_msg &= ~(RENO_WOL_MASK|RENO_GPI_MASK);
    if(param[0] == 1){
        if( (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPIO)
         && (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPI)
         && (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
         &&(GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_MIO_GPIO)
#endif
         ){
            EMCU_ALERT("%s(%d)unvaild wol pcb param 0x%llx\n",
                __func__,__LINE__,param[1]);
            ret = -EINVAL;
            goto FINISH_OUT;
        }
        /*
         WOL func may have enable flag only case
         So, we aleert enable flag anyway, no mater PCB ino is exist or not.
         */
        pm_event_msg |= RENO_WOL_MASK;
        if(param[1] && (reno_gpi_cnt<4)){

            index   = GET_PIN_INDEX(param[1]);
            invert  = GET_GPIO_INVERT(param[1]);

            tmp = index|(((invert)?1:0)<<7);
            reg_val = rtd_inl(IDX_REG_GPI) & ~(0xffU << (reno_gpi_cnt*8));
            reg_val = reg_val | (tmp << (reno_gpi_cnt*8));
            reno_gpi_cnt += 1;
            rtd_outl(IDX_REG_GPI,reg_val);
        }
    }else{
        /*nothing to do here? */
    }
    pm_event_msg |= (reno_gpi_cnt<<SHT_RENO_GPI);
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= WOL_SSP_EVENT;
    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_wol, S_IRUGO | S_IWUSR,
        emcu_reno_wol_dev_show,emcu_reno_wol_dev_store);

/************************************************************************
 * set wake on voice func.
 * param1 : enable/disable,1:eanble/other:disable
 * param2 : PCB parameter if need
 ************************************************************************/
static
ssize_t emcu_reno_wov_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  eanble_flag,pcb_param > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_wov\"\n");
}

static
ssize_t emcu_reno_wov_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int tmp;
    unsigned int reg_val;
    unsigned int index;
    unsigned int invert;
    unsigned int param_cnt;
    unsigned long long param[2] = {0};
    size_t ret = 0;


    if(emcu_get_param_multi(PR_TYPE_HEX,2,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 2){
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    pm_event_msg = GET_EVENT_MSG();

    pm_event_msg &= ~(RENO_WOV_MASK|RENO_GPI_MASK);
    if(param[0] == 1){
        if( (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPIO)
         && (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPI)
         && (GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_ISO_GPIO_MIO)
#ifdef CONFIG_RTK_KDRV_MIO
         &&(GET_PIN_TYPE(param[1]) != PCB_PIN_TYPE_MIO_GPIO)
#endif
         ){
            EMCU_ALERT("%s(%d)unvaild wol pcb param 0x%llx\n",
                __func__,__LINE__,param[1]);
            ret = -EINVAL;
            goto FINISH_OUT;
        }
        /*
         WOVs func may have enable flag only case
         So, we aleert enable flag anyway, no mater PCB ino is exist or not.
         */
        pm_event_msg |= RENO_WOV_MASK;
        if(param[1] && (reno_gpi_cnt<4)){

            index   = GET_PIN_INDEX(param[1]);
            invert  = GET_GPIO_INVERT(param[1]);

            tmp = index|(((invert)?1:0)<<7);
            reg_val = rtd_inl(IDX_REG_GPI) & ~(0xffU << (reno_gpi_cnt*8));
            reg_val = reg_val | (tmp << (reno_gpi_cnt*8));
            reno_gpi_cnt += 1;
            rtd_outl(IDX_REG_GPI,reg_val);
        }
    }else{
        /*nothing to do here? */
    }
    pm_event_msg |= (reno_gpi_cnt<<SHT_RENO_GPI);
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= WOV_SSP_EVENT;
    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_wov, S_IRUGO | S_IWUSR,
        emcu_reno_wov_dev_show,emcu_reno_wov_dev_store);


/************************************************************************
 * To set CEC wakeup func.
 * param1 : enable/disable;1:eanble/0:disable/other:reserve.
  ************************************************************************/
static
ssize_t emcu_reno_cec_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  eanble_flag > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_cec\"\n");
}

static
ssize_t emcu_reno_cec_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int param_cnt;
    unsigned long long param[1] = {0};
    size_t ret = 0;

    if(emcu_get_param_multi(PR_TYPE_HEX,1,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 1){
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    pm_event_msg = GET_EVENT_MSG();

    pm_event_msg &= ~(RENO_CEC_MASK);
    if(param[0] == 1){
        pm_event_msg |= RENO_CEC_MASK;
    }else{
        /*nothing to do here? */
    }
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= CEC_SSP_EVENT;
    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_cec, S_IRUGO | S_IWUSR,
        emcu_reno_cec_dev_show,emcu_reno_cec_dev_store);

/************************************************************************
 * To set IRDA wakeup func.
 * param1   : number of IRDA key set.
 * param2   : default protocal 1.
 * param3   : default protocal 2.
 * param4~67: IRDA key set 1~64.
  ************************************************************************/
static
ssize_t emcu_reno_irda_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 256,
        "please key in \"echo  cnt,pt1,pt2,ir_key1,...ir_key64 > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_irda\"\n");
}

static
ssize_t emcu_reno_irda_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned int tmp;
    unsigned int param_cnt;
    unsigned int ir_key;
    unsigned int ir_header;
    unsigned int tmp_ir_key;
    unsigned int tmp_ir_header;
    //unsigned int reg_val;
    unsigned long long param[67] = {0};
    size_t ret = 0;
    int i;
    u32 reg_tmp;

    EMCU_DEBUG("\n(%s)count : %d\n",__func__,count);
    EMCU_DEBUG("\n(%s)%s\n",__func__,buf);
    if(emcu_get_param_multi(PR_TYPE_HEX,67,&param_cnt,param,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param_cnt > 67){
        EMCU_ALERT("\n(%s)param_cnt=%d\n",__func__,param_cnt);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    if(param[0] != (param_cnt-3)){
        EMCU_ALERT("param_cnt=%d param[0]=%lld\n",param_cnt,param[0]);
        EMCU_ALERT("input mismatch\n");
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    for(i=0;i<16;i++)
        rtd_outl(IDX_REG_IRDA_KEY+(i*4),0);
    for(i=0;i<7;i++)
        rtd_outl(IDX_REG_IRDA_PT+(i*4),0);

    tmp_ir_key = 0;
    tmp_ir_header = 0;
    for(i=0;i<param[0];i++){

        ir_key = (param[3+i] & (0xff));
        ir_header = ((param[3+i]>>8) & (0xf));
        EMCU_DEBUG("i=%d,ir_key=0x%x ir_header=0x%x\n",i,ir_key,ir_header);

        //tmp_ir_key = ((tmp_ir_key << 8) | ir_key);
        //tmp_ir_header = ((tmp_ir_header << 4) | ir_header);
        tmp_ir_key = tmp_ir_key  | (ir_key << (8*(i&0x3)));
        tmp_ir_header = tmp_ir_header | (ir_header << (4*(i&0x7)));
        EMCU_DEBUG("tmp_ir_key=0x%x tmp_ir_header=0x%x\n",tmp_ir_key,tmp_ir_header);

        /* fill ir key reg (b8060510~b806054C, total 16) */
        if((i&0x3) == 3){
            reg_tmp = IDX_REG_IRDA_KEY+((i/4)*4);
            rtd_outl(reg_tmp,tmp_ir_key);
            EMCU_DEBUG("%s(%d)reg=0x%x tmp_ir_key=0x%x\n",
                    __func__,__LINE__,reg_tmp,tmp_ir_key);
            tmp_ir_key = 0;
        }
        /* fill ir protocol (b8060550~b806056C, total 8) */
        if((i&0x7) == 7){
            reg_tmp = IDX_REG_IRDA_PT+((i/8)*4);
            rtd_outl(IDX_REG_IRDA_PT+((i/8)*4),tmp_ir_header);
            EMCU_DEBUG("%s(%d)reg=0x%x tmp_ir_header=0x%x\n",
                    __func__,__LINE__,reg_tmp,tmp_ir_header);
            tmp_ir_header = 0;

        }
    }

    EMCU_DEBUG("i=%d,tmp_ir_key=0x%x tmp_ir_header=0x%x\n",i,tmp_ir_key,tmp_ir_header);

    /* fill tmp_ir_key if number of key is not multiple of 4 */
    if(tmp_ir_key){
        reg_tmp = IDX_REG_IRDA_KEY+(((i-1)/4)*4);
        EMCU_DEBUG("%s(%d)reg=0x%x tmp_ir_key=0x%x\n",
                __func__,__LINE__,reg_tmp,tmp_ir_key);
        rtd_outl(reg_tmp,tmp_ir_key);
    }

    /* fill tmp_ir_header if number of header is not multiple of 8 */
    if(tmp_ir_header){
        reg_tmp = IDX_REG_IRDA_PT+(((i-1)/8)*4);
        EMCU_DEBUG("%s(%d)reg=0x%x tmp_ir_header=0x%x\n",
                __func__,__LINE__,reg_tmp,tmp_ir_header);
        rtd_outl(reg_tmp,tmp_ir_header);
    }
    EMCU_DEBUG("%s(%d)0x%x=0x%x, 0x%x=0x%x\n",
            __func__,__LINE__,
            (u32)IDX_REG_IRDA_KEY,(u32)rtd_inl(IDX_REG_IRDA_KEY),
            (u32)IDX_REG_IRDA_PT,(u32)rtd_inl(IDX_REG_IRDA_PT));

    /* fill default protocol */
    pm_event_msg = GET_EVENT_MSG() & ~(RENO_IRDA_PT1_MASK|RENO_IRDA_PT2_MASK|RENO_IRDA_MASK);
    tmp = (unsigned int)(((param[1]&0xf)<<4)|(param[2]&0xf));
    pm_event_msg |= (tmp << SHT_RENO_IRDA_PT1)|(param[0]<<SHT_RENO_IRDA);
    SET_EVENT_MSG(pm_event_msg);
    emcu_suspend_event |= IRDA_SSP_EVENT;
    if(ret == 0)
        ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_irda, S_IRUGO | S_IWUSR,
        emcu_reno_irda_dev_show,emcu_reno_irda_dev_store);
/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_reno_wut_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    return snprintf(buf, 128,
        "please key in \"echo  min > "
        "/sys/devices/virtual/misc/emcu_kdv/emcu_reno_wut\"\n");
}

static
ssize_t emcu_reno_wut_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned int pm_event_msg;
    unsigned long long in_val = 0;
    size_t ret = 0;

    if(emcu_get_param_single(16,&in_val,buf,count)){
        EMCU_ALERT("\n(%s)get param fail\n",__func__);
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    pm_event_msg = GET_EVENT_MSG();
    pm_event_msg &= ~(RENO_WUT_MASK);
    pm_event_msg |= ((unsigned int)in_val << SHT_RENO_WUT);

    SET_EVENT_MSG(pm_event_msg);

    /* Timer wakeup skip emcu_suspend_event announce */

    ret = count;

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_reno_wut, S_IRUGO | S_IWUSR,
        emcu_reno_wut_dev_show,emcu_reno_wut_dev_store);
#endif


void rtk_emcu_set_bootinfo_to_8051(void)
{
    unsigned int boot_info[14] = {0};
    unsigned long long param;
    int ret;
    int boot_cnt = 3;

    boot_info[0] = get_product_type();
    if(boot_info[0] == PRODUCT_TYPE_DIAS){
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
        if (pcb_mgr_get_enum_info_byname("PMIC_CFG", &param) == 0) {
            EMCU_INFO("PMIC information: %llx\n", param);
            boot_info[1] = param & 0xFFFFFFFF;
        }else{
            EMCU_INFO("PMIC_CFG not found, please check bootcode whether defined\n");
        }
#else
        rtd_pr_emcu_alert("%s(%d)need CONFIG_REALTEK_PCBMGR support\n",__func__,__LINE__);
#endif
    }

    boot_info[2] = get_board_version();
    ret = send_packet(EMCU_BOOT_INFO, boot_cnt, boot_info, NULL, boot_cnt, 0);
    if(ret != EMCU_CMD_DONE){
        EMCU_ERR("set boot_info fialed=%d\n",ret);
    }
}

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_show_version_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 128, "EMCU Libs 2017/03/06 21:40\n");
}


static
ssize_t emcu_show_version_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	return count;
}

DEVICE_ATTR(emcu_show_version, S_IRUGO | S_IWUSR,
		emcu_show_version_dev_show,emcu_show_version_dev_store);


#define ENABLE_HW_ERR_REPORT
#ifdef ENABLE_HW_ERR_REPORT

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_set_err_status_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    unsigned int err_status;

    err_status = rtd_inl(REG_MAGIC_51);

    EMCU_INFO("\nCurrent HW error status :0x%08x\n",err_status );

    return snprintf(buf, 128, "%x\n",err_status);
}


static
ssize_t emcu_set_err_status_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned long long in_val;
    size_t ret = count;

    if(emcu_get_param_single(16,&in_val,buf,count)){
        ret = -EINVAL;
        goto FINISH_OUT;
    }

    EMCU_INFO("%s(%d)0x%x=0x%x\n",
            __func__,__LINE__,
            (u32)REG_MAGIC_51,(u32)rtd_inl(REG_MAGIC_51));

    rtd_outl(REG_MAGIC_51,HW_ERR_STA|in_val);

    EMCU_INFO("%s(%d)0x%x=0x%x\n",
            __func__,__LINE__,
            (u32)REG_MAGIC_51,(u32)rtd_inl(REG_MAGIC_51));

FINISH_OUT:
    return ret;

}

DEVICE_ATTR(emcu_set_err_status, S_IRUGO | S_IWUSR,
        emcu_set_err_status_dev_show,emcu_set_err_status_dev_store);

#endif  //#ifdef ENABLE_HW_ERR_REPORT


#define ENABLE_AC_ON_ALERT
#ifdef ENABLE_AC_ON_ALERT

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_get_ac_alert_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    EMCU_ALERT("%s(%d)emcu_ac_on_alt=0x%x\n",
            __func__,__LINE__,emcu_ac_on_alt);
    return snprintf(buf, 128, "0x%x\n",emcu_ac_on_alt);
}


static
ssize_t emcu_get_ac_alert_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{

    EMCU_INFO("\n(%s)no operation!\n",__func__);
    return count;
}

DEVICE_ATTR(emcu_get_ac_alert, S_IRUGO | S_IWUSR,
        emcu_get_ac_alert_dev_show,emcu_get_ac_alert_dev_store);

#endif  //#ifdef ENABLE_AC_ON_ALERT


/************************************************************************
 *
 ************************************************************************/
static __attribute__((unused))
int emcu_chk_gpix(unsigned int pcb_info,unsigned int pin_cnt)
{
	unsigned int tmp1,tmp2,tmp3,tmp4,tmp5;

	tmp1 = (pcb_info>>20) & 0xf;
	tmp2 = (pcb_info>>16) & 0xf;
	tmp3 = (pcb_info>>8)  & 0xff;
	tmp4 = (pcb_info>>0)  & 0xff;
	tmp5 = (pcb_info>>24) & 0xff;
	if((tmp1 == 4)||(tmp1 == 0xd)){
		EMCU_ALERT("pin%d: %s%d; default level:%s; special index:0x%x \n",
			pin_cnt,
			(tmp1 == 4)?"iso_gpio":"iso_gpi",
			(tmp4 == 4)?(32+tmp3):tmp3,
			(tmp2)?"high":"low",tmp5);
	}else if(tmp1 == 1){
		EMCU_ALERT("key%d: LSADC set%d; HITVALUE:0x%x; TOLERANCE:0x%x \n",
			pin_cnt,tmp2,tmp3,tmp4);
	}else{
		EMCU_ALERT("pin type[%d] unsupport!\n",tmp1);
	}
	return 0;
}


/************************************************************************
 *
 ************************************************************************/
static __attribute__((unused))
void emcu_show_sharemem(void)
{
	int i;
	unsigned int share_tmp[32];

	for(i=0;i<RTD_SHARE_MEM_LEN;i++){
		share_tmp[i] = rtd_inl(RTD_SHARE_MEM_BASE+(4*i));
		EMCU_INFO("mem[%02d]=0x%08x\n",i,share_tmp[i]);
	}
}

/************************************************************************
 *
 ************************************************************************/
#define ENCU_CHK_DEBUG
int emcu_chk_event(void)
{
#ifdef CONFIG_MULTI_IR_KEY
    EMCU_ALERT("\nSkip check on RENO!!!!\n");
    return 0;
#else
    int i=0;
    unsigned int pm_event_msg;
    int tmp1,tmp2;
    int tmp3 = 0;
    int ret;

	if(new_protocol_enable == 1){
	    /* check Power enable pin always */
	    if(!(emcu_auto_event & EMCU_POWER_EN_ALERT)){
	        EMCU_ALERT("%s(%d)no power enable pin setting, auto parse it!\n",__func__,__LINE__);
	        if(powerMgr_power_en_parser(NULL,0,PARSE_AUTO)){
	            EMCU_ALERT("%s(%d)power_en auto parse fail!\n",__func__,__LINE__);
	        }
	    }else{
	        EMCU_ALERT("%s(%d)power enable pin set already\n",__func__,__LINE__);
	    }

	    /* check Keypad */
	    if(!(emcu_auto_event & EMCU_KEYPAD_ALERT)){
	        if(powerMgr_keypad_parser(NULL,0,PARSE_AUTO)){
	            EMCU_WARNING("%s(%d)keypad auto parse fail!\n",__func__,__LINE__);
	        }
	    }else{
	        EMCU_WARNING("%s(%d)skip auto parse!\n",__func__,__LINE__);
	    }

	    if(emcu_test_ctl & EMCU_PARSE_EN){
	        /*
	         * When Auto parse mode enable,IRDA,WIFI_dongle and Keypad self.
	         * To check if it's settd because AP may call emcu's API still.
	         */

	        /* check Irda  */
	        if(!(emcu_auto_event & EMCU_IRDA_ALERT)){
	            if(powerMgr_irda_parser(NULL,0,PARSE_AUTO)){
	                EMCU_WARNING("%s(%d)irda auto parse fail!\n",__func__,__LINE__);
	            }
	        }else{
	            EMCU_WARNING("%s(%d)skip auto parse!\n",__func__,__LINE__);
	        }

	    }

		EMCU_ALERT("irda_num:%u keypad_valid_num:%u power_pin_valid_num:%u\n", irda_valid_num, keypad_valid_num, power_pin_valid_num);

		/*keypad setting*/
		if(emcu_auto_event & EMCU_KEYPAD_ALERT || emcu_auto_event & WOL_SSP_EVENT || keypad_valid_num){
			ret = send_packet(EMCU_KEYPAD, keypad_valid_num&0x7F, keypad_arry_ext, NULL, keypad_valid_num, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("EMCU_KEYPAD set fialed=%d\n",ret);
			}
			keypad_valid_num=0;
			memset(keypad_arry_ext, 0, sizeof(keypad_arry_ext));
		}

		/*irda setting*/
		if(emcu_auto_event & EMCU_IRDA_ALERT || irda_valid_num){
			ret = send_packet(EMCU_IRDA, irda_valid_num&0x7F, irda_arry_ext, NULL, irda_valid_num+1, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("irda set fialed=%d\n",ret);
			}
			irda_valid_num=0;
			memset(irda_arry_ext, 0, sizeof(irda_arry_ext));
		}

		/*power pin setting*/
		if(emcu_auto_event & EMCU_POWER_EN_ALERT || power_pin_valid_num){
			ret = send_packet(EMCU_POWER_PIN, (power_pin_valid_num&0x7F), power_pin_arry_ext, NULL, power_pin_valid_num, 0);
			if(ret != EMCU_CMD_DONE){
				EMCU_ERR("power_pin set fialed=%d\n",ret);
			}
			power_pin_valid_num=0;
			memset(power_pin_arry_ext, 0, sizeof(power_pin_arry_ext));
		}

		
	    EMCU_ALERT("\nSkip check on New emcu protocol!!!!\n");
	    return 0;
	}else{

	    EMCU_ALERT("\nCheck setting that suspend need!!!!\n");

	    if(!(emcu_auto_event & EMCU_POWER_EN_ALERT)){
	        EMCU_ALERT("%s(%d)no power enable pin setting, auto parse it!\n",__func__,__LINE__);
	        if(powerMgr_power_en_parser(NULL,0,PARSE_AUTO)){
	            EMCU_ALERT("%s(%d)power_en auto parse fail!\n",__func__,__LINE__);
	        }
	    }else{
	        EMCU_ALERT("%s(%d)power enable pin set already\n",__func__,__LINE__);
	    }
		/* check Keypad */
		if(!(emcu_auto_event & EMCU_KEYPAD_ALERT)){
			EMCU_ALERT("%s(%d)no keypad setting, auto parse it!\n",__func__,__LINE__);
			if(powerMgr_keypad_parser(NULL,0,PARSE_AUTO)){
				EMCU_WARNING("%s(%d)keypad auto parse fail!\n",__func__,__LINE__);
			}
		}else{
			EMCU_WARNING("%s(%d)skip auto parse!\n",__func__,__LINE__);
		}
		pm_event_msg = GET_EVENT_MSG();
#ifdef NEW_POWER_PIN_LIB
		/* to fill power pin info here */
		tmp1 = ((pm_event_msg >> loc_pwen) & 0xf);      //get power pin num
		tmp2 = ((pm_event_msg >> loc_keypad) & 0xf);    //get keypad num
		if(tmp1 > PARAM_LEN_PWEN_LGY){
			tmp3 = (tmp2+(tmp1-PARAM_LEN_PWEN_LGY));        //check if over space
		}
		if(tmp3 > PARAM_LEN_KEYPAD){
			EMCU_ALERT("Over space! we are going to lose %d power pin info.\n",(tmp3-PARAM_LEN_KEYPAD));
			tmp1 = PARAM_LEN_PWEN_LGY + (PARAM_LEN_KEYPAD - tmp2);
			//WARN_ON(1);
		}
		for(i=0; i< tmp1; i++){
			if(i < PARAM_LEN_PWEN_LGY){	/* old field */
				SET_PWEN_PARM(i,pw_arry_ext[i]);
			}else{	/* followong the field of keypad */
				SET_KEYPAD_PARM(tmp2+(i-PARAM_LEN_PWEN_LGY),pw_arry_ext[i]);
			}
		}
		emcu_show_sharemem();

		/* To check power pin */
		EMCU_ALERT("%s power pin count are %u\n",(tmp1)?"Total":"Warning!!!",tmp1);
		for(i=0; i< tmp1; i++){
			if(i < PARAM_LEN_PWEN_LGY){
				tmp3 = GET_PWEN_PARM(i);
			}else{
				tmp3 = GET_KEYPAD_PARM(tmp2+(i-PARAM_LEN_PWEN_LGY));
			}
			emcu_chk_gpix(tmp3,i+1);
		}
#else
		tmp1 = ((pm_event_msg >> loc_pwen) & 0xf);      //get power pin num
		for(i=0; i< tmp1; i++){
			if(i < PARAM_LEN_PWEN)
				SET_PWEN_PARM(i,pw_arry_ext[i]);
		}
	    emcu_show_sharemem();

	    pm_event_msg = GET_EVENT_MSG();

	    /* To check power pin */
	    tmp1 = (pm_event_msg >> loc_pwen) & 0xf;
	    if(tmp1){
	        EMCU_ALERT("Total Power pin count are %u\n",tmp1);
	        for(i=0; i< tmp1; i++){
	            tmp2 = GET_PWEN_PARM(i);
	            emcu_chk_gpix(tmp2,i+1);
	        }

	    }else{
	        EMCU_ALERT("Warning!!! No Power pin for sleeping assigned!\n");
	    }
#endif
	    /* To check IRDA setting */
	    tmp1 = (pm_event_msg >> loc_irda) & 0xf;
	    if(tmp1){
	        EMCU_ALERT("Total IRDA key used for wakeup are %u sets.\n",tmp1);
	        tmp2 = GET_IRDA_PARM(0);
	        EMCU_ALERT("irda protocol is %u\n",tmp2);
	        for(i=1; i<= tmp1; i++){
	            tmp3 = GET_IRDA_PARM(i);
	            EMCU_ALERT("Irda key%d:0x%08x\n",i,tmp3);
	        }

	    }else{
	        EMCU_ALERT("Warning!!! No IRDA key for wakeup assigned!\n");
	    }

	    /* To check KeyPad setting */
	    tmp1 = (pm_event_msg >> loc_keypad) & 0xf;
	    if(tmp1){
	        EMCU_ALERT("Total Keypad used for wakeup are %u sets.\n",tmp1);
	        for(i=0; i< tmp1; i++){
	            tmp2 = GET_KEYPAD_PARM(i);
	            emcu_chk_gpix(tmp2,i+1);
	        }
	    }else{
	        EMCU_ALERT("Warning!!! No Keypad pin for wakeup assigned!\n");
	    }

	    /* To check CEC setting */
	    tmp1 = (pm_event_msg >> loc_cec) & 0xf;
#ifndef EMCU_WIFI_POWER_NAME
		EMCU_ALERT("There are %sCEC function for wake up.[event=%u] \n",(tmp1)?"":"no ",tmp1);
#else
		if(tmp1 & 0x3){
			EMCU_ALERT("There are %sCEC function for wake up.[event=%u] \n",(tmp1 & 0x3)?"":"no ",(tmp1 & 0x3));
		}
		if(tmp1 & BIT(3)){
			EMCU_ALERT("There are WIFI function for wake up.\n");
		}
#endif

	    /* To check PPS setting */
	    tmp1 = (pm_event_msg >> loc_pps) & 0xf;
	    tmp2 = GET_PPS_PARM();
	    EMCU_ALERT("There are %spervious power source  function for wake up.[event=%u] \n",(tmp1)?"":"no ",tmp2);

	    /* To check WUT setting */
	    tmp1 = (pm_event_msg >> loc_wut) & 0xf;
	    if(tmp1 & (tmp1<2)){
	        tmp2 = GET_WUT_PARM();
	        EMCU_ALERT("Timer wake up after %u %s.\n",tmp2,(tmp1==1)?"min":"sec");
	    }else{
	        EMCU_ALERT("There are no timer function for wake up.\n");
	    }

	    /* To check WOL and WOWLAN[re-key] */
	    tmp1 = (pm_event_msg >> loc_wow) & 0xf;
	    if(tmp1 & BIT(0)){
	        EMCU_ALERT("Wake on LAN function alert.\n");

	        tmp2 = GET_WIFI_DG_PARM();
	        tmp3 = ((tmp2>>24) & 0xff);
	        if(tmp3 == IDX_REKEY){
	            EMCU_ALERT("Wake on Wide LAN re-key function enable.\n");
	            emcu_chk_gpix(tmp2,1);
	        }
	    }else{
	        EMCU_ALERT("There are no \"Wake on LAN\" or \"Wake on Wide LAN re-key\".\n");
	    }

	    /* To check MHL */
	    EMCU_ALERT("There are %sMHL function for wake up.\n",(tmp1 & BIT(1))?"":"no ");

	    /* To check STR */
	    tmp1 = (pm_event_msg >> loc_stm) & 0xf;
	    EMCU_ALERT("There are %sSTR function for wake up.[event=%u]\n",(tmp1 & 0x03)?"":"no ",tmp1&0x03);

	    /* To check RTC */
	    EMCU_ALERT("There are %sRTC function for wake up.\n",(tmp1 & 0x04)?"":"no ");

	    return 0;
	}
#endif
}
EXPORT_SYMBOL(emcu_chk_event);

/************************************************************************
 *
 ************************************************************************/
static
ssize_t emcu_show_help_dev_show(struct device *dev,
    struct device_attribute *attr,
    char *buf)
{
    EMCU_ALERT(
        "\n"
        "All Command List:\n"
        "=========================================================================\n"
        " @ emcu_show_share_mem : display all share memory value.\n"
        " @ emcu_set_sharemem   : Fill data to certian share memory.\n"
        " @ emcu_set_sharemem_mask   : Fill data to certian share memory with mask.\n"
        " @ emcu_clr_share_mem  : claer all share memory immediately.\n"
#ifdef CONFIG_MULTI_IR_KEY
        " @ emcu_reno_gpo            : set power pin for RENO.\n"
        " @ emcu_reno_lsadc          : set Keypad for RENO.\n"
        " @ emcu_reno_wol            : set wake on lan for RENO.\n"
        " @ emcu_reno_wov            : set wake on voice for RENO.\n"
        " @ emcu_reno_cec            : set CEC wake up for RENO.\n"
        " @ emcu_reno_irda           : set IRDA key to wakeup for RENO.\n"
#else
        " @ emcu_clr_pm_task    : clear assigned operation, not clear all share memory.\n"
        " @ emcu_set_STM        : enable suspend to RAM.\n"
        " @ emcu_set_RTC        : enable RTC wakeup function.\n"
        " @ emcu_set_WOWLAN     : enable weke on wide LAN func.\n"
        " @ emcu_setup_cec      : enable and setting keypad parameter.\n"
        " @ emcu_setup_mhl      : enable mhl func.\n"
        " @ emcu_setup_ewbs     : enable ewbs func.\n"
        " @ emcu_setup_uart     : enable uart func.\n"
        " @ emcu_setup_ppsource : enable pervious power on source.\n"
        " @ emcu_setup_irda     : set IRDA key to wakeup.\n"
        " @ emcu_setup_power_en : set power pin information to EMCU.\n"
        " @ emcu_set_cur_time   : set current time to emcu.\n"
        " @ emcu_get_rec_time   : get recorded time from emcu.\n"
        " @ emcu_get_sleep_dur  : get how long sleeping from emcu.\n"
        " @ emcu_setup_keypad_multi  : enable and setting keypad parameter.\n"
        " @ emcu_setup_wakeup_timer  : set wake up timers, minimum is minute.\n"
        " @ emcu_setup_wakeup_timer1 : set wake up timers, minimum is sec.\n"
        " @ emcu_setup_wakeup_min    : set wake up timers and reason, minimum is min.\n"
        " @ emcu_setup_wakeup_sec    : set wake up timers and reason, minimum is sec.\n"
        " @ emcu_get_wakeup_source   : get wake up source from emcu.\n"
        " @ emcu_get_wakeup_status   : get wake up status.\n"
#endif

#ifdef ENABLE_HW_ERR_REPORT
        " @ emcu_set_err_status      : set HW error status.\n"
#endif
        "=========================================================================\n"
        "\n");
        return snprintf(buf,128,
                "enable log level to 4 then disply message\n");
}

static
ssize_t emcu_show_help_dev_store(struct device *dev,
    struct device_attribute *attr,
    const char *buf, size_t count)
{
    unsigned long long in_val;
    size_t ret = count;

    if(emcu_get_param_single(16,&in_val,buf,count)){
        ret = -EINVAL;
        goto FINISH_OUT;
    }
    if(in_val == 1){
        EMCU_ALERT("\n"
            "User guide of emcu device attribute cmd.\n");
        EMCU_ALERT("\n"
            " @ emcu_show_share_mem: Display all share memory value.\n"
            "    example:\n"
            "        echo 1 > emcu_show_share_mem\n");
        EMCU_ALERT("\n"
            " @ emcu_set_sharemem: Fill data to certian share memory.\n"
            "    example:\n"
            "       echo [addr],[value] > emcu_set_sharemem\n");
        EMCU_ALERT("\n"
            " @ emcu_set_sharemem_mask: Fill data to certian share memory with mask.\n"
            "    example:\n"
            "       echo [addr],[mask],[value] > emcu_set_sharemem_mask\n");
        EMCU_ALERT("\n"
            " @ emcu_clr_share_mem: claer all share memory immediately.\n"
            "    example:\n"
            "        echo 1 > emcu_clr_share_mem\n");
#ifdef CONFIG_MULTI_IR_KEY
        EMCU_ALERT("\n"
            " @ emcu_reno_gpo: set power pin for RENO.\n"
            "    example:\n"
            "        echo [cnt],[pcb_param1],...,[pcb_param4] > emcu_reno_gpo\n");
        EMCU_ALERT("\n"
            " @ emcu_reno_lsadc: set Keypad for RENO.\n"
            "    example:\n"
            "        echo [cnt],[lsadc1],[lsadc2] > emcu_reno_lsadc\n");
        EMCU_ALERT("\n"
            " @ emcu_reno_wol: set wake on lan for RENO.\n"
            "    example:\n"
            "        echo [eanble],[pcb_param] > emcu_reno_wol\n");
        EMCU_ALERT("\n"
            " @ emcu_reno_wov: set set wake on voice for RENO.\n"
            "    example:\n"
            "        echo [eanble],[pcb_param] > emcu_reno_wov\n");
        EMCU_ALERT("\n"
            " @ emcu_reno_cec: set CEC wake up for RENO.\n"
            "    example:\n"
            "        echo [eanble] > emcu_reno_cec\n");
        EMCU_ALERT("\n"
            " @ emcu_reno_irda: set IRDA key to wakeup for RENO.\n"
            "    example:\n"
            "        echo [cnt],[pt1],[pt2],[key1],...,[key64] > emcu_reno_irda\n");
#else
        EMCU_ALERT("\n"
            " @ emcu_clr_pm_task: clear assigned operation, not clear all share memory.\n"
            "    example:\n"
            "        echo [param] > emcu_clr_pm_task\n");
        EMCU_ALERT("\n"
            " @ emcu_set_STM: enable suspend to RAM.\n"
            "    example:\n"
            "       echo 1 > emcu_set_STM\n");
        EMCU_ALERT("\n"
            " @ emcu_set_WOWLAN:  enable weke on wide LAN func.\n"
            "    example:\n"
            "        echo 1 > emcu_set_WOWLAN\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_cec: enable and setting keypad parameter.\n"
            "    example:\n"
            "        echo 1 > emcu_setup_cec\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_mhl: enable mhl func.\n"
            "    example:\n"
            "        echo 1 > emcu_setup_mhl\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_uart: enable uart func.\n"
            "    example:\n"
            "        echo 1 > emcu_setup_uart\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_ppsource: enable pervious power on source func.\n"
            "    example:\n"
            "        echo [param] > emcu_setup_uart\n"
            "    note: support VGA1 only, please to set param=3\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_irda: set IRDA key to wakeup.\n"
            "    example:\n"
            "        echo [protocol],[key_num],[keycode1],[keycode2],.. > emcu_setup_irda\n"
            "    note: The max number of key is 15.\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_power_en: set power pin information to EMCU.\n"
            "    example:\n"
            "       echo [pw_num],[pin1],[pin2],.. > emcu_setup_power_en\n"
            "    note: The max number of power pin is 3.\n");
        EMCU_ALERT("\n"
            " @ emcu_set_cur_time: set current time to emcu.\n"
            "    example:\n"
            "       echo  [hours],[mins],[sec] > emcu_set_cur_time\n"
            "    note: The format of \"hours,mins,sec\" are DEC\n");
        EMCU_ALERT("\n"
            " @ emcu_get_rec_time: get recorded time from emcu.\n"
            "    example:\n"
            "       cat emcu_get_rec_time\n");
        EMCU_ALERT("\n"
            " @ emcu_get_sleep_dur: get how long sleeping from emcu.\n"
            "    example:\n"
            "       cat emcu_get_sleep_dur\n"
            "    note: the report format is \"min,sec,ms\"\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_keypad_multi: enable and setting keypad parameter.\n"
            "    example:\n"
            "        echo [key_num],[param1],[param2],...,... > "
            "/sys/devices/virtual/misc/emcu_kdv/emcu_setup_keypad_multi\n"
            "    note: the max number of keypad are 10\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_wakeup_timer: set wake up timers, minimum is minute.\n"
            "    example:\n"
            "        echo [days],[hours],[mins] > emcu_setup_wakeup_timer\n"
            "    note: The format of \"days,hours,mins\" are DEC\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_wakeup_timer1: set wake up timers, minimum is sec.\n"
            "    example:\n"
            "        echo [days],[hours],[mins],[sec] > emcu_setup_wakeup_timer1\n"
            "    note: The format of \"days,hours,mins,sec\" are DEC\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_wakeup_min: set wake up timers and reason, minimum is min.\n"
            "    example:\n"
            "        echo [reason],[mins] > emcu_setup_wakeup_min\n"
            "    note: The format of \"reason,mins\" are DEC\n");
        EMCU_ALERT("\n"
            " @ emcu_setup_wakeup_sec: set wake up timers and reason, minimum is sec.\n"
            "    example:\n"
            "        echo [reason],[mins] > emcu_setup_wakeup_sec\n"
            "    note: The format of \"reason,sec\" are DEC\n");
        EMCU_ALERT("\n"
            " @ emcu_get_wakeup_source: get wake up source from emcu.\n"
            "    example:\n"
            "       cat emcu_get_wakeup_source\n"
            "    note: The meaning of report data is below.\n"
            "       str[][7]={\"UNDEF\",\"KEYPAD\",\"WUT\",\"IRDA\",\"CEC\",\"PPS\"} \n");
        EMCU_ALERT("\n"
            " @ emcu_get_wakeup_status:  get wake up status.\n"
            "    example:\n"
            "       cat emcu_get_wakeup_status\n");
#endif

#ifdef ENABLE_HW_ERR_REPORT
        EMCU_ALERT("\n"
            " @ emcu_set_err_status:  set HW error status.\n"
            "    example:\n"
            "       echo [err_event] > emcu_set_err_status\n"
            "    note: err_event in Hex format.\n");
#endif
        EMCU_ALERT("\n\n");
    }
FINISH_OUT:
    return ret;
}

DEVICE_ATTR(emcu_show_help, S_IRUGO | S_IWUSR,
		emcu_show_help_dev_show,emcu_show_help_dev_store);

 static ssize_t emcu_protocol_test_dev_show(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    int i = 0;
    unsigned int temp[16];

    // only use 16*4 bytes
    for(i=0;i<(RTD_SHARE_MEM_LEN/2);i++){
        temp[i] = rtd_inl((RTD_SHARE_MEM_BASE + (4*i)));
    }

    return snprintf(buf, 512,
            "mem[00]=0x%08x mem[01]=0x%08x mem[02]=0x%08x mem[03]=0x%08x\n"
            "mem[04]=0x%08x mem[05]=0x%08x mem[06]=0x%08x mem[07]=0x%08x\n"
            "mem[08]=0x%08x mem[09]=0x%08x mem[0a]=0x%08x mem[0b]=0x%08x\n"
            "mem[0c]=0x%08x mem[0d]=0x%08x mem[0e]=0x%08x mem[0f]=0x%08x\n",
            temp[ 0],temp[ 1],temp[ 2],temp[ 3],
            temp[ 4],temp[ 5],temp[ 6],temp[ 7],
            temp[ 8],temp[ 9],temp[10],temp[11],
            temp[12],temp[13],temp[14],temp[15]);
}

    static
ssize_t emcu_protocol_test_dev_store(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    int ret = 0;
    unsigned int test_pattern[] = {0x1234,0x5678,0xabcd};
    unsigned int ret_data[16] = {0};

    if (!strncmp(buf, "read", 4)){
        EMCU_ALERT("EMCU read test\n", __func__);
        ret = send_packet(0x01, 0xfdfc, test_pattern, ret_data, sizeof(test_pattern)/sizeof(unsigned int), 1);
    }
    else if (!strncmp(buf, "write", 5)){
        EMCU_ALERT("EMCU write test\n", __func__);
        ret = send_packet(0x02, 0xbabc, test_pattern, ret_data, sizeof(test_pattern)/sizeof(unsigned int), 0);
    }
    else
        EMCU_ALERT("EMCU invalid option\n", __func__);

    EMCU_ALERT("EMCU test result:%d\n", ret);
    return count;
}

DEVICE_ATTR(emcu_protocol_test, S_IRUGO | S_IWUSR,
        emcu_protocol_test_dev_show,emcu_protocol_test_dev_store);
/* new protocol end*/

/*-----------------------------------------------------------------------------------
 * File Operations
 *------------------------------------------------------------------------------------*/
/************************************************************************
 *
 ************************************************************************/
int rtk_emcu_open(struct inode *inode, struct file *file)
{
	EMCU_INFO("%s(%d) Open\n",__func__,__LINE__);
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
int rtk_emcu_release(struct inode *inode, struct file *file)
{
	EMCU_INFO("%s(%d) Release\n",__func__,__LINE__);
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
static long rtk_emcu_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	EMCU_INFO("%s(%d) \n",__func__,__LINE__);
	return 0;
}

/************************************************************************
 *
 ************************************************************************/
#ifdef CONFIG_COMPAT
long rtk_emcu_compat_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	EMCU_INFO("%s(%d)\n",__func__,__LINE__);
	return 0;
}
#endif

static
struct file_operations rtk_emcu_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rtk_emcu_ioctl,
	.open = rtk_emcu_open,
	.release = rtk_emcu_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl = rtk_emcu_compat_ioctl,
#endif
};

static struct miscdevice rtk_emcu_miscdev = {
	MISC_DYNAMIC_MINOR,
	"emcu_kdv",
	&rtk_emcu_fops
};


#define EMCU_DRV_NAME  "emcu_drv"

static int emcu_drv_suspend    (struct device *dev);
static int emcu_drv_resume    (struct device *dev);

static struct platform_device emcu_drv_pm_device = {
    .name          = EMCU_DRV_NAME,
    .id            = -1,
};

#ifdef CONFIG_PM
static const struct dev_pm_ops rtkemcu_pm_ops =
{
	.suspend_noirq = emcu_drv_suspend,
	.resume  = emcu_drv_resume,
};
#endif

static struct platform_driver emcu_drv_pm_driver = {
    .driver    = {
        .name  = EMCU_DRV_NAME,
        .owner = THIS_MODULE,
#ifdef CONFIG_PM
        .pm    = &rtkemcu_pm_ops,
#endif
    },
};

#ifdef CONFIG_PM
static int emcu_pm_notifier_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	switch (event)
	{
	case PM_SUSPEND_PREPARE:
		powerMgr_set_wakeup_source_undef();
		break;

	default:
		break;
	}
	return NOTIFY_DONE;
}


static struct notifier_block emcu_pm_notifier = {
        .notifier_call = emcu_pm_notifier_event,
};

extern int rtk_pm_load_8051(int flag);
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
extern unsigned int str_status ;
#endif
static int emcu_drv_suspend(struct device *dev)
{
	int i;
    EMCU_INFO("%s(%d)\n",__func__,__LINE__);

	if(new_protocol_enable == 0){
	    /* check Power enable pin always */
	    if(!(emcu_auto_event & EMCU_POWER_EN_ALERT)){
	        EMCU_ALERT("%s(%d)no power enable pin setting, auto parse it!\n",__func__,__LINE__);
	        if(powerMgr_power_en_parser(NULL,0,PARSE_AUTO)){
	            EMCU_ALERT("%s(%d)power_en auto parse fail!\n",__func__,__LINE__);
	        }
	    }else{
	        EMCU_ALERT("%s(%d)power enable pin set already\n",__func__,__LINE__);
	    }

	    if(emcu_test_ctl & EMCU_PARSE_EN){
	        /*
	         * When Auto parse mode enable,IRDA,WIFI_dongle and Keypad self.
	         * To check if it's settd because AP may call emcu's API still.
	         */

	        /* check Irda  */
	        if(!(emcu_auto_event & EMCU_IRDA_ALERT)){
	            if(powerMgr_irda_parser(NULL,0,PARSE_AUTO)){
	                EMCU_WARNING("%s(%d)irda auto parse fail!\n",__func__,__LINE__);
	            }
	        }else{
	            EMCU_WARNING("%s(%d)skip auto parse!\n",__func__,__LINE__);
	        }

	        /* check Keypad */
	        if(!(emcu_auto_event & EMCU_KEYPAD_ALERT)){
	            if(powerMgr_keypad_parser(NULL,0,PARSE_AUTO)){
	                EMCU_WARNING("%s(%d)keypad auto parse fail!\n",__func__,__LINE__);
	            }
	        }else{
	            EMCU_WARNING("%s(%d)skip auto parse!\n",__func__,__LINE__);
	        }
	    }

	    for(i=0; i<RTD_SHARE_MEM_LEN; i++){
	        local_sharemem_sup[i] = rtd_inl(RTD_SHARE_MEM_BASE+4*i);
	        local_sharemem_res[i] = 0;
	    }
	}
	/* clear AC on flag */
	emcu_ac_on_alt  = 0;
	g_rtk_load_8051 = 0;

	/*Because cpu_resume not exported, so the register is set by ATF when core 0 issue cpu suspend with cpu_resume address*/
	//rtd_outl(STB_WDOG_DATA2_reg, virt_to_phys(cpu_resume));
	rtd_pr_emcu_info("<< %s >>: Ready to load 8051 (%lu)...\n", __FUNCTION__, SUSPEND_RAM);
	rtk_pm_load_8051(SUSPEND_RAM);
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
	str_status = 1;
#endif
    return 0;
}

void pm_send_power_key(void)
{
/*  for Android TV if resume from 8051,
that means wake up source register not zero, then sned a power key for wakeup Android.*/
#ifdef CONFIG_ANDROID
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
    venus_ir_gen_keyupdown_event(KEY_POWER);
#endif
#endif
}

static int emcu_drv_resume(struct device *dev)
{
	int i;

#if 0 //IS_ENABLED(CONFIG_RTK_KDRV_EMCU)
	int wakeup_reg_row;
	unsigned int wakeup_src;
#endif

    EMCU_INFO("%s(%d)\n",__func__,__LINE__);

    emcu_auto_event = 0;
    /* clear event last time */
    emcu_suspend_event = 0;
	stm_flag = 0;
	cec_flag = 0;
	irda_valid_num=0;
	keypad_valid_num=0;
	power_pin_valid_num=0;

	if(new_protocol_enable == 0){
	    if((GET_EVENT_MSG() == 0x00) && (GET_WK_SOR() != 0x00)){
	        /* GET_EVENT_MSG() == 0x00,
	           update resume information only when have been run */
	        for(i=0; i<RTD_SHARE_MEM_LEN; i++){
	            local_sharemem_res[i] = rtd_inl(RTD_SHARE_MEM_BASE+4*i);
	            local_sharemem_sup[i] = 0;
	        }
	    }
	}
#ifdef CONFIG_MULTI_IR_KEY
    reno_gpi_cnt = 0;
#endif

#if 0 //IS_ENABLED(CONFIG_RTK_KDRV_EMCU)
	powerMgr_get_wakeup_source(&wakeup_reg_row, NULL);
	wakeup_src = wakeup_reg_row & 0xffff;   //[MK2BU-650][EMCU]Add sub_wakeup source

	if((wakeup_src != WKSOR_UNDEF) && ((wakeup_src != WKSOR_RTC) && (wakeup_src != WKSOR_WIFI) && (wakeup_src != WKSOR_WOW)))
		pm_send_power_key();
	else
		rtd_pr_emcu_err("No send power key(%x)\n", wakeup_src);
#endif
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
	str_status = 0;
#endif
    return 0;
}

#else

#define emcu_drv_suspend   NULL
#define emcu_drv_resume    NULL

#endif

/************************************************************************
 * To use mmc param for dynamic operation
 ************************************************************************/
static
int rtk_emcu_parse_param(void)
{
	char *opt = NULL;
	char *platform_mmc_param = NULL;
	__attribute__((unused))long value;

#ifdef AUTO_PCB_PARAM
	emcu_test_ctl |= EMCU_PARSE_EN;
	EMCU_ALERT("CONFIG_EMCU_AUTO_PARSER enable\n");
#endif
	emcu_suspend_event = 0;
	platform_mmc_param = kmalloc(MMC_PARAM_LEN,GFP_KERNEL);
	if(platform_mmc_param == NULL)
		return 0;

	memcpy(platform_mmc_param,platform_info.mmc_param,MMC_PARAM_LEN);

	EMCU_INFO("mmc_param: \"%s\"\n",platform_mmc_param);

	while ((opt = strsep(&platform_mmc_param, " ")) != NULL) {
		if (!*opt){
			continue;
		}
#ifndef AUTO_PCB_PARAM
		if (!strncmp(opt, "emcu_auto=", 10)) {
			if (!kstrtol(opt + 10, 10, (long *)&value)){
				if(value == 1){
					emcu_test_ctl |= EMCU_PARSE_EN;
				}else{
					emcu_test_ctl &= ~EMCU_PARSE_EN;
				}
				EMCU_ALERT("emcu auto parse %s by bootcode.\n",
						((emcu_test_ctl & EMCU_PARSE_EN))?"enable":"disable");
			}
		}
#endif
		if (!strncmp(opt, "emcu_chk_skip", 13)) {
			EMCU_ALERT("emcu skip panic if no setting\n");
			emcu_test_ctl |= EMCU_CHK_SKIP;
		}
	}

	if(platform_mmc_param)
		kfree(platform_mmc_param);

	return 0;

}

/************************************************************************
 *
 ************************************************************************/
#ifndef CONFIG_RTK_KDRV_EMCU_MODULE
static int __init emcu_new_protocol_set(char *options)
{
	rtd_pr_emcu_info("emculog\n");

    if(options == NULL){
        EMCU_INFO("boot EMCU new_protocol NULL\n");
		new_protocol_enable = 0;
    }   
    else{
        EMCU_INFO("boot EMCU new_protocol:: %s\n", options);
        if (sscanf(options, "%d", &new_protocol_enable) < 1){ 
            return 1;
        }
        new_protocol_enable =1;
    }

	rtd_pr_emcu_info("new_protocol_enable:%d",new_protocol_enable);
    return 1;
}
early_param("emcu_new_protocol", emcu_new_protocol_set);
#else
int rtk_emcu_new_protocol_set(void)
{
    char strings[32] = {0};
	if(rtk_parse_commandline_equal("emcu_new_protocol", strings, sizeof(strings)))
	{
		new_protocol_enable = 1;
	}else{
		new_protocol_enable = 0;
	}
	rtd_pr_emcu_info("new_protocol_enable:%d",new_protocol_enable);

	return 0;
}
#endif

static void rtk_platfrom_power_off(void)
{
	rtk_pm_load_8051(SUSPEND_NORMAL); // #define SUSPEND_NORMAL      1
}

static
int __init rtk_emcu_module_init(void)
{
    int retval = 0;
	int i;

#if (!IS_ENABLED(CONFIG_REALTEK_PCBMGR))
	rtd_pr_emcu_alert("%s(%d)No CONFIG_REALTEK_PCBMGR supporting, It could malfunction.\n",__func__,__LINE__);
#endif
    emcu_auto_event = 0;
    emcu_test_ctl = 0;

#ifdef CONFIG_RTK_KDRV_EMCU_MODULE
    rtk_emcu_new_protocol_set(); //to check whether use new protocol;
    /* TO check if ac on case */
    rtk_emcu_ac_alert();
    /* if the keyword "POWERDOWN" appears, shut down immediately. */
    rtk_pm_load_8051_code();
#endif

    if(new_protocol_enable == 1){
        rtk_emcu_set_bootinfo_to_8051();
    }

    if(g_rtk_load_8051 == 1)
    {
        EMCU_ALERT("%s(%u) begin to load 8051\n",__func__,__LINE__);
        rtk_pm_load_8051(0);
        g_rtk_load_8051 = 0;
        goto FINISH_OUT;
    }

    rtk_emcu_parse_param();

#ifdef CONFIG_MULTI_IR_KEY
    /* disable parse and check func which enable by bootcode */
    reno_gpi_cnt = 0;
    emcu_test_ctl = EMCU_CHK_SKIP;
#endif
#ifdef CONFIG_PM
	register_pm_notifier(&emcu_pm_notifier);
#endif
    if (misc_register(&rtk_emcu_miscdev)) {
        EMCU_INFO("rtk_emcu_module_init failed - register misc device failed\n");
        retval = -ENODEV;
        goto FINISH_OUT;
    }
	if(new_protocol_enable == 0){
	    for(i=0; i<RTD_SHARE_MEM_LEN; i++){
	        local_sharemem_res[i] = rtd_inl(RTD_SHARE_MEM_BASE+4*i);
	    }
	}
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_share_mem);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_sharemem);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_sharemem_mask);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_clr_share_mem);
#ifdef CONFIG_MULTI_IR_KEY
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_gpo);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_lsadc);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wol);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wov);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_cec);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_irda);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wut);
#else
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_clr_pm_task);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_wifi_dg);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOL);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOWLAN);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_keypad_multi);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_cec);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_mhl);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_ewbs);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_uart);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wov);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_ppsource);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_timer);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_timer1);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_min);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_sec);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_irda);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_irda_in_code);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_power_en);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_power_en_in_name);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_cur_time);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_STM);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_RTC);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_wakeup_source_undef);
#ifdef INTERNAL_WOW_SUPPORT
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOW);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOW_chksum);
#endif
#endif

#ifdef ENABLE_HW_ERR_REPORT
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_err_status);
#endif
#ifdef ENABLE_AC_ON_ALERT
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_ac_alert);
#endif
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_sleep_dur);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_rec_time);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_source);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_status);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_status_keycode);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_version);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_help);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_protocol_test);
    device_create_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_led_flash);

    retval = platform_device_register(&emcu_drv_pm_device);
    if (retval)
        goto FINISH_OUT;

    retval = platform_driver_probe(&emcu_drv_pm_driver, NULL);
    if (retval)
        goto FINISH_OUT;

#ifdef CONFIG_EMCU_ISR_MODE
    retval = register_emcu_isr_device();
    if (retval)
        goto FINISH_OUT;
#endif
    EMCU_INFO("emcu_kdv driver install success\n");

	/*[ANDROIDTV-3123] For long press RC power key -> power off */
	pm_power_off = rtk_platfrom_power_off;

FINISH_OUT:
	return retval;
}

/************************************************************************
 *
 ************************************************************************/
static
void __exit rtk_emcu_module_exit(void)
{
#ifdef CONFIG_EMCU_ISR_MODE
	unregister_emcu_isr_device();
#endif
	platform_driver_unregister(&emcu_drv_pm_driver);

    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_share_mem);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_sharemem);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_sharemem_mask);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_clr_share_mem);
#ifdef CONFIG_MULTI_IR_KEY
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_gpo);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_lsadc);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wol);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wov);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_cec);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_irda);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_reno_wut);
#else
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_clr_pm_task);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_wifi_dg);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOL);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOWLAN);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_keypad_multi);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_cec);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_mhl);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_ewbs);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_uart);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wov);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_ppsource);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_timer);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_timer1);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_min);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_wakeup_sec);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_irda_in_code);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_irda);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_power_en);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_setup_power_en_in_name);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_cur_time);

    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_STM);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_RTC);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_wakeup_source_undef);
#ifdef INTERNAL_WOW_SUPPORT
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOW);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_WOW_chksum);
#endif
#endif

#ifdef ENABLE_HW_ERR_REPORT
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_err_status);
#endif
#ifdef ENABLE_AC_ON_ALERT
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_ac_alert);
#endif
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_sleep_dur);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_rec_time);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_source);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_status);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_status_keycode);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_version);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_show_help);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_protocol_test);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_get_wakeup_status_keycode);
    device_remove_file(rtk_emcu_miscdev.this_device, &dev_attr_emcu_set_led_flash);

#ifdef CONFIG_PM
    unregister_pm_notifier(&emcu_pm_notifier);
#endif
    EMCU_INFO("%s(%d)rtk emcu_kdv driver uninstall success\n",__func__,__LINE__);
}

int emcu_enter_poweroff(int mode)
{
	int ret;
	
	ret = send_packet(EMCU_POWER_MODE, mode, NULL, NULL, 0, 0);
	if(ret != EMCU_CMD_DONE){
		EMCU_ERR("power_en set fialed=%d\n",ret);
	}
	return 0;
}
/* =======================================================================
 * Kernel setup function
 * ======================================================================= */
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
extern eirda_keymap_type ir_keymap_type;
extern int ir_proto_irda_init(char *options);
#endif

__attribute__((unused))
int rtk_emcu_smem_set_ir(char *ibuf){
#if IS_ENABLED(CONFIG_RTK_KDRV_IR)
#define MAX_IRKEY_LEN 100
    __attribute__((unused))int i;
    u32 len = 0;
    char buff[MAX_IRKEY_LEN] = {0};
    char *opt = NULL;
    char *pbuff = NULL;
    unsigned long tmp_val;
    unsigned int ir_key[15];
	uint32_t user_key[15];

    pbuff = buff;
    len = strlen(ibuf);
    EMCU_DEBUG("%s(%u)parameter len is %u.\n",__func__,__LINE__,len);
    if(len < 1 )
        return -EINVAL;

    if(len > MAX_IRKEY_LEN -1){
        EMCU_ALERT("%s(%u)parameter size bigger then buffer(%u).\n",__func__,__LINE__,len);
        len = MAX_IRKEY_LEN -1;
    }
    strncpy(buff,ibuf,len);
    buff[sizeof(buff)-1] = 0;
    len = 0;
    while ((opt = strsep(&pbuff, ",")) != NULL) {
        if (!*opt)
            continue;

        if(len == 0){
            unsigned int boot_sel_proto = 0;
            unsigned char remoter[32] = {0};
            if (sscanf(opt, "%d-%31s", &boot_sel_proto, remoter) < 1){
                EMCU_ALERT("%s(%u)get IR protocol fail\n",__func__,__LINE__);
                return -EINVAL;
            }
            ir_proto_irda_init(opt);
            user_key[0] = boot_sel_proto;
            len++;
        }else{
            if (!kstrtol(opt, 16, &tmp_val)){
                user_key[len] = (uint32_t)tmp_val;
                EMCU_DEBUG("%s(%u)get param = 0x%x\n",__func__,__LINE__,user_key[len]);
                len++;
            }else {
                EMCU_ALERT("%s(%u)get param fail\n",__func__,__LINE__);
            }
        }
    }
    if(len <2){
        EMCU_ALERT("%s(%u)There are no vaild key\n",__func__,__LINE__);
        return -EINVAL;
    }
    EMCU_DEBUG("%s(%u)get %d set key\n",__func__,__LINE__,len);

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    ir_key[0] = user_key[0];
	#ifdef  CONFIG_CUSTOMER_TV030
	if(ir_key[0] == 8) {
		 ir_key[1] = 0xf2a0d5;
         ir_key[2] = 0xfef010;
         ir_key[3] = 0xf300cf;
         ir_key[4] = 0xf310ce;
         ir_key[5] = 0xf320cd;
         ir_key[6] = 0xf330cc;
         ir_key[7] = 0xf340cb;
         ir_key[8] = 0xf350ca;
         ir_key[9] = 0xf360c9;
         ir_key[10] = 0xf370c8;
         ir_key[11] = 0xf380c7;
         ir_key[12] = 0xf390c6;
         len = 13;
     }
     else if(ir_key[0] == 1)  {
         ir_key[1] = 0xa05ffb04;
         ir_key[2] = 0xbb44fb04;
         ir_key[3] = 0xac53fb04;
         ir_key[4] = 0xaf50fb04;
         ir_key[5] = 0xed12fb04;
         ir_key[6] = 0xb04ffb04;
         ir_key[7] = 0xb34cfb04;
         ir_key[8] = 0xf10efb04;
         ir_key[9] = 0xb44bfb04;
         ir_key[10] = 0xb748fb04;
         ir_key[11] = 0xf50afb04;
         ir_key[12] = 0xff00fb04; // CARD key for Netflix test
         len = 13;
     }
	 else if(ir_key[0] == 18)  {
         ir_key[1] = 0x2ad5b916;
         ir_key[2] = 0xef10b916;
         ir_key[3] = 0x2ad5be16;//0xaf50b916;
         ir_key[4] = 0x31ceb916;
         ir_key[5] = 0x32cdb916;
         ir_key[6] = 0x33ccb916;
         ir_key[7] = 0x34cbb916;
         ir_key[8] = 0x35cab916;
         ir_key[9] = 0x36c9b916;
         ir_key[10] = 0x37c8b916;
         ir_key[11] = 0x38c7b916;
         ir_key[12] = 0x39c6b916;
         len = 13;
     }
     else {
         venus_ir_keylist_to_scanlist(user_key+1, (uint32_t)len, ir_key+1, (uint32_t)len);
     }
	 #else
	 venus_ir_keylist_to_scanlist(user_key+1, (uint32_t)len, ir_key+1, (uint32_t)len);
	 #endif
    for(i=0;i<(len-1);i++){
        EMCU_DEBUG("irda keycode%d=0x%08x\n",i,ir_key[i+1]);
    }

//#define EMCU_FAKE_IR_KEY
#ifdef EMCU_FAKE_IR_KEY
		EMCU_ALERT("hack for testing\n");
		ir_key[0] = 0x11;   //protocol
		ir_key[1] = 0x123456aa;
		ir_key[2] = 0x123456bb;
		ir_key[3] = 0x123456cc;
		EMCU_ALERT("irda protocol=0x%x\n",ir_key[0]);
        for(i=0;i<(len-1);i++){
			EMCU_ALERT("irda keycode%d=0x%08x\n",i,ir_key[i+1]);
		}
#endif
#else
		EMCU_ALERT("irda keycode transfer function not ready\n");
		return -EINVAL;
#endif
	if(new_protocol_enable==1){
#if 0 //just for debug, open uart1
		//pinmux
		rtd_maskl(0xb8060218, 0x0f0fffff, 0x60600000);
		rtd_maskl(0xb8060248, 0x0ffffff8, 0x00000002);
#endif
	}
	powerMgr_setup_irda(ir_key[0],len-1,ir_key+1);

    return 0;
#else
    EMCU_ALERT("CONFIG_RTK_KDRV_IR has't enable\n");
    return -EINVAL;
#endif
}
fs_initcall(rtk_emcu_module_init);
module_exit(rtk_emcu_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexkh <alexkh@realtek.com>");

#ifndef CONFIG_RTK_KDRV_EMCU_MODULE
static int rtk_emcu_ac_alert(char *s){

    EMCU_ALERT("%s(%u)\n",__func__,__LINE__);
    emcu_ac_on_alt  = 1;

    return 0;
}
__setup("ac_on",    rtk_emcu_ac_alert);
#else
static int rtk_emcu_ac_alert(void){
	emcu_ac_on_alt = rtk_parse_commandline("ac_on");
	EMCU_ALERT("%s(%u)ac_on alert = %d\n",__func__,__LINE__,emcu_ac_on_alt);
	return 0;
}
#endif
