#ifndef MISC_EXPORT_H
#define MISC_EXPORT_H

#include <linux/types.h>

struct misc_dev {
    struct semaphore sem;     /* mutual exclusion semaphore     */
    struct cdev cdev;   /* Char device structure          */
};

/*
 * Ioctl definitions
 */
// #define SUPPORT_DCU2
typedef enum
{
        RHAL_SYS_CMD_SET_SPREAD_SPRECTRUM = 0,
        RHAL_SYS_CMD_GET_SPREAD_SPRECTRUM,
        RHAL_SYS_CMD_SET_DDR_PERIOD,
        RHAL_SYS_CMD_GET_DDR_PERIOD,
        RHAL_SYS_CMD_GET_SUPPORT_MODULE,
} RHAL_SYS_CMD_DDR_SSC_T;

/* SPREAD SPECTRUM ENUMERATIONS HAVE TO BE SAME AS KAPI */
typedef enum
{
        RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR = 0,		// include ddr1
#ifdef SUPPORT_DCU2
        RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR2,                          // include ddr2 only
#endif
        RHAL_SYS_SPREAD_SPECTRUM_MODULE_MAX,
} RHAL_SYS_SPREAD_SPRECTRUM_MODULE_T;

typedef enum
{
        RHAL_SYS_SPREAD_SPECTRUM_OFF = 0,               // 0.00%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_10,            // -0.10%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_20,            // -0.20%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_30,            // -0.30%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_40,            // -0.40%         
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_50,            // -0.50%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_60,            // -0.60%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_70,            // -0.70%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_80,            // -0.80%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_0_90,            // -0.90%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_1_00,            // -1.00%

        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_10,        // +0.1%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_20,        // +0.2%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_30,        // +0.3%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_40,        // +0.4%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_50,        // +0.5%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_60,        // +0.6%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_70,        // +0.7%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_80,        // +0.8%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_90,        // +0.9%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_1_00,        // +1.0%
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_MAX,
} RHAL_SYS_SPREAD_SPECTRUM_RATIO_T;


typedef enum
{
        RHAL_SYS_DDR_PERIOD_30K = 0,
        RHAL_SYS_DDR_PERIOD_33K,
        RHAL_SYS_DDR_PERIOD_60K,
        RHAL_SYS_DDR_PERIOD_66K,
        RHAL_SYS_DDR_PERIOD_MAX,
} RHAL_SYS_DDR_PERIOD_T;

typedef struct
{
        RHAL_SYS_CMD_DDR_SSC_T cmd;
        RHAL_SYS_SPREAD_SPRECTRUM_MODULE_T module;
        RHAL_SYS_SPREAD_SPECTRUM_RATIO_T ratio;
        RHAL_SYS_DDR_PERIOD_T period;
        unsigned char module_support;
} DDR_SSC_PARA;

/* module test get pin*/
typedef struct
{
	unsigned int pin_signal;
	unsigned int pin_scl;
	unsigned int pin_sda;
	unsigned int pin_status0;
	unsigned int pin_status1;
} MODULE_TEST_PIN_T;

/* pinmux enable for uart0 or uart1*/
typedef enum
{
        UART0_PINMUX_ENABLE = 0,
        UART1_PINMUX_ENABLE,
        UART0_UART1_PINMUX_DISABLE,
} UART_PINMUX_ENABLE_T;

/* Use 's' as magic number */
#define MISC_IOC_MAGIC  's'

#define MISC_WAKEUP_ECPU    		_IO(MISC_IOC_MAGIC, 0)
#define MISC_SET_WOL_ENABLE    		_IOW(MISC_IOC_MAGIC, 1,int)
#define MISC_GET_WOL_ENABLE    		_IOR(MISC_IOC_MAGIC, 2,int*)
#define MISC_SET_DEBUG_LOCK    		_IOW(MISC_IOC_MAGIC, 3,int)
#define MISC_SET_DDR_SSC_PARAM    	_IOWR(MISC_IOC_MAGIC, 6,unsigned int *)
#ifdef CONFIG_COMPAT
#define MISC_SET_DDR_SSC_PARAM_COMPAT    	_IOWR(MISC_IOC_MAGIC, 6,unsigned int)
#endif
#define MISC_SET_RTDLOG          _IOW(MISC_IOC_MAGIC, 7, unsigned int)
#define MISC_GET_RTDLOG          _IOR(MISC_IOC_MAGIC, 8, unsigned int*)
//#define MISC_SET_RTDLOG_MODULE          _IOW(MISC_IOC_MAGIC, 9, unsigned int)
//#define MISC_GET_RTDLOG_MODULE          _IOR(MISC_IOC_MAGIC, 10, unsigned int*)
#define MISC_SET_WARM_WOL_ENABLE    		_IOW(MISC_IOC_MAGIC, 11,int)
#define MISC_GET_WARM_WOL_ENABLE    		_IOR(MISC_IOC_MAGIC, 12,int*)
#define MISC_QSM_OFF                        _IO(MISC_IOC_MAGIC, 13)

#if IS_ENABLED(CONFIG_RTK_KDRV_WATCHDOG)
#define MISC_SET_WATCHDOG_ENABLE  _IOW(MISC_IOC_MAGIC, 14, int)
#endif

#define MISC_GET_MODULE_TEST_PIN          _IOR(MISC_IOC_MAGIC, 15, MODULE_TEST_PIN_T *)

/*magic number for uart switch*/
#define MISC_UART_PINMUX_SWITCH                   _IOW(MISC_IOC_MAGIC, 17,int)
#define MISC_UART_GET_UART0_ENABLE                _IOR(MISC_IOC_MAGIC, 18,int)
#define MISC_UART_GET_UART1_ENABLE                _IOR(MISC_IOC_MAGIC, 19,int)

#endif /*#ENDIF MISC_EXPORT_H*/
