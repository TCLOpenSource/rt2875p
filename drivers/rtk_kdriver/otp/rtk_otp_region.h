#ifndef __RTK_OTP_REGION_H__
#define __RTK_OTP_REGION_H__

/* should be same with rhal
   add new region id at the end */
typedef enum {
    OTP_REGION_HDR_DISABLE = 0,
    OTP_REGION_REMARK_EN = 1,
    OTP_REGION_HDCP_1_4_BKSV_KEY = 2,
    OTP_REGION_HDCP_1_4_PRIVATE_KEY = 3,
    OTP_REGION_NNIP_DISABLE = 4,
    OTP_REGION_EMMC_LDO_TRIM_INFO = 5,
    OTP_REGION_USB_DISCONNECT_LEVEL = 6,
    OTP_REGION_USB_SQUELCH_LEVEL = 7,
    OTP_REGION_USB_SWING_LEVEL = 8,
    OTP_REGION_USB_DISCONNECT_LEVEL_DONE_BIT = 9,
    OTP_REGION_USB_SQUELCH_LEVEL_DONE_BIT = 10,
    OTP_REGION_USB_SWING_LEVEL_DONE_BIT = 11,
    /*rhal */
    OTP_REGION_GINGA = 12,
    OTP_REGION_BOOT_TYPE_SEL = 13,
    OTP_REGION_MEM_OBF_EN = 14,
    OTP_REGION_CHIPID_KEY = 15,
    OTP_REGION_CI_PLUS = 16,
    OTP_REGION_MHEG5 = 17,
    OTP_REGION_HbbTV = 18,
    OTP_REGION_WAFER_INFO_SLOT_ID = 19,
    OTP_REGION_FVP = 20,
    OTP_REGION_HDCP_2_2_KEY = 21,
    OTP_REGION_HDCP_2_2_BKSB_KEY = 22,
    OTP_REGION_OVER_FREQ = 23,
} OTP_REGION_ID;

#define OTP_ERROR_GENERAL                       (-1)
#define OTP_ERROR_REGION_NOT_FIND               (-2)
#define OTP_ERROR_TABLE_INVALID                 (-3)
#define OTP_ERROR_REMARK_BIT_ERROR              (-4)
#define OTP_ERROR_REGION_OFFSET_INVALID         (-5)



extern int rtk_otp_region_read(OTP_REGION_ID region_id, unsigned char *data, unsigned int len);
extern int rtk_otp_region_read_by_name(const char *name, unsigned char *data, unsigned int len);
extern void rtk_otp_show_field(char *buf);
extern int rtk_otp_region_get_size_by_name(const char *name);
#endif
