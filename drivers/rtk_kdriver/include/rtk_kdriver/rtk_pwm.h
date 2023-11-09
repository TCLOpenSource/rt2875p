#ifndef __RTK_PWM_CONFIG_RTD284x_H__
#define __RTK_PWM_CONFIG_RTD284x_H__

#ifndef BUILD_QUICK_SHOW
#include <linux/interrupt.h>

#else
#include <no_os/spinlock.h>
#include <no_os/compiler.h>
#endif /*BUILD_QUICK_SHOW*/

typedef unsigned long long          UINT64;     /* !< for unsigned long long (64 bits) */
typedef long long                   INT64;      /* !< for signed long long (64 bits) */

#define DUTY_RATIO_BIT9 (1 << 9) //100%
#define DUTY_RATIO_BIT8 (1 << 8) //50%
#define DUTY_RATIO_BIT7 (1 << 7) //25%
#define DUTY_RATIO_BIT6 (1 << 6) //12.5%
#define DUTY_RATIO_BIT5 (1 << 5) //6.25%
#define DUTY_RATIO_BIT4 (1 << 4) //3.125%
#define DUTY_RATIO_BIT3 (1 << 3) //1.5625%
#define DUTY_RATIO_BIT2 (1 << 2) //0.78125%
#define DUTY_RATIO_BIT1 (1 << 1) //0.390625%
#define DUTY_RATIO_BIT0 (1 << 0) //0.1953125%

//m_index : PWM ID  b[7:5] = type b[4:0] index
#define PWM_ID(type, id)    (((type  & 0x7)<< 5) | (id & 0x1F))
#define PWM_TYPE(id)        ((id >> 5) & 0x7)
#define PWM_IDX(id)         (id & 0x1F)

#define MAX_PWM_COUNT 10
#define PCB_ENUM_NAME_LEN_MAX	64

#define PWM_OK                0
#define PWM_PCB_ENUM_ERROR    1
#define PWM_DUTY_INVALID      2
#define PWM_FREQEUNCY_INVALID 3
#define PWM_POLARITY_INVALID  4
#define PWM_VSYNC_INVALID     5
#define PWM_OUTPUT_INVALID    6
#define PWM_NULL_POINTER      7
#define PWM_INIT_FAILED       8
#define PWM_PARAM_ID_INVALID  9
#define PWM_MODE_INVALID      10
#define PWM_VALUE_INVALID     11

typedef enum
{
    PWM_NON_FORCE = 0,
    PWM_FORCE_LOW = 1,
    PWM_FORCE_HIGH = 2,
} RTK_PWM_FORCE_MODe;

typedef enum
{
    PWM0_DB0 = 0,
    PWM1_DB0,
    PWM2_DB0,
    PWM3_DB0,
    PWM4_DB0,
    PWM5_DB0,
    PWM6_DB0,
    PWM7_DB0,
    PWM_DB0_MAX,
} RTK_PWM_DB0_GROUP;

typedef enum
{
    MISC_PWM0 = 0,
    MISC_PWM1,
    MISC_PWM2,
    MISC_PWM3,
    MISC_PWM4,
    MISC_PWM5,
    MISC_PWM6,
    MISC_PWM7,
    MISC_PWM_MAX,
} RTK_MSIC_PWM_GROUP;

typedef enum{
    PWM_OUTPUT_DISABLE   = 0,
    PWM_OUTPUT_ENABLE    = 1,
}RTK_PWM_OUTPUT;

typedef struct linux_pcb_pwm_enum_t {
    char name[PCB_ENUM_NAME_LEN_MAX];
    unsigned long long value;
}RTK_PCB_ENUM;

typedef enum{
    PWM_DRIVING_LED_CURRENT     = 0,    /* PWM1(global dimming )+ PWM2(led current) */
    PWM_DRIVING_2CH_PHASE_SAME  = 1,    /* PWM1(global dimming) + PWM2(global dimming) : same phase */
    PWM_DRIVING_2CH_PHASE_DIFF  = 2,    /* PWM1(global dimming) + PWM2(global dimming) : diffrent phase */
    PWM_DRIVING_MODE_MAX        = 3
}PWM_DRIVING_MODE_T;

typedef enum{
    PWM_MODE   = 0,
    GPIO_MODE  = 1,
}PIN_MODE_T;

typedef enum {
    PWM_MISC = 0,
    PWM_ISO = 1,
} PWM_TYPE;


typedef enum {
    CRT_PWM_ISO = 0,
    CRT_PWM_MISC = 1,
} CRT_PWM_TYPE;

typedef enum {
    PWM_PARAM_POLARITY = 0,
    PWM_PARAM_FREQUENCY = 1,
    PWM_PARAM_DUTY = 2,
    PWM_PARAM_VSYNC_RESET = 3,
    PWM_PARAM_OUTPUT = 4,
    PWM_PARAM_VSYNC_DELAY = 5,
    PWM_PARAM_VSYNC_INTERVAL = 6,
    PWM_PARAM_MPLUS = 7,
    PWM_PARAM_CYCLE_MAX = 8,
    PWM_PARAM_FORCE_MODE = 9,
    PWM_PARAM_TOTALCNT = 10,
} PWM_PARAMETER_ID;

typedef enum {
    PWM_SUCCESS = 0,
    PWM_FAIL = -1,
} PWM_RET;

typedef enum {
    PWM_CRYSTAL = 0,
    PWM_PLL     = 1,
} PWM_CLK_SRC;

typedef enum {
    CLK_27MHZ = 0,
    CLK_196MHZ= 1,
    D_CLK_DIV4= 2,
    CLK_147MHZ= 3,
} PWM_CLK_SEL;

typedef enum {
    ISO_CLK_27MHZ = 0,
    ISO_CLK_147MHZ= 1,
} ISO_PWM_CLK_SEL;

typedef enum {
    CLK_27MHZ_8BIT     = 0,
    CLK_27MHZ_12BIT    = 1,
    CLK_196MHZ_8BIT    = 2,
    CLK_196MHZ_12BIT   = 3,
    CLK_DIV4_8BIT      = 4,
    CLK_DIV4_12BIT     = 5,
    CLK_147MHZ_8BIT      = 6,
    CLK_147MHZ_12BIT     = 7,
} PWM_FREQ_RANGE;

typedef enum{
    PWM_ALIGN_CLOSED     = 0,
    PWM_ALIGN_SW_AUTO    = 1,
    PWM_ALIGN_HW_AUTO    = 2,
    PWM_ALIGN_ILLEAGLE   = 3,
}PWM_ALIGN_MODE;

//For PWM_CTRL Register
typedef enum
{
    PWM_VSYNC_DISABLE = 0,
    PWM_VSYNC_ENABLE = 1,
} RTK_PWM_VSYNC_EN;

typedef enum
{
    PWM_DVS_MUX_FORM_GOLBAL = 0,
    PWM_DVS_MUX_FORM_DVS = 1,
} RTK_PWM_DVS_MUX_EN;

//For PWM_TRACK_SET Register
typedef enum
{
    HW_TRACK_INDEX0 = 0,
    HW_TRACK_INDEX1 = 1,
} RTK_PWM_TRACK_INDEX;

//For PWM_ERR_STATUS Register
typedef enum
{
    PWM_VS_STABLE = 0,
    PWM_VS_UNSTABLE = 1,
} RTK_PWM_VS_UNSTABLE;

typedef enum
{
    HW_TRACK_INT_DISABLE = 0,
    HW_TRACK_INT_ENABLE = 1,
} RTK_PWM_TRACK_INT;

typedef enum
{
    HW_CONTINUE = 0,
    HW_ONE_TIME = 1,
    SW_ONE_TIME = 2,
} RTK_PWM_TRACK_MODE;

typedef enum
{
    HW_TRACK_START_DONE = 0,
    HW_TRACK_START = 1,
} RTK_PWM_TRACK_START;

typedef enum
{
    HW_TRACK_STOP_DONE = 0,
    HW_TRACK_STOP = 1,
} RTK_PWM_TRACK_STOP;

typedef enum
{
    HW_TRACK_DISABLE = 0,
    HW_TRACK_ENABLE = 1,
} RTK_PWM_TRACK_EN;

//For PWM_TRACK_FSM Register
typedef enum
{   
    HW_TRACK_PHASE_INT_DISABLE= 0,
    HW_TRACK_PHASE_INT_ENABLE = 1,
} RTK_PWM_PHASE_INT;

typedef enum
{
    HW_TRACK_FREQ_INT_DISABLE= 0,
    HW_TRACK_FREQ_INT_ENABLE = 1,
} RTK_PWM_FREQ_INT;

typedef enum
{
    HW_TRACK_PHASE_NOT_FINISH = 0,
    HW_TRACK_PHASE_FINISH = 1,
} RTK_PWM_PHASE_FINISH;
    
typedef enum
{
    HW_TRACK_FREQ_NOT_FINISH = 0,
    HW_TRACK_FREQ_FINISH = 1,
} RTK_PWM_FREQ_FINISH;
    
typedef enum
{
    HW_TRACK_FSM_IDLE = 0,
    HW_TRACK_FSM_FREQ = 1, 
    HW_TRACK_FSM_PHASE = 2,
    HW_TRACK_FSM_FINISH = 3,
} RTK_PWM_TRACK_FSM;

//For PWM_HW_PREDICT Register
typedef enum
{
    HW_PREDICT_DISABLE = 0,
    HW_PREDICT_ENABLE = 1,
} RTK_PWM_HW_PREDICT_EN;

typedef enum
{
    HW_VS_NO_UPDATE = 0,
    HW_VS_UPDATE = 1,
    SW_VS_UPDATE = 2,
} RTK_PWM_HW_NXT_VS_UPDATE;

typedef enum
{
    IDLE     = 0,
    TRACKING = 1,
    PHASE    = 2,
} RTK_PWM_VSYNC_MODE;

typedef struct {
    unsigned long min_freq;
    unsigned long max_freq;
} RTK_PWM_FREQ_RANGE;

typedef struct {
    int m_index;            /* pwm index */
    unsigned int id;       /* parameter id*/
    unsigned int val;      /* parameter val*/
} RTK_PWM_PARAM_T;

typedef struct {
   unsigned int id;       /* parameter id*/
   unsigned int val;      /* parameter val*/
   char pcbname[64];
} RTK_PWM_PARAM_EX_T;

typedef struct {
    unsigned int m_polarity;
    unsigned int m_run;
    unsigned int m_duty;
    unsigned int m_freq;
    unsigned int m_vsync;
    char pcbname[64];
} RTK_PWM_INFO_EX_T;

typedef struct {
    unsigned int m_val;
    unsigned int m_mode;
    char pcbname[64];
} RTK_PWM_MODE_INFO_T;

typedef struct RTK_PWM_INFO {
    int m_index;
    int m_run;
    int m_duty;
    int m_freq;
    int m_adpt_en;
    int m_freq48n;
    int m_freq50n;
    int m_freq60n;
    int m_vsync;
    int m_pos;
} RTK_PWM_INFO_T;

extern int rtk_pwm_backlight_enable(int enable);
extern int rtk_pwm_backlight_set_duty( int duty );
//void rtk_pwm_chip_PcbMgrValue_set(int ricindex, unsigned long long value);
int rtk_pwm_set_scaler_source( int h_total, int v_total, int ifold,unsigned char bEnableDB);
int rtk_pwm_set_tracking(void);

static const RTK_PWM_FREQ_RANGE pwm_freq_range[8] =
{
    //min_freq,max_freq
    {3,105600}, //CLK_27MHZ_8BIT(3.2Hz~105.6KHz)
    {1,13500000},//CLK_27MHZ_12BIT(0.2Hz~13.5MHz)
    {23,765000},//CLK_196MHZ_8BIT(23.2Hz~765KHz)
    {2,98000000},//CLK_196MHZ_12BIT(1.46Hz~98Hz)
    {70,580000},//CLK_DIV4_8BIT(70.8Hz~580KHz)
    {4,3600000},//CLK_DIV4_12BIT(4.5Hz~36KHz)
    {2,57400},//CLK_147MHZ_8BIT(1.7Hz~57.4KHz)
    {1,735000},//CLK_147MHZ_12BIT(0.1Hz~735KHz)
};

#define PWM_IOC_MAGIC  'p'
#define PWM_IOCR_SET _IOR(PWM_IOC_MAGIC,  0, RTK_PWM_INFO_T)
#define PWM_BACKLIGHT_DUTY _IOW(PWM_IOC_MAGIC,  1, int)
#define PWM_PARAM_SET _IOW(PWM_IOC_MAGIC,  2, RTK_PWM_PARAM_T)
#define PWM_PARAM_GET _IOR(PWM_IOC_MAGIC,  3, RTK_PWM_PARAM_T)
#define PWM_IOCR_SET_EX _IOW(PWM_IOC_MAGIC,  4, RTK_PWM_INFO_EX_T)
#define PWM_IOCR_GET_EX _IOR(PWM_IOC_MAGIC,  5, RTK_PWM_INFO_EX_T)
#define PWM_PARAM_SET_EX _IOW(PWM_IOC_MAGIC,  6, RTK_PWM_PARAM_EX_T)
#define PWM_PARAM_GET_EX _IOR(PWM_IOC_MAGIC,  7, RTK_PWM_PARAM_EX_T)
#define PWM_MODE_SET _IOW(PWM_IOC_MAGIC,  10, RTK_PWM_MODE_INFO_T)
#define PWM_MODE_GET _IOR(PWM_IOC_MAGIC,  11, RTK_PWM_MODE_INFO_T)

#define N_MAX  4096
#define M_MAX  3
#define TOTAL_MAX 4096
#define CONFIG_CPU_XTAL_FREQ      27000000
#define CONFIG_PLL_FREQ           196000000
#define CONFIG_ISO_PLL_FREQ       147000000

#define CONFIG_VSYNC_FREQ         (60)
#define CONFIG_CPU_XTAL_NS        (37)    /*1/27000000*/
#define CONFIG_PLL_NS             (5)     /*1/196000000*/
#define CONFIG_ISO_PLL_NS         (7)     /*1/147000000*/

#define CONFIG_CPU_XTAL_NS_27MHZ  (37)
#define CONFIG_CPU_PLL_NS_196MHZ  (5)
#define CONFIG_CPU_PLL_NS_147MHZ  (7)

#define MAX_PWM_FREQ (105600)
#define MAX_PWM_FREQ_196MHZ (765000)

#define HZ_48 48
#define HZ_50 50
#define HZ_60 60

#define GET_PWM_PINMUX_GPIO_NUM(x)        ((unsigned int)  (((x)) & 0x1FF))
#define GET_PWM_PINMUX_GPIO_INIT_VALUE(x) ((unsigned int)  (((x) >> 9) & 0x1))
#define GET_PWM_PINMUX_GPIO_PINTYPE(x)    ((unsigned int)  (((x) >> 10) & 0xF))
#define GET_PWM_PINMUX_GPIOVALUE(x)       ((unsigned char) (((x) >> 14) & 0xF))
#define GET_PWM_PINMUX_PWMVALUE(x)        ((unsigned char) (((x) >> 18) & 0xF))
#define GET_PWM_PINMUX_REG_ENDBIT(x)      ((unsigned char) (((x) >> 22) & 0x1F))
#define GET_PWM_PINMUX_REG_STARTBIT(x)    ((unsigned char) (((x) >> 27) & 0x1F))
#define GET_PWM_PINMUX_ADDR(x)            ((unsigned int)  (((x) >> 32) & 0xFFFFFFFF))
#define INVALID_GPIO_GID   0xFFFF
#define MAX_PWM_DUTY  255
#define MIN_PWM_DUTY  0
struct rtk_pwm_pin_reg_t {
    unsigned long  addr;
    unsigned char  mask_h;
    unsigned char  mask_l;
    unsigned char  pwmValue;
    unsigned char  gpioValue;
};
typedef struct rtk_pwm_pin_reg_t rtk_pwm_pin_reg;


//int rtk_pwm_backlight_set_duty( int duty );

#ifndef BUILD_QUICK_SHOW
/** Interrupt work schedule Function **/
void rtk_pwm_work(struct work_struct *work);
irqreturn_t rtk_pwm_isr(int this_irq, void *dev_id);
#endif /*BUILD_QUICK_SHOW*/

#endif
