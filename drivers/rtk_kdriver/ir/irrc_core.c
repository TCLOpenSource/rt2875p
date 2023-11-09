//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>       /* DBG_PRINT() */
#include <linux/slab.h>         /* kmalloc() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/sysfs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/freezer.h>
#include <linux/device.h>

//#include <asm/system.h>               /* cli(), *_flags */
#include <linux/uaccess.h>        /* copy_*_user */
#include <linux/sched.h>
//#include <linux/devfs_fs_kernel.h> /* for devfs */
#include <linux/time.h>
#include <rtk_kdriver/io.h>
#include <asm/irq.h>
#include <linux/signal.h>
#include <asm/bitops.h>         /* bit operations */
#include <linux/atomic.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <rtk_kdriver/rtk_irrc_event.h>
#include "irrc_core.h"
#include "irrc_raw.h"
#include "irrc_record.h"
#include "irrc_filter.h"

#define IRRC_IRRP_FLUSH()                          \
    do {                                            \
        /*@4 for main0 ir_sr*/                      \
        while(rtd_inl(REG_IR_SR) & 0x1) {           \
            /*@4 clear */                           \
            /*rtd_outl(REG_IR_SR, 0x00000003);*/    \
            rtd_outl(REG_IR_SR, 0x01011131);        \
            rtd_inl(REG_IR_RP);                     \
            rtd_inl(REG_IR_RP2);                     \
        }                                           \
        /*@4 for main1 ir_sr begin*/                \
        while(rtd_inl(REG_IR_SR_MAIN1) & 0x1) {     \
            rtd_outl(REG_IR_SR_MAIN1, 0x01011131);  \
            rtd_inl(REG_IR_RP_MAIN1);           \
        }   rtd_inl(REG_IR_RP2_MAIN1);          \
        /*@4 for main1 ir_sr end*/                  \
    /*@4 mod */ \
    /*rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) | BIT(IRDA_IR_CR_irrp_clr_shift));*/ \
    rtd_outl(REG_IR_MAIN0, rtd_inl(REG_IR_MAIN0) | BIT(REG_IR_RP_RESET_SHIFT));\
    rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) | BIT(REG_IR_RP_MAIN1_RESET_SHIFT));\
    }while(0)

#define IRRC_RAW_FIFO_FLUSH() \
    do {                    \
        unsigned int i = 0;         \
        unsigned int raw_fifo_level = rtd_inl(REG_IR_RAW_CTRL) & 0x7F;   \
        for(i = 0; i < raw_fifo_level; i++)   \
            rtd_inl(REG_IR_RAW_FF);   \
        rtd_outl(REG_IR_RAW_CTRL, (0x01000000 | raw_fifo_level));   \
    }while(0)

struct IRDA_PROTOCOL irda_proto_list[IRDA_PROTO_LIST_SIZE] = {
    {0, 0, "", "IR_none"},
    {1, NEC, "NEC", "IR_nec"},
    {2, RC5, "RC5", "IR_rc5"},
    {3, SHARP, "SHARP", "IR_sharp"},
    {4, SONY, "SONY", "IR_sony"},
    {5, C03, "C03", "IR_c03"},
    {6, RC6, "RC6", "IR_rc6"},
    {7, NEC, "RAW_NEC", "IR_nec"},
    {8, RCA, "RCA", "IR_rca"},
    {9, PANASONIC, "PANASONIC", "IR_panasonic"},
    {10, KONKA, "KONKA", "IR_konka"},
    {11, RC6, "RAW_RC6", "IR_rc6"},
    {12, TSB, "TSB", "IR_toshiba"},
    {13, TSB, "RAW_TSB", "IR_toshiba"},
    {14, RCA, "RAW_RCA", "IR_rca"},
    {15, C03, "RAW_C03", "IR_c03"},
    {16, KONKA, "RAW_KONKA", "IR_konka"},
    {17, SONY, "RAW_SONY", "IR_sony"},
    {18, AEHA, "AEHA", "IR_aeha"},
    {19, RC3, "RC3", "IR_RC3"},
};


#ifdef CONFIG_QUICK_HIBERNATION
extern int in_suspend;
#endif
static unsigned int driver_mode = SINGLE_WORD_IF;
/*
*main1decode for ir_support_protocols[0],main0decode for ir_support_protocols[1]
*/
int ir_support_protocols[MAX_HW_RTK_KDRV_PROTOCOLS_NUM] = {IR_PROTOCOL, 0};
int ir_protocol = IR_PROTOCOL;

static IRDA_BOOT_PARAMETER   ir_boot_parameter_settings[MAX_IRDA_BOOT_PARAMETER_NUM];

static atomic_t ir_suspended = ATOMIC_INIT(0);
uint32_t ir_disable_irda_decode_key  = 0;
unsigned int ir_repeat_interval = REPEAT_INTERVAL;
unsigned int debounce = 100;
uint64_t lastRecvTick;
uint32_t lastSrValue = 0;
bool ir_decode_dbg_en = false;
DECLARE_WAIT_QUEUE_HEAD(ir_read_wait);
DEFINE_SPINLOCK(ir_lock);

#ifdef CONFIG_DELAY_FIRST_REPEAT
static uint64_t PressRecvTick;
unsigned int firstRepeatInterval = 300; // default disable first repeat delay
static unsigned int firstRepeatTriggered = false;
#endif

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
unsigned int repeat_event = 0;
unsigned int rawkey_event = 0;
extern eirda_report_mode ir_report_mode;
#endif

struct kfifo ir_fifo;
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE) || defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE) || defined(CONFIG_RTK_KDRV_TV035_IR)
struct delayed_work ir_work;
struct kfifo ir_fifo_raw;
int first_fifo = 0;
struct irrplen ir_len[2];
#endif

#if defined(CONFIG_RTK_KDRV_TV030_IR)
#if defined(CONFIG_RTK_KDRV_TV030_JP_IR)
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_TV030_JP;
#else
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_TV030;
#endif
#elif defined(CONFIG_RTK_KDRV_TV036_10D_IR)
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_TV036_10D;
#elif defined(CONFIG_RTK_KDRV_TV001_IR) && !defined(CONFIG_RTK_KDRV_TV002_IR)
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_NEC;
#elif defined(CONFIG_RTK_KDRV_TV002_IR)
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_TV002;
#else
eirda_keymap_type ir_keymap_type = IRDA_KEYMAP_TYPE_TV036_10D;
#endif
EXPORT_SYMBOL(ir_keymap_type);

IR_SCANCODE_FILTER_LIST g_scancode_filter_list; /*white name list*/
char *g_scancode_filter_params = NULL;
uint32_t g_scancode_filter_function_enable = 1;
char g_ir_customer_name[32] = {0};
unsigned int g_last_sony_key_bits_len = 0;
uint64_t get_cur_system_counter(void)
{
	uint64_t val;
#if defined (CONFIG_ARM64)
    asm volatile("mrs %0, cntpct_el0" :"=r" (val));
#else
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (val));
#endif
	return val;
}

uint64_t  get_take_time_ns(uint64_t start, uint64_t end)
{
	uint64_t take_time = 0;
	if(end >= start)
                take_time = (end - start);
        else
                take_time = 0xFFFFFFFFFFFFFFFF - start + end;
	take_time = take_time * 37;
	return take_time;

}

static int _ir_get_keymap(const char* remoter, eirda_keymap_type* type)
{
    if(NULL == remoter || NULL == type)
    {
        IR_ERR("input invalid paramters\n");
        return -1;
    }

    if(0==strcmp(remoter,"IR_nec"))
        *type = IRDA_KEYMAP_TYPE_NEC;
    else if (0 == strcmp(remoter,"IR_hikeen"))
        *type = IRDA_KEYMAP_TYPE_TV036_10D;
    else if (0 == strcmp(remoter,"IR_sony"))
        *type = IRDA_KEYMAP_TYPE_TV002;
    else if (0 == strcmp(remoter,"IR_rca"))
        *type = IRDA_KEYMAP_TYPE_TV030;
    else if (0 == strcmp(remoter,"IR_aeha"))
        *type = IRDA_KEYMAP_TYPE_TV030_JP;
    else if (0 == strcmp(remoter,"IR_tcl_kt1752"))
        *type = IRDA_KEYMAP_TYPE_TV030_KT1752;
	else if (0 == strcmp(remoter,"IR_tclnec"))
        *type = IRDA_KEYMAP_TYPE_TV030_NEC;
    else
    {
        *type = IRDA_KEYMAP_TYPE_TV036_10D;
    }
    IR_INFO("switch irda keymap %s, type %d\n",remoter, *type);

    return 0;
}


#if defined(CONFIG_RTK_KDRV_HAIER_IR)
static struct timer_list led_flash_timer;
static unsigned int isLedFlash = 0;

static void venus_ir_led_flash_start(void)
{
    unsigned long led_flash_jiffies = jiffies + msecs_to_jiffies(200);
//      rtd_maskl(0xb8061108, ~BIT(11), 0);
    rtk_gpio_output(rtk_gpio_id(ISO_GPIO, 11), 0);
    mod_timer(&led_flash_timer, led_flash_jiffies);
}

void venus_ir_led_flash_end(unsigned long arg)
{
    unsigned long led_flash_jiffies = 0;
    if(isLedFlash < 5 ) {
        if(isLedFlash % 2 == 0)
//                      rtd_maskl(0xb8061108, ~BIT(11), BIT(11));
            rtk_gpio_output(rtk_gpio_id(ISO_GPIO, 11), 1);
        else
//                      rtd_maskl(0xb8061108, ~BIT(11), 0);
            rtk_gpio_output(rtk_gpio_id(ISO_GPIO, 11), 0);
        led_flash_jiffies = jiffies + msecs_to_jiffies(200);
        mod_timer(&led_flash_timer, led_flash_jiffies);
        isLedFlash++;
    } else {
//              rtd_maskl(0xb8061108, ~BIT(11), BIT(11));
        rtk_gpio_output(rtk_gpio_id(ISO_GPIO, 11), 1);
        isLedFlash = 0;
    }
}
#endif

static int _tune_ir_rp_value(int protocol, int core, int flag, uint32_t* rpReg, uint32_t* rpReg2)
{
    char* dbgOut = "";
    // notice AEHA 24/32
    switch(protocol)
    {
    // it does not have AEHA for scancode to keycode
    case AEHA:
        IR_DBG("[%s] tune AEHA irrp value\n",__func__);
        if(flag==48)
        {
            *rpReg2 = (((*rpReg2) << 16) & 0xffff0000) | (((*rpReg) >> 16) & 0xffff);
            *rpReg  = ((*rpReg) & 0xffff);
            dbgOut = "48";
        } else if(flag == 40) {
            uint32_t tmp = * rpReg;
            * rpReg = * rpReg2;
            * rpReg2 = tmp;
	 }
        break;

    case SONY:
        // data_num 20
        if(flag & 0x100)
        {
            g_last_sony_key_bits_len = 20;
            dbgOut = "20";
        }
        // 2nd data num 15
        else if(flag & 0x1000)
        {
            g_last_sony_key_bits_len = 15;
            dbgOut = "15";
        }
        // 3rd data num 12
        else if(flag & 0x10000)
        {
            g_last_sony_key_bits_len = 12;
            dbgOut = "12";
        }
        else {
            g_last_sony_key_bits_len = 0;
            IR_ERR("[%s] there is no correct length, fail to decode for Sony\n",__func__);
        }
        break;

    case NEC:
    case TSB:
    case C03:
    case RAW_C03:
    case SHARP:
    case PANASONIC:
    case KONKA: //wangzhh add ,this value must same with the AP layer 20120927
    case RC5:
    case RC3:
    case RC6:
    case RCA:
    case RAW_RCA:
    //RAW_RC6 = 11,
    //RAW_TSB = 13,
    //RAW_NEC:
    //RAW_KONKA = 16,
    //RAW_SONY = 17,
    default:
        break;
    }

    IR_DBG("[%s] protocol=%d core=%u after tune, REG_IR_RP = [%08X] REG_IR_RP2 = [%08X] message=%s\n"
            , __func__
            ,ir_protocol
            ,core
            ,*rpReg
            ,*rpReg2
            ,dbgOut);
    return 0;
}

static int _get_ir_rp_reg(int protocol, int core, uint32_t* rpReg, uint32_t* rpReg2)
{
    // notice C03, konka, sony, AEHA
    switch(protocol)
    {
    case NEC:
    case TSB:
    case C03:
    case SONY:
    case RAW_C03:
    case SHARP:
    case PANASONIC:
    case RCA:
        if(0==core)
        {
            *rpReg  = REG_IR_RP2;   // high
            *rpReg2 = REG_IR_RP;    // low
        }
        else
        {
            *rpReg  = REG_IR_RP2_MAIN1; // high
            *rpReg2 = REG_IR_RP_MAIN1;  // low
        }
        IR_DBG("[%s] main%d irrp=high irrp2=low\n",__func__,core);
        break;

    case KONKA: //wangzhh add ,this value must same with the AP layer 20120927
    case RC5:
    case RC3:
    case RC6:
    case RAW_RCA:
        if(0==core)
        {
            *rpReg  = REG_IR_RP;    // low
            *rpReg2 = REG_IR_RP2;   // high
        }
        else
        {
            *rpReg  = REG_IR_RP_MAIN1;// low
            *rpReg2 = REG_IR_RP2_MAIN1;// high
        }
        IR_DBG("[%s] main%d irrp=low irrp2=high\n",__func__,core);
        break;

    case AEHA:
    //RAW_RC6 = 11,
    //RAW_TSB = 13,
    //RAW_NEC:
    //RAW_KONKA = 16,
    //RAW_SONY = 17,
    default:
        if(0==core)
        {
            *rpReg  = REG_IR_RP;    // low
            *rpReg2 = REG_IR_RP2;   // high
        }
        else
        {
            *rpReg  = REG_IR_RP_MAIN1;
            *rpReg2 = REG_IR_RP2_MAIN1;
        }
        IR_DBG("[%s] main%d protocol=%d use default irrp=low irrp2=high\n",__func__,core,protocol);
        break;
    }

    return 0;
}
/* Module Functions */
static int examine_ir_avail(uint32_t *which, uint32_t *regValue, uint32_t *irrp2Value, int *dataRepeat)
{
    uint8_t data_bits_len   = 0;
    uint32_t core           = 0;
    uint32_t srValue        = rtd_inl(REG_IR_SR);
    uint32_t srReg          = 0;
    uint32_t crReg          = 0;
    uint32_t rpReg          = 0;
    uint32_t rpReg2         = 0;
    uint32_t crRegVal          = 0;
    *irrp2Value = 0;

    IR_DBG("[%s]REG_IR_SR_MAIN0 = [%08X]\n", __func__, srValue);
    // main0 ir_dvf
    if(srValue & BIT(0))
    {
    	*which = MAIN0;
        ir_protocol = ir_support_protocols[1];
        srReg       = REG_IR_SR;
        crReg       = REG_IR_CR;
        core        = 0;
	 crRegVal = rtd_inl(crReg);
        _get_ir_rp_reg(ir_protocol,core,&rpReg,&rpReg2);
    }
    // main1
    else
    {
    	*which = MAIN1;
        ir_protocol = ir_support_protocols[0];
        srReg       = REG_IR_SR_MAIN1;
        crReg       = REG_IR_CR_MAIN1;
        core        = 1;
	 crRegVal = rtd_inl(crReg);
        _get_ir_rp_reg(ir_protocol,core,&rpReg,&rpReg2);

        srValue     = rtd_inl(srReg);
        IR_DBG("[%s]REG_IR_SR_MAIN1 = [%08X %08x]\n", __func__, srValue, crRegVal);
    }
    IR_DBG("[%s]The ir_protocol is %d from main%d\n",
           __FUNCTION__, ir_protocol, core);

    /* mark this, because there is no (srValue & 0x10) in merlin4
    if((srValue & 0x10) && ((srValue & BIT(9)) || ir_protocol != RAW_C03)) {
        rtd_outl(REG_IR_SR, 0x00000013);
        return -ERESTARTSYS;
    }  else
    */
    if(srValue & 0x1) {
        uint8_t repeat_key_invalid = 0;

        if(srValue & 0x10) {
            *dataRepeat = 1;
            repeat_key_invalid = (srValue & 0x20) ? 1 : 0;
        } else {
            *dataRepeat = 0;
        }

        //rtd_outl(REG_IR_SR, 0x00000003); /* clear IRDVF */
        if(0==core)
            rtd_outl(REG_IR_SR,      0x01011131); // clear IRDVF
        else
            rtd_outl(REG_IR_SR_MAIN1,0x01011131); /* clear IRDVF */

        // read irrp2
        *irrp2Value = rtd_inl(rpReg2);

        if(*dataRepeat && repeat_key_invalid)
            return -ERESTARTSYS;

        if(ir_protocol == AEHA) {// AEHA has 24/32/48 bits len
            // main1, without main0
            //data_bits_len = (rtd_inl(REG_IR_CR_MAIN1) >> 24) & 0x3F;
            data_bits_len = (crRegVal >> 24) & 0x3F;
            IR_DBG("[%s]AEHA data_bits_len = [%d]\n", __func__, data_bits_len);
            if((data_bits_len != 24) && (data_bits_len != 32)
			&& (data_bits_len != 40) && (data_bits_len != 48))
            {
                return -ERESTARTSYS;
            }

        }

        *regValue = rtd_inl(rpReg); /* read IRRP */

#ifdef CONFIG_RTK_KDRV_TV030_MOUSE
        // this is used in TCL, when this condition is satisfied, it should be RCA protocol
        if(ir_protocol == RAW_RCA && (srValue & BIT(9))) {
            *regValue = ~(*regValue) & 0x00FFFFFF;
        }
#endif
        IR_DBG("[%s]REG_IR_RP = [%08X] REG_IR_RP2 = [%08X]\n", __func__, *regValue,*irrp2Value);

        if(SONY == ir_protocol)
            _tune_ir_rp_value(ir_protocol,core,srValue,regValue,irrp2Value);
        else if(AEHA == ir_protocol)
            _tune_ir_rp_value(ir_protocol,core,data_bits_len,regValue,irrp2Value);

       /*do ir event notify*/
#ifdef CONFIG_RTK_KDRV_SUPPORT_IR_EVENT
       if(SONY == ir_protocol){
            irrc_event_notify();
         }
#endif

        //mark data repeat
        if(ir_protocol == RC6) {
            if((lastSrValue & 0xffff) == (*regValue & 0xffff)) {
                *dataRepeat = 1;
                debounce = 0;
            } else
                *dataRepeat = 0;
            lastSrValue = *regValue;
        } else if(ir_protocol == RCA) {
            if(lastSrValue == *regValue) {
                *dataRepeat = 1;
                debounce = 0; /* for first repeat interval*/
            } else
                *dataRepeat = 0;
            lastSrValue = *regValue;
        }
        //else if((srValue & BIT(9)) == 0 && ir_protocol == RAW_C03 && (*regValue & 0xffff) != 0xf1f1) {
        // there is no main0, so remove srValue & BIT(9)
        else if(ir_protocol == RAW_C03 && (*regValue & 0xffff) != 0xf1f1) {
            return -ERESTARTSYS;
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        } else if(ir_protocol == RAW_C03) {
            int iCustomCode = venus_ir_input_get_customcode();
            if((*regValue & 0xffff) != iCustomCode && (*regValue & 0xffff) != 0xf5f5)
                return -ERESTARTSYS;
#endif
        } else if(ir_protocol == SONY) {
            if(g_last_sony_key_bits_len == 0)
                return -ERESTARTSYS;
            
            if((lastSrValue & (~((1 << (32 - g_last_sony_key_bits_len)) - 1))) == (*regValue & (~((1 << (32 - g_last_sony_key_bits_len)) - 1)))) {
                *dataRepeat = 1;
                debounce = 0;
            } else
                *dataRepeat = 0;
            lastSrValue = *regValue;
        }

        return 0;
    } else {
		return -ERESTARTSYS;
    }
}

static int repeat_key_handle(uint32_t which, uint32_t regValue, uint32_t irrp2Value, int dataRepeat)
{
    int received = 0;

#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    u32 keycode = 0;
    keycode = venus_ir_scancode_to_keycode(regValue);
#endif

    if(dataRepeat == 1 && get_take_time_ns(lastRecvTick, get_cur_system_counter()) < debounce * NS_PER_MS) {
        IR_DBG(" %lluns, ignored..\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
        lastRecvTick = get_cur_system_counter();
    }
#ifdef CONFIG_DELAY_FIRST_REPEAT
    else if(dataRepeat == 1 && firstRepeatInterval > 0 && firstRepeatTriggered == false && get_take_time_ns(PressRecvTick, get_cur_system_counter()) < firstRepeatInterval * NS_PER_MS) {
        IR_DBG(" %lluns, ignored for first repeat interval..\n", get_take_time_ns(PressRecvTick, get_cur_system_counter()));
        lastRecvTick = get_cur_system_counter();
    }
#endif
    else if(ir_protocol == NEC && dataRepeat == 1 && get_take_time_ns(lastRecvTick, get_cur_system_counter()) > REPEAT_MAX_INTERVAL * NS_PER_MS) {
        //@4 mod
        //rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) | BIT(IRDA_IR_CR_irrp_clr_shift));
        rtd_outl(REG_IR_MAIN0, rtd_inl(REG_IR_MAIN0) | BIT(REG_IR_RP_RESET_SHIFT));
    	rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) | BIT(REG_IR_RP_MAIN1_RESET_SHIFT));
        IR_DBG("Venus IR: Repeat Symbol after %lluns, ignored..\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
    } else if(ir_protocol == RAW_C03 && dataRepeat == 1 && get_take_time_ns(lastRecvTick, get_cur_system_counter()) > REPEAT_MAX_INTERVAL * NS_PER_MS) {
        //@4 mod
        //rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) | BIT(IRDA_IR_CR_irrp_clr_shift));
        rtd_outl(REG_IR_MAIN0, rtd_inl(REG_IR_MAIN0) | BIT(REG_IR_RP_RESET_SHIFT));
    	rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) | BIT(REG_IR_RP_MAIN1_RESET_SHIFT));
        IR_DBG("Venus IR: Repeat Symbol after %lluns, ignored..\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        schedule_venus_ir_keyup_handler(which);
#endif
    } else if(ir_protocol == KONKA && dataRepeat == 0 && get_take_time_ns(lastRecvTick, get_cur_system_counter()) < ir_repeat_interval * NS_PER_MS) {
        IR_DBG(" Repeat Symbol after %lluns, ignored..\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
    }

    else if(dataRepeat == 1 && get_take_time_ns(lastRecvTick, get_cur_system_counter()) < ir_repeat_interval * NS_PER_MS) {
        IR_DBG(" Repeat Symbol after %lluns, ignored..\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
    } else {
        IR_DBG(" Non-repeated frame [%lluns]\n", get_take_time_ns(lastRecvTick, get_cur_system_counter()));
        IR_DBG(" IRRP = [%08X].\n", regValue);
        lastRecvTick = get_cur_system_counter();
        if(ir_protocol == RC6) {
            regValue = ~(regValue) & 0x1fffff; // 21 bits
            IR_DBG(" IRRP = [%08X].\n", regValue);
        } else if(ir_protocol == RC5)
            regValue = (regValue & 0xfff);

        if(ir_protocol == PANASONIC && irrp2Value != 0x20020000) {
            IR_DBG(" Maker code = [%08X].\n", irrp2Value);
        } else {

            kfifo_in(&ir_fifo, (unsigned char *)&regValue, sizeof(uint32_t));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
            venus_ir_input_report_key(dataRepeat);

            if(!ir_poll_settings[which][0]) {
                venus_ir_input_report_end(NULL);
            } else {
                schedule_venus_ir_keyup_handler(which);
            }
#endif
            if(ir_protocol == AEHA) {
                kfifo_in(&ir_fifo, (unsigned char *)&irrp2Value, sizeof(int32_t));
            }

            if(driver_mode == DOUBLE_WORD_IF)
                kfifo_in(&ir_fifo, (unsigned char *)&dataRepeat, sizeof(int32_t));

#ifdef CONFIG_DELAY_FIRST_REPEAT
            if(dataRepeat == 1)
                firstRepeatTriggered = true;
            else {
                PressRecvTick = lastRecvTick;
                firstRepeatTriggered = false;
            }
#endif

            received = 1;
        }
    }

    return received;
}

static void IR_hw_decode_handler(void)
{
    int received = 0;
    int dataRepeat = 0;
    uint32_t which;
    uint32_t regValue;
    uint32_t irrp2Value = 0x0;

    while(examine_ir_avail(&which, &regValue, &irrp2Value, &dataRepeat) == 0) {
        received = repeat_key_handle(which, regValue, irrp2Value, dataRepeat);
    }

    if(received == 1) {
        wake_up_interruptible(&ir_read_wait);
#if defined(CONFIG_RTK_KDRV_HAIER_IR)
        venus_ir_led_flash_start();
#endif
    }
}

#ifdef CONFIG_RTK_KDRV_RAW_MODE
static irqreturn_t IR_raw_decode_handler(void)
{
    int received = 0;
    int dataRepeat = 0;
    uint32_t regValue;
    uint32_t irrp2Value = 0x0;


    switch(ir_support_protocols[0]) {
    case RAW_NEC:
        ir_protocol = ir_support_protocols[0];
#ifdef CONFIG_RTK_KDRV_TV005_MOUSE
        IR_raw_fifo_decode(16, ir_len[1].fifo_len);
#else
        regValue = raw_nec_decoder(&dataRepeat);
        if(regValue == 0) {
            return IRQ_HANDLED;
        }
        received = repeat_key_handle(MAIN1, regValue, irrp2Value, dataRepeat);
#endif
        break;
    case RAW_RC6:
        regValue = raw_rc6_decoder(&dataRepeat);
        if(regValue == 0x8000) {
            return IRQ_HANDLED;
        }
        received = repeat_key_handle(MAIN1, regValue, irrp2Value, dataRepeat);
        break;
    case RAW_TSB:
        regValue = raw_tsb_decoder(&dataRepeat);
        if(regValue == 0x8000) {
            return IRQ_HANDLED;
        }
        received = repeat_key_handle(MAIN1, regValue, irrp2Value, dataRepeat);
        break;
#ifdef CONFIG_RTK_KDRV_TV002_IR_RAW_MODE
    case RAW_SONY:
        ir_protocol = ir_support_protocols[0];
        regValue = raw_sony_decoder(&dataRepeat);
        if(regValue == 0) {
            return IRQ_HANDLED;
        }
        received = repeat_key_handle(MAIN1, regValue, irrp2Value, dataRepeat);
#ifdef CONFIG_RTK_KDRV_SUPPORT_IR_EVENT
        irrc_event_notify();
#endif
        break;
#endif
#ifdef CONFIG_RTK_KDRV_YK_54LU_SHUTTLE
    case RAW_C03:
        IR_raw_fifo_decode(16, 64);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV035_IR
    case RAW_KONKA:
        IR_raw_fifo_decode(16, ir_len[0].fifo_len);
        break;
#endif
#ifdef  CONFIG_RTK_KDRV_TV030_MOUSE
    case RAW_RCA:
        IR_raw_fifo_decode(16, 128);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_RECORD_KEY
    case RAW_RECORDER:
        IR_raw_record_decode();
        break;
#endif
    default:
        IR_WARNING(" the ir protocol isn't support%d\n", ir_support_protocols[0]);
    }
    if(received == 1) {
        wake_up_interruptible(&ir_read_wait);
#if defined(CONFIG_RTK_KDRV_HAIER_IR)
        venus_ir_led_flash_start();
#endif
    }
    // clear fifo for clear raw irq
    IRRC_RAW_FIFO_FLUSH();
    return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_RTK_KDRV_RAW_MODE
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE)|| defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE) || defined(CONFIG_RTK_KDRV_TV035_IR)
static void ir_dsr(struct work_struct * work)
{
    switch(ir_support_protocols[0]) {
    case RAW_NEC:
#ifdef CONFIG_RTK_KDRV_TV005_MOUSE
        ir_dsr_nec_decode();
#endif
        break;
#ifdef CONFIG_RTK_KDRV_YK_54LU_SHUTTLE
    case RAW_C03:
        ir_dsr_sky_decode();
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV035_IR
    case RAW_KONKA:
        ir_dsr_konka_decode();
        break;
#endif
#ifdef  CONFIG_RTK_KDRV_TV030_MOUSE
    case RAW_RCA:
        ir_dsr_rca_decode();
        break;
#endif
    default:
        IR_WARNING(" the ir protocol[%d] isn't handled\n", ir_support_protocols[0]);
    }
}
#endif
#endif

int ir_hw_decode_check_enable(void)
{
    //@4
    //if((rtd_inl(REG_IR_CR) & REG_IR_CR_MAIN0_EN_MASK) || (rtd_inl(REG_IR_CR_MAIN1) & REG_IR_CR_MAIN1_EN_MASK)) {
    if((rtd_inl(REG_IR_CR) & REG_IR_CR_MAIN0_EN_MASK) || (rtd_inl(REG_IR_CR_MAIN1) & REG_IR_CR_MAIN1_EN_MASK)) {
        return 1;
    }

    return 0;
}

int ir_hw_decode_int_status(void)
{
    return rtd_inl(REG_ISR) & BIT(REG_ISR_IR_IN_SHIFT);
}

int ir_raw_decode_int_status(void)
{
    return rtd_inl(REG_OFF_ISR) & BIT(REG_OFF_ISR_IR_IN_SHIFT);
}

static irqreturn_t IR_interrupt_handler(int irq, void *dev_id)
{
    /* HW mode interrupt */
    if(ir_hw_decode_int_status()) {
        IR_DBG("(HW)Interrupt Handler Triggered.\n");

        if(unlikely(ir_disable_irda_decode_key)) // || unlikely(pm_freezing))
            IRRC_IRRP_FLUSH();
        else if(ir_hw_decode_check_enable())
        {
            IR_hw_decode_handler();
            IRRC_RAW_FIFO_FLUSH();
        }
        return IRQ_HANDLED;
    }
#ifdef CONFIG_RTK_KDRV_RAW_MODE
    /* RAW mode interrupt */
    else if(ir_raw_decode_int_status()) {
        IR_DBG("(RAW)Interrupt Handler Triggered.\n");
        /* Clear interrupt */
        rtd_setbits(REG_IR_RAW_SR, BIT(REG_IR_RAW_SR_FIFO_VAL_SHIFT) | BIT(REG_IR_RAW_SR_FIFO_OV_SHIFT));

        if(unlikely(ir_disable_irda_decode_key)){// || unlikely(pm_freezing)) {
            IRRC_RAW_FIFO_FLUSH();
            return IRQ_HANDLED;
        } else {
            return IR_raw_decode_handler();
        }
    }
#endif
    else {
        return IRQ_NONE;
    }
}


int IR_protocol_setting(int main1_protocol, int main0_protocol)
{

    /* Irda Main1 hw setting */
    switch(main1_protocol) {
    case IR_PROTO_NONE:
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
    case NEC:
        rtd_outl(REG_IR_PSR_MAIN1,      0x4E13133B);
        rtd_outl(REG_IR_PER_MAIN1,      0x014e1327);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0xFFFFFFFF);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0xFFFFFFFF);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00A00090);
        rtd_outl(REG_IR_CR_MAIN1,       0x00004A01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000020);
        // HW decode int en
        rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN1][0] = 1;
        ir_poll_settings[MAIN1][1] = 130;
        ir_repeat_interval = 0;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
        ir_report_mode = IRDA_REPORT_MODE_TCL;
        _ir_get_keymap(ir_boot_parameter_settings[0].keylist_remoter, &ir_keymap_type);
#else
	ir_keymap_type = IRDA_KEYMAP_TYPE_TV036_10D;
#endif
#endif
        debounce = 0;
        break;
    case PANASONIC:
        rtd_outl(REG_IR_PSR_MAIN1,      0x24100E30);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000010);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0X30202050);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0X000000FF);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00000900);
        rtd_outl(REG_IR_CR_MAIN1,       0x00000A01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000030);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case SHARP:
        rtd_outl(REG_IR_PSR_MAIN1,      0x000b1754);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000000);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x0019ffff);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0xff000000);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x007003ff);
        rtd_outl(REG_IR_CR_MAIN1,       0x00000801);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x0000000F);
        //HW decode int en
         rtd_outl(REG_IR_MAIN1,
                 rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case SONY:
        rtd_outl(REG_IR_PSR_MAIN1,      0x1814142D);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000006);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x20282850);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x000000ff);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x0050026E);
        rtd_outl(REG_IR_CR_MAIN1,       0x00001a01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x000C0F14);
        //HW decode int en
         rtd_outl(REG_IR_MAIN1,
                 rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN1][0] = 1;
        ir_poll_settings[MAIN1][1] = 95;
	ir_repeat_interval = 0;
#endif
        ir_keymap_type = IRDA_KEYMAP_TYPE_TV002;
        debounce = 0;
        break;
    case C03:
        rtd_outl(REG_IR_PSR_MAIN1,      0x32101034);
        rtd_outl(REG_IR_PER_MAIN1,      0x00323232);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x40302060);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x40408040);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x000003ff);
        rtd_outl(REG_IR_CR_MAIN1,       0x00004a81);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case TSB:
        rtd_outl(REG_IR_PSR_MAIN1,      0x4E13133B);
        rtd_outl(REG_IR_PER_MAIN1,      0x014e1327);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x80302060);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x802500ff);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00a003ff);
        rtd_outl(REG_IR_CR_MAIN1,       0x00004a01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case RC5:
        rtd_outl(REG_IR_PSR_MAIN1,      0x00242424);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000000);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x00303030);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x00000000);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00d00d00);
        rtd_outl(REG_IR_CR_MAIN1,       0x00000501);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x0000000D);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;

    case RC3:
        rtd_outl(REG_IR_PSR_MAIN1,      0x25102140);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000024);

        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x30353585);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x00000030);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x000003ff);    //a8
        rtd_outl(REG_IR_CR_MAIN1,       0x00000A01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000018);

        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;

    case RC6:
        rtd_outl(REG_IR_PSR_MAIN1,      0x1a100e0e);
        rtd_outl(REG_IR_PER_MAIN1,      0x02000008);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x25303030);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x000000ff);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00d00d00);
        rtd_outl(REG_IR_CR_MAIN1,       0x0000a781);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000014);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case KONKA:
        rtd_outl(REG_IR_PSR_MAIN1,      0x1e103a60);
        rtd_outl(REG_IR_PER_MAIN1,      0x0300001a);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x2d245f99);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x0000002D);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x0076030c);
        rtd_outl(REG_IR_CR_MAIN1,       0x00000B01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000010);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        break;
    case RCA://TCL REMOTE Control
        rtd_outl(REG_IR_PSR_MAIN1,      0x25102147);
        rtd_outl(REG_IR_PER_MAIN1,      0x01000025);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x3c223C78);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x000000ff);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x006100fa);
        rtd_outl(REG_IR_CR_MAIN1,       0x00000a01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000018);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN1][0] = 1;
        ir_poll_settings[MAIN1][1] = 90;
        ir_repeat_interval = 0;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
	ir_report_mode = IRDA_REPORT_MODE_TCL;
	rawkey_event = 1;
#endif
#endif
	ir_keymap_type = IRDA_KEYMAP_TYPE_TV030;
        break;
    case AEHA:
        rtd_outl(REG_IR_PSR_MAIN1,      0x1F100D2F);
        rtd_outl(REG_IR_PER_MAIN1,      0x011F0F0F);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x361B1B4E);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x361B601B);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x00A003ff);
        rtd_outl(REG_IR_CR_MAIN1,       0x00800A01);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000030);
        //HW decode int en
        rtd_outl(REG_IR_MAIN1,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
	ir_poll_settings[MAIN1][0] = 1;
        ir_poll_settings[MAIN1][1] = 150;
        ir_repeat_interval = 0;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
        ir_report_mode = IRDA_REPORT_MODE_TCL;
#endif
#endif
	ir_keymap_type = IRDA_KEYMAP_TYPE_TV030_JP;
        break;
    case RAW_NEC:
        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
#ifdef CONFIG_RTK_KDRV_TV005_MOUSE
        ir_len[0].bit_num = 32;
        ir_len[0].burst_len = 40;
        ir_len[0].silence_len = 20;
        ir_len[0].mod_len = 3;
        ir_len[0].zero_len = 3;
        ir_len[0].one_len = 8;
        ir_len[0].stop_len = 190;
        ir_len[0].fifo_len = 128;
        ir_len[1].stop_len = 25;
        ir_len[1].fifo_len = 64;
#endif
        debounce = 0;
        //disable main1
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
    case RAW_RC6:
    case RAW_TSB: /* for TSB spec, not NEC protocol */
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x03138808);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        debounce = 0;
        //disable main1
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
#ifdef  CONFIG_RTK_KDRV_TV030_MOUSE
    case RAW_RCA:
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[0].bit_num = 24;
        ir_len[0].burst_len = 40;
        ir_len[0].silence_len = 40;
        ir_len[0].mod_len = 5;
        ir_len[0].zero_len = 10;
        ir_len[0].one_len = 20;
        ir_len[0].stop_len = 80;
        ir_len[0].fifo_len = 96;
        ir_len[1].bit_num = 24;
        ir_len[1].burst_len = 18;
        ir_len[1].silence_len = 18;
        ir_len[1].mod_len = 3;
        ir_len[1].zero_len = 6;
        ir_len[1].one_len = 3;
        ir_len[1].stop_len = 33;
        ir_len[1].fifo_len = 32;
        debounce = 0;
        //disable main1
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_YK_54LU_SHUTTLE
    case RAW_C03:
        // this is C03 protocol
        rtd_outl(REG_IR_PSR_MAIN1,      0x32101034);
        rtd_outl(REG_IR_PER_MAIN1,      0x00322532);
        rtd_outl(REG_IR_PSR_UP_MAIN1,   0x40302060);
        rtd_outl(REG_IR_PER_UP_MAIN1,   0x40408040);
        rtd_outl(REG_IR_MAIN1,          0x0000021B);
        rtd_outl(REG_IR_DPIR_MAIN1,     0x000003ff);
        rtd_outl(REG_IR_CR_MAIN1,       0x00004a81);
        rtd_outl(REG_IR_DATA_NUM_MAIN1, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_CR,
                rtd_inl(REG_IR_MAIN1) | (1 << REG_IR_MAIN1_IRDVF_INT_SHIFT));
        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[1].bit_num = 8;
        ir_len[1].burst_len = 12;
        ir_len[1].silence_len = 6;
        ir_len[1].mod_len = 3;
        ir_len[1].zero_len = 3;
        ir_len[1].one_len = 8;
        ir_len[1].stop_len = 10;
        ir_len[1].fifo_len = 16;
        ir_repeat_interval = 200;
        debounce = 0;
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV035_IR
    case RAW_KONKA:
        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[0].bit_num = 17;
        ir_len[0].burst_len = 15;
        ir_len[0].silence_len = 15;
        ir_len[0].mod_len = 3;
        ir_len[0].zero_len = 8;
        ir_len[0].one_len = 13;
        ir_len[0].stop_len = 55;
        ir_len[0].fifo_len = 48;
        debounce = 0;
        //disable main1
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV002_IR_RAW_MODE
    case RAW_SONY:
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
	ir_poll_settings[MAIN1][0] = 1;
        ir_poll_settings[MAIN1][1] = 55;
#endif
        ir_repeat_interval = 0;
        debounce = 0;
        //disable main1
        rtd_outl(REG_IR_CR_MAIN1,       0x0);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_RECORD_KEY
    case RAW_RECORDER:
        rtd_outl(REG_IR_RAW_SF, PLL_RBUS_FRQ / RECORD_RAW_MODE_FREQ - 1);               // use 20000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, (PLL_RBUS_FRQ / RECORD_RAW_MODE_FREQ - 1) / 2);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000020);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        break;
#endif
    default:
        IR_WARNING(" the ir protocol[%d] isn't support for main1 initial\n", main1_protocol);
    }

    /* Irda Main0 hw setting */
    switch(main0_protocol) {
    case IR_PROTO_NONE:
        rtd_outl(REG_IR_CR,       0x0);
        break;
    case NEC:
        rtd_outl(REG_IR_PSR,      0x4E13133B);
        rtd_outl(REG_IR_PER,      0x014e1327);
        rtd_outl(REG_IR_PSR_UP,   0xFFFFFFFF);
        rtd_outl(REG_IR_PER_UP,   0xFFFFFFFF);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00A00090);
        rtd_outl(REG_IR_CR,       0x00004A01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000020);
        // HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN0][0] = 1;
        ir_poll_settings[MAIN0][1] = 130;
        ir_repeat_interval = 0;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
        ir_report_mode = IRDA_REPORT_MODE_TCL;
        _ir_get_keymap(ir_boot_parameter_settings[0].keylist_remoter, &ir_keymap_type);
#else
	ir_keymap_type = IRDA_KEYMAP_TYPE_TV036_10D;
#endif
#endif
        debounce = 0;
        break;
    case PANASONIC:
        rtd_outl(REG_IR_PSR,      0x24100E30);
        rtd_outl(REG_IR_PER,      0x01000010);
        rtd_outl(REG_IR_PSR_UP,   0x30202050);
        rtd_outl(REG_IR_PER_UP,   0X000000FF);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00000900);
        rtd_outl(REG_IR_CR,       0x00000A01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000030);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case SHARP:
        rtd_outl(REG_IR_PSR,      0x000b1754);
        rtd_outl(REG_IR_PER,      0x01000000);
        rtd_outl(REG_IR_PSR_UP,   0x0019ffff);
        rtd_outl(REG_IR_PER_UP,   0xff000000);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x007003ff);
        rtd_outl(REG_IR_CR,       0x00000801);
        rtd_outl(REG_IR_DATA_NUM, 0x0000000F);
        //HW decode int en
         rtd_outl(REG_IR_MAIN0,
                 rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case SONY:
        rtd_outl(REG_IR_PSR,      0x1814142D);
        rtd_outl(REG_IR_PER,      0x01000006);
        rtd_outl(REG_IR_PSR_UP,   0x20202850);
        rtd_outl(REG_IR_PER_UP,   0x000000ff);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x0050026E);
        rtd_outl(REG_IR_CR,       0x00001a01);
        rtd_outl(REG_IR_DATA_NUM, 0x000C0F14);
        //HW decode int en
         rtd_outl(REG_IR_MAIN0,
                 rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN0][0] = 1;
        ir_poll_settings[MAIN0][1] = 95;
	ir_repeat_interval = 0;
#endif

        debounce = 0;
        break;
    case C03:
        rtd_outl(REG_IR_PSR,      0x32101034);
        rtd_outl(REG_IR_PER,      0x00323232);
        rtd_outl(REG_IR_PSR_UP,   0x40302060);
        rtd_outl(REG_IR_PER_UP,   0x40408040);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x000003ff);
        rtd_outl(REG_IR_CR,       0x00004a81);
        rtd_outl(REG_IR_DATA_NUM, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case TSB: /* for TSB spec */
        rtd_outl(REG_IR_PSR,      0x4E13133B);
        rtd_outl(REG_IR_PER,      0x014e1327);
        rtd_outl(REG_IR_PSR_UP,   0x80302060);
        rtd_outl(REG_IR_PER_UP,   0x802500ff);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00a003ff);
        rtd_outl(REG_IR_CR,       0x00004a01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case RC5:
        rtd_outl(REG_IR_PSR,      0x00242424);
        rtd_outl(REG_IR_PER,      0x01000000);
        rtd_outl(REG_IR_PSR_UP,   0x00303030);
        rtd_outl(REG_IR_PER_UP,   0x00000000);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00d00d00);
        rtd_outl(REG_IR_CR,       0x00000501);
        rtd_outl(REG_IR_DATA_NUM, 0x0000000D);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;

    case RC3:
        rtd_outl(REG_IR_PSR,      0x25102140);
        rtd_outl(REG_IR_PER,      0x01000024);

        rtd_outl(REG_IR_PSR_UP,   0x30353585);
        rtd_outl(REG_IR_PER_UP,   0x00000030);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x000003ff);  //a8
        rtd_outl(REG_IR_CR,       0x00000A01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000018);

        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;

    case RC6:
        rtd_outl(REG_IR_PSR,      0x1a100e0e);
        rtd_outl(REG_IR_PER,      0x02000008);
        rtd_outl(REG_IR_PSR_UP,   0x25303030);
        rtd_outl(REG_IR_PER_UP,   0x000000ff);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00d00d00);
        rtd_outl(REG_IR_CR,       0x0000a781);
        rtd_outl(REG_IR_DATA_NUM, 0x00000014);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case KONKA:
        rtd_outl(REG_IR_PSR,      0x1e103a60);
        rtd_outl(REG_IR_PER,      0x0300001a);
        rtd_outl(REG_IR_PSR_UP,   0x2d245f99);
        rtd_outl(REG_IR_PER_UP,   0x0000002D);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x0076030c);
        rtd_outl(REG_IR_CR,       0x00000B01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000010);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
        break;
    case RCA://TCL REMOTE Control
        rtd_outl(REG_IR_PSR,      0x25152147);
        rtd_outl(REG_IR_PER,      0x01000025);
        rtd_outl(REG_IR_PSR_UP,   0x37223672);
        rtd_outl(REG_IR_PER_UP,   0x00000037);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x006100fa);
        rtd_outl(REG_IR_CR,       0x00000a01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000018);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN0][0] = 1;
        ir_poll_settings[MAIN0][1] = 90;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
	ir_report_mode = IRDA_REPORT_MODE_TCL;
	rawkey_event = 1;
#endif
#endif
        break;
    case AEHA:
        rtd_outl(REG_IR_PSR,      0x1F100D2F);
        rtd_outl(REG_IR_PER,      0x011F0F0F);
        rtd_outl(REG_IR_PSR_UP,   0x361B1B4E);
        rtd_outl(REG_IR_PER_UP,   0x361B601B);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x00A003ff);
        rtd_outl(REG_IR_CR,       0x00800A01);
        rtd_outl(REG_IR_DATA_NUM, 0x00000030);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN0][0] = 1;
        ir_poll_settings[MAIN0][1] = 150;
        ir_repeat_interval = 0;
#if defined(CONFIG_RTK_KDRV_TV030_IR )
        ir_report_mode = IRDA_REPORT_MODE_TCL;
#endif
#endif
        break;
    case RAW_NEC:
        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
#ifdef CONFIG_RTK_KDRV_TV005_MOUSE
        ir_len[0].bit_num = 32;
        ir_len[0].burst_len = 40;
        ir_len[0].silence_len = 20;
        ir_len[0].mod_len = 3;
        ir_len[0].zero_len = 3;
        ir_len[0].one_len = 8;
        ir_len[0].stop_len = 190;
        ir_len[0].fifo_len = 128;
        ir_len[1].stop_len = 25;
        ir_len[1].fifo_len = 64;
#endif
        debounce = 0;
        //disable main0
        rtd_outl(REG_IR_CR,       0x0);
        break;

    case RAW_RC6:
    case RAW_TSB: /* for TSB spec, not NEC protocol */
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x03138808);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        debounce = 0;
        //disable main0
        rtd_outl(REG_IR_CR,       0x0);
        break;
#ifdef  CONFIG_RTK_KDRV_TV030_MOUSE
    case RAW_RCA:
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[0].bit_num = 24;
        ir_len[0].burst_len = 40;
        ir_len[0].silence_len = 40;
        ir_len[0].mod_len = 5;
        ir_len[0].zero_len = 10;
        ir_len[0].one_len = 20;
        ir_len[0].stop_len = 80;
        ir_len[0].fifo_len = 96;
        ir_len[1].bit_num = 24;
        ir_len[1].burst_len = 18;
        ir_len[1].silence_len = 18;
        ir_len[1].mod_len = 3;
        ir_len[1].zero_len = 6;
        ir_len[1].one_len = 3;
        ir_len[1].stop_len = 33;
        ir_len[1].fifo_len = 32;
        debounce = 0;
        break;
#endif
#ifdef CONFIG_RTK_KDRV_YK_54LU_SHUTTLE
    case RAW_C03:
        // this is C03 protocol
        rtd_outl(REG_IR_PSR,      0x32101034);
        rtd_outl(REG_IR_PER,      0x00322532);
        rtd_outl(REG_IR_PSR_UP,   0x40302060);
        rtd_outl(REG_IR_PER_UP,   0x40408040);
        rtd_outl(REG_IR_MAIN0,    0x0000021B);
        rtd_outl(REG_IR_DPIR,     0x000003ff);
        rtd_outl(REG_IR_CR,       0x00004a81);
        rtd_outl(REG_IR_DATA_NUM, 0x00000020);
        //HW decode int en
        rtd_outl(REG_IR_MAIN0,
                rtd_inl(REG_IR_MAIN0) | (1 << REG_IR_MAIN0_IRDVF_INT_SHIFT));

        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[1].bit_num = 8;
        ir_len[1].burst_len = 12;
        ir_len[1].silence_len = 6;
        ir_len[1].mod_len = 3;
        ir_len[1].zero_len = 3;
        ir_len[1].one_len = 8;
        ir_len[1].stop_len = 10;
        ir_len[1].fifo_len = 16;
        debounce = 0;
        ir_repeat_interval = 200;
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV035_IR
    case RAW_KONKA:
        rtd_outl(REG_IR_RAW_SF, 0x1517);       // use 5000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x0a8b);  // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        ir_len[0].bit_num = 17;
        ir_len[0].burst_len = 15;
        ir_len[0].silence_len = 15;
        ir_len[0].mod_len = 3;
        ir_len[0].zero_len = 8;
        ir_len[0].one_len = 13;
        ir_len[0].stop_len = 55;
        ir_len[0].fifo_len = 48;
        debounce = 0;
        //disable main0
        rtd_outl(REG_IR_CR,       0x0);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_TV002_IR_RAW_MODE
    case RAW_SONY:
        rtd_outl(REG_IR_RAW_SF, 0xa8b);                 // use 10000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, 0x545);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000010);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        ir_poll_settings[MAIN0][0] = 1;
        ir_poll_settings[MAIN0][1] = 55;
#endif
        ir_repeat_interval = 0;
        debounce = 0;
        //disable main0
        rtd_outl(REG_IR_CR,       0x0);
        break;
#endif
#ifdef CONFIG_RTK_KDRV_RECORD_KEY
    case RAW_RECORDER:
        rtd_outl(REG_IR_RAW_SF, PLL_RBUS_FRQ / RECORD_RAW_MODE_FREQ - 1);               // use 20000 Hz sampling rate
        rtd_outl(REG_IR_RAW_DEB, (PLL_RBUS_FRQ / RECORD_RAW_MODE_FREQ - 1) / 2);        // 2x over sampling for debounce
        rtd_outl(REG_IR_RAW_CTRL, 0x00000020);
        rtd_outl(REG_IR_RAW_CR, 0x7000); // enable raw mode
        break;
#endif
    default:
        IR_WARNING(" the ir protocol[%d] isn't support for main0 initial\n", main0_protocol);
    }

    return 0;
}

void ir_set_rx_detect_on_off(uint32_t on)
{
#ifdef CONFIG_IRDA_POWER_SAVING_MODE
    if(on == ON) {
        rtd_setbits(IRDA_HW_MODE_RX_DECTECT_REG, IRDA_HW_MODE_RX_DECTECT_ON);
        rtd_setbits(IRDA_RAW_MODE_RX_DECTECT_REG, IRDA_RAW_MODE_RX_DECTECT_ON);
    } else {
        rtd_clearbits(IRDA_HW_MODE_RX_DECTECT_REG, IRDA_HW_MODE_RX_DECTECT_ON);
        rtd_clearbits(IRDA_RAW_MODE_RX_DECTECT_REG, IRDA_RAW_MODE_RX_DECTECT_ON);
    }
#endif
}

void ir_global_int_on_off(bool hw_mode, bool on)
{
    if(on == ON) {
        if(hw_mode == HW_MODE)
            rtd_outl(INT_CTRL_SCPU_REG, BIT(INT_CTRL_SCPU_MISC_INT_EN_SHIFT) | BIT(INT_CTRL_SCPU_INT_EN_WRITE_DATA));
        else
            rtd_outl(INT_CTRL_SCPU_REG, BIT(INT_CTRL_SCPU_ISO_INT_EN_SHIFT) | BIT(INT_CTRL_SCPU_INT_EN_WRITE_DATA));
    } else {
        if(hw_mode == HW_MODE)
            rtd_outl(INT_CTRL_SCPU_REG, BIT(INT_CTRL_SCPU_MISC_INT_EN_SHIFT));
        else
            rtd_outl(INT_CTRL_SCPU_REG, BIT(INT_CTRL_SCPU_ISO_INT_EN_SHIFT));
    }
}

void ir_local_int_on_off(bool hw_mode, bool on)
{
    if(on == ON) {
        if(hw_mode == HW_MODE)
        {
            //@???
            //rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) | BIT(REG_IR_CR_HW_INT_SHIFT)); /*enable HW decode interrupts*/
            rtd_outl(REG_IR_MAIN0, rtd_inl(REG_IR_MAIN0) | BIT(REG_IR_MAIN0_IRDVF_INT_SHIFT)); /*enable HW decode interrupts*/
            rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) | BIT(REG_IR_MAIN1_IRDVF_INT_SHIFT)); /*enable HW decode interrupts*/
        }
#ifdef CONFIG_RTK_KDRV_RAW_MODE
        else
            rtd_outl(REG_IR_RAW_CR, rtd_inl(REG_IR_RAW_CR) | BIT(REG_IR_CR_RAW_FIFO_OV_EN_SHIFT) | BIT(REG_IR_CR_RAW_FIFO_VAL_EN_SHIFT)); /*enable raw mode decode interrupts*/
#endif
    } else {
        if(hw_mode == HW_MODE)
        {
            //@???
            //rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) & ~BIT(REG_IR_CR_HW_INT_SHIFT)); /*disable HW decode interrupts*/
            rtd_outl(REG_IR_MAIN0, rtd_inl(REG_IR_MAIN0) & ~BIT(REG_IR_MAIN0_IRDVF_INT_SHIFT)); /*disable HW decode interrupts*/
            rtd_outl(REG_IR_MAIN1, rtd_inl(REG_IR_MAIN1) & ~BIT(REG_IR_MAIN1_IRDVF_INT_SHIFT)); /*disable HW decode interrupts*/
        }
#ifdef CONFIG_RTK_KDRV_RAW_MODE
        else
            rtd_outl(REG_IR_RAW_CR, rtd_inl(REG_IR_RAW_CR) & ~(BIT(REG_IR_CR_RAW_FIFO_OV_EN_SHIFT) | BIT(REG_IR_CR_RAW_FIFO_VAL_EN_SHIFT))); /*disable raw mode decode interrupts*/
#endif
    }
}

void ir_all_int_on_off(bool on)
{
    ir_global_int_on_off(HW_MODE, on);
    ir_global_int_on_off(RAW_MODE, on);
    ir_local_int_on_off(HW_MODE, on);
    ir_local_int_on_off(RAW_MODE, on);
}

void ir_hw_decode_on_off(bool main0, bool on)
{
    if(on == ON) {
        if(main0 == MAIN0)
            rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) | REG_IR_CR_MAIN0_EN(ON)); /*enable main0 decode*/
        else
            rtd_outl(REG_IR_CR_MAIN1, rtd_inl(REG_IR_CR_MAIN1) | REG_IR_CR_MAIN1_EN(ON)); /*enable main1 decode*/
    } else {
        if(main0 == MAIN0)
            rtd_outl(REG_IR_CR, rtd_inl(REG_IR_CR) & ~REG_IR_CR_MAIN0_EN(ON)); /*disable main0 decode*/
        else
            rtd_outl(REG_IR_CR_MAIN1, rtd_inl(REG_IR_CR_MAIN1) & ~REG_IR_CR_MAIN1_EN(ON)); /*disable main1 decode*/
    }
}

void ir_raw_decode_on_off(bool on)
{
    if(on == ON) {
        rtd_setbits(REG_IR_RAW_CR, BIT(REG_IR_CR_RAW_EN_SHIFT)); /*enable raw mode decode*/
    } else {
        rtd_clearbits(REG_IR_RAW_CR, BIT(REG_IR_CR_RAW_FIFO_OV_EN_SHIFT)); /*disable raw mode decode*/
    }
}

void ir_all_decode_on_off(bool on)
{
    ir_hw_decode_on_off(MAIN0, on);
    ir_hw_decode_on_off(MAIN1, on);
    ir_raw_decode_on_off(on);
}

void ir_decode_key_on_off(bool on)
{
    ir_disable_irda_decode_key = ((on ==ON) ? 0 : 1);
    if(atomic_read(&ir_suspended) == 0) {
        IRRC_IRRP_FLUSH();
#ifdef CONFIG_RTK_KDRV_RAW_MODE
        IRRC_RAW_FIFO_FLUSH();
#endif
        ir_local_int_on_off(HW_MODE, on);
        ir_local_int_on_off(RAW_MODE, on);
    }
}


const char* ir_core_get_cur_remote_control_name(void)
{
    char *boot_remoter = NULL;
    boot_remoter = ir_proto_get_boot_remoter_name(ir_support_protocols[0]);
    if(boot_remoter) {
	return boot_remoter;
    } else if((ir_support_protocols[0] >= 0) && (ir_support_protocols[0] < sizeof(irda_proto_list) /sizeof(irda_proto_list[0]))) {
        return irda_proto_list[ir_support_protocols[0]].remote_control_name;
    }
    else {
        return irda_proto_list[0].remote_control_name;
    }
}


/* *** ALL INITIALIZATION HERE *** */
int IR_Init(int main1_protocol, int main0_protocol, bool int_on)
{
    int retval = 0;
    const char *ir_remoter = NULL;

    if(main1_protocol > 0) {
        IR_INFO("!! switch ir protocol to %s for main1 !!\n", irda_proto_list[main1_protocol].proto_name);
    }
    if(main0_protocol > 0) {
        IR_INFO("!! switch ir protocol to %s for main0 !!\n", irda_proto_list[main0_protocol].proto_name);
    }

    lastRecvTick = get_cur_system_counter();

    /*Irda register settings*/
    IR_protocol_setting(main1_protocol, main0_protocol);
    ir_remoter = ir_core_get_cur_remote_control_name();
    if(ir_remoter) {
	_ir_get_keymap(ir_remoter, &ir_keymap_type);
    }

    if(int_on == ON) {
        /* Enable global interrupt */
        ir_global_int_on_off(HW_MODE, ON);
        ir_global_int_on_off(RAW_MODE, ON);
    } else {
        /* Disable interrupt */
        ir_all_int_on_off(OFF);
    }

    ir_set_rx_detect_on_off(ON);
    return retval;
}

int ir_suspend_protocol(void)
{
    /* Pin Mux Pinmux setting in boot, so driver no need setting the pinmux*/
    //rtd_maskl(PINMUX_ST_ST_CFG_1_REG, ~PINMUX_ST_ST_CFG_1_IRRX_MASK, PINMUX_ST_ST_CFG_1_IRRX_DATA);
    /* Clock & Reset */
    CRT_CLK_OnOff(IrDA, CLK_ON, NULL);
    /* Use main1 decode in suspend */
    IR_Init(irda_proto_list[ir_support_protocols[0]].proto_standby_id, 0, OFF);

    /* Disable HW/RAW decode */
    ir_all_decode_on_off(OFF);

    IRRC_IRRP_FLUSH();

	return 0;
}
EXPORT_SYMBOL(ir_suspend_protocol);

int ir_core_pm_suspend(struct device* dev)
{
    atomic_set(&ir_suspended, 1);
    /* Disable ir interrupt */
    ir_all_int_on_off(OFF);
#ifdef CONFIG_QUICK_HIBERNATION
    if(!in_suspend)
        IRRC_IRRP_FLUSH();
#else
    IRRC_IRRP_FLUSH();
#endif

    return 0;
}

int ir_core_pm_resume(struct device * dev)
{
    /* Pin Mux */
    //rtd_maskl(PINMUX_ST_ST_CFG_1_REG, ~PINMUX_ST_ST_CFG_1_IRRX_MASK, PINMUX_ST_ST_CFG_1_IRRX_DATA);
    /* Clock & Reset */
    CRT_CLK_OnOff(IrDA, CLK_ON, NULL);
    // reinitialize ir register
    IR_Init(ir_support_protocols[0], ir_support_protocols[1], ON);

#ifdef CONFIG_QUICK_HIBERNATION
    if (!in_suspend) {
#endif
        /* reinitialize kfifo */
        kfifo_reset(&ir_fifo);
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE) || defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE)|| defined(CONFIG_RTK_KDRV_TV035_IR)
        kfifo_reset(&ir_fifo_raw);
        first_fifo = 0;
#endif
        IRRC_IRRP_FLUSH();
#ifdef CONFIG_QUICK_HIBERNATION
    }
#endif

    atomic_set(&ir_suspended, 0);
    return 0;
}

int ir_core_open(struct inode * inode, struct file * filp)
{
    kfifo_reset(&ir_fifo);
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE) || defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE)  || defined(CONFIG_RTK_KDRV_TV035_IR)
    kfifo_reset(&ir_fifo_raw);
#endif
    return 0;
}


ssize_t ir_core_read(struct file * filp, char __user * buf, size_t count, loff_t *f_pos)
{
    int uintBuf;
    int i, readCount = count;
    int len = 0;

restart:
    if(wait_event_interruptible(ir_read_wait, kfifo_len(&ir_fifo) > 0) != 0) {
        //if(unlikely(current->flags & pm_freezing)) {
        if(freezing(current)){
            try_to_freeze();
            goto restart;
        } else
            return -ERESTARTSYS;
    }

    spin_lock(&ir_lock);
    if(kfifo_len(&ir_fifo) < count)
        readCount = kfifo_len(&ir_fifo);

    for(i = 0 ; i < readCount ; i += 4) {
        len = kfifo_out(&ir_fifo, &uintBuf, sizeof(uint32_t));
        if(copy_to_user(buf, &uintBuf, 4)) {
            spin_unlock(&ir_lock);
            return -EFAULT;
        }
        buf = buf + 4;
    }
    spin_unlock(&ir_lock);

    return readCount;
}

unsigned int ir_core_poll(struct file * filp, poll_table * wait)
{
    poll_wait(filp, &ir_read_wait, wait);
    if(kfifo_len(&ir_fifo) > 0)
        return POLLIN | POLLRDNORM;
    else
        return 0;
}

static int _ir_core_ioctl_flush_irrp(void)
{
        int retval = 0;
        /* Pin Mux */
        //rtd_maskl(PINMUX_ST_ST_CFG_1_REG, ~PINMUX_ST_ST_CFG_1_IRRX_MASK, PINMUX_ST_ST_CFG_1_IRRX_DATA);
        /* Clock & Reset */
        CRT_CLK_OnOff(IrDA, CLK_ON, NULL);
        if((retval = IR_Init(ir_support_protocols[0], ir_support_protocols[1], ON)) == 0)
            kfifo_reset(&ir_fifo);
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
        venus_ir_input_set_input_dev_name(ir_core_get_cur_remote_control_name());
#endif
        return retval;
}

static int _ir_core_ioctl_change_protocol(int which_hw,  int protocol)
{
    if(which_hw == MAIN0)
        ir_support_protocols[1] = protocol;
    else
        ir_support_protocols[0] = protocol;
    return _ir_core_ioctl_flush_irrp();
}

long ir_core_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    u32 keycode = 0;
    unsigned long buf;
#endif
    spin_lock(&ir_lock);
    switch(cmd) {
    case IR_IOC_SET_IRPSR:
        rtd_outl(REG_IR_PSR, arg);
        break;
    case IR_IOC_SET_IRPER:
        rtd_outl(REG_IR_PER, arg);
        break;
    case IR_IOC_SET_IRSF:
        rtd_outl(REG_IR_MAIN0, arg);
        break;
    case IR_IOC_SET_IRCR:
        rtd_outl(REG_IR_CR, arg);
        break;
    case IR_IOC_SET_IRIOTCDP:
        rtd_outl(REG_IR_DPIR, arg);
        break;
    case IR_IOC_SET_PROTOCOL:
        retval = _ir_core_ioctl_change_protocol(MAIN1, arg);
        break;
    case IR_IOC_SET_MAIN0_PROTOCOL:
        retval = _ir_core_ioctl_change_protocol(MAIN0, arg);
        break;
    case IR_IOC_FLUSH_IRRP:
        retval = _ir_core_ioctl_flush_irrp();
        break;
    case IR_IOC_SET_DEBOUNCE:
        debounce = (unsigned int)arg;
        break;
    case IR_IOC_SET_FIRST_REPEAT_DELAY:
#ifdef CONFIG_DELAY_FIRST_REPEAT
        firstRepeatInterval = (unsigned int)arg;
#else
        retval = -EPERM;
#endif
        break;
    case IR_IOC_SET_DRIVER_MODE:
        if(((unsigned int)arg) >= 2)
            retval = -EFAULT;
        else {
            kfifo_reset(&ir_fifo);
            driver_mode = (unsigned int)arg;
        }
        break;
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    case IR_IOC_SET_IR_TO_MOUSE:
        if (((unsigned int)arg == 1) || ((unsigned int)arg == 0)) {
            ir_to_mouse = (unsigned int)arg;
            IR_WARNING("Venus IR: simulate IR as mouse %s\n", ir_to_mouse ? "ON" : "OFF");
        } else
            retval = -EINVAL;
        break;
#endif
    case IR_IOC_GET_PROTOCOL:
        if (copy_to_user((void __user *)arg, (void *)&ir_support_protocols[0], sizeof(ir_support_protocols[0]))) {
            retval = -EFAULT;
        }
        break;
    case IR_IOC_GET_MAIN0_PROTOCOL:
        if (copy_to_user((void __user *)arg, (void *)&ir_support_protocols[1], sizeof(ir_support_protocols[1]))) {
            retval = -EFAULT;
        }
        break;
#ifdef CONFIG_RTK_KDRV_INPUT_DEVICE
    case IR_IOC_SET_CUSTOM:
        retval = venus_ir_input_set_customcode(arg);
        break;
    case IR_IOC_GET_CUSTOM:
        retval = venus_ir_input_get_customcode();
        break;
    case IOCTK_KBD_STATUS:
        retval = __get_user(buf, (unsigned long __user *)arg);
        keycode = venus_ir_scancode_to_keycode(buf);
        venus_ir_input_report_key(0);
        schedule_venus_ir_keyup_handler(MAIN1);
        break;
#endif
    default:
        retval = -ENOIOCTLCMD;
    }
    spin_unlock(&ir_lock);

    return retval;
}

int  ir_core_init(int irq_num)
{
    int result = 0;
    IR_DBG("%s irq_num=%u\n", __func__,irq_num);
    if(0 >= irq_num)
    {
        IR_ERR("fail to get irq_num=%u\n",irq_num);
        return -EINVAL;
    }

    ir_scancode_filter_function_init(&g_scancode_filter_list, NULL);
    ir_scancode_filter_parse_params(&g_scancode_filter_list, g_scancode_filter_params);

#if defined(CONFIG_RTK_KDRV_HAIER_IR) //hardy20140228
    setup_timer(&led_flash_timer, venus_ir_led_flash_end, 0);
#endif
    if(kfifo_alloc(&ir_fifo, FIFO_DEPTH * sizeof(uint32_t), GFP_KERNEL)) {
        result = -ENOMEM;
        goto fail_alloc_hw_kfifo;
    }

#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE)|| defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE) || defined(CONFIG_RTK_KDRV_TV035_IR)
    if(kfifo_alloc(&ir_fifo_raw, FIFO_DEPTH * FIFO_DEPTH * sizeof(uint32_t), GFP_KERNEL)) {
        result = -ENOMEM;
        goto fail_alloc_raw_kfifo;
    }

    INIT_DELAYED_WORK(&ir_work, ir_dsr);
#endif



    /* Pin Mux */
    //rtd_maskl(PINMUX_ST_ST_CFG_1_REG, ~PINMUX_ST_ST_CFG_1_IRRX_MASK, PINMUX_ST_ST_CFG_1_IRRX_DATA);
    IR_WARNING("rtd_inl(REG_ISR) == 0x%08x\n", rtd_inl(REG_ISR));
    /* Clock & Reset */
    CRT_CLK_OnOff(IrDA, CLK_ON, NULL);
    IR_WARNING("rtd_inl(REG_ISR) == 0x%08x\n", rtd_inl(REG_ISR));
    if(request_irq(irq_num, IR_interrupt_handler,
                   IRQF_SHARED, "Venus_IR", IR_interrupt_handler)) {
        IR_WARNING("IR: cannot register IRQ %d\n", irq_num);
        result = -EIO;
        goto fail_alloc_irq;
    }

    /* Hardware Registers Initialization */
    IR_Init(ir_support_protocols[0], ir_support_protocols[1], ON);

    return result;       /* succeed ! */

fail_alloc_irq:
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE)|| defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE) || defined(CONFIG_RTK_KDRV_TV035_IR)
    kfifo_free(&ir_fifo_raw);
fail_alloc_raw_kfifo:
#endif
    kfifo_free(&ir_fifo);
fail_alloc_hw_kfifo:
    return result;
}

void ir_core_uninit(int irq_num)
{
    /* Reset Hardware Registers */
    CRT_CLK_OnOff(IrDA, CLK_OFF, NULL);

#ifdef CONFIG_RTK_KDRV_RECORD_KEY
    IR_record_key_fini();
#endif
    free_irq(irq_num, IR_interrupt_handler);
    kfifo_free(&ir_fifo);
#if defined(CONFIG_RTK_KDRV_TV005_MOUSE) || defined(CONFIG_RTK_KDRV_TV030_MOUSE) || defined(CONFIG_RTK_KDRV_YK_54LU_SHUTTLE)  || defined(CONFIG_RTK_KDRV_TV035_IR)
    kfifo_free(&ir_fifo_raw);
    cancel_delayed_work_sync(&ir_work);
#endif
    ir_scancode_filter_function_uninit(&g_scancode_filter_list);
}

unsigned int ir_get_standby_protocol(void){
    return irda_proto_list[ir_support_protocols[0]].proto_standby_id;
}
EXPORT_SYMBOL(ir_get_standby_protocol);

unsigned int ir_proto_get_boot_powerup_keylist(unsigned int proto,
								unsigned int *key_array, unsigned int arr_len)
{
	unsigned int i = 0;
	if(!proto || !key_array || !arr_len)
		return 0;
	for(i = 0; i < MAX_IRDA_BOOT_PARAMETER_NUM; i++) {
		if(ir_boot_parameter_settings[i].keylist_proto_id == proto) {
			unsigned int n = 0;
			unsigned int size = (ir_boot_parameter_settings[i].keylist_len > arr_len)
								? arr_len : ir_boot_parameter_settings[i].keylist_len;
			for(n = 0; n < size; n++)
				key_array[n] = ir_boot_parameter_settings[i].powerup_keylist[n];
			return size;
		}
	}
	return 0;
}
EXPORT_SYMBOL(ir_proto_get_boot_powerup_keylist);

char * ir_proto_get_boot_remoter_name(unsigned int proto)
{
	unsigned int i = 0;
	if(!proto)
		return NULL;
	for(i = 0; i < MAX_IRDA_BOOT_PARAMETER_NUM; i++) {
		if(ir_boot_parameter_settings[i].keylist_proto_id == proto) {
			return ir_boot_parameter_settings[i].keylist_remoter;
		}
	}
	return NULL;
}


int ir_proto_irda_init(char *options)
{
    unsigned int cnt = 0;
    char *pTmp = NULL;
    unsigned int boot_sel_proto = 0;
    char remoter[32] = {0};
    if(options == NULL)
        return 1;

    IR_INFO("boot select ir protocol:: %s\n", options);
    while(NULL != ( pTmp = strsep(&options, "-"))) {
        if(cnt == 0) {
            char *endp = NULL;
            boot_sel_proto = simple_strtoul(pTmp, &endp, 10);
        } else if(cnt == 1) {
            snprintf(remoter, 32, "%s", pTmp);
        }
        cnt++;
    }
    if(cnt != 2 || !boot_sel_proto)
        return 1;

    ir_support_protocols[0] = boot_sel_proto;
    ir_protocol = boot_sel_proto;
    ir_boot_parameter_settings[0].keylist_proto_id = boot_sel_proto;
    snprintf(ir_boot_parameter_settings[0].keylist_remoter, 32, "IR_%s", remoter);
    _ir_get_keymap(ir_boot_parameter_settings[0].keylist_remoter, &ir_keymap_type);
    IR_INFO("boot select ir protocol1:: boot_sel_proto = %d, remoter = %s, keymap=%d\n",
                    boot_sel_proto, ir_boot_parameter_settings[0].keylist_remoter,ir_keymap_type);
    return 1;
}
EXPORT_SYMBOL(ir_proto_irda_init);

int ir_proto_irda1_init(char *options)
{
    unsigned int cnt = 0;
    char *pTmp = NULL;
    unsigned int boot_sel_proto = 0;
    char remoter[32] = {0};
    if(options == NULL)
        return 1;

    IR_INFO("boot ir_proto_irda1_init:: %s\n", options);
    while(NULL != ( pTmp = strsep(&options, "-"))) {
        if(cnt == 0) {
            char *endp = NULL;
            boot_sel_proto = simple_strtoul(pTmp, &endp, 10);
        } else if(cnt == 1) {
            snprintf(remoter, 32, "%s", pTmp);
        }
        cnt++;
    }
    if(cnt != 2 || !boot_sel_proto)
        return 1;

    ir_boot_parameter_settings[1].keylist_proto_id = boot_sel_proto;
    snprintf(ir_boot_parameter_settings[1].keylist_remoter, 32, "IR_%s", remoter);
    IR_INFO("boot select ir protocol2:: boot_sel_proto = %d, remoter = %s\n",
                    boot_sel_proto, ir_boot_parameter_settings[1].keylist_remoter);
    return 1;
}



int ir_proto_irda_powerup_init(char *options)
{
    unsigned int keycode = 0;
    char *pTmp = NULL;
    if(options == NULL)
        return 1;

    IR_INFO("ir_pro to_irda_powerup_init:: %s\n", options);
    while(NULL != ( pTmp = strsep(&options, ","))) {
        char *endp = NULL;
        keycode = simple_strtoul(pTmp, &endp, 16);
	if(keycode != 0) {
		ir_boot_parameter_settings[0].powerup_keylist[ir_boot_parameter_settings[0].keylist_len] = keycode;
		ir_boot_parameter_settings[0].keylist_len++;
		if(ir_boot_parameter_settings[0].keylist_len >= MAX_POWERUP_KEYLIST_NUM)
			break;
	}

    }
    return 1;
}


int ir_proto_irda_powerup1_init(char *options)
{
    unsigned int keycode = 0;
    char *pTmp = NULL;
    if(options == NULL)
        return 1;

    IR_INFO("ir_pro ir_proto_irda_powerup1_init:: %s\n", options);
    while(NULL != ( pTmp = strsep(&options, ","))) {
        char *endp = NULL;
        keycode = simple_strtoul(pTmp, &endp, 16);
	if(keycode != 0) {
		ir_boot_parameter_settings[1].powerup_keylist[ir_boot_parameter_settings[1].keylist_len] = keycode;
		ir_boot_parameter_settings[1].keylist_len++;
		if(ir_boot_parameter_settings[1].keylist_len >= MAX_POWERUP_KEYLIST_NUM)
			break;
	}

    }
    return 1;
}


int ir_remoter_filter_init(char *options)
{
    g_scancode_filter_params = options;
    IR_INFO("ir_pro ir_remoter_filter_init:: %s\n", options);
    ir_scancode_filter_function_init(&g_scancode_filter_list, NULL);
    ir_scancode_filter_parse_params(&g_scancode_filter_list, g_scancode_filter_params);
    return 1;
}

int ir_remoter_filter_enable_init(char *options)
{
    sscanf(options, "%u", &g_scancode_filter_function_enable);
    IR_INFO("irda_filter_en %u\n", g_scancode_filter_function_enable);
    return 1;
}


int ir_customer_name_init(char *options)
{
    if(options == NULL)
        return 1;
    snprintf(g_ir_customer_name, 32, "%s", options);
    return 1;
}
#ifdef CONFIG_RTK_KDRV_IR
early_param("irda", ir_proto_irda_init);
early_param("irda1", ir_proto_irda1_init);
early_param("irda_powerup", ir_proto_irda_powerup_init);
early_param("irda_powerup1", ir_proto_irda_powerup1_init);
early_param("irda_filter", ir_remoter_filter_init);
early_param("irda_filter_en", ir_remoter_filter_enable_init);
early_param("ir_customer_name", ir_customer_name_init);
#endif

