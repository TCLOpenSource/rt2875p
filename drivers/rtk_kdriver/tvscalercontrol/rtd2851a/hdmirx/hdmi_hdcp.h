#ifndef __HDMI_HDCP_H__
#define __HDMI_HDCP_H__

#ifdef BUILD_QUICK_SHOW
#define GFP_KERNEL      0
#endif

//-----------------------------------------------------------------
// Data Structure
//-----------------------------------------------------------------

#define HDCP_10_DVI     0x91
#define HDCP_11_HDMI    0x93

typedef enum {
    IIC_STATUS_IDLE = 0,  // do nothing
    IIC_STATUS_CHKDEV,    // check device address
    IIC_STATUS_RTMP,      // read temp and ack without increasing ddc address
    IIC_STATUS_RWREC,     // r/w receiving state
    IIC_STATUS_DATW,      // data writing
    IIC_STATUS_OADDW,     // offset address writing
    IIC_STATUS_READ,      // read state
    IIC_STATUS_WTMP       // write temp, ack without increasing ddc address
}HDMI_HDCP_I2C_STATE;

typedef enum _HDMI_HDCP_{
    NO_HDCP = 0,
    HDCP14,
    HDCP22,
    HDCP_OFF
}HDMI_HDCP_E;

typedef enum {
    HDCP2_DISABLE = 0,
    HDCP2_IDEL,
    HDCP2_AKE_INIT,
    HDCP2_NO_STOREE_KM,
    HDCP2_STOREE_KM,
    HDCP2_SEND_EKS,
}HDMI_HDCP2_RX_STATE;

typedef struct {
    unsigned char BKsv[5];
    unsigned char Key[320];
} HDCP_KEY_T;

typedef struct {
	unsigned char tx_aksv[5];
	unsigned char hdcp14_auth_count;
	unsigned char hdcp14_ri_count;
	unsigned char hdcp_enc;             // 0 : no enc, 1 : enc
	unsigned int  hdcp_reauth_cnt;
	unsigned char hdcp2_reauth   : 1;   // force reauth hdcp2
	unsigned char hdcp2_state    : 3;   // hdcp2 state : please ref HDMI_HDCP2_RX_STATE
	unsigned char hdcp2_auth_count : 4; // number of hdcp2 authen performed
	unsigned long hdcp2_wait_mesage_timout;
	unsigned char hdcp_init_period; // Timeout to reset to turn on (hdcp1.4/2.2), while no sync
	unsigned long hdcp_iic_state_time; // checkout i2c status
} HDMI_HDCP_ST;

//-----------------------------------------------------------------
// Global
//-----------------------------------------------------------------

extern void lib_hdmi_hdcp_init(unsigned char nport);
extern void lib_hdmi_write_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320]);
extern void lib_hdmi_read_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320]);

// for HDCP1.4
extern void lib_hdmi_hdcp_port_write_conti(unsigned char port, unsigned char addr, const unsigned char* value, unsigned char num);
extern void lib_hdmi_hdcp_port_write(unsigned char port, unsigned char addr ,unsigned char value);
extern unsigned char lib_hdmi_hdcp_port_read(unsigned char port, unsigned char addr);
extern void lib_hdmi_hdcp_toggle_dkapde(unsigned char nport);

#define   DEFAULT_HDCP_MODE HDCP_OFF

//-----------------------------------------------------------------
// Global
//-----------------------------------------------------------------
// Internal used functions
extern void lib_hdmi_hdcp_set_win_opp_mode(unsigned char nport, unsigned char mode);
extern void lib_hdmi_hdcp_set_win_enc_window(unsigned char nport, unsigned int enc_window);
extern unsigned char lib_hdmi_hdcp_get_vs_polarity(unsigned char nport);

// Public function
extern HDMI_HDCP_ST* newbase_hdcp_get_hdcp_status(unsigned char port);
extern const char* _hdmi_hdcp_mode_str(HDMI_HDCP_E hdcp_mode);
extern void newbase_hdmi_switch_hdcp_mode(unsigned char port, HDMI_HDCP_E target_hdcp_mode);
extern void newbase_hdmi_hdcp_init(void);
extern void newbase_hdmi_hdcp_enable(unsigned char nport, unsigned char enable);
extern void newbase_hdmi_hdcp_reset_fsm(unsigned char nport);
extern int  newbase_hdcp_init_timer(unsigned char port);
extern void newbase_hdcp_handler(unsigned char port);
extern HDMI_HDCP_E newbase_hdcp_get_auth_mode(unsigned char port);
extern void newbase_hdmi_hdcp_disable_hdcp2(unsigned char port, unsigned char disable);
#define EVERY_HDMI_PORT  0xF
extern int newbase_hdmi_hdcp_is_hdcp2_disabled(unsigned char port);
extern void lock_hdcp_sram(void);
extern void unlock_hdcp_sram(void);
extern void newbase_hdmi_reset_hdcp_once(unsigned char port, unsigned char set_value);
extern unsigned char newbase_hdmi_is_reset_hdcp_once(unsigned char port);

//-----------------------------------------------------------------
// HDCP 1.4 related
//-----------------------------------------------------------------
// Internal used functions
extern void          lib_hdmi_hdcp14_enable(unsigned char port, unsigned char on);
extern unsigned char lib_hdmi_hdcp14_is_onoff(unsigned char nport);
extern unsigned char lib_hdmi_hdcp14_check_aksv_reg(unsigned char port);
extern void          lib_hdmi_hdcp14_clear_aksv_reg(unsigned char port);
extern unsigned char lib_hdmi_hdcp14_check_bksv_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_clear_bksv_reg(unsigned char nport);
extern unsigned char lib_hdmi_hdcp14_check_akm_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_clear_akm_reg(unsigned char nport);
extern unsigned char lib_hdmi_hdcp_get_enc_toggle(unsigned char nport);
extern void          lib_hdmi_hdcp_clear_enc_toggle(unsigned char nport);
extern unsigned char lib_hdmi_hdcp14_check_adne_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_clear_adne_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_cypher_enable(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_hdcp14_is_irq_pending(unsigned char nport);
extern void          lib_hdmi_hdcp14_clr_irq_pending(unsigned char nport);

extern void lib_hdmi_hdcp22_set_wr_msg_done_irq_en(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_hdcp22_is_wr_msg_done_irq_en(unsigned char nport);
extern unsigned int  lib_hdmi_hdcp22_get_state(unsigned char nport);
extern void          lib_hdmi_hdcp22_set_state(unsigned char nport, unsigned int new_state);
#define HDCP2_STATE_UNAUTH                       0x1
#define HDCP2_STATE_COMPUTE_KM                   0x2
#define HDCP2_STATE_COMPUTE_L_PRIME              0x4
#define HDCP2_STATE_COMPUTE_KS                   0x8
#define HDCP2_STATE_AUTHENTICATED                0x10
#define HDCP2_STATE_WAIT_FOR_DOWN_STREAM         0x20
#define HDCP2_STATE_ASSEMBLE_RECIVER_ID          0x40
#define HDCP2_STATE_VERIFY_RECEIVER_ID_LIST_ACK  0x80
#define HDCP2_STATE_CONTENT_STREAM_MANAGEMENT    0x100

// External used functions
extern unsigned char newbase_hdmi_hdcp14_read_aksv(unsigned char port, unsigned char *pAKSV);
extern void newbase_hdmi_hdcp14_read_hdcp_key(unsigned char bksv[5], unsigned char hdcp_key[320]);
extern void newbase_hdmi_hdcp14_write_hdcp_key(unsigned char bksv[5], unsigned char hdcp_key[320], unsigned char mode);
#define APPLY_KEY_IMMEDATELY     1
extern unsigned char newbase_hdmi_hdcp14_read_ri(unsigned char port,unsigned char Ri[2]);
extern void newbase_hdmi_hdcp14_read_an(unsigned char port,unsigned char An[8]);
extern void newbase_hdmi_hdcp14_read_bksv(unsigned char port,unsigned char Bksv[5]);
extern void newbase_hdmi_hdcp14_read_bcaps(unsigned char port,unsigned char *pBcaps);
extern void newbase_hdmi_hdcp14_read_bstatus(unsigned char port,unsigned char Bstatus[2]);
extern unsigned char* newbase_hdmi_get_default_hdcp14_bksv(void);
extern unsigned char* newbase_hdmi_get_default_hdcp14_key(void);
extern void newbase_hdmi_hdcp_set_reauth_msg_threshold(unsigned int threshold);
extern unsigned int newbase_hdmi_hdcp_get_reauth_msg_threshold(void);

//-----------------------------------------------------------------
// HDCP2.2 related
//-----------------------------------------------------------------

// Internal used functions
extern void          lib_hdmi_hdcp22_enable(unsigned char port, unsigned char enable);
extern unsigned char lib_hdmi_hdcp22_is_onoff(unsigned char nport);
extern unsigned char lib_hdmi_hdcp22_get_auth_done(unsigned char nport);
extern void          lib_hdmi_hdcp22_init(unsigned char port, unsigned char *uc_lc128);
extern int          lib_hdmi_hdcp22_set_ks(unsigned char nport, unsigned char* bKs);
extern void          lib_hdmi_hdcp22_update_message_size(unsigned char nport, unsigned short wLen);
extern unsigned char lib_hdmi_is_hdcp22_event_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_get_rx_status(unsigned char nport, unsigned char RxStatus[2]);
extern void          lib_hdmi_hdcp22_aes_engine_sel(unsigned char nport, unsigned char hd21);
extern unsigned char lib_hdmi_hdcp22_is_irq_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_clr_irq_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_enable_reauth(unsigned char nport, unsigned char enable);
extern void          lib_hdmi_hdcp22_reset_aes_engine(unsigned char nport);

// External used functions
extern unsigned char newbase_hdmi_hdcp22_msg_handler(unsigned char port);
extern void newbase_hdmi_hdcp22_write_data_to_tx(unsigned char nport, unsigned char* bSendData, unsigned short wLen);
extern void newbase_hdmi_hdcp22_clear_status(unsigned char port);
extern void newbase_hdmi_hdcp22_set_reauth(unsigned char nport);
extern void newbase_hdmi_hdcp22_check_iic_status(unsigned char port);

extern void newbase_hdmi_hdcp_qs_init(void);
extern int check_vcpu_hdcp_idle(void);

#endif //__HDMI_HDCP_H__
