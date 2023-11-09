#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <rtk_kdriver/io.h>
#include <linux/mutex.h>
#include "rtk_otp_region.h"
#include "rtk_otp_debug.h"
#include <rtk_kdriver/rtk_otp_region_api.h>


bool rtk_is_dobly_Vision_supported(void)
{
    bool ret =  false;
    unsigned int doblyVision = 0x0;

    if (rtk_otp_region_read(OTP_REGION_HDR_DISABLE, (unsigned char *)&doblyVision, sizeof(doblyVision)) == 0) {
        if (doblyVision == 0) {
            ret = true;
        }
    }

    return ret;
}
EXPORT_SYMBOL(rtk_is_dobly_Vision_supported);

int rtk_get_hdcp_1_4_bksv_key(unsigned char *buf, unsigned int len)
{
    return rtk_otp_region_read(OTP_REGION_HDCP_1_4_BKSV_KEY, buf, len);
}
EXPORT_SYMBOL(rtk_get_hdcp_1_4_bksv_key);

int rtk_get_hdcp_1_4_private_key(unsigned char *buf, unsigned int len)
{
    return rtk_otp_region_read(OTP_REGION_HDCP_1_4_PRIVATE_KEY, buf, len);
}
EXPORT_SYMBOL(rtk_get_hdcp_1_4_private_key);

/* hdcp 2.2 key has remove from OTP */
int rtk_get_hdcp_2_2_key(unsigned char *buf, unsigned int len)
{
    return rtk_otp_region_read(OTP_REGION_HDCP_2_2_KEY, buf, len);
}
EXPORT_SYMBOL(rtk_get_hdcp_2_2_key);

int rtk_get_hdcp_2_2_bksb_key(unsigned char *buf, unsigned int len)
{
    return rtk_otp_region_read(OTP_REGION_HDCP_2_2_BKSB_KEY, buf, len);
}
EXPORT_SYMBOL(rtk_get_hdcp_2_2_bksb_key);

bool rtk_is_NNIP_function_disable(void)
{
    bool ret = false;
    unsigned char data = 0;
    if (rtk_otp_region_read(OTP_REGION_NNIP_DISABLE, &data, 1) != 0) {
        goto EXIT;
    }

    if (data) {
        ret = true;
    }
EXIT:
    return ret;
}
EXPORT_SYMBOL(rtk_is_NNIP_function_disable);


bool rtk_is_pcmcia_disable(unsigned int index)
{
	int bit_size;
	int val = -1;
	if(index >= 2)
		return true;
	if(index == 0) {
		bit_size = rtk_otp_region_get_size_by_name("io_pcmcia_disable");
		if(bit_size != 1 && bit_size != 4)
			return true;
		if((val = rtk_otp_field_read_int_by_name("io_pcmcia_disable")) == -1)
			return true;
	} else {
		bit_size = rtk_otp_region_get_size_by_name("io_pcmcia1_disable");
		if(bit_size != 1 && bit_size != 4)
			return true;
		if((val = rtk_otp_field_read_int_by_name("io_pcmcia1_disable")) == -1)
			return true;
	}
	if(bit_size == 4) {
		if(val == 0 || val == 1 || val == 8 || val ==9)
			return false;
	} else {
		if(val == 0)
			return false;
	}
	return true;
}
EXPORT_SYMBOL(rtk_is_pcmcia_disable);

bool rtk_is_sc_disable(unsigned int index)
{
	int bit_size;
	int val = -1;
	if(index >= 2)
		return true;
	if(index == 0) {
		bit_size = rtk_otp_region_get_size_by_name("io_sc_disable");
		if(bit_size != 1 && bit_size != 4)
			return true;
		if((val = rtk_otp_field_read_int_by_name("io_sc_disable")) == -1)
			return true;
	} else {
		bit_size = rtk_otp_region_get_size_by_name("io_sc1_disable");
		if(bit_size != 1 && bit_size != 4)
			return true;
		if((val = rtk_otp_field_read_int_by_name("io_sc1_disable")) == -1)
			return true;
	}
	if(bit_size == 4) {
		if(val == 0 || val == 1 || val == 8 || val ==9)
			return false;
	} else {
		if(val == 0)
			return false;	
	}
	return true;
}	
EXPORT_SYMBOL(rtk_is_sc_disable);

int rtk_get_iddq_core_power(void)
{
    int bit_size;
    int ret;
    bit_size = rtk_otp_region_get_size_by_name("hw_mp_status_ft_iddq_done");
    ret = rtk_otp_field_read_int_by_name("hw_mp_status_ft_iddq_done");
    /************MAY NEED CHECK***********/
    if(bit_size != 1 && bit_size != 3)
		return -1;
    if(ret == -1)
		return -1;
    if(bit_size == 1 && ret != 1)
		return -1;
    if(bit_size == 3 && ret != 5)
		return -1;

    return rtk_otp_field_read_int_by_name("hw_iddq_core_power");
    
}
EXPORT_SYMBOL(rtk_get_iddq_core_power);

int rtk_get_iddq_scpu_power(void)
{
    int bit_size;
    int ret;
    bit_size = rtk_otp_region_get_size_by_name("hw_mp_status_ft_iddq_done");
    ret = rtk_otp_field_read_int_by_name("hw_mp_status_ft_iddq_done");
    /************MAY NEED CHECK***********/
    if(bit_size != 1 && bit_size != 3)
		return -1;
    if(ret == -1)
		return -1;
    if(bit_size == 1 && ret != 1)
		return -1;
    if(bit_size == 3 && ret != 5)
		return -1;

    return rtk_otp_field_read_int_by_name("hw_iddq_scpu_power");
    
}
EXPORT_SYMBOL(rtk_get_iddq_scpu_power);

RTK_SET_TYPE rtk_get_set_type(void)
{
	RTK_SET_TYPE type = SET_TYPE_UNKNOWN;
	int ret = rtk_otp_field_read_int_by_name("sw_func_set_type");
	if(ret == -1)
		return type;
	if(ret == 0x2) {//Dias 
		OTP_INFO("Set Type is Dias\n");
		type =  SET_TYPE_DIAS;
	} else {//TV
		if(ret == 0)
			OTP_WARNING("Set Type not burned\n");
		OTP_INFO("Set Type is TV\n");
		type =  SET_TYPE_TV;
	}
	return type;	
}
EXPORT_SYMBOL(rtk_get_set_type);


PACKAGE_FEATURE rtk_get_package_feature(void)
{
	PACKAGE_FEATURE feature = PACKAGE_FEATURE_UNKNOWN;
	int ret = rtk_otp_field_read_int_by_name("hw_cpu_package_feature");
	if(ret == -1)
		return feature;

	switch(ret) {
       case 0:
	   	feature = PACKAGE_FEATURE_4K_60;
		break;
	case 1:
		feature = PACKAGE_FEATURE_4K_120;
		break;
	case 2:
		feature = PACKAGE_FEATURE_2K_60;
		break;
	case 3:
		feature = PACKAGE_FEATURE_2K_60_FOR_4K;
		break;	
	default:
		break;
	};
	return feature;
}
EXPORT_SYMBOL(rtk_get_package_feature);

bool rtk_otp_field_read(OTP_FIELD_ID field_id, unsigned char *data, unsigned int len)
{
    bool ret = false;
    switch (field_id) {
        case OTP_FIELD_WAFER_CLASS:
            /* no wafer class */
            ret = false;
            break;
        case OTP_FIELD_EMMC_LDO_TRIM_INFO:
            ret = (rtk_otp_region_read(OTP_REGION_EMMC_LDO_TRIM_INFO, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_DISCONNECT_LEVEL:
            ret = (rtk_otp_region_read(OTP_REGION_USB_DISCONNECT_LEVEL, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_SQUELCH_LEVEL:
            ret = (rtk_otp_region_read(OTP_REGION_USB_SQUELCH_LEVEL, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_SWING_LEVEL:
            ret = (rtk_otp_region_read(OTP_REGION_USB_SWING_LEVEL, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_DISCONNECT_LEVEL_DONE_BIT:
            ret = (rtk_otp_region_read(OTP_REGION_USB_DISCONNECT_LEVEL_DONE_BIT, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_SQUELCH_LEVEL_DONE_BIT:
            ret = (rtk_otp_region_read(OTP_REGION_USB_SQUELCH_LEVEL_DONE_BIT, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_USB_SWING_LEVEL_DONE_BIT:
            ret = (rtk_otp_region_read(OTP_REGION_USB_SWING_LEVEL_DONE_BIT, data, len) == 0) ? true : false;
            break;
        case OTP_FIELD_OVER_FREQ:
            ret = (rtk_otp_region_read(OTP_REGION_OVER_FREQ, data, len) == 0) ? true : false;
            break;
        default:
            ret = false;
            break;
    }

    return ret;
}
EXPORT_SYMBOL(rtk_otp_field_read);

bool rtk_otp_field_read_by_name(const char *name, unsigned char *data, unsigned int len)
{
    char done_name[64] = {0};
    unsigned int done = 0;
    int ret = 0;
    if (name == NULL)
        return false;
    snprintf(done_name, 64, "%s_done", name);
    ret = rtk_otp_region_read_by_name((const char *)done_name, (unsigned char *)&done, sizeof(done));
    if (ret == (-2)) {
        return (rtk_otp_region_read_by_name(name, data, len) == 0) ? true : false;
    }
    else if (ret == 0) {
        if (done == 1)
            return (rtk_otp_region_read_by_name(name, data, len) == 0) ? true : false;
        else
            return false;
    }
    else
        return false;
}
EXPORT_SYMBOL(rtk_otp_field_read_by_name);

int rtk_otp_field_read_int_by_name(const char *name)
{
    int i = 0;
    if (rtk_otp_field_read_by_name(name, (unsigned char*)&i, sizeof(int))==true)
        return i;
    return -1;
}
EXPORT_SYMBOL(rtk_otp_field_read_int_by_name);
