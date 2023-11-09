#ifndef __RTK_OTP_REGION_API_H__
#define __RTK_OTP_REGION_API_H__

typedef enum {
    OTP_FIELD_WAFER_CLASS,
    OTP_FIELD_EMMC_LDO_TRIM_INFO,
    OTP_FIELD_USB_DISCONNECT_LEVEL,
    OTP_FIELD_USB_SQUELCH_LEVEL,
    OTP_FIELD_USB_SWING_LEVEL,
    OTP_FIELD_USB_DISCONNECT_LEVEL_DONE_BIT,
    OTP_FIELD_USB_SQUELCH_LEVEL_DONE_BIT,
    OTP_FIELD_USB_SWING_LEVEL_DONE_BIT,    
    OTP_FIELD_OVER_FREQ,
}OTP_FIELD_ID;

/*
   get otp field by enum OTP_FIELD_ID
   data: input buffer, OTP field info will fill into data
   len:  input buffer length
*/
extern bool rtk_otp_field_read(OTP_FIELD_ID field_id, unsigned char *data, unsigned int len);
/*
   get otp field by name, name can be find at
   "arch/arm64/boot/dts/rtk_tv/xxx_otp_table.dtsi"
   data: input buffer, OTP field info will fill into data
   len:  input buffer length
*/
extern bool rtk_otp_field_read_by_name(const char *name, unsigned char *data, unsigned int len);
/*
   get max to 31 bit by name, name can be find at
   "arch/arm64/boot/dts/rtk_tv/xxx_otp_table.dtsi"
   if ret == -1, means fail, otherwise
   return value is OTP field value, support filed lentgh < 32bit.
*/
extern int  rtk_otp_field_read_int_by_name(const char *name);



bool rtk_is_dobly_Vision_supported(void);
int rtk_get_hdcp_1_4_bksv_key(unsigned char *buf, unsigned int len);
int rtk_get_hdcp_1_4_private_key(unsigned char *buf, unsigned int len);
int rtk_get_hdcp_2_2_key(unsigned char *buf, unsigned int len);
int rtk_get_hdcp_2_2_bksb_key(unsigned char *buf, unsigned int len);
bool rtk_is_NNIP_function_disable(void);
bool rtk_is_pcmcia_disable(unsigned int index);
bool rtk_is_sc_disable(unsigned int index);
/*
   if ret == -1, means fail, otherwise
   return value is OTP field value, support filed lentgh < 32bit.
*/
int rtk_get_iddq_core_power(void);
/*
   if ret == -1, means fail, otherwise
   return value is OTP field value, support filed lentgh < 32bit.
*/
int rtk_get_iddq_scpu_power(void);

typedef enum _RTK_SET_TYPE
{
	SET_TYPE_UNKNOWN,
	SET_TYPE_TV,
	SET_TYPE_DIAS,
}RTK_SET_TYPE;
RTK_SET_TYPE rtk_get_set_type(void);

typedef enum _PACKAGE_FEATURE
{
	PACKAGE_FEATURE_UNKNOWN,
	PACKAGE_FEATURE_4K_60,
	PACKAGE_FEATURE_4K_120,
	PACKAGE_FEATURE_2K_60,
	PACKAGE_FEATURE_2K_60_FOR_4K,
}PACKAGE_FEATURE;
PACKAGE_FEATURE rtk_get_package_feature(void);

#endif
