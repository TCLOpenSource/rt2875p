#ifndef __RTK_DDC_PRIV_H__
#define __RTK_DDC_PRIV_H__
#include "rtk_ddc_osal.h"
#include <rtk_kdriver/io.h>

#define CONFIG_DDC_TOOL_DEBUG_MODE      1
#define RTK_DDC_NAME_MAX        24
#define RTK_DDC_MAX_NUM         6

#define RTK_EDID_MAX_LEN    512

typedef enum
{
    DDC_HDMI,
    DDC_MHL,
    DDC_VGA,
    MAX_DDC_TYPE
} ERTK_DDC_TYPE;

typedef struct srtk_edid_t                   srtk_edid;

struct srtk_edid_t{
    int len;
    unsigned char content[RTK_EDID_MAX_LEN];
};

typedef struct srtk_ddc_reg_map_t            srtk_ddc_reg_map;
typedef struct srtk_ddc_config_t             srtk_ddc_config;
typedef struct srtk_ddc_chip_t               srtk_ddc_chip;

struct srtk_ddc_reg_map_t
{
    unsigned long DDC_I2C_CR;
    unsigned long DDC_EDID_CR;
    unsigned long DDC_EDID_IR;
    unsigned long DDC_DDC_SIR;
    unsigned long DDC_DDC_SAP;
    unsigned long DDC_DDC_SCR;
    unsigned long DDC_DDC_FSMS;
    unsigned long DDC_DDC_SPR;
    unsigned long SRST1_RSTN_DDC_RESET_MASK;
    unsigned long CLKEN1_CLKEN_DDC_ENABLE_MSAK;
    unsigned long MISC_ISR;
    unsigned long ISR_DDC_INT_MASK;
    unsigned long DDC_DDC_SSAR;
    spinlock_t  g_ddc_spinlock;
#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
    unsigned long WAKEUP_ADDR_1;
    unsigned long WAKEUP_HDCP;
#endif
};

struct srtk_ddc_chip_t
{
    const int                       id;
    srtk_ddc_reg_map          *reg_remap;
    int                             is_fs_registered;
    char                            name[RTK_DDC_NAME_MAX];
    ERTK_DDC_TYPE                   ddc_type;
    struct class                    class;
};

// lib api
int rtk_ddc_drv_set_read_delay(unsigned char id, unsigned char delay);
int rtk_ddc_drv_set_write_debounce(unsigned char id, unsigned char clock);
int rtk_ddc_drv_enable_external_dev_access(unsigned char id, int enable);
int rtk_ddc_drv_crt_reset(unsigned char id);
int rtk_ddc_drv_read(unsigned char id, srtk_edid* pedid);
int rtk_ddc_drv_write(unsigned char id, srtk_edid* pedid);
int rtk_ddc_get_edid_state(unsigned char id, unsigned char* start_sts, unsigned char* stop_sts);
int rtk_ddc_set_edid_address(unsigned char id, unsigned char  address_num);
int rtk_ddc_drv_set_aux_rx_enable(unsigned char id, int enable);

// sys api
int rtk_ddc_drv_is_enable(unsigned char id);
int rtk_ddc_drv_set_enable(unsigned char id, int enable);
int rtk_ddc_drv_get_write_debounce(unsigned char id);
int rtk_ddc_drv_get_read_delay(unsigned char id);
int rtk_ddc_drv_get_segen(unsigned char id);
int rtk_ddc_drv_set_segen(unsigned char id, int enable);
int rtk_ddc_drv_set_isr(unsigned char id, int enable);
int rtk_ddc_drv_crt_on(unsigned char ch);
int rtk_ddc_drv_segen_on(void);

// interrupt
int rtk_ddc_drv_interrupt(int ddc_irq_t);
int rtk_ddc_drv_free_interrupt(int ddc_irq_t);
int rtk_ddc_drv_isr_on(unsigned char ch);
int rtk_ddc_drv_set_scpu_init(int enable);
int rtk_ddc_drv_set_emcu_init(int enable);

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
void rtk_ddc_drv_wakeup_init(unsigned char ch);

/**
 * @func: Enable or disable the ddc wakeup function.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_enable_ddc_wakeup(unsigned char id, unsigned char enable);

/**
 * @func: Gets the enable status of wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_wakeup_enable(unsigned char id, unsigned char *enable);

/**
 * @func: Get the address which has been hitted.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] addr: Which address has been hitted
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_hitted_wakeup_addr(unsigned char id, unsigned char *addr);

/**
 * @func: Clear trigger state.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_clr_ddc_hitted_wakeup_addr(unsigned char id);

/**
 * @func: Enable or disable the wakeup of the specified address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] addr: The specified address for wakeup
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_set_ddc_wakeup_addr(unsigned char id, unsigned char addr, unsigned char enable);

/**
 * @func: Get all wakeup addresses.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] *addr: The pointer for save wakeup addresses
 * @param[in] addr_len: The length of *addr
 * @param[out] *addr_cnt: how many addresses are enabled
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_drv_get_ddc_wakeup_addr(unsigned char id, unsigned char *addr, unsigned char addr_len, unsigned char *addr_cnt);

int rtk_ddc_drv_reset_ddc_wakeup_addr(unsigned char id);
int rtk_ddc_drv_set_ddc_wakeup_irq(unsigned char id, unsigned char irq);
#endif // CONFIG_RTK_KDRV_DDC_WAKEUP


#endif
