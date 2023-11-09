#ifndef __RTK_GPIO_H__
#define __RTK_GPIO_H__
#include <rtd_log/rtd_module_log.h>

#include <mach/pcbMgr.h>
#include <rtk_gpio-debug.h>


//////////////////////////////////////////////////////////////////////////////////////////////
// Chipset informations
//////////////////////////////////////////////////////////////////////////////////////////////
#if  defined(CONFIG_ARCH_RTK6702)
#define MIS_GPIO_CNT                    97
#define ISO_GPIO_CNT                    90
#define GPI_MIS_CNT                     32
#define GPI_ISO_CNT                     0
#define GPO_MIS_CNT                     0
#define GPO_ISO_CNT                     0
#define MIO_GPIO_CNT                    99
#define RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#elif defined(CONFIG_ARCH_RTK6748)
#define MIS_GPIO_CNT                    163
#define ISO_GPIO_CNT                    61
#define GPI_MIS_CNT                     32
#define GPI_ISO_CNT                     0
#define GPO_MIS_CNT                     0
#define GPO_ISO_CNT                     0
#define RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#elif defined(CONFIG_ARCH_RTK2851A)
#define MIS_GPIO_CNT                    181
#define ISO_GPIO_CNT                    54
#define GPI_MIS_CNT                     24
#define GPI_ISO_CNT                     0
#define GPO_MIS_CNT                     0
#define GPO_ISO_CNT                     0
#define RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#elif defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
#define MIS_GPIO_CNT                    117
#define ISO_GPIO_CNT                    50
#define GPI_MIS_CNT                     24
#define GPI_ISO_CNT                     0
#define GPO_MIS_CNT                     0
#define GPO_ISO_CNT                     0
#define MIO_GPIO_CNT                    98
#define RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#elif defined(CONFIG_ARCH_RTK2885P)
#define MIS_GPIO_CNT                    107
#define ISO_GPIO_CNT                    50
#define GPI_MIS_CNT                     32
#define GPI_ISO_CNT                     0
#define GPO_MIS_CNT                     0
#define GPO_ISO_CNT                     0
#define MIO_GPIO_CNT                    99
#define ISO_GPIO_MIO_CNT                19
#define RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
#endif

#define MAX_PCB_GPIO_COUNT 64

#define GPIO_SUCCESS                 0
#define GPIO_FAIL                   -1
#define GPIO_PIN_UNDEFINED          -2
#define GPIO_TYPE_WRONG             -3


#ifdef  BUILD_QUICK_SHOW

#undef RTK_GPIO_TRACE
#undef RTK_GPIO_INFO

#undef RTK_GPIO_WARNING
#undef GPIO_WARNING_DEBUG_LEVEL
#undef RTK_GPIO_ERROR

#define RTK_GPIO_TRACE(fmt, args...)
#define RTK_GPIO_INFO(fmt, args...)

#define RTK_GPIO_WARNING(fmt, args...)
#define GPIO_WARNING_DEBUG_LEVEL(fmt, args...)
#define RTK_GPIO_ERROR(fmt, args...)                printf2(fmt, ## args)
#endif


//////////////////////////////////////////////////////////////////////////////////////////////
// Data Structures
//////////////////////////////////////////////////////////////////////////////////////////////
#define RTK_GPIO_ID         unsigned int

typedef struct
{
    char name[32];
    char trigger_type;
    char irq_flag;
    void *dev_id;
    void (*handler) (RTK_GPIO_ID id, unsigned char assert, void *data);   /* assert call back */
} RTK_GPIO_IRQ;

typedef enum
{
    ISO_GPIO = 0,
    MIS_GPIO,
    ISO_GPI,
    ISO_GPO,
    MIS_GPI,
    MIS_GPO,
    MIO_GPIO,
    ISO_GPIO_MIO,
} RTK_GPIO_GROUP;

#ifdef BUILD_QUICK_SHOW
typedef RTK_GPIO_GROUP RT_GPIO_TYPE;
#endif

typedef enum
{
    RTK_GPIO_DEBOUNCE_37ns = 0,
    RTK_GPIO_DEBOUNCE_1us,
    RTK_GPIO_DEBOUNCE_10us,
    RTK_GPIO_DEBOUNCE_100us,
    RTK_GPIO_DEBOUNCE_1ms,
    RTK_GPIO_DEBOUNCE_10ms,
    RTK_GPIO_DEBOUNCE_20ms,
    RTK_GPIO_DEBOUNCE_30ms,
} RTK_GPIO_DEBOUNCE;

typedef enum
{
    GPIO_LOW_ACTIVE      = 0,
    GPIO_HIGH_ACTIVE     = 1,
} RTK_GPIO_POLARITY;

typedef enum
{
    GPIO_RISING_EDGE     = 0,
    GPIO_FALLING_EDGE    = 1,
    GPIO_BOTH_EDGE       = 2,
} RTK_GPIO_TRIGGER;

typedef enum
{
    GPIO_ASSERT_INTERRUPT       = 0,
    GPIO_DISASSERT_INTERRUPT    = 1,
    GPIO_BOTH_INTERRUPT         = 2,
} RTK_GPIO_GLOBAL_INTSEL;

typedef struct
{
    unsigned int direction;
    unsigned int outputValue;
} RTK_GPIO_CONFIG;

typedef struct {
    char pin_name[32];
    unsigned char valid;
    unsigned char pin_type;
    unsigned char pin_index;
    unsigned char input_type;
    unsigned char invert;
    unsigned char init_value;
    unsigned int  gid;
    unsigned char gpio_type;
} RTK_GPIO_PCBINFO_T;


typedef struct {
    RTK_GPIO_PCBINFO_T pcblist[MAX_PCB_GPIO_COUNT];
    int pcbcnt;
} RTK_GPIO_PCB_T;

//////////////////////////////////////////////////////////////////////////////////////////////
// APIs
//////////////////////////////////////////////////////////////////////////////////////////////

#define rtk_gpio_id(group, idx)     (((group & 0x7) << 8) | (idx & 0xFF))
#define gpio_group(id)              ((id >> 8) & 0x7)
#define gpio_idx(id)                (id & 0xFF)

static inline char* gpio_type(int group)
{
    switch(group)
    {
    case ISO_GPIO:  return "ISO_GPIO";
    case MIS_GPIO:  return "MIS_GPIO";
    case ISO_GPI:   return "GPI_ISO";
    case ISO_GPO:   return "GPO_ISO";
    case MIS_GPI:   return "GPI_MIS";
    case MIS_GPO:   return "GPO_MIS";
    case MIO_GPIO:  return "MIO_GPIO";
    case ISO_GPIO_MIO:  return "ISO_GPIO_MIO";
    default:        return "unknow_GPIO";
    }
}

int rtk_gpio_set_dir(RTK_GPIO_ID gid, unsigned char out);

int rtk_gpio_get_dir(RTK_GPIO_ID gid);

int rtk_gpio_set_debounce(RTK_GPIO_ID gid, unsigned char val);

int rtk_gpio_input(RTK_GPIO_ID gid);

int rtk_gpio_output(RTK_GPIO_ID gid, unsigned char val);

int rtk_gpio_output_get(RTK_GPIO_ID gid );

int rtk_gpio_set_irq_polarity(RTK_GPIO_ID gid, unsigned char positive);

int rtk_gpio_set_irq_trigger_type(RTK_GPIO_ID gid, RTK_GPIO_TRIGGER type);

int rtk_gpio_set_irq_enable(RTK_GPIO_ID gid, unsigned char on);

int rtk_gpio_set_dis_irq_enable(RTK_GPIO_ID gid, unsigned char On);

int rtk_gpio_request_irq(RTK_GPIO_ID gid, void (*handler) (RTK_GPIO_ID gid, unsigned char assert, void *dev_id), char *name, void *dev_id);

void rtk_gpio_free_irq(RTK_GPIO_ID gid, void *dev_id);

int rtk_gpio_clear_isr(RTK_GPIO_ID gid);

int rtk_gpio_chk_irq_enable(RTK_GPIO_ID gid);

int rtk_gpio_chk_dis_irq_enable(RTK_GPIO_ID gid);

void rtk_gpio_lg_hal_int_isr(RTK_GPIO_ID gid, unsigned char assert, void* dev_id);

int rtk_gpio_get_irq_flag( RTK_GPIO_ID gid, unsigned char positive );

#ifdef CONFIG_RTK_KDRV_GPIO_WDT_DEBUG_LOG
void rtk_gpio_micom(void);
#endif

void rtk_gpio_rtk_int_isr(RTK_GPIO_ID gid, unsigned char assert, void* dev_id);

int rtk_gpio_get_irq_flag_ex( RTK_GPIO_ID gid, unsigned char positive );

void rtk_gpio_init_pcb_pin(void);

int rtk_gpio_get_pcb_info(char* pinName,RTK_GPIO_PCBINFO_T*pInfo);

void rtk_gpio_get_all_pinInfo(RTK_GPIO_PCBINFO_T* pinInfolist,int *count);

void rtk_gpio_syncInfo_toDB(void);

RTK_GPIO_ID rtk_gpio_get_gid_by_pintype(PCB_PIN_TYPE_T pin_type, unsigned char pin_index);

int rtk_gpio_scpu_iso_irq_unmask(RTK_GPIO_ID gid);


#endif /*__RTK_GPIO_H__ */
