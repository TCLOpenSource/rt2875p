#include "rtk_ddc_priv.h"
#include "rtk_ddc_dbg.h"
#include <rtk_kdriver/rtk_ddc_lib.h>

#define SUPPORT_EDID_384B_512B    1
#define SUPPORT_EDID_Interrupt    1

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_enable
 *
 * Desc : enable a ddc hw
 *
 * Para : id       : DDC port id
 *        enable   : 0: disabled, others : enabled
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_enable(unsigned char id, unsigned char enable)
{
    int ret  = 0;

    ret = rtk_ddc_drv_enable_external_dev_access(id, enable);

    if (ret)
        RTK_DDC_ERR("fail to enable ddc %d for external device access\n", id);

    return ret;
}

EXPORT_SYMBOL(rtk_ddc_lib_enable);


/*-------------------------------------------------------
 * Func : rtk_ddc_lib_sync_reset
 *
 * Desc : reset a specified ddc HW
 *
 * Para : id : DDC port id
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_sync_reset(unsigned char id)
{
    int sg_status = 0;
    sg_status = rtk_ddc_drv_get_segen(id);
    rtk_ddc_drv_crt_reset(id);

    #if SUPPORT_EDID_384B_512B
    rtk_ddc_drv_set_segen(id, sg_status);
    #endif

    #if SUPPORT_EDID_Interrupt
    rtk_ddc_drv_set_isr(id, sg_status);
    #endif

    return 0;
}

EXPORT_SYMBOL(rtk_ddc_lib_sync_reset);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_crt_on
 *
 * Desc : enable clock of ddc
 *
 * Para : N/A
 *
 * Retn : N/A
 -------------------------------------------------------*/
void rtk_ddc_lib_crt_on(unsigned char ch)
{
    rtk_ddc_drv_crt_on(ch);

    #if SUPPORT_EDID_Interrupt
    rtk_ddc_drv_isr_on(ch);
    #endif
}

EXPORT_SYMBOL(rtk_ddc_lib_crt_on);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_set_sda_delay
 *
 * Desc : set delay time of ddc
 *
 * Para : id : DDC port id
 *        delay : delay time
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_set_sda_delay(unsigned char id, unsigned char delay)
{
    int ret  = 0;

    if(delay > 7)
    {
        RTK_DDC_ERR("%s , port[%d] invalid delay = %d \n" , __func__, id ,delay );
        return -1;
    }

    ret = rtk_ddc_drv_set_read_delay(id, delay);

    if(ret)
    {
        RTK_DDC_ERR("%s fail to set DDC_%d read delay value = %d \n", __func__ , id , delay );
        return -1;
    }

    return ret;
}

EXPORT_SYMBOL(rtk_ddc_lib_set_sda_delay);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_set_write_debounce
 *
 * Desc : set write debounce of a ddc driver
 *
 * Para : id : DDC port id
 *        clock : debounde time
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_set_write_debounce(
    unsigned char           id,
    unsigned char           clock
    )
{
    if( clock <= 0 || clock > 3)
    {
        RTK_DDC_ERR("%s , Port[%d] invalid clock = %d \n" , __func__ , id , clock );
        return -1;
    }

    rtk_ddc_drv_set_write_debounce(id, (int)clock);

    return 0;
}


EXPORT_SYMBOL(rtk_ddc_lib_set_write_debounce);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_write_edid
 *
 * Desc : write edid table to specified ddc
 *
 * Para : id       : DDC port id
 *        edid_tbl : edid table to be written
 *        len      : size of edid table
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_write_edid(
    unsigned char           id,
    unsigned char*          edid_tbl,
    unsigned int            len
    )
{
    int ret  = 0;
    srtk_edid edid;

    RTK_DDC_INFO("%s write DDC len %d \n", __func__ , len);

    if (NULL == edid_tbl)
    {
        RTK_DDC_ERR("%s invalid parameters, when read edid by rtk ddc lib\n" ,  __func__ );
        return -EINVAL;
    }

    // initial SRAM to all 0
    edid.len = RTK_EDID_MAX_LEN;
    memset(edid.content, 0, RTK_EDID_MAX_LEN);
    ret = rtk_ddc_drv_write(id, &edid);

    if(len > 256)
    {
        rtk_ddc_drv_set_segen(id, 1);
        rtk_ddc_drv_set_isr(id, 1);
    }
    else
    {
        rtk_ddc_drv_set_segen(id, 0);
        rtk_ddc_drv_set_isr(id, 0);
    }

    // write EDID
    edid.len = len;

    memcpy(edid.content, edid_tbl, len);

    ret = rtk_ddc_drv_write(id, &edid);

    if(ret)
    {
        RTK_DDC_ERR("%s fail to write DDC %d edid\n", __func__ , id);
        return -1;
    }

    return ret;
}

EXPORT_SYMBOL(rtk_ddc_lib_write_edid);


/*-------------------------------------------------------
 * Func : rtk_ddc_lib_read_edid
 *
 * Desc : read edid table from specified ddc
 *
 * Para : id       : DDC port id
 *        edid_tbl : edid output buffer
 *        len      : size of edid buffer
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_read_edid(
    unsigned char           id,
    unsigned char*          edid_tbl,
    unsigned int            len
    )
{
    int ret  = 0;
    srtk_edid edid;

    if (edid_tbl == NULL)
    {
        RTK_DDC_ERR("%s invalid parameters, when read edid by rtk ddc lib\n" , __func__ );
        return -EINVAL;
    }

    RTK_DDC_INFO("%s read DDC len %d \n", __func__ , len);

    edid.len = len;
    ret = rtk_ddc_drv_read(id, &edid);

    if (ret)
    {
        RTK_DDC_ERR("%s fail to read edid of ddc %d\n", __func__ , id);
        return ret;
    }

    memcpy(edid_tbl, edid.content, len);

    return 0;
}
EXPORT_SYMBOL(rtk_ddc_lib_read_edid);
/*-------------------------------------------------------
 * Func : rtk_ddc_lib_get_sram_edid_state
 *
 * Desc : Get EDID state whether the external device read the TV EDID.
 *
 * Para : id       : DDC port id
 *        edid_sts : EDID output state (bit 0 : stop_status, bit 1 : start_status)
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_get_ddc_state(
    unsigned char id, 
    srtk_edid_sts *edid_sts)
{
    int ret  = 0;
    unsigned char edid_start_sts = 0;
    unsigned char edid_stop_sts = 0;
    
    ret = rtk_ddc_get_edid_state(id, &edid_start_sts, &edid_stop_sts);
    edid_sts->st_edid_start = edid_start_sts;
    edid_sts->st_edid_stop = edid_stop_sts;

    return ret;
}
EXPORT_SYMBOL(rtk_ddc_lib_get_ddc_state);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_get_sram_edid_state
 *
 * Desc : Get EDID state whether the external device read the TV EDID.
 *
 * Para : id       : DDC port id
 *        edid_sts : EDID output state (bit 0 : stop_status, bit 1 : start_status)
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_set_edid_address(
    unsigned char id, 
    unsigned char  address_num)
{
    int ret  = 0;
    ret = rtk_ddc_set_edid_address(id, address_num);
    return ret;
}
EXPORT_SYMBOL(rtk_ddc_lib_set_edid_address);

/*-------------------------------------------------------
 * Func : rtk_ddc_lib_aux_rx_enable
 *
 * Desc : Enable i2c over dprx aux.
 *
 * Para : id       : DDC port id
 *        enable   : 0: disabled, others : enabled
 *
 * Retn : 0 : success, others : failed
 -------------------------------------------------------*/
int rtk_ddc_lib_aux_rx_enable(
    unsigned char       id, 
    int                 enable)
{
    int ret  = 0;

    ret = rtk_ddc_drv_set_aux_rx_enable(id, enable);

    return ret;
}
EXPORT_SYMBOL(rtk_ddc_lib_aux_rx_enable);

#ifdef CONFIG_RTK_KDRV_DDC_WAKEUP
/**
 * @func: Initialize the DDC WAKEUP, usually setting the default ddc wake up address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
void rtk_ddc_lib_wakeup_init(unsigned char id)
{
    return rtk_ddc_drv_wakeup_init(id);
}
EXPORT_SYMBOL(rtk_ddc_lib_wakeup_init);

/**
 * @func: Reset all status about ddc wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_wakeup_reset(unsigned char id)
{
    // Disable DDC wake up
    if (!rtk_ddc_drv_enable_ddc_wakeup(id, 0))
        return -1;
    // Clear all wakeup addresses and disable them 
    if (!rtk_ddc_drv_reset_ddc_wakeup_addr(id))
        return -1;
    // Clear hitted status
    if (!rtk_ddc_drv_clr_ddc_hitted_wakeup_addr(id))
        return -1;
    // disable interrupt.
    if (!rtk_ddc_drv_set_ddc_wakeup_irq(id, 0))
        return -1;
    return 0;
}
EXPORT_SYMBOL(rtk_ddc_lib_wakeup_reset);

/**
 * @func: Enable or disable the ddc wakeup function.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_enable_wakeup(unsigned char id, unsigned char enable)
{
    return rtk_ddc_drv_enable_ddc_wakeup(id, enable);
}
EXPORT_SYMBOL(rtk_ddc_lib_enable_wakeup);

/**
 * @func: Gets the enable status of wakeup
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_get_ddc_wakeup_enable(unsigned char id, unsigned char *enable)
{
    return rtk_ddc_drv_get_ddc_wakeup_enable(id, enable);
}
EXPORT_SYMBOL(rtk_ddc_lib_get_ddc_wakeup_enable);


/**
 * @func: Enable or disable the wakeup of the specified address.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[in] addr: The specified address for wakeup
 * @param[in] enable: Enable or disable wakeup
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_set_ddc_wakeup_addr(unsigned char id, unsigned char addr, unsigned char enable)
{
    return rtk_ddc_drv_set_ddc_wakeup_addr(id, addr, enable);
}
EXPORT_SYMBOL(rtk_ddc_lib_set_ddc_wakeup_addr);

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
int rtk_ddc_lib_get_ddc_wakeup_addr(unsigned char id, unsigned char *addr, unsigned char addr_len, unsigned char *addr_cnt)
{
    return rtk_ddc_drv_get_ddc_wakeup_addr(id, addr, addr_len, addr_cnt);
}
EXPORT_SYMBOL(rtk_ddc_lib_get_ddc_wakeup_addr);

/**
 * @func: Get the address which has been hitted.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @param[out] addr: Which address has been hitted
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_get_ddc_hitted_wakeup_addr(unsigned char id, unsigned char *addr)
{
    return rtk_ddc_drv_get_ddc_hitted_wakeup_addr(id, addr);
}
EXPORT_SYMBOL(rtk_ddc_lib_get_ddc_hitted_wakeup_addr);


/**
 * @func: Clear trigger state.
 * @param[in] id: DDC channel id [1,2,3,4]
 * @return
 *      - 0  success
 *      - other fail
 */
int rtk_ddc_lib_clr_ddc_hitted_wakeup_addr(unsigned char id)
{
    return rtk_ddc_drv_clr_ddc_hitted_wakeup_addr(id);
}
EXPORT_SYMBOL(rtk_ddc_lib_clr_ddc_hitted_wakeup_addr);

int rtk_ddc_lib_set_ddc_wakeup_irq(unsigned char id, unsigned char irq)
{
    return rtk_ddc_drv_set_ddc_wakeup_irq(id, irq);
}
EXPORT_SYMBOL(rtk_ddc_lib_set_ddc_wakeup_irq);

#endif // CONFIG_RTK_KDRV_DDC_WAKEUP

