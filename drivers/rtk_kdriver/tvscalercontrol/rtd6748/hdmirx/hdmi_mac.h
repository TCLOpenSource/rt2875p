#ifndef __HDMI_MAC_H_
#define __HDMI_MAC_H_

#define FW_PP_DEFAULT_PHASE(x)          ((x &0x1F) << 24)
#define FW_PP_DEFAULT_PHASE_MASK        (0x1F << 24)

typedef enum
{
    YUV422_COLOR_DEPTH_8_BITS,
    YUV422_COLOR_DEPTH_10_BITS,
    YUV422_COLOR_DEPTH_12_BITS,
    YUV422_COLOR_DEPTH_UNKNOWN,
} YUV422_CD_STATUS_T;


//------------------------------------------------------------------
// CRT
//------------------------------------------------------------------
extern void lib_hdmi_sys_crt_on(void);    // system crt
extern void lib_hdmi_crt_reset(unsigned char nport);
extern void lib_hdmi_crt_reset_hdmi_common(void);
extern void lib_hdmi_crt_off_hdmi_common(void);
extern unsigned char lib_hdmi_crt_is_hdmi_common_enabled(void);
extern void lib_hdmi_mac_crt_perport_off(unsigned char nport);
extern void lib_hdmi_mac_crt_perport_on(unsigned char nport);
extern void lib_hdmi_mac_crt_perport_reset(unsigned char nport);
extern void lib_hdmi_mac_sram_light_sleep_enable(unsigned char nport, unsigned char enable);

//------------------------------------------------------------------
// Per Port CRC
//------------------------------------------------------------------
extern void lib_hdmi_crc_check(unsigned char nport);
extern unsigned int lib_hdmi_read_crc(unsigned char nport);
extern void lib_hdmi_read_crc_long(unsigned char nport, unsigned int*  mac_crc0, unsigned int* mac_crc1);

//------------------------------------------------------------------
// Video Common CRC
//------------------------------------------------------------------
extern int lib_hdmi_read_video_common_crc(unsigned char res_sel, unsigned int* p_crc,unsigned char is_continue_mode);
extern void lib_hdmi_crc_continue_mode_en(unsigned char enable);
extern int lib_hdmi_get_crc_continue_mode_status(void);
extern int lib_hdmi_get_crc_auto_cmp_status(void);
extern void lib_hdmi_crc_auto_cmp_en(unsigned char enable);
extern int lib_hdmi_get_crc_err_cnt(void);
extern void lib_hdmi_set_des_crc(unsigned int common_crc);
extern int lib_hdmi_get_des_crc(void);

//------------------------------------------------------------------
// Video Common YUV422 color depth
//------------------------------------------------------------------
void lib_hdmi_yuv422_cd_det_en(unsigned char nport,unsigned char enable);
int lib_hdmi_get_yuv422_cd_det_en_status(unsigned char nport);
void lib_hdmi_yuv422_cd_set_frame_cnt_thr(unsigned char nport);
int lib_hdmi_yuv422_cd_get_frame_cnt_thr(unsigned char nport);
void lib_hdmi_yuv422_cd_set_pixel_cnt_thr(unsigned char nport);
int lib_hdmi_yuv422_cd_get_pixel_cnt_thr(unsigned char nport);
YUV422_CD_STATUS_T lib_hdmi_yuv422_get_color_depth(unsigned char nport);

//------------------------------------------------------------------
// TMDS
//------------------------------------------------------------------
extern void lib_hdmi_tmds_init(unsigned char port);
extern void lib_hdmi_tmds_out_ctrl(unsigned char nport, unsigned char bypass, unsigned int r, unsigned int g, unsigned int b);
extern void lib_hdmi_power(unsigned char port, unsigned char enable);
extern unsigned char lib_hdmi_is_rgb_status_ready(unsigned char nport);
extern void lib_hdmi_clear_rgb_hv_status(unsigned char nport);
extern unsigned int lib_hdmi_read_rgb_hv_status(unsigned char nport);
extern unsigned char lib_hdmi_is_vsync_detected(unsigned char nport);
extern void lib_hdmi_clear_vsync_detect(unsigned char nport);
extern unsigned char lib_hdmi_vsync_checked(unsigned char nport);
extern void lib_hdmi_valid_format_condition(unsigned char nport);
extern void lib_hdmi_clear_all_infoframe(unsigned char nport);
extern unsigned char lib_hdmi_tmds_get_br_swap(unsigned char nport);
extern void lib_hdmi_mac_set_hde_enable(unsigned char nport, unsigned char enable);
extern void lib_hdmi_mac_hd20_tmds_input_eanbale(unsigned char nport, unsigned char enable);
extern void lib_hdmi_tmds_cps_pll_div2_en(unsigned char nport, unsigned char en);

//------------------------------------------------------------------
// Video Data Error Detection
//------------------------------------------------------------------
extern void lib_hdmi_bit_err_rst(unsigned char nport, unsigned char rst);
extern void lib_hdmi_bit_err_start(unsigned char nport, unsigned char conti_mode, unsigned char period);
extern unsigned char lib_hdmi_bit_err_det_done(unsigned char nport);
extern unsigned char lib_hdmi_bit_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);

//------------------------------------------------------------------
// BCH Error control
//------------------------------------------------------------------
extern void lib_hdmi_set_bch_eanble(unsigned char nport, unsigned char on);
extern unsigned char lib_hdmi_get_bch_1bit_error(unsigned char port);
extern unsigned char lib_hdmi_get_bch_2bit_error(unsigned char port);
extern void lib_hdmi_bch_1bit_error_clr(unsigned char nport);
extern void lib_hdmi_bch_2bit_error_clr(unsigned char nport);
extern void lib_hdmi_bch_error_irq_en(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_is_bch_error_irq_en(unsigned char nport);
extern unsigned int lib_hdmi_get_bch_err2(unsigned char nport);
extern void lib_hdmi_bch_err2_clr(unsigned char nport);
extern unsigned int lib_hdmi_get_gcp_bch_err2(unsigned char nport);
extern void lib_hdmi_gcp_bch_err2_clr(unsigned char nport);

//------------------------------------------------------------------
// AV Mute control
//------------------------------------------------------------------
extern void lib_hdmi_video_output(unsigned char port, unsigned char on);
extern unsigned char lib_hdmi_is_video_output(unsigned char nport);
extern void lib_hdmi_set_avmute_ignore(unsigned char nport, unsigned char on);
extern void lib_hdmi_clear_hdmi_sr_avmute(unsigned char nport);
extern void lib_hdmi_clear_avmute(unsigned char port);
extern unsigned char lib_hdmi_get_avmute(unsigned char port);
extern unsigned char lib_hdmi_set_avmute_wd_en(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_get_avmute_wd_en(unsigned char nport);
extern void lib_hdmi_set_hdirq_en(unsigned char nport, unsigned char en);
extern unsigned char lib_hdmi_get_hdirq_en(unsigned char nport);
extern unsigned char lib_hdmi_is_modedet_fwmode(unsigned char nport);
extern unsigned char lib_hdmi_is_hdmi_mode(unsigned char nport);
extern unsigned char lib_hdmi_get_hdmi_mode_reg(unsigned char nport);

//------------------------------------------------------------------
// Packet Variation
//------------------------------------------------------------------
extern void lib_hdmi_set_video_packet_variation_irq(unsigned char nport, unsigned int val);

//------------------------------------------------------------------
// Video Flow Control
//------------------------------------------------------------------
extern void lib_hdmi_clear_interlace_reg(unsigned char port);
extern unsigned char lib_hdmi_get_is_interlace_reg(unsigned char port);
extern void lib_hdmi_clr_int_pro_chg_flag(unsigned char nport);
extern unsigned char lib_hdmi_get_int_pro_chg_flag(unsigned char nport);
extern void lib_hdmi_set_repeat_manual(unsigned char port, unsigned char rep_value);
extern void lib_hdmi_set_repeat_auto(unsigned char port);
extern unsigned char lib_hdmi_get_pixelrepeat(unsigned char port);
extern void lib_hdmi_set_colorspace_manual(unsigned char port, unsigned char color_space);
extern void lib_hdmi_set_colorspace_auto(unsigned char port);
extern unsigned char lib_hdmi_get_colorspace(unsigned char port);

//------------------------------------------------------------------
// Deep Color
//------------------------------------------------------------------
extern unsigned char lib_hdmi_get_color_depth(unsigned char port);
extern void lib_hdmi_set_dpc_enable(unsigned char nport, unsigned char on);
extern void lib_hdmi_set_dpc_bypass_dis(unsigned char nport, unsigned char on);
extern unsigned char lib_hdmi_is_dpc_default_phase(unsigned char nport);

//------------------------------------------------------------------
// Mode Control
//------------------------------------------------------------------
extern void lib_hdmi2p0_scramble_enable(unsigned char port, unsigned char on_off);
unsigned int lib_hdmi2p0_get_scramble_cr(unsigned char nport);
extern void lib_hdmi_gcp_ackg_header_parsing_mode(unsigned char nport, unsigned char hd21);

//------------------------------------------------------------------
// IRQ
//------------------------------------------------------------------
extern void lib_hdmi_set_interrupt_enable(unsigned char nport, unsigned int val);
extern unsigned int lib_hdmi_get_interrupt_enable(unsigned char nport);
extern void lib_hdmi_avmute_irq_enable(unsigned char nport);
extern void lib_hdmi_avmute_irq_disable(unsigned char nport);
extern unsigned char lib_hdmi_is_avmute_irq_enable(unsigned char nport);
extern unsigned char lib_hdmi_is_irq_pending(unsigned char nport);
extern void lib_hdmi_clr_irq_pending(unsigned char nport);
extern unsigned char lib_hdmi_get_is_irq_fw_all_0(unsigned char nport);

//------------------------------------------------------------------
// Video Format
//------------------------------------------------------------------
extern unsigned char lib_hdmi_get_video_format(unsigned char port);
extern unsigned char lib_hdmi_get_vsi_vic(unsigned char port);

//------------------------------------------------------------------
// 3D Format
//------------------------------------------------------------------
extern unsigned char lib_hdmi_get_3d_structure(unsigned char nport);
extern unsigned char lib_hdmi_get_3d_extdata(unsigned char port);


//------------------------------------------------------------------
// Async Phy FIFO control
//------------------------------------------------------------------
extern void lib_hdmi_mac_afifo_enable(unsigned char nport,unsigned char lane_mode);

//------------------------------------------------------------------
// CTS FIFO control
//------------------------------------------------------------------
extern void lib_hdmi_ofms_clock_sel(unsigned char nport, unsigned char src);
extern void lib_hdmi_mac_out_sel(unsigned char nport, unsigned char hd21);

//------------------------------------------------------------------
// Async CH FIFO control
//------------------------------------------------------------------
extern void lib_hdmi_afifo_enable(unsigned char nport, unsigned char enable);

//------------------------------------------------------------------
// CED
//------------------------------------------------------------------
extern void lib_hdmi_fw_char_err_rst(unsigned char nport, unsigned char rst);
extern void lib_hdmi_scdc_char_err_keep(unsigned char nport, unsigned char enable);
extern void lib_hdmi_scdc_char_err_rst(unsigned char nport, unsigned char rst);
extern void lib_hdmi_char_err_start(unsigned char nport, unsigned char conti_mode, unsigned char period);
extern unsigned char lib_hdmi_char_err_det_done(unsigned char nport);
extern unsigned char lib_hdmi_char_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);
extern unsigned char lib_hdmi_char_err_get_scdc_ced(unsigned char nport, unsigned char frl_mode, unsigned int err_cnt[4]);

//------------------------------------------------------------------
// YUV 420 ti 444
//------------------------------------------------------------------
extern void lib_hdmi_420_en(unsigned char nport, unsigned char en);
extern void lib_hdmi_420_fva_ds_mode(unsigned char nport, unsigned char en);
extern unsigned int lib_hdmi_get_420_fva_ds_mode(unsigned char nport);
extern void lib_hdmi_420_clkgen_en(unsigned char nport, unsigned char en);
extern void lib_hdmi_set_420_vactive(unsigned char nport, unsigned int vactive);
extern void lib_hdmi_force_bypass_yuv420_engine_en(unsigned char nport, unsigned char en);

//------------------------------------------------------------------
// Power Saving Measure
//------------------------------------------------------------------
extern void lib_hdmi_ps_measure_enable(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_ps_measure_complete(unsigned char nport);
extern void lib_hdmi_ps_measure_get_value(unsigned char nport, unsigned int* p_vs2vs_cycle, unsigned int* p_last_pixel2vs);

//------------------------------------------------------------------
// YUV420 4K 120 Hz mode (shared by all ports)
//------------------------------------------------------------------
extern void lib_hdmi_4k120_yu420_en(unsigned char nport, unsigned char en);
extern unsigned char lib_hdmi_is_4k120_yu420_en(unsigned char nport);

//------------------------------------------------------------------
// Video PLL Register (shared by all ports)
//------------------------------------------------------------------
extern unsigned char lib_hdmi_set_video_pll(unsigned char nport, unsigned int b_clk, unsigned char frl_mode, unsigned char cd, unsigned char is_interlace, unsigned char is_420, unsigned long* pll_pixelclockx1024);
extern void lib_hdmi_set_video_pll_power_down(unsigned char nport);
extern void lib_hdmi_set_video_pll_power_up(unsigned char nport);

//------------------------------------------------------------------
// One Time Register
//------------------------------------------------------------------
extern void lib_hdmi_ddc_sel(unsigned char nport, unsigned char ddc_ch);

//------------------------------------------------------------------
// FW Function
//------------------------------------------------------------------
extern void lib_hdmi_mac_select_port(unsigned char port);
extern void lib_hdmi_meas_select_port(unsigned char nport);

//------------------------------------------------------------------
// HD20/21 CTRL
//------------------------------------------------------------------
extern void lib_hdmi_set_hd20_21_ctrl_pixel_mode(unsigned char nport, unsigned char pixel_mode);

// FW Debug register control
extern unsigned char lib_hdmi_get_fw_debug_bit(unsigned int debug_bit);
extern void lib_hdmi_set_fw_debug_bit(unsigned int debug_bit, unsigned char val);

//reg = b800d508 

#define DEBUG_31_PLUG_CABLE_TEST    _BIT31
#define DEBUG_30_DFE_SCAN_TEST      _BIT30
#define DEBUG_29_BYPASS_AUDIO_DET   _BIT29

#define DEBUG_27_BYPASS_PHY_ISR     _BIT27
#define DEBUG_26_BYPASS_DETECT      _BIT26
#define DEBUG_25_BYPASS_CHECK       _BIT25
#define DEBUG_24_FORCE_6G           _BIT24

#define DEBUG_23_BITERR_DET         _BIT23
#define DEBUG_22_PHY_SCAN_TEST      _BIT22

#define DEBUG_21_FORCE_NO_INFOPKT   _BIT21
#define DEBUG_20_FORCE_NO_DRMPKT    _BIT20
#define DEBUG_19_BYPASS_PACKET_DET  _BIT19

#define DEBUG_18_PHY_DBG_MSG	   _BIT18
#define DEBUG_17_DUMP_REG           _BIT17
#define DEBUG_16_PRINT_DRM          _BIT16
#define DEBUG_15_PRINT_AVI          _BIT15

//#define DEBUG_14_CLK_DBG_MSG	   _BIT14

#define DEBUG_14_FRL_FORCE_MODE     _BIT14
#define DEBUG_13_FRL_FORCE_MEASURE     _BIT13

#define DEBUG_0_TMDS_4K120_YUV420_EN  _BIT0

//------------------------------------------------------------------
// MAC high level Functios
//------------------------------------------------------------------
extern void lib_hdmi_mac_init(unsigned char nport, unsigned char br_swap);
extern void lib_hdmi_mac_reset(unsigned char nport);
extern void lib_hdmi_mac_release(unsigned char nport, unsigned char frl_mode);

//------------------------------------------------------------------
// DMA VGIP
//------------------------------------------------------------------
extern void lib_hdmi_dma_vgip_capture_pixel_enable(unsigned short x, unsigned short y);
extern void lib_hdmi_dma_vgip_capture_pixel_disable(void);
extern unsigned int lib_hdmi_dma_vgip_is_video_capture_en(void);
extern void lib_hdmi_dma_vgip_get_pixel_value(unsigned short val[3]);

//------------------------------------------------------------------
// Debug Functios
//------------------------------------------------------------------
extern unsigned char lib_hdmi_get_mac_h_sync(unsigned char nport, unsigned is_per_port);
extern void lib_hdmi_dbg_reg_dump(unsigned char nport);
extern void lib_hdmi_wait_hdcp_vs(unsigned char nport, unsigned char cnt, unsigned int timeout);
extern unsigned int lib_hdmi_get_debug_bch_err_pkt(unsigned char port);

//------------------------------------------------------------------
// Clock Detection
//------------------------------------------------------------------
extern void lib_hdmi_hd20_clkdet_set_pop_up(unsigned char nport);
extern void lib_hdmi_hd20_init_clkdet_counter(unsigned char nport, unsigned char lane_sel);
extern unsigned char lib_hdmi_hd20_clkdet_get_clk_counter(unsigned char nport, unsigned int* get_clk);

#endif

