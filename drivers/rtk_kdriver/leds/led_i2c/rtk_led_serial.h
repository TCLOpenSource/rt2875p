#ifndef __RTK_LED_SERIAL_H__
#define __RTK_LED_SERIAL_H__
#include <rtd_log/rtd_module_log.h>
#if IS_ENABLED(CONFIG_REALTEK_PCBMGR)
#include <mach/pcbMgr.h>
#endif
#include <rbus/stb_reg.h>

#ifdef CONFIG_ARCH_RTK2851A
#include <rbus/pinmux_main_reg.h>
#elif defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F) || defined(CONFIG_ARCH_RTK2885P)
#include <rbus/pinmux_main_reg.h>
#include <rtk_kdriver/io.h>
#include <rbus/misc_irtx_reg.h>
#else
#include <rbus/pinmux_reg.h>
#include <rtk_kdriver/io.h>
#include <rbus/misc_irtx_reg.h>
#endif

/* DEFINE */
#undef  TAG_NAME
#define TAG_NAME  "LEDSERIAL"
#ifndef BIT0
#define BIT0                 	0x00000001
#endif
#define UART_RBR_THR_DLL        (ISO_MISC_OFF_UART_U0RBR_THR_DLL_reg)
#define UART_IER_DLH            (ISO_MISC_OFF_UART_U0IER_DLH_reg)
#define UART_IIR_FCR            (ISO_MISC_OFF_UART_U0IIR_FCR_reg)
#define UART_LCR                (ISO_MISC_OFF_UART_U0LCR_reg)
#define UART_CLKMUX             (STB_ST_CLKMUX_reg)
#define UART2_ENABLE_FIFO()     (0x01 << 0 )
#define UART2_TX_FIFO_RESET()   (0x01 << 2 )

#ifdef CONFIG_ARCH_RTK2851A
#define LEDSERIAL_PINMUX        (PINMUX_MAIN_ST_GPIO_ST_CFG_8_reg)
#define LEDSERIAL_GPIO_TYPE     ISO_GPIO    
#define LEDSERIAL_GPIO_INDEX    44
#endif

#ifdef CONFIG_ARCH_RTK6748
#define LEDSERIAL_PINMUX        (PINMUX_ST_GPIO_ST_CFG_7_reg)
#define LEDSERIAL_GPIO_TYPE     ISO_GPIO
#define LEDSERIAL_GPIO_INDEX    39
#endif

#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
#define LEDSERIAL_PINMUX        (PINMUX_MAIN_ST_GPIO_ST_CFG_4_reg)
#define LEDSERIAL_GPIO_TYPE     ISO_GPIO
#define LEDSERIAL_GPIO_INDEX    16
#endif

#if defined(CONFIG_ARCH_RTK2885P)
#define LEDSERIAL_PINMUX        (PINMUX_MAIN_ST_GPIO_ST_CFG_2_reg)
#define LEDSERIAL_GPIO_TYPE     ISO_GPIO
#define LEDSERIAL_GPIO_INDEX    14
#endif

#define LED_COUNT_MAX           8
#define LED_COLOR_COUNT         4

/******************************************************************
*** AB22 uart/localdimming ,L4,L5 Switch on/off
*** AB22 0xB8060208[23:20] 0xF GPIO, 
*** PIN_V19,PIN_U20 == PIN_AB22,PIN_AA23
******************************************************************/
#define PIN_AB22 0xB8060208      //(0xF&0x6)//0xB8060208[23:20]; //localdimming uart
#define PIN_AA23 0xB8060208      //(0xF&0x6)//0xB8060208[31:28];

/******************************************************************
*** rtd_maskl(0xb8060248, ~((0x7)<<0), ((0x1&0x7)<<0)); aa23 6:uart0 0xb8060248
*** PIN_R7,PIN_T7 == PIN_W10,PIN_Y10
*** { rtd_maskl(0xb806024c, ~((0x7)<<28), ((0x1&0x7)<<28)); }; // 7: uart1_rxdi_src1,<I>
******************************************************************/
#define PIN_W10 0xB8060208       //(0xF&0x7)//0xB8060208[7:4];  //debug uart
#define PIN_Y10 0xB8060208       //(0xF&0x7)//0xB8060208[15:12];

#define PINMUX_UART0 0xb8060248  //set aa23 6:uart0
#define PINMUX_UART1 0xb806024C  //set Y10  7:uart1_rxdi_src1

#define LEDSERIAL_WARN(fmt, args...)    rtd_pr_led_serial_warn("[Warn]" fmt, ## args)
#define LEDSERIAL_ERROR(fmt, args...)   rtd_pr_led_serial_err("[Error]" fmt, ## args)

#ifndef CONFIG_ARCH_RTK2851A
#define LED_IRTX_TI_BUF_WDATA_LENGTH_SHIFT           17
#define LED_IRTX_TI_BUF_WDATA_LENGTH_MASK            0xFFFE0000
#define LED_IRTX_TI_BUF_WDATA_LENGTH(x)              (((x) & 0x7FFF) << LED_IRTX_TI_BUF_WDATA_LENGTH_SHIFT)

#define LED_IRTX_TI_BUF_WDATA_LEVEL_SHIFT            16
#define LED_IRTX_TI_BUF_WDATA_LEVEL_MASK             0x00010000
#define LED_IRTX_TI_BUF_WDATA_LEVEL(x)               (((x) & 0x01) << LED_IRTX_TI_BUF_WDATA_LEVEL_SHIFT)

#define LED_IRTX_TI_BUF_IDX_SHIFT                    MISC_IRTX_TI_CTRL_irtx_ti_buf_idx_shift
#define LED_IRTX_TI_BUF_IDX_MASK                     MISC_IRTX_TI_CTRL_irtx_ti_buf_idx_mask
#define LED_IRTX_TI_BUF_IDX(x)                       (((x) & 0xFF) << LED_IRTX_TI_BUF_IDX_SHIFT)

#define LED_IRTX_TI_BUF_WR_SHIFT                     MISC_IRTX_TI_CTRL_irtx_ti_buf_wr_shift
#define LED_IRTX_TI_BUF_WR_MASK                      MISC_IRTX_TI_CTRL_irtx_ti_buf_wr_mask
#define LED_IRTX_TI_BUF_WR(x)                        (((x) & 0x01) << LED_IRTX_TI_BUF_WR_SHIFT)

#define LED_IRTX_LEN_SHIFT                           MISC_IRTX_IRTX_START_irtx_len_shift
#define LED_IRTX_LEN_MASK                            MISC_IRTX_IRTX_START_irtx_len_mask
#define LED_IRTX_LEN(x)                              ((((x) - 1) & 0xFF) << LED_IRTX_LEN_SHIFT)

#define LED_IRTX_START_SHIFT                         MISC_IRTX_IRTX_START_irtx_start_shift
#define LED_IRTX_START_MASK                          MISC_IRTX_IRTX_START_irtx_start_mask
#define LED_IRTX_START(x)                            (((x) & 0x1) << LED_IRTX_START_SHIFT)
#endif

/* STRUCT */
typedef struct {
    unsigned char  inited;
    unsigned int uart_lcr_value;
    unsigned int uart_ier_dlh_value;
    unsigned int uart_rbr_thr_dll_value;
    unsigned int uart_iir_fcr_value;
    unsigned int uart_clkmux_value;
} LED_UART_REG_BAKUP;
typedef struct {
    int count;
    int colors[LED_COUNT_MAX];
} LED_COLOR_INPUT;
typedef struct _RTK_LEDSERIAL {
    bool mapped;
    char nodename[16];
    int index;
    int pinNumber;
    struct device *ledserial_device_st;
} RTK_LEDSERIAL_ATTR;
/* IOCTL */
#define RTK_LED_IOC_MAGIC               'l'
#define RTK_LED_IOC_SET_COLOR           _IOW(RTK_LED_IOC_MAGIC, 1, int)
#define RTK_LED_IOC_DEINIT              _IOW(RTK_LED_IOC_MAGIC, 2, int)
#define RTK_LED_IOC_INIT                _IOW(RTK_LED_IOC_MAGIC, 3, int)
#define RTK_LED_IOC_TEST                _IOW(RTK_LED_IOC_MAGIC, 4, int)
#define RTK_LED_IOC_ENABLE_LED          _IOW(RTK_LED_IOC_MAGIC, 5, int)
#define RTK_LED_IOC_SET_DMIC_ON_OFF     _IOW(RTK_LED_IOC_MAGIC, 6, unsigned int)
#define RTK_LED_IOC_SET_PINMUX          _IOW(RTK_LED_IOC_MAGIC, 8, unsigned int)

#endif
