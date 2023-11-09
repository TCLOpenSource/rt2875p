#ifndef _RTK_EMCU_EXPORT_H_
#define _RTK_EMCU_EXPORT_H_

#if defined(CONFIG_REALTEK_INT_MICOM)
#include <linux/i2c.h>
#include "rtk_gpio.h"
#include "rtk_gpio-dev.h"
#endif

#define SUSPEND_BOOTCODE    0
#define SUSPEND_NORMAL      1
#define SUSPEND_RAM         2
#define SUSPEND_WAKEUP      3

#define IDX_WIFI_POWER          (0x59ULL)
#define IDX_WOW_PIN             (0x58ULL)
#define IDX_REKEY               (0x57ULL)
#define IDX_DDR_POWER           (0x56ULL)

typedef enum {
    WKSOR_UNDEF,    // 0: undefine
    WKSOR_KEYPAD,   // 1: wakeup via keypad
    WKSOR_WUT,      // 2: wakeup via timer
    WKSOR_IRDA,     // 3: wakeup via remote control
    WKSOR_CEC,      // 4: wakeup via CEC
    WKSOR_PPS,      // 5: wakeup via VGA
    WKSOR_WOW,      // 6: wakeup via WOW
    WKSOR_MHL,      // 7: wakeup via HML
    WKSOR_RTC,      // 8: wakeup via RTC
    WKSOR_WOV,      // 9: wakeup via voice
    WKSOR_EWBS,     // 10: wakeup via EWBS
    WKSOR_END,      //end
} WAKE_UP_T;

#define WKSOR_GPIO WKSOR_KEYPAD

#define WKSOR_SUB_KEYPAD    WKSOR_KEYPAD	// 0: for keypad
#define WKSOR_SUB_WOW       (WKSOR_KEYPAD | (IDX_WOW_PIN << 8))	// for WOW gpio pin
#define WKSOR_SUB_REKEY     (WKSOR_KEYPAD | (IDX_REKEY << 8))		// for rekey gpio pin

#define WKSOR_WIFI   WKSOR_SUB_WOW
#define WKSOR_REKEY  WKSOR_SUB_REKEY
#define GET_WAKE_UP_GPIO_SUB(x) (x & 0xffff)

int powerMgr_get_wakeup_source(unsigned int* row, unsigned int* status);
int powerMgr_set_wakeup_source_undef(void);

#endif



