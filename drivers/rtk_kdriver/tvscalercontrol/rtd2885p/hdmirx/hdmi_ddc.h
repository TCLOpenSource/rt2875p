#ifndef __HDMI_DDC_H__
#define __HDMI_DDC_H__
#include <rtk_kdriver/rtk_ddc_lib.h>

extern void lib_hdmiddc_crt_on(unsigned char ch);
extern void lib_hdmiddc_sync_reset(unsigned char port);
extern unsigned char lib_hdmiddc_write(unsigned char port, unsigned char* EDID, unsigned int len);
extern unsigned char lib_hdmiddc_read(unsigned char port, unsigned char *edid_tbl, unsigned int len);
extern void lib_hdmiddc_enable(unsigned char port, unsigned char enable);
extern void lib_hdmiddc_set_sda_delay(unsigned char port, unsigned char delay_sel);
extern void lib_hdmiddc_sda2gpio(unsigned char port);
extern void lib_hdmiddc_gpio2sda(unsigned char port);
extern void lib_hdmiddc_sda_dir_out(unsigned char port, unsigned char dir_o);
extern void lib_hdmiddc_sync_reset(unsigned char port);
extern void lib_hdmiddc_get_ddc_state(unsigned char port, srtk_edid_sts *edid_sts);
extern void lib_hdmiddc_set_edid_address(unsigned char port, unsigned char address);

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
/**
 * @func: Initialize the DDC WAKEUP, usually setting the default ddc wake up address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
void lib_hdmiddc_wakeup_init(unsigned char id);

/**
 * @func: Enable or disable the ddc wakeup function.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
extern int lib_hdmiddc_set_wakeup_enable(unsigned char id, unsigned char enable);

/**
 * @func: Gets the enable status of wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int lib_hdmiddc_get_wakeup_enable(unsigned char id, unsigned char *enable);

#endif // CONFIG_RTK_KDRV_DDC_WAKEUP

#endif //__HDMI_DDC_H__
