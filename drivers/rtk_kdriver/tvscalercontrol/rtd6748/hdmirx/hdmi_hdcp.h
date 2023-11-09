#ifndef __HDMI_HDCP_H__
#define __HDMI_HDCP_H__

//#define ENABLE_DVI_1_1_SUPPORT
#define	HDCP_10_DVI 	0x91
#define HDCP_11_DVI     0x93
#ifdef ENABLE_DVI_1_1_SUPPORT
#define HDCP_11_HDMI    HDCP_11_DVI
#else
#define HDCP_11_HDMI    HDCP_10_DVI
#endif

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

extern void lib_hdmi_hdcp_init(unsigned char nport);
extern void lib_hdmi_write_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320]);
extern void lib_hdmi_read_hdcpkey(unsigned char nport, unsigned char bksv[5], unsigned char hdcp_key[320]);

// for HDCP1.4


extern void lib_hdmi_hdcp_port_write_conti(unsigned char port, unsigned char addr, const unsigned char* value, unsigned char num);
extern void lib_hdmi_hdcp_port_write(unsigned char port, unsigned char addr ,unsigned char value);
extern unsigned char lib_hdmi_hdcp_port_read(unsigned char port, unsigned char addr);
extern void lib_hdmi_hdcp_toggle_dkapde(unsigned char nport);

#define   DEFAULT_HDCP_MODE HDCP_OFF

//////////////////////////////////////////////

typedef struct {
    unsigned char BKsv[5];
    unsigned char Key[320];
} HDCP_KEY_T;


//-----------------------------------------------------------------
// Global
//-----------------------------------------------------------------
// Internal used functions
extern void lib_hdmi_hdcp_set_i2c_addr(unsigned char nport, unsigned char i2c_addr);
extern void lib_hdmi_hdcp_set_win_opp_mode(unsigned char nport, unsigned char mode);
extern void lib_hdmi_hdcp_set_win_enc_window(unsigned char nport, unsigned int enc_window);
extern unsigned char lib_hdmi_hdcp_get_vs_polarity(unsigned char nport);
extern void lib_hdmi_hdcp_set_fifo_mode(unsigned char nport, unsigned char auto_mode,unsigned char fw_fifo_mode_hdcp22);

// Public function
extern HDMI_HDCP_ST* newbase_hdcp_get_hdcp_status(unsigned char port);
extern const char* _hdmi_hdcp_mode_str(HDMI_HDCP_E hdcp_mode);
extern void newbase_hdmi_switch_hdcp_mode(unsigned char port, HDMI_HDCP_E target_hdcp_mode);
extern void newbase_hdmi_hdcp_init(void);
extern void newbase_hdmi_hdcp_enable(unsigned char nport, unsigned char enable);
extern void newbase_hdmi_hdcp_reset_fsm(unsigned char nport);
extern void newbase_hdcp_init_timer(unsigned char port);
extern void newbase_hdcp_handler(unsigned char port);
extern HDMI_HDCP_E newbase_hdcp_get_auth_mode(unsigned char port);
extern void newbase_hdmi_hdcp_disable_hdcp2(unsigned char port, unsigned char disable);
#define EVERY_HDMI_PORT  0xF
extern int newbase_hdmi_hdcp_is_hdcp2_disabled(unsigned char port);
extern spinlock_t hdcp2_spin_lock;
#define lock_hdcp_sram(flags)       spin_lock_irqsave(&hdcp2_spin_lock, flags)
#define unlock_hdcp_sram(flags)     spin_unlock_irqrestore(&hdcp2_spin_lock, flags)
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
extern void          lib_hdmi_hdcp_set_enc_tog_sel(unsigned char nport, unsigned char mode);
extern unsigned char lib_hdmi_hdcp_get_enc_toggle(unsigned char nport);
extern void          lib_hdmi_hdcp_clear_enc_toggle(unsigned char nport);
extern unsigned char lib_hdmi_hdcp14_check_adne_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_clear_adne_reg(unsigned char nport);
extern void          lib_hdmi_hdcp14_cypher_enable(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_hdcp14_get_aksv_wr_irq_en(unsigned char nport);
extern void          lib_hdmi_hdcp14_set_aksv_wr_irq(unsigned char nport, unsigned char enable);
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
extern unsigned char newbase_hdmi_hdcp14_read_aksv_ex(unsigned char port, unsigned char *pAKSV, unsigned char big_endian);// extend for endian control
extern void newbase_hdmi_hdcp14_read_hdcp_key(unsigned char bksv[5], unsigned char hdcp_key[320]);
extern void newbase_hdmi_hdcp14_write_hdcp_key(unsigned char bksv[5], unsigned char hdcp_key[320], unsigned char mode);
#define APPLY_KEY_IMMEDATELY     1
extern unsigned char newbase_hdmi_hdcp14_read_ri(unsigned char port,unsigned char Ri[2]);
extern void newbase_hdmi_hdcp14_read_an(unsigned char port,unsigned char An[8]);
extern void newbase_hdmi_hdcp14_read_an_ex(unsigned char port,unsigned char An[8], unsigned char big_endian); // extend for endian control
extern void newbase_hdmi_hdcp14_read_bksv(unsigned char port,unsigned char Bksv[5]);
extern void newbase_hdmi_hdcp14_read_bcaps(unsigned char port,unsigned char *pBcaps);
extern void newbase_hdmi_hdcp14_write_bcaps(unsigned char port,unsigned char Bcaps);
extern void newbase_hdmi_hdcp14_read_bstatus(unsigned char port,unsigned char Bstatus[2]);
extern void newbase_hdmi_hdcp14_read_bstatus_ex(unsigned char port,unsigned char Bstatus[2], unsigned char big_endian); // extend for endian control
extern void newbase_hdmi_hdcp14_write_bstatus(unsigned char port,unsigned char Bstatus[2]);
extern void newbase_hdmi_hdcp14_write_v_prime(unsigned char port,unsigned char v_prime[20]);
extern void newbase_hdmi_hdcp14_write_ksv_fifo(unsigned char port, unsigned char* p_ksv_fifo, unsigned int len);
extern unsigned char* newbase_hdmi_get_default_hdcp14_bksv(void);
extern unsigned char* newbase_hdmi_get_default_hdcp14_key(void);
extern void newbase_hdmi_hdcp14_set_aksv_intr(unsigned char port, unsigned char en);
extern void newbase_hdmi_hdcp_set_reauth_msg_threshold(unsigned int threshold);
extern unsigned int newbase_hdmi_hdcp_get_reauth_msg_threshold(void);
extern int newbase_hdmi_hdcp14_interrupt_handler(unsigned char nport);
extern unsigned char newbase_rxphy_set_phy_for_aksv(unsigned char port);

//-----------------------------------------------------------------
// HDCP2.2 related
//-----------------------------------------------------------------

/*
0x001: unauthenticated
0x002: compute km
0x004: compute L'
0x008: compute ks
0x010: authenticated
0x020: wait for downstream
0x040: assemble receiver ID list
0x080: verify receiver ID list acknowledgement
0x100: content stream management
0x200: content stream management from wait for downstream which has not been done
0x400: content stream management from assemble receiver ID list which has not been done
0x800: content stream management from verify receiver ID list acknowledgement which has not been done
*/

typedef enum {
	HDMI_HDCP_2P2_UNAUTH = 0x001,
	HDMI_HDCP_2P2_COMPUTE_KM = 0x002,
	HDMI_HDCP_2P2_COMPUTE_LP = 0x004,
	HDMI_HDCP_2P2_COMPUTE_KS = 0x008,
	HDMI_HDCP_2P2_AUTHENTICATED = 0x010,
	HDMI_HDCP_2P2_WAIT_FOR_DOWNSTREAM = 0x020,
	HDMI_HDCP_2P2_ASSEMBLE_RX_ID_LIST = 0x040,
	HDMI_HDCP_2P2_VERIFY_RX_ID_LIST_ACK = 0x080,
	HDMI_HDCP_2P2_CONTENT_STREAM_MANAGEMENT = 0x100,
	HDMI_HDCP_2P2_CONTENT_STREAM_MANAGEMENT_2 = 0x200,
	HDMI_HDCP_2P2_CONTENT_STREAM_MANAGEMENT_3 = 0x400,
	HDMI_HDCP_2P2_CONTENT_STREAM_MANAGEMENT_4 = 0x800,
	HDMI_HDCP_2P2_CONTENT_UNKNOW,
}  HDMI_HDCP_2P2_STATUS_T;

// Internal used functions
extern void          lib_hdmi_hdcp22_enable(unsigned char port, unsigned char enable);
extern unsigned char lib_hdmi_hdcp22_is_onoff(unsigned char nport);
extern unsigned char lib_hdmi_hdcp22_get_auth_done(unsigned char nport);
extern void          lib_hdmi_hdcp22_init(unsigned char port, unsigned char *uc_lc128);
extern int           lib_hdmi_hdcp22_set_ks(unsigned char nport, unsigned char* bKs);
extern void          lib_hdmi_hdcp22_update_message_size(unsigned char nport, unsigned short wLen);
extern unsigned char lib_hdmi_is_hdcp22_event_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_get_rx_status(unsigned char nport, unsigned char RxStatus[2]);
extern void          lib_hdmi_hdcp22_aes_engine_sel(unsigned char nport, unsigned char hd21);
extern unsigned char lib_hdmi_hdcp22_is_irq_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_clr_irq_pending(unsigned char nport);
extern void          lib_hdmi_hdcp22_enable_reauth(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_hdcp22_get_enable_reauth(unsigned char nport);
extern void          lib_hdmi_hdcp22_write_data_to_tx(unsigned char nport, unsigned char* bSendData, unsigned short wLen);

// External used functions
extern unsigned char newbase_hdmi_hdcp22_msg_handler(unsigned char port);
extern void newbase_hdmi_hdcp22_write_data_to_tx(unsigned char nport, unsigned char* bSendData, unsigned short wLen);
extern void newbase_hdmi_hdcp22_clear_status(unsigned char port);
extern void newbase_hdmi_hdcp22_set_reauth(unsigned char nport);
extern void newbase_hdmi_hdcp22_set_hw_reauth(unsigned char en);
extern unsigned char newbase_hdmi_hdcp22_get_hw_reauth(unsigned char port);
extern void newbase_hdmi_hdcp22_check_iic_status(unsigned char port);

//-----------------------------------------------------------------
// HDCP Repeater related
//-----------------------------------------------------------------

#include "hdmi_hdcp_repeater.h"
#endif //__HDMI_HDCP_H__
