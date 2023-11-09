/*
 *Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/uaccess.h>        /* copy_*_user */
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#endif
#include <mach/rtk_platform.h>

#include "rtk_pcmcia_reg.h"
#include "rtk_pcmcia.h"

#ifdef CONFIG_CUSTOMER_TV006
#include <rtk_kdriver/rtk_lg_board.h>
#endif
#include <rtk_kdriver/pcbMgr.h>
#include <rtk_otp_region_api.h>

/*#define MAC3_VERIFY*/

#define SA_SHIRQ    IRQF_SHARED
#define PCMCIA_IRQ  IRQ_MISC
#define LOCK_PCMCIA()   mutex_lock(&rtk_pcmcia_lock)
#define UNLOCK_PCMCIA() mutex_unlock(&rtk_pcmcia_lock)

#ifdef RTK_PCMCIA_DIRECT_READ_MIO
#define MIO_READ_SPINLOCK() spin_lock_irqsave(&p_this->lock, irq_flags);
#define MIO_READ_SPINUNLOCK() spin_unlock_irqrestore(&p_this->lock, irq_flags);
#else
#define MIO_READ_SPINLOCK()
#define MIO_READ_SPINUNLOCK()
#endif

static DEFINE_MUTEX(rtk_pcmcia_lock);
static DEFINE_MUTEX(rtk_pcmcia_ioctl_lock);
RTK_PCMCIA *rtk_pcmcia[2] = { NULL, NULL };

static RTK_PCMCIA *rtk_pcmcia_ptr = NULL;
extern char *parse_token(const char *parsed_string, const char *token);
static unsigned long long pcmcia_cfg =
            RTK_PCMCIA_CFG_PCMCIA1_EN | RTK_PCMCIA_CFG_PCMCIA0_EN;
static unsigned int pcmcia_cfg2;
static unsigned int pcmcia_cfg3;
static int bRtkPcmciaResume = 0;
int pcmcia_fifo_mode_enable = 1;
int pcmcia_fifo_write_debug_enable = 0;
int pcmcia_fifo_read_debug_enable = 0;
int pcmcia_poll_event_debug_enable = 0;
RTK_PCMCIA_BOOT_STATUS pcmcia_boot_status = PCMCIA_ON;

#define POLL_EVENT_DBG(fmt, args...)    if (pcmcia_poll_event_debug_enable) { PCMCIA_WARNING(fmt, ##args); }

#define MAX_RX_BURST        (32 * 16)
#define MAX_TX_BURST        (32 * 16)

#if 0
/*old timing parameters*/
static unsigned long pcmcia_amtc0 = 0x00130304;
static unsigned long pcmcia_amtc1 = 0x00110002;
static unsigned long pcmcia_iomtc = 0x08020602;
static unsigned long pcmcia_matc0 = 0x00130002;
static unsigned long pcmcia_matc1 = 0x0013000a;
#elif 1
/*optimal timing parameters */
#define DEF_TAOE            10

#define TSU(x)           ((x) & 0xF)
#define THD(x)           (((x) & 0xF)<<8)
#define TWE(x)           (((x) & 0x3F)<<16)
unsigned long pcmcia_amtc0 = TWE(DEF_TAOE + 2) | THD(8) | TSU(3);
#define TAOE(x)          (((x) & 0x3F)<<16)
#define THCE(x)          ((x) & 0x3F)
unsigned long pcmcia_amtc1 = TAOE(DEF_TAOE) | THCE(2);
unsigned long pcmcia_iomtc = 0x06040502;
#define TC(x)            (((x) & 0x3F)<<16)
#define THDIO(x)         ((x) & 0x3F)
unsigned long pcmcia_matc0 = TC(0x12) | THDIO(2);
#define TCIO(x)          (((x) & 0x1FF)<<16)
#define TWIOWR(x)        ((x) & 0xFF)
unsigned long pcmcia_matc1 = TCIO(0x12) | TWIOWR(8);
#else
/*for saving inpack pin timing parameters*/
static unsigned long pcmcia_amtc0 = 0x000b0303;
static unsigned long pcmcia_amtc1 = 0x00090002;
static unsigned long pcmcia_iomtc = 0x03020102;
static unsigned long pcmcia_matc0 = 0x00040002;
static unsigned long pcmcia_matc1 = 0x00040005;
#endif

static int g_cd_seperate2pins      = 0; /*1: CD need 2 pins , 0: CD need only 1 pin */
RTK_PCMCIA_RESET_PULSE_WIDTH_TYPE g_reset_pulse_width_type = RTK_PCMCIA_RESET_PULSE_WIDTH_2MS; /*default reset pulse with is 2ms*/


static bool gpio_cd_pin_enable = 0;
static int gpio_cd_pin_index = 0;
static RTK_GPIO_GROUP gpio_cd_pin_type = MIO_GPIO;
static unsigned short last_wr_num = 0;

extern void creat_pcmcia_device_node(struct device *dev);
extern void remove_pcmcia_device_node(struct device *dev);

void PCMCIA_ClockEnable(unsigned char On)
{
    if (On) {
        CRT_CLK_OnOff(PCMCIA, CLK_ON, NULL);
    }
    else {
        CRT_CLK_OnOff(PCMCIA, CLK_OFF, NULL);
    }
}

static int rtk_pcmcia_get_boot_parameters(void)
{
    char pcmcia_boot_str[64] = {0};


    switch (pcmcia_boot_status) {
        case PCMCIA_OFF:
            pcmcia_cfg &= (~RTK_PCMCIA_CFG_PCMCIA0_EN);
            sprintf(pcmcia_boot_str, "pcmcia_off");
            break;

        case PCMCIA_ON:
            pcmcia_cfg |= RTK_PCMCIA_CFG_PCMCIA0_EN;
            sprintf(pcmcia_boot_str, "pcmcia_on");
            break;

        default:
            pcmcia_cfg &= (~RTK_PCMCIA_CFG_PCMCIA0_EN);
            PCMCIA_ERROR("%s no set bootcode PMCIA env parameter , default disable pcmcia driver \n", __func__);
            PCMCIA_ERROR("%s no set bootcode PMCIA env parameter , default disable pcmcia driver \n", __func__);
            PCMCIA_ERROR("%s no set bootcode PMCIA env parameter , default disable pcmcia driver \n", __func__);
    }

    PCMCIA_WARNING("pcmcia_bootcode = %s pcmcia_cfg= 0x%llX , pcmcia_cfg3= 0x%X \n", pcmcia_boot_str, pcmcia_cfg, pcmcia_cfg3);
    return PCMCIA_OK;
}

char *get_reset_pulse_width_info(RTK_PCMCIA_RESET_PULSE_WIDTH_TYPE reset_pulse_width_type)
{
    char *reset_pulse_width_info = "2ms";
    switch (reset_pulse_width_type) {
        case RTK_PCMCIA_RESET_PULSE_WIDTH_114US:
            reset_pulse_width_info = "114us";
            break;
        case RTK_PCMCIA_RESET_PULSE_WIDTH_1MS:
            reset_pulse_width_info = "1ms";
            break;
        case RTK_PCMCIA_RESET_PULSE_WIDTH_2MS:
            reset_pulse_width_info = "2ms";
            break;
        case RTK_PCMCIA_RESET_PULSE_WIDTH_20MS:
            reset_pulse_width_info = "20ms";
            break;
        default:
            reset_pulse_width_info = "unknown";
            break;
    };
    return reset_pulse_width_info;
}

unsigned int GET_PCMCIA_STS(void)
{
    int ret = 0;
    unsigned long flags = 0;
    if (PCMCIA_INT_VALID(pcmcia_cfg2)) {
        ret = rtd_inl(PCMCIA_REG_STS);

        spin_lock_irqsave(&rtk_pcmcia_ptr->lock_int, flags);

        /*PCMCIA_INFO("pcmcia_int_status = 0x%08X\n" , rtk_pcmcia_ptr->event ); */

        if (RTK_PCMCIA_EVENT_IREQ_ASSERT & rtk_pcmcia_ptr->event) {
            if (rtk_pcmcia_ptr == rtk_pcmcia[0]) {
                ret |= PC_PII1;
            }
            else {
                ret |= PC_PII2;
            }

            /*rtk_pcmcia_ptr->event &= ~rtk_PCMCIA_EVENT_IREQ_ASSERT; */
        }
        else {
            if (rtk_pcmcia_ptr == rtk_pcmcia[0]) {
                ret &= ~PC_PII1;
            }
            else {
                ret &= ~PC_PII2;
            }
        }

        /*PCMCIA_INFO("pcmcia_int_status = 0x%08X  , ret = 0x%08X  \n" , rtk_pcmcia_ptr->event , ret );*/

        spin_unlock_irqrestore(&rtk_pcmcia_ptr->lock_int, flags);
    }
    else {
        ret = rtd_inl(PCMCIA_REG_STS);
    }

    return ret;
}
unsigned char get_detect_pin_value(void)
{
    if (gpio_cd_pin_enable) {
        return rtk_gpio_input(rtk_gpio_id(MIO_GPIO, gpio_cd_pin_index));
    }
    return 0;
}
static irqreturn_t rtk_pcmcia_isr(int this_irq, void *dev_id)
{
    RTK_PCMCIA *p_this = (RTK_PCMCIA *) dev_id;
    unsigned long event;
    unsigned long flags;
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    unsigned long debounce_event;
#endif

    if ((rtd_inl(PCMCIA_INT) & PCMCIA_INT_MASK) == 0) {
        return IRQ_NONE;
    }

    spin_lock_irqsave(&p_this->lock, flags);

    event = GET_PCMCIA_STS();
    /* Card Detection Interrupt Handler */
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    /*new isr debounce mode*/
    debounce_event = rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB1_reg);
    if (!(debounce_event & p_this->reg_bits.CD_DEB_OUT)
            && ((debounce_event & p_this->reg_bits.CD_DEB_INSERT_FLAG) == p_this->reg_bits.CD_DEB_INSERT_FLAG)) {
        PCMCIA_WARNING("PCMCIA-%d, Card Inserted\n", p_this->id);
        p_this->flags &= ~PCMCIA_CARD_RESET_COMPLETE;
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB1_reg, debounce_event | p_this->reg_bits.CD_DEB_INSERT_FLAG);
        p_this->event |= RTK_PCMCIA_EVENT_CARD_STATUS_CHANGE;
        wake_up(&p_this->wq);
    }
    else if ((debounce_event & p_this->reg_bits.CD_DEB_REMOVE_FLAG) && ((debounce_event & p_this->reg_bits.CD_DEB_OUT) == p_this->reg_bits.CD_DEB_OUT)) {
        PCMCIA_WARNING("PCMCIA-%d, Card Removed, disable card\n", p_this->id);
        rtk_pcmcia_card_enable(p_this, 0);
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB1_reg, debounce_event | p_this->reg_bits.CD_DEB_REMOVE_FLAG);
        p_this->event |= RTK_PCMCIA_EVENT_CARD_STATUS_CHANGE;
        wake_up(&p_this->wq);
    }
#else
    /*original isr debounce mode*/
    if ((event & p_this->reg_bits.CD_INT)
            && (GET_PCMCIA_CTRL() | p_this->reg_bits.CD_INT)) {
        if ((rtk_pcmcia_get_card_status(p_this) &RTK_PCMCIA_STS_CARD_PRESENT)
                && (get_detect_pin_value() == 0)) {

            PCMCIA_WARNING("PCMCIA-%d, Card Inserted\n", p_this->id);
            p_this->flags &= ~PCMCIA_CARD_RESET_COMPLETE;
        }
        else {

            PCMCIA_WARNING("PCMCIA-%d, Card Removed, disable card\n",
                           p_this->id);

            rtk_pcmcia_card_enable(p_this, 0);
        }
        SET_PCMCIA_STS(p_this->reg_bits.CD_INT);
        p_this->event |= RTK_PCMCIA_EVENT_CARD_STATUS_CHANGE;
        wake_up(&p_this->wq);
    }
#endif /*RTK_PCMCIA_CD_DEBOUNCE*/
    if ((event & p_this->reg_bits.PII) && (GET_PCMCIA_CTRL() |
                                           p_this->reg_bits.PII)) {

        POLL_EVENT_DBG("PCMCIA-%d, IREQ# detected\n", p_this->id);
        SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() & ~(p_this->reg_bits.PII));   /* disable PII interrupt*/
        SET_PCMCIA_STS(p_this->reg_bits.PII);   /* clear PII */
        p_this->event |= RTK_PCMCIA_EVENT_IREQ_ASSERT;
        POLL_EVENT_DBG("PCMCIA-%d, Status = %08X / event=%08X\n", p_this->id, GET_PCMCIA_STS(), p_this->event);
        wake_up(&p_this->wq);
    }

    /* Assess Interrupt Handler */
    if ((event & PC_ACCESS_INT) && (p_this->flags & PCMCIA_TX_ON)) {
        SET_PCMCIA_STS(PC_ACCESS_INT);
        PCMCIA_DBG("PCMCIA-%d, tx complete\n", p_this->id);
        p_this->tx_status = (event & PC_ACCESS_INT);
        p_this->flags &= ~PCMCIA_TX_ON;
        wake_up(&p_this->wq);
    }

#if 0
    rtd_outl(PCMCIA_INT, PCMCIA_INT_EN_MASK);

#endif  /*
 */
    spin_unlock_irqrestore(&p_this->lock, flags);
    return IRQ_HANDLED;
}

static void rtk_pcmcia_gpio_int_isr(RTK_GPIO_ID gid, unsigned char assert,
                                    void *dev_id)
{
    unsigned long flags;

    /*
        PCMCIA_DBG("%s GPIO %d assert=%d, val=%d\n",
                gpio_type(gpio_group(gid)), gpio_idx(gid),
                assert, rtk_gpio_input(gid));
    */


    spin_lock_irqsave(&rtk_pcmcia_ptr->lock, flags);

    /*  PCMCIA_DBG("gpio interrupt detected\n");*/

    rtk_pcmcia_ptr->event |= RTK_PCMCIA_EVENT_IREQ_ASSERT;

    /*PCMCIA_INFO("pcmcia_int_status = 0x%08X\n" , rtk_pcmcia_ptr->event );*/

    spin_unlock_irqrestore(&rtk_pcmcia_ptr->lock, flags);
}

void rtk_pcmcia_phy_reset(void)
{
    int try_cnt = 0;
    PCMCIA_DBG("rtk_pcmcia_phy_reset\n");
    SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() | PC_PSR);
    PCMCIA_DBG("wait PSR done\n");
    while (GET_PCMCIA_CTRL() & PC_PSR && try_cnt++ < 100) {
        PCMCIA_DBG("wait PSR ready : %lx=%lx\n", PCMCIA_REG_CTRL,
                   GET_PCMCIA_CTRL());
        udelay(1);
    }
    PCMCIA_DBG("PSR done\n");

    if (GET_PCMCIA_CTRL() & PC_PSR) {
        PCMCIA_DBG("Wait PSR timeout\n");
        PCMCIA_DBG("%lx=%lx\n", PCMCIA_CLOCK_ENABLE, rtd_inl(PCMCIA_CLOCK_ENABLE));
        PCMCIA_DBG("%lx=%lx\n", PCMCIA_SOFT_RESET, rtd_inl(PCMCIA_SOFT_RESET));
    }
}

#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)

#include "mach/pcbMgr.h"
static void rtk_pcmcia_gpio_config(void)
{
    unsigned long long param;
    if (pcb_mgr_get_enum_info_byname("PCMCIA_CFG", &param) == 0) {
        pcmcia_cfg = param & 0xFFFFFFFFFFFFFFFF;
    }
    else {
        /*Card Detect PAD */
        if (pcb_mgr_get_enum_info_byname("M_PCM_CD_N", &param) == 0) {
            pcmcia_cfg |= PCMCIA_CD_SET(GET_PIN_INDEX(param));
            PCMCIA_WARNING("Card dtection is GPIO%d\n", GET_PIN_INDEX(param));
        }
        else {
            pcmcia_cfg |= PCMCIA_CD_SET(61);
            PCMCIA_WARNING("Can't find M_PCM_CD_N config, use default as card detect.\n");
        }
        /*Power Control GPIO */
        if (pcb_mgr_get_enum_info_byname("PIN_PCMCIA_EN", &param) == 0) {
            if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_ISO_GPIO
                    || GET_PIN_TYPE(param) == PCB_PIN_TYPE_GPIO) {

                pcmcia_cfg |= RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_VALID;

                if (GET_PIN_TYPE(param) ==
                        PCB_PIN_TYPE_ISO_GPIO) {
                    pcmcia_cfg |= RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_GPIO_TYPE;
                }

                pcmcia_cfg |= GET_PIN_INDEX(param);
                if (!GET_PIN_PARAM2(param)) { /*inverted */
                    pcmcia_cfg |= RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_POAL;
                }
            }
        }
        else {
            PCMCIA_WARNING
            ("Can't find PIN_PCMCIA_EN config, don't use GPIO to control power.\n");
        }

        /*demo baord setting */
        /*pcmcia_cfg |= ( RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_VALID | */
        /*                                     RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_POAL |(15)); */
#ifdef MAC3_VERIFY
        pcmcia_cfg = pcmcia_cfg & (~RTK_PCMCIA_CFG_PCMCIA1_EN);
        pcmcia_cfg |= (RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_VALID |
                       RTK_PCMCIA_CFG_PCMCIA0_EN |
                       RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_POAL |
                       RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_GPIO_TYPE | (11));

#endif /*
 */
    }
    PCMCIA_WARNING("pcmcia_cfg = %llx\n", (unsigned long long)pcmcia_cfg);
}
static void rtk_pcmcia_gpio_config2(void)
{
    unsigned long long param2;

    if (pcb_mgr_get_enum_info_byname("PCMCIA_CFG2", &param2) == 0) {
        pcmcia_cfg2 = param2 & 0xFFFFFFFF;
        PCMCIA_WARNING("find PCMCIA_CFG2 config.\n");
    }
    else {
        PCMCIA_WARNING("Can't find PCMCIA_CFG2 config.\n");

        /*GPIO INT pin*/
        if (pcb_mgr_get_enum_info_byname("PIN_PCMCIA_INT", &param2) ==  0) {

            PCMCIA_WARNING("PIN_PCMCIA_INT = 0x%llx.\n", param2);

            if (GET_PIN_TYPE(param2) == PCB_PIN_TYPE_ISO_GPIO ||
                    GET_PIN_TYPE(param2) == PCB_PIN_TYPE_GPIO) {
                pcmcia_cfg2 |= RTK_PCMCIA_CFG_PCMCIA_INT_VALID;

                if (GET_PIN_TYPE(param2) ==
                        PCB_PIN_TYPE_ISO_GPIO)
                    pcmcia_cfg2 |=
                                RTK_PCMCIA_CFG_PCMCIA_INT_GPIO_TYPE;

                pcmcia_cfg2 |= GET_PIN_INDEX(param2);

                if (!GET_PIN_PARAM2(param2))    /*inverted*/
                    pcmcia_cfg2 |=
                                RTK_PCMCIA_CFG_PCMCIA_INT_POAL;
            }
        }
        else {
            PCMCIA_WARNING("Can't find PIN_PCMCIA_INT config, pcmcia operation will be abnormal.\n");
        }

        /*demo baord setting*/
        /* pcmcia_cfg2 |= ( rtk_PCMCIA_CFG_PCMCIA_INT_VALID |
                    rtk_PCMCIA_CFG_PCMCIA_INT_POAL |(14));
        */
    }

    PCMCIA_WARNING("pcmcia_cfg2 = 0x%x\n", pcmcia_cfg2);
}

static void rtk_pcmcia_gpio_config3(void)
{
    unsigned long long param3;

    if (pcb_mgr_get_enum_info_byname("PCMCIA_CFG3", &param3) == 0) {
        pcmcia_cfg3 = param3 & 0xFFFFFFFF;
        PCMCIA_WARNING("find PCMCIA_CFG3 config.\n");

        gpio_cd_pin_enable = PCMCIA_GPI_CD1_ENABLE(pcmcia_cfg3);
        if (gpio_cd_pin_enable) {
            gpio_cd_pin_type = PCMCIA_GPI_CD1_TYPE(pcmcia_cfg3);
            gpio_cd_pin_index = PCMCIA_GPI_PIN_CD1_INDEX(pcmcia_cfg3);
            rtk_gpio_set_dir(rtk_gpio_id(MIO_GPIO, gpio_cd_pin_index), 0);
        }
    }
    else {
        PCMCIA_WARNING("Can't find PCMCIA_CFG3 config.\n");
    }


    PCMCIA_WARNING("pcmcia_cfg3 = 0x%x\n", pcmcia_cfg3);
}

#else /*
 */
static void rtk_pcmcia_gpio_config(void)
{
    pcmcia_cfg |= (RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_VALID |
                   RTK_PCMCIA_CFG_PCMCIA0_PWR_EN_POAL | (0xF));
}

static void rtk_pcmcia_gpio_config2(void)
{
    pcmcia_cfg2 = 0;
}

static void rtk_pcmcia_gpio_config3(void)
{
    pcmcia_cfg3 = 0;
}

#endif /*
 */

/*-----------------------------------------------------------------------------------
 * low level PCMCIA device driver
 *------------------------------------------------------------------------------------*/
RTK_PCMCIA *create_rtk_pcmcia(unsigned char id)
{
    unsigned int reset_pulse_width_cfg = 0;
    RTK_PCMCIA *p_this = NULL;
    if (id >= MAX_PCMCIA_ENTRY) {
        return NULL;
    }

    g_cd_seperate2pins  = PCMCIA_CD_SEPERATE_2PIN(pcmcia_cfg3);

    p_this = (RTK_PCMCIA *) kmalloc(sizeof(RTK_PCMCIA), GFP_KERNEL);
    if (p_this) {
        memset(p_this, 0, sizeof(RTK_PCMCIA));
        p_this->id = id;
        init_waitqueue_head(&p_this->wq);
        spin_lock_init(&p_this->lock);
        spin_lock_init(&p_this->lock_int);

        atomic_set(&(p_this->atomic_init), 0);

        switch (id) {
            case 0:
                p_this->reg_bits.CE = PCMCIA_CE_SWAP(pcmcia_cfg) ? PC_CE1_CARD2 : PC_CE1_CARD1;
                p_this->reg_bits.PCR_OE = PC_PCR1_OE;
                p_this->reg_bits.PRES = (g_cd_seperate2pins) ? (PC_PRES1 | PC_PRES2) : (PC_PRES1);
                p_this->reg_bits.PII = PC_PII1;
                p_this->reg_bits.PCR = PC_PCR1;
                p_this->reg_bits.CD_INT = PC_CARD_DETECT1_INT;
#ifdef RTK_PCMCIA_CD_DEBOUNCE
                p_this->reg_bits.CD_DEB_EN = (g_cd_seperate2pins) ? (PC_CD0_DEB_EN | PC_CD1_DEB_EN) : (PC_CD0_DEB_EN);
                p_this->reg_bits.CD_DEB_INSERT_ISREN = (g_cd_seperate2pins) ? (PC_CD0_DEB_INSERT_ISREN | PC_CD1_DEB_INSERT_ISREN) : (PC_CD0_DEB_INSERT_ISREN);
                p_this->reg_bits.CD_DEB_REMOVE_ISREN = (g_cd_seperate2pins) ? (PC_CD0_DEB_REMOVE_ISREN | PC_CD1_DEB_REMOVE_ISREN) : (PC_CD0_DEB_REMOVE_ISREN);
                p_this->reg_bits.CD_DEB_OUT = (g_cd_seperate2pins) ? (PC_CD0_DEB_OUT | PC_CD1_DEB_OUT) : (PC_CD0_DEB_OUT);
                p_this->reg_bits.CD_DEB_INSERT_FLAG = (g_cd_seperate2pins) ? (PC_CD0_DEB_INSERT_FLAG | PC_CD1_DEB_INSERT_FLAG) : (PC_CD0_DEB_INSERT_FLAG);
                p_this->reg_bits.CD_DEB_REMOVE_FLAG = (g_cd_seperate2pins) ? (PC_CD0_DEB_REMOVE_FLAG | PC_CD1_DEB_REMOVE_FLAG) : (PC_CD0_DEB_REMOVE_FLAG);
#endif

                if (PCMCIA0_PWR_EN_VALID(pcmcia_cfg)) {
                    p_this->flags |= PCMCIA_WITH_POWER_CTRL_GPIO;
                    p_this->pwr_en_pin = rtk_gpio_id(MIO_GPIO,
                                                     PCMCIA0_PWR_EN_GPIO(pcmcia_cfg));
                }
                rtd_outl(PCMCIA_REG_PIN_SEL, (rtd_inl(PCMCIA_REG_PIN_SEL)) | PC_INPACK0_DIS(1) | PC_WAIT0_DIS(0));
                break;
            case 1:
                p_this->reg_bits.CE = PCMCIA_CE_SWAP(pcmcia_cfg) ? PC_CE1_CARD1 : PC_CE1_CARD2;
                p_this->reg_bits.PCR_OE = PC_PCR2_OE;
                p_this->reg_bits.PRES = (g_cd_seperate2pins) ? (PC_PRES1 | PC_PRES2) : (PC_PRES2);
                p_this->reg_bits.PII = PC_PII2;
                p_this->reg_bits.PCR = PC_PCR2;
                p_this->reg_bits.CD_INT = PC_CARD_DETECT2_INT;
#ifdef RTK_PCMCIA_CD_DEBOUNCE
                p_this->reg_bits.CD_DEB_EN = (g_cd_seperate2pins) ? (PC_CD0_DEB_EN | PC_CD1_DEB_EN) : (PC_CD1_DEB_EN);
                p_this->reg_bits.CD_DEB_INSERT_ISREN = (g_cd_seperate2pins) ? (PC_CD0_DEB_INSERT_ISREN | PC_CD1_DEB_INSERT_ISREN) : (PC_CD1_DEB_INSERT_ISREN);
                p_this->reg_bits.CD_DEB_REMOVE_ISREN = (g_cd_seperate2pins) ? (PC_CD0_DEB_REMOVE_ISREN | PC_CD1_DEB_REMOVE_ISREN) : (PC_CD1_DEB_REMOVE_ISREN);
                p_this->reg_bits.CD_DEB_OUT = (g_cd_seperate2pins) ? (PC_CD0_DEB_OUT | PC_CD1_DEB_OUT) : (PC_CD1_DEB_OUT);
                p_this->reg_bits.CD_DEB_INSERT_FLAG = (g_cd_seperate2pins) ? (PC_CD0_DEB_INSERT_FLAG | PC_CD1_DEB_INSERT_FLAG) : (PC_CD1_DEB_INSERT_FLAG);
                p_this->reg_bits.CD_DEB_REMOVE_FLAG = (g_cd_seperate2pins) ? (PC_CD0_DEB_REMOVE_FLAG | PC_CD1_DEB_REMOVE_FLAG) : (PC_CD1_DEB_REMOVE_FLAG);
#endif

                if (PCMCIA1_PWR_EN_VALID(pcmcia_cfg)) {

                    p_this->flags |= PCMCIA_WITH_POWER_CTRL_GPIO;
                    p_this->pwr_en_pin = rtk_gpio_id(MIO_GPIO,
                                                     PCMCIA1_PWR_EN_GPIO(pcmcia_cfg));
                }

                rtd_outl(PCMCIA_REG_PIN_SEL, (rtd_inl(PCMCIA_REG_PIN_SEL)) | PC_INPACK1_DIS(1) | PC_WAIT1_DIS(0));

                break;
        }
        if (p_this->flags & PCMCIA_WITH_POWER_CTRL_GPIO) {

            PCMCIA_WARNING("PCMCIA-%d : Power pin : %s_GPIO %d, CE=%lu\n",
                           p_this->id,
                           gpio_type(gpio_group(p_this->pwr_en_pin)),
                           gpio_idx(p_this->pwr_en_pin),
                           p_this->reg_bits.CE);

        }
        else {
            PCMCIA_WARNING("PCMCIA-%d : N/A\n", p_this->id);
        }

        p_this->skipTime = 1800;

        PCMCIA_WARNING("PCMCIA-%d : skipTime: %d\n", p_this->id, p_this->skipTime);


        reset_pulse_width_cfg = PCMCIA_GET_RESET_PLUSE_WIDTH_TYPE(pcmcia_cfg3);
        if ((reset_pulse_width_cfg > 0) && (reset_pulse_width_cfg <= (RTK_PCMCIA_RESET_PULSE_WIDTH_20MS + 1))) {
            g_reset_pulse_width_type = reset_pulse_width_cfg - 1;
        }

        PCMCIA_WARNING("PCMCIA-%d : set reset pulse width to %s \n",
                       p_this->id, get_reset_pulse_width_info(g_reset_pulse_width_type));

        PCMCIA_WARNING("PCMCIA-%d : Card detect using %d pins \n",
                       p_this->id, g_cd_seperate2pins ? (2) : (1));

        return p_this;
    }

    return NULL;
}

void release_rtk_pcmcia(RTK_PCMCIA *p_this)
{
    kfree(p_this);
}
void rtk_pcmcia_int_gpio_set(void)
{

    PCMCIA_WARNING("pcmcia_cfg2 = 0x%08X\n", pcmcia_cfg2);

    if (PCMCIA_INT_VALID(pcmcia_cfg2)) {
        /* gpio interrupt pin */
        rtk_pcmcia_ptr->int_pin =
                    rtk_gpio_id(MIO_GPIO,
                                PCMCIA_INT_GPIO(pcmcia_cfg2));

        rtk_gpio_set_dir(rtk_pcmcia_ptr->int_pin, 0);

        /* 100 us*/
        rtk_gpio_set_debounce(rtk_pcmcia_ptr->int_pin,
                              RTK_GPIO_DEBOUNCE_100us);

        rtk_gpio_set_irq_polarity(rtk_pcmcia_ptr->int_pin, 0);

        rtk_gpio_request_irq(rtk_pcmcia_ptr->int_pin,
                             rtk_pcmcia_gpio_int_isr, "PCMCIA_GPIO_INT",
                             rtk_pcmcia_gpio_int_isr);

        rtk_gpio_set_irq_enable(rtk_pcmcia_ptr->int_pin, 1);

        PCMCIA_WARNING("rtk_pcmcia_gpio_int for %s GPIO %d (%x) ready\n",
                       gpio_type(gpio_group(rtk_pcmcia_ptr->int_pin)),
                       gpio_idx(rtk_pcmcia_ptr->int_pin),
                       rtk_pcmcia_ptr->int_pin);
    }

}

int rtk_pcmcia_get_card_status(RTK_PCMCIA *p_this)
{
    int status = 0;
    unsigned long event = GET_PCMCIA_STS();

    if ((event & p_this->reg_bits.PRES) == p_this->reg_bits.PRES) {
        status |= RTK_PCMCIA_STS_CARD_PRESENT;
    }

    if (event & PC_AFI) {
        status |= RTK_PCMCIA_STS_ACCESS_FAULT;
    }

    if (event & p_this->reg_bits.PII) {
        status |= RTK_PCMCIA_STS_IREQ_ASSERT;
    }

    return status;
}

int rtk_pcmcia_get_card_status_ext(void)
{
    int status = 0;
    unsigned long PRES = (g_cd_seperate2pins) ? (PC_PRES1 | PC_PRES2) : (PC_PRES1);

    if ((GET_PCMCIA_STS() & PRES) == PRES) {
        status = 1;
    }
    return status;
}
EXPORT_SYMBOL(rtk_pcmcia_get_card_status_ext);

//just for mio read
static int __rtk_pcmcia_read_rx_fifo(unsigned char *data, unsigned short fifo_len)
{
#ifdef RTK_PCMCIA1_REG
    pcmcia1_mis_pcmcia_dff_RBUS data_fifo_reg;
#else
    pcmcia_mis_pcmcia_dff_RBUS data_fifo_reg;
#endif
    unsigned int read_mode = 0;

#ifdef RTK_PCMCIA_DIRECT_READ_MIO
    read_mode = PCMCIA_READ_MODE;
#endif

    if (fifo_len >= 4) {
        SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1 | PCMCIA_DF2 | PCMCIA_DF3 | read_mode);
    }

    while (fifo_len >= 4) {
        data_fifo_reg.regValue = GET_PCMCIA_MIO_DATA_FIFO();
        *data = data_fifo_reg.df0;
        *(data + 1) = data_fifo_reg.df1;
        *(data + 2) = data_fifo_reg.df2;
        *(data + 3) = data_fifo_reg.df3;
        data += 4;
        fifo_len -= 4;
    }
    switch (fifo_len) {
        case 3:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1 | PCMCIA_DF2 | read_mode);
            data_fifo_reg.regValue = GET_PCMCIA_MIO_DATA_FIFO();
            *data = data_fifo_reg.df0;
            *(data + 1) = data_fifo_reg.df1;
            *(data + 2) = data_fifo_reg.df2;
            data += 3;
            break;
        case 2:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1 | read_mode);
            data_fifo_reg.regValue = GET_PCMCIA_MIO_DATA_FIFO();
            *data = data_fifo_reg.df0;
            *(data + 1) = data_fifo_reg.df1;
            data += 2;
            break;
        case 1:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | read_mode);
            data_fifo_reg.regValue = GET_PCMCIA_MIO_DATA_FIFO();
            *data = data_fifo_reg.df0;
            data += 1;
            break;
        case 0:
            break;
    }
    return PCMCIA_OK;
}

int _rtk_pcmcia_fifo_read(RTK_PCMCIA *p_this, unsigned short addr,
                          unsigned char *p_data, unsigned short len,
                          unsigned long flags)
{
    int ret = PCMCIA_OK;
    unsigned long fifo_ctrl = 0;
    unsigned long ctrl;
    unsigned char attr = (flags & PCMCIA_FLAGS_ATTR) ? 1 : 0;
    unsigned long cmd = PC_CT_READ | PC_AT(attr) | PC_PA(addr);
    unsigned long try_count = 0;
    unsigned long event = 0;
    unsigned long irq_flags;
    static unsigned int rx_round = 0;
    unsigned int sub_round = 0;

    if (len == 0) {
        return ret;
    }

#ifdef RTK_PCMCIA_DIRECT_READ_MIO
    SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_READ_MODE);
#endif

    if (!(GET_PCMCIA_STS() & p_this->reg_bits.PRES)) {
        PCMCIA_WARNING("access pcmcia failed - no card exists!!!\n");
        return PCMCIA_NODEV;
    }

    if (!(GET_PCMCIA_CTRL() & p_this->reg_bits.PCR_OE)) {
        PCMCIA_WARNING
        ("access pcmcia failed - please enable port first!!!\n");
        return PCMCIA_FAIL;
    }

    if (pcmcia_fifo_read_debug_enable) {
        rx_round++;
    }

    preempt_disable();

    spin_lock_irqsave(&p_this->lock, irq_flags);

    ctrl = GET_PCMCIA_CTRL() & ~(PC_ACCESS_INT | PC_FIFO_INT);
    ctrl |= p_this->reg_bits.CE;
    SET_PCMCIA_CTRL(ctrl);
    SET_PCMCIA_STS(PC_ACCESS_INT | PC_FIFO_INT);


    // fifo control setting
    if (attr) {
        fifo_ctrl |= PC_ADDR_STEP(1) | PC_WR_NUM(len) | PC_FIFO_R_TRIG(1);
    }
    else {
        fifo_ctrl |= PC_ADDR_STEP(0) | PC_WR_NUM(len) | PC_FIFO_R_TRIG(1);

        rtd_outl(PCMCIA_REG_PIN_SEL, (rtd_inl(PCMCIA_REG_PIN_SEL))  | PC_INPACK0_DIS(1) |  PC_WAIT0_DIS(0));
    }

    if ((flags & PCMCIA_FLAGS_FIFO) || len == 1) {
        fifo_ctrl |= PC_FIFO_WR_MODE_ADDRESS_NOCHANGE;    // FIFO mode, do not increase register address
    }
    else {
        fifo_ctrl |= PC_FIFO_WR_MODE_ADDRESS_INCREASE;
    }

    SET_PCMCIA_FIFO_CTRL(fifo_ctrl | PC_FIFO_EN(1) | PC_PIN_SHARE_SEL(1)); //mio read/write need set PC_PIN_SHARE_SEL(1)
    spin_unlock_irqrestore(&p_this->lock, irq_flags);
    ndelay(10);
#ifndef RTK_PCMCIA_DIRECT_READ_MIO
    spin_lock_irqsave(&p_this->lock, irq_flags);
#endif
    SET_PCMCIA_CMDFF(cmd);      // kick off fifo read
    while (ret == PCMCIA_OK) {
        try_count = 3000;       // around polling time = 3000 * 300ns = 1 ms
        event = GET_PCMCIA_STS();

        //-------------- wait fifo read request interrupt --------------
        while (!(event & PC_FIFO_RX_INT) && (try_count--)) {
            ndelay(300);
            event = GET_PCMCIA_STS();
        }

        if (pcmcia_fifo_read_debug_enable) {
            PCMCIA_WARNING("_rtk_pcmcia_fifo_read(%d-%d) : event=%08lx (%08lx) \n",
                           rx_round, sub_round,
                           event, (event & (PC_FIFO_R_IE | PC_FIFO_WR_OVER_IE)));
            sub_round++;
        }
        if ((event & PC_FIFO_RX_INT) == 0) {
            PCMCIA_ERROR("%s access pcmcia failed - wait rx fifo interrupt timeout,event=%x STS = 0x%X, FIFO_CTRL=%08X, FIFO_STATUS=%08X!!!\n",
                         __FUNCTION__, event, GET_PCMCIA_STS(), GET_PCMCIA_FIFO_CTRL(), GET_PCMCIA_FIFO_STATUS());
            ret = PCMCIA_FAIL;
            goto end_proc;
        }

        //-------------- handle interrupts ----------------------
        if ((event & PC_FIFO_ERRO_IE)) {
            PCMCIA_ERROR("%s access error , PC_FIFO_ERRO_IE , event = 0x%08lX !!! \n", __FUNCTION__, event);
            SET_PCMCIA_STS(PC_FIFO_ERRO_IE);
            ret = PCMCIA_FAIL;
            goto end_proc;
        }

        if ((event & PC_FIFO_R_IE)) {
            unsigned short fifo_len;

            MIO_READ_SPINLOCK();
#ifdef RTK_PCMCIA_DIRECT_READ_MIO
            fifo_len = PC_FIFO_LEN(GET_PCMCIA_FIFO_STATUS());
#else
            fifo_len = 16;
#endif
            SET_PCMCIA_STS(PC_FIFO_R_IE);       // trigger to start next read

#ifdef RTK_PCMCIA_DIRECT_READ_MIO
            if (fifo_len >= 16) {
                fifo_len =  16;
            }
#endif
            if (len > 0) {
                __rtk_pcmcia_read_rx_fifo(p_data, fifo_len);  //just for mio read
                len -= fifo_len;
                p_data += fifo_len;
            }

            MIO_READ_SPINUNLOCK();
        }

        if ((event & PC_FIFO_WR_OVER_IE)) {
            unsigned short fifo_len;

            MIO_READ_SPINLOCK();

            SET_PCMCIA_STS(PC_FIFO_WR_OVER_IE);

            fifo_len = PC_FIFO_LEN(GET_PCMCIA_FIFO_STATUS());

            if (len > 0) {
                __rtk_pcmcia_read_rx_fifo(p_data, fifo_len);  //just for mio read
                len -= fifo_len;
                p_data += fifo_len;
            }
            MIO_READ_SPINUNLOCK();

            if (len != 0) {
                PCMCIA_ERROR("%s access error , remain %d bytes unreaded !!! \n", __FUNCTION__, len);
                ret = PCMCIA_FAIL;
            }
            break;
        }
    }

end_proc:
#ifndef RTK_PCMCIA_DIRECT_READ_MIO
    spin_unlock_irqrestore(&p_this->lock, irq_flags);
#endif
    preempt_enable();
    return ret;
}

int rtk_pcmcia_fifo_read(RTK_PCMCIA *p_this, unsigned short addr,
                         unsigned char *p_data, unsigned short len,
                         unsigned long flags)
{
    int ret = PCMCIA_OK;
    int rx_len;
    int step;

    step = (flags & PCMCIA_FLAGS_FIFO) ? 0 : ((flags & PCMCIA_FLAGS_ATTR)) ? 2 : 1;

    if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
        PCMCIA_WARNING("access pcmcia failed - no card exists!!!\n");
        return PCMCIA_NODEV;
    }

    if (!(GET_PCMCIA_CTRL() & p_this->reg_bits.PCR_OE)) {
        PCMCIA_WARNING
        ("access pcmcia failed - please enable port first!!!\n");
        return PCMCIA_FAIL;
    }
    if (pcmcia_fifo_read_debug_enable) {
        PCMCIA_WARNING("rd %s %s, addr = %d, len=%d\n", (flags & PCMCIA_FLAGS_ATTR) ? "attr" : "io",
                       (flags & PCMCIA_FLAGS_FIFO) ? "port" : "mem", addr, len);
    }

    LOCK_PCMCIA();
    //--- do fifo read
    while (len) {
        rtk_pcmcia_phy_reset();
        rx_len = (len > MAX_RX_BURST) ? MAX_RX_BURST : len;
        ret = _rtk_pcmcia_fifo_read(p_this, addr, p_data, rx_len, flags);

        if (ret != PCMCIA_OK) {
            UNLOCK_PCMCIA();
            PCMCIA_WARNING("%s read failed\n", __FUNCTION__);
            return ret;
        }

        p_data += rx_len;
        len   -= rx_len;
        addr  += rx_len * step;
    }

    UNLOCK_PCMCIA();

    return ret;
}


int __rtk_pcmcia_write_tx_fifo(unsigned char *p_data, unsigned short len)
{
    int tx_cnt = 0;
#ifdef RTK_PCMCIA1_REG
    pcmcia1_mis_pcmcia_dff_RBUS data_fifo_reg;
#else
    pcmcia_mis_pcmcia_dff_RBUS data_fifo_reg;
#endif

    if (len >= 4) {
        SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1 | PCMCIA_DF2 | PCMCIA_DF3);
    }

    while (len >= 4) {
        data_fifo_reg.regValue = 0;
        data_fifo_reg.df0 = *p_data;
        data_fifo_reg.df1 = *(p_data + 1);
        data_fifo_reg.df2 = *(p_data + 2);
        data_fifo_reg.df3 = *(p_data + 3);
        SET_PCMCIA_MIO_DATA_FIFO(data_fifo_reg.regValue);
        p_data += 4;
        len -= 4;
        tx_cnt += 4;
    }

    switch (len) {
        case 3:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1 | PCMCIA_DF2);
            data_fifo_reg.regValue = 0;
            data_fifo_reg.df0 = *p_data;
            data_fifo_reg.df1 = *(p_data + 1);
            data_fifo_reg.df2 = *(p_data + 2);
            SET_PCMCIA_MIO_DATA_FIFO(data_fifo_reg.regValue);
            p_data += 3;
            tx_cnt += 3;
            break;
        case 2:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0 | PCMCIA_DF1);
            data_fifo_reg.regValue = 0;
            data_fifo_reg.df0 = *p_data;
            data_fifo_reg.df1 = *(p_data + 1);
            SET_PCMCIA_MIO_DATA_FIFO(data_fifo_reg.regValue);
            p_data += 2;
            tx_cnt += 2;
            break;
        case 1:
            SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0);
            data_fifo_reg.regValue = 0;
            data_fifo_reg.df0 = *p_data;
            SET_PCMCIA_MIO_DATA_FIFO(data_fifo_reg.regValue);
            p_data += 1;
            tx_cnt += 1;
            break;
        case 0:
        default:
            break;
    }
    return tx_cnt;
}


/*----------------------------------------------------------
 * Func : _rtk_pcmcia_fifo_write
 *
 * Desc : lowlevel fifo write function.
 *
 * Para :
 *
 * Retn : >=0 : number of bytes written,
 *        <0  : failed
 *----------------------------------------------------------*/
int _rtk_pcmcia_fifo_write(RTK_PCMCIA *p_this, unsigned short addr,
                           unsigned char *p_data, unsigned short len,
                           unsigned long flags)
{
    int ret = PCMCIA_OK;
    int tx_bytes = 0;
    unsigned long fifo_ctrl = 0;
    unsigned char attr = (flags & PCMCIA_FLAGS_ATTR) ? 1 : 0;
    unsigned long ctrl;
    unsigned long cmd = PC_CT_WRITE | PC_AT(attr) | PC_PA(addr);
    unsigned long try_count = 3000;    // 3000 * 300ns = 1ms
    unsigned long event;
    static unsigned int tx_round = 0;
    unsigned int sub_round = 0;
    unsigned long irq_flags;
    unsigned int try_sts = 0;
    if (pcmcia_fifo_write_debug_enable)
        PCMCIA_WARNING("wr %s %s, addr = %d, len=%d\n", (flags & PCMCIA_FLAGS_ATTR) ? "attr" : "io",
                       (flags & PCMCIA_FLAGS_FIFO) ? "port" : "mem", addr, len);

    if (p_data == NULL || len <= 0) {
        return PCMCIA_FAIL;
    }

    if (len > MAX_TX_BURST) {
        len = MAX_TX_BURST;
    }

    tx_round++;

    if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
        PCMCIA_WARNING("access pcmcia failed - no card exists!!!\n");
        return PCMCIA_NODEV;
    }

    if (!(GET_PCMCIA_CTRL() & p_this->reg_bits.PCR_OE)) {
        PCMCIA_WARNING
        ("access pcmcia failed - please enable port first!!!\n");
        return PCMCIA_FAIL;
    }

    LOCK_PCMCIA();

    preempt_disable();
    //---------- Initial fifo control register -------------
    spin_lock_irqsave(&p_this->lock, irq_flags);

    if (attr) {
        fifo_ctrl = PC_ADDR_STEP(1);    // address step = 2
    }
    else {
        fifo_ctrl = PC_ADDR_STEP(0);   // address step = 1
        rtd_outl(PCMCIA_REG_PIN_SEL, (rtd_inl(PCMCIA_REG_PIN_SEL)) | PC_INPACK0_DIS(1) | PC_WAIT0_DIS(0));
    }

    fifo_ctrl |= PC_WR_NUM(len) | PC_FIFO_R_TRIG(0);

    if ((flags & PCMCIA_FLAGS_FIFO) || len == 1) {
        fifo_ctrl |= PC_FIFO_WR_MODE_ADDRESS_NOCHANGE;    // FIFO mode, do not increase register address
    }
    else {
        fifo_ctrl |= PC_FIFO_WR_MODE_ADDRESS_INCREASE;
    }

    SET_PCMCIA_FIFO_CTRL(fifo_ctrl | PC_FIFO_EN(1) | PC_PIN_SHARE_SEL(1)); //mio read/write need set PC_PIN_SHARE_SEL(1)


    ctrl  = GET_PCMCIA_CTRL() & ~(PC_ACCESS_INT | PC_FIFO_INT);
    ctrl |= p_this->reg_bits.CE;

    SET_PCMCIA_STS(PC_ACCESS_INT | PC_FIFO_INT);

    SET_PCMCIA_CTRL(ctrl);

    spin_unlock_irqrestore(&p_this->lock, irq_flags);

    ndelay(10);

    SET_PCMCIA_CMDFF(cmd);      // enable pcmcia write;

    /*mio_pcmcia workaround*/
    if (last_wr_num >= 32 && !(last_wr_num % 32) && len % 32) {
        while (!(GET_PCMCIA_STS() & PC_FIFO_W_IE) && try_sts++ < 3) {
            if (pcmcia_fifo_write_debug_enable) {
                PCMCIA_WARNING(" Wait wrong fifo_w flag :STS=%lx last_wr_num=%d  len=%d\n", GET_PCMCIA_STS(), last_wr_num, len);
            }
            ndelay(5);
        }
    }

    last_wr_num = len;
    /*********************/

    while (ret == PCMCIA_OK) {
        //---------- load fifo data if necessary -------------
        if (len) {
            if (pcmcia_fifo_write_debug_enable) {
                PCMCIA_WARNING("going to write data(%d-%d) : STS = 0x%X, FIFO_CTRL=%08X, FIFO_STATUS=%08X, tx_bytes=%d!!!\n",
                               tx_round, sub_round,
                               GET_PCMCIA_STS(), GET_PCMCIA_FIFO_CTRL(), GET_PCMCIA_FIFO_STATUS(), tx_bytes);
            }
            if (tx_bytes == 0 || (GET_PCMCIA_STS() & PC_FIFO_W_IE)) { // 1st write or FIFO_W enabled
                int tx_len = (len > 32) ? 32 : len;

                spin_lock_irqsave(&p_this->lock, irq_flags);  // lock spinlock to make sure the following register access
                // won't be affected by IRQ#
                if ((GET_PCMCIA_STS() & PC_FIFO_W_IE)) {
                    SET_PCMCIA_STS(PC_FIFO_W_IE);
                }

                if (__rtk_pcmcia_write_tx_fifo(p_data, tx_len) != tx_len) {
                    spin_unlock_irqrestore(&p_this->lock, irq_flags);

                    PCMCIA_ERROR("%s access pcmcia failed(%d-%d) - write %d bytes failed, STS = 0x%X, FIFO_CTRL=%08X, FIFO_STATUS=%08X, tx_bytes=%d!!!\n",
                                 __func__, tx_round, sub_round, tx_len,
                                 GET_PCMCIA_STS(), GET_PCMCIA_FIFO_CTRL(), GET_PCMCIA_FIFO_STATUS(),
                                 tx_bytes);

                    ret = PCMCIA_FAIL;
                    goto end_proc;
                }

                spin_unlock_irqrestore(&p_this->lock, irq_flags);
                len -= tx_len;
                p_data += tx_len;
                tx_bytes += tx_len;

                if (pcmcia_fifo_write_debug_enable) {
                    PCMCIA_WARNING("after write data(%d-%d) : STS = 0x%X, FIFO_CTRL=%08X, FIFO_STATUS=%08X, tx_bytes=%d!!!\n",
                                   tx_round, sub_round,
                                   GET_PCMCIA_STS(), GET_PCMCIA_FIFO_CTRL(), GET_PCMCIA_FIFO_STATUS(), tx_bytes);
                }
            }
        }

        //---------- wait for interrupts -----------

        try_count = 3000;
        event = GET_PCMCIA_STS();
        while ((event & PC_FIFO_TX_INT) == 0 && try_count--) {
            ndelay(300);
            event = GET_PCMCIA_STS();
        }

        if (pcmcia_fifo_write_debug_enable) {
            PCMCIA_WARNING("_rtk_pcmcia_fifo_write(%d-%d) : event=%08lx (%08lx) \n",
                           tx_round, sub_round,
                           event, (event & (PC_FIFO_R_IE | PC_FIFO_WR_OVER_IE)));
        }

        sub_round++;

        //---------- handle interrupts -----------
        if ((event & PC_FIFO_TX_INT) == 0) {
            PCMCIA_ERROR("%s access pcmcia failed - wait WR_OVER interrupt timeout (%u-%u), STS = 0x%X, FIFO_CTRL=%08X, FIFO_STATUS=%08X, tx_bytes=%d!!!\n",
                         __FUNCTION__, tx_round, sub_round,
                         GET_PCMCIA_STS(), GET_PCMCIA_FIFO_CTRL(), GET_PCMCIA_FIFO_STATUS(),
                         tx_bytes);
            ret = PCMCIA_FAIL;
            goto end_proc;
        }

        if ((event & PC_FIFO_ERRO_IE)) {
            PCMCIA_ERROR("%s access pcmcia failed - FIFO ERROR STS=%08lX fifo_status=%x len=%d tx_bytes=%d",
                         __FUNCTION__, event, GET_PCMCIA_FIFO_STATUS(), len,  tx_bytes);

            ret = PCMCIA_FAIL;
            goto end_proc;
        }

        if ((event & PC_FIFO_WR_OVER_IE)) {
            ret = tx_bytes;
            if (len) {
                PCMCIA_ERROR("%s access pcmcia failed - write imcomplete remain=%d\n",
                             __FUNCTION__, len);
                ret = PCMCIA_FAIL;
            }

            goto end_proc;
        }

        if ((GET_PCMCIA_STS() & PC_FIFO_W_IE)) {  // got write request, do next write
            continue;
        }
    }

end_proc:
    SET_PCMCIA_STS(PC_FIFO_TX_INT);
    preempt_enable();
    UNLOCK_PCMCIA();
    return ret;
}



int rtk_pcmcia_fifo_write(RTK_PCMCIA *p_this, unsigned short addr,
                          unsigned char *p_data, unsigned short len,
                          unsigned long flags)
{
    int ret = PCMCIA_OK;
    int step = (flags & PCMCIA_FLAGS_FIFO) ? 0 : ((flags & PCMCIA_FLAGS_ATTR)) ? 2 : 1;

    while (len) {
        rtk_pcmcia_phy_reset();

        ret = _rtk_pcmcia_fifo_write(p_this, addr, p_data, len, flags);

        if (ret <= 0) {
            PCMCIA_WARNING("%s failed\n", __FUNCTION__);
            return ret;
        }

        p_data += ret;
        len   -= ret;
        addr  += ret * step ;
    }

    return PCMCIA_OK;
}


static inline int rtk_pcmcia_do_command(RTK_PCMCIA *p_this,
                                        unsigned long cmd,
                                        unsigned long ctrl)
{
    unsigned long event = 0;
    int wait_time = 20000;
    unsigned int fifo_ctrl = 0;
    ctrl &= ~PC_ACCESS_INT;
    ctrl |= p_this->reg_bits.CE;

    //  PCMCIA_WARNING("%s 1)  fifo_ctrl = 0x%X ,  0x%X  =  0x%X ! \n" , __FUNCTION__ , fifo_ctrl, 0xb801b71c  , rtd_inl(0xb801b71c)  );
    fifo_ctrl = fifo_ctrl | PC_FIFO_EN(0);
    //  PCMCIA_WARNING("%s 2)fifo_ctrl = 0x%X  , x%X  =  0x%X ! \n" , __FUNCTION__ , fifo_ctrl ,  0xb801b71c  , rtd_inl(0xb801b71c)  );
    SET_PCMCIA_FIFO_CTRL(fifo_ctrl);
    SET_PCMCIA_STS(PC_ACCESS_INT);
    SET_PCMCIA_CTRL(ctrl);
    SET_PCMCIA_CMDFF(cmd);
    while (wait_time--) {
        event = GET_PCMCIA_STS();
        if (event & PC_ACCESS_INT) {
            break;
        }
        ndelay(100);
    }
    if (event & PC_APFI) {
        return PCMCIA_OK;
    }
    if (event & PC_AFI) {
        PCMCIA_WARNING("%s access pcmcia failed - Access Fault!!! , GET_PCMCIA_STS() = 0x%X \n", __func__, GET_PCMCIA_STS());
    }
    else {
        PCMCIA_WARNING("%s access pcmcia failed - Access Time Out!!! , GET_PCMCIA_STS() = 0x%X \n", __func__, GET_PCMCIA_STS());
        /*rtk_pcmcia_phy_reset(); */
    }
    SET_PCMCIA_STS(PC_ACCESS_INT);
    return PCMCIA_FAIL;
}

int rtk_pcmcia_read(RTK_PCMCIA *p_this, unsigned short addr,
                    unsigned char *p_data, unsigned short len,
                    unsigned long flags)
{
    int ret = PCMCIA_OK;
    unsigned char attr = (flags & PCMCIA_FLAGS_ATTR) ? 1 : 0;
    unsigned long ctrl = GET_PCMCIA_CTRL();
#ifdef RTK_PCMCIA1_REG
    pcmcia1_mis_pcmcia_dff_RBUS data_fifo_reg;
#else
    pcmcia_mis_pcmcia_dff_RBUS data_fifo_reg;
#endif

    PCMCIA_DBG("rd %s %s, addr = %d, len=%d\n",
               (flags & PCMCIA_FLAGS_ATTR) ? "attr" : "io",
               (flags & PCMCIA_FLAGS_FIFO) ? "port" : "mem", addr, len);

    if ((p_this->flags & PCMCIA_CARD_RESET_COMPLETE) == 0) {
        PCMCIA_WARNING("pcmcia read operation is failed!!, read data without card reset!!!\n");
        return PCMCIA_FAIL;
    }

    if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
        PCMCIA_WARNING("access pcmcia failed - no card exists!!!\n");
        return PCMCIA_NODEV;
    }

    if (!(GET_PCMCIA_CTRL() & p_this->reg_bits.PCR_OE)) {
        PCMCIA_WARNING
        ("access pcmcia failed - please enable port first!!!\n");
        return PCMCIA_FAIL;
    }

    LOCK_PCMCIA();

    preempt_disable();

    while (len && ret == PCMCIA_OK) {
        ret = rtk_pcmcia_do_command(p_this,
                                    PC_CT_READ | PC_AT(attr) |
                                    PC_PA(addr), ctrl);
        if (ret != PCMCIA_OK) {
            break;
        }

        SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0);
        data_fifo_reg.regValue = GET_PCMCIA_MIO_DATA_FIFO();
        *p_data = data_fifo_reg.df0;

        /*PCMCIA_WARNING("%s MEM[%d] = %02x\n", (attr) ? "ATTR" : "IO", addr, *p_data); */
        p_data++;
        if ((flags & PCMCIA_FLAGS_FIFO) == 0) { /* sequential read */
            addr += (attr) ? 2 : 1;
        }
        len--;
    }
    preempt_enable();
    UNLOCK_PCMCIA();

    return ret;
}

int rtk_pcmcia_write(RTK_PCMCIA *p_this, unsigned short addr,
                     unsigned char *p_data, unsigned short len,
                     unsigned long flags)
{
    int ret = PCMCIA_OK;
    unsigned char attr = (flags & PCMCIA_FLAGS_ATTR) ? 1 : 0;
    unsigned long ctrl = GET_PCMCIA_CTRL();
#ifdef RTK_PCMCIA1_REG
    pcmcia1_mis_pcmcia_dff_RBUS data_fifo_reg;
#else
    pcmcia_mis_pcmcia_dff_RBUS data_fifo_reg;
#endif

    PCMCIA_DBG("wr %s %s, addr = %d, len=%d\n",
               (flags & PCMCIA_FLAGS_ATTR) ? "attr" : "io",
               (flags & PCMCIA_FLAGS_FIFO) ? "mem" : "port", addr, len);

    if ((p_this->flags & PCMCIA_CARD_RESET_COMPLETE) == 0) {
        PCMCIA_WARNING("pcmcia write operation is failed!!, write data without card reset!!!\n");
        return PCMCIA_FAIL;
    }

    if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
        PCMCIA_WARNING("access pcmcia failed - no card exists!!!\n");
        return PCMCIA_NODEV;
    }

    if (!(GET_PCMCIA_CTRL() & p_this->reg_bits.PCR_OE)) {
        PCMCIA_WARNING
        ("access pcmcia failed - please enable port first!!!\n");
        return PCMCIA_FAIL;
    }

    LOCK_PCMCIA();

    preempt_disable();
    while (len && ret == PCMCIA_OK) {
        ret = rtk_pcmcia_do_command(p_this,
                                    PC_CT_WRITE | PC_AT(attr) |
                                    PC_PA(addr),
                                    ctrl);

        if (ret != PCMCIA_OK) {
            break;
        }

        SET_PCMCIA_MIO_FIFO_VALID(PCMCIA_DF0);
        data_fifo_reg.regValue = 0;
        data_fifo_reg.df0 = *p_data;
        SET_PCMCIA_MIO_DATA_FIFO(data_fifo_reg.regValue);
        p_data++;

        if ((flags & PCMCIA_FLAGS_FIFO) == 0) { /* sequential read */
            addr += (attr) ? 2 : 1;
        }
        len--;
    }
    preempt_enable();
    UNLOCK_PCMCIA();

    return ret;
}


int rtk_pcmcia_card_reset(RTK_PCMCIA *p_this,
                          unsigned long CardResetTimeout)
{
    int ret = PCMCIA_FAIL;
    unsigned long timeout;
    unsigned long stime;

    if (!(rtk_pcmcia_get_card_status(p_this) &
            RTK_PCMCIA_STS_CARD_PRESENT)) {

        PCMCIA_WARNING("hardware reset pcmcia (%d) failed - no card detected\n",
                       p_this->id);

        return PCMCIA_NODEV;
    }


    /* convert from ms to jiffes */
    PCMCIA_WARNING("reset pcmcia (%d) waiting for %lu ms \n", p_this->id, CardResetTimeout);

    CardResetTimeout *= HZ;
    CardResetTimeout /= 1000;
    LOCK_PCMCIA();
    SET_PCMCIA_STS(p_this->reg_bits.PII);
    SET_PCMCIA_CTRL((GET_PCMCIA_CTRL() & (~PC_PCR_SEL_MASK))
                    | PC_PCR_SEL(g_reset_pulse_width_type) | p_this->reg_bits.PCR);

    stime = jiffies;
    timeout = jiffies + CardResetTimeout;   /* timeout = 3 sec */
    while ((GET_PCMCIA_CTRL() & p_this->reg_bits.PCR)
            && time_before(jiffies, timeout)) {
        msleep(10);
    }

    if ((GET_PCMCIA_CTRL() & p_this->reg_bits.PCR)) {
        PCMCIA_WARNING("hardware reset pcmcia (%d) failed - timeout (%lu)\n",
                       p_this->id, ((jiffies - stime) * 1000) / HZ);
        goto end_proc;
    }
    msleep(1);

    if (!PCMCIA_INT_VALID(pcmcia_cfg2)) {
        /*PC card asserts READY within 10us of RESET if it will not be ready for access after 20ms */
        while ((rtd_inl(PCMCIA_REG_STS + 0x80) & p_this->reg_bits.PII) && //direct to read mio register(+0x80), because the mark2 PII status not correct
                time_before(jiffies, timeout)) {
            msleep(10);
        }

        if ((rtd_inl(PCMCIA_REG_STS + 0x80) & p_this->reg_bits.PII)) { //direct to read mio register(+0x80), because the mark2 PII status not correct
            ret = PCMCIA_FAIL;
            PCMCIA_WARNING("hardware reset pcmcia (%d) failed - wait PII timeout (%lu)\n",
                           p_this->id,
                           ((jiffies - stime) * 1000) / HZ);
            goto end_proc;
        }

        SET_PCMCIA_STS(p_this->reg_bits.PII);

    }
    else {
        /* polling int status */
        while ((0 == rtk_gpio_input(rtk_pcmcia_ptr->int_pin))
                && time_before(jiffies, timeout)) {
            msleep(10);
        }

        if (0 == rtk_gpio_input(rtk_pcmcia_ptr->int_pin)) {
            ret = PCMCIA_FAIL;
            PCMCIA_WARNING("hardware reset pcmcia (%d) failed - wait PII timeout (%lu)\n",
                           p_this->id, ((jiffies - stime) * 1000) / HZ);
            goto end_proc;
        }
    }

    p_this->flags |= PCMCIA_CARD_RESET_COMPLETE;
    PCMCIA_WARNING("hardware reset pcmcia (%d) successed (time= %lu ms)!!\n",
                   p_this->id, ((jiffies - stime) * 1000) / HZ);
    ret = PCMCIA_OK;
end_proc:
    UNLOCK_PCMCIA();


    if (PCMCIA_FAIL == ret && 1 == PCMCIA_RESET_WIHOUT_IREQ_CHECCK(pcmcia_cfg3)) {
        PCMCIA_WARNING("%s this card is has a long reset duration , but skip this situation!\n",
                       __func__);
        ret = PCMCIA_OK;
    }

    return ret;
}

int rtk_pcmcia_card_enable(RTK_PCMCIA *p_this, unsigned char On)
{
    if (On) {
        PCMCIA_WARNING("enable card %d\n", p_this->id);
        if (!
                (rtk_pcmcia_get_card_status(p_this) &
                 RTK_PCMCIA_STS_CARD_PRESENT)) {
            PCMCIA_WARNING("enable card %d failed, no card detected\n",
                           p_this->id);
            return PCMCIA_FAIL;
        }

        if (p_this->flags & PCMCIA_WITH_POWER_CTRL_GPIO) {
            rtk_gpio_output(p_this->pwr_en_pin, 1);
            rtk_gpio_set_dir(p_this->pwr_en_pin, 1);
            PCMCIA_WARNING("PCMCIA_PWR_EN GPIO : %s_GPIO %d=1\n",
                           gpio_type(gpio_group(p_this->pwr_en_pin)),
                           gpio_idx(p_this->pwr_en_pin));
        }
        msleep(2);
        SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() | p_this->
                        reg_bits.PCR_OE | p_this->reg_bits.PII);
        /*msleep(400); */
    }
    else {
        PCMCIA_WARNING("Card %d Disabled\n", p_this->id);
        SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() & ~p_this->reg_bits.PCR_OE);
        if (p_this->flags & PCMCIA_WITH_POWER_CTRL_GPIO) {

            rtk_gpio_output(p_this->pwr_en_pin, 0);
            rtk_gpio_set_dir(p_this->pwr_en_pin, 1);
            PCMCIA_WARNING("PCMCIA_PWR_EN GPIO : %s_GPIO %d=0\n",
                           gpio_type(gpio_group(p_this->pwr_en_pin)),
                           gpio_idx(p_this->pwr_en_pin));

            p_this->flags &= ~PCMCIA_CARD_RESET_COMPLETE;
        }
    }
    return PCMCIA_OK;
}

int rtk_pcmcia_enable(RTK_PCMCIA *p_this, unsigned char On)
{
    if (On) {
        /* enabled card detect interrupts  */
#ifdef RTK_PCMCIA_CD_DEBOUNCE
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, (rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg)) | (p_this->reg_bits.CD_DEB_INSERT_ISREN | p_this->reg_bits.CD_DEB_REMOVE_ISREN));
#else
        SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() | p_this->reg_bits.CD_INT);
#endif
    }
    else {
        rtk_pcmcia_card_enable(p_this, 0);
        /* clear card detect interrupts   */
#ifdef RTK_PCMCIA_CD_DEBOUNCE
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, (rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg)) & (~(p_this->reg_bits.CD_DEB_INSERT_ISREN | p_this->reg_bits.CD_DEB_REMOVE_ISREN)));
#else
        SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() & ~p_this->reg_bits.CD_INT);
#endif
        wake_up(&p_this->wq);
    }
    return 0;
}

#ifdef RTK_PCMCIA_CD_DEBOUNCE
int rtk_pcmcia_cd_debounce_enable(RTK_PCMCIA *p_this, unsigned char On)
{
    unsigned long ret;

    ret = rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB1_reg);

    if (On) {
        /* enabled cd debounce*/
        ret |= p_this->reg_bits.CD_DEB_EN;
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, ret);
    }
    else {
        ret &= (~p_this->reg_bits.CD_DEB_EN);
        rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, ret);

    }
    return 0;
}

int rtk_pcmcia_cd_debounce_set(RTK_PCMCIA *p_this, unsigned int value)
{
    unsigned long ret;
    if (g_cd_seperate2pins) {
        ret = PCMCIA_MIS_PCMCIA_CD_DEB0_cd0_debounce_threshold(value) | PCMCIA_MIS_PCMCIA_CD_DEB0_cd1_debounce_threshold(value);
    }
    else {
        switch (p_this->id) {
            case 0:
                ret = PCMCIA_MIS_PCMCIA_CD_DEB0_cd0_debounce_threshold(value);
                break;
            case 1:
                ret = PCMCIA_MIS_PCMCIA_CD_DEB0_cd1_debounce_threshold(value);
                break;
        }
    }
    rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB0_reg, ret);
    PCMCIA_WARNING("pcmcia[%d] set deounce value=%x\n", p_this->id, value);
    return 0;
}

unsigned int rtk_pcmcia_cd_debounce_get(RTK_PCMCIA *p_this)
{
    unsigned long ret;
    unsigned int deb_value = 0;

    ret = rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB0_reg);
    if (g_cd_seperate2pins) {
        deb_value = PCMCIA_MIS_PCMCIA_CD_DEB0_get_cd0_debounce_threshold(ret);
    }
    else {
        switch (p_this->id) {
            case 0:
                deb_value = PCMCIA_MIS_PCMCIA_CD_DEB0_get_cd0_debounce_threshold(ret);
                break;
            case 1:
                deb_value = PCMCIA_MIS_PCMCIA_CD_DEB0_get_cd1_debounce_threshold(ret);
                break;
        }
    }
    PCMCIA_WARNING("pcmcia[%d] get deounce value=%x\n", p_this->id, deb_value);
    return deb_value;
}

unsigned int rtk_pcmcia_cd_manual_mode_set(RTK_PCMCIA *p_this, unsigned int value)
{
    unsigned long ret;
    ret = rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg);
    if (g_cd_seperate2pins) {
        ret |= (PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_manual_mode(value) | PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_manual_mode(value));
    }
    else {
        switch (p_this->id) {
            case 0:
                ret |= PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_manual_mode(value);
                break;
            case 1:
                ret |= PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_manual_mode(value);
                break;
        }
    }
    rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, ret);
    PCMCIA_WARNING("pcmcia[%d] set manual mode=%x\n", p_this->id, value);
    return 0;

}

unsigned int rtk_pcmcia_cd_manual_value_set(RTK_PCMCIA *p_this, unsigned int value)
{
    unsigned long ret;
    ret = rtd_inl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg);
    if (g_cd_seperate2pins) {
        ret |= (PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_manual_value(value) | PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_manual_value(value));
    }
    else {
        switch (p_this->id) {
            case 0:
                ret |= PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_manual_value(value);
                break;
            case 1:
                ret |= PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_manual_value(value);
                break;
        }
    }
    rtd_outl(PCMCIA_MIS_PCMCIA_CD_DEB2_reg, ret);
    PCMCIA_WARNING("pcmcia[%d] set manual value=%x\n", p_this->id, value);
    return 0;

}
#endif


int rtk_pcmcia_poll_event(RTK_PCMCIA *p_this, unsigned long event_mask)
{
    int ret = 0;
    unsigned long flags = 0;

    POLL_EVENT_DBG("PollEvent=%lX (%d)\n", event_mask, current->pid);

    if (!PCMCIA_INT_VALID(pcmcia_cfg2)) {

        spin_lock_irqsave(&p_this->lock, flags);

        POLL_EVENT_DBG("PCMCIA STS=%x\n", GET_PCMCIA_STS());

        if ((GET_PCMCIA_STS() & p_this->reg_bits.PII)) {
            p_this->event |= RTK_PCMCIA_EVENT_IREQ_ASSERT;
        }
        else {
            p_this->event &= ~RTK_PCMCIA_EVENT_IREQ_ASSERT;
        }

        if (event_mask && (p_this->event & event_mask) == 0) {
            if (event_mask & RTK_PCMCIA_EVENT_IREQ_ASSERT)
                SET_PCMCIA_CTRL(GET_PCMCIA_CTRL() |
                                (p_this->reg_bits.PII));    /* eanble PII interrupt  */

            POLL_EVENT_DBG("start wait interrupt, PCMCIA CTRL=%x, PCMCIA STS=%x\n",
                           GET_PCMCIA_CTRL(), GET_PCMCIA_STS());

            spin_unlock_irqrestore(&p_this->lock, flags);
            wait_event_interruptible_timeout(p_this->wq,
                                             p_this->event & event_mask, HZ);

            POLL_EVENT_DBG("wait interrupt complete, PCMCIA STS=%x\n", GET_PCMCIA_STS());
            spin_lock_irqsave(&p_this->lock, flags);

        }

        ret = (p_this->event & event_mask);
        p_this->event = 0;
        spin_unlock_irqrestore(&p_this->lock, flags);

    }
    else {
        spin_lock_irqsave(&p_this->lock, flags);
        POLL_EVENT_DBG("pcmcia_int_status = 0x%08X\n", (unsigned int)(p_this->event));

        if (RTK_PCMCIA_EVENT_IREQ_ASSERT & p_this->event) {
            ret |= RTK_PCMCIA_EVENT_IREQ_ASSERT;
            p_this->event &= ~RTK_PCMCIA_EVENT_IREQ_ASSERT;
        }
        else {
            ret &= ~RTK_PCMCIA_EVENT_IREQ_ASSERT;
        }

        POLL_EVENT_DBG("pcmcia_int_status = 0x%08X  , ret = 0x%08X  \n",
                       p_this->event, ret);

        spin_unlock_irqrestore(&p_this->lock, flags);
    }

    POLL_EVENT_DBG("ExitPollEvent=%d  (%d)\n", ret, current->pid);
    return ret;
}

/*-----------------------------------------------------------------------------------
 * File Operations
 *------------------------------------------------------------------------------------*/
int rtk_pcmcia_open(struct inode *inode, struct file *file)
{
    RTK_PCMCIA *p_this = (RTK_PCMCIA *) rtk_pcmcia[0];
    file->private_data = (void *)p_this;
    if (p_this) {
        rtk_pcmcia_enable(p_this, 1);
        return 0;
    }
    return -ENOMEM;
}

int rtk_pcmcia_release(struct inode *inode, struct file *file)
{
    RTK_PCMCIA *p_this = (RTK_PCMCIA *) file->private_data;
    rtk_pcmcia_enable(p_this, 0);
    file->private_data = NULL;
    return 0;
}

int rtk_pcmcia_ioctrl_init(RTK_PCMCIA *p_this)
{

    PCMCIA_WARNING("pcmcia_ioctrl_init = 0x%08X  \n",
                   atomic_read(&p_this->atomic_init));

    if (atomic_read(&p_this->atomic_init) == 1) {
        return -1;
    }
    else {
        atomic_set(&p_this->atomic_init, 1);
        return 0;
    }

    return -1;

}

int rtk_pcmcia_ioctrl_deinit(RTK_PCMCIA *p_this)
{

    PCMCIA_WARNING("pcmcia_ioctrl_init = 0x%08X  \n", atomic_read(&p_this->atomic_init));

    if (atomic_read(&p_this->atomic_init) == 1) {
        atomic_set(&p_this->atomic_init, 0);
        return 0;
    }

    return -1;
}


static long rtk_pcmcia_ioctl(struct file *file, unsigned int cmd,
                             unsigned long arg)
{
    RTK_PCMCIA *p_this = (RTK_PCMCIA *) file->private_data;
    RTK_PCMCIA_RW rw_cmd;
    RTK_PCMCIA_MULTI_RW multi_rw_cmd;

    int ret = -ENOTTY;
    unsigned long CardResetTimeout = DEFAULT_PCMCIA_RESET_TIMEOUT;

    mutex_lock(&rtk_pcmcia_ioctl_lock);

    switch (cmd) {
        case RTK_PCMCIA_IOC_ENABLE:
            ret = rtk_pcmcia_card_enable(p_this, (arg) ? 1 : 0);
            break;
        case RTK_PCMCIA_IOC_CARD_RESET:

            PCMCIA_WARNING("%s  RTK_PCMCIA_IOC_CARD_RESET , %lu ms\n", __func__, arg);

            if (arg) {
                if (arg < MINIMUM_PCMCIA_RESET_TIMEOUT) {
                    CardResetTimeout = MINIMUM_PCMCIA_RESET_TIMEOUT;
                }
                else if (arg > MAXIMUM_PCMCIA_RESET_TIMEOUT) {
                    CardResetTimeout = MAXIMUM_PCMCIA_RESET_TIMEOUT;
                }
                else {
                    CardResetTimeout = arg;
                }
            }
            ret = rtk_pcmcia_card_reset(p_this, CardResetTimeout);
            break;

        case RTK_PCMCIA_IOC_GET_CARD_STATUS:

            ret = rtk_pcmcia_get_card_status(p_this);

            if (1 == bRtkPcmciaResume) {
                ret |= RTK_PCMCIA_STS_DC_OFF_ON;
                bRtkPcmciaResume = 0;
                PCMCIA_WARNING("DC on/off happened!\n");
            }
            break;

        case RTK_PCMCIA_IOC_READ_WRITE:
            if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
                PCMCIA_WARNING("%s access pcmcia failed - no card exists!!!\n", __func__);
                ret = -EFAULT;
                break;
            }

            if (copy_from_user(&rw_cmd, (void __user *)arg, sizeof(RTK_PCMCIA_RW))) {
                PCMCIA_WARNING("read write PCMCIA failed, copy data from user space failed\n");
                ret = -EFAULT;
                break;
            }

            if (rw_cmd.flags & PCMCIA_FLAGS_RD) {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_read(p_this,
                                               rw_cmd.addr, &rw_cmd.data,
                                               1, rw_cmd.flags);
                else
                    ret = rtk_pcmcia_read(p_this, rw_cmd.addr,
                                          &rw_cmd.data,
                                          1, rw_cmd.flags);
                if (ret == PCMCIA_OK) {
                    if (copy_to_user((void __user *)arg, &rw_cmd, sizeof(RTK_PCMCIA_RW))) {
                        PCMCIA_WARNING("read write PCMCIA failed, copy data to user space failed\n");
                        ret = -EFAULT;
                        break;
                    }
                }
            }
            else {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_write(p_this,
                                                rw_cmd.addr,
                                                &rw_cmd.data,
                                                1, rw_cmd.flags);
                else
                    ret = rtk_pcmcia_write(p_this, rw_cmd.addr,
                                           &rw_cmd.data,
                                           1, rw_cmd.flags);
            }
            break;
        case RTK_PCMCIA_IOC_MULTI_READ_WRITE:

            if (!(rtk_pcmcia_get_card_status(p_this) & RTK_PCMCIA_STS_CARD_PRESENT)) {
                PCMCIA_WARNING("%s access pcmcia failed - no card exists!!!\n", __func__);
                ret = -EFAULT;
                break;
            }

            if (copy_from_user(&multi_rw_cmd, (void __user *)arg,
                               sizeof(RTK_PCMCIA_MULTI_RW))) {
                PCMCIA_WARNING("read write PCMCIA failed, copy data from user space failed\n");
                ret = -EFAULT;
                break;
            }
            if (multi_rw_cmd.len > sizeof(p_this->buff)) {
                PCMCIA_WARNING("read write PCMCIA failed, data size large than %d\n",
                               (unsigned int)sizeof(p_this->buff));
                ret = -EFAULT;
                break;
            }
            if (multi_rw_cmd.flags & PCMCIA_FLAGS_RD) {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_read(p_this,
                                               multi_rw_cmd.addr,
                                               p_this->buff, multi_rw_cmd.len,
                                               multi_rw_cmd.flags);
                else
                    ret = rtk_pcmcia_read(p_this,
                                          multi_rw_cmd.addr,
                                          p_this->buff, multi_rw_cmd.len,
                                          multi_rw_cmd.flags);
                if (ret == PCMCIA_OK) {
                    if (copy_to_user((void __user *)multi_rw_cmd.p_data, p_this->buff, multi_rw_cmd.len)) {
                        PCMCIA_WARNING("read write PCMCIA failed, copy data to user space failed\n");
                        ret = -EFAULT;
                        break;
                    }
                }
            }
            else {
                if (copy_from_user(&p_this->buff, (void __user *)multi_rw_cmd.p_data, multi_rw_cmd.len)) {
                    PCMCIA_WARNING("read write PCMCIA failed, copy data from user space failed\n");
                    ret = -EFAULT;
                    break;
                }
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_write(p_this,
                                                multi_rw_cmd.addr,
                                                p_this->buff,
                                                multi_rw_cmd.len,
                                                multi_rw_cmd.flags);
                else
                    ret = rtk_pcmcia_write(p_this,
                                           multi_rw_cmd.addr,
                                           p_this->buff,
                                           multi_rw_cmd.len,
                                           multi_rw_cmd.flags);

            }
            break;
        case RTK_PCMCIA_IOC_PROBE_COMMAND:
            switch (arg) {
                case RTK_PCMCIA_IOC_ENABLE:
                case RTK_PCMCIA_IOC_CARD_RESET:
                case RTK_PCMCIA_IOC_GET_CARD_STATUS:
                case RTK_PCMCIA_IOC_READ_WRITE:
                case RTK_PCMCIA_IOC_MULTI_READ_WRITE:
                case RTK_PCMCIA_IOC_PROBE_COMMAND:
                case RTK_PCMCIA_IOC_POLL_EVENT:
                    ret = 0;
                    break;
                case RTK_PCMCIA_IOC_INIT_COMMAND:
                    ret = rtk_pcmcia_ioctrl_init(p_this);
                    break;
                case RTK_PCMCIA_IOC_DEINIT_COMMAND:
                    ret = rtk_pcmcia_ioctrl_deinit(p_this);
                    break;
                default:
                    ret = -EFAULT;
            }
            break;

        case RTK_PCMCIA_IOC_POLL_EVENT:
            mutex_unlock(&rtk_pcmcia_ioctl_lock);    // unlock mutex to make sure other request won't be b;pcled
            ret = rtk_pcmcia_poll_event(p_this, arg);
            mutex_lock(&rtk_pcmcia_ioctl_lock);
            break;

        default:
            PCMCIA_WARNING("do ioctl failed, invalid command - %x\n", cmd);
    }

    mutex_unlock(&rtk_pcmcia_ioctl_lock);

    return ret;
}

#ifdef CONFIG_COMPAT
static long rtk_pcmcia_compat_ioctl(struct file *file, unsigned int cmd,
                                    unsigned long arg)
{
    RTK_PCMCIA *p_this = (RTK_PCMCIA *) file->private_data;
    RTK_PCMCIA_RW rw_cmd;
    COMPAT_RTK_PCMCIA_MULTI_RW multi_rw_cmd;

    int ret = -ENOTTY;
    unsigned long CardResetTimeout = DEFAULT_PCMCIA_RESET_TIMEOUT;
    switch (cmd) {
        case RTK_PCMCIA_IOC_ENABLE:
            return rtk_pcmcia_card_enable(p_this, (arg) ? 1 : 0);
        case RTK_PCMCIA_IOC_CARD_RESET:
            if (arg) {
                if (arg < MINIMUM_PCMCIA_RESET_TIMEOUT) {
                    CardResetTimeout = MINIMUM_PCMCIA_RESET_TIMEOUT;
                }
                else if (arg > MAXIMUM_PCMCIA_RESET_TIMEOUT) {
                    CardResetTimeout = MAXIMUM_PCMCIA_RESET_TIMEOUT;
                }
                else {
                    CardResetTimeout = arg;
                }
            }
            ret = rtk_pcmcia_card_reset(p_this, CardResetTimeout);
            return ret;
        case RTK_PCMCIA_IOC_GET_CARD_STATUS:
            ret = rtk_pcmcia_get_card_status(p_this);
            if (1 == bRtkPcmciaResume) {
                ret |= RTK_PCMCIA_STS_DC_OFF_ON;
                bRtkPcmciaResume = 0;
                PCMCIA_WARNING("DC on/off happened!\n");
            }
            break;
        case COMPAT_RTK_PCMCIA_IOC_READ_WRITE:
            if (copy_from_user
                    (&rw_cmd, (void __user *)arg, sizeof(RTK_PCMCIA_RW))) {
                PCMCIA_WARNING
                ("read write PCMCIA failed, copy data from user space failed\n");
                return -EFAULT;
            }
            if (rw_cmd.flags & PCMCIA_FLAGS_RD) {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_read(p_this, rw_cmd.addr, &rw_cmd.data,
                                               1, rw_cmd.flags);
                else
                    ret = rtk_pcmcia_read(p_this, rw_cmd.addr, &rw_cmd.data,
                                          1, rw_cmd.flags);
                if (ret == PCMCIA_OK) {
                    if (copy_to_user
                            ((void __user *)arg, &rw_cmd,
                             sizeof(RTK_PCMCIA_RW))) {
                        PCMCIA_WARNING
                        ("read write PCMCIA failed, copy data to user space failed\n");
                        return -EFAULT;
                    }
                }
            }
            else {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_write(p_this, rw_cmd.addr, &rw_cmd.data,
                                                1, rw_cmd.flags);
                else
                    ret = rtk_pcmcia_write(p_this, rw_cmd.addr, &rw_cmd.data,
                                           1, rw_cmd.flags);
            }
            break;
        case COMPAT_RTK_PCMCIA_IOC_MULTI_READ_WRITE:
            if (copy_from_user(&multi_rw_cmd, compat_ptr(arg),
                               sizeof(COMPAT_RTK_PCMCIA_MULTI_RW))) {
                PCMCIA_WARNING
                ("read write PCMCIA failed, copy data from user space failed\n");
                return -EFAULT;
            }
            if (multi_rw_cmd.len > sizeof(p_this->buff)) {
                PCMCIA_WARNING
                ("read write PCMCIA failed, data size large than %lu\n",
                 sizeof(p_this->buff));
                return -EFAULT;
            }
            if (multi_rw_cmd.flags & PCMCIA_FLAGS_RD) {
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_read(p_this,
                                               multi_rw_cmd.addr,
                                               p_this->buff, multi_rw_cmd.len,
                                               multi_rw_cmd.flags);
                else
                    ret = rtk_pcmcia_read(p_this,
                                          multi_rw_cmd.addr,
                                          p_this->buff, multi_rw_cmd.len,
                                          multi_rw_cmd.flags);
                if (ret == PCMCIA_OK) {
                    if (copy_to_user
                            (compat_ptr(multi_rw_cmd.p_data), p_this->buff,
                             multi_rw_cmd.len)) {
                        PCMCIA_WARNING
                        ("read write PCMCIA failed, copy data to user space failed\n");
                        return -EFAULT;
                    }
                }
            }
            else {
                if (copy_from_user
                        (&p_this->buff, compat_ptr(multi_rw_cmd.p_data),
                         multi_rw_cmd.len)) {
                    PCMCIA_WARNING
                    ("read write PCMCIA failed, copy data from user space failed\n");
                    return -EFAULT;
                }
                if (pcmcia_fifo_mode_enable)
                    ret = rtk_pcmcia_fifo_write(p_this,
                                                multi_rw_cmd.addr,
                                                p_this->buff,
                                                multi_rw_cmd.len,
                                                multi_rw_cmd.flags);
                else
                    ret = rtk_pcmcia_write(p_this,
                                           multi_rw_cmd.addr,
                                           p_this->buff,
                                           multi_rw_cmd.len,
                                           multi_rw_cmd.flags);

            }
            break;
        case COMPAT_RTK_PCMCIA_IOC_PROBE_COMMAND:
            switch (arg) {
                case RTK_PCMCIA_IOC_ENABLE:
                case RTK_PCMCIA_IOC_CARD_RESET:
                case RTK_PCMCIA_IOC_GET_CARD_STATUS:
                case COMPAT_RTK_PCMCIA_IOC_READ_WRITE:
                case COMPAT_RTK_PCMCIA_IOC_MULTI_READ_WRITE:
                case COMPAT_RTK_PCMCIA_IOC_PROBE_COMMAND:
                case COMPAT_RTK_PCMCIA_IOC_POLL_EVENT:
                    return 0;
                default:
                    ret = -EFAULT;
            }
            break;
        case COMPAT_RTK_PCMCIA_IOC_POLL_EVENT:
            ret = rtk_pcmcia_poll_event(p_this, arg);
            break;
        case COMPAT_RTK_PCMCIA_IOC_INIT_COMMAND:
            return rtk_pcmcia_ioctrl_init(p_this);
        case COMPAT_RTK_PCMCIA_IOC_DEINIT_COMMAND:
            return rtk_pcmcia_ioctrl_deinit(p_this);
        default:
            PCMCIA_WARNING("%s, do ioctl failed, invalid command - %x\n", __FUNCTION__, cmd);
    }
    return ret;
}
#endif

static struct file_operations rtk_pcmcia_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = rtk_pcmcia_ioctl,
    .open = rtk_pcmcia_open,
    .release = rtk_pcmcia_release,
#ifdef CONFIG_COMPAT
    .compat_ioctl = rtk_pcmcia_compat_ioctl,
#endif
};

/***************************************************************************
*------------------- Power Management ----------------
****************************************************************************/

//static struct platform_device *pcmcia_platform_dev;

#ifdef CONFIG_PM
static int pcmcia_suspend(struct platform_device *dev, pm_message_t state)
{
    PCMCIA_WARNING("suspended\n");

#ifdef CONFIG_ARM64
    rtk_pcmcia_enable(rtk_pcmcia_ptr, 0);
#else
    rtk_pcmcia_enable(rtk_pcmcia[0], 0);
#endif

    if (PCMCIA_INT_VALID(pcmcia_cfg2)) {
        rtk_gpio_set_irq_enable(rtk_pcmcia_ptr->int_pin, 0);
        rtk_gpio_clear_isr(rtk_pcmcia_ptr->int_pin);

#ifdef CONFIG_ARM64
        rtk_gpio_free_irq(rtk_pcmcia_ptr->int_pin, (void *)0);
#else
        rtk_gpio_free_irq(rtk_pcmcia_ptr->int_pin, rtk_pcmcia_gpio_int_isr);
#endif
    }
    bRtkPcmciaResume = 1;
	
    PCMCIA_ClockEnable(0);     /*  disable PCMCIA clock */
    return 0;
}

static int pcmcia_resume(struct platform_device *dev)
{
    PCMCIA_WARNING("resumed\n");
    PCMCIA_ClockEnable(1);     /*  enable PCMCIA clock */
    msleep(1);//wait pcmcia clk ready
	
    rtk_pcmcia_phy_reset();

    if (PCMCIA_INT_VALID(pcmcia_cfg2)) {
        rtk_pcmcia_int_gpio_set();
    }
    rtd_outl(PCMCIA_REG_PIN_SEL, (rtd_inl(PCMCIA_REG_PIN_SEL))
             | PC_INPACK0_DIS(1) | PC_WAIT0_DIS(0));
    SET_PCMCIA_AMTC_0(pcmcia_amtc0);
    SET_PCMCIA_AMTC_1(pcmcia_amtc1);
    SET_PCMCIA_IOMTC(pcmcia_iomtc);
    SET_PCMCIA_MATC_0(pcmcia_matc0);
    SET_PCMCIA_MATC_1(pcmcia_matc1);
    return 0;
}

#endif

static int rtk_pcmcia_probe(struct platform_device *pdev)
{
    int irq, ret;

    irq = platform_get_irq(pdev, 0);

    if (irq < 0) {
        PCMCIA_WARNING("rtk_pcmcia_probe get_irq failed \n");
        return irq;
    }

    ret = request_irq(irq, rtk_pcmcia_isr, SA_SHIRQ, "pcmcia", rtk_pcmcia[0]);

    if (ret < 0) {
        PCMCIA_WARNING("rtk_pcmcia_module_init failed - \n");
        release_rtk_pcmcia(rtk_pcmcia[0]);
        rtk_pcmcia_ptr = NULL;
        return -ENODEV;
    }
    return 0;
}

static int rtk_pcmcia_remove(struct platform_device *pdev)
{
    int irq;

    irq = platform_get_irq(pdev, 0);

    if (irq >= 0) {
        free_irq(irq, (void *)pdev);
    }

    return 0;
}

static const struct of_device_id of_rtk_pcmcia_ids[] = {
    { .compatible = "realtek,PCMCIA" },
    {}
};
MODULE_DEVICE_TABLE(of, of_rtk_pcmcia_ids);

static struct platform_driver pcmcia_platform_drv = {
    .probe      = rtk_pcmcia_probe,
    .remove     = rtk_pcmcia_remove,
#ifdef CONFIG_PM
    .suspend = pcmcia_suspend,
    .resume  = pcmcia_resume,
#endif
    .driver   = {
        .name = "PCMCIA",
        .bus  = &platform_bus_type,

        .of_match_table = of_match_ptr(of_rtk_pcmcia_ids),
    }
};

/*
 * module Init
 */
static struct miscdevice rtk_pcmcia_miscdev = {
    MISC_DYNAMIC_MINOR,
    "pcmcia-0",
    &rtk_pcmcia_fops
};

static void pcmcia_pinmux_set(void)
{
    // TBD
}

int __init rtk_pcmcia_module_init(void)
{
    int ret = 0;

#ifdef CONFIG_CUSTOMER_TV006
    PCMCIA_WARNING("%s isPcbWithPcmcia = %d \n", __func__, isPcbWithPcmcia());
    if (0 == isPcbWithPcmcia()) {
        PCMCIA_WARNING("this pcb is japan type , pcmcia driver is not needed \n");
        PCMCIA_WARNING("this pcb is japan type , pcmcia driver is not needed \n");
        PCMCIA_WARNING("this pcb is japan type , pcmcia driver is not needed \n");
        PCMCIA_WARNING("this pcb is japan type , pcmcia driver is not needed \n");
        PCMCIA_WARNING("this pcb is japan type , pcmcia driver is not needed \n");
        return -ENODEV;
    }
#endif
    if (get_product_type() == PRODUCT_TYPE_DIAS) {
        PCMCIA_WARNING("pcmcia not support on dias platform\n");
        return 0;
    }
#ifdef CONFIG_ARCH_RTK6702
#else
    if ( rtk_is_pcmcia_disable(PCMCIA_MIO) ) {
        PCMCIA_WARNING("pcmcia 1 otp disabled\n");
        return -ENODEV;
    }
#endif
    rtk_pcmcia_gpio_config();
    rtk_pcmcia_gpio_config2();
    rtk_pcmcia_gpio_config3();
    if ((pcmcia_cfg & (RTK_PCMCIA_CFG_PCMCIA0_EN | RTK_PCMCIA_CFG_PCMCIA1_EN)) == 0) {
        PCMCIA_WARNING("PCMCIA_config=0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);
        PCMCIA_WARNING("PCMCIA_config=0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);
        PCMCIA_WARNING("PCMCIA_config=0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);

        return -ENODEV;
    }
    /* get pcmcia=on or off from bootcode, if "pcmcia=off", no pcmcia driver
    ** default pcmcia=on, if no parameter from bootcode
    */
    rtk_pcmcia_get_boot_parameters();
    if ((pcmcia_cfg & (RTK_PCMCIA_CFG_PCMCIA0_EN | RTK_PCMCIA_CFG_PCMCIA1_EN)) == 0) {
        PCMCIA_WARNING("bootcode pcmcia=off,pcmcia_cfg = 0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);
        PCMCIA_WARNING("bootcode pcmcia=off,pcmcia_cfg = 0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);
        PCMCIA_WARNING("bootcode pcmcia=off,pcmcia_cfg = 0x%llX, bit19 = 0 & bit23 = 0 , no PCMCIA kdriver \n", pcmcia_cfg);

        return -ENODEV;
    }

    PCMCIA_ClockEnable(1);  /*  enable PCMCIA clock */
    msleep(1);
    rtk_pcmcia_phy_reset();
    rtk_pcmcia[0] = create_rtk_pcmcia(0);
    if (rtk_pcmcia[0] == NULL) {
        PCMCIA_WARNING("rtk_pcmcia_module_init failed - create pcmcia driver failed\n");
        return -ENODEV;
    }
    rtk_pcmcia_ptr = rtk_pcmcia[0];
    pcmcia_pinmux_set();
    rtk_pcmcia_int_gpio_set();
#ifdef RTK_PCMCIA_CD_DEBOUNCE
    PCMCIA_WARNING("Enable PCMCIA new CD debounce mode\n");
    rtk_pcmcia_cd_debounce_enable(rtk_pcmcia[0], 1);
    rtk_pcmcia_cd_debounce_set(rtk_pcmcia[0], 0x999A); //1.5ms
#endif

    rtk_pcmcia_enable(rtk_pcmcia[0], 0);    /* disable pcmcia */
    /*
        ret = request_irq(IRQ_GET_KERNEL_IRQ_NUM(PCMCIA_IRQ), rtk_pcmcia_isr, SA_SHIRQ, "pcmcia",
                        rtk_pcmcia[0]);

        if (ret < 0)
        {
            PCMCIA_WARNING("rtk_pcmcia_module_init failed - Request irq%d failed(ret=%d)\n",
                           IRQ_GET_KERNEL_IRQ_NUM(PCMCIA_IRQ), ret);
            release_rtk_pcmcia(rtk_pcmcia[0]);
            rtk_pcmcia_ptr = NULL;
            return -ENODEV;
        }
    */
    if (misc_register(&rtk_pcmcia_miscdev)) {
        PCMCIA_WARNING("rtk_pcmcia_module_init failed - register misc device failed\n");
        release_rtk_pcmcia(rtk_pcmcia[0]);
        rtk_pcmcia_ptr = NULL;
        //free_irq(IRQ_GET_KERNEL_IRQ_NUM(PCMCIA_IRQ), &rtk_pcmcia_miscdev);
        return -ENODEV;
    }

    platform_driver_register(&pcmcia_platform_drv); /* regist mcp driver */

    if ((pcmcia_cfg & RTK_PCMCIA_CFG_SKIP_DRIVERS_TIMING_CONFIGURATION) == 0) {
        PCMCIA_WARNING("using drivers pcmcia timing configuration\n");
        /* this timing is provided by DIC. */
        SET_PCMCIA_AMTC_0(pcmcia_amtc0);
        SET_PCMCIA_AMTC_1(pcmcia_amtc1);
        SET_PCMCIA_IOMTC(pcmcia_iomtc);
        SET_PCMCIA_MATC_0(pcmcia_matc0);
        SET_PCMCIA_MATC_1(pcmcia_matc1);
    }
    else {
        PCMCIA_WARNING("skip pcmcia timing configuration\n");
    }

    creat_pcmcia_device_node(rtk_pcmcia_miscdev.this_device);
    return ret;
}

static void __exit rtk_pcmcia_module_exit(void)
{

    if (NULL != rtk_pcmcia_ptr) {

        misc_deregister(&rtk_pcmcia_miscdev);

        platform_driver_unregister(&pcmcia_platform_drv);

        //free_irq(IRQ_GET_KERNEL_IRQ_NUM(PCMCIA_IRQ), &rtk_pcmcia_miscdev);
        rtk_pcmcia_enable(rtk_pcmcia[0], 0);    /* disable pcmcia */
        release_rtk_pcmcia(rtk_pcmcia[0]);

        remove_pcmcia_device_node(rtk_pcmcia_miscdev.this_device);
    }
}

module_init(rtk_pcmcia_module_init);
module_exit(rtk_pcmcia_module_exit);
MODULE_AUTHOR("Kevin Wang, Realtek Semiconductor");
MODULE_LICENSE("GPL");
