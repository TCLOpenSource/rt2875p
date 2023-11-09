#ifndef _HDCP14_OPTEE_H
#define _HDCP14_OPTEE_H

typedef enum {
    HDMI_HDCP_MODE,
    DP_OPTEE_HDCP_MODE,
}HDMI_DP_HDCP_MODE_T;

#define HDCP14_CMD_LOAD_KEY             0
#define HDCP14_CMD_DP_AKSV_KSEL_INDEX   0x40
#define HDCP14_CMD_GET_BKSV             0x41

extern void optee_hdcp14_load_key(HDMI_DP_HDCP_MODE_T hdmi_dp, unsigned char load_mode);
extern void optee_hdcp14_dp_aksv_ksel_index(unsigned char key_sel_idx);
extern int optee_hdcp14_get_key(unsigned char *bksv, unsigned int bksv_len, unsigned char *bkey, unsigned int bkey_len);
#endif
