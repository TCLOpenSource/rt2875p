#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <rtk_kdriver/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "rtk_otp_util.h"
#include "rtk_otp_region.h"
#include "rtk_otp_region_priv.h"
#include "rtk_otp_debug.h"

#define INV_VALUE 0xffffffff

extern RTK_OTP_OPS g_rtk_otp_ops;
static struct device_node *otp_np = NULL;

/* default table */
static OTP_REGION_INFO g_otp_region_table[] = {
    /* kernel */
    {OTP_REGION_HDR_DISABLE,                   INV_VALUE, INV_VALUE, INV_VALUE, "chip_type_hdr_disable"},
    {OTP_REGION_REMARK_EN,                     INV_VALUE, INV_VALUE, INV_VALUE, "chip_type_remark_en"},
    {OTP_REGION_HDCP_1_4_BKSV_KEY,             INV_VALUE, INV_VALUE, INV_VALUE, "hdcp_1_4_bksv_key"},
    {OTP_REGION_HDCP_1_4_PRIVATE_KEY,          INV_VALUE, INV_VALUE, INV_VALUE, "hdcp_1_4_private_key"},
    {OTP_REGION_NNIP_DISABLE,                  INV_VALUE, INV_VALUE, INV_VALUE, "chip_type_nnip_disable"},
    {OTP_REGION_EMMC_LDO_TRIM_INFO,            INV_VALUE, INV_VALUE, INV_VALUE, "hw_emmc_ldo_trim_info"},
    {OTP_REGION_USB_DISCONNECT_LEVEL,          INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_disconnect_level"},
    {OTP_REGION_USB_SQUELCH_LEVEL,             INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_squelch_level"},
    {OTP_REGION_USB_SWING_LEVEL,               INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_swing_level"},
    {OTP_REGION_USB_DISCONNECT_LEVEL_DONE_BIT, INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_disconnect_done"},
    {OTP_REGION_USB_SQUELCH_LEVEL_DONE_BIT,    INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_squelch_done"},
    {OTP_REGION_USB_SWING_LEVEL_DONE_BIT,      INV_VALUE, INV_VALUE, INV_VALUE, "hw_usb_swing_done"},
    /* rhal */
    {OTP_REGION_BOOT_TYPE_SEL,      INV_VALUE, INV_VALUE, INV_VALUE, "func_boot_type_sel"},
    {OTP_REGION_MEM_OBF_EN,         INV_VALUE, INV_VALUE, INV_VALUE, "func_mem_obf_en"},
    {OTP_REGION_CHIPID_KEY,         INV_VALUE, INV_VALUE, INV_VALUE, "wafer_info_slot"},
    {OTP_REGION_GINGA,              INV_VALUE, INV_VALUE, INV_VALUE, "sw_func_ginga"},
    {OTP_REGION_CI_PLUS,            INV_VALUE, INV_VALUE, INV_VALUE, "sw_func_ciplus"},
    {OTP_REGION_MHEG5,              INV_VALUE, INV_VALUE, INV_VALUE, "sw_func_mheg5"},
    {OTP_REGION_HbbTV,              INV_VALUE, INV_VALUE, INV_VALUE, "sw_func_hbbtv"},
    {OTP_REGION_WAFER_INFO_SLOT_ID, INV_VALUE, INV_VALUE, INV_VALUE, "wafer_info_all"},
    {OTP_REGION_FVP,                INV_VALUE, INV_VALUE, INV_VALUE, "sw_func_fvp"},
    {OTP_REGION_HDCP_2_2_KEY,             INV_VALUE, INV_VALUE, INV_VALUE, "hdcp_2_2_key"},
    {OTP_REGION_HDCP_2_2_BKSB_KEY,          INV_VALUE, INV_VALUE, INV_VALUE, "hdcp_2_2_bksb_key"},
    {OTP_REGION_OVER_FREQ,          INV_VALUE, INV_VALUE, INV_VALUE, "sw_over_freq"},
    
};

#define OTP_REGION_TABLE_SIZE (sizeof(g_otp_region_table) / sizeof(g_otp_region_table[0]))

#ifdef OTP_DBG_EN
static const char *rtk_otp_region_get_name(OTP_REGION_ID region_id)
{
    unsigned int i = 0;
    unsigned int len = OTP_REGION_TABLE_SIZE;
    for (i = 0; i < len; i++) {
        if (g_otp_region_table[i].region_id == region_id) {
            return g_otp_region_table[i].region_name;
        }
    }
    return "bad region id";
}
#endif

int rtk_otp_region_query_by_name(const char *name, OTP_SIMPLE_INFO *region_info)
{
    int tmp[3];
    if (otp_np == NULL) {
        otp_np = of_find_node_by_name(NULL, "rtk_otp_table");
        if (otp_np == NULL) {
            OTP_WARNING("otp_table in device tree not exist\n");
            return OTP_ERROR_TABLE_INVALID;
        }
    }
    if (of_property_read_u32_array(otp_np, name, tmp, 3) == 0) {
        int ofst = tmp[0];
        int ofst_remark = tmp[1];
        int len = tmp[2];
        if (ofst_remark != 0 && ofst_remark < 0xffff) {
            int ret =  false;
            unsigned int remark = 0;
            /* remark_en bit could not have remark offset  */
            if (strncmp("remark_en", name, 9) == 0) {
                return OTP_ERROR_REMARK_BIT_ERROR;
            }
            ret = rtk_otp_region_read(OTP_REGION_REMARK_EN, (unsigned char *)&remark, sizeof(remark));
            if (ret) {
                return ret;
            }
            if (remark) {
                region_info->ofst = ofst_remark;
                region_info->len  = len;
                return 0;
            }
        }
        region_info->ofst = ofst;
        region_info->len  = len;
        return 0;
    }
    OTP_DBG("otp_table name %s not exist\n", name);
    return OTP_ERROR_REGION_NOT_FIND;
}

static int rtk_otp_region_query_region_ofst(OTP_REGION_ID region_id, OTP_SIMPLE_INFO *region_info)
{
    unsigned int i = 0;
    unsigned int len = OTP_REGION_TABLE_SIZE;

    if (region_info == NULL) {
        return OTP_ERROR_GENERAL;
    }

    for (i = 0; i < len; i++) {
        if (g_otp_region_table[i].region_id == region_id) {
            break;
        }
    }

    /* not find region id */
    if (i == len) {
        return OTP_ERROR_REGION_NOT_FIND;
    }

    return rtk_otp_region_query_by_name(g_otp_region_table[i].region_name, region_info);
}

static int rtk_otp_simple_read(OTP_SIMPLE_INFO *region_info, unsigned char *data, unsigned int len)
{
    int ret = OTP_ERROR_GENERAL;
    unsigned char *buf = NULL;
    unsigned int byte_ofst = 0;
    unsigned int bit_ofst = 0;
    unsigned int byte_len = 0;
    if (!data || !len) {
        return ret;
    }
    if (region_info->ofst < 0 || region_info->ofst > 0xffff ||
            region_info->len <= 0 || region_info->len > 0xffff) {
        OTP_WARNING("invalid ofst:%d len %d\n", region_info->ofst, region_info->len);
        return OTP_ERROR_REGION_OFFSET_INVALID;
    }

    memset(data, 0, len);
    byte_ofst = region_info->ofst / 8;
    bit_ofst = region_info->ofst % 8;
    byte_len = (bit_ofst + region_info->len - 1) / 8 + 1;

    if(in_interrupt())
		buf = (unsigned char *)kmalloc(byte_len + 1, GFP_ATOMIC);
    else
    	buf = (unsigned char *)kmalloc(byte_len + 1, GFP_KERNEL);
	
    if (!buf) {
        return ret;
    }
    memset(buf, 0, byte_len + 1);

    ret = g_rtk_otp_ops.otp_read_region_byte(byte_ofst, buf, byte_len);
    if (ret == 0) {
        unsigned int copy_len = 0;
        if (bit_ofst) {
            unsigned int i = 0;
            for (i = 0; i < byte_len; i++) {
                unsigned char tmp = 0;
                buf[i] = (buf[i] >> bit_ofst);
                tmp = (buf[i + 1] << (8 - bit_ofst));
                buf[i] = tmp | buf[i];
            }
        }
        if (region_info->len % 8) {
            unsigned char tmp  = 0;
            byte_ofst = region_info->len / 8;
            bit_ofst = region_info->len % 8;
            tmp = (buf[byte_ofst] << (8 - bit_ofst));
            buf[byte_ofst] = (tmp >> (8 - bit_ofst));
        }
        copy_len = (region_info->len - 1) / 8 + 1;
        memcpy(data, buf, (copy_len > len) ? len : copy_len);
    }
    kfree(buf);
    return ret;
}

int rtk_otp_region_read(OTP_REGION_ID region_id, unsigned char *data, unsigned int len)
{
    OTP_SIMPLE_INFO tmp_info;
    OTP_SIMPLE_INFO *region_info = &tmp_info;
    int ret = OTP_ERROR_GENERAL;
    ret = rtk_otp_region_query_region_ofst(region_id, region_info);
    if (ret) {
        OTP_WARNING("query fail region ofset fail %d, ret %d\n", region_id, ret);
        return ret;
    }
    OTP_DBG("id:%-2d %-24s ofst:%-4d len:%-2d\n", region_id, rtk_otp_region_get_name(region_id), region_info->ofst, region_info->len);
    ret = rtk_otp_simple_read(region_info, data, len);
    return ret;
}

int rtk_otp_region_read_by_name(const char *name, unsigned char *data, unsigned int len)
{
    OTP_SIMPLE_INFO tmp_info;
    OTP_SIMPLE_INFO *region_info = &tmp_info;
    int ret = -1;
    if (name == NULL) {
        return ret;
    }
    ret = rtk_otp_region_query_by_name(name, region_info);
    if (ret) {
        if (ret != OTP_ERROR_REGION_NOT_FIND)
            OTP_WARNING("query %s region ofset fail, ret %d\n", name, ret);
        return ret;
    }
    OTP_DBG("name: %-24s ofst:%-4d len:%-2d\n", name, region_info->ofst, region_info->len);
    ret = rtk_otp_simple_read(region_info, data, len);
    return ret;
}

int rtk_otp_region_get_size_by_name(const char *name)
{
    OTP_SIMPLE_INFO tmp_info;
    OTP_SIMPLE_INFO *region_info = &tmp_info;
    int ret = -1;
    if (name == NULL) {
        return ret;
    }
    ret = rtk_otp_region_query_by_name(name, region_info);
    if (ret) {
        if (ret != OTP_ERROR_REGION_NOT_FIND)
            OTP_WARNING("query %s region ofset fail, ret %d\n", name, ret);
        return ret;
    }
    OTP_DBG("query %s %d\n", name, region_info->len);
    return region_info->len;
}

void rtk_otp_show_field(char *buf)
{
    struct  property *otp_pp;
    unsigned char tmp[16];
    int ret = 0;
    buf[0] = 0;
    if (otp_np == NULL) {
        otp_np = of_find_node_by_name(NULL, "rtk_otp_table");
        if (otp_np == NULL) {
            OTP_WARNING("otp_table in device tree not exist\n");
            return;
        }
    }
    snprintf(buf, 4096, "**************************** OTP Field ****************************\n");
    for_each_property_of_node(otp_np, otp_pp) {
        ret = rtk_otp_region_read_by_name(otp_pp->name, tmp, 16);
        if (ret == 0) {
            unsigned int ofst;
            unsigned int ofst_remark;
            unsigned int len;
            unsigned char line_buf[64];
            const unsigned int *cur = NULL;
            cur = of_prop_next_u32(otp_pp, cur, &ofst);
            cur = of_prop_next_u32(otp_pp, cur, &ofst_remark);
            cur = of_prop_next_u32(otp_pp, cur, &len);
            if (ofst_remark != 0) {
                snprintf(buf + strlen(buf), PAGE_SIZE, "%-24s: ofst :%-4d, len :%-2d, ofst_remark :%-4d\n", otp_pp->name, ofst, len, ofst_remark);
            }
            else {
                snprintf(buf + strlen(buf), PAGE_SIZE, "%-24s: ofst :%-4d, len :%-2d\n", otp_pp->name, ofst, len);
            }
            hex_dump_to_buffer(tmp, (len + 7) / 8, 16, 1, line_buf, 64, 0);
            snprintf(buf + strlen(buf), PAGE_SIZE, "                          value: %s\n", line_buf);
        }
    }
    return;
}

