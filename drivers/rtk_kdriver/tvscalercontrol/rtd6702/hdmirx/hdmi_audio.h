#ifndef __HDMI_AUDIO_H__
#define __HDMI_AUDIO_H__

extern const char* _hdmi_audio_fsm_str(unsigned int fsm);
extern const char* _hdmi_audio_fmt_str(unsigned char fmt);
extern unsigned char _hdmi_audio_get_channel_count(unsigned char ch_mode);
extern void lib_hdmi_audio_init(unsigned char port);
extern void lib_hdmi_audio_pop_n_cts(unsigned char port);
extern unsigned char lib_hdmi_audio_pop_n_cts_done(unsigned char port);
extern unsigned int lib_hdmi_audio_get_n(unsigned char port);
extern unsigned int lib_hdmi_audio_get_cts(unsigned char port);
extern void lib_hdmi_audio_cts_bound(unsigned char nport, unsigned int clk);
extern void lib_hdmi_audio_n_bound(unsigned char nport, unsigned int freq);
extern unsigned char lib_hdmi_audio_get_channel_mode(unsigned char nport);

extern unsigned char lib_hdmi_audio_is_fifo_overflow(unsigned char port);
extern void lib_hdmi_audio_wclr_fifo_overflow(unsigned char port);
extern unsigned char lib_hdmi_audio_is_output(unsigned char port);
extern void lib_hdmi_audio_output(unsigned char port, unsigned char on);
extern void lib_hdmi_audio_set_hbr_manual_mode(unsigned char nport, unsigned char manu, unsigned char mode_fw);
extern void lib_hdmi_audio_clear_hbr_status(unsigned char nport);
extern unsigned char lib_hdmi_audio_get_hbr_manual_mode(unsigned char nport);
extern unsigned char lib_hdmi_audio_get_hbr_mode(unsigned char nport);
extern void lib_hdmi_audio_source_select(unsigned char nport, unsigned char src);
#define AUDIO_SRC_NONE           0x0
#define AUDIO_SRC_NORMAL_AUDIO   0x1
#define AUDIO_SRC_HBR_AUDIO      0x2
#define AUDIO_SRC_BOTH           0x3
extern void lib_hdmi_audio_dbg_reg_dump(unsigned char nport);
extern void lib_hdmi_audio_set_thread_run(unsigned char nport, unsigned char uc_run);

extern void newbase_hdmi_audio_thread_enable(unsigned char create);
extern void newbase_hdmi_audio_var_init(unsigned char port);
extern unsigned char newbase_hdmi_audio_detect(void);
extern void newbase_hdmi_audio_close(unsigned char port);
extern void newbase_hdmi_audio_reset_status(void);
extern unsigned char newbase_hdmi_audio_get_current_status(void);
extern unsigned char newbase_hdmi_audio_wait_sample(unsigned char port);
extern unsigned char newbase_hdmi_audio_wait_2_samples(unsigned char port);

extern int newbase_hdmi_audio_get_audio_status(unsigned char nport, HDMI_AUDIO_ST* p_status);

//-------------- AUDIO DEC -----------
extern HDMI_AUDIO_FORMAT_T lib_hdmi_audio_get_coding_type(unsigned char nport);
extern unsigned char lib_hdmi_audio_get_channel_num(unsigned char nport);
extern unsigned char lib_hdmi_audio_get_channel_status_bit(unsigned char nport);
extern unsigned char lib_hdmi_audio_get_audio_packet_layout_value(unsigned char nport);

#endif //__HDMI_AUDIO_H__
