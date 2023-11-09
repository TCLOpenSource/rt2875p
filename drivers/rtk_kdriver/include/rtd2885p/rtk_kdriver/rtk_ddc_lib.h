#ifndef __RTK_DDC_LIB_H__
#define __RTK_DDC_LIB_H__

#define RTK_VGA_DDC         0
#define RTK_HDMI_DDC(x)     (1+x)    

typedef struct srtk_edid_sts{
    unsigned char   st_edid_start;
    unsigned char   st_edid_stop;
} srtk_edid_sts;

// for low level API
void rtk_ddc_lib_crt_on(unsigned char ch);

int  rtk_ddc_lib_sync_reset(unsigned char id);

int  rtk_ddc_lib_set_write_debounce(unsigned char id, unsigned char clock);

int  rtk_ddc_lib_write_edid(unsigned char id, unsigned char *edid_tbl, unsigned int len);

int  rtk_ddc_lib_read_edid(unsigned char id, unsigned char *edid_tbl, unsigned int len);

int  rtk_ddc_lib_enable(unsigned char id, unsigned char enable);

int  rtk_ddc_lib_set_sda_delay(unsigned char id, unsigned char delay_sel);

int  rtk_ddc_lib_get_ddc_state(unsigned char id, srtk_edid_sts *edid_sts);

int  rtk_ddc_lib_aux_rx_enable(unsigned char id, int enable);

int rtk_ddc_lib_set_edid_address(unsigned char id, unsigned char  address_num);

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
/**
 * @func: Initialize the DDC WAKEUP, usually setting the default ddc wake up address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
void rtk_ddc_lib_wakeup_init(unsigned char id);

/**
 * @func: Reset all status about ddc wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_wakeup_reset(unsigned char id);

/**
 * @func: Enable or disable the ddc wakeup function.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_enable_wakeup(unsigned char id, unsigned char enable);

/**
 * @func: Gets the enable status of wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_get_ddc_wakeup_enable(unsigned char id, unsigned char *enable);

/**
 * @func: Enable or disable the wakeup of the specified address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] addr: The specified address for wakeup
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_set_ddc_wakeup_addr(unsigned char id, unsigned char addr, unsigned char enable);

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
int rtk_ddc_lib_get_ddc_wakeup_addr(unsigned char id, unsigned char *addr, unsigned char addr_len, unsigned char *addr_cnt);

/**
 * @func: Get the address which has been hitted.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] addr: Which address has been hitted
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_get_ddc_hitted_wakeup_addr(unsigned char id, unsigned char *addr);


/**
 * @func: Clear trigger state.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_clr_ddc_hitted_wakeup_addr(unsigned char id);

int rtk_ddc_lib_set_ddc_wakeup_irq(unsigned char id, unsigned char irq);

#endif // CONFIG_RTK_KDRV_DDC_WAKEUP


// for VGA DDC
#define rtk_vddc_crt_on                             rtk_ddc_lib_crt_on()
#define rtk_vddc_sync_reset()                       rtk_ddc_lib_sync_reset(RTK_VGA_DDC)   
#define rtk_vddc_set_write_debounce(clock)          rtk_ddc_lib_set_write_debounce(RTK_VGA_DDC, clock)
#define rtk_vddc_write_edid(edid_tbl, len)          rtk_ddc_lib_write_edid(RTK_VGA_DDC, edid_tbl, len)
#define rtk_vddc_read_edid(edid_tbl, len)           rtk_ddc_lib_read_edid(RTK_VGA_DDC, edid_tbl, len)
#define rtk_vddc_enable(enable)                     rtk_ddc_lib_enable(RTK_VGA_DDC, enable)
#define rtk_vddc_lib_set_sda_delay(delay_sel)       rtk_ddc_lib_set_sda_delay(RTK_VGA_DDC, delay_sel)
#define rtk_vddc_lib_get_ddc_state(id,edid_sts)     rtk_ddc_lib_get_ddc_state(RTK_HDMI_DDC(id), edid_sts)
#define rtk_vddc_lib_set_edid_address(id,address)   rtk_ddc_lib_set_edid_address(RTK_HDMI_DDC(id), address)

// for HDMI DDC
#define rtk_hddc_crt_on                             rtk_ddc_lib_crt_on()
#define rtk_hddc_sync_reset(id)                     rtk_ddc_lib_sync_reset(RTK_HDMI_DDC(id))   
#define rtk_hddc_set_write_debounce(id,clock)       rtk_ddc_lib_set_write_debounce(RTK_HDMI_DDC(id), clock)
#define rtk_hddc_write_edid(id, edid_tbl, len)      rtk_ddc_lib_write_edid(RTK_HDMI_DDC(id), edid_tbl, len)
#define rtk_hddc_read_edid(id,edid_tbl, len)        rtk_ddc_lib_read_edid(RTK_HDMI_DDC(id), edid_tbl, len)
#define rtk_hddc_enable(id,enable)                  rtk_ddc_lib_enable(RTK_HDMI_DDC(id), enable)
#define rtk_hddc_lib_set_sda_delay(id,delay_sel)    rtk_ddc_lib_set_sda_delay(RTK_HDMI_DDC(id), delay_sel)
#define rtk_hddc_lib_get_ddc_state(id,edid_sts)     rtk_ddc_lib_get_ddc_state(RTK_HDMI_DDC(id), edid_sts)
#define rtk_hddc_lib_set_edid_address(id,address)   rtk_ddc_lib_set_edid_address(RTK_HDMI_DDC(id), address)

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
#define rtk_hddc_wakeup_init(id)                    rtk_ddc_lib_wakeup_init(RTK_HDMI_DDC(id))
#define rtk_hddc_wakeup_reset(id)                   rtk_ddc_lib_wakeup_reset(RTK_HDMI_DDC(id))
#define rtk_hddc_enable_wakeup(id,enable)           rtk_ddc_lib_enable_wakeup(RTK_HDMI_DDC(id), enable)
#define rtk_hddc_get_ddc_wakeup_enable(id,enable)   rtk_ddc_lib_get_ddc_wakeup_enable(RTK_HDMI_DDC(id), enable)
#define rtk_hddc_set_ddc_wakeup_addr(id,addr,enable) rtk_ddc_lib_set_ddc_wakeup_addr(), addr, enable)
#define rtk_hddc_get_ddc_wakeup_addr(id,addr,addr_len,addr_cnt) rtk_ddc_lib_get_ddc_wakeup_addr(RTK_HDMI_DDC(id), addr, addr_len, addr_cnt)
#define rtk_hddc_get_ddc_hitted_wakeup_addr(id,addr) rtk_ddc_lib_get_ddc_hitted_wakeup_addr(RTK_HDMI_DDC(id), addr)
#define rtk_hddc_clr_ddc_hitted_wakeup_addr(id)     rtk_ddc_lib_clr_ddc_hitted_wakeup_addr(RTK_HDMI_DDC(id))
#define rtk_hddc_set_ddc_wakeup_irq(id,irq)         rtk_ddc_lib_set_ddc_wakeup_irq(RTK_HDMI_DDC(id), irq)

#endif // CONFIG_RTK_KDRV_DDC_WAKEUP

#endif  // __RTK_DDC_LIB_H__
