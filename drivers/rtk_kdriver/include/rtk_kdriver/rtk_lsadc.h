#ifndef __RTK_LSADC_H__
#define __RTK_LSADC_H__

//#include "rbus/rbusTypes.h"
//#include "rtk_lsadc_reg.h"


#include <rtd_log/rtd_module_log.h>
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
#include <mach/pcbMgr.h>
#endif

#if (defined(CONFIG_ARCH_RTK2851A) || defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F))
#define MAX_LSADC_COUNT            9
#else
#define MAX_LSADC_COUNT            11
#endif

#define MAX_LSADC_THRESHOLD        0xff
#define MAX_LSADC_PCBINFO_COUNT    20
#define MAX_LSADC_INT_BLOCK_PIN    3
#define MAX_LSADC_INT_BLOCK        6

#define LSADC_INIT_MASK_NONE   0
#define LSADC_INIT_MASK_BIT_0  1 << 0
#define LSADC_INIT_MASK_BIT_1  1 << 1
#define LSADC_INIT_MASK_BIT_2  1 << 2
#define LSADC_INIT_MASK_BIT_3  1 << 3
#define LSADC_INIT_MASK_BIT_4  1 << 4
#define LSADC_INIT_MASK_BIT_5  1 << 5
#define LSADC_INIT_MASK_BIT_6  1 << 6
#define LSADC_INIT_MASK_BIT_7  1 << 7
#define LSADC_INIT_MASK_BIT_8  1 << 8
#define LSADC_INIT_MASK_BIT_9  1 << 9

/*0xfff means 12 lsadc pins,every pin owns one status bit*/
#define LSADC_INT_THREHOLD_STATUS_MASK  0xffe
/*0x4f means 6 blocks,every pin owns six block status bit*/
#define LSADC_INT_BLOCKSTATUS_MASK      0x3f



#define LSADC_BLOCK_INT_MASK_NONE   0
#define LSADC_BLOCK0_INT_MASK  1 << 0
#define LSADC_BLOCK1_INT_MASK  1 << 1
#define LSADC_BLOCK2_INT_MASK  1 << 2
#define LSADC_BLOCK3_INT_MASK  1 << 3
#define LSADC_BLOCK4_INT_MASK  1 << 4
#define LSADC_BLOCK5_INT_MASK  1 << 5



#define LSADC_THREDHOLD_INT_MASK   1<<7

enum LSADC_INIT_MASK_BIT {
        LSADC_INIT_NONE = LSADC_INIT_MASK_NONE,
        LSADC_INIT_0  = LSADC_INIT_MASK_BIT_0,
        LSADC_INIT_1  = LSADC_INIT_MASK_BIT_1,
        LSADC_INIT_2  = LSADC_INIT_MASK_BIT_2,
        LSADC_INIT_3  = LSADC_INIT_MASK_BIT_3,
        LSADC_INIT_4  = LSADC_INIT_MASK_BIT_4,
        LSADC_INIT_5  = LSADC_INIT_MASK_BIT_5,
        LSADC_INIT_6  = LSADC_INIT_MASK_BIT_6,
        LSADC_INIT_7  = LSADC_INIT_MASK_BIT_7,
        LSADC_INIT_8  = LSADC_INIT_MASK_BIT_8,
        LSADC_INIT_9  = LSADC_INIT_MASK_BIT_9,
};

#define SET_LSADC_INIT_MASK(value)   \
lsadc_init_status_mask = lsadc_init_status_mask | value;\

#define RTK_LSADC_ID      unsigned int
#define RTK_LSADC_NAME    char*

typedef enum{
    VOLTAGE_MODE = 0,
    CURRENT_MODE = 1,
}LSADC_MODE;

typedef enum{
    ADC_NONE =0,
    /*In this mode lsadc interrupt trigger when the changed value is bigger than the thredhold value which you set */
    ADC_THREDHOLD_MODE = 1,
    /*In this mode lsadc interrupt trigger when value change to a block area which you */
    ADC_BLOCK_MODE = 2,
}LSADC_IRQ_MODE;

typedef enum{
    LSADC_NONE_USED =0,
    USED_AS_LSADC_PIN = 1,
    USED_AS_BLOCK_SOURCE = 2,
}LSADC_PIN_STATUS;

typedef struct _RTK_LSADC_BLOCK
{
    char top_bound;
    char low_bound;
} LSADC_BLOCK;

typedef struct _RTK_LSADC_BLOCK_SOURCE
{
    RTK_LSADC_ID index;
    LSADC_PIN_STATUS status;
    LSADC_IRQ_MODE int_status;
    char pad_switch;
    unsigned char thredHold;
    unsigned char block_cnt;
    LSADC_BLOCK block[6];
} LSADC_BLOCK_SOURCE;


typedef struct _RTK_LSADC_IRQ_SET 
{
    unsigned char pin_name[32];
    /*There two IRQ mode as Thredhold mode and block mode                               */
    /*If you set ThredHold mode, you can ignore the para block_cnt and block array,and  */
    /*  and you can change the default thredHold.*/
    /*If you set Block mode, you should set block_cnt to choose how many block you want,*/
    /*  and to set the block info as top_bound and low_bound. */
    LSADC_IRQ_MODE irq_mode;
    unsigned char thredHold;
    unsigned char block_cnt;
    LSADC_BLOCK block[6];
} RTK_LSADC_IRQ_SET;

typedef struct
{
    char name[32];
    RTK_LSADC_ID index;
    RTK_LSADC_IRQ_SET irq_info;
    /*lsadc1/2/3/4/8/... can not set block mode interrupt, must use lsadc5/6/7 pad to switch them*/
    char block_soure;
    char irq_flag;
    void *dev_id;
    void (*handler) (RTK_LSADC_ID id, unsigned char assert, void *dev_id);   /* assert call back */
} RTK_LSADC_IRQ;

typedef struct _lsadc_pcb_info {
    unsigned char pinName[64];
    unsigned long long ullPcbMgrValue;
    unsigned int index;
    unsigned char hitValue;
    unsigned char torrence;
    unsigned char irq_mode;
    unsigned char irq_thredHold;
}LSADC_PCBINFO;

typedef struct _RTK_LSADC_INFO 
{
    int pinNumber;
    int pad_active;
    int pad_thred;
    int pad_sw;
    int pad_ctrl;
    int adc_val ;

} RTK_LSADC_INFO;

typedef struct {
    unsigned char pin_name[32];
    RTK_LSADC_INFO info;
} RTK_LSADC_PCBINFO_T;

typedef struct {
    RTK_LSADC_PCBINFO_T  pcbInfo[20];
    int count;
} RTK_ALL_LSADC_PIN_T ;

typedef struct {
    char  pin_name[32];
    unsigned char val ;
} RTK_LSADC_VAL_T ;

void LSADC_ClockEnable(unsigned char On);
int rtk_hw_setting_adc_init(void);
void rtk_lsadc_init_pcb_pin(void);
int rtk_lsadc_dump_all_pininfo(RTK_ALL_LSADC_PIN_T* pcbInfo);
int rtk_lsadc_get_index_byName(char* pinName,unsigned int *val);
int get_lsadc_init_status(RTK_LSADC_ID index);
int rtk_lsadc_defualt_enable(int index, LSADC_MODE lsadc_mode);

int rtk_lsadc_get_adcValue(RTK_LSADC_ID index, unsigned char* value);
int rtk_lsadc_get_threshold(RTK_LSADC_ID index, unsigned char* threshold);
int rtk_lsadc_get_activeStatus(RTK_LSADC_ID index, unsigned char* activeStatus);
int rtk_lsadc_get_padSwitch(RTK_LSADC_ID index, unsigned char* padSwitch);
int rtk_lsadc_get_voltageMode(RTK_LSADC_ID index, unsigned char* voltageMode);

int rtk_lsadc_set_threshold(RTK_LSADC_ID index, unsigned char threshold);
int rtk_lsadc_set_active(RTK_LSADC_ID index, unsigned char activeStatus);
int rtk_lsadc_set_padSwitch(RTK_LSADC_ID index, unsigned char padSwitch);
int rtk_lsadc_set_voltageMode(RTK_LSADC_ID index, unsigned char voltageMode);

int rtk_lsadc_get_lsadc_config(RTK_LSADC_INFO  *lsadc_info);
int rtk_lsadc_set_lsadc_config(RTK_LSADC_INFO  *lsadc_info);
int rtk_lsadc_dump_mainLsadcCtrl(void);
int rtk_lsadc_dump_mainAnalogCtrl(void);

int rtk_lsadc_get_adcValue_byName(RTK_LSADC_NAME name, unsigned char* value);

int rtk_lsadc_enable_int(RTK_LSADC_IRQ_SET irq_info,unsigned char enable);
int rtk_lsadc_request_irq(RTK_LSADC_ID index,void (*handler) (RTK_LSADC_ID id, unsigned char assert, void *dev_id),char *int_name,void *dev_id);
void rtk_lsadc_free_irq(RTK_LSADC_ID index, void *dev_id);irqreturn_t rtk_lsadc_interrupt_handler(int irq, void *dev_id);




#define LSADC_DEBUG(fmt, args...) \
{ \
        if (log_onoff)  \
        {   \
                rtd_pr_lsadc_emerg(fmt, ## args); \
        }   \
}

#define LSADC_INFO(fmt, args...)	rtd_pr_lsadc_info("[Info]" fmt, ## args)
#define LSADC_WARN(fmt, args...)	rtd_pr_lsadc_warn("[Warn]" fmt, ## args)
#define LSADC_ERROR(fmt, args...)	rtd_pr_lsadc_err("[Error]" fmt, ## args)


#endif
