#include <rbus/hdmi_p0_reg.h>
#include <rbus/hdmi_p1_reg.h>
#include <rbus/hdmi_p2_reg.h>
#include <rbus/hdmi_p3_reg.h>
#include <rbus/common_rx_420_reg.h>

#ifndef uint32_t
typedef unsigned int __UINT32_T;
#define uint32_t __UINT32_T
#endif

//---------------------------------------------------------------------------------
// HDMI 2.0/1.4 MAC
//---------------------------------------------------------------------------------

#define HDMI_MAC0_BASE           (HDMI_P0_PORT0_SOFT_0_reg)
#define HDMI_MAC1_BASE           (0xB803B000)
#define HDMI_MAC2_BASE           (0xB803C000)
#define HDMI_MAC3_BASE           (0xB80B2000)


struct hdmi_mac_reg_st
{
    // CRC Function
    uint32_t soft_0;
    uint32_t soft_1;

    // CRC Function
    uint32_t pp_tmds_crcc;
    uint32_t pp_tmds_crco0;
    uint32_t pp_tmds_crco1;

    // TMDS Channel Status
    uint32_t tmds_ctrl;
    uint32_t tmds_outctl;
    uint32_t tmds_pwdctl;
    uint32_t tmds_z0cc;
    uint32_t tmds_cps;
    uint32_t tmds_udc;
    uint32_t tmds_errc;
    uint32_t tmds_out_ctrl;
    uint32_t tmds_rout;
    uint32_t tmds_gout;
    uint32_t tmds_bout;
    uint32_t tmds_dpc0;
    uint32_t tmds_dpc1;
    uint32_t tmds_dpc_set0;
    uint32_t tmds_dpc_set1;
    uint32_t tmds_dpc_set2;
    uint32_t tmds_dpc_set3;

    // Video Data Error Detect
    uint32_t video_bit_err_det;
    uint32_t video_bit_err_status_b;
    uint32_t video_bit_err_status_g;
    uint32_t video_bit_err_status_r;

    // TERC4 Error Detect
    uint32_t terc4_err_det;
    uint32_t terc4_err_status_b;
    uint32_t terc4_err_status_g;
    uint32_t terc4_err_status_r;

    // Flow and Packet
    uint32_t hdmi_sr;
    uint32_t hdmi_gpvs;
    uint32_t hdmi_gpvs1;
    uint32_t hdmi_psap;
    uint32_t hdmi_psdp;

    uint32_t hdmi_scr;
    uint32_t hdmi_bchcr;
    uint32_t hdmi_avmcr;
    uint32_t hdmi_wdcr;
    uint32_t hdmi_pamicr;
    uint32_t hdmi_ptrsv1;
    uint32_t hdmi_ptrsv2;
    uint32_t hdmi_ptrsv3;
    uint32_t hdmi_pvgcr0;
    uint32_t hdmi_pvsr0;
    uint32_t hdmi_vcr;
    uint32_t hdmi_acrcr;
    uint32_t hdmi_acrsr0;
    uint32_t hdmi_acrsr1;
    // IRQ
    uint32_t hdmi_intcr;

    // Packet Guard Band
    uint32_t hdmi_bcsr;
    uint32_t hdmi_asr0;
    uint32_t hdmi_asr1;
    uint32_t hdmi_video_format;
    uint32_t hdmi_3d_format;
    uint32_t hdmi_fvs;
    uint32_t hdmi_drm;
    uint32_t hdmi_pcmc;
    uint32_t hdrap;
    uint32_t hdrdp;
    uint32_t hdr_st;
    uint32_t em_ct;
    uint32_t fapa_ct;
    uint32_t hdmi_emap;
    uint32_t hdmi_emdp;
    uint32_t hdmi_emap_pp;
    uint32_t hdmi_emdp_pp;
    uint32_t vrrem_st;
    uint32_t hdmi_vrr_emc;

    // Audio Control
    uint32_t audio_sample_rate_change_irq;
    uint32_t high_bit_rate_audio_packet;
    uint32_t hdmi_audcr;
    uint32_t hdmi_audsr;

    // Clock Detection
    uint32_t clkdetcr;
    uint32_t clkdetsr;
    uint32_t clk_setting_00;
    uint32_t clk_setting_01;
    uint32_t clk_setting_02;

    // RPBS
    uint32_t prbs_r_ctrl;
    uint32_t prbs_g_ctrl;
    uint32_t prbs_b_ctrl;

    // MHL
    uint32_t hdmi_leading_gb_cmp_ctrl;
    uint32_t hdmi_leading_gb_cmp_times;
    uint32_t hdmi_leading_gb_cmp_cnt;
    uint32_t audio_cts_up_bound;
    uint32_t audio_cts_low_bound;
    uint32_t audio_n_up_bound;
    uint32_t audio_n_low_bound;

    // BIST & FIFO
    uint32_t light_sleep;
    uint32_t read_margin_enable;
    uint32_t read_margin3;
    uint32_t read_margin1;
    uint32_t read_margin0;
    uint32_t bist_mode;
    uint32_t bist_done;
    uint32_t bist_fail;
    uint32_t drf_mode;
    uint32_t drf_resume;
    uint32_t drf_done;
    uint32_t drf_pause;
    uint32_t drf_fail;

    // PHY FIFO
    uint32_t phy_fifo_cr0;
    uint32_t phy_fifo_cr1;
    uint32_t phy_fifo_sr0;
    uint32_t phy_fifo_sr1;
    uint32_t hdmi_cts_fifo_ctl;

    // CH FIFO
    uint32_t ch_cr;
    uint32_t ch_sr;

    //-------------------------------------------------------------
    // HDMI 2.0
    //-------------------------------------------------------------
    uint32_t hdmi_2p0_cr;
    uint32_t hdmi_2p0_cr1;
    uint32_t scr_cr;

    // CED
    uint32_t cercr;
    uint32_t cersr0;
    uint32_t cersr1;
    uint32_t cersr2;
    uint32_t cersr3;
    uint32_t cersr4;

    // YUV420 to 444
    uint32_t yuv420_cr;
    uint32_t yuv420_cr1;

    // HDR
    uint32_t fapa_ct_tmp;
    uint32_t hdr_em_ct2;
    uint32_t hdmi_vsem_emc;
    uint32_t hdmi_vsem_emc2;
    uint32_t vsem_st;
    uint32_t hd20_ps_ct;
    uint32_t hd20_ps_st;
    uint32_t irq_all_status;
    uint32_t xtal_1ms;
    uint32_t xtal_10ms;
    uint32_t ps_measure_ctrl;
    uint32_t ps_measure_xtal_clk;
    uint32_t ps_measure_tmds_clk;

    // Packet Absence
    uint32_t hdmi_pa_irq_en;
    uint32_t hdmi_pa_irq_sta;
    uint32_t hdmi_pa_frame_cnt;

    // HDMI scrmble control align
    uint32_t sc_align_b;
    uint32_t sc_align_r;
    uint32_t scr_ctr_per_lfsr;

    // HDCP 1.4
    uint32_t hdcp_keepout;
    uint32_t hdcp_cr;
    uint32_t hdcp_cr2;
    uint32_t hdcp_pcr;
    uint32_t hdcp_flag1;
    uint32_t hdcp_flag2;

    // HDCP 2.2
    uint32_t hdcp_2p2_cr;
    uint32_t hdcp_2p2_ks0;
    uint32_t hdcp_2p2_ks1;
    uint32_t hdcp_2p2_ks2;
    uint32_t hdcp_2p2_ks3;
    uint32_t hdcp_2p2_lc0;
    uint32_t hdcp_2p2_lc1;
    uint32_t hdcp_2p2_lc2;
    uint32_t hdcp_2p2_lc3;
    uint32_t hdcp_2p2_riv0;
    uint32_t hdcp_2p2_riv1;
    uint32_t hdcp_2p2_sr0;
    uint32_t hdcp_2p2_sr1;
    uint32_t hdcp_hoa;
    uint32_t hdcp_power_saving_ctrl;
    uint32_t win_opp_ctl;

    // Video PLL
    uint32_t hdmi_vpllcr0;
    uint32_t hdmi_vpllcr1;
    uint32_t hdmi_vpllcr2;
    uint32_t mn_sclkg_ctrl;
    uint32_t mn_sclkg_divs;

    // Metadata Detection
    uint32_t mdd_cr;
    uint32_t mdd_sr;

    // FW Function
    uint32_t fw_func;
    uint32_t port_switch;
    uint32_t power_saving;

    // Video Common CRC
    uint32_t crc_ctrl;
    uint32_t crc_result;
    uint32_t des_crc;
    uint32_t crc_err_cnt0;
    uint32_t hdmi_20_21_ctrl;

    // YUV422_CD
    uint32_t yuv422_cd;

    // HDR Extended Metadata Packet for common port  //(0xXXXXX530~0xXXXXX5C8)only for 2nd common port (p0 register), 1st common port0 still use "Per port" r/w
    uint32_t common_hdrap; // 530
    uint32_t common_hdrdp;
    uint32_t common_hdr_st;
    uint32_t common_em_ct;
    uint32_t common_fapa_ct;
    uint32_t common_fapa_ct_tmp;
    uint32_t common_hdr_em_ct2;
    uint32_t common_hdmi_vsem_emc;
    uint32_t common_hdmi_vsem_emc2;
    uint32_t common_vsem_st;
    uint32_t common_hdmi_emap;
    uint32_t common_hdmi_emdp;
    uint32_t common_hd21_hdmi_hdrap;
    uint32_t common_hd21_hdmi_hdrdp;
    uint32_t common_hd21_hdr_em_st;
    uint32_t common_hd21_hdr_em_ct;
    uint32_t common_hd21_hdr_em_ct2;
    uint32_t common_hd21_hdmi_emap;
    uint32_t common_hd21_hdmi_emdp;
    uint32_t common_hd21_vsem_st; // D554
    uint32_t common_hd21_cvtem_st;
    uint32_t common_hd21_hdmi_vsem_emc;
    uint32_t common_hd21_hdmi_cvtem_emc;
    uint32_t common_hd21_hdmi_vsem_emc2;

    // HDMI DSCD DBG Ctrl for common port
    uint32_t common_hd21_dscd_dbg;
    uint32_t common_hd21_dscd_dbg_ap;
    uint32_t common_hd21_dscd_dbg_dp;

    // Audio Control for common port
    uint32_t common_hdmi_audcr;
    uint32_t common_hdmi_audsr;
    uint32_t common_hd21_hdmi_audcr;
    uint32_t common_hd21_hdmi_audsr;

    // HDMI Packet Status Register for common port
    uint32_t common_hdmi_gpvs;
    uint32_t common_hd21_hdmi_gpvs;

    // RM Zero Pading for common port
    uint32_t common_hd21_rmzp;

    // Video Timing Generator Status Register for common port
    uint32_t common_hd21_timing_gen_sr; // d5c8

    // One Time Register  (No Function for MAC P1,P2,P3)
    uint32_t ot;
    uint32_t irq;
    uint32_t mp;
    uint32_t hdmi_v_bypass;
    uint32_t hdmi_mp;
    uint32_t debug_port;
    // Audio Common port select
    uint32_t aud_common_port_switch;

    // CRT
    uint32_t hdmi_rst0;
    uint32_t hdmi_rst3;
    uint32_t hdmi_clken0;
    uint32_t hdmi_clken3;
};


extern const struct hdmi_mac_reg_st mac[4];


#define  HDMI_SOFT_0_reg                       (mac[nport].soft_0)
#define  HDMI_SOFT_1_reg                       (mac[nport].soft_1)
#define  HDMI_PP_TMDS_CRCC_reg                 (mac[nport].pp_tmds_crcc)
#define  HDMI_PP_TMDS_CRCO0_reg                (mac[nport].pp_tmds_crco0)
#define  HDMI_PP_TMDS_CRCO1_reg                (mac[nport].pp_tmds_crco1)
#define  HDMI_TMDS_CTRL_reg                    (mac[nport].tmds_ctrl)
#define  HDMI_TMDS_OUTCTL_reg                  (mac[nport].tmds_outctl)
#define  HDMI_TMDS_PWDCTL_reg                  (mac[nport].tmds_pwdctl)
#define  HDMI_TMDS_Z0CC_reg                    (mac[nport].tmds_z0cc)
#define  HDMI_TMDS_CPS_reg                     (mac[nport].tmds_cps)
#define  HDMI_TMDS_UDC_reg                     (mac[nport].tmds_udc)
#define  HDMI_TMDS_ERRC_reg                    (mac[nport].tmds_errc)
#define  HDMI_TMDS_OUT_CTRL_reg                (mac[nport].tmds_out_ctrl)
#define  HDMI_TMDS_ROUT_reg                    (mac[nport].tmds_rout)
#define  HDMI_TMDS_GOUT_reg                    (mac[nport].tmds_gout)
#define  HDMI_TMDS_BOUT_reg                    (mac[nport].tmds_bout)
#define  HDMI_TMDS_DPC0_reg                    (mac[nport].tmds_dpc0)
#define  HDMI_TMDS_DPC1_reg                    (mac[nport].tmds_dpc1)
#define  HDMI_TMDS_DPC_SET0_reg                (mac[nport].tmds_dpc_set0)
#define  HDMI_TMDS_DPC_SET1_reg                (mac[nport].tmds_dpc_set1)
#define  HDMI_TMDS_DPC_SET2_reg                (mac[nport].tmds_dpc_set2)
#define  HDMI_TMDS_DPC_SET3_reg                (mac[nport].tmds_dpc_set3)


#define  HDMI_VIDEO_BIT_ERR_DET_reg            (mac[nport].video_bit_err_det)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_reg       (mac[nport].video_bit_err_status_b)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_reg       (mac[nport].video_bit_err_status_g)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_reg       (mac[nport].video_bit_err_status_r)
#define  HDMI_TERC4_ERR_DET_reg                (mac[nport].terc4_err_det)
#define  HDMI_TERC4_ERR_STATUS_B_reg           (mac[nport].terc4_err_status_b)
#define  HDMI_TERC4_ERR_STATUS_G_reg           (mac[nport].terc4_err_status_g)
#define  HDMI_TERC4_ERR_STATUS_R_reg           (mac[nport].terc4_err_status_r)

#define  HDMI_HDMI_SR_reg                      (mac[nport].hdmi_sr)
#define  HDMI_HDMI_GPVS_reg                    (mac[nport].hdmi_gpvs)
#define  HDMI_HDMI_GPVS1_reg                   (mac[nport].hdmi_gpvs1)
#define  HDMI_HDMI_PSAP_reg                    (mac[nport].hdmi_psap)
#define  HDMI_HDMI_PSDP_reg                    (mac[nport].hdmi_psdp)
#define  HDMI_HDMI_SCR_reg                     (mac[nport].hdmi_scr)
#define  HDMI_HDMI_BCHCR_reg                   (mac[nport].hdmi_bchcr)
#define  HDMI_HDMI_AVMCR_reg                   (mac[nport].hdmi_avmcr)
#define  HDMI_HDMI_WDCR_reg                    (mac[nport].hdmi_wdcr)

// Packet variation
#define  HDMI_HDMI_PAMICR_reg                  (mac[nport].hdmi_pamicr)
#define  HDMI_HDMI_PTRSV1_reg                  (mac[nport].hdmi_ptrsv1)
#define  HDMI_HDMI_PTRSV2_reg                  (mac[nport].hdmi_ptrsv2)
#define  HDMI_HDMI_PTRSV3_reg                  (mac[nport].hdmi_ptrsv3)
#define  HDMI_HDMI_PVGCR0_reg                  (mac[nport].hdmi_pvgcr0)
#define  HDMI_HDMI_PVSR0_reg                   (mac[nport].hdmi_pvsr0)
#define  HDMI_HDMI_VCR_reg                     (mac[nport].hdmi_vcr)
#define  HDMI_HDMI_ACRCR_reg                   (mac[nport].hdmi_acrcr)
#define  HDMI_HDMI_ACRSR0_reg                  (mac[nport].hdmi_acrsr0)
#define  HDMI_HDMI_ACRSR1_reg                  (mac[nport].hdmi_acrsr1)
#define  HDMI_HDMI_INTCR_reg                   (mac[nport].hdmi_intcr)
#define  HDMI_HDMI_BCSR_reg                    (mac[nport].hdmi_bcsr)
#define  HDMI_HDMI_ASR0_reg                    (mac[nport].hdmi_asr0)
#define  HDMI_HDMI_ASR1_reg                    (mac[nport].hdmi_asr1)
#define  HDMI_HDMI_VIDEO_FORMAT_reg            (mac[nport].hdmi_video_format)
#define  HDMI_HDMI_3D_FORMAT_reg               (mac[nport].hdmi_3d_format)
#define  HDMI_HDMI_FVS_reg                     (mac[nport].hdmi_fvs)
#define  HDMI_HDMI_DRM_reg                     (mac[nport].hdmi_drm)
#define  HDMI_HDMI_PCMC_reg                    (mac[nport].hdmi_pcmc)

#define  HDMI_HDRAP_reg                        (mac[nport].hdrap)
#define  HDMI_HDRDP_reg                        (mac[nport].hdrdp)
#define  HDMI_HDRST_reg                        (mac[nport].hdr_st)
#define  HDMI_EM_CT_reg                        (mac[nport].em_ct)
#define  HDMI_fapa_ct_reg                      (mac[nport].fapa_ct)
#define  HDMI_HDMI_EMAP_reg                    (mac[nport].hdmi_emap)
#define  HDMI_HDMI_EMDP_reg                    (mac[nport].hdmi_emdp)
#define  HDMI_HDMI_EMAP_PP_reg                 (mac[nport].hdmi_emap_pp)
#define  HDMI_HDMI_EMDP_PP_reg                 (mac[nport].hdmi_emdp_pp)
#define  HDMI_VRREM_ST_reg                     (mac[nport].vrrem_st)
#define  HDMI_HDMI_VRR_EMC_reg                 (mac[nport].hdmi_vrr_emc)
#define  HDMI_Audio_Sample_Rate_Change_IRQ_reg (mac[nport].audio_sample_rate_change_irq)
#define  HDMI_High_Bit_Rate_Audio_Packet_reg   (mac[nport].high_bit_rate_audio_packet)
#define  HDMI_HDMI_AUDCR_reg                   (mac[nport].hdmi_audcr)
#define  HDMI_HDMI_AUDSR_reg                   (mac[nport].hdmi_audsr)


#define  HDMI_CLKDETCR_reg                     (mac[nport].clkdetcr)
#define  HDMI_CLKDETSR_reg                     (mac[nport].clkdetsr)
#define  HDMI_CLK_SETTING_00_reg               (mac[nport].clk_setting_00)
#define  HDMI_CLK_SETTING_01_reg               (mac[nport].clk_setting_01)
#define  HDMI_CLK_SETTING_02_reg               (mac[nport].clk_setting_02)

#define  HDMI_PRBS_R_CTRL_reg                  (mac[nport].prbs_r_ctrl)
#define  HDMI_PRBS_G_CTRL_reg                  (mac[nport].prbs_g_ctrl)
#define  HDMI_PRBS_B_CTRL_reg                  (mac[nport].prbs_b_ctrl)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_reg     (mac[nport].hdmi_leading_gb_cmp_ctrl)
#define  HDMI_HDMI_leading_GB_cmp_times_reg    (mac[nport].hdmi_leading_gb_cmp_times)
#define  HDMI_HDMI_leading_GB_cmp_cnt_reg      (mac[nport].hdmi_leading_gb_cmp_cnt)
#define  HDMI_AUDIO_CTS_UP_BOUND_reg           (mac[nport].audio_cts_up_bound)
#define  HDMI_AUDIO_CTS_LOW_BOUND_reg          (mac[nport].audio_cts_low_bound)
#define  HDMI_AUDIO_N_UP_BOUND_reg             (mac[nport].audio_n_up_bound)
#define  HDMI_AUDIO_N_LOW_BOUND_reg            (mac[nport].audio_n_low_bound)
#define  HDMI_LIGHT_SLEEP_reg                  (mac[nport].light_sleep)
#define  HDMI_READ_MARGIN_ENABLE_reg           (mac[nport].read_margin_enable)
#define  HDMI_READ_MARGIN3_reg                 (mac[nport].read_margin3)
#define  HDMI_READ_MARGIN1_reg                 (mac[nport].read_margin1)
#define  HDMI_READ_MARGIN0_reg                 (mac[nport].read_margin0)
#define  HDMI_BIST_MODE_reg                    (mac[nport].bist_mode)
#define  HDMI_BIST_DONE_reg                    (mac[nport].bist_done)
#define  HDMI_BIST_FAIL_reg                    (mac[nport].bist_fail)
#define  HDMI_DRF_MODE_reg                     (mac[nport].drf_mode)
#define  HDMI_DRF_RESUME_reg                   (mac[nport].drf_resume)
#define  HDMI_DRF_DONE_reg                     (mac[nport].drf_done)
#define  HDMI_DRF_PAUSE_reg                    (mac[nport].drf_pause)
#define  HDMI_DRF_FAIL_reg                     (mac[nport].drf_fail)
#define  HDMI_PHY_FIFO_CR0_reg                 (mac[nport].phy_fifo_cr0)
#define  HDMI_PHY_FIFO_CR1_reg                 (mac[nport].phy_fifo_cr1)
#define  HDMI_PHY_FIFO_SR0_reg                 (mac[nport].phy_fifo_sr0)
#define  HDMI_PHY_FIFO_SR1_reg                 (mac[nport].phy_fifo_sr1)
#define  HDMI_HDMI_CTS_FIFO_CTL_reg            (mac[nport].hdmi_cts_fifo_ctl)
#define  HDMI_CH_CR_reg                        (mac[nport].ch_cr)
#define  HDMI_CH_SR_reg                        (mac[nport].ch_sr)
#define  HDMI_HDMI_2p0_CR_reg                  (mac[nport].hdmi_2p0_cr)
#define  HDMI_HDMI_2p0_CR1_reg                 (mac[nport].hdmi_2p0_cr1)
#define  HDMI_SCR_CR_reg                       (mac[nport].scr_cr)
#define  HDMI_CERCR_reg                        (mac[nport].cercr)
#define  HDMI_CERSR0_reg                       (mac[nport].cersr0)
#define  HDMI_CERSR1_reg                       (mac[nport].cersr1)
#define  HDMI_CERSR2_reg                       (mac[nport].cersr2)
#define  HDMI_CERSR3_reg                       (mac[nport].cersr3)
#define  HDMI_CERSR4_reg                       (mac[nport].cersr4)
#define  HDMI_YUV420_CR_reg                    (mac[nport].yuv420_cr)
#define  HDMI_YUV420_CR1_reg                   (mac[nport].yuv420_cr1)
#define  HDMI_fapa_ct_tmp_reg                  (mac[nport].fapa_ct_tmp)
#define  HDMI_HDR_EM_CT2_reg                   (mac[nport].hdr_em_ct2)
#define  HDMI_HDMI_VSEM_EMC_reg                (mac[nport].hdmi_vsem_emc)
#define  HDMI_HDMI_VSEM_EMC2_reg               (mac[nport].hdmi_vsem_emc2)
#define  HDMI_VSEM_ST_reg                      (mac[nport].vsem_st)
#define  HDMI_HD20_ps_ct_reg                   (mac[nport].hd20_ps_ct)
#define  HDMI_HD20_ps_st_reg                   (mac[nport].hd20_ps_st)
#define  HDMI_irq_all_status_reg               (mac[nport].irq_all_status)
#define  HDMI_xtal_1ms_reg                     (mac[nport].xtal_1ms)
#define  HDMI_xtal_10ms_reg                    (mac[nport].xtal_10ms)
#define  HDMI_ps_measure_ctrl_reg              (mac[nport].ps_measure_ctrl)
#define  HDMI_ps_measure_xtal_clk_reg          (mac[nport].ps_measure_xtal_clk)
#define  HDMI_ps_measure_tmds_clk_reg          (mac[nport].ps_measure_tmds_clk)
#define  HDMI_HDMI_PA_IRQ_EN_reg               (mac[nport].hdmi_pa_irq_en)
#define  HDMI_HDMI_PA_IRQ_STA_reg              (mac[nport].hdmi_pa_irq_sta)
#define  HDMI_HDMI_PA_FRAME_CNT_reg            (mac[nport].hdmi_pa_frame_cnt)

// HDMI scrmble control align
#define  HDMI_SC_ALIGN_B_reg                   (mac[nport].sc_align_b)
#define  HDMI_SC_ALIGN_R_reg                   (mac[nport].sc_align_r)
#define  HDMI_SCR_CTR_PER_LFSR_reg             (mac[nport].scr_ctr_per_lfsr)

// HDCP 1.4
#define  HDMI_HDCP_KEEPOUT_reg                 (mac[nport].hdcp_keepout)
#define  HDMI_HDCP_CR_reg                      (mac[nport].hdcp_cr)
#define  HDMI_HDCP_CR2_reg                     (mac[nport].hdcp_cr2)
#define  HDMI_HDCP_PCR_reg                     (mac[nport].hdcp_pcr)
#define  HDMI_HDCP_FLAG1_reg                   (mac[nport].hdcp_flag1)
#define  HDMI_HDCP_FLAG2_reg                   (mac[nport].hdcp_flag2)

// HDCP 2.2
#define  HDMI_HDCP_2p2_CR_reg                  (mac[nport].hdcp_2p2_cr)
#define  HDMI_HDCP_2p2_ks0_reg                 (mac[nport].hdcp_2p2_ks0)
#define  HDMI_HDCP_2p2_ks1_reg                 (mac[nport].hdcp_2p2_ks1)
#define  HDMI_HDCP_2p2_ks2_reg                 (mac[nport].hdcp_2p2_ks2)
#define  HDMI_HDCP_2p2_ks3_reg                 (mac[nport].hdcp_2p2_ks3)
#define  HDMI_HDCP_2p2_lc0_reg                 (mac[nport].hdcp_2p2_lc0)
#define  HDMI_HDCP_2p2_lc1_reg                 (mac[nport].hdcp_2p2_lc1)
#define  HDMI_HDCP_2p2_lc2_reg                 (mac[nport].hdcp_2p2_lc2)
#define  HDMI_HDCP_2p2_lc3_reg                 (mac[nport].hdcp_2p2_lc3)
#define  HDMI_HDCP_2p2_riv0_reg                (mac[nport].hdcp_2p2_riv0)
#define  HDMI_HDCP_2p2_riv1_reg                (mac[nport].hdcp_2p2_riv1)
#define  HDMI_HDCP_2p2_SR0_reg                 (mac[nport].hdcp_2p2_sr0)
#define  HDMI_HDCP_2p2_SR1_reg                 (mac[nport].hdcp_2p2_sr1)
#define  HDMI_HDCP_HOA_reg                     (mac[nport].hdcp_hoa)
#define  HDMI_HDCP_POWER_SAVING_CTRL_reg       (mac[nport].hdcp_power_saving_ctrl)
#define  HDMI_WIN_OPP_CTRL_reg                 (mac[nport].win_opp_ctl)
#define  HDMI_HDMI_VPLLCR0_reg                 (mac[0].hdmi_vpllcr0)
#define  HDMI_HDMI_VPLLCR1_reg                 (mac[0].hdmi_vpllcr1)
#define  HDMI_HDMI_VPLLCR2_reg                 (mac[0].hdmi_vpllcr2)
#define  HDMI_MN_SCLKG_CTRL_reg                (mac[0].mn_sclkg_ctrl)
#define  HDMI_MN_SCLKG_DIVS_reg                (mac[0].mn_sclkg_divs)
#define  HDMI_MDD_CR_reg                       (mac[0].mdd_cr)
#define  HDMI_MDD_SR_reg                       (mac[0].mdd_sr)
#define  HDMI_FW_FUNC_reg                      (mac[0].fw_func)
#define  HDMI_PORT_SWITCH_reg                  (mac[0].port_switch)
#define  HDMI_POWER_SAVING_reg                 (mac[0].power_saving)
#define  HDMI_CRC_Ctrl_reg                     (mac[0].crc_ctrl)
#define  HDMI_CRC_Result_reg                   (mac[0].crc_result)
#define  HDMI_DES_CRC_reg                      (mac[0].des_crc)
#define  HDMI_CRC_ERR_CNT0_reg                 (mac[0].crc_err_cnt0)
#define  HDMI_hdmi_20_21_ctrl_reg              (mac[0].hdmi_20_21_ctrl)
#define  HDMI_YUV422_CD_reg                    (mac[0].yuv422_cd)
#define  HDMI_ot_reg                           (mac[0].ot)
#define  HDMI_IRQ_reg                          (mac[0].irq)
#define  HDMI_mp_reg                           (mac[0].mp)
#define  HDMI_hdmi_v_bypass_reg                (mac[0].hdmi_v_bypass)
#define  HDMI_hdmi_mp_reg                      (mac[0].hdmi_mp)
#define  HDMI_debug_port_reg                   (mac[0].debug_port)
#define  HDMI_COMMON_PORT_SWITCH_reg           (mac[0].aud_common_port_switch)
#define  HDMI_hdmi_rst0_reg                    (mac[0].hdmi_rst0)
#define  HDMI_hdmi_rst3_reg                    (mac[0].hdmi_rst3)
#define  HDMI_hdmi_clken0_reg                  (mac[0].hdmi_clken0)
#define  HDMI_hdmi_clken3_reg                  (mac[0].hdmi_clken3)

//---------------------------------------------------------------------------------
// Register Macro Wrapper
//---------------------------------------------------------------------------------
#define  HDMI_SOFT_0_i2c_scdc_rst_n_mask                                 HDMI_P0_SOFT_0_i2c_scdc_rst_n_mask
#define  HDMI_SOFT_0_i2c_scdc_rst_n(data)                                HDMI_P0_SOFT_0_i2c_scdc_rst_n(data)
#define  HDMI_SOFT_0_get_i2c_scdc_rst_n(data)                            HDMI_P0_SOFT_0_get_i2c_scdc_rst_n(data)


#define  HDMI_SOFT_1_i2c_scdc_clken_mask                                 HDMI_P0_SOFT_1_i2c_scdc_clken_mask
#define  HDMI_SOFT_1_i2c_scdc_clken(data)                                HDMI_P0_SOFT_1_i2c_scdc_clken(data)
#define  HDMI_SOFT_1_get_i2c_scdc_clken(data)                            HDMI_P0_SOFT_1_get_i2c_scdc_clken(data)


#define  HDMI_PP_TMDS_CRCC_crc_r_en_mask                                 HDMI_P0_PP_TMDS_CRCC_crc_r_en_mask
#define  HDMI_PP_TMDS_CRCC_crc_g_en_mask                                 HDMI_P0_PP_TMDS_CRCC_crc_g_en_mask
#define  HDMI_PP_TMDS_CRCC_crc_b_en_mask                                 HDMI_P0_PP_TMDS_CRCC_crc_b_en_mask
#define  HDMI_PP_TMDS_CRCC_crc_done_mask                                 HDMI_P0_PP_TMDS_CRCC_crc_done_mask
#define  HDMI_PP_TMDS_CRCC_crc_nonstable_mask                            HDMI_P0_PP_TMDS_CRCC_crc_nonstable_mask
#define  HDMI_PP_TMDS_CRCC_crcts_mask                                    HDMI_P0_PP_TMDS_CRCC_crcts_mask
#define  HDMI_PP_TMDS_CRCC_crcc_mask                                     HDMI_P0_PP_TMDS_CRCC_crcc_mask
#define  HDMI_PP_TMDS_CRCC_crc_r_en(data)                                HDMI_P0_PP_TMDS_CRCC_crc_r_en(data)
#define  HDMI_PP_TMDS_CRCC_crc_g_en(data)                                HDMI_P0_PP_TMDS_CRCC_crc_g_en(data)
#define  HDMI_PP_TMDS_CRCC_crc_b_en(data)                                HDMI_P0_PP_TMDS_CRCC_crc_b_en(data)
#define  HDMI_PP_TMDS_CRCC_crc_done(data)                                HDMI_P0_PP_TMDS_CRCC_crc_done(data)
#define  HDMI_PP_TMDS_CRCC_crc_nonstable(data)                           HDMI_P0_PP_TMDS_CRCC_crc_nonstable(data)
#define  HDMI_PP_TMDS_CRCC_crcts(data)                                   HDMI_P0_PP_TMDS_CRCC_crcts(data)
#define  HDMI_PP_TMDS_CRCC_crcc(data)                                    HDMI_P0_PP_TMDS_CRCC_crcc(data)
#define  HDMI_PP_TMDS_CRCC_get_crc_r_en(data)                            HDMI_P0_PP_TMDS_CRCC_get_crc_r_en(data)
#define  HDMI_PP_TMDS_CRCC_get_crc_g_en(data)                            HDMI_P0_PP_TMDS_CRCC_get_crc_g_en(data)
#define  HDMI_PP_TMDS_CRCC_get_crc_b_en(data)                            HDMI_P0_PP_TMDS_CRCC_get_crc_b_en(data)
#define  HDMI_PP_TMDS_CRCC_get_crc_done(data)                            HDMI_P0_PP_TMDS_CRCC_get_crc_done(data)
#define  HDMI_PP_TMDS_CRCC_get_crc_nonstable(data)                       HDMI_P0_PP_TMDS_CRCC_get_crc_nonstable(data)
#define  HDMI_PP_TMDS_CRCC_get_crcts(data)                               HDMI_P0_PP_TMDS_CRCC_get_crcts(data)
#define  HDMI_PP_TMDS_CRCC_get_crcc(data)                                HDMI_P0_PP_TMDS_CRCC_get_crcc(data)


#define  HDMI_PP_TMDS_CRCO0_crcob2_3_mask                                HDMI_P0_PP_TMDS_CRCO0_crcob2_3_mask
#define  HDMI_PP_TMDS_CRCO0_crcob2_2_mask                                HDMI_P0_PP_TMDS_CRCO0_crcob2_2_mask
#define  HDMI_PP_TMDS_CRCO0_crcob2_1_mask                                HDMI_P0_PP_TMDS_CRCO0_crcob2_1_mask
#define  HDMI_PP_TMDS_CRCO0_crcob2_3(data)                               HDMI_P0_PP_TMDS_CRCO0_crcob2_3(data)
#define  HDMI_PP_TMDS_CRCO0_crcob2_2(data)                               HDMI_P0_PP_TMDS_CRCO0_crcob2_2(data)
#define  HDMI_PP_TMDS_CRCO0_crcob2_1(data)                               HDMI_P0_PP_TMDS_CRCO0_crcob2_1(data)
#define  HDMI_PP_TMDS_CRCO0_get_crcob2_3(data)                           HDMI_P0_PP_TMDS_CRCO0_get_crcob2_3(data)
#define  HDMI_PP_TMDS_CRCO0_get_crcob2_2(data)                           HDMI_P0_PP_TMDS_CRCO0_get_crcob2_2(data)
#define  HDMI_PP_TMDS_CRCO0_get_crcob2_1(data)                           HDMI_P0_PP_TMDS_CRCO0_get_crcob2_1(data)


#define  HDMI_PP_TMDS_CRCO1_crcob2_6_mask                                HDMI_P0_PP_TMDS_CRCO1_crcob2_6_mask
#define  HDMI_PP_TMDS_CRCO1_crcob2_5_mask                                HDMI_P0_PP_TMDS_CRCO1_crcob2_5_mask
#define  HDMI_PP_TMDS_CRCO1_crcob2_4_mask                                HDMI_P0_PP_TMDS_CRCO1_crcob2_4_mask
#define  HDMI_PP_TMDS_CRCO1_crcob2_6(data)                               HDMI_P0_PP_TMDS_CRCO1_crcob2_6(data)
#define  HDMI_PP_TMDS_CRCO1_crcob2_5(data)                               HDMI_P0_PP_TMDS_CRCO1_crcob2_5(data)
#define  HDMI_PP_TMDS_CRCO1_crcob2_4(data)                               HDMI_P0_PP_TMDS_CRCO1_crcob2_4(data)
#define  HDMI_PP_TMDS_CRCO1_get_crcob2_6(data)                           HDMI_P0_PP_TMDS_CRCO1_get_crcob2_6(data)
#define  HDMI_PP_TMDS_CRCO1_get_crcob2_5(data)                           HDMI_P0_PP_TMDS_CRCO1_get_crcob2_5(data)
#define  HDMI_PP_TMDS_CRCO1_get_crcob2_4(data)                           HDMI_P0_PP_TMDS_CRCO1_get_crcob2_4(data)


#define  HDMI_TMDS_CTRL_end_phase_mask                                   HDMI_P0_TMDS_CTRL_end_phase_mask
#define  HDMI_TMDS_CTRL_bcd_mask                                         HDMI_P0_TMDS_CTRL_bcd_mask
#define  HDMI_TMDS_CTRL_gcd_mask                                         HDMI_P0_TMDS_CTRL_gcd_mask
#define  HDMI_TMDS_CTRL_rcd_mask                                         HDMI_P0_TMDS_CTRL_rcd_mask
#define  HDMI_TMDS_CTRL_ho_mask                                          HDMI_P0_TMDS_CTRL_ho_mask
#define  HDMI_TMDS_CTRL_yo_mask                                          HDMI_P0_TMDS_CTRL_yo_mask
#define  HDMI_TMDS_CTRL_dummy_2_0_mask                                   HDMI_P0_TMDS_CTRL_dummy_2_0_mask
#define  HDMI_TMDS_CTRL_end_phase(data)                                  HDMI_P0_TMDS_CTRL_end_phase(data)
#define  HDMI_TMDS_CTRL_bcd(data)                                        HDMI_P0_TMDS_CTRL_bcd(data)
#define  HDMI_TMDS_CTRL_gcd(data)                                        HDMI_P0_TMDS_CTRL_gcd(data)
#define  HDMI_TMDS_CTRL_rcd(data)                                        HDMI_P0_TMDS_CTRL_rcd(data)
#define  HDMI_TMDS_CTRL_ho(data)                                         HDMI_P0_TMDS_CTRL_ho(data)
#define  HDMI_TMDS_CTRL_yo(data)                                         HDMI_P0_TMDS_CTRL_yo(data)
#define  HDMI_TMDS_CTRL_dummy_2_0(data)                                  HDMI_P0_TMDS_CTRL_dummy_2_0(data)
#define  HDMI_TMDS_CTRL_get_end_phase(data)                              HDMI_P0_TMDS_CTRL_get_end_phase(data)
#define  HDMI_TMDS_CTRL_get_bcd(data)                                    HDMI_P0_TMDS_CTRL_get_bcd(data)
#define  HDMI_TMDS_CTRL_get_gcd(data)                                    HDMI_P0_TMDS_CTRL_get_gcd(data)
#define  HDMI_TMDS_CTRL_get_rcd(data)                                    HDMI_P0_TMDS_CTRL_get_rcd(data)
#define  HDMI_TMDS_CTRL_get_ho(data)                                     HDMI_P0_TMDS_CTRL_get_ho(data)
#define  HDMI_TMDS_CTRL_get_yo(data)                                     HDMI_P0_TMDS_CTRL_get_yo(data)
#define  HDMI_TMDS_CTRL_get_dummy_2_0(data)                              HDMI_P0_TMDS_CTRL_get_dummy_2_0(data)


#define  HDMI_TMDS_OUTCTL_trcoe_r_mask                                   HDMI_P0_TMDS_OUTCTL_trcoe_r_mask
#define  HDMI_TMDS_OUTCTL_tgcoe_r_mask                                   HDMI_P0_TMDS_OUTCTL_tgcoe_r_mask
#define  HDMI_TMDS_OUTCTL_tbcoe_r_mask                                   HDMI_P0_TMDS_OUTCTL_tbcoe_r_mask
#define  HDMI_TMDS_OUTCTL_ocke_r_mask                                    HDMI_P0_TMDS_OUTCTL_ocke_r_mask
#define  HDMI_TMDS_OUTCTL_aoe_mask                                       HDMI_P0_TMDS_OUTCTL_aoe_mask
#define  HDMI_TMDS_OUTCTL_trcoe_mask                                     HDMI_P0_TMDS_OUTCTL_trcoe_mask
#define  HDMI_TMDS_OUTCTL_tgcoe_mask                                     HDMI_P0_TMDS_OUTCTL_tgcoe_mask
#define  HDMI_TMDS_OUTCTL_tbcoe_mask                                     HDMI_P0_TMDS_OUTCTL_tbcoe_mask
#define  HDMI_TMDS_OUTCTL_ocke_mask                                      HDMI_P0_TMDS_OUTCTL_ocke_mask
#define  HDMI_TMDS_OUTCTL_ockie_mask                                     HDMI_P0_TMDS_OUTCTL_ockie_mask
#define  HDMI_TMDS_OUTCTL_focke_mask                                     HDMI_P0_TMDS_OUTCTL_focke_mask
#define  HDMI_TMDS_OUTCTL_cbus_dbg_cke_mask                              HDMI_P0_TMDS_OUTCTL_cbus_dbg_cke_mask
#define  HDMI_TMDS_OUTCTL_dummy_1_0_mask                                 HDMI_P0_TMDS_OUTCTL_dummy_1_0_mask
#define  HDMI_TMDS_OUTCTL_trcoe_r(data)                                  HDMI_P0_TMDS_OUTCTL_trcoe_r(data)
#define  HDMI_TMDS_OUTCTL_tgcoe_r(data)                                  HDMI_P0_TMDS_OUTCTL_tgcoe_r(data)
#define  HDMI_TMDS_OUTCTL_tbcoe_r(data)                                  HDMI_P0_TMDS_OUTCTL_tbcoe_r(data)
#define  HDMI_TMDS_OUTCTL_ocke_r(data)                                   HDMI_P0_TMDS_OUTCTL_ocke_r(data)
#define  HDMI_TMDS_OUTCTL_aoe(data)                                      HDMI_P0_TMDS_OUTCTL_aoe(data)
#define  HDMI_TMDS_OUTCTL_trcoe(data)                                    HDMI_P0_TMDS_OUTCTL_trcoe(data)
#define  HDMI_TMDS_OUTCTL_tgcoe(data)                                    HDMI_P0_TMDS_OUTCTL_tgcoe(data)
#define  HDMI_TMDS_OUTCTL_tbcoe(data)                                    HDMI_P0_TMDS_OUTCTL_tbcoe(data)
#define  HDMI_TMDS_OUTCTL_ocke(data)                                     HDMI_P0_TMDS_OUTCTL_ocke(data)
#define  HDMI_TMDS_OUTCTL_ockie(data)                                    HDMI_P0_TMDS_OUTCTL_ockie(data)
#define  HDMI_TMDS_OUTCTL_focke(data)                                    HDMI_P0_TMDS_OUTCTL_focke(data)
#define  HDMI_TMDS_OUTCTL_cbus_dbg_cke(data)                             HDMI_P0_TMDS_OUTCTL_cbus_dbg_cke(data)
#define  HDMI_TMDS_OUTCTL_dummy_1_0(data)                                HDMI_P0_TMDS_OUTCTL_dummy_1_0(data)
#define  HDMI_TMDS_OUTCTL_get_trcoe_r(data)                              HDMI_P0_TMDS_OUTCTL_get_trcoe_r(data)
#define  HDMI_TMDS_OUTCTL_get_tgcoe_r(data)                              HDMI_P0_TMDS_OUTCTL_get_tgcoe_r(data)
#define  HDMI_TMDS_OUTCTL_get_tbcoe_r(data)                              HDMI_P0_TMDS_OUTCTL_get_tbcoe_r(data)
#define  HDMI_TMDS_OUTCTL_get_ocke_r(data)                               HDMI_P0_TMDS_OUTCTL_get_ocke_r(data)
#define  HDMI_TMDS_OUTCTL_get_aoe(data)                                  HDMI_P0_TMDS_OUTCTL_get_aoe(data)
#define  HDMI_TMDS_OUTCTL_get_trcoe(data)                                HDMI_P0_TMDS_OUTCTL_get_trcoe(data)
#define  HDMI_TMDS_OUTCTL_get_tgcoe(data)                                HDMI_P0_TMDS_OUTCTL_get_tgcoe(data)
#define  HDMI_TMDS_OUTCTL_get_tbcoe(data)                                HDMI_P0_TMDS_OUTCTL_get_tbcoe(data)
#define  HDMI_TMDS_OUTCTL_get_ocke(data)                                 HDMI_P0_TMDS_OUTCTL_get_ocke(data)
#define  HDMI_TMDS_OUTCTL_get_ockie(data)                                HDMI_P0_TMDS_OUTCTL_get_ockie(data)
#define  HDMI_TMDS_OUTCTL_get_focke(data)                                HDMI_P0_TMDS_OUTCTL_get_focke(data)
#define  HDMI_TMDS_OUTCTL_get_cbus_dbg_cke(data)                         HDMI_P0_TMDS_OUTCTL_get_cbus_dbg_cke(data)
#define  HDMI_TMDS_OUTCTL_get_dummy_1_0(data)                            HDMI_P0_TMDS_OUTCTL_get_dummy_1_0(data)


#define  HDMI_TMDS_PWDCTL_ecc_r_mask                                     HDMI_P0_TMDS_PWDCTL_ecc_r_mask
#define  HDMI_TMDS_PWDCTL_erip_r_mask                                    HDMI_P0_TMDS_PWDCTL_erip_r_mask
#define  HDMI_TMDS_PWDCTL_egip_r_mask                                    HDMI_P0_TMDS_PWDCTL_egip_r_mask
#define  HDMI_TMDS_PWDCTL_ebip_r_mask                                    HDMI_P0_TMDS_PWDCTL_ebip_r_mask
#define  HDMI_TMDS_PWDCTL_dummy_27_10_mask                               HDMI_P0_TMDS_PWDCTL_dummy_27_10_mask
#define  HDMI_TMDS_PWDCTL_video_preamble_off_en_mask                     HDMI_P0_TMDS_PWDCTL_video_preamble_off_en_mask
#define  HDMI_TMDS_PWDCTL_hs_width_sel_mask                              HDMI_P0_TMDS_PWDCTL_hs_width_sel_mask
#define  HDMI_TMDS_PWDCTL_deo_mask                                       HDMI_P0_TMDS_PWDCTL_deo_mask
#define  HDMI_TMDS_PWDCTL_brcw_mask                                      HDMI_P0_TMDS_PWDCTL_brcw_mask
#define  HDMI_TMDS_PWDCTL_pnsw_mask                                      HDMI_P0_TMDS_PWDCTL_pnsw_mask
#define  HDMI_TMDS_PWDCTL_iccaf_mask                                     HDMI_P0_TMDS_PWDCTL_iccaf_mask
#define  HDMI_TMDS_PWDCTL_ecc_mask                                       HDMI_P0_TMDS_PWDCTL_ecc_mask
#define  HDMI_TMDS_PWDCTL_erip_mask                                      HDMI_P0_TMDS_PWDCTL_erip_mask
#define  HDMI_TMDS_PWDCTL_egip_mask                                      HDMI_P0_TMDS_PWDCTL_egip_mask
#define  HDMI_TMDS_PWDCTL_ebip_mask                                      HDMI_P0_TMDS_PWDCTL_ebip_mask
#define  HDMI_TMDS_PWDCTL_ecc_r(data)                                    HDMI_P0_TMDS_PWDCTL_ecc_r(data)
#define  HDMI_TMDS_PWDCTL_erip_r(data)                                   HDMI_P0_TMDS_PWDCTL_erip_r(data)
#define  HDMI_TMDS_PWDCTL_egip_r(data)                                   HDMI_P0_TMDS_PWDCTL_egip_r(data)
#define  HDMI_TMDS_PWDCTL_ebip_r(data)                                   HDMI_P0_TMDS_PWDCTL_ebip_r(data)
#define  HDMI_TMDS_PWDCTL_dummy_27_10(data)                              HDMI_P0_TMDS_PWDCTL_dummy_27_10(data)
#define  HDMI_TMDS_PWDCTL_video_preamble_off_en(data)                    HDMI_P0_TMDS_PWDCTL_video_preamble_off_en(data)
#define  HDMI_TMDS_PWDCTL_hs_width_sel(data)                             HDMI_P0_TMDS_PWDCTL_hs_width_sel(data)
#define  HDMI_TMDS_PWDCTL_deo(data)                                      HDMI_P0_TMDS_PWDCTL_deo(data)
#define  HDMI_TMDS_PWDCTL_brcw(data)                                     HDMI_P0_TMDS_PWDCTL_brcw(data)
#define  HDMI_TMDS_PWDCTL_pnsw(data)                                     HDMI_P0_TMDS_PWDCTL_pnsw(data)
#define  HDMI_TMDS_PWDCTL_iccaf(data)                                    HDMI_P0_TMDS_PWDCTL_iccaf(data)
#define  HDMI_TMDS_PWDCTL_ecc(data)                                      HDMI_P0_TMDS_PWDCTL_ecc(data)
#define  HDMI_TMDS_PWDCTL_erip(data)                                     HDMI_P0_TMDS_PWDCTL_erip(data)
#define  HDMI_TMDS_PWDCTL_egip(data)                                     HDMI_P0_TMDS_PWDCTL_egip(data)
#define  HDMI_TMDS_PWDCTL_ebip(data)                                     HDMI_P0_TMDS_PWDCTL_ebip(data)
#define  HDMI_TMDS_PWDCTL_get_ecc_r(data)                                HDMI_P0_TMDS_PWDCTL_get_ecc_r(data)
#define  HDMI_TMDS_PWDCTL_get_erip_r(data)                               HDMI_P0_TMDS_PWDCTL_get_erip_r(data)
#define  HDMI_TMDS_PWDCTL_get_egip_r(data)                               HDMI_P0_TMDS_PWDCTL_get_egip_r(data)
#define  HDMI_TMDS_PWDCTL_get_ebip_r(data)                               HDMI_P0_TMDS_PWDCTL_get_ebip_r(data)
#define  HDMI_TMDS_PWDCTL_get_dummy_27_10(data)                          HDMI_P0_TMDS_PWDCTL_get_dummy_27_10(data)
#define  HDMI_TMDS_PWDCTL_get_video_preamble_off_en(data)                HDMI_P0_TMDS_PWDCTL_get_video_preamble_off_en(data)
#define  HDMI_TMDS_PWDCTL_get_hs_width_sel(data)                         HDMI_P0_TMDS_PWDCTL_get_hs_width_sel(data)
#define  HDMI_TMDS_PWDCTL_get_deo(data)                                  HDMI_P0_TMDS_PWDCTL_get_deo(data)
#define  HDMI_TMDS_PWDCTL_get_brcw(data)                                 HDMI_P0_TMDS_PWDCTL_get_brcw(data)
#define  HDMI_TMDS_PWDCTL_get_pnsw(data)                                 HDMI_P0_TMDS_PWDCTL_get_pnsw(data)
#define  HDMI_TMDS_PWDCTL_get_iccaf(data)                                HDMI_P0_TMDS_PWDCTL_get_iccaf(data)
#define  HDMI_TMDS_PWDCTL_get_ecc(data)                                  HDMI_P0_TMDS_PWDCTL_get_ecc(data)
#define  HDMI_TMDS_PWDCTL_get_erip(data)                                 HDMI_P0_TMDS_PWDCTL_get_erip(data)
#define  HDMI_TMDS_PWDCTL_get_egip(data)                                 HDMI_P0_TMDS_PWDCTL_get_egip(data)
#define  HDMI_TMDS_PWDCTL_get_ebip(data)                                 HDMI_P0_TMDS_PWDCTL_get_ebip(data)


#define  HDMI_TMDS_Z0CC_hde_mask                                         HDMI_P0_TMDS_Z0CC_hde_mask
#define  HDMI_TMDS_Z0CC_hde(data)                                        HDMI_P0_TMDS_Z0CC_hde(data)
#define  HDMI_TMDS_Z0CC_get_hde(data)                                    HDMI_P0_TMDS_Z0CC_get_hde(data)


#define  HDMI_TMDS_CPS_pk_gb_num_mask                                    HDMI_P0_TMDS_CPS_pk_gb_num_mask
#define  HDMI_TMDS_CPS_vd_gb_num_mask                                    HDMI_P0_TMDS_CPS_vd_gb_num_mask
#define  HDMI_TMDS_CPS_pk_pre_num_mask                                   HDMI_P0_TMDS_CPS_pk_pre_num_mask
#define  HDMI_TMDS_CPS_vd_pre_num_mask                                   HDMI_P0_TMDS_CPS_vd_pre_num_mask
#define  HDMI_TMDS_CPS_pll_div2_en_mask                                  HDMI_P0_TMDS_CPS_pll_div2_en_mask
#define  HDMI_TMDS_CPS_no_clk_in_mask                                    HDMI_P0_TMDS_CPS_no_clk_in_mask
#define  HDMI_TMDS_CPS_de_inv_disable_mask                               HDMI_P0_TMDS_CPS_de_inv_disable_mask
#define  HDMI_TMDS_CPS_clr_infoframe_dvi_mask                            HDMI_P0_TMDS_CPS_clr_infoframe_dvi_mask
#define  HDMI_TMDS_CPS_auto_dvi2hdmi_mask                                HDMI_P0_TMDS_CPS_auto_dvi2hdmi_mask
#define  HDMI_TMDS_CPS_dummy_1_0_mask                                    HDMI_P0_TMDS_CPS_dummy_1_0_mask
#define  HDMI_TMDS_CPS_pk_gb_num(data)                                   HDMI_P0_TMDS_CPS_pk_gb_num(data)
#define  HDMI_TMDS_CPS_vd_gb_num(data)                                   HDMI_P0_TMDS_CPS_vd_gb_num(data)
#define  HDMI_TMDS_CPS_pk_pre_num(data)                                  HDMI_P0_TMDS_CPS_pk_pre_num(data)
#define  HDMI_TMDS_CPS_vd_pre_num(data)                                  HDMI_P0_TMDS_CPS_vd_pre_num(data)
#define  HDMI_TMDS_CPS_pll_div2_en(data)                                 HDMI_P0_TMDS_CPS_pll_div2_en(data)
#define  HDMI_TMDS_CPS_no_clk_in(data)                                   HDMI_P0_TMDS_CPS_no_clk_in(data)
#define  HDMI_TMDS_CPS_de_inv_disable(data)                              HDMI_P0_TMDS_CPS_de_inv_disable(data)
#define  HDMI_TMDS_CPS_clr_infoframe_dvi(data)                           HDMI_P0_TMDS_CPS_clr_infoframe_dvi(data)
#define  HDMI_TMDS_CPS_auto_dvi2hdmi(data)                               HDMI_P0_TMDS_CPS_auto_dvi2hdmi(data)
#define  HDMI_TMDS_CPS_dummy_1_0(data)                                   HDMI_P0_TMDS_CPS_dummy_1_0(data)
#define  HDMI_TMDS_CPS_get_pk_gb_num(data)                               HDMI_P0_TMDS_CPS_get_pk_gb_num(data)
#define  HDMI_TMDS_CPS_get_vd_gb_num(data)                               HDMI_P0_TMDS_CPS_get_vd_gb_num(data)
#define  HDMI_TMDS_CPS_get_pk_pre_num(data)                              HDMI_P0_TMDS_CPS_get_pk_pre_num(data)
#define  HDMI_TMDS_CPS_get_vd_pre_num(data)                              HDMI_P0_TMDS_CPS_get_vd_pre_num(data)
#define  HDMI_TMDS_CPS_get_pll_div2_en(data)                             HDMI_P0_TMDS_CPS_get_pll_div2_en(data)
#define  HDMI_TMDS_CPS_get_no_clk_in(data)                               HDMI_P0_TMDS_CPS_get_no_clk_in(data)
#define  HDMI_TMDS_CPS_get_de_inv_disable(data)                          HDMI_P0_TMDS_CPS_get_de_inv_disable(data)
#define  HDMI_TMDS_CPS_get_clr_infoframe_dvi(data)                       HDMI_P0_TMDS_CPS_get_clr_infoframe_dvi(data)
#define  HDMI_TMDS_CPS_get_auto_dvi2hdmi(data)                           HDMI_P0_TMDS_CPS_get_auto_dvi2hdmi(data)
#define  HDMI_TMDS_CPS_get_dummy_1_0(data)                               HDMI_P0_TMDS_CPS_get_dummy_1_0(data)


#define  HDMI_TMDS_UDC_debug_sel_mask                                    HDMI_P0_TMDS_UDC_debug_sel_mask
#define  HDMI_TMDS_UDC_dummy_3_mask                                      HDMI_P0_TMDS_UDC_dummy_3_mask
#define  HDMI_TMDS_UDC_debug_sel(data)                                   HDMI_P0_TMDS_UDC_debug_sel(data)
#define  HDMI_TMDS_UDC_dummy_3(data)                                     HDMI_P0_TMDS_UDC_dummy_3(data)
#define  HDMI_TMDS_UDC_get_debug_sel(data)                               HDMI_P0_TMDS_UDC_get_debug_sel(data)
#define  HDMI_TMDS_UDC_get_dummy_3(data)                                 HDMI_P0_TMDS_UDC_get_dummy_3(data)


#define  HDMI_TMDS_ERRC_dc_mask                                          HDMI_P0_TMDS_ERRC_dc_mask
#define  HDMI_TMDS_ERRC_nl_mask                                          HDMI_P0_TMDS_ERRC_nl_mask
#define  HDMI_TMDS_ERRC_dc(data)                                         HDMI_P0_TMDS_ERRC_dc(data)
#define  HDMI_TMDS_ERRC_nl(data)                                         HDMI_P0_TMDS_ERRC_nl(data)
#define  HDMI_TMDS_ERRC_get_dc(data)                                     HDMI_P0_TMDS_ERRC_get_dc(data)
#define  HDMI_TMDS_ERRC_get_nl(data)                                     HDMI_P0_TMDS_ERRC_get_nl(data)


#define  HDMI_TMDS_OUT_CTRL_tmds_bypass_mask                             HDMI_P0_TMDS_OUT_CTRL_tmds_bypass_mask
#define  HDMI_TMDS_OUT_CTRL_dummy_6_0_mask                               HDMI_P0_TMDS_OUT_CTRL_dummy_6_0_mask
#define  HDMI_TMDS_OUT_CTRL_tmds_bypass(data)                            HDMI_P0_TMDS_OUT_CTRL_tmds_bypass(data)
#define  HDMI_TMDS_OUT_CTRL_dummy_6_0(data)                              HDMI_P0_TMDS_OUT_CTRL_dummy_6_0(data)
#define  HDMI_TMDS_OUT_CTRL_get_tmds_bypass(data)                        HDMI_P0_TMDS_OUT_CTRL_get_tmds_bypass(data)
#define  HDMI_TMDS_OUT_CTRL_get_dummy_6_0(data)                          HDMI_P0_TMDS_OUT_CTRL_get_dummy_6_0(data)


#define  HDMI_TMDS_ROUT_tmds_rout_h_mask                                 HDMI_P0_TMDS_ROUT_tmds_rout_h_mask
#define  HDMI_TMDS_ROUT_tmds_rout_l_mask                                 HDMI_P0_TMDS_ROUT_tmds_rout_l_mask
#define  HDMI_TMDS_ROUT_tmds_rout_h(data)                                HDMI_P0_TMDS_ROUT_tmds_rout_h(data)
#define  HDMI_TMDS_ROUT_tmds_rout_l(data)                                HDMI_P0_TMDS_ROUT_tmds_rout_l(data)
#define  HDMI_TMDS_ROUT_get_tmds_rout_h(data)                            HDMI_P0_TMDS_ROUT_get_tmds_rout_h(data)
#define  HDMI_TMDS_ROUT_get_tmds_rout_l(data)                            HDMI_P0_TMDS_ROUT_get_tmds_rout_l(data)


#define  HDMI_TMDS_GOUT_tmds_gout_h_mask                                 HDMI_P0_TMDS_GOUT_tmds_gout_h_mask
#define  HDMI_TMDS_GOUT_tmds_gout_l_mask                                 HDMI_P0_TMDS_GOUT_tmds_gout_l_mask
#define  HDMI_TMDS_GOUT_tmds_gout_h(data)                                HDMI_P0_TMDS_GOUT_tmds_gout_h(data)
#define  HDMI_TMDS_GOUT_tmds_gout_l(data)                                HDMI_P0_TMDS_GOUT_tmds_gout_l(data)
#define  HDMI_TMDS_GOUT_get_tmds_gout_h(data)                            HDMI_P0_TMDS_GOUT_get_tmds_gout_h(data)
#define  HDMI_TMDS_GOUT_get_tmds_gout_l(data)                            HDMI_P0_TMDS_GOUT_get_tmds_gout_l(data)


#define  HDMI_TMDS_BOUT_tmds_bout_h_mask                                 HDMI_P0_TMDS_BOUT_tmds_bout_h_mask
#define  HDMI_TMDS_BOUT_tmds_bout_l_mask                                 HDMI_P0_TMDS_BOUT_tmds_bout_l_mask
#define  HDMI_TMDS_BOUT_tmds_bout_h(data)                                HDMI_P0_TMDS_BOUT_tmds_bout_h(data)
#define  HDMI_TMDS_BOUT_tmds_bout_l(data)                                HDMI_P0_TMDS_BOUT_tmds_bout_l(data)
#define  HDMI_TMDS_BOUT_get_tmds_bout_h(data)                            HDMI_P0_TMDS_BOUT_get_tmds_bout_h(data)
#define  HDMI_TMDS_BOUT_get_tmds_bout_l(data)                            HDMI_P0_TMDS_BOUT_get_tmds_bout_l(data)


#define  HDMI_TMDS_DPC0_dpc_pp_valid_mask                                HDMI_P0_TMDS_DPC0_dpc_pp_valid_mask
#define  HDMI_TMDS_DPC0_dpc_default_ph_mask                              HDMI_P0_TMDS_DPC0_dpc_default_ph_mask
#define  HDMI_TMDS_DPC0_dpc_pp_mask                                      HDMI_P0_TMDS_DPC0_dpc_pp_mask
#define  HDMI_TMDS_DPC0_dpc_cd_mask                                      HDMI_P0_TMDS_DPC0_dpc_cd_mask
#define  HDMI_TMDS_DPC0_dpc_pp_valid(data)                               HDMI_P0_TMDS_DPC0_dpc_pp_valid(data)
#define  HDMI_TMDS_DPC0_dpc_default_ph(data)                             HDMI_P0_TMDS_DPC0_dpc_default_ph(data)
#define  HDMI_TMDS_DPC0_dpc_pp(data)                                     HDMI_P0_TMDS_DPC0_dpc_pp(data)
#define  HDMI_TMDS_DPC0_dpc_cd(data)                                     HDMI_P0_TMDS_DPC0_dpc_cd(data)
#define  HDMI_TMDS_DPC0_get_dpc_pp_valid(data)                           HDMI_P0_TMDS_DPC0_get_dpc_pp_valid(data)
#define  HDMI_TMDS_DPC0_get_dpc_default_ph(data)                         HDMI_P0_TMDS_DPC0_get_dpc_default_ph(data)
#define  HDMI_TMDS_DPC0_get_dpc_pp(data)                                 HDMI_P0_TMDS_DPC0_get_dpc_pp(data)
#define  HDMI_TMDS_DPC0_get_dpc_cd(data)                                 HDMI_P0_TMDS_DPC0_get_dpc_cd(data)


#define  HDMI_TMDS_DPC1_dpc_cd_chg_flag_mask                             HDMI_P0_TMDS_DPC1_dpc_cd_chg_flag_mask
#define  HDMI_TMDS_DPC1_dpc_cd_chg_int_en_mask                           HDMI_P0_TMDS_DPC1_dpc_cd_chg_int_en_mask
#define  HDMI_TMDS_DPC1_dummy_15_11_mask                                 HDMI_P0_TMDS_DPC1_dummy_15_11_mask
#define  HDMI_TMDS_DPC1_dpc_auto_mask                                    HDMI_P0_TMDS_DPC1_dpc_auto_mask
#define  HDMI_TMDS_DPC1_dpc_pp_valid_fw_mask                             HDMI_P0_TMDS_DPC1_dpc_pp_valid_fw_mask
#define  HDMI_TMDS_DPC1_dpc_default_ph_fw_mask                           HDMI_P0_TMDS_DPC1_dpc_default_ph_fw_mask
#define  HDMI_TMDS_DPC1_dpc_pp_fw_mask                                   HDMI_P0_TMDS_DPC1_dpc_pp_fw_mask
#define  HDMI_TMDS_DPC1_dpc_cd_fw_mask                                   HDMI_P0_TMDS_DPC1_dpc_cd_fw_mask
#define  HDMI_TMDS_DPC1_dpc_cd_chg_flag(data)                            HDMI_P0_TMDS_DPC1_dpc_cd_chg_flag(data)
#define  HDMI_TMDS_DPC1_dpc_cd_chg_int_en(data)                          HDMI_P0_TMDS_DPC1_dpc_cd_chg_int_en(data)
#define  HDMI_TMDS_DPC1_dummy_15_11(data)                                HDMI_P0_TMDS_DPC1_dummy_15_11(data)
#define  HDMI_TMDS_DPC1_dpc_auto(data)                                   HDMI_P0_TMDS_DPC1_dpc_auto(data)
#define  HDMI_TMDS_DPC1_dpc_pp_valid_fw(data)                            HDMI_P0_TMDS_DPC1_dpc_pp_valid_fw(data)
#define  HDMI_TMDS_DPC1_dpc_default_ph_fw(data)                          HDMI_P0_TMDS_DPC1_dpc_default_ph_fw(data)
#define  HDMI_TMDS_DPC1_dpc_pp_fw(data)                                  HDMI_P0_TMDS_DPC1_dpc_pp_fw(data)
#define  HDMI_TMDS_DPC1_dpc_cd_fw(data)                                  HDMI_P0_TMDS_DPC1_dpc_cd_fw(data)
#define  HDMI_TMDS_DPC1_get_dpc_cd_chg_flag(data)                        HDMI_P0_TMDS_DPC1_get_dpc_cd_chg_flag(data)
#define  HDMI_TMDS_DPC1_get_dpc_cd_chg_int_en(data)                      HDMI_P0_TMDS_DPC1_get_dpc_cd_chg_int_en(data)
#define  HDMI_TMDS_DPC1_get_dummy_15_11(data)                            HDMI_P0_TMDS_DPC1_get_dummy_15_11(data)
#define  HDMI_TMDS_DPC1_get_dpc_auto(data)                               HDMI_P0_TMDS_DPC1_get_dpc_auto(data)
#define  HDMI_TMDS_DPC1_get_dpc_pp_valid_fw(data)                        HDMI_P0_TMDS_DPC1_get_dpc_pp_valid_fw(data)
#define  HDMI_TMDS_DPC1_get_dpc_default_ph_fw(data)                      HDMI_P0_TMDS_DPC1_get_dpc_default_ph_fw(data)
#define  HDMI_TMDS_DPC1_get_dpc_pp_fw(data)                              HDMI_P0_TMDS_DPC1_get_dpc_pp_fw(data)
#define  HDMI_TMDS_DPC1_get_dpc_cd_fw(data)                              HDMI_P0_TMDS_DPC1_get_dpc_cd_fw(data)


#define  HDMI_TMDS_DPC_SET0_dpc_bypass_dis_mask                          HDMI_P0_TMDS_DPC_SET0_dpc_bypass_dis_mask
#define  HDMI_TMDS_DPC_SET0_dpc_en_mask                                  HDMI_P0_TMDS_DPC_SET0_dpc_en_mask
#define  HDMI_TMDS_DPC_SET0_phase_errcnt_in_mask                         HDMI_P0_TMDS_DPC_SET0_phase_errcnt_in_mask
#define  HDMI_TMDS_DPC_SET0_phase_clrcnt_in_mask                         HDMI_P0_TMDS_DPC_SET0_phase_clrcnt_in_mask
#define  HDMI_TMDS_DPC_SET0_phase_clr_sel_mask                           HDMI_P0_TMDS_DPC_SET0_phase_clr_sel_mask
#define  HDMI_TMDS_DPC_SET0_dpc_bypass_dis(data)                         HDMI_P0_TMDS_DPC_SET0_dpc_bypass_dis(data)
#define  HDMI_TMDS_DPC_SET0_dpc_en(data)                                 HDMI_P0_TMDS_DPC_SET0_dpc_en(data)
#define  HDMI_TMDS_DPC_SET0_phase_errcnt_in(data)                        HDMI_P0_TMDS_DPC_SET0_phase_errcnt_in(data)
#define  HDMI_TMDS_DPC_SET0_phase_clrcnt_in(data)                        HDMI_P0_TMDS_DPC_SET0_phase_clrcnt_in(data)
#define  HDMI_TMDS_DPC_SET0_phase_clr_sel(data)                          HDMI_P0_TMDS_DPC_SET0_phase_clr_sel(data)
#define  HDMI_TMDS_DPC_SET0_get_dpc_bypass_dis(data)                     HDMI_P0_TMDS_DPC_SET0_get_dpc_bypass_dis(data)
#define  HDMI_TMDS_DPC_SET0_get_dpc_en(data)                             HDMI_P0_TMDS_DPC_SET0_get_dpc_en(data)
#define  HDMI_TMDS_DPC_SET0_get_phase_errcnt_in(data)                    HDMI_P0_TMDS_DPC_SET0_get_phase_errcnt_in(data)
#define  HDMI_TMDS_DPC_SET0_get_phase_clrcnt_in(data)                    HDMI_P0_TMDS_DPC_SET0_get_phase_clrcnt_in(data)
#define  HDMI_TMDS_DPC_SET0_get_phase_clr_sel(data)                      HDMI_P0_TMDS_DPC_SET0_get_phase_clr_sel(data)


#define  HDMI_TMDS_DPC_SET1_set_full_noti_mask                           HDMI_P0_TMDS_DPC_SET1_set_full_noti_mask
#define  HDMI_TMDS_DPC_SET1_set_empty_noti_mask                          HDMI_P0_TMDS_DPC_SET1_set_empty_noti_mask
#define  HDMI_TMDS_DPC_SET1_set_full_noti(data)                          HDMI_P0_TMDS_DPC_SET1_set_full_noti(data)
#define  HDMI_TMDS_DPC_SET1_set_empty_noti(data)                         HDMI_P0_TMDS_DPC_SET1_set_empty_noti(data)
#define  HDMI_TMDS_DPC_SET1_get_set_full_noti(data)                      HDMI_P0_TMDS_DPC_SET1_get_set_full_noti(data)
#define  HDMI_TMDS_DPC_SET1_get_set_empty_noti(data)                     HDMI_P0_TMDS_DPC_SET1_get_set_empty_noti(data)


#define  HDMI_TMDS_DPC_SET2_fifo_errcnt_in_mask                          HDMI_P0_TMDS_DPC_SET2_fifo_errcnt_in_mask
#define  HDMI_TMDS_DPC_SET2_clr_phase_flag_mask                          HDMI_P0_TMDS_DPC_SET2_clr_phase_flag_mask
#define  HDMI_TMDS_DPC_SET2_clr_fifo_flag_mask                           HDMI_P0_TMDS_DPC_SET2_clr_fifo_flag_mask
#define  HDMI_TMDS_DPC_SET2_dpc_phase_ok_mask                            HDMI_P0_TMDS_DPC_SET2_dpc_phase_ok_mask
#define  HDMI_TMDS_DPC_SET2_dpc_phase_err_flag_mask                      HDMI_P0_TMDS_DPC_SET2_dpc_phase_err_flag_mask
#define  HDMI_TMDS_DPC_SET2_dpc_fifo_err_flag_mask                       HDMI_P0_TMDS_DPC_SET2_dpc_fifo_err_flag_mask
#define  HDMI_TMDS_DPC_SET2_fifo_errcnt_in(data)                         HDMI_P0_TMDS_DPC_SET2_fifo_errcnt_in(data)
#define  HDMI_TMDS_DPC_SET2_clr_phase_flag(data)                         HDMI_P0_TMDS_DPC_SET2_clr_phase_flag(data)
#define  HDMI_TMDS_DPC_SET2_clr_fifo_flag(data)                          HDMI_P0_TMDS_DPC_SET2_clr_fifo_flag(data)
#define  HDMI_TMDS_DPC_SET2_dpc_phase_ok(data)                           HDMI_P0_TMDS_DPC_SET2_dpc_phase_ok(data)
#define  HDMI_TMDS_DPC_SET2_dpc_phase_err_flag(data)                     HDMI_P0_TMDS_DPC_SET2_dpc_phase_err_flag(data)
#define  HDMI_TMDS_DPC_SET2_dpc_fifo_err_flag(data)                      HDMI_P0_TMDS_DPC_SET2_dpc_fifo_err_flag(data)
#define  HDMI_TMDS_DPC_SET2_get_fifo_errcnt_in(data)                     HDMI_P0_TMDS_DPC_SET2_get_fifo_errcnt_in(data)
#define  HDMI_TMDS_DPC_SET2_get_clr_phase_flag(data)                     HDMI_P0_TMDS_DPC_SET2_get_clr_phase_flag(data)
#define  HDMI_TMDS_DPC_SET2_get_clr_fifo_flag(data)                      HDMI_P0_TMDS_DPC_SET2_get_clr_fifo_flag(data)
#define  HDMI_TMDS_DPC_SET2_get_dpc_phase_ok(data)                       HDMI_P0_TMDS_DPC_SET2_get_dpc_phase_ok(data)
#define  HDMI_TMDS_DPC_SET2_get_dpc_phase_err_flag(data)                 HDMI_P0_TMDS_DPC_SET2_get_dpc_phase_err_flag(data)
#define  HDMI_TMDS_DPC_SET2_get_dpc_fifo_err_flag(data)                  HDMI_P0_TMDS_DPC_SET2_get_dpc_fifo_err_flag(data)


#define  HDMI_TMDS_DPC_SET3_dpc_fifo_over_flag_mask                      HDMI_P0_TMDS_DPC_SET3_dpc_fifo_over_flag_mask
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_under_flag_mask                     HDMI_P0_TMDS_DPC_SET3_dpc_fifo_under_flag_mask
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_over_xflag_mask                     HDMI_P0_TMDS_DPC_SET3_dpc_fifo_over_xflag_mask
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_under_xflag_mask                    HDMI_P0_TMDS_DPC_SET3_dpc_fifo_under_xflag_mask
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_over_flag(data)                     HDMI_P0_TMDS_DPC_SET3_dpc_fifo_over_flag(data)
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_under_flag(data)                    HDMI_P0_TMDS_DPC_SET3_dpc_fifo_under_flag(data)
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_over_xflag(data)                    HDMI_P0_TMDS_DPC_SET3_dpc_fifo_over_xflag(data)
#define  HDMI_TMDS_DPC_SET3_dpc_fifo_under_xflag(data)                   HDMI_P0_TMDS_DPC_SET3_dpc_fifo_under_xflag(data)
#define  HDMI_TMDS_DPC_SET3_get_dpc_fifo_over_flag(data)                 HDMI_P0_TMDS_DPC_SET3_get_dpc_fifo_over_flag(data)
#define  HDMI_TMDS_DPC_SET3_get_dpc_fifo_under_flag(data)                HDMI_P0_TMDS_DPC_SET3_get_dpc_fifo_under_flag(data)
#define  HDMI_TMDS_DPC_SET3_get_dpc_fifo_over_xflag(data)                HDMI_P0_TMDS_DPC_SET3_get_dpc_fifo_over_xflag(data)
#define  HDMI_TMDS_DPC_SET3_get_dpc_fifo_under_xflag(data)               HDMI_P0_TMDS_DPC_SET3_get_dpc_fifo_under_xflag(data)


#define  HDMI_VIDEO_BIT_ERR_DET_thd_mask                                 HDMI_P0_VIDEO_BIT_ERR_DET_thd_mask
#define  HDMI_VIDEO_BIT_ERR_DET_disparity_reset_mode_mask                HDMI_P0_VIDEO_BIT_ERR_DET_disparity_reset_mode_mask
#define  HDMI_VIDEO_BIT_ERR_DET_irq_en_mask                              HDMI_P0_VIDEO_BIT_ERR_DET_irq_en_mask
#define  HDMI_VIDEO_BIT_ERR_DET_reset_mask                               HDMI_P0_VIDEO_BIT_ERR_DET_reset_mask
#define  HDMI_VIDEO_BIT_ERR_DET_period_mask                              HDMI_P0_VIDEO_BIT_ERR_DET_period_mask
#define  HDMI_VIDEO_BIT_ERR_DET_ch_sel_mask                              HDMI_P0_VIDEO_BIT_ERR_DET_ch_sel_mask
#define  HDMI_VIDEO_BIT_ERR_DET_mode_mask                                HDMI_P0_VIDEO_BIT_ERR_DET_mode_mask
#define  HDMI_VIDEO_BIT_ERR_DET_en_mask                                  HDMI_P0_VIDEO_BIT_ERR_DET_en_mask
#define  HDMI_VIDEO_BIT_ERR_DET_thd(data)                                HDMI_P0_VIDEO_BIT_ERR_DET_thd(data)
#define  HDMI_VIDEO_BIT_ERR_DET_disparity_reset_mode(data)               HDMI_P0_VIDEO_BIT_ERR_DET_disparity_reset_mode(data)
#define  HDMI_VIDEO_BIT_ERR_DET_irq_en(data)                             HDMI_P0_VIDEO_BIT_ERR_DET_irq_en(data)
#define  HDMI_VIDEO_BIT_ERR_DET_reset(data)                              HDMI_P0_VIDEO_BIT_ERR_DET_reset(data)
#define  HDMI_VIDEO_BIT_ERR_DET_period(data)                             HDMI_P0_VIDEO_BIT_ERR_DET_period(data)
#define  HDMI_VIDEO_BIT_ERR_DET_ch_sel(data)                             HDMI_P0_VIDEO_BIT_ERR_DET_ch_sel(data)
#define  HDMI_VIDEO_BIT_ERR_DET_mode(data)                               HDMI_P0_VIDEO_BIT_ERR_DET_mode(data)
#define  HDMI_VIDEO_BIT_ERR_DET_en(data)                                 HDMI_P0_VIDEO_BIT_ERR_DET_en(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_thd(data)                            HDMI_P0_VIDEO_BIT_ERR_DET_get_thd(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_disparity_reset_mode(data)           HDMI_P0_VIDEO_BIT_ERR_DET_get_disparity_reset_mode(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_irq_en(data)                         HDMI_P0_VIDEO_BIT_ERR_DET_get_irq_en(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_reset(data)                          HDMI_P0_VIDEO_BIT_ERR_DET_get_reset(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_period(data)                         HDMI_P0_VIDEO_BIT_ERR_DET_get_period(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_ch_sel(data)                         HDMI_P0_VIDEO_BIT_ERR_DET_get_ch_sel(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_mode(data)                           HDMI_P0_VIDEO_BIT_ERR_DET_get_mode(data)
#define  HDMI_VIDEO_BIT_ERR_DET_get_en(data)                             HDMI_P0_VIDEO_BIT_ERR_DET_get_en(data)


#define  HDMI_VIDEO_BIT_ERR_STATUS_B_ad_max_b_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_B_ad_max_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_ad_min_b_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_B_ad_min_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask                 HDMI_P0_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask                      HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask               HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_b_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_b_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_ad_max_b(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_B_ad_max_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_ad_min_b(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_B_ad_min_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b(data)                HDMI_P0_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b(data)                     HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b(data)              HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_b(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_ad_max_b(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_ad_max_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_ad_min_b(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_ad_min_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_no_dis_reset_b(data)            HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_no_dis_reset_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_bit_err_thd_b(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_bit_err_thd_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_bit_err_b(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_bit_err_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_bit_err_cnt_of_b(data)          HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_bit_err_cnt_of_b(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_B_get_bit_err_cnt_b(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_B_get_bit_err_cnt_b(data)


#define  HDMI_VIDEO_BIT_ERR_STATUS_G_ad_max_g_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_G_ad_max_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_ad_min_g_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_G_ad_min_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_no_dis_reset_g_mask                 HDMI_P0_VIDEO_BIT_ERR_STATUS_G_no_dis_reset_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_thd_g_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_thd_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_g_mask                      HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_of_g_mask               HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_of_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_g_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_g_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_ad_max_g(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_G_ad_max_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_ad_min_g(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_G_ad_min_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_no_dis_reset_g(data)                HDMI_P0_VIDEO_BIT_ERR_STATUS_G_no_dis_reset_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_thd_g(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_thd_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_g(data)                     HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_of_g(data)              HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_of_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_g(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_G_bit_err_cnt_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_ad_max_g(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_ad_max_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_ad_min_g(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_ad_min_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_no_dis_reset_g(data)            HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_no_dis_reset_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_bit_err_thd_g(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_bit_err_thd_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_bit_err_g(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_bit_err_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_bit_err_cnt_of_g(data)          HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_bit_err_cnt_of_g(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_G_get_bit_err_cnt_g(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_G_get_bit_err_cnt_g(data)


#define  HDMI_VIDEO_BIT_ERR_STATUS_R_ad_max_r_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_R_ad_max_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_ad_min_r_mask                       HDMI_P0_VIDEO_BIT_ERR_STATUS_R_ad_min_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_no_dis_reset_r_mask                 HDMI_P0_VIDEO_BIT_ERR_STATUS_R_no_dis_reset_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_thd_r_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_thd_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_r_mask                      HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_of_r_mask               HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_of_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_r_mask                  HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_r_mask
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_ad_max_r(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_R_ad_max_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_ad_min_r(data)                      HDMI_P0_VIDEO_BIT_ERR_STATUS_R_ad_min_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_no_dis_reset_r(data)                HDMI_P0_VIDEO_BIT_ERR_STATUS_R_no_dis_reset_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_thd_r(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_thd_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_r(data)                     HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_of_r(data)              HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_of_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_r(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_R_bit_err_cnt_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_ad_max_r(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_ad_max_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_ad_min_r(data)                  HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_ad_min_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_no_dis_reset_r(data)            HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_no_dis_reset_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_bit_err_thd_r(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_bit_err_thd_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_bit_err_r(data)                 HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_bit_err_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_bit_err_cnt_of_r(data)          HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_bit_err_cnt_of_r(data)
#define  HDMI_VIDEO_BIT_ERR_STATUS_R_get_bit_err_cnt_r(data)             HDMI_P0_VIDEO_BIT_ERR_STATUS_R_get_bit_err_cnt_r(data)


#define  HDMI_TERC4_ERR_DET_terc4_thd_mask                               HDMI_P0_TERC4_ERR_DET_terc4_thd_mask
#define  HDMI_TERC4_ERR_DET_terc4_thd(data)                              HDMI_P0_TERC4_ERR_DET_terc4_thd(data)
#define  HDMI_TERC4_ERR_DET_get_terc4_thd(data)                          HDMI_P0_TERC4_ERR_DET_get_terc4_thd(data)


#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_thd_b_mask                    HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_thd_b_mask
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_b_mask                        HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_b_mask
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_cnt_of_b_mask                 HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_cnt_of_b_mask
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_cnt_b_mask                    HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_cnt_b_mask
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_thd_b(data)                   HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_thd_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_b(data)                       HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_cnt_of_b(data)                HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_cnt_of_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_terc4_err_cnt_b(data)                   HDMI_P0_TERC4_ERR_STATUS_B_terc4_err_cnt_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_get_terc4_err_thd_b(data)               HDMI_P0_TERC4_ERR_STATUS_B_get_terc4_err_thd_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_get_terc4_err_b(data)                   HDMI_P0_TERC4_ERR_STATUS_B_get_terc4_err_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_get_terc4_err_cnt_of_b(data)            HDMI_P0_TERC4_ERR_STATUS_B_get_terc4_err_cnt_of_b(data)
#define  HDMI_TERC4_ERR_STATUS_B_get_terc4_err_cnt_b(data)               HDMI_P0_TERC4_ERR_STATUS_B_get_terc4_err_cnt_b(data)


#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_thd_g_mask                    HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_thd_g_mask
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_g_mask                        HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_g_mask
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_cnt_of_g_mask                 HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_cnt_of_g_mask
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_cnt_g_mask                    HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_cnt_g_mask
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_thd_g(data)                   HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_thd_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_g(data)                       HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_cnt_of_g(data)                HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_cnt_of_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_terc4_err_cnt_g(data)                   HDMI_P0_TERC4_ERR_STATUS_G_terc4_err_cnt_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_get_terc4_err_thd_g(data)               HDMI_P0_TERC4_ERR_STATUS_G_get_terc4_err_thd_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_get_terc4_err_g(data)                   HDMI_P0_TERC4_ERR_STATUS_G_get_terc4_err_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_get_terc4_err_cnt_of_g(data)            HDMI_P0_TERC4_ERR_STATUS_G_get_terc4_err_cnt_of_g(data)
#define  HDMI_TERC4_ERR_STATUS_G_get_terc4_err_cnt_g(data)               HDMI_P0_TERC4_ERR_STATUS_G_get_terc4_err_cnt_g(data)


#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_thd_r_mask                    HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_thd_r_mask
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_r_mask                        HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_r_mask
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_cnt_of_r_mask                 HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_cnt_of_r_mask
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_cnt_r_mask                    HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_cnt_r_mask
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_thd_r(data)                   HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_thd_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_r(data)                       HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_cnt_of_r(data)                HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_cnt_of_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_terc4_err_cnt_r(data)                   HDMI_P0_TERC4_ERR_STATUS_R_terc4_err_cnt_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_get_terc4_err_thd_r(data)               HDMI_P0_TERC4_ERR_STATUS_R_get_terc4_err_thd_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_get_terc4_err_r(data)                   HDMI_P0_TERC4_ERR_STATUS_R_get_terc4_err_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_get_terc4_err_cnt_of_r(data)            HDMI_P0_TERC4_ERR_STATUS_R_get_terc4_err_cnt_of_r(data)
#define  HDMI_TERC4_ERR_STATUS_R_get_terc4_err_cnt_r(data)               HDMI_P0_TERC4_ERR_STATUS_R_get_terc4_err_cnt_r(data)


#define  HDMI_HDMI_SR_avmute_fw_mask                                     HDMI_P0_HDMI_SR_avmute_fw_mask
#define  HDMI_HDMI_SR_avmute_bg_mask                                     HDMI_P0_HDMI_SR_avmute_bg_mask
#define  HDMI_HDMI_SR_avmute_mask                                        HDMI_P0_HDMI_SR_avmute_mask
#define  HDMI_HDMI_SR_mode_mask                                          HDMI_P0_HDMI_SR_mode_mask
#define  HDMI_HDMI_SR_avmute_fw(data)                                    HDMI_P0_HDMI_SR_avmute_fw(data)
#define  HDMI_HDMI_SR_avmute_bg(data)                                    HDMI_P0_HDMI_SR_avmute_bg(data)
#define  HDMI_HDMI_SR_avmute(data)                                       HDMI_P0_HDMI_SR_avmute(data)
#define  HDMI_HDMI_SR_mode(data)                                         HDMI_P0_HDMI_SR_mode(data)
#define  HDMI_HDMI_SR_get_avmute_fw(data)                                HDMI_P0_HDMI_SR_get_avmute_fw(data)
#define  HDMI_HDMI_SR_get_avmute_bg(data)                                HDMI_P0_HDMI_SR_get_avmute_bg(data)
#define  HDMI_HDMI_SR_get_avmute(data)                                   HDMI_P0_HDMI_SR_get_avmute(data)
#define  HDMI_HDMI_SR_get_mode(data)                                     HDMI_P0_HDMI_SR_get_mode(data)


#define  HDMI_HDMI_GPVS_dummy_31_mask                                    HDMI_P0_HDMI_GPVS_dummy_31_mask
#define  HDMI_HDMI_GPVS_audps_mask                                       HDMI_P0_HDMI_GPVS_audps_mask
#define  HDMI_HDMI_GPVS_hbr_audps_mask                                   HDMI_P0_HDMI_GPVS_hbr_audps_mask
#define  HDMI_HDMI_GPVS_drmps_v_mask                                     HDMI_P0_HDMI_GPVS_drmps_v_mask
#define  HDMI_HDMI_GPVS_mpegps_v_mask                                    HDMI_P0_HDMI_GPVS_mpegps_v_mask
#define  HDMI_HDMI_GPVS_aps_v_mask                                       HDMI_P0_HDMI_GPVS_aps_v_mask
#define  HDMI_HDMI_GPVS_spdps_v_mask                                     HDMI_P0_HDMI_GPVS_spdps_v_mask
#define  HDMI_HDMI_GPVS_avips_v_mask                                     HDMI_P0_HDMI_GPVS_avips_v_mask
#define  HDMI_HDMI_GPVS_hdr10pvsps_v_mask                                HDMI_P0_HDMI_GPVS_hdr10pvsps_v_mask
#define  HDMI_HDMI_GPVS_dvsps_v_mask                                     HDMI_P0_HDMI_GPVS_dvsps_v_mask
#define  HDMI_HDMI_GPVS_fvsps_v_mask                                     HDMI_P0_HDMI_GPVS_fvsps_v_mask
#define  HDMI_HDMI_GPVS_vsps_v_mask                                      HDMI_P0_HDMI_GPVS_vsps_v_mask
#define  HDMI_HDMI_GPVS_gmps_v_mask                                      HDMI_P0_HDMI_GPVS_gmps_v_mask
#define  HDMI_HDMI_GPVS_isrc1ps_v_mask                                   HDMI_P0_HDMI_GPVS_isrc1ps_v_mask
#define  HDMI_HDMI_GPVS_acpps_v_mask                                     HDMI_P0_HDMI_GPVS_acpps_v_mask
#define  HDMI_HDMI_GPVS_drmps_mask                                       HDMI_P0_HDMI_GPVS_drmps_mask
#define  HDMI_HDMI_GPVS_mpegps_mask                                      HDMI_P0_HDMI_GPVS_mpegps_mask
#define  HDMI_HDMI_GPVS_aps_mask                                         HDMI_P0_HDMI_GPVS_aps_mask
#define  HDMI_HDMI_GPVS_spdps_mask                                       HDMI_P0_HDMI_GPVS_spdps_mask
#define  HDMI_HDMI_GPVS_avips_mask                                       HDMI_P0_HDMI_GPVS_avips_mask
#define  HDMI_HDMI_GPVS_hdr10pvsps_mask                                  HDMI_P0_HDMI_GPVS_hdr10pvsps_mask
#define  HDMI_HDMI_GPVS_dvsps_mask                                       HDMI_P0_HDMI_GPVS_dvsps_mask
#define  HDMI_HDMI_GPVS_fvsps_mask                                       HDMI_P0_HDMI_GPVS_fvsps_mask
#define  HDMI_HDMI_GPVS_vsps_mask                                        HDMI_P0_HDMI_GPVS_vsps_mask
#define  HDMI_HDMI_GPVS_gmps_mask                                        HDMI_P0_HDMI_GPVS_gmps_mask
#define  HDMI_HDMI_GPVS_isrc1ps_mask                                     HDMI_P0_HDMI_GPVS_isrc1ps_mask
#define  HDMI_HDMI_GPVS_acpps_mask                                       HDMI_P0_HDMI_GPVS_acpps_mask
#define  HDMI_HDMI_GPVS_nps_mask                                         HDMI_P0_HDMI_GPVS_nps_mask
#define  HDMI_HDMI_GPVS_rsv3ps_mask                                      HDMI_P0_HDMI_GPVS_rsv3ps_mask
#define  HDMI_HDMI_GPVS_rsv2ps_mask                                      HDMI_P0_HDMI_GPVS_rsv2ps_mask
#define  HDMI_HDMI_GPVS_rsv1ps_mask                                      HDMI_P0_HDMI_GPVS_rsv1ps_mask
#define  HDMI_HDMI_GPVS_rsv0ps_mask                                      HDMI_P0_HDMI_GPVS_rsv0ps_mask
#define  HDMI_HDMI_GPVS_dummy_31(data)                                   HDMI_P0_HDMI_GPVS_dummy_31(data)
#define  HDMI_HDMI_GPVS_audps(data)                                      HDMI_P0_HDMI_GPVS_audps(data)
#define  HDMI_HDMI_GPVS_hbr_audps(data)                                  HDMI_P0_HDMI_GPVS_hbr_audps(data)
#define  HDMI_HDMI_GPVS_drmps_v(data)                                    HDMI_P0_HDMI_GPVS_drmps_v(data)
#define  HDMI_HDMI_GPVS_mpegps_v(data)                                   HDMI_P0_HDMI_GPVS_mpegps_v(data)
#define  HDMI_HDMI_GPVS_aps_v(data)                                      HDMI_P0_HDMI_GPVS_aps_v(data)
#define  HDMI_HDMI_GPVS_spdps_v(data)                                    HDMI_P0_HDMI_GPVS_spdps_v(data)
#define  HDMI_HDMI_GPVS_avips_v(data)                                    HDMI_P0_HDMI_GPVS_avips_v(data)
#define  HDMI_HDMI_GPVS_hdr10pvsps_v(data)                               HDMI_P0_HDMI_GPVS_hdr10pvsps_v(data)
#define  HDMI_HDMI_GPVS_dvsps_v(data)                                    HDMI_P0_HDMI_GPVS_dvsps_v(data)
#define  HDMI_HDMI_GPVS_fvsps_v(data)                                    HDMI_P0_HDMI_GPVS_fvsps_v(data)
#define  HDMI_HDMI_GPVS_vsps_v(data)                                     HDMI_P0_HDMI_GPVS_vsps_v(data)
#define  HDMI_HDMI_GPVS_gmps_v(data)                                     HDMI_P0_HDMI_GPVS_gmps_v(data)
#define  HDMI_HDMI_GPVS_isrc1ps_v(data)                                  HDMI_P0_HDMI_GPVS_isrc1ps_v(data)
#define  HDMI_HDMI_GPVS_acpps_v(data)                                    HDMI_P0_HDMI_GPVS_acpps_v(data)
#define  HDMI_HDMI_GPVS_drmps(data)                                      HDMI_P0_HDMI_GPVS_drmps(data)
#define  HDMI_HDMI_GPVS_mpegps(data)                                     HDMI_P0_HDMI_GPVS_mpegps(data)
#define  HDMI_HDMI_GPVS_aps(data)                                        HDMI_P0_HDMI_GPVS_aps(data)
#define  HDMI_HDMI_GPVS_spdps(data)                                      HDMI_P0_HDMI_GPVS_spdps(data)
#define  HDMI_HDMI_GPVS_avips(data)                                      HDMI_P0_HDMI_GPVS_avips(data)
#define  HDMI_HDMI_GPVS_hdr10pvsps(data)                                 HDMI_P0_HDMI_GPVS_hdr10pvsps(data)
#define  HDMI_HDMI_GPVS_dvsps(data)                                      HDMI_P0_HDMI_GPVS_dvsps(data)
#define  HDMI_HDMI_GPVS_fvsps(data)                                      HDMI_P0_HDMI_GPVS_fvsps(data)
#define  HDMI_HDMI_GPVS_vsps(data)                                       HDMI_P0_HDMI_GPVS_vsps(data)
#define  HDMI_HDMI_GPVS_gmps(data)                                       HDMI_P0_HDMI_GPVS_gmps(data)
#define  HDMI_HDMI_GPVS_isrc1ps(data)                                    HDMI_P0_HDMI_GPVS_isrc1ps(data)
#define  HDMI_HDMI_GPVS_acpps(data)                                      HDMI_P0_HDMI_GPVS_acpps(data)
#define  HDMI_HDMI_GPVS_nps(data)                                        HDMI_P0_HDMI_GPVS_nps(data)
#define  HDMI_HDMI_GPVS_rsv3ps(data)                                     HDMI_P0_HDMI_GPVS_rsv3ps(data)
#define  HDMI_HDMI_GPVS_rsv2ps(data)                                     HDMI_P0_HDMI_GPVS_rsv2ps(data)
#define  HDMI_HDMI_GPVS_rsv1ps(data)                                     HDMI_P0_HDMI_GPVS_rsv1ps(data)
#define  HDMI_HDMI_GPVS_rsv0ps(data)                                     HDMI_P0_HDMI_GPVS_rsv0ps(data)
#define  HDMI_HDMI_GPVS_get_dummy_31(data)                               HDMI_P0_HDMI_GPVS_get_dummy_31(data)
#define  HDMI_HDMI_GPVS_get_audps(data)                                  HDMI_P0_HDMI_GPVS_get_audps(data)
#define  HDMI_HDMI_GPVS_get_hbr_audps(data)                              HDMI_P0_HDMI_GPVS_get_hbr_audps(data)
#define  HDMI_HDMI_GPVS_get_drmps_v(data)                                HDMI_P0_HDMI_GPVS_get_drmps_v(data)
#define  HDMI_HDMI_GPVS_get_mpegps_v(data)                               HDMI_P0_HDMI_GPVS_get_mpegps_v(data)
#define  HDMI_HDMI_GPVS_get_aps_v(data)                                  HDMI_P0_HDMI_GPVS_get_aps_v(data)
#define  HDMI_HDMI_GPVS_get_spdps_v(data)                                HDMI_P0_HDMI_GPVS_get_spdps_v(data)
#define  HDMI_HDMI_GPVS_get_avips_v(data)                                HDMI_P0_HDMI_GPVS_get_avips_v(data)
#define  HDMI_HDMI_GPVS_get_hdr10pvsps_v(data)                           HDMI_P0_HDMI_GPVS_get_hdr10pvsps_v(data)
#define  HDMI_HDMI_GPVS_get_dvsps_v(data)                                HDMI_P0_HDMI_GPVS_get_dvsps_v(data)
#define  HDMI_HDMI_GPVS_get_fvsps_v(data)                                HDMI_P0_HDMI_GPVS_get_fvsps_v(data)
#define  HDMI_HDMI_GPVS_get_vsps_v(data)                                 HDMI_P0_HDMI_GPVS_get_vsps_v(data)
#define  HDMI_HDMI_GPVS_get_gmps_v(data)                                 HDMI_P0_HDMI_GPVS_get_gmps_v(data)
#define  HDMI_HDMI_GPVS_get_isrc1ps_v(data)                              HDMI_P0_HDMI_GPVS_get_isrc1ps_v(data)
#define  HDMI_HDMI_GPVS_get_acpps_v(data)                                HDMI_P0_HDMI_GPVS_get_acpps_v(data)
#define  HDMI_HDMI_GPVS_get_drmps(data)                                  HDMI_P0_HDMI_GPVS_get_drmps(data)
#define  HDMI_HDMI_GPVS_get_mpegps(data)                                 HDMI_P0_HDMI_GPVS_get_mpegps(data)
#define  HDMI_HDMI_GPVS_get_aps(data)                                    HDMI_P0_HDMI_GPVS_get_aps(data)
#define  HDMI_HDMI_GPVS_get_spdps(data)                                  HDMI_P0_HDMI_GPVS_get_spdps(data)
#define  HDMI_HDMI_GPVS_get_avips(data)                                  HDMI_P0_HDMI_GPVS_get_avips(data)
#define  HDMI_HDMI_GPVS_get_hdr10pvsps(data)                             HDMI_P0_HDMI_GPVS_get_hdr10pvsps(data)
#define  HDMI_HDMI_GPVS_get_dvsps(data)                                  HDMI_P0_HDMI_GPVS_get_dvsps(data)
#define  HDMI_HDMI_GPVS_get_fvsps(data)                                  HDMI_P0_HDMI_GPVS_get_fvsps(data)
#define  HDMI_HDMI_GPVS_get_vsps(data)                                   HDMI_P0_HDMI_GPVS_get_vsps(data)
#define  HDMI_HDMI_GPVS_get_gmps(data)                                   HDMI_P0_HDMI_GPVS_get_gmps(data)
#define  HDMI_HDMI_GPVS_get_isrc1ps(data)                                HDMI_P0_HDMI_GPVS_get_isrc1ps(data)
#define  HDMI_HDMI_GPVS_get_acpps(data)                                  HDMI_P0_HDMI_GPVS_get_acpps(data)
#define  HDMI_HDMI_GPVS_get_nps(data)                                    HDMI_P0_HDMI_GPVS_get_nps(data)
#define  HDMI_HDMI_GPVS_get_rsv3ps(data)                                 HDMI_P0_HDMI_GPVS_get_rsv3ps(data)
#define  HDMI_HDMI_GPVS_get_rsv2ps(data)                                 HDMI_P0_HDMI_GPVS_get_rsv2ps(data)
#define  HDMI_HDMI_GPVS_get_rsv1ps(data)                                 HDMI_P0_HDMI_GPVS_get_rsv1ps(data)
#define  HDMI_HDMI_GPVS_get_rsv0ps(data)                                 HDMI_P0_HDMI_GPVS_get_rsv0ps(data)


#define  HDMI_HDMI_GPVS1_dummy_31_4_mask                                 HDMI_P0_HDMI_GPVS1_dummy_31_4_mask
#define  HDMI_HDMI_GPVS1_drmps_rv_mask                                   HDMI_P0_HDMI_GPVS1_drmps_rv_mask
#define  HDMI_HDMI_GPVS1_hdr10pvsps_rv_mask                              HDMI_P0_HDMI_GPVS1_hdr10pvsps_rv_mask
#define  HDMI_HDMI_GPVS1_dvsps_rv_mask                                   HDMI_P0_HDMI_GPVS1_dvsps_rv_mask
#define  HDMI_HDMI_GPVS1_vsps_rv_mask                                    HDMI_P0_HDMI_GPVS1_vsps_rv_mask
#define  HDMI_HDMI_GPVS1_dummy_31_4(data)                                HDMI_P0_HDMI_GPVS1_dummy_31_4(data)
#define  HDMI_HDMI_GPVS1_drmps_rv(data)                                  HDMI_P0_HDMI_GPVS1_drmps_rv(data)
#define  HDMI_HDMI_GPVS1_hdr10pvsps_rv(data)                             HDMI_P0_HDMI_GPVS1_hdr10pvsps_rv(data)
#define  HDMI_HDMI_GPVS1_dvsps_rv(data)                                  HDMI_P0_HDMI_GPVS1_dvsps_rv(data)
#define  HDMI_HDMI_GPVS1_vsps_rv(data)                                   HDMI_P0_HDMI_GPVS1_vsps_rv(data)
#define  HDMI_HDMI_GPVS1_get_dummy_31_4(data)                            HDMI_P0_HDMI_GPVS1_get_dummy_31_4(data)
#define  HDMI_HDMI_GPVS1_get_drmps_rv(data)                              HDMI_P0_HDMI_GPVS1_get_drmps_rv(data)
#define  HDMI_HDMI_GPVS1_get_hdr10pvsps_rv(data)                         HDMI_P0_HDMI_GPVS1_get_hdr10pvsps_rv(data)
#define  HDMI_HDMI_GPVS1_get_dvsps_rv(data)                              HDMI_P0_HDMI_GPVS1_get_dvsps_rv(data)
#define  HDMI_HDMI_GPVS1_get_vsps_rv(data)                               HDMI_P0_HDMI_GPVS1_get_vsps_rv(data)


#define  HDMI_HDMI_PSAP_apss_mask                                        HDMI_P0_HDMI_PSAP_apss_mask
#define  HDMI_HDMI_PSAP_apss(data)                                       HDMI_P0_HDMI_PSAP_apss(data)
#define  HDMI_HDMI_PSAP_get_apss(data)                                   HDMI_P0_HDMI_PSAP_get_apss(data)


#define  HDMI_HDMI_PSDP_dpss_mask                                        HDMI_P0_HDMI_PSDP_dpss_mask
#define  HDMI_HDMI_PSDP_dpss(data)                                       HDMI_P0_HDMI_PSDP_dpss(data)
#define  HDMI_HDMI_PSDP_get_dpss(data)                                   HDMI_P0_HDMI_PSDP_get_dpss(data)


#define  HDMI_HDMI_SCR_packet_header_parsing_mode_mask                   HDMI_P0_HDMI_SCR_packet_header_parsing_mode_mask
#define  HDMI_HDMI_SCR_nval_mask                                         HDMI_P0_HDMI_SCR_nval_mask
#define  HDMI_HDMI_SCR_no_vs_det_en_mask                                 HDMI_P0_HDMI_SCR_no_vs_det_en_mask
#define  HDMI_HDMI_SCR_dvi_reset_ds_cm_en_mask                           HDMI_P0_HDMI_SCR_dvi_reset_ds_cm_en_mask
#define  HDMI_HDMI_SCR_packet_ignore_mask                                HDMI_P0_HDMI_SCR_packet_ignore_mask
#define  HDMI_HDMI_SCR_mode_mask                                         HDMI_P0_HDMI_SCR_mode_mask
#define  HDMI_HDMI_SCR_msmode_mask                                       HDMI_P0_HDMI_SCR_msmode_mask
#define  HDMI_HDMI_SCR_cabs_mask                                         HDMI_P0_HDMI_SCR_cabs_mask
#define  HDMI_HDMI_SCR_fcddip_mask                                       HDMI_P0_HDMI_SCR_fcddip_mask
#define  HDMI_HDMI_SCR_packet_header_parsing_mode(data)                  HDMI_P0_HDMI_SCR_packet_header_parsing_mode(data)
#define  HDMI_HDMI_SCR_nval(data)                                        HDMI_P0_HDMI_SCR_nval(data)
#define  HDMI_HDMI_SCR_no_vs_det_en(data)                                HDMI_P0_HDMI_SCR_no_vs_det_en(data)
#define  HDMI_HDMI_SCR_dvi_reset_ds_cm_en(data)                          HDMI_P0_HDMI_SCR_dvi_reset_ds_cm_en(data)
#define  HDMI_HDMI_SCR_packet_ignore(data)                               HDMI_P0_HDMI_SCR_packet_ignore(data)
#define  HDMI_HDMI_SCR_mode(data)                                        HDMI_P0_HDMI_SCR_mode(data)
#define  HDMI_HDMI_SCR_msmode(data)                                      HDMI_P0_HDMI_SCR_msmode(data)
#define  HDMI_HDMI_SCR_cabs(data)                                        HDMI_P0_HDMI_SCR_cabs(data)
#define  HDMI_HDMI_SCR_fcddip(data)                                      HDMI_P0_HDMI_SCR_fcddip(data)
#define  HDMI_HDMI_SCR_get_packet_header_parsing_mode(data)              HDMI_P0_HDMI_SCR_get_packet_header_parsing_mode(data)
#define  HDMI_HDMI_SCR_get_nval(data)                                    HDMI_P0_HDMI_SCR_get_nval(data)
#define  HDMI_HDMI_SCR_get_no_vs_det_en(data)                            HDMI_P0_HDMI_SCR_get_no_vs_det_en(data)
#define  HDMI_HDMI_SCR_get_dvi_reset_ds_cm_en(data)                      HDMI_P0_HDMI_SCR_get_dvi_reset_ds_cm_en(data)
#define  HDMI_HDMI_SCR_get_packet_ignore(data)                           HDMI_P0_HDMI_SCR_get_packet_ignore(data)
#define  HDMI_HDMI_SCR_get_mode(data)                                    HDMI_P0_HDMI_SCR_get_mode(data)
#define  HDMI_HDMI_SCR_get_msmode(data)                                  HDMI_P0_HDMI_SCR_get_msmode(data)
#define  HDMI_HDMI_SCR_get_cabs(data)                                    HDMI_P0_HDMI_SCR_get_cabs(data)
#define  HDMI_HDMI_SCR_get_fcddip(data)                                  HDMI_P0_HDMI_SCR_get_fcddip(data)


#define  HDMI_HDMI_BCHCR_bches2_airq_en_mask                             HDMI_P0_HDMI_BCHCR_bches2_airq_en_mask
#define  HDMI_HDMI_BCHCR_bches2_irq_en_mask                              HDMI_P0_HDMI_BCHCR_bches2_irq_en_mask
#define  HDMI_HDMI_BCHCR_bche_mask                                       HDMI_P0_HDMI_BCHCR_bche_mask
#define  HDMI_HDMI_BCHCR_bches_mask                                      HDMI_P0_HDMI_BCHCR_bches_mask
#define  HDMI_HDMI_BCHCR_bches2_mask                                     HDMI_P0_HDMI_BCHCR_bches2_mask
#define  HDMI_HDMI_BCHCR_pe_mask                                         HDMI_P0_HDMI_BCHCR_pe_mask
#define  HDMI_HDMI_BCHCR_bches2_airq_en(data)                            HDMI_P0_HDMI_BCHCR_bches2_airq_en(data)
#define  HDMI_HDMI_BCHCR_bches2_irq_en(data)                             HDMI_P0_HDMI_BCHCR_bches2_irq_en(data)
#define  HDMI_HDMI_BCHCR_bche(data)                                      HDMI_P0_HDMI_BCHCR_bche(data)
#define  HDMI_HDMI_BCHCR_bches(data)                                     HDMI_P0_HDMI_BCHCR_bches(data)
#define  HDMI_HDMI_BCHCR_bches2(data)                                    HDMI_P0_HDMI_BCHCR_bches2(data)
#define  HDMI_HDMI_BCHCR_pe(data)                                        HDMI_P0_HDMI_BCHCR_pe(data)
#define  HDMI_HDMI_BCHCR_get_bches2_airq_en(data)                        HDMI_P0_HDMI_BCHCR_get_bches2_airq_en(data)
#define  HDMI_HDMI_BCHCR_get_bches2_irq_en(data)                         HDMI_P0_HDMI_BCHCR_get_bches2_irq_en(data)
#define  HDMI_HDMI_BCHCR_get_bche(data)                                  HDMI_P0_HDMI_BCHCR_get_bche(data)
#define  HDMI_HDMI_BCHCR_get_bches(data)                                 HDMI_P0_HDMI_BCHCR_get_bches(data)
#define  HDMI_HDMI_BCHCR_get_bches2(data)                                HDMI_P0_HDMI_BCHCR_get_bches2(data)
#define  HDMI_HDMI_BCHCR_get_pe(data)                                    HDMI_P0_HDMI_BCHCR_get_pe(data)


#define  HDMI_HDMI_AVMCR_avmute_flag_mask                                HDMI_P0_HDMI_AVMCR_avmute_flag_mask
#define  HDMI_HDMI_AVMCR_avmute_win_en_mask                              HDMI_P0_HDMI_AVMCR_avmute_win_en_mask
#define  HDMI_HDMI_AVMCR_ve_mask                                         HDMI_P0_HDMI_AVMCR_ve_mask
#define  HDMI_HDMI_AVMCR_avmute_flag(data)                               HDMI_P0_HDMI_AVMCR_avmute_flag(data)
#define  HDMI_HDMI_AVMCR_avmute_win_en(data)                             HDMI_P0_HDMI_AVMCR_avmute_win_en(data)
#define  HDMI_HDMI_AVMCR_ve(data)                                        HDMI_P0_HDMI_AVMCR_ve(data)
#define  HDMI_HDMI_AVMCR_get_avmute_flag(data)                           HDMI_P0_HDMI_AVMCR_get_avmute_flag(data)
#define  HDMI_HDMI_AVMCR_get_avmute_win_en(data)                         HDMI_P0_HDMI_AVMCR_get_avmute_win_en(data)
#define  HDMI_HDMI_AVMCR_get_ve(data)                                    HDMI_P0_HDMI_AVMCR_get_ve(data)


#define  HDMI_HDMI_WDCR_hdmi_wd_debug_mode_mask                          HDMI_P0_HDMI_WDCR_hdmi_wd_debug_mode_mask
#define  HDMI_HDMI_WDCR_hdmi_wd_debug_target_mask                        HDMI_P0_HDMI_WDCR_hdmi_wd_debug_target_mask
#define  HDMI_HDMI_WDCR_avmute_wd_en_mask                                HDMI_P0_HDMI_WDCR_avmute_wd_en_mask
#define  HDMI_HDMI_WDCR_hdmi_wd_debug_mode(data)                         HDMI_P0_HDMI_WDCR_hdmi_wd_debug_mode(data)
#define  HDMI_HDMI_WDCR_hdmi_wd_debug_target(data)                       HDMI_P0_HDMI_WDCR_hdmi_wd_debug_target(data)
#define  HDMI_HDMI_WDCR_avmute_wd_en(data)                               HDMI_P0_HDMI_WDCR_avmute_wd_en(data)
#define  HDMI_HDMI_WDCR_get_hdmi_wd_debug_mode(data)                     HDMI_P0_HDMI_WDCR_get_hdmi_wd_debug_mode(data)
#define  HDMI_HDMI_WDCR_get_hdmi_wd_debug_target(data)                   HDMI_P0_HDMI_WDCR_get_hdmi_wd_debug_target(data)
#define  HDMI_HDMI_WDCR_get_avmute_wd_en(data)                           HDMI_P0_HDMI_WDCR_get_avmute_wd_en(data)


#define  HDMI_HDMI_PAMICR_aicpvsb_mask                                   HDMI_P0_HDMI_PAMICR_aicpvsb_mask
#define  HDMI_HDMI_PAMICR_icpvsb_mask                                    HDMI_P0_HDMI_PAMICR_icpvsb_mask
#define  HDMI_HDMI_PAMICR_aicpvsb(data)                                  HDMI_P0_HDMI_PAMICR_aicpvsb(data)
#define  HDMI_HDMI_PAMICR_icpvsb(data)                                   HDMI_P0_HDMI_PAMICR_icpvsb(data)
#define  HDMI_HDMI_PAMICR_get_aicpvsb(data)                              HDMI_P0_HDMI_PAMICR_get_aicpvsb(data)
#define  HDMI_HDMI_PAMICR_get_icpvsb(data)                               HDMI_P0_HDMI_PAMICR_get_icpvsb(data)


#define  HDMI_HDMI_PTRSV1_pt3_mask                                       HDMI_P0_HDMI_PTRSV1_pt3_mask
#define  HDMI_HDMI_PTRSV1_pt2_mask                                       HDMI_P0_HDMI_PTRSV1_pt2_mask
#define  HDMI_HDMI_PTRSV1_pt1_mask                                       HDMI_P0_HDMI_PTRSV1_pt1_mask
#define  HDMI_HDMI_PTRSV1_pt0_mask                                       HDMI_P0_HDMI_PTRSV1_pt0_mask
#define  HDMI_HDMI_PTRSV1_pt3(data)                                      HDMI_P0_HDMI_PTRSV1_pt3(data)
#define  HDMI_HDMI_PTRSV1_pt2(data)                                      HDMI_P0_HDMI_PTRSV1_pt2(data)
#define  HDMI_HDMI_PTRSV1_pt1(data)                                      HDMI_P0_HDMI_PTRSV1_pt1(data)
#define  HDMI_HDMI_PTRSV1_pt0(data)                                      HDMI_P0_HDMI_PTRSV1_pt0(data)
#define  HDMI_HDMI_PTRSV1_get_pt3(data)                                  HDMI_P0_HDMI_PTRSV1_get_pt3(data)
#define  HDMI_HDMI_PTRSV1_get_pt2(data)                                  HDMI_P0_HDMI_PTRSV1_get_pt2(data)
#define  HDMI_HDMI_PTRSV1_get_pt1(data)                                  HDMI_P0_HDMI_PTRSV1_get_pt1(data)
#define  HDMI_HDMI_PTRSV1_get_pt0(data)                                  HDMI_P0_HDMI_PTRSV1_get_pt0(data)


#define  HDMI_HDMI_PTRSV2_pt3_oui_1st_mask                               HDMI_P0_HDMI_PTRSV2_pt3_oui_1st_mask
#define  HDMI_HDMI_PTRSV2_pt2_oui_1st_mask                               HDMI_P0_HDMI_PTRSV2_pt2_oui_1st_mask
#define  HDMI_HDMI_PTRSV2_pt1_oui_1st_mask                               HDMI_P0_HDMI_PTRSV2_pt1_oui_1st_mask
#define  HDMI_HDMI_PTRSV2_pt0_oui_1st_mask                               HDMI_P0_HDMI_PTRSV2_pt0_oui_1st_mask
#define  HDMI_HDMI_PTRSV2_pt3_oui_1st(data)                              HDMI_P0_HDMI_PTRSV2_pt3_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_pt2_oui_1st(data)                              HDMI_P0_HDMI_PTRSV2_pt2_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_pt1_oui_1st(data)                              HDMI_P0_HDMI_PTRSV2_pt1_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_pt0_oui_1st(data)                              HDMI_P0_HDMI_PTRSV2_pt0_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_get_pt3_oui_1st(data)                          HDMI_P0_HDMI_PTRSV2_get_pt3_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_get_pt2_oui_1st(data)                          HDMI_P0_HDMI_PTRSV2_get_pt2_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_get_pt1_oui_1st(data)                          HDMI_P0_HDMI_PTRSV2_get_pt1_oui_1st(data)
#define  HDMI_HDMI_PTRSV2_get_pt0_oui_1st(data)                          HDMI_P0_HDMI_PTRSV2_get_pt0_oui_1st(data)


#define  HDMI_HDMI_PTRSV3_pt3_recognize_oui_en_mask                      HDMI_P0_HDMI_PTRSV3_pt3_recognize_oui_en_mask
#define  HDMI_HDMI_PTRSV3_pt2_recognize_oui_en_mask                      HDMI_P0_HDMI_PTRSV3_pt2_recognize_oui_en_mask
#define  HDMI_HDMI_PTRSV3_pt1_recognize_oui_en_mask                      HDMI_P0_HDMI_PTRSV3_pt1_recognize_oui_en_mask
#define  HDMI_HDMI_PTRSV3_pt0_recognize_oui_en_mask                      HDMI_P0_HDMI_PTRSV3_pt0_recognize_oui_en_mask
#define  HDMI_HDMI_PTRSV3_pt3_recognize_oui_en(data)                     HDMI_P0_HDMI_PTRSV3_pt3_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_pt2_recognize_oui_en(data)                     HDMI_P0_HDMI_PTRSV3_pt2_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_pt1_recognize_oui_en(data)                     HDMI_P0_HDMI_PTRSV3_pt1_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_pt0_recognize_oui_en(data)                     HDMI_P0_HDMI_PTRSV3_pt0_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_get_pt3_recognize_oui_en(data)                 HDMI_P0_HDMI_PTRSV3_get_pt3_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_get_pt2_recognize_oui_en(data)                 HDMI_P0_HDMI_PTRSV3_get_pt2_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_get_pt1_recognize_oui_en(data)                 HDMI_P0_HDMI_PTRSV3_get_pt1_recognize_oui_en(data)
#define  HDMI_HDMI_PTRSV3_get_pt0_recognize_oui_en(data)                 HDMI_P0_HDMI_PTRSV3_get_pt0_recognize_oui_en(data)


#define  HDMI_HDMI_PVGCR0_pvsef_mask                                     HDMI_P0_HDMI_PVGCR0_pvsef_mask
#define  HDMI_HDMI_PVGCR0_pvsef(data)                                    HDMI_P0_HDMI_PVGCR0_pvsef(data)
#define  HDMI_HDMI_PVGCR0_get_pvsef(data)                                HDMI_P0_HDMI_PVGCR0_get_pvsef(data)


#define  HDMI_HDMI_PVSR0_pvs_mask                                        HDMI_P0_HDMI_PVSR0_pvs_mask
#define  HDMI_HDMI_PVSR0_pvs(data)                                       HDMI_P0_HDMI_PVSR0_pvs(data)
#define  HDMI_HDMI_PVSR0_get_pvs(data)                                   HDMI_P0_HDMI_PVSR0_get_pvs(data)


#define  HDMI_HDMI_VCR_hdmi_field_mask                                   HDMI_P0_HDMI_VCR_hdmi_field_mask
#define  HDMI_HDMI_VCR_int_pro_chg_flag_mask                             HDMI_P0_HDMI_VCR_int_pro_chg_flag_mask
#define  HDMI_HDMI_VCR_int_pro_chg_int_en_mask                           HDMI_P0_HDMI_VCR_int_pro_chg_int_en_mask
#define  HDMI_HDMI_VCR_bg_gck_mask                                       HDMI_P0_HDMI_VCR_bg_gck_mask
#define  HDMI_HDMI_VCR_iclk_sel_mask                                     HDMI_P0_HDMI_VCR_iclk_sel_mask
#define  HDMI_HDMI_VCR_csc_r_mask                                        HDMI_P0_HDMI_VCR_csc_r_mask
#define  HDMI_HDMI_VCR_hdirq_mask                                        HDMI_P0_HDMI_VCR_hdirq_mask
#define  HDMI_HDMI_VCR_csam_mask                                         HDMI_P0_HDMI_VCR_csam_mask
#define  HDMI_HDMI_VCR_csc_mask                                          HDMI_P0_HDMI_VCR_csc_mask
#define  HDMI_HDMI_VCR_field_decide_sel_mask                             HDMI_P0_HDMI_VCR_field_decide_sel_mask
#define  HDMI_HDMI_VCR_prdsam_mask                                       HDMI_P0_HDMI_VCR_prdsam_mask
#define  HDMI_HDMI_VCR_dsc_r_mask                                        HDMI_P0_HDMI_VCR_dsc_r_mask
#define  HDMI_HDMI_VCR_eoi_mask                                          HDMI_P0_HDMI_VCR_eoi_mask
#define  HDMI_HDMI_VCR_eot_mask                                          HDMI_P0_HDMI_VCR_eot_mask
#define  HDMI_HDMI_VCR_se_mask                                           HDMI_P0_HDMI_VCR_se_mask
#define  HDMI_HDMI_VCR_rs_mask                                           HDMI_P0_HDMI_VCR_rs_mask
#define  HDMI_HDMI_VCR_dsc_mask                                          HDMI_P0_HDMI_VCR_dsc_mask
#define  HDMI_HDMI_VCR_hdmi_field(data)                                  HDMI_P0_HDMI_VCR_hdmi_field(data)
#define  HDMI_HDMI_VCR_int_pro_chg_flag(data)                            HDMI_P0_HDMI_VCR_int_pro_chg_flag(data)
#define  HDMI_HDMI_VCR_int_pro_chg_int_en(data)                          HDMI_P0_HDMI_VCR_int_pro_chg_int_en(data)
#define  HDMI_HDMI_VCR_bg_gck(data)                                      HDMI_P0_HDMI_VCR_bg_gck(data)
#define  HDMI_HDMI_VCR_iclk_sel(data)                                    HDMI_P0_HDMI_VCR_iclk_sel(data)
#define  HDMI_HDMI_VCR_csc_r(data)                                       HDMI_P0_HDMI_VCR_csc_r(data)
#define  HDMI_HDMI_VCR_hdirq(data)                                       HDMI_P0_HDMI_VCR_hdirq(data)
#define  HDMI_HDMI_VCR_csam(data)                                        HDMI_P0_HDMI_VCR_csam(data)
#define  HDMI_HDMI_VCR_csc(data)                                         HDMI_P0_HDMI_VCR_csc(data)
#define  HDMI_HDMI_VCR_field_decide_sel(data)                            HDMI_P0_HDMI_VCR_field_decide_sel(data)
#define  HDMI_HDMI_VCR_prdsam(data)                                      HDMI_P0_HDMI_VCR_prdsam(data)
#define  HDMI_HDMI_VCR_dsc_r(data)                                       HDMI_P0_HDMI_VCR_dsc_r(data)
#define  HDMI_HDMI_VCR_eoi(data)                                         HDMI_P0_HDMI_VCR_eoi(data)
#define  HDMI_HDMI_VCR_eot(data)                                         HDMI_P0_HDMI_VCR_eot(data)
#define  HDMI_HDMI_VCR_se(data)                                          HDMI_P0_HDMI_VCR_se(data)
#define  HDMI_HDMI_VCR_rs(data)                                          HDMI_P0_HDMI_VCR_rs(data)
#define  HDMI_HDMI_VCR_dsc(data)                                         HDMI_P0_HDMI_VCR_dsc(data)
#define  HDMI_HDMI_VCR_get_hdmi_field(data)                              HDMI_P0_HDMI_VCR_get_hdmi_field(data)
#define  HDMI_HDMI_VCR_get_int_pro_chg_flag(data)                        HDMI_P0_HDMI_VCR_get_int_pro_chg_flag(data)
#define  HDMI_HDMI_VCR_get_int_pro_chg_int_en(data)                      HDMI_P0_HDMI_VCR_get_int_pro_chg_int_en(data)
#define  HDMI_HDMI_VCR_get_bg_gck(data)                                  HDMI_P0_HDMI_VCR_get_bg_gck(data)
#define  HDMI_HDMI_VCR_get_iclk_sel(data)                                HDMI_P0_HDMI_VCR_get_iclk_sel(data)
#define  HDMI_HDMI_VCR_get_csc_r(data)                                   HDMI_P0_HDMI_VCR_get_csc_r(data)
#define  HDMI_HDMI_VCR_get_hdirq(data)                                   HDMI_P0_HDMI_VCR_get_hdirq(data)
#define  HDMI_HDMI_VCR_get_csam(data)                                    HDMI_P0_HDMI_VCR_get_csam(data)
#define  HDMI_HDMI_VCR_get_csc(data)                                     HDMI_P0_HDMI_VCR_get_csc(data)
#define  HDMI_HDMI_VCR_get_field_decide_sel(data)                        HDMI_P0_HDMI_VCR_get_field_decide_sel(data)
#define  HDMI_HDMI_VCR_get_prdsam(data)                                  HDMI_P0_HDMI_VCR_get_prdsam(data)
#define  HDMI_HDMI_VCR_get_dsc_r(data)                                   HDMI_P0_HDMI_VCR_get_dsc_r(data)
#define  HDMI_HDMI_VCR_get_eoi(data)                                     HDMI_P0_HDMI_VCR_get_eoi(data)
#define  HDMI_HDMI_VCR_get_eot(data)                                     HDMI_P0_HDMI_VCR_get_eot(data)
#define  HDMI_HDMI_VCR_get_se(data)                                      HDMI_P0_HDMI_VCR_get_se(data)
#define  HDMI_HDMI_VCR_get_rs(data)                                      HDMI_P0_HDMI_VCR_get_rs(data)
#define  HDMI_HDMI_VCR_get_dsc(data)                                     HDMI_P0_HDMI_VCR_get_dsc(data)


#define  HDMI_HDMI_ACRCR_pucnr_mask                                      HDMI_P0_HDMI_ACRCR_pucnr_mask
#define  HDMI_HDMI_ACRCR_mode_mask                                       HDMI_P0_HDMI_ACRCR_mode_mask
#define  HDMI_HDMI_ACRCR_pucnr(data)                                     HDMI_P0_HDMI_ACRCR_pucnr(data)
#define  HDMI_HDMI_ACRCR_mode(data)                                      HDMI_P0_HDMI_ACRCR_mode(data)
#define  HDMI_HDMI_ACRCR_get_pucnr(data)                                 HDMI_P0_HDMI_ACRCR_get_pucnr(data)
#define  HDMI_HDMI_ACRCR_get_mode(data)                                  HDMI_P0_HDMI_ACRCR_get_mode(data)


#define  HDMI_HDMI_ACRSR0_cts_mask                                       HDMI_P0_HDMI_ACRSR0_cts_mask
#define  HDMI_HDMI_ACRSR0_cts(data)                                      HDMI_P0_HDMI_ACRSR0_cts(data)
#define  HDMI_HDMI_ACRSR0_get_cts(data)                                  HDMI_P0_HDMI_ACRSR0_get_cts(data)


#define  HDMI_HDMI_ACRSR1_n_mask                                         HDMI_P0_HDMI_ACRSR1_n_mask
#define  HDMI_HDMI_ACRSR1_n(data)                                        HDMI_P0_HDMI_ACRSR1_n(data)
#define  HDMI_HDMI_ACRSR1_get_n(data)                                    HDMI_P0_HDMI_ACRSR1_get_n(data)


#define  HDMI_HDMI_INTCR_airq_gen_mask                                   HDMI_P0_HDMI_INTCR_airq_gen_mask
#define  HDMI_HDMI_INTCR_irq_gen_mask                                    HDMI_P0_HDMI_INTCR_irq_gen_mask
#define  HDMI_HDMI_INTCR_apending_mask                                   HDMI_P0_HDMI_INTCR_apending_mask
#define  HDMI_HDMI_INTCR_pending_mask                                    HDMI_P0_HDMI_INTCR_pending_mask
#define  HDMI_HDMI_INTCR_aavmute_mask                                    HDMI_P0_HDMI_INTCR_aavmute_mask
#define  HDMI_HDMI_INTCR_avmute_mask                                     HDMI_P0_HDMI_INTCR_avmute_mask
#define  HDMI_HDMI_INTCR_avc_mask                                        HDMI_P0_HDMI_INTCR_avc_mask
#define  HDMI_HDMI_INTCR_vc_mask                                         HDMI_P0_HDMI_INTCR_vc_mask
#define  HDMI_HDMI_INTCR_airq_gen(data)                                  HDMI_P0_HDMI_INTCR_airq_gen(data)
#define  HDMI_HDMI_INTCR_irq_gen(data)                                   HDMI_P0_HDMI_INTCR_irq_gen(data)
#define  HDMI_HDMI_INTCR_apending(data)                                  HDMI_P0_HDMI_INTCR_apending(data)
#define  HDMI_HDMI_INTCR_pending(data)                                   HDMI_P0_HDMI_INTCR_pending(data)
#define  HDMI_HDMI_INTCR_aavmute(data)                                   HDMI_P0_HDMI_INTCR_aavmute(data)
#define  HDMI_HDMI_INTCR_avmute(data)                                    HDMI_P0_HDMI_INTCR_avmute(data)
#define  HDMI_HDMI_INTCR_avc(data)                                       HDMI_P0_HDMI_INTCR_avc(data)
#define  HDMI_HDMI_INTCR_vc(data)                                        HDMI_P0_HDMI_INTCR_vc(data)
#define  HDMI_HDMI_INTCR_get_airq_gen(data)                              HDMI_P0_HDMI_INTCR_get_airq_gen(data)
#define  HDMI_HDMI_INTCR_get_irq_gen(data)                               HDMI_P0_HDMI_INTCR_get_irq_gen(data)
#define  HDMI_HDMI_INTCR_get_apending(data)                              HDMI_P0_HDMI_INTCR_get_apending(data)
#define  HDMI_HDMI_INTCR_get_pending(data)                               HDMI_P0_HDMI_INTCR_get_pending(data)
#define  HDMI_HDMI_INTCR_get_aavmute(data)                               HDMI_P0_HDMI_INTCR_get_aavmute(data)
#define  HDMI_HDMI_INTCR_get_avmute(data)                                HDMI_P0_HDMI_INTCR_get_avmute(data)
#define  HDMI_HDMI_INTCR_get_avc(data)                                   HDMI_P0_HDMI_INTCR_get_avc(data)
#define  HDMI_HDMI_INTCR_get_vc(data)                                    HDMI_P0_HDMI_INTCR_get_vc(data)

#define  HDMI_HDMI_BCSR_bch_err2_mask                                    HDMI_P0_HDMI_BCSR_bch_err2_mask
#define  HDMI_HDMI_BCSR_gcp_bch_err2_mask                                HDMI_P0_HDMI_BCSR_gcp_bch_err2_mask
#define  HDMI_HDMI_BCSR_nvlgb_mask                                       HDMI_P0_HDMI_BCSR_nvlgb_mask
#define  HDMI_HDMI_BCSR_nalgb_mask                                       HDMI_P0_HDMI_BCSR_nalgb_mask
#define  HDMI_HDMI_BCSR_natgb_mask                                       HDMI_P0_HDMI_BCSR_natgb_mask
#define  HDMI_HDMI_BCSR_ngb_mask                                         HDMI_P0_HDMI_BCSR_ngb_mask
#define  HDMI_HDMI_BCSR_pe_mask                                          HDMI_P0_HDMI_BCSR_pe_mask
#define  HDMI_HDMI_BCSR_gcp_mask                                         HDMI_P0_HDMI_BCSR_gcp_mask
#define  HDMI_HDMI_BCSR_bch_err2(data)                                   HDMI_P0_HDMI_BCSR_bch_err2(data)
#define  HDMI_HDMI_BCSR_gcp_bch_err2(data)                               HDMI_P0_HDMI_BCSR_gcp_bch_err2(data)
#define  HDMI_HDMI_BCSR_nvlgb(data)                                      HDMI_P0_HDMI_BCSR_nvlgb(data)
#define  HDMI_HDMI_BCSR_nalgb(data)                                      HDMI_P0_HDMI_BCSR_nalgb(data)
#define  HDMI_HDMI_BCSR_natgb(data)                                      HDMI_P0_HDMI_BCSR_natgb(data)
#define  HDMI_HDMI_BCSR_ngb(data)                                        HDMI_P0_HDMI_BCSR_ngb(data)
#define  HDMI_HDMI_BCSR_pe(data)                                         HDMI_P0_HDMI_BCSR_pe(data)
#define  HDMI_HDMI_BCSR_gcp(data)                                        HDMI_P0_HDMI_BCSR_gcp(data)
#define  HDMI_HDMI_BCSR_get_bch_err2(data)                               HDMI_P0_HDMI_BCSR_get_bch_err2(data)
#define  HDMI_HDMI_BCSR_get_gcp_bch_err2(data)                           HDMI_P0_HDMI_BCSR_get_gcp_bch_err2(data)
#define  HDMI_HDMI_BCSR_get_nvlgb(data)                                  HDMI_P0_HDMI_BCSR_get_nvlgb(data)
#define  HDMI_HDMI_BCSR_get_nalgb(data)                                  HDMI_P0_HDMI_BCSR_get_nalgb(data)
#define  HDMI_HDMI_BCSR_get_natgb(data)                                  HDMI_P0_HDMI_BCSR_get_natgb(data)
#define  HDMI_HDMI_BCSR_get_ngb(data)                                    HDMI_P0_HDMI_BCSR_get_ngb(data)
#define  HDMI_HDMI_BCSR_get_pe(data)                                     HDMI_P0_HDMI_BCSR_get_pe(data)
#define  HDMI_HDMI_BCSR_get_gcp(data)                                    HDMI_P0_HDMI_BCSR_get_gcp(data)


#define  HDMI_HDMI_ASR0_fsre_mask                                        HDMI_P0_HDMI_ASR0_fsre_mask
#define  HDMI_HDMI_ASR0_fsif_mask                                        HDMI_P0_HDMI_ASR0_fsif_mask
#define  HDMI_HDMI_ASR0_fsre(data)                                       HDMI_P0_HDMI_ASR0_fsre(data)
#define  HDMI_HDMI_ASR0_fsif(data)                                       HDMI_P0_HDMI_ASR0_fsif(data)
#define  HDMI_HDMI_ASR0_get_fsre(data)                                   HDMI_P0_HDMI_ASR0_get_fsre(data)
#define  HDMI_HDMI_ASR0_get_fsif(data)                                   HDMI_P0_HDMI_ASR0_get_fsif(data)


#define  HDMI_HDMI_ASR1_fbif_mask                                        HDMI_P0_HDMI_ASR1_fbif_mask
#define  HDMI_HDMI_ASR1_fbif(data)                                       HDMI_P0_HDMI_ASR1_fbif(data)
#define  HDMI_HDMI_ASR1_get_fbif(data)                                   HDMI_P0_HDMI_ASR1_get_fbif(data)


#define  HDMI_HDMI_VIDEO_FORMAT_hvf_mask                                 HDMI_P0_HDMI_VIDEO_FORMAT_hvf_mask
#define  HDMI_HDMI_VIDEO_FORMAT_hdmi_vic_mask                            HDMI_P0_HDMI_VIDEO_FORMAT_hdmi_vic_mask
#define  HDMI_HDMI_VIDEO_FORMAT_hvf(data)                                HDMI_P0_HDMI_VIDEO_FORMAT_hvf(data)
#define  HDMI_HDMI_VIDEO_FORMAT_hdmi_vic(data)                           HDMI_P0_HDMI_VIDEO_FORMAT_hdmi_vic(data)
#define  HDMI_HDMI_VIDEO_FORMAT_get_hvf(data)                            HDMI_P0_HDMI_VIDEO_FORMAT_get_hvf(data)
#define  HDMI_HDMI_VIDEO_FORMAT_get_hdmi_vic(data)                       HDMI_P0_HDMI_VIDEO_FORMAT_get_hdmi_vic(data)


#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_ext_data_mask                       HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_ext_data_mask
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_metadata_type_mask                  HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_metadata_type_mask
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_structure_mask                      HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_structure_mask
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_meta_present_mask                   HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_meta_present_mask
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_ext_data(data)                      HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_ext_data(data)
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_metadata_type(data)                 HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_metadata_type(data)
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_structure(data)                     HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_structure(data)
#define  HDMI_HDMI_3D_FORMAT_hdmi_3d_meta_present(data)                  HDMI_P0_HDMI_3D_FORMAT_hdmi_3d_meta_present(data)
#define  HDMI_HDMI_3D_FORMAT_get_hdmi_3d_ext_data(data)                  HDMI_P0_HDMI_3D_FORMAT_get_hdmi_3d_ext_data(data)
#define  HDMI_HDMI_3D_FORMAT_get_hdmi_3d_metadata_type(data)             HDMI_P0_HDMI_3D_FORMAT_get_hdmi_3d_metadata_type(data)
#define  HDMI_HDMI_3D_FORMAT_get_hdmi_3d_structure(data)                 HDMI_P0_HDMI_3D_FORMAT_get_hdmi_3d_structure(data)
#define  HDMI_HDMI_3D_FORMAT_get_hdmi_3d_meta_present(data)              HDMI_P0_HDMI_3D_FORMAT_get_hdmi_3d_meta_present(data)


#define  HDMI_HDMI_FVS_allm_mode_mask                                    HDMI_P0_HDMI_FVS_allm_mode_mask
#define  HDMI_HDMI_FVS_ccbpc_mask                                        HDMI_P0_HDMI_FVS_ccbpc_mask
#define  HDMI_HDMI_FVS_allm_mode(data)                                   HDMI_P0_HDMI_FVS_allm_mode(data)
#define  HDMI_HDMI_FVS_ccbpc(data)                                       HDMI_P0_HDMI_FVS_ccbpc(data)
#define  HDMI_HDMI_FVS_get_allm_mode(data)                               HDMI_P0_HDMI_FVS_get_allm_mode(data)
#define  HDMI_HDMI_FVS_get_ccbpc(data)                                   HDMI_P0_HDMI_FVS_get_ccbpc(data)


#define  HDMI_HDMI_DRM_static_metadata_descriptor_id_mask                HDMI_P0_HDMI_DRM_static_metadata_descriptor_id_mask
#define  HDMI_HDMI_DRM_eotf_mask                                         HDMI_P0_HDMI_DRM_eotf_mask
#define  HDMI_HDMI_DRM_static_metadata_descriptor_id(data)               HDMI_P0_HDMI_DRM_static_metadata_descriptor_id(data)
#define  HDMI_HDMI_DRM_eotf(data)                                        HDMI_P0_HDMI_DRM_eotf(data)
#define  HDMI_HDMI_DRM_get_static_metadata_descriptor_id(data)           HDMI_P0_HDMI_DRM_get_static_metadata_descriptor_id(data)
#define  HDMI_HDMI_DRM_get_eotf(data)                                    HDMI_P0_HDMI_DRM_get_eotf(data)


#define  HDMI_HDMI_PCMC_drm_info_clr_mask                                HDMI_P0_HDMI_PCMC_drm_info_clr_mask
#define  HDMI_HDMI_PCMC_mpeg_info_clr_mask                               HDMI_P0_HDMI_PCMC_mpeg_info_clr_mask
#define  HDMI_HDMI_PCMC_audio_info_clr_mask                              HDMI_P0_HDMI_PCMC_audio_info_clr_mask
#define  HDMI_HDMI_PCMC_spd_info_clr_mask                                HDMI_P0_HDMI_PCMC_spd_info_clr_mask
#define  HDMI_HDMI_PCMC_avi_info_clr_mask                                HDMI_P0_HDMI_PCMC_avi_info_clr_mask
#define  HDMI_HDMI_PCMC_hdr10pvs_info_clr_mask                           HDMI_P0_HDMI_PCMC_hdr10pvs_info_clr_mask
#define  HDMI_HDMI_PCMC_dvs_info_clr_mask                                HDMI_P0_HDMI_PCMC_dvs_info_clr_mask
#define  HDMI_HDMI_PCMC_fvs_info_clr_mask                                HDMI_P0_HDMI_PCMC_fvs_info_clr_mask
#define  HDMI_HDMI_PCMC_vs_info_clr_mask                                 HDMI_P0_HDMI_PCMC_vs_info_clr_mask
#define  HDMI_HDMI_PCMC_gm_clr_mask                                      HDMI_P0_HDMI_PCMC_gm_clr_mask
#define  HDMI_HDMI_PCMC_isrc2_clr_mask                                   HDMI_P0_HDMI_PCMC_isrc2_clr_mask
#define  HDMI_HDMI_PCMC_isrc1_clr_mask                                   HDMI_P0_HDMI_PCMC_isrc1_clr_mask
#define  HDMI_HDMI_PCMC_acp_clr_mask                                     HDMI_P0_HDMI_PCMC_acp_clr_mask
#define  HDMI_HDMI_PCMC_ackg_clr_mask                                    HDMI_P0_HDMI_PCMC_ackg_clr_mask
#define  HDMI_HDMI_PCMC_drm_info_clr(data)                               HDMI_P0_HDMI_PCMC_drm_info_clr(data)
#define  HDMI_HDMI_PCMC_mpeg_info_clr(data)                              HDMI_P0_HDMI_PCMC_mpeg_info_clr(data)
#define  HDMI_HDMI_PCMC_audio_info_clr(data)                             HDMI_P0_HDMI_PCMC_audio_info_clr(data)
#define  HDMI_HDMI_PCMC_spd_info_clr(data)                               HDMI_P0_HDMI_PCMC_spd_info_clr(data)
#define  HDMI_HDMI_PCMC_avi_info_clr(data)                               HDMI_P0_HDMI_PCMC_avi_info_clr(data)
#define  HDMI_HDMI_PCMC_hdr10pvs_info_clr(data)                          HDMI_P0_HDMI_PCMC_hdr10pvs_info_clr(data)
#define  HDMI_HDMI_PCMC_dvs_info_clr(data)                               HDMI_P0_HDMI_PCMC_dvs_info_clr(data)
#define  HDMI_HDMI_PCMC_fvs_info_clr(data)                               HDMI_P0_HDMI_PCMC_fvs_info_clr(data)
#define  HDMI_HDMI_PCMC_vs_info_clr(data)                                HDMI_P0_HDMI_PCMC_vs_info_clr(data)
#define  HDMI_HDMI_PCMC_gm_clr(data)                                     HDMI_P0_HDMI_PCMC_gm_clr(data)
#define  HDMI_HDMI_PCMC_isrc2_clr(data)                                  HDMI_P0_HDMI_PCMC_isrc2_clr(data)
#define  HDMI_HDMI_PCMC_isrc1_clr(data)                                  HDMI_P0_HDMI_PCMC_isrc1_clr(data)
#define  HDMI_HDMI_PCMC_acp_clr(data)                                    HDMI_P0_HDMI_PCMC_acp_clr(data)
#define  HDMI_HDMI_PCMC_ackg_clr(data)                                   HDMI_P0_HDMI_PCMC_ackg_clr(data)
#define  HDMI_HDMI_PCMC_get_drm_info_clr(data)                           HDMI_P0_HDMI_PCMC_get_drm_info_clr(data)
#define  HDMI_HDMI_PCMC_get_mpeg_info_clr(data)                          HDMI_P0_HDMI_PCMC_get_mpeg_info_clr(data)
#define  HDMI_HDMI_PCMC_get_audio_info_clr(data)                         HDMI_P0_HDMI_PCMC_get_audio_info_clr(data)
#define  HDMI_HDMI_PCMC_get_spd_info_clr(data)                           HDMI_P0_HDMI_PCMC_get_spd_info_clr(data)
#define  HDMI_HDMI_PCMC_get_avi_info_clr(data)                           HDMI_P0_HDMI_PCMC_get_avi_info_clr(data)
#define  HDMI_HDMI_PCMC_get_hdr10pvs_info_clr(data)                      HDMI_P0_HDMI_PCMC_get_hdr10pvs_info_clr(data)
#define  HDMI_HDMI_PCMC_get_dvs_info_clr(data)                           HDMI_P0_HDMI_PCMC_get_dvs_info_clr(data)
#define  HDMI_HDMI_PCMC_get_fvs_info_clr(data)                           HDMI_P0_HDMI_PCMC_get_fvs_info_clr(data)
#define  HDMI_HDMI_PCMC_get_vs_info_clr(data)                            HDMI_P0_HDMI_PCMC_get_vs_info_clr(data)
#define  HDMI_HDMI_PCMC_get_gm_clr(data)                                 HDMI_P0_HDMI_PCMC_get_gm_clr(data)
#define  HDMI_HDMI_PCMC_get_isrc2_clr(data)                              HDMI_P0_HDMI_PCMC_get_isrc2_clr(data)
#define  HDMI_HDMI_PCMC_get_isrc1_clr(data)                              HDMI_P0_HDMI_PCMC_get_isrc1_clr(data)
#define  HDMI_HDMI_PCMC_get_acp_clr(data)                                HDMI_P0_HDMI_PCMC_get_acp_clr(data)
#define  HDMI_HDMI_PCMC_get_ackg_clr(data)                               HDMI_P0_HDMI_PCMC_get_ackg_clr(data)


#define  HDMI_hdrap_apss_mask                                            HDMI_P0_hdrap_apss_mask
#define  HDMI_hdrap_apss(data)                                           HDMI_P0_hdrap_apss(data)
#define  HDMI_hdrap_get_apss(data)                                       HDMI_P0_hdrap_get_apss(data)


#define  HDMI_hdrdp_dpss_mask                                            HDMI_P0_hdrdp_dpss_mask
#define  HDMI_hdrdp_dpss(data)                                           HDMI_P0_hdrdp_dpss(data)
#define  HDMI_hdrdp_get_dpss(data)                                       HDMI_P0_hdrdp_get_dpss(data)


#define  HDMI_hdr_st_em_hdr_no_mask                                      HDMI_P0_hdr_st_em_hdr_no_mask
#define  HDMI_hdr_st_em_hdr_done_block_mask                              HDMI_P0_hdr_st_em_hdr_done_block_mask
#define  HDMI_hdr_st_em_hdr_timeout_mask                                 HDMI_P0_hdr_st_em_hdr_timeout_mask
#define  HDMI_hdr_st_em_hdr_discont_mask                                 HDMI_P0_hdr_st_em_hdr_discont_mask
#define  HDMI_hdr_st_em_hdr_end_mask                                     HDMI_P0_hdr_st_em_hdr_end_mask
#define  HDMI_hdr_st_em_hdr_new_mask                                     HDMI_P0_hdr_st_em_hdr_new_mask
#define  HDMI_hdr_st_em_hdr_first_mask                                   HDMI_P0_hdr_st_em_hdr_first_mask
#define  HDMI_hdr_st_em_hdr_last_mask                                    HDMI_P0_hdr_st_em_hdr_last_mask
#define  HDMI_hdr_st_em_hdr_no(data)                                     HDMI_P0_hdr_st_em_hdr_no(data)
#define  HDMI_hdr_st_em_hdr_done_block(data)                             HDMI_P0_hdr_st_em_hdr_done_block(data)
#define  HDMI_hdr_st_em_hdr_timeout(data)                                HDMI_P0_hdr_st_em_hdr_timeout(data)
#define  HDMI_hdr_st_em_hdr_discont(data)                                HDMI_P0_hdr_st_em_hdr_discont(data)
#define  HDMI_hdr_st_em_hdr_end(data)                                    HDMI_P0_hdr_st_em_hdr_end(data)
#define  HDMI_hdr_st_em_hdr_new(data)                                    HDMI_P0_hdr_st_em_hdr_new(data)
#define  HDMI_hdr_st_em_hdr_first(data)                                  HDMI_P0_hdr_st_em_hdr_first(data)
#define  HDMI_hdr_st_em_hdr_last(data)                                   HDMI_P0_hdr_st_em_hdr_last(data)
#define  HDMI_hdr_st_get_em_hdr_no(data)                                 HDMI_P0_hdr_st_get_em_hdr_no(data)
#define  HDMI_hdr_st_get_em_hdr_done_block(data)                         HDMI_P0_hdr_st_get_em_hdr_done_block(data)
#define  HDMI_hdr_st_get_em_hdr_timeout(data)                            HDMI_P0_hdr_st_get_em_hdr_timeout(data)
#define  HDMI_hdr_st_get_em_hdr_discont(data)                            HDMI_P0_hdr_st_get_em_hdr_discont(data)
#define  HDMI_hdr_st_get_em_hdr_end(data)                                HDMI_P0_hdr_st_get_em_hdr_end(data)
#define  HDMI_hdr_st_get_em_hdr_new(data)                                HDMI_P0_hdr_st_get_em_hdr_new(data)
#define  HDMI_hdr_st_get_em_hdr_first(data)                              HDMI_P0_hdr_st_get_em_hdr_first(data)
#define  HDMI_hdr_st_get_em_hdr_last(data)                               HDMI_P0_hdr_st_get_em_hdr_last(data)


#define  HDMI_em_ct_hdr_hd20_hd21_sel_mask                               HDMI_P0_em_ct_hdr_hd20_hd21_sel_mask
#define  HDMI_em_ct_org_id_mask                                          HDMI_P0_em_ct_org_id_mask
#define  HDMI_em_ct_set_tag_msb_mask                                     HDMI_P0_em_ct_set_tag_msb_mask
#define  HDMI_em_ct_set_tag_lsb_mask                                     HDMI_P0_em_ct_set_tag_lsb_mask
#define  HDMI_em_ct_recognize_tag_en_mask                                HDMI_P0_em_ct_recognize_tag_en_mask
#define  HDMI_em_ct_hdr_pkt_cnt_clr_mask                                 HDMI_P0_em_ct_hdr_pkt_cnt_clr_mask
#define  HDMI_em_ct_hdr_block_sel_mask                                   HDMI_P0_em_ct_hdr_block_sel_mask
#define  HDMI_em_ct_em_err2_proc_mask                                    HDMI_P0_em_ct_em_err2_proc_mask
#define  HDMI_em_ct_rec_em_hdr_irq_en_mask                               HDMI_P0_em_ct_rec_em_hdr_irq_en_mask
#define  HDMI_em_ct_hdr_hd20_hd21_sel(data)                              HDMI_P0_em_ct_hdr_hd20_hd21_sel(data)
#define  HDMI_em_ct_org_id(data)                                         HDMI_P0_em_ct_org_id(data)
#define  HDMI_em_ct_set_tag_msb(data)                                    HDMI_P0_em_ct_set_tag_msb(data)
#define  HDMI_em_ct_set_tag_lsb(data)                                    HDMI_P0_em_ct_set_tag_lsb(data)
#define  HDMI_em_ct_recognize_tag_en(data)                               HDMI_P0_em_ct_recognize_tag_en(data)
#define  HDMI_em_ct_hdr_pkt_cnt_clr(data)                                HDMI_P0_em_ct_hdr_pkt_cnt_clr(data)
#define  HDMI_em_ct_hdr_block_sel(data)                                  HDMI_P0_em_ct_hdr_block_sel(data)
#define  HDMI_em_ct_em_err2_proc(data)                                   HDMI_P0_em_ct_em_err2_proc(data)
#define  HDMI_em_ct_rec_em_hdr_irq_en(data)                              HDMI_P0_em_ct_rec_em_hdr_irq_en(data)
#define  HDMI_em_ct_get_hdr_hd20_hd21_sel(data)                          HDMI_P0_em_ct_get_hdr_hd20_hd21_sel(data)
#define  HDMI_em_ct_get_org_id(data)                                     HDMI_P0_em_ct_get_org_id(data)
#define  HDMI_em_ct_get_set_tag_msb(data)                                HDMI_P0_em_ct_get_set_tag_msb(data)
#define  HDMI_em_ct_get_set_tag_lsb(data)                                HDMI_P0_em_ct_get_set_tag_lsb(data)
#define  HDMI_em_ct_get_recognize_tag_en(data)                           HDMI_P0_em_ct_get_recognize_tag_en(data)
#define  HDMI_em_ct_get_hdr_pkt_cnt_clr(data)                            HDMI_P0_em_ct_get_hdr_pkt_cnt_clr(data)
#define  HDMI_em_ct_get_hdr_block_sel(data)                              HDMI_P0_em_ct_get_hdr_block_sel(data)
#define  HDMI_em_ct_get_em_err2_proc(data)                               HDMI_P0_em_ct_get_em_err2_proc(data)
#define  HDMI_em_ct_get_rec_em_hdr_irq_en(data)                          HDMI_P0_em_ct_get_rec_em_hdr_irq_en(data)


#define  HDMI_fapa_ct_end_mask                                           HDMI_P0_fapa_ct_end_mask
#define  HDMI_fapa_ct_em_timeout_frame_mask                              HDMI_P0_fapa_ct_em_timeout_frame_mask
#define  HDMI_fapa_ct_mode_mask                                          HDMI_P0_fapa_ct_mode_mask
#define  HDMI_fapa_ct_end(data)                                          HDMI_P0_fapa_ct_end(data)
#define  HDMI_fapa_ct_em_timeout_frame(data)                             HDMI_P0_fapa_ct_em_timeout_frame(data)
#define  HDMI_fapa_ct_mode(data)                                         HDMI_P0_fapa_ct_mode(data)
#define  HDMI_fapa_ct_get_end(data)                                      HDMI_P0_fapa_ct_get_end(data)
#define  HDMI_fapa_ct_get_em_timeout_frame(data)                         HDMI_P0_fapa_ct_get_em_timeout_frame(data)
#define  HDMI_fapa_ct_get_mode(data)                                     HDMI_P0_fapa_ct_get_mode(data)


#define  HDMI_HDMI_EMAP_apss_mask                                        HDMI_P0_HDMI_EMAP_apss_mask
#define  HDMI_HDMI_EMAP_apss(data)                                       HDMI_P0_HDMI_EMAP_apss(data)
#define  HDMI_HDMI_EMAP_get_apss(data)                                   HDMI_P0_HDMI_EMAP_get_apss(data)


#define  HDMI_HDMI_EMDP_dpss_mask                                        HDMI_P0_HDMI_EMDP_dpss_mask
#define  HDMI_HDMI_EMDP_dpss(data)                                       HDMI_P0_HDMI_EMDP_dpss(data)
#define  HDMI_HDMI_EMDP_get_dpss(data)                                   HDMI_P0_HDMI_EMDP_get_dpss(data)


#define  HDMI_VRREM_ST_em_vrr_no_mask                                    HDMI_P0_VRREM_ST_em_vrr_no_mask
#define  HDMI_VRREM_ST_em_vrr_timeout_mask                               HDMI_P0_VRREM_ST_em_vrr_timeout_mask
#define  HDMI_VRREM_ST_em_vrr_discont_mask                               HDMI_P0_VRREM_ST_em_vrr_discont_mask
#define  HDMI_VRREM_ST_em_vrr_end_mask                                   HDMI_P0_VRREM_ST_em_vrr_end_mask
#define  HDMI_VRREM_ST_em_vrr_new_mask                                   HDMI_P0_VRREM_ST_em_vrr_new_mask
#define  HDMI_VRREM_ST_em_vrr_first_mask                                 HDMI_P0_VRREM_ST_em_vrr_first_mask
#define  HDMI_VRREM_ST_em_vrr_last_mask                                  HDMI_P0_VRREM_ST_em_vrr_last_mask
#define  HDMI_VRREM_ST_em_vrr_no(data)                                   HDMI_P0_VRREM_ST_em_vrr_no(data)
#define  HDMI_VRREM_ST_em_vrr_timeout(data)                              HDMI_P0_VRREM_ST_em_vrr_timeout(data)
#define  HDMI_VRREM_ST_em_vrr_discont(data)                              HDMI_P0_VRREM_ST_em_vrr_discont(data)
#define  HDMI_VRREM_ST_em_vrr_end(data)                                  HDMI_P0_VRREM_ST_em_vrr_end(data)
#define  HDMI_VRREM_ST_em_vrr_new(data)                                  HDMI_P0_VRREM_ST_em_vrr_new(data)
#define  HDMI_VRREM_ST_em_vrr_first(data)                                HDMI_P0_VRREM_ST_em_vrr_first(data)
#define  HDMI_VRREM_ST_em_vrr_last(data)                                 HDMI_P0_VRREM_ST_em_vrr_last(data)
#define  HDMI_VRREM_ST_get_em_vrr_no(data)                               HDMI_P0_VRREM_ST_get_em_vrr_no(data)
#define  HDMI_VRREM_ST_get_em_vrr_timeout(data)                          HDMI_P0_VRREM_ST_get_em_vrr_timeout(data)
#define  HDMI_VRREM_ST_get_em_vrr_discont(data)                          HDMI_P0_VRREM_ST_get_em_vrr_discont(data)
#define  HDMI_VRREM_ST_get_em_vrr_end(data)                              HDMI_P0_VRREM_ST_get_em_vrr_end(data)
#define  HDMI_VRREM_ST_get_em_vrr_new(data)                              HDMI_P0_VRREM_ST_get_em_vrr_new(data)
#define  HDMI_VRREM_ST_get_em_vrr_first(data)                            HDMI_P0_VRREM_ST_get_em_vrr_first(data)
#define  HDMI_VRREM_ST_get_em_vrr_last(data)                             HDMI_P0_VRREM_ST_get_em_vrr_last(data)


#define  HDMI_HDMI_VRR_EMC_vrr_clr_mask                                  HDMI_P0_HDMI_VRR_EMC_vrr_clr_mask
#define  HDMI_HDMI_VRR_EMC_rec_em_vrr_irq_en_mask                        HDMI_P0_HDMI_VRR_EMC_rec_em_vrr_irq_en_mask
#define  HDMI_HDMI_VRR_EMC_vrr_clr(data)                                 HDMI_P0_HDMI_VRR_EMC_vrr_clr(data)
#define  HDMI_HDMI_VRR_EMC_rec_em_vrr_irq_en(data)                       HDMI_P0_HDMI_VRR_EMC_rec_em_vrr_irq_en(data)
#define  HDMI_HDMI_VRR_EMC_get_vrr_clr(data)                             HDMI_P0_HDMI_VRR_EMC_get_vrr_clr(data)
#define  HDMI_HDMI_VRR_EMC_get_rec_em_vrr_irq_en(data)                   HDMI_P0_HDMI_VRR_EMC_get_rec_em_vrr_irq_en(data)


#define  HDMI_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq_mask          HDMI_P0_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq_mask
#define  HDMI_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq_mask             HDMI_P0_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq_mask
#define  HDMI_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq(data)         HDMI_P0_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq(data)
#define  HDMI_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq(data)            HDMI_P0_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq(data)
#define  HDMI_Audio_Sample_Rate_Change_IRQ_get_en_ncts_chg_irq(data)     HDMI_P0_Audio_Sample_Rate_Change_IRQ_get_en_ncts_chg_irq(data)
#define  HDMI_Audio_Sample_Rate_Change_IRQ_get_ncts_chg_irq(data)        HDMI_P0_Audio_Sample_Rate_Change_IRQ_get_ncts_chg_irq(data)


#define  HDMI_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask             HDMI_P0_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask
#define  HDMI_High_Bit_Rate_Audio_Packet_hbr_audio_mode(data)            HDMI_P0_High_Bit_Rate_Audio_Packet_hbr_audio_mode(data)
#define  HDMI_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(data)        HDMI_P0_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(data)


#define  HDMI_HDMI_AUDCR_aud_data_fw_mask                                HDMI_P0_HDMI_AUDCR_aud_data_fw_mask
#define  HDMI_HDMI_AUDCR_pck_hbr_aud_gated_en_mask                       HDMI_P0_HDMI_AUDCR_pck_hbr_aud_gated_en_mask
#define  HDMI_HDMI_AUDCR_pck_aud_gated_en_mask                           HDMI_P0_HDMI_AUDCR_pck_aud_gated_en_mask
#define  HDMI_HDMI_AUDCR_aud_cptest_en_mask                              HDMI_P0_HDMI_AUDCR_aud_cptest_en_mask
#define  HDMI_HDMI_AUDCR_sine_replace_en_mask                            HDMI_P0_HDMI_AUDCR_sine_replace_en_mask
#define  HDMI_HDMI_AUDCR_bch2_repeat_en_mask                             HDMI_P0_HDMI_AUDCR_bch2_repeat_en_mask
#define  HDMI_HDMI_AUDCR_flat_en_mask                                    HDMI_P0_HDMI_AUDCR_flat_en_mask
#define  HDMI_HDMI_AUDCR_aud_en_mask                                     HDMI_P0_HDMI_AUDCR_aud_en_mask
#define  HDMI_HDMI_AUDCR_aud_data_fw(data)                               HDMI_P0_HDMI_AUDCR_aud_data_fw(data)
#define  HDMI_HDMI_AUDCR_pck_hbr_aud_gated_en(data)                      HDMI_P0_HDMI_AUDCR_pck_hbr_aud_gated_en(data)
#define  HDMI_HDMI_AUDCR_pck_aud_gated_en(data)                          HDMI_P0_HDMI_AUDCR_pck_aud_gated_en(data)
#define  HDMI_HDMI_AUDCR_aud_cptest_en(data)                             HDMI_P0_HDMI_AUDCR_aud_cptest_en(data)
#define  HDMI_HDMI_AUDCR_sine_replace_en(data)                           HDMI_P0_HDMI_AUDCR_sine_replace_en(data)
#define  HDMI_HDMI_AUDCR_bch2_repeat_en(data)                            HDMI_P0_HDMI_AUDCR_bch2_repeat_en(data)
#define  HDMI_HDMI_AUDCR_flat_en(data)                                   HDMI_P0_HDMI_AUDCR_flat_en(data)
#define  HDMI_HDMI_AUDCR_aud_en(data)                                    HDMI_P0_HDMI_AUDCR_aud_en(data)
#define  HDMI_HDMI_AUDCR_get_aud_data_fw(data)                           HDMI_P0_HDMI_AUDCR_get_aud_data_fw(data)
#define  HDMI_HDMI_AUDCR_get_pck_hbr_aud_gated_en(data)                  HDMI_P0_HDMI_AUDCR_get_pck_hbr_aud_gated_en(data)
#define  HDMI_HDMI_AUDCR_get_pck_aud_gated_en(data)                      HDMI_P0_HDMI_AUDCR_get_pck_aud_gated_en(data)
#define  HDMI_HDMI_AUDCR_get_aud_cptest_en(data)                         HDMI_P0_HDMI_AUDCR_get_aud_cptest_en(data)
#define  HDMI_HDMI_AUDCR_get_sine_replace_en(data)                       HDMI_P0_HDMI_AUDCR_get_sine_replace_en(data)
#define  HDMI_HDMI_AUDCR_get_bch2_repeat_en(data)                        HDMI_P0_HDMI_AUDCR_get_bch2_repeat_en(data)
#define  HDMI_HDMI_AUDCR_get_flat_en(data)                               HDMI_P0_HDMI_AUDCR_get_flat_en(data)
#define  HDMI_HDMI_AUDCR_get_aud_en(data)                                HDMI_P0_HDMI_AUDCR_get_aud_en(data)


#define  HDMI_HDMI_AUDSR_aud_ch_mode_mask                                HDMI_P0_HDMI_AUDSR_aud_ch_mode_mask
#define  HDMI_HDMI_AUDSR_aud_fifof_mask                                  HDMI_P0_HDMI_AUDSR_aud_fifof_mask
#define  HDMI_HDMI_AUDSR_aud_ch_mode(data)                               HDMI_P0_HDMI_AUDSR_aud_ch_mode(data)
#define  HDMI_HDMI_AUDSR_aud_fifof(data)                                 HDMI_P0_HDMI_AUDSR_aud_fifof(data)
#define  HDMI_HDMI_AUDSR_get_aud_ch_mode(data)                           HDMI_P0_HDMI_AUDSR_get_aud_ch_mode(data)
#define  HDMI_HDMI_AUDSR_get_aud_fifof(data)                             HDMI_P0_HDMI_AUDSR_get_aud_fifof(data)


#define  HDMI_CLKDETCR_clkv_meas_sel_mask                                HDMI_P0_CLKDETCR_clkv_meas_sel_mask
#define  HDMI_CLKDETCR_clock_det_en_mask                                 HDMI_P0_CLKDETCR_clock_det_en_mask
#define  HDMI_CLKDETCR_clkv_meas_sel(data)                               HDMI_P0_CLKDETCR_clkv_meas_sel(data)
#define  HDMI_CLKDETCR_clock_det_en(data)                                HDMI_P0_CLKDETCR_clock_det_en(data)
#define  HDMI_CLKDETCR_get_clkv_meas_sel(data)                           HDMI_P0_CLKDETCR_get_clkv_meas_sel(data)
#define  HDMI_CLKDETCR_get_clock_det_en(data)                            HDMI_P0_CLKDETCR_get_clock_det_en(data)


#define  HDMI_CLKDETSR_reset_counter_mask                                HDMI_P0_CLKDETSR_reset_counter_mask
#define  HDMI_CLKDETSR_pop_out_mask                                      HDMI_P0_CLKDETSR_pop_out_mask
#define  HDMI_CLKDETSR_clk_counter_mask                                  HDMI_P0_CLKDETSR_clk_counter_mask
#define  HDMI_CLKDETSR_en_clk_chg_irq_mask                               HDMI_P0_CLKDETSR_en_clk_chg_irq_mask
#define  HDMI_CLKDETSR_clk_chg_irq_mask                                  HDMI_P0_CLKDETSR_clk_chg_irq_mask
#define  HDMI_CLKDETSR_dummy_3_2_mask                                    HDMI_P0_CLKDETSR_dummy_3_2_mask
#define  HDMI_CLKDETSR_clk_in_target_irq_en_mask                         HDMI_P0_CLKDETSR_clk_in_target_irq_en_mask
#define  HDMI_CLKDETSR_clk_in_target_mask                                HDMI_P0_CLKDETSR_clk_in_target_mask
#define  HDMI_CLKDETSR_reset_counter(data)                               HDMI_P0_CLKDETSR_reset_counter(data)
#define  HDMI_CLKDETSR_pop_out(data)                                     HDMI_P0_CLKDETSR_pop_out(data)
#define  HDMI_CLKDETSR_clk_counter(data)                                 HDMI_P0_CLKDETSR_clk_counter(data)
#define  HDMI_CLKDETSR_en_clk_chg_irq(data)                              HDMI_P0_CLKDETSR_en_clk_chg_irq(data)
#define  HDMI_CLKDETSR_clk_chg_irq(data)                                 HDMI_P0_CLKDETSR_clk_chg_irq(data)
#define  HDMI_CLKDETSR_dummy_3_2(data)                                   HDMI_P0_CLKDETSR_dummy_3_2(data)
#define  HDMI_CLKDETSR_clk_in_target_irq_en(data)                        HDMI_P0_CLKDETSR_clk_in_target_irq_en(data)
#define  HDMI_CLKDETSR_clk_in_target(data)                               HDMI_P0_CLKDETSR_clk_in_target(data)
#define  HDMI_CLKDETSR_get_reset_counter(data)                           HDMI_P0_CLKDETSR_get_reset_counter(data)
#define  HDMI_CLKDETSR_get_pop_out(data)                                 HDMI_P0_CLKDETSR_get_pop_out(data)
#define  HDMI_CLKDETSR_get_clk_counter(data)                             HDMI_P0_CLKDETSR_get_clk_counter(data)
#define  HDMI_CLKDETSR_get_en_clk_chg_irq(data)                          HDMI_P0_CLKDETSR_get_en_clk_chg_irq(data)
#define  HDMI_CLKDETSR_get_clk_chg_irq(data)                             HDMI_P0_CLKDETSR_get_clk_chg_irq(data)
#define  HDMI_CLKDETSR_get_dummy_3_2(data)                               HDMI_P0_CLKDETSR_get_dummy_3_2(data)
#define  HDMI_CLKDETSR_get_clk_in_target_irq_en(data)                    HDMI_P0_CLKDETSR_get_clk_in_target_irq_en(data)
#define  HDMI_CLKDETSR_get_clk_in_target(data)                           HDMI_P0_CLKDETSR_get_clk_in_target(data)


#define  HDMI_CLK_SETTING_00_dclk_cnt_start_mask                         HDMI_P0_CLK_SETTING_00_dclk_cnt_start_mask
#define  HDMI_CLK_SETTING_00_dclk_cnt_end_mask                           HDMI_P0_CLK_SETTING_00_dclk_cnt_end_mask
#define  HDMI_CLK_SETTING_00_dclk_cnt_start(data)                        HDMI_P0_CLK_SETTING_00_dclk_cnt_start(data)
#define  HDMI_CLK_SETTING_00_dclk_cnt_end(data)                          HDMI_P0_CLK_SETTING_00_dclk_cnt_end(data)
#define  HDMI_CLK_SETTING_00_get_dclk_cnt_start(data)                    HDMI_P0_CLK_SETTING_00_get_dclk_cnt_start(data)
#define  HDMI_CLK_SETTING_00_get_dclk_cnt_end(data)                      HDMI_P0_CLK_SETTING_00_get_dclk_cnt_end(data)


#define  HDMI_CLK_SETTING_01_target_for_maximum_clk_counter_mask         HDMI_P0_CLK_SETTING_01_target_for_maximum_clk_counter_mask
#define  HDMI_CLK_SETTING_01_target_for_minimum_clk_counter_mask         HDMI_P0_CLK_SETTING_01_target_for_minimum_clk_counter_mask
#define  HDMI_CLK_SETTING_01_target_for_maximum_clk_counter(data)        HDMI_P0_CLK_SETTING_01_target_for_maximum_clk_counter(data)
#define  HDMI_CLK_SETTING_01_target_for_minimum_clk_counter(data)        HDMI_P0_CLK_SETTING_01_target_for_minimum_clk_counter(data)
#define  HDMI_CLK_SETTING_01_get_target_for_maximum_clk_counter(data)    HDMI_P0_CLK_SETTING_01_get_target_for_maximum_clk_counter(data)
#define  HDMI_CLK_SETTING_01_get_target_for_minimum_clk_counter(data)    HDMI_P0_CLK_SETTING_01_get_target_for_minimum_clk_counter(data)


#define  HDMI_CLK_SETTING_02_clk_counter_err_threshold_mask              HDMI_P0_CLK_SETTING_02_clk_counter_err_threshold_mask
#define  HDMI_CLK_SETTING_02_clk_counter_debounce_mask                   HDMI_P0_CLK_SETTING_02_clk_counter_debounce_mask
#define  HDMI_CLK_SETTING_02_clk_counter_err_threshold(data)             HDMI_P0_CLK_SETTING_02_clk_counter_err_threshold(data)
#define  HDMI_CLK_SETTING_02_clk_counter_debounce(data)                  HDMI_P0_CLK_SETTING_02_clk_counter_debounce(data)
#define  HDMI_CLK_SETTING_02_get_clk_counter_err_threshold(data)         HDMI_P0_CLK_SETTING_02_get_clk_counter_err_threshold(data)
#define  HDMI_CLK_SETTING_02_get_clk_counter_debounce(data)              HDMI_P0_CLK_SETTING_02_get_clk_counter_debounce(data)


#define  HDMI_PRBS_R_CTRL_prbs23_rxen_mask                               HDMI_P0_PRBS_R_CTRL_prbs23_rxen_mask
#define  HDMI_PRBS_R_CTRL_prbs_rxbist_err_cnt_mask                       HDMI_P0_PRBS_R_CTRL_prbs_rxbist_err_cnt_mask
#define  HDMI_PRBS_R_CTRL_prbs_err_cnt_mask                              HDMI_P0_PRBS_R_CTRL_prbs_err_cnt_mask
#define  HDMI_PRBS_R_CTRL_prbs_bit_err_mask                              HDMI_P0_PRBS_R_CTRL_prbs_bit_err_mask
#define  HDMI_PRBS_R_CTRL_prbs_lock_mask                                 HDMI_P0_PRBS_R_CTRL_prbs_lock_mask
#define  HDMI_PRBS_R_CTRL_prbs_err_cnt_clr_mask                          HDMI_P0_PRBS_R_CTRL_prbs_err_cnt_clr_mask
#define  HDMI_PRBS_R_CTRL_prbs_reverse_mask                              HDMI_P0_PRBS_R_CTRL_prbs_reverse_mask
#define  HDMI_PRBS_R_CTRL_prbs7_rxen_mask                                HDMI_P0_PRBS_R_CTRL_prbs7_rxen_mask
#define  HDMI_PRBS_R_CTRL_prbs23_rxen(data)                              HDMI_P0_PRBS_R_CTRL_prbs23_rxen(data)
#define  HDMI_PRBS_R_CTRL_prbs_rxbist_err_cnt(data)                      HDMI_P0_PRBS_R_CTRL_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_R_CTRL_prbs_err_cnt(data)                             HDMI_P0_PRBS_R_CTRL_prbs_err_cnt(data)
#define  HDMI_PRBS_R_CTRL_prbs_bit_err(data)                             HDMI_P0_PRBS_R_CTRL_prbs_bit_err(data)
#define  HDMI_PRBS_R_CTRL_prbs_lock(data)                                HDMI_P0_PRBS_R_CTRL_prbs_lock(data)
#define  HDMI_PRBS_R_CTRL_prbs_err_cnt_clr(data)                         HDMI_P0_PRBS_R_CTRL_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_R_CTRL_prbs_reverse(data)                             HDMI_P0_PRBS_R_CTRL_prbs_reverse(data)
#define  HDMI_PRBS_R_CTRL_prbs7_rxen(data)                               HDMI_P0_PRBS_R_CTRL_prbs7_rxen(data)
#define  HDMI_PRBS_R_CTRL_get_prbs23_rxen(data)                          HDMI_P0_PRBS_R_CTRL_get_prbs23_rxen(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_rxbist_err_cnt(data)                  HDMI_P0_PRBS_R_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_err_cnt(data)                         HDMI_P0_PRBS_R_CTRL_get_prbs_err_cnt(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_bit_err(data)                         HDMI_P0_PRBS_R_CTRL_get_prbs_bit_err(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_lock(data)                            HDMI_P0_PRBS_R_CTRL_get_prbs_lock(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_err_cnt_clr(data)                     HDMI_P0_PRBS_R_CTRL_get_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_R_CTRL_get_prbs_reverse(data)                         HDMI_P0_PRBS_R_CTRL_get_prbs_reverse(data)
#define  HDMI_PRBS_R_CTRL_get_prbs7_rxen(data)                           HDMI_P0_PRBS_R_CTRL_get_prbs7_rxen(data)


#define  HDMI_PRBS_G_CTRL_prbs23_rxen_mask                               HDMI_P0_PRBS_G_CTRL_prbs23_rxen_mask
#define  HDMI_PRBS_G_CTRL_prbs_rxbist_err_cnt_mask                       HDMI_P0_PRBS_G_CTRL_prbs_rxbist_err_cnt_mask
#define  HDMI_PRBS_G_CTRL_prbs_err_cnt_mask                              HDMI_P0_PRBS_G_CTRL_prbs_err_cnt_mask
#define  HDMI_PRBS_G_CTRL_prbs_bit_err_mask                              HDMI_P0_PRBS_G_CTRL_prbs_bit_err_mask
#define  HDMI_PRBS_G_CTRL_prbs_lock_mask                                 HDMI_P0_PRBS_G_CTRL_prbs_lock_mask
#define  HDMI_PRBS_G_CTRL_prbs_err_cnt_clr_mask                          HDMI_P0_PRBS_G_CTRL_prbs_err_cnt_clr_mask
#define  HDMI_PRBS_G_CTRL_prbs_reverse_mask                              HDMI_P0_PRBS_G_CTRL_prbs_reverse_mask
#define  HDMI_PRBS_G_CTRL_prbs7_rxen_mask                                HDMI_P0_PRBS_G_CTRL_prbs7_rxen_mask
#define  HDMI_PRBS_G_CTRL_prbs23_rxen(data)                              HDMI_P0_PRBS_G_CTRL_prbs23_rxen(data)
#define  HDMI_PRBS_G_CTRL_prbs_rxbist_err_cnt(data)                      HDMI_P0_PRBS_G_CTRL_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_G_CTRL_prbs_err_cnt(data)                             HDMI_P0_PRBS_G_CTRL_prbs_err_cnt(data)
#define  HDMI_PRBS_G_CTRL_prbs_bit_err(data)                             HDMI_P0_PRBS_G_CTRL_prbs_bit_err(data)
#define  HDMI_PRBS_G_CTRL_prbs_lock(data)                                HDMI_P0_PRBS_G_CTRL_prbs_lock(data)
#define  HDMI_PRBS_G_CTRL_prbs_err_cnt_clr(data)                         HDMI_P0_PRBS_G_CTRL_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_G_CTRL_prbs_reverse(data)                             HDMI_P0_PRBS_G_CTRL_prbs_reverse(data)
#define  HDMI_PRBS_G_CTRL_prbs7_rxen(data)                               HDMI_P0_PRBS_G_CTRL_prbs7_rxen(data)
#define  HDMI_PRBS_G_CTRL_get_prbs23_rxen(data)                          HDMI_P0_PRBS_G_CTRL_get_prbs23_rxen(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_rxbist_err_cnt(data)                  HDMI_P0_PRBS_G_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_err_cnt(data)                         HDMI_P0_PRBS_G_CTRL_get_prbs_err_cnt(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_bit_err(data)                         HDMI_P0_PRBS_G_CTRL_get_prbs_bit_err(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_lock(data)                            HDMI_P0_PRBS_G_CTRL_get_prbs_lock(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_err_cnt_clr(data)                     HDMI_P0_PRBS_G_CTRL_get_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_G_CTRL_get_prbs_reverse(data)                         HDMI_P0_PRBS_G_CTRL_get_prbs_reverse(data)
#define  HDMI_PRBS_G_CTRL_get_prbs7_rxen(data)                           HDMI_P0_PRBS_G_CTRL_get_prbs7_rxen(data)


#define  HDMI_PRBS_B_CTRL_prbs23_rxen_mask                               HDMI_P0_PRBS_B_CTRL_prbs23_rxen_mask
#define  HDMI_PRBS_B_CTRL_prbs_rxbist_err_cnt_mask                       HDMI_P0_PRBS_B_CTRL_prbs_rxbist_err_cnt_mask
#define  HDMI_PRBS_B_CTRL_prbs_err_cnt_mask                              HDMI_P0_PRBS_B_CTRL_prbs_err_cnt_mask
#define  HDMI_PRBS_B_CTRL_prbs_bit_err_mask                              HDMI_P0_PRBS_B_CTRL_prbs_bit_err_mask
#define  HDMI_PRBS_B_CTRL_prbs_lock_mask                                 HDMI_P0_PRBS_B_CTRL_prbs_lock_mask
#define  HDMI_PRBS_B_CTRL_prbs_err_cnt_clr_mask                          HDMI_P0_PRBS_B_CTRL_prbs_err_cnt_clr_mask
#define  HDMI_PRBS_B_CTRL_prbs_reverse_mask                              HDMI_P0_PRBS_B_CTRL_prbs_reverse_mask
#define  HDMI_PRBS_B_CTRL_prbs7_rxen_mask                                HDMI_P0_PRBS_B_CTRL_prbs7_rxen_mask
#define  HDMI_PRBS_B_CTRL_prbs23_rxen(data)                              HDMI_P0_PRBS_B_CTRL_prbs23_rxen(data)
#define  HDMI_PRBS_B_CTRL_prbs_rxbist_err_cnt(data)                      HDMI_P0_PRBS_B_CTRL_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_B_CTRL_prbs_err_cnt(data)                             HDMI_P0_PRBS_B_CTRL_prbs_err_cnt(data)
#define  HDMI_PRBS_B_CTRL_prbs_bit_err(data)                             HDMI_P0_PRBS_B_CTRL_prbs_bit_err(data)
#define  HDMI_PRBS_B_CTRL_prbs_lock(data)                                HDMI_P0_PRBS_B_CTRL_prbs_lock(data)
#define  HDMI_PRBS_B_CTRL_prbs_err_cnt_clr(data)                         HDMI_P0_PRBS_B_CTRL_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_B_CTRL_prbs_reverse(data)                             HDMI_P0_PRBS_B_CTRL_prbs_reverse(data)
#define  HDMI_PRBS_B_CTRL_prbs7_rxen(data)                               HDMI_P0_PRBS_B_CTRL_prbs7_rxen(data)
#define  HDMI_PRBS_B_CTRL_get_prbs23_rxen(data)                          HDMI_P0_PRBS_B_CTRL_get_prbs23_rxen(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_rxbist_err_cnt(data)                  HDMI_P0_PRBS_B_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_err_cnt(data)                         HDMI_P0_PRBS_B_CTRL_get_prbs_err_cnt(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_bit_err(data)                         HDMI_P0_PRBS_B_CTRL_get_prbs_bit_err(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_lock(data)                            HDMI_P0_PRBS_B_CTRL_get_prbs_lock(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_err_cnt_clr(data)                     HDMI_P0_PRBS_B_CTRL_get_prbs_err_cnt_clr(data)
#define  HDMI_PRBS_B_CTRL_get_prbs_reverse(data)                         HDMI_P0_PRBS_B_CTRL_get_prbs_reverse(data)
#define  HDMI_PRBS_B_CTRL_get_prbs7_rxen(data)                           HDMI_P0_PRBS_B_CTRL_get_prbs7_rxen(data)


#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_err_flag_mask                 HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_err_flag_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_err_int_en_mask               HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_err_int_en_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_preamble_mask                     HDMI_P0_HDMI_leading_GB_cmp_ctrl_preamble_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_value_mask                    HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_value_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_dvi_mode_sel_mask                 HDMI_P0_HDMI_leading_GB_cmp_ctrl_dvi_mode_sel_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_channel_sel_mask                  HDMI_P0_HDMI_leading_GB_cmp_ctrl_channel_sel_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_lgb_cal_conti_mask                HDMI_P0_HDMI_leading_GB_cmp_ctrl_lgb_cal_conti_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_lgb_cal_en_mask                   HDMI_P0_HDMI_leading_GB_cmp_ctrl_lgb_cal_en_mask
#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_err_flag(data)                HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_err_flag(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_err_int_en(data)              HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_err_int_en(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_preamble(data)                    HDMI_P0_HDMI_leading_GB_cmp_ctrl_preamble(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_cmp_value(data)                   HDMI_P0_HDMI_leading_GB_cmp_ctrl_cmp_value(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_dvi_mode_sel(data)                HDMI_P0_HDMI_leading_GB_cmp_ctrl_dvi_mode_sel(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_channel_sel(data)                 HDMI_P0_HDMI_leading_GB_cmp_ctrl_channel_sel(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_lgb_cal_conti(data)               HDMI_P0_HDMI_leading_GB_cmp_ctrl_lgb_cal_conti(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_lgb_cal_en(data)                  HDMI_P0_HDMI_leading_GB_cmp_ctrl_lgb_cal_en(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_cmp_err_flag(data)            HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_cmp_err_flag(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_cmp_err_int_en(data)          HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_cmp_err_int_en(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_preamble(data)                HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_preamble(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_cmp_value(data)               HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_cmp_value(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_dvi_mode_sel(data)            HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_dvi_mode_sel(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_channel_sel(data)             HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_channel_sel(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_lgb_cal_conti(data)           HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_lgb_cal_conti(data)
#define  HDMI_HDMI_leading_GB_cmp_ctrl_get_lgb_cal_en(data)              HDMI_P0_HDMI_leading_GB_cmp_ctrl_get_lgb_cal_en(data)


#define  HDMI_HDMI_leading_GB_cmp_times_cmp_err_cnt_mask                 HDMI_P0_HDMI_leading_GB_cmp_times_cmp_err_cnt_mask
#define  HDMI_HDMI_leading_GB_cmp_times_cmp_times_mask                   HDMI_P0_HDMI_leading_GB_cmp_times_cmp_times_mask
#define  HDMI_HDMI_leading_GB_cmp_times_cmp_err_cnt(data)                HDMI_P0_HDMI_leading_GB_cmp_times_cmp_err_cnt(data)
#define  HDMI_HDMI_leading_GB_cmp_times_cmp_times(data)                  HDMI_P0_HDMI_leading_GB_cmp_times_cmp_times(data)
#define  HDMI_HDMI_leading_GB_cmp_times_get_cmp_err_cnt(data)            HDMI_P0_HDMI_leading_GB_cmp_times_get_cmp_err_cnt(data)
#define  HDMI_HDMI_leading_GB_cmp_times_get_cmp_times(data)              HDMI_P0_HDMI_leading_GB_cmp_times_get_cmp_times(data)


#define  HDMI_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_mask                     HDMI_P0_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_mask
#define  HDMI_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_by_frame_mask            HDMI_P0_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_by_frame_mask
#define  HDMI_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt(data)                    HDMI_P0_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt(data)
#define  HDMI_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_by_frame(data)           HDMI_P0_HDMI_leading_GB_cmp_cnt_preamble_cmp_cnt_by_frame(data)
#define  HDMI_HDMI_leading_GB_cmp_cnt_get_preamble_cmp_cnt(data)                HDMI_P0_HDMI_leading_GB_cmp_cnt_get_preamble_cmp_cnt(data)
#define  HDMI_HDMI_leading_GB_cmp_cnt_get_preamble_cmp_cnt_by_frame(data)       HDMI_P0_HDMI_leading_GB_cmp_cnt_get_preamble_cmp_cnt_by_frame(data)


#define  HDMI_AUDIO_CTS_UP_BOUND_cts_over_flag_mask                      HDMI_P0_AUDIO_CTS_UP_BOUND_cts_over_flag_mask
#define  HDMI_AUDIO_CTS_UP_BOUND_cts_up_bound_mask                       HDMI_P0_AUDIO_CTS_UP_BOUND_cts_up_bound_mask
#define  HDMI_AUDIO_CTS_UP_BOUND_cts_over_flag(data)                     HDMI_P0_AUDIO_CTS_UP_BOUND_cts_over_flag(data)
#define  HDMI_AUDIO_CTS_UP_BOUND_cts_up_bound(data)                      HDMI_P0_AUDIO_CTS_UP_BOUND_cts_up_bound(data)
#define  HDMI_AUDIO_CTS_UP_BOUND_get_cts_over_flag(data)                 HDMI_P0_AUDIO_CTS_UP_BOUND_get_cts_over_flag(data)
#define  HDMI_AUDIO_CTS_UP_BOUND_get_cts_up_bound(data)                  HDMI_P0_AUDIO_CTS_UP_BOUND_get_cts_up_bound(data)


#define  HDMI_AUDIO_CTS_LOW_BOUND_cts_under_flag_mask                    HDMI_P0_AUDIO_CTS_LOW_BOUND_cts_under_flag_mask
#define  HDMI_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask                     HDMI_P0_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask
#define  HDMI_AUDIO_CTS_LOW_BOUND_cts_under_flag(data)                   HDMI_P0_AUDIO_CTS_LOW_BOUND_cts_under_flag(data)
#define  HDMI_AUDIO_CTS_LOW_BOUND_cts_low_bound(data)                    HDMI_P0_AUDIO_CTS_LOW_BOUND_cts_low_bound(data)
#define  HDMI_AUDIO_CTS_LOW_BOUND_get_cts_under_flag(data)               HDMI_P0_AUDIO_CTS_LOW_BOUND_get_cts_under_flag(data)
#define  HDMI_AUDIO_CTS_LOW_BOUND_get_cts_low_bound(data)                HDMI_P0_AUDIO_CTS_LOW_BOUND_get_cts_low_bound(data)


#define  HDMI_AUDIO_N_UP_BOUND_n_over_flag_mask                          HDMI_P0_AUDIO_N_UP_BOUND_n_over_flag_mask
#define  HDMI_AUDIO_N_UP_BOUND_n_up_bound_mask                           HDMI_P0_AUDIO_N_UP_BOUND_n_up_bound_mask
#define  HDMI_AUDIO_N_UP_BOUND_n_over_flag(data)                         HDMI_P0_AUDIO_N_UP_BOUND_n_over_flag(data)
#define  HDMI_AUDIO_N_UP_BOUND_n_up_bound(data)                          HDMI_P0_AUDIO_N_UP_BOUND_n_up_bound(data)
#define  HDMI_AUDIO_N_UP_BOUND_get_n_over_flag(data)                     HDMI_P0_AUDIO_N_UP_BOUND_get_n_over_flag(data)
#define  HDMI_AUDIO_N_UP_BOUND_get_n_up_bound(data)                      HDMI_P0_AUDIO_N_UP_BOUND_get_n_up_bound(data)


#define  HDMI_AUDIO_N_LOW_BOUND_n_under_flag_mask                        HDMI_P0_AUDIO_N_LOW_BOUND_n_under_flag_mask
#define  HDMI_AUDIO_N_LOW_BOUND_n_low_bound_mask                         HDMI_P0_AUDIO_N_LOW_BOUND_n_low_bound_mask
#define  HDMI_AUDIO_N_LOW_BOUND_n_under_flag(data)                       HDMI_P0_AUDIO_N_LOW_BOUND_n_under_flag(data)
#define  HDMI_AUDIO_N_LOW_BOUND_n_low_bound(data)                        HDMI_P0_AUDIO_N_LOW_BOUND_n_low_bound(data)
#define  HDMI_AUDIO_N_LOW_BOUND_get_n_under_flag(data)                   HDMI_P0_AUDIO_N_LOW_BOUND_get_n_under_flag(data)
#define  HDMI_AUDIO_N_LOW_BOUND_get_n_low_bound(data)                    HDMI_P0_AUDIO_N_LOW_BOUND_get_n_low_bound(data)


#define  HDMI_LIGHT_SLEEP_linebuf_ls2_mask                               HDMI_P0_LIGHT_SLEEP_linebuf_ls2_mask
#define  HDMI_LIGHT_SLEEP_linebuf_ls1_mask                               HDMI_P0_LIGHT_SLEEP_linebuf_ls1_mask
#define  HDMI_LIGHT_SLEEP_linebuf_ls0_mask                               HDMI_P0_LIGHT_SLEEP_linebuf_ls0_mask
#define  HDMI_LIGHT_SLEEP_hdr_ls_1_mask                                  HDMI_P0_LIGHT_SLEEP_hdr_ls_1_mask
#define  HDMI_LIGHT_SLEEP_hdr_ls_0_mask                                  HDMI_P0_LIGHT_SLEEP_hdr_ls_0_mask
#define  HDMI_LIGHT_SLEEP_yuv420_ls3_mask                                HDMI_P0_LIGHT_SLEEP_yuv420_ls3_mask
#define  HDMI_LIGHT_SLEEP_yuv420_ls2_mask                                HDMI_P0_LIGHT_SLEEP_yuv420_ls2_mask
#define  HDMI_LIGHT_SLEEP_yuv420_ls1_mask                                HDMI_P0_LIGHT_SLEEP_yuv420_ls1_mask
#define  HDMI_LIGHT_SLEEP_yuv420_ls0_mask                                HDMI_P0_LIGHT_SLEEP_yuv420_ls0_mask
#define  HDMI_LIGHT_SLEEP_audio_ls_mask                                  HDMI_P0_LIGHT_SLEEP_audio_ls_mask
#define  HDMI_LIGHT_SLEEP_linebuf_ls2(data)                              HDMI_P0_LIGHT_SLEEP_linebuf_ls2(data)
#define  HDMI_LIGHT_SLEEP_linebuf_ls1(data)                              HDMI_P0_LIGHT_SLEEP_linebuf_ls1(data)
#define  HDMI_LIGHT_SLEEP_linebuf_ls0(data)                              HDMI_P0_LIGHT_SLEEP_linebuf_ls0(data)
#define  HDMI_LIGHT_SLEEP_hdr_ls_1(data)                                 HDMI_P0_LIGHT_SLEEP_hdr_ls_1(data)
#define  HDMI_LIGHT_SLEEP_hdr_ls_0(data)                                 HDMI_P0_LIGHT_SLEEP_hdr_ls_0(data)
#define  HDMI_LIGHT_SLEEP_yuv420_ls3(data)                               HDMI_P0_LIGHT_SLEEP_yuv420_ls3(data)
#define  HDMI_LIGHT_SLEEP_yuv420_ls2(data)                               HDMI_P0_LIGHT_SLEEP_yuv420_ls2(data)
#define  HDMI_LIGHT_SLEEP_yuv420_ls1(data)                               HDMI_P0_LIGHT_SLEEP_yuv420_ls1(data)
#define  HDMI_LIGHT_SLEEP_yuv420_ls0(data)                               HDMI_P0_LIGHT_SLEEP_yuv420_ls0(data)
#define  HDMI_LIGHT_SLEEP_audio_ls(data)                                 HDMI_P0_LIGHT_SLEEP_audio_ls(data)
#define  HDMI_LIGHT_SLEEP_get_linebuf_ls2(data)                          HDMI_P0_LIGHT_SLEEP_get_linebuf_ls2(data)
#define  HDMI_LIGHT_SLEEP_get_linebuf_ls1(data)                          HDMI_P0_LIGHT_SLEEP_get_linebuf_ls1(data)
#define  HDMI_LIGHT_SLEEP_get_linebuf_ls0(data)                          HDMI_P0_LIGHT_SLEEP_get_linebuf_ls0(data)
#define  HDMI_LIGHT_SLEEP_get_hdr_ls_1(data)                             HDMI_P0_LIGHT_SLEEP_get_hdr_ls_1(data)
#define  HDMI_LIGHT_SLEEP_get_hdr_ls_0(data)                             HDMI_P0_LIGHT_SLEEP_get_hdr_ls_0(data)
#define  HDMI_LIGHT_SLEEP_get_yuv420_ls3(data)                           HDMI_P0_LIGHT_SLEEP_get_yuv420_ls3(data)
#define  HDMI_LIGHT_SLEEP_get_yuv420_ls2(data)                           HDMI_P0_LIGHT_SLEEP_get_yuv420_ls2(data)
#define  HDMI_LIGHT_SLEEP_get_yuv420_ls1(data)                           HDMI_P0_LIGHT_SLEEP_get_yuv420_ls1(data)
#define  HDMI_LIGHT_SLEEP_get_yuv420_ls0(data)                           HDMI_P0_LIGHT_SLEEP_get_yuv420_ls0(data)
#define  HDMI_LIGHT_SLEEP_get_audio_ls(data)                             HDMI_P0_LIGHT_SLEEP_get_audio_ls(data)


#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme2_mask                       HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme2_mask
#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme1_mask                       HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme1_mask
#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme0_mask                       HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme0_mask
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmeb_1_mask                         HDMI_P0_READ_MARGIN_ENABLE_hdr_rmeb_1_mask
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmeb_0_mask                         HDMI_P0_READ_MARGIN_ENABLE_hdr_rmeb_0_mask
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmea_1_mask                         HDMI_P0_READ_MARGIN_ENABLE_hdr_rmea_1_mask
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmea_0_mask                         HDMI_P0_READ_MARGIN_ENABLE_hdr_rmea_0_mask
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme3_mask                        HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme3_mask
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme2_mask                        HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme2_mask
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme1_mask                        HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme1_mask
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme0_mask                        HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme0_mask
#define  HDMI_READ_MARGIN_ENABLE_audio_rme_mask                          HDMI_P0_READ_MARGIN_ENABLE_audio_rme_mask
#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme2(data)                      HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme2(data)
#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme1(data)                      HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme1(data)
#define  HDMI_READ_MARGIN_ENABLE_linebuf_rme0(data)                      HDMI_P0_READ_MARGIN_ENABLE_linebuf_rme0(data)
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmeb_1(data)                        HDMI_P0_READ_MARGIN_ENABLE_hdr_rmeb_1(data)
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmeb_0(data)                        HDMI_P0_READ_MARGIN_ENABLE_hdr_rmeb_0(data)
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmea_1(data)                        HDMI_P0_READ_MARGIN_ENABLE_hdr_rmea_1(data)
#define  HDMI_READ_MARGIN_ENABLE_hdr_rmea_0(data)                        HDMI_P0_READ_MARGIN_ENABLE_hdr_rmea_0(data)
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme3(data)                       HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme3(data)
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme2(data)                       HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme2(data)
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme1(data)                       HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme1(data)
#define  HDMI_READ_MARGIN_ENABLE_yuv420_rme0(data)                       HDMI_P0_READ_MARGIN_ENABLE_yuv420_rme0(data)
#define  HDMI_READ_MARGIN_ENABLE_audio_rme(data)                         HDMI_P0_READ_MARGIN_ENABLE_audio_rme(data)
#define  HDMI_READ_MARGIN_ENABLE_get_linebuf_rme2(data)                  HDMI_P0_READ_MARGIN_ENABLE_get_linebuf_rme2(data)
#define  HDMI_READ_MARGIN_ENABLE_get_linebuf_rme1(data)                  HDMI_P0_READ_MARGIN_ENABLE_get_linebuf_rme1(data)
#define  HDMI_READ_MARGIN_ENABLE_get_linebuf_rme0(data)                  HDMI_P0_READ_MARGIN_ENABLE_get_linebuf_rme0(data)
#define  HDMI_READ_MARGIN_ENABLE_get_hdr_rmeb_1(data)                    HDMI_P0_READ_MARGIN_ENABLE_get_hdr_rmeb_1(data)
#define  HDMI_READ_MARGIN_ENABLE_get_hdr_rmeb_0(data)                    HDMI_P0_READ_MARGIN_ENABLE_get_hdr_rmeb_0(data)
#define  HDMI_READ_MARGIN_ENABLE_get_hdr_rmea_1(data)                    HDMI_P0_READ_MARGIN_ENABLE_get_hdr_rmea_1(data)
#define  HDMI_READ_MARGIN_ENABLE_get_hdr_rmea_0(data)                    HDMI_P0_READ_MARGIN_ENABLE_get_hdr_rmea_0(data)
#define  HDMI_READ_MARGIN_ENABLE_get_yuv420_rme3(data)                   HDMI_P0_READ_MARGIN_ENABLE_get_yuv420_rme3(data)
#define  HDMI_READ_MARGIN_ENABLE_get_yuv420_rme2(data)                   HDMI_P0_READ_MARGIN_ENABLE_get_yuv420_rme2(data)
#define  HDMI_READ_MARGIN_ENABLE_get_yuv420_rme1(data)                   HDMI_P0_READ_MARGIN_ENABLE_get_yuv420_rme1(data)
#define  HDMI_READ_MARGIN_ENABLE_get_yuv420_rme0(data)                   HDMI_P0_READ_MARGIN_ENABLE_get_yuv420_rme0(data)
#define  HDMI_READ_MARGIN_ENABLE_get_audio_rme(data)                     HDMI_P0_READ_MARGIN_ENABLE_get_audio_rme(data)


#define  HDMI_READ_MARGIN3_linebuf_test1_2_mask                          HDMI_P0_READ_MARGIN3_linebuf_test1_2_mask
#define  HDMI_READ_MARGIN3_linebuf_test1_1_mask                          HDMI_P0_READ_MARGIN3_linebuf_test1_1_mask
#define  HDMI_READ_MARGIN3_linebuf_test1_0_mask                          HDMI_P0_READ_MARGIN3_linebuf_test1_0_mask
#define  HDMI_READ_MARGIN3_linebuf_rm2_mask                              HDMI_P0_READ_MARGIN3_linebuf_rm2_mask
#define  HDMI_READ_MARGIN3_linebuf_rm1_mask                              HDMI_P0_READ_MARGIN3_linebuf_rm1_mask
#define  HDMI_READ_MARGIN3_linebuf_rm0_mask                              HDMI_P0_READ_MARGIN3_linebuf_rm0_mask
#define  HDMI_READ_MARGIN3_linebuf_test1_2(data)                         HDMI_P0_READ_MARGIN3_linebuf_test1_2(data)
#define  HDMI_READ_MARGIN3_linebuf_test1_1(data)                         HDMI_P0_READ_MARGIN3_linebuf_test1_1(data)
#define  HDMI_READ_MARGIN3_linebuf_test1_0(data)                         HDMI_P0_READ_MARGIN3_linebuf_test1_0(data)
#define  HDMI_READ_MARGIN3_linebuf_rm2(data)                             HDMI_P0_READ_MARGIN3_linebuf_rm2(data)
#define  HDMI_READ_MARGIN3_linebuf_rm1(data)                             HDMI_P0_READ_MARGIN3_linebuf_rm1(data)
#define  HDMI_READ_MARGIN3_linebuf_rm0(data)                             HDMI_P0_READ_MARGIN3_linebuf_rm0(data)
#define  HDMI_READ_MARGIN3_get_linebuf_test1_2(data)                     HDMI_P0_READ_MARGIN3_get_linebuf_test1_2(data)
#define  HDMI_READ_MARGIN3_get_linebuf_test1_1(data)                     HDMI_P0_READ_MARGIN3_get_linebuf_test1_1(data)
#define  HDMI_READ_MARGIN3_get_linebuf_test1_0(data)                     HDMI_P0_READ_MARGIN3_get_linebuf_test1_0(data)
#define  HDMI_READ_MARGIN3_get_linebuf_rm2(data)                         HDMI_P0_READ_MARGIN3_get_linebuf_rm2(data)
#define  HDMI_READ_MARGIN3_get_linebuf_rm1(data)                         HDMI_P0_READ_MARGIN3_get_linebuf_rm1(data)
#define  HDMI_READ_MARGIN3_get_linebuf_rm0(data)                         HDMI_P0_READ_MARGIN3_get_linebuf_rm0(data)


#define  HDMI_READ_MARGIN1_hdr_test1b_1_mask                             HDMI_P0_READ_MARGIN1_hdr_test1b_1_mask
#define  HDMI_READ_MARGIN1_hdr_test1a_1_mask                             HDMI_P0_READ_MARGIN1_hdr_test1a_1_mask
#define  HDMI_READ_MARGIN1_hdr_test1b_0_mask                             HDMI_P0_READ_MARGIN1_hdr_test1b_0_mask
#define  HDMI_READ_MARGIN1_hdr_test1a_0_mask                             HDMI_P0_READ_MARGIN1_hdr_test1a_0_mask
#define  HDMI_READ_MARGIN1_yuv420_test1_3_mask                           HDMI_P0_READ_MARGIN1_yuv420_test1_3_mask
#define  HDMI_READ_MARGIN1_yuv420_test1_2_mask                           HDMI_P0_READ_MARGIN1_yuv420_test1_2_mask
#define  HDMI_READ_MARGIN1_yuv420_test1_1_mask                           HDMI_P0_READ_MARGIN1_yuv420_test1_1_mask
#define  HDMI_READ_MARGIN1_yuv420_test1_0_mask                           HDMI_P0_READ_MARGIN1_yuv420_test1_0_mask
#define  HDMI_READ_MARGIN1_yuv420_rm3_mask                               HDMI_P0_READ_MARGIN1_yuv420_rm3_mask
#define  HDMI_READ_MARGIN1_yuv420_rm2_mask                               HDMI_P0_READ_MARGIN1_yuv420_rm2_mask
#define  HDMI_READ_MARGIN1_yuv420_rm1_mask                               HDMI_P0_READ_MARGIN1_yuv420_rm1_mask
#define  HDMI_READ_MARGIN1_yuv420_rm0_mask                               HDMI_P0_READ_MARGIN1_yuv420_rm0_mask
#define  HDMI_READ_MARGIN1_hdr_test1b_1(data)                            HDMI_P0_READ_MARGIN1_hdr_test1b_1(data)
#define  HDMI_READ_MARGIN1_hdr_test1a_1(data)                            HDMI_P0_READ_MARGIN1_hdr_test1a_1(data)
#define  HDMI_READ_MARGIN1_hdr_test1b_0(data)                            HDMI_P0_READ_MARGIN1_hdr_test1b_0(data)
#define  HDMI_READ_MARGIN1_hdr_test1a_0(data)                            HDMI_P0_READ_MARGIN1_hdr_test1a_0(data)
#define  HDMI_READ_MARGIN1_yuv420_test1_3(data)                          HDMI_P0_READ_MARGIN1_yuv420_test1_3(data)
#define  HDMI_READ_MARGIN1_yuv420_test1_2(data)                          HDMI_P0_READ_MARGIN1_yuv420_test1_2(data)
#define  HDMI_READ_MARGIN1_yuv420_test1_1(data)                          HDMI_P0_READ_MARGIN1_yuv420_test1_1(data)
#define  HDMI_READ_MARGIN1_yuv420_test1_0(data)                          HDMI_P0_READ_MARGIN1_yuv420_test1_0(data)
#define  HDMI_READ_MARGIN1_yuv420_rm3(data)                              HDMI_P0_READ_MARGIN1_yuv420_rm3(data)
#define  HDMI_READ_MARGIN1_yuv420_rm2(data)                              HDMI_P0_READ_MARGIN1_yuv420_rm2(data)
#define  HDMI_READ_MARGIN1_yuv420_rm1(data)                              HDMI_P0_READ_MARGIN1_yuv420_rm1(data)
#define  HDMI_READ_MARGIN1_yuv420_rm0(data)                              HDMI_P0_READ_MARGIN1_yuv420_rm0(data)
#define  HDMI_READ_MARGIN1_get_hdr_test1b_1(data)                        HDMI_P0_READ_MARGIN1_get_hdr_test1b_1(data)
#define  HDMI_READ_MARGIN1_get_hdr_test1a_1(data)                        HDMI_P0_READ_MARGIN1_get_hdr_test1a_1(data)
#define  HDMI_READ_MARGIN1_get_hdr_test1b_0(data)                        HDMI_P0_READ_MARGIN1_get_hdr_test1b_0(data)
#define  HDMI_READ_MARGIN1_get_hdr_test1a_0(data)                        HDMI_P0_READ_MARGIN1_get_hdr_test1a_0(data)
#define  HDMI_READ_MARGIN1_get_yuv420_test1_3(data)                      HDMI_P0_READ_MARGIN1_get_yuv420_test1_3(data)
#define  HDMI_READ_MARGIN1_get_yuv420_test1_2(data)                      HDMI_P0_READ_MARGIN1_get_yuv420_test1_2(data)
#define  HDMI_READ_MARGIN1_get_yuv420_test1_1(data)                      HDMI_P0_READ_MARGIN1_get_yuv420_test1_1(data)
#define  HDMI_READ_MARGIN1_get_yuv420_test1_0(data)                      HDMI_P0_READ_MARGIN1_get_yuv420_test1_0(data)
#define  HDMI_READ_MARGIN1_get_yuv420_rm3(data)                          HDMI_P0_READ_MARGIN1_get_yuv420_rm3(data)
#define  HDMI_READ_MARGIN1_get_yuv420_rm2(data)                          HDMI_P0_READ_MARGIN1_get_yuv420_rm2(data)
#define  HDMI_READ_MARGIN1_get_yuv420_rm1(data)                          HDMI_P0_READ_MARGIN1_get_yuv420_rm1(data)
#define  HDMI_READ_MARGIN1_get_yuv420_rm0(data)                          HDMI_P0_READ_MARGIN1_get_yuv420_rm0(data)


#define  HDMI_READ_MARGIN0_hdr_rmb_1_mask                                HDMI_P0_READ_MARGIN0_hdr_rmb_1_mask
#define  HDMI_READ_MARGIN0_hdr_rmb_0_mask                                HDMI_P0_READ_MARGIN0_hdr_rmb_0_mask
#define  HDMI_READ_MARGIN0_hdr_rma_1_mask                                HDMI_P0_READ_MARGIN0_hdr_rma_1_mask
#define  HDMI_READ_MARGIN0_hdr_rma_0_mask                                HDMI_P0_READ_MARGIN0_hdr_rma_0_mask
#define  HDMI_READ_MARGIN0_audio_test1_0_mask                            HDMI_P0_READ_MARGIN0_audio_test1_0_mask
#define  HDMI_READ_MARGIN0_audio_rm_mask                                 HDMI_P0_READ_MARGIN0_audio_rm_mask
#define  HDMI_READ_MARGIN0_hdr_rmb_1(data)                               HDMI_P0_READ_MARGIN0_hdr_rmb_1(data)
#define  HDMI_READ_MARGIN0_hdr_rmb_0(data)                               HDMI_P0_READ_MARGIN0_hdr_rmb_0(data)
#define  HDMI_READ_MARGIN0_hdr_rma_1(data)                               HDMI_P0_READ_MARGIN0_hdr_rma_1(data)
#define  HDMI_READ_MARGIN0_hdr_rma_0(data)                               HDMI_P0_READ_MARGIN0_hdr_rma_0(data)
#define  HDMI_READ_MARGIN0_audio_test1_0(data)                           HDMI_P0_READ_MARGIN0_audio_test1_0(data)
#define  HDMI_READ_MARGIN0_audio_rm(data)                                HDMI_P0_READ_MARGIN0_audio_rm(data)
#define  HDMI_READ_MARGIN0_get_hdr_rmb_1(data)                           HDMI_P0_READ_MARGIN0_get_hdr_rmb_1(data)
#define  HDMI_READ_MARGIN0_get_hdr_rmb_0(data)                           HDMI_P0_READ_MARGIN0_get_hdr_rmb_0(data)
#define  HDMI_READ_MARGIN0_get_hdr_rma_1(data)                           HDMI_P0_READ_MARGIN0_get_hdr_rma_1(data)
#define  HDMI_READ_MARGIN0_get_hdr_rma_0(data)                           HDMI_P0_READ_MARGIN0_get_hdr_rma_0(data)
#define  HDMI_READ_MARGIN0_get_audio_test1_0(data)                       HDMI_P0_READ_MARGIN0_get_audio_test1_0(data)
#define  HDMI_READ_MARGIN0_get_audio_rm(data)                            HDMI_P0_READ_MARGIN0_get_audio_rm(data)


#define  HDMI_BIST_MODE_linebuf_bist_en_mask                             HDMI_P0_BIST_MODE_linebuf_bist_en_mask
#define  HDMI_BIST_MODE_hdr_bist_en_mask                                 HDMI_P0_BIST_MODE_hdr_bist_en_mask
#define  HDMI_BIST_MODE_yuv420_bist_en_mask                              HDMI_P0_BIST_MODE_yuv420_bist_en_mask
#define  HDMI_BIST_MODE_audio_bist_en_mask                               HDMI_P0_BIST_MODE_audio_bist_en_mask
#define  HDMI_BIST_MODE_linebuf_bist_en(data)                            HDMI_P0_BIST_MODE_linebuf_bist_en(data)
#define  HDMI_BIST_MODE_hdr_bist_en(data)                                HDMI_P0_BIST_MODE_hdr_bist_en(data)
#define  HDMI_BIST_MODE_yuv420_bist_en(data)                             HDMI_P0_BIST_MODE_yuv420_bist_en(data)
#define  HDMI_BIST_MODE_audio_bist_en(data)                              HDMI_P0_BIST_MODE_audio_bist_en(data)
#define  HDMI_BIST_MODE_get_linebuf_bist_en(data)                        HDMI_P0_BIST_MODE_get_linebuf_bist_en(data)
#define  HDMI_BIST_MODE_get_hdr_bist_en(data)                            HDMI_P0_BIST_MODE_get_hdr_bist_en(data)
#define  HDMI_BIST_MODE_get_yuv420_bist_en(data)                         HDMI_P0_BIST_MODE_get_yuv420_bist_en(data)
#define  HDMI_BIST_MODE_get_audio_bist_en(data)                          HDMI_P0_BIST_MODE_get_audio_bist_en(data)


#define  HDMI_BIST_DONE_linebuf_bist_done_mask                           HDMI_P0_BIST_DONE_linebuf_bist_done_mask
#define  HDMI_BIST_DONE_hdr_bist_done_mask                               HDMI_P0_BIST_DONE_hdr_bist_done_mask
#define  HDMI_BIST_DONE_yuv420_bist_done_mask                            HDMI_P0_BIST_DONE_yuv420_bist_done_mask
#define  HDMI_BIST_DONE_audio_bist_done_mask                             HDMI_P0_BIST_DONE_audio_bist_done_mask
#define  HDMI_BIST_DONE_linebuf_bist_done(data)                          HDMI_P0_BIST_DONE_linebuf_bist_done(data)
#define  HDMI_BIST_DONE_hdr_bist_done(data)                              HDMI_P0_BIST_DONE_hdr_bist_done(data)
#define  HDMI_BIST_DONE_yuv420_bist_done(data)                           HDMI_P0_BIST_DONE_yuv420_bist_done(data)
#define  HDMI_BIST_DONE_audio_bist_done(data)                            HDMI_P0_BIST_DONE_audio_bist_done(data)
#define  HDMI_BIST_DONE_get_linebuf_bist_done(data)                      HDMI_P0_BIST_DONE_get_linebuf_bist_done(data)
#define  HDMI_BIST_DONE_get_hdr_bist_done(data)                          HDMI_P0_BIST_DONE_get_hdr_bist_done(data)
#define  HDMI_BIST_DONE_get_yuv420_bist_done(data)                       HDMI_P0_BIST_DONE_get_yuv420_bist_done(data)
#define  HDMI_BIST_DONE_get_audio_bist_done(data)                        HDMI_P0_BIST_DONE_get_audio_bist_done(data)


#define  HDMI_BIST_FAIL_linebuf_bist_fail_mask                           HDMI_P0_BIST_FAIL_linebuf_bist_fail_mask
#define  HDMI_BIST_FAIL_linebuf_bist_fail0_mask                          HDMI_P0_BIST_FAIL_linebuf_bist_fail0_mask
#define  HDMI_BIST_FAIL_linebuf_bist_fail1_mask                          HDMI_P0_BIST_FAIL_linebuf_bist_fail1_mask
#define  HDMI_BIST_FAIL_linebuf_bist_fail2_mask                          HDMI_P0_BIST_FAIL_linebuf_bist_fail2_mask
#define  HDMI_BIST_FAIL_hdr_bist_fail_mask                               HDMI_P0_BIST_FAIL_hdr_bist_fail_mask
#define  HDMI_BIST_FAIL_hdr_bist_fail0_mask                              HDMI_P0_BIST_FAIL_hdr_bist_fail0_mask
#define  HDMI_BIST_FAIL_hdr_bist_fail1_mask                              HDMI_P0_BIST_FAIL_hdr_bist_fail1_mask
#define  HDMI_BIST_FAIL_yuv420_bist_fail_mask                            HDMI_P0_BIST_FAIL_yuv420_bist_fail_mask
#define  HDMI_BIST_FAIL_yuv420_bist_fail0_mask                           HDMI_P0_BIST_FAIL_yuv420_bist_fail0_mask
#define  HDMI_BIST_FAIL_yuv420_bist_fail1_mask                           HDMI_P0_BIST_FAIL_yuv420_bist_fail1_mask
#define  HDMI_BIST_FAIL_yuv420_bist_fail2_mask                           HDMI_P0_BIST_FAIL_yuv420_bist_fail2_mask
#define  HDMI_BIST_FAIL_yuv420_bist_fail3_mask                           HDMI_P0_BIST_FAIL_yuv420_bist_fail3_mask
#define  HDMI_BIST_FAIL_audio_bist_fail_mask                             HDMI_P0_BIST_FAIL_audio_bist_fail_mask
#define  HDMI_BIST_FAIL_linebuf_bist_fail(data)                          HDMI_P0_BIST_FAIL_linebuf_bist_fail(data)
#define  HDMI_BIST_FAIL_linebuf_bist_fail0(data)                         HDMI_P0_BIST_FAIL_linebuf_bist_fail0(data)
#define  HDMI_BIST_FAIL_linebuf_bist_fail1(data)                         HDMI_P0_BIST_FAIL_linebuf_bist_fail1(data)
#define  HDMI_BIST_FAIL_linebuf_bist_fail2(data)                         HDMI_P0_BIST_FAIL_linebuf_bist_fail2(data)
#define  HDMI_BIST_FAIL_hdr_bist_fail(data)                              HDMI_P0_BIST_FAIL_hdr_bist_fail(data)
#define  HDMI_BIST_FAIL_hdr_bist_fail0(data)                             HDMI_P0_BIST_FAIL_hdr_bist_fail0(data)
#define  HDMI_BIST_FAIL_hdr_bist_fail1(data)                             HDMI_P0_BIST_FAIL_hdr_bist_fail1(data)
#define  HDMI_BIST_FAIL_yuv420_bist_fail(data)                           HDMI_P0_BIST_FAIL_yuv420_bist_fail(data)
#define  HDMI_BIST_FAIL_yuv420_bist_fail0(data)                          HDMI_P0_BIST_FAIL_yuv420_bist_fail0(data)
#define  HDMI_BIST_FAIL_yuv420_bist_fail1(data)                          HDMI_P0_BIST_FAIL_yuv420_bist_fail1(data)
#define  HDMI_BIST_FAIL_yuv420_bist_fail2(data)                          HDMI_P0_BIST_FAIL_yuv420_bist_fail2(data)
#define  HDMI_BIST_FAIL_yuv420_bist_fail3(data)                          HDMI_P0_BIST_FAIL_yuv420_bist_fail3(data)
#define  HDMI_BIST_FAIL_audio_bist_fail(data)                            HDMI_P0_BIST_FAIL_audio_bist_fail(data)
#define  HDMI_BIST_FAIL_get_linebuf_bist_fail(data)                      HDMI_P0_BIST_FAIL_get_linebuf_bist_fail(data)
#define  HDMI_BIST_FAIL_get_linebuf_bist_fail0(data)                     HDMI_P0_BIST_FAIL_get_linebuf_bist_fail0(data)
#define  HDMI_BIST_FAIL_get_linebuf_bist_fail1(data)                     HDMI_P0_BIST_FAIL_get_linebuf_bist_fail1(data)
#define  HDMI_BIST_FAIL_get_linebuf_bist_fail2(data)                     HDMI_P0_BIST_FAIL_get_linebuf_bist_fail2(data)
#define  HDMI_BIST_FAIL_get_hdr_bist_fail(data)                          HDMI_P0_BIST_FAIL_get_hdr_bist_fail(data)
#define  HDMI_BIST_FAIL_get_hdr_bist_fail0(data)                         HDMI_P0_BIST_FAIL_get_hdr_bist_fail0(data)
#define  HDMI_BIST_FAIL_get_hdr_bist_fail1(data)                         HDMI_P0_BIST_FAIL_get_hdr_bist_fail1(data)
#define  HDMI_BIST_FAIL_get_yuv420_bist_fail(data)                       HDMI_P0_BIST_FAIL_get_yuv420_bist_fail(data)
#define  HDMI_BIST_FAIL_get_yuv420_bist_fail0(data)                      HDMI_P0_BIST_FAIL_get_yuv420_bist_fail0(data)
#define  HDMI_BIST_FAIL_get_yuv420_bist_fail1(data)                      HDMI_P0_BIST_FAIL_get_yuv420_bist_fail1(data)
#define  HDMI_BIST_FAIL_get_yuv420_bist_fail2(data)                      HDMI_P0_BIST_FAIL_get_yuv420_bist_fail2(data)
#define  HDMI_BIST_FAIL_get_yuv420_bist_fail3(data)                      HDMI_P0_BIST_FAIL_get_yuv420_bist_fail3(data)
#define  HDMI_BIST_FAIL_get_audio_bist_fail(data)                        HDMI_P0_BIST_FAIL_get_audio_bist_fail(data)


#define  HDMI_DRF_MODE_linebuf_drf_mode_mask                             HDMI_P0_DRF_MODE_linebuf_drf_mode_mask
#define  HDMI_DRF_MODE_hdr_drf_mode_mask                                 HDMI_P0_DRF_MODE_hdr_drf_mode_mask
#define  HDMI_DRF_MODE_yuv420_drf_mode_mask                              HDMI_P0_DRF_MODE_yuv420_drf_mode_mask
#define  HDMI_DRF_MODE_audio_drf_mode_mask                               HDMI_P0_DRF_MODE_audio_drf_mode_mask
#define  HDMI_DRF_MODE_linebuf_drf_mode(data)                            HDMI_P0_DRF_MODE_linebuf_drf_mode(data)
#define  HDMI_DRF_MODE_hdr_drf_mode(data)                                HDMI_P0_DRF_MODE_hdr_drf_mode(data)
#define  HDMI_DRF_MODE_yuv420_drf_mode(data)                             HDMI_P0_DRF_MODE_yuv420_drf_mode(data)
#define  HDMI_DRF_MODE_audio_drf_mode(data)                              HDMI_P0_DRF_MODE_audio_drf_mode(data)
#define  HDMI_DRF_MODE_get_linebuf_drf_mode(data)                        HDMI_P0_DRF_MODE_get_linebuf_drf_mode(data)
#define  HDMI_DRF_MODE_get_hdr_drf_mode(data)                            HDMI_P0_DRF_MODE_get_hdr_drf_mode(data)
#define  HDMI_DRF_MODE_get_yuv420_drf_mode(data)                         HDMI_P0_DRF_MODE_get_yuv420_drf_mode(data)
#define  HDMI_DRF_MODE_get_audio_drf_mode(data)                          HDMI_P0_DRF_MODE_get_audio_drf_mode(data)


#define  HDMI_DRF_RESUME_linebuf_drf_resume_mask                         HDMI_P0_DRF_RESUME_linebuf_drf_resume_mask
#define  HDMI_DRF_RESUME_hdr_drf_resume_mask                             HDMI_P0_DRF_RESUME_hdr_drf_resume_mask
#define  HDMI_DRF_RESUME_yuv420_drf_resume_mask                          HDMI_P0_DRF_RESUME_yuv420_drf_resume_mask
#define  HDMI_DRF_RESUME_audio_drf_resume_mask                           HDMI_P0_DRF_RESUME_audio_drf_resume_mask
#define  HDMI_DRF_RESUME_linebuf_drf_resume(data)                        HDMI_P0_DRF_RESUME_linebuf_drf_resume(data)
#define  HDMI_DRF_RESUME_hdr_drf_resume(data)                            HDMI_P0_DRF_RESUME_hdr_drf_resume(data)
#define  HDMI_DRF_RESUME_yuv420_drf_resume(data)                         HDMI_P0_DRF_RESUME_yuv420_drf_resume(data)
#define  HDMI_DRF_RESUME_audio_drf_resume(data)                          HDMI_P0_DRF_RESUME_audio_drf_resume(data)
#define  HDMI_DRF_RESUME_get_linebuf_drf_resume(data)                    HDMI_P0_DRF_RESUME_get_linebuf_drf_resume(data)
#define  HDMI_DRF_RESUME_get_hdr_drf_resume(data)                        HDMI_P0_DRF_RESUME_get_hdr_drf_resume(data)
#define  HDMI_DRF_RESUME_get_yuv420_drf_resume(data)                     HDMI_P0_DRF_RESUME_get_yuv420_drf_resume(data)
#define  HDMI_DRF_RESUME_get_audio_drf_resume(data)                      HDMI_P0_DRF_RESUME_get_audio_drf_resume(data)


#define  HDMI_DRF_DONE_linebuf_drf_done_mask                             HDMI_P0_DRF_DONE_linebuf_drf_done_mask
#define  HDMI_DRF_DONE_hdr_drf_done_mask                                 HDMI_P0_DRF_DONE_hdr_drf_done_mask
#define  HDMI_DRF_DONE_yuv420_drf_done_mask                              HDMI_P0_DRF_DONE_yuv420_drf_done_mask
#define  HDMI_DRF_DONE_audio_drf_done_mask                               HDMI_P0_DRF_DONE_audio_drf_done_mask
#define  HDMI_DRF_DONE_linebuf_drf_done(data)                            HDMI_P0_DRF_DONE_linebuf_drf_done(data)
#define  HDMI_DRF_DONE_hdr_drf_done(data)                                HDMI_P0_DRF_DONE_hdr_drf_done(data)
#define  HDMI_DRF_DONE_yuv420_drf_done(data)                             HDMI_P0_DRF_DONE_yuv420_drf_done(data)
#define  HDMI_DRF_DONE_audio_drf_done(data)                              HDMI_P0_DRF_DONE_audio_drf_done(data)
#define  HDMI_DRF_DONE_get_linebuf_drf_done(data)                        HDMI_P0_DRF_DONE_get_linebuf_drf_done(data)
#define  HDMI_DRF_DONE_get_hdr_drf_done(data)                            HDMI_P0_DRF_DONE_get_hdr_drf_done(data)
#define  HDMI_DRF_DONE_get_yuv420_drf_done(data)                         HDMI_P0_DRF_DONE_get_yuv420_drf_done(data)
#define  HDMI_DRF_DONE_get_audio_drf_done(data)                          HDMI_P0_DRF_DONE_get_audio_drf_done(data)


#define  HDMI_DRF_PAUSE_linebuf_drf_pause_mask                           HDMI_P0_DRF_PAUSE_linebuf_drf_pause_mask
#define  HDMI_DRF_PAUSE_hdr_drf_pause_mask                               HDMI_P0_DRF_PAUSE_hdr_drf_pause_mask
#define  HDMI_DRF_PAUSE_yuv420_drf_pause_mask                            HDMI_P0_DRF_PAUSE_yuv420_drf_pause_mask
#define  HDMI_DRF_PAUSE_audio_drf_pause_mask                             HDMI_P0_DRF_PAUSE_audio_drf_pause_mask
#define  HDMI_DRF_PAUSE_linebuf_drf_pause(data)                          HDMI_P0_DRF_PAUSE_linebuf_drf_pause(data)
#define  HDMI_DRF_PAUSE_hdr_drf_pause(data)                              HDMI_P0_DRF_PAUSE_hdr_drf_pause(data)
#define  HDMI_DRF_PAUSE_yuv420_drf_pause(data)                           HDMI_P0_DRF_PAUSE_yuv420_drf_pause(data)
#define  HDMI_DRF_PAUSE_audio_drf_pause(data)                            HDMI_P0_DRF_PAUSE_audio_drf_pause(data)
#define  HDMI_DRF_PAUSE_get_linebuf_drf_pause(data)                      HDMI_P0_DRF_PAUSE_get_linebuf_drf_pause(data)
#define  HDMI_DRF_PAUSE_get_hdr_drf_pause(data)                          HDMI_P0_DRF_PAUSE_get_hdr_drf_pause(data)
#define  HDMI_DRF_PAUSE_get_yuv420_drf_pause(data)                       HDMI_P0_DRF_PAUSE_get_yuv420_drf_pause(data)
#define  HDMI_DRF_PAUSE_get_audio_drf_pause(data)                        HDMI_P0_DRF_PAUSE_get_audio_drf_pause(data)


#define  HDMI_DRF_FAIL_linebuf_drf_fail_mask                             HDMI_P0_DRF_FAIL_linebuf_drf_fail_mask
#define  HDMI_DRF_FAIL_linebuf_drf_fail0_mask                            HDMI_P0_DRF_FAIL_linebuf_drf_fail0_mask
#define  HDMI_DRF_FAIL_linebuf_drf_fail1_mask                            HDMI_P0_DRF_FAIL_linebuf_drf_fail1_mask
#define  HDMI_DRF_FAIL_linebuf_drf_fail2_mask                            HDMI_P0_DRF_FAIL_linebuf_drf_fail2_mask
#define  HDMI_DRF_FAIL_hdr_drf_fail_mask                                 HDMI_P0_DRF_FAIL_hdr_drf_fail_mask
#define  HDMI_DRF_FAIL_hdr_drf_fail0_mask                                HDMI_P0_DRF_FAIL_hdr_drf_fail0_mask
#define  HDMI_DRF_FAIL_hdr_drf_fail1_mask                                HDMI_P0_DRF_FAIL_hdr_drf_fail1_mask
#define  HDMI_DRF_FAIL_yuv420_drf_fail_mask                              HDMI_P0_DRF_FAIL_yuv420_drf_fail_mask
#define  HDMI_DRF_FAIL_yuv420_drf_fail0_mask                             HDMI_P0_DRF_FAIL_yuv420_drf_fail0_mask
#define  HDMI_DRF_FAIL_yuv420_drf_fail1_mask                             HDMI_P0_DRF_FAIL_yuv420_drf_fail1_mask
#define  HDMI_DRF_FAIL_yuv420_drf_fail2_mask                             HDMI_P0_DRF_FAIL_yuv420_drf_fail2_mask
#define  HDMI_DRF_FAIL_yuv420_drf_fail3_mask                             HDMI_P0_DRF_FAIL_yuv420_drf_fail3_mask
#define  HDMI_DRF_FAIL_audio_drf_fail_mask                               HDMI_P0_DRF_FAIL_audio_drf_fail_mask
#define  HDMI_DRF_FAIL_linebuf_drf_fail(data)                            HDMI_P0_DRF_FAIL_linebuf_drf_fail(data)
#define  HDMI_DRF_FAIL_linebuf_drf_fail0(data)                           HDMI_P0_DRF_FAIL_linebuf_drf_fail0(data)
#define  HDMI_DRF_FAIL_linebuf_drf_fail1(data)                           HDMI_P0_DRF_FAIL_linebuf_drf_fail1(data)
#define  HDMI_DRF_FAIL_linebuf_drf_fail2(data)                           HDMI_P0_DRF_FAIL_linebuf_drf_fail2(data)
#define  HDMI_DRF_FAIL_hdr_drf_fail(data)                                HDMI_P0_DRF_FAIL_hdr_drf_fail(data)
#define  HDMI_DRF_FAIL_hdr_drf_fail0(data)                               HDMI_P0_DRF_FAIL_hdr_drf_fail0(data)
#define  HDMI_DRF_FAIL_hdr_drf_fail1(data)                               HDMI_P0_DRF_FAIL_hdr_drf_fail1(data)
#define  HDMI_DRF_FAIL_yuv420_drf_fail(data)                             HDMI_P0_DRF_FAIL_yuv420_drf_fail(data)
#define  HDMI_DRF_FAIL_yuv420_drf_fail0(data)                            HDMI_P0_DRF_FAIL_yuv420_drf_fail0(data)
#define  HDMI_DRF_FAIL_yuv420_drf_fail1(data)                            HDMI_P0_DRF_FAIL_yuv420_drf_fail1(data)
#define  HDMI_DRF_FAIL_yuv420_drf_fail2(data)                            HDMI_P0_DRF_FAIL_yuv420_drf_fail2(data)
#define  HDMI_DRF_FAIL_yuv420_drf_fail3(data)                            HDMI_P0_DRF_FAIL_yuv420_drf_fail3(data)
#define  HDMI_DRF_FAIL_audio_drf_fail(data)                              HDMI_P0_DRF_FAIL_audio_drf_fail(data)
#define  HDMI_DRF_FAIL_get_linebuf_drf_fail(data)                        HDMI_P0_DRF_FAIL_get_linebuf_drf_fail(data)
#define  HDMI_DRF_FAIL_get_linebuf_drf_fail0(data)                       HDMI_P0_DRF_FAIL_get_linebuf_drf_fail0(data)
#define  HDMI_DRF_FAIL_get_linebuf_drf_fail1(data)                       HDMI_P0_DRF_FAIL_get_linebuf_drf_fail1(data)
#define  HDMI_DRF_FAIL_get_linebuf_drf_fail2(data)                       HDMI_P0_DRF_FAIL_get_linebuf_drf_fail2(data)
#define  HDMI_DRF_FAIL_get_hdr_drf_fail(data)                            HDMI_P0_DRF_FAIL_get_hdr_drf_fail(data)
#define  HDMI_DRF_FAIL_get_hdr_drf_fail0(data)                           HDMI_P0_DRF_FAIL_get_hdr_drf_fail0(data)
#define  HDMI_DRF_FAIL_get_hdr_drf_fail1(data)                           HDMI_P0_DRF_FAIL_get_hdr_drf_fail1(data)
#define  HDMI_DRF_FAIL_get_yuv420_drf_fail(data)                         HDMI_P0_DRF_FAIL_get_yuv420_drf_fail(data)
#define  HDMI_DRF_FAIL_get_yuv420_drf_fail0(data)                        HDMI_P0_DRF_FAIL_get_yuv420_drf_fail0(data)
#define  HDMI_DRF_FAIL_get_yuv420_drf_fail1(data)                        HDMI_P0_DRF_FAIL_get_yuv420_drf_fail1(data)
#define  HDMI_DRF_FAIL_get_yuv420_drf_fail2(data)                        HDMI_P0_DRF_FAIL_get_yuv420_drf_fail2(data)
#define  HDMI_DRF_FAIL_get_yuv420_drf_fail3(data)                        HDMI_P0_DRF_FAIL_get_yuv420_drf_fail3(data)
#define  HDMI_DRF_FAIL_get_audio_drf_fail(data)                          HDMI_P0_DRF_FAIL_get_audio_drf_fail(data)
#define  HDMI_PHY_FIFO_CR0_dummy_6_mask                                  HDMI_P0_PHY_FIFO_CR0_dummy_6_mask
#define  HDMI_PHY_FIFO_CR0_port0_r_flush_mask                            HDMI_P0_PHY_FIFO_CR0_port0_r_flush_mask
#define  HDMI_PHY_FIFO_CR0_port0_r_afifo_en_mask                         HDMI_P0_PHY_FIFO_CR0_port0_r_afifo_en_mask
#define  HDMI_PHY_FIFO_CR0_port0_g_flush_mask                            HDMI_P0_PHY_FIFO_CR0_port0_g_flush_mask
#define  HDMI_PHY_FIFO_CR0_port0_g_afifo_en_mask                         HDMI_P0_PHY_FIFO_CR0_port0_g_afifo_en_mask
#define  HDMI_PHY_FIFO_CR0_port0_b_flush_mask                            HDMI_P0_PHY_FIFO_CR0_port0_b_flush_mask
#define  HDMI_PHY_FIFO_CR0_port0_b_afifo_en_mask                         HDMI_P0_PHY_FIFO_CR0_port0_b_afifo_en_mask
#define  HDMI_PHY_FIFO_CR0_dummy_6(data)                                 HDMI_P0_PHY_FIFO_CR0_dummy_6(data)
#define  HDMI_PHY_FIFO_CR0_port0_r_flush(data)                           HDMI_P0_PHY_FIFO_CR0_port0_r_flush(data)
#define  HDMI_PHY_FIFO_CR0_port0_r_afifo_en(data)                        HDMI_P0_PHY_FIFO_CR0_port0_r_afifo_en(data)
#define  HDMI_PHY_FIFO_CR0_port0_g_flush(data)                           HDMI_P0_PHY_FIFO_CR0_port0_g_flush(data)
#define  HDMI_PHY_FIFO_CR0_port0_g_afifo_en(data)                        HDMI_P0_PHY_FIFO_CR0_port0_g_afifo_en(data)
#define  HDMI_PHY_FIFO_CR0_port0_b_flush(data)                           HDMI_P0_PHY_FIFO_CR0_port0_b_flush(data)
#define  HDMI_PHY_FIFO_CR0_port0_b_afifo_en(data)                        HDMI_P0_PHY_FIFO_CR0_port0_b_afifo_en(data)
#define  HDMI_PHY_FIFO_CR0_get_dummy_6(data)                             HDMI_P0_PHY_FIFO_CR0_get_dummy_6(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_r_flush(data)                       HDMI_P0_PHY_FIFO_CR0_get_port0_r_flush(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_r_afifo_en(data)                    HDMI_P0_PHY_FIFO_CR0_get_port0_r_afifo_en(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_g_flush(data)                       HDMI_P0_PHY_FIFO_CR0_get_port0_g_flush(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_g_afifo_en(data)                    HDMI_P0_PHY_FIFO_CR0_get_port0_g_afifo_en(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_b_flush(data)                       HDMI_P0_PHY_FIFO_CR0_get_port0_b_flush(data)
#define  HDMI_PHY_FIFO_CR0_get_port0_b_afifo_en(data)                    HDMI_P0_PHY_FIFO_CR0_get_port0_b_afifo_en(data)
#define  HDMI_PHY_FIFO_CR1_port0_rclk_inv_mask                           HDMI_P0_PHY_FIFO_CR1_port0_rclk_inv_mask
#define  HDMI_PHY_FIFO_CR1_port0_gclk_inv_mask                           HDMI_P0_PHY_FIFO_CR1_port0_gclk_inv_mask
#define  HDMI_PHY_FIFO_CR1_port0_bclk_inv_mask                           HDMI_P0_PHY_FIFO_CR1_port0_bclk_inv_mask
#define  HDMI_PHY_FIFO_CR1_port0_rclk_inv(data)                          HDMI_P0_PHY_FIFO_CR1_port0_rclk_inv(data)
#define  HDMI_PHY_FIFO_CR1_port0_gclk_inv(data)                          HDMI_P0_PHY_FIFO_CR1_port0_gclk_inv(data)
#define  HDMI_PHY_FIFO_CR1_port0_bclk_inv(data)                          HDMI_P0_PHY_FIFO_CR1_port0_bclk_inv(data)
#define  HDMI_PHY_FIFO_CR1_get_port0_rclk_inv(data)                      HDMI_P0_PHY_FIFO_CR1_get_port0_rclk_inv(data)
#define  HDMI_PHY_FIFO_CR1_get_port0_gclk_inv(data)                      HDMI_P0_PHY_FIFO_CR1_get_port0_gclk_inv(data)
#define  HDMI_PHY_FIFO_CR1_get_port0_bclk_inv(data)                      HDMI_P0_PHY_FIFO_CR1_get_port0_bclk_inv(data)

#define  HDMI_PHY_FIFO_SR0_port0_r_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_r_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_r_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_r_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_r_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_r_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_r_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_r_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_r_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR0_port0_r_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_g_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_g_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_g_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_g_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_g_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_g_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_g_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_g_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_g_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR0_port0_g_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_b_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_b_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_b_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR0_port0_b_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_b_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_b_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_b_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR0_port0_b_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_b_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR0_port0_b_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR0_port0_r_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_r_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_r_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_r_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_r_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR0_port0_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_g_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_g_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_g_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_g_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_g_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR0_port0_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_b_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_b_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR0_port0_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_b_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_b_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR0_port0_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_port0_b_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR0_port0_b_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_r_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_r_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_r_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_r_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_r_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR0_get_port0_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_g_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_g_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_g_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_g_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_g_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR0_get_port0_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_b_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_b_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR0_get_port0_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_b_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_b_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR0_get_port0_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR0_get_port0_b_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR0_get_port0_b_rflush_flag(data)


#define  HDMI_PHY_FIFO_SR1_port3_r_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_r_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_r_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_r_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_r_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_r_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_r_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_r_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_r_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port3_r_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_g_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_g_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_g_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_g_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_g_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_g_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_g_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_g_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_g_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port3_g_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_b_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_b_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_b_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port3_b_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_b_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_b_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_b_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port3_b_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_b_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port3_b_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_r_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_r_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_r_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_r_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_r_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_r_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_r_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_r_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_r_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port2_r_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_g_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_g_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_g_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_g_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_g_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_g_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_g_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_g_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_g_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port2_g_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_b_wrclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_b_wrclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_b_rwclk_det_timeout_flag_mask           HDMI_P0_PHY_FIFO_SR1_port2_b_rwclk_det_timeout_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_b_wovflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_b_wovflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_b_rudflow_flag_mask                     HDMI_P0_PHY_FIFO_SR1_port2_b_rudflow_flag_mask
#define  HDMI_PHY_FIFO_SR1_port2_b_rflush_flag_mask                      HDMI_P0_PHY_FIFO_SR1_port2_b_rflush_flag_mask
#define  HDMI_PHY_FIFO_SR1_port3_r_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_r_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_r_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_r_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_r_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port3_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_g_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_g_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_g_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_g_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_g_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port3_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_b_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_b_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port3_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_b_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_b_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port3_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port3_b_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port3_b_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_r_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_r_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_r_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_r_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_r_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port2_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_g_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_g_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_g_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_g_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_g_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port2_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_b_wrclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_b_rwclk_det_timeout_flag(data)          HDMI_P0_PHY_FIFO_SR1_port2_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_b_wovflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_b_rudflow_flag(data)                    HDMI_P0_PHY_FIFO_SR1_port2_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_port2_b_rflush_flag(data)                     HDMI_P0_PHY_FIFO_SR1_port2_b_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_r_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_r_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_r_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_r_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_r_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port3_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_g_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_g_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_g_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_g_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_g_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port3_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_b_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_b_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port3_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_b_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_b_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port3_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port3_b_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port3_b_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_r_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_r_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_r_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_r_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_r_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_r_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_r_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_r_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_r_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port2_r_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_g_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_g_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_g_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_g_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_g_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_g_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_g_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_g_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_g_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port2_g_rflush_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_b_wrclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_b_wrclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_b_rwclk_det_timeout_flag(data)      HDMI_P0_PHY_FIFO_SR1_get_port2_b_rwclk_det_timeout_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_b_wovflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_b_wovflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_b_rudflow_flag(data)                HDMI_P0_PHY_FIFO_SR1_get_port2_b_rudflow_flag(data)
#define  HDMI_PHY_FIFO_SR1_get_port2_b_rflush_flag(data)                 HDMI_P0_PHY_FIFO_SR1_get_port2_b_rflush_flag(data)


#define  HDMI_HDMI_CTS_FIFO_CTL_dbg_clk_freq_sel_mask                    HDMI_P0_HDMI_CTS_FIFO_CTL_dbg_clk_freq_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_mhl_test_sel_mask                        HDMI_P0_HDMI_CTS_FIFO_CTL_mhl_test_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_toggle_hdmi_mask              HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_toggle_hdmi_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_offms_clock_sel_mask                     HDMI_P0_HDMI_CTS_FIFO_CTL_offms_clock_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_2x_out_sel_mask                     HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_2x_out_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel_mask                       HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_test_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_mac_out_sel_mask                         HDMI_P0_HDMI_CTS_FIFO_CTL_mac_out_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_out_sel_mask                        HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_out_sel_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_force_ctsfifo_rstn_hdmi_mask             HDMI_P0_HDMI_CTS_FIFO_CTL_force_ctsfifo_rstn_hdmi_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_vsrst_hdmi_mask               HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_vsrst_hdmi_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_bypass_hdmi_mask              HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_bypass_hdmi_mask
#define  HDMI_HDMI_CTS_FIFO_CTL_dbg_clk_freq_sel(data)                   HDMI_P0_HDMI_CTS_FIFO_CTL_dbg_clk_freq_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_mhl_test_sel(data)                       HDMI_P0_HDMI_CTS_FIFO_CTL_mhl_test_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_toggle_hdmi(data)             HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_toggle_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_offms_clock_sel(data)                    HDMI_P0_HDMI_CTS_FIFO_CTL_offms_clock_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_2x_out_sel(data)                    HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_2x_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel(data)                      HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_test_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_mac_out_sel(data)                        HDMI_P0_HDMI_CTS_FIFO_CTL_mac_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_hdmi_out_sel(data)                       HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_force_ctsfifo_rstn_hdmi(data)            HDMI_P0_HDMI_CTS_FIFO_CTL_force_ctsfifo_rstn_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_vsrst_hdmi(data)              HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_vsrst_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_en_ctsfifo_bypass_hdmi(data)             HDMI_P0_HDMI_CTS_FIFO_CTL_en_ctsfifo_bypass_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_dbg_clk_freq_sel(data)               HDMI_P0_HDMI_CTS_FIFO_CTL_get_dbg_clk_freq_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_mhl_test_sel(data)                   HDMI_P0_HDMI_CTS_FIFO_CTL_get_mhl_test_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_toggle_hdmi(data)         HDMI_P0_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_toggle_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_offms_clock_sel(data)                HDMI_P0_HDMI_CTS_FIFO_CTL_get_offms_clock_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_hdmi_2x_out_sel(data)                HDMI_P0_HDMI_CTS_FIFO_CTL_get_hdmi_2x_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_mac_out_sel(data)                    HDMI_P0_HDMI_CTS_FIFO_CTL_get_mac_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_hdmi_out_sel(data)                   HDMI_P0_HDMI_CTS_FIFO_CTL_get_hdmi_out_sel(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_force_ctsfifo_rstn_hdmi(data)        HDMI_P0_HDMI_CTS_FIFO_CTL_get_force_ctsfifo_rstn_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_vsrst_hdmi(data)          HDMI_P0_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_vsrst_hdmi(data)
#define  HDMI_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_bypass_hdmi(data)         HDMI_P0_HDMI_CTS_FIFO_CTL_get_en_ctsfifo_bypass_hdmi(data)


#define  HDMI_CH_CR_br_afifo_align_mode_mask                             HDMI_P0_CH_CR_br_afifo_align_mode_mask
#define  HDMI_CH_CR_r_ch_afifo_en_mask                                   HDMI_P0_CH_CR_r_ch_afifo_en_mask
#define  HDMI_CH_CR_g_ch_afifo_en_mask                                   HDMI_P0_CH_CR_g_ch_afifo_en_mask
#define  HDMI_CH_CR_b_ch_afifo_en_mask                                   HDMI_P0_CH_CR_b_ch_afifo_en_mask
#define  HDMI_CH_CR_ch_sync_sel_mask                                     HDMI_P0_CH_CR_ch_sync_sel_mask
#define  HDMI_CH_CR_dummy_19_17_mask                                     HDMI_P0_CH_CR_dummy_19_17_mask
#define  HDMI_CH_CR_rden_thr_mask                                        HDMI_P0_CH_CR_rden_thr_mask
#define  HDMI_CH_CR_dummy_11_9_mask                                      HDMI_P0_CH_CR_dummy_11_9_mask
#define  HDMI_CH_CR_udwater_thr_mask                                     HDMI_P0_CH_CR_udwater_thr_mask
#define  HDMI_CH_CR_dummy_3_mask                                         HDMI_P0_CH_CR_dummy_3_mask
#define  HDMI_CH_CR_flush_mask                                           HDMI_P0_CH_CR_flush_mask
#define  HDMI_CH_CR_ch_afifo_airq_en_mask                                HDMI_P0_CH_CR_ch_afifo_airq_en_mask
#define  HDMI_CH_CR_ch_afifo_irq_en_mask                                 HDMI_P0_CH_CR_ch_afifo_irq_en_mask
#define  HDMI_CH_CR_br_afifo_align_mode(data)                            HDMI_P0_CH_CR_br_afifo_align_mode(data)
#define  HDMI_CH_CR_r_ch_afifo_en(data)                                  HDMI_P0_CH_CR_r_ch_afifo_en(data)
#define  HDMI_CH_CR_g_ch_afifo_en(data)                                  HDMI_P0_CH_CR_g_ch_afifo_en(data)
#define  HDMI_CH_CR_b_ch_afifo_en(data)                                  HDMI_P0_CH_CR_b_ch_afifo_en(data)
#define  HDMI_CH_CR_ch_sync_sel(data)                                    HDMI_P0_CH_CR_ch_sync_sel(data)
#define  HDMI_CH_CR_dummy_19_17(data)                                    HDMI_P0_CH_CR_dummy_19_17(data)
#define  HDMI_CH_CR_rden_thr(data)                                       HDMI_P0_CH_CR_rden_thr(data)
#define  HDMI_CH_CR_dummy_11_9(data)                                     HDMI_P0_CH_CR_dummy_11_9(data)
#define  HDMI_CH_CR_udwater_thr(data)                                    HDMI_P0_CH_CR_udwater_thr(data)
#define  HDMI_CH_CR_dummy_3(data)                                        HDMI_P0_CH_CR_dummy_3(data)
#define  HDMI_CH_CR_flush(data)                                          HDMI_P0_CH_CR_flush(data)
#define  HDMI_CH_CR_ch_afifo_airq_en(data)                               HDMI_P0_CH_CR_ch_afifo_airq_en(data)
#define  HDMI_CH_CR_ch_afifo_irq_en(data)                                HDMI_P0_CH_CR_ch_afifo_irq_en(data)
#define  HDMI_CH_CR_get_br_afifo_align_mode(data)                        HDMI_P0_CH_CR_get_br_afifo_align_mode(data)
#define  HDMI_CH_CR_get_r_ch_afifo_en(data)                              HDMI_P0_CH_CR_get_r_ch_afifo_en(data)
#define  HDMI_CH_CR_get_g_ch_afifo_en(data)                              HDMI_P0_CH_CR_get_g_ch_afifo_en(data)
#define  HDMI_CH_CR_get_b_ch_afifo_en(data)                              HDMI_P0_CH_CR_get_b_ch_afifo_en(data)
#define  HDMI_CH_CR_get_ch_sync_sel(data)                                HDMI_P0_CH_CR_get_ch_sync_sel(data)
#define  HDMI_CH_CR_get_dummy_19_17(data)                                HDMI_P0_CH_CR_get_dummy_19_17(data)
#define  HDMI_CH_CR_get_rden_thr(data)                                   HDMI_P0_CH_CR_get_rden_thr(data)
#define  HDMI_CH_CR_get_dummy_11_9(data)                                 HDMI_P0_CH_CR_get_dummy_11_9(data)
#define  HDMI_CH_CR_get_udwater_thr(data)                                HDMI_P0_CH_CR_get_udwater_thr(data)
#define  HDMI_CH_CR_get_dummy_3(data)                                    HDMI_P0_CH_CR_get_dummy_3(data)
#define  HDMI_CH_CR_get_flush(data)                                      HDMI_P0_CH_CR_get_flush(data)
#define  HDMI_CH_CR_get_ch_afifo_airq_en(data)                           HDMI_P0_CH_CR_get_ch_afifo_airq_en(data)
#define  HDMI_CH_CR_get_ch_afifo_irq_en(data)                            HDMI_P0_CH_CR_get_ch_afifo_irq_en(data)


#define  HDMI_CH_SR_rgb_de_align_flag_mask                               HDMI_P0_CH_SR_rgb_de_align_flag_mask
#define  HDMI_CH_SR_r_rwclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_r_rwclk_det_timeout_flag_mask
#define  HDMI_CH_SR_r_wrclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_r_wrclk_det_timeout_flag_mask
#define  HDMI_CH_SR_g_rwclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_g_rwclk_det_timeout_flag_mask
#define  HDMI_CH_SR_g_wrclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_g_wrclk_det_timeout_flag_mask
#define  HDMI_CH_SR_b_rwclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_b_rwclk_det_timeout_flag_mask
#define  HDMI_CH_SR_b_wrclk_det_timeout_flag_mask                        HDMI_P0_CH_SR_b_wrclk_det_timeout_flag_mask
#define  HDMI_CH_SR_r_rudflow_flag_mask                                  HDMI_P0_CH_SR_r_rudflow_flag_mask
#define  HDMI_CH_SR_r_wovflow_flag_mask                                  HDMI_P0_CH_SR_r_wovflow_flag_mask
#define  HDMI_CH_SR_r_rflush_flag_mask                                   HDMI_P0_CH_SR_r_rflush_flag_mask
#define  HDMI_CH_SR_r_rw_water_lv_mask                                   HDMI_P0_CH_SR_r_rw_water_lv_mask
#define  HDMI_CH_SR_g_rudflow_flag_mask                                  HDMI_P0_CH_SR_g_rudflow_flag_mask
#define  HDMI_CH_SR_g_wovflow_flag_mask                                  HDMI_P0_CH_SR_g_wovflow_flag_mask
#define  HDMI_CH_SR_g_rflush_flag_mask                                   HDMI_P0_CH_SR_g_rflush_flag_mask
#define  HDMI_CH_SR_g_rw_water_lv_mask                                   HDMI_P0_CH_SR_g_rw_water_lv_mask
#define  HDMI_CH_SR_b_rudflow_flag_mask                                  HDMI_P0_CH_SR_b_rudflow_flag_mask
#define  HDMI_CH_SR_b_wovflow_flag_mask                                  HDMI_P0_CH_SR_b_wovflow_flag_mask
#define  HDMI_CH_SR_b_rflush_flag_mask                                   HDMI_P0_CH_SR_b_rflush_flag_mask
#define  HDMI_CH_SR_b_rw_water_lv_mask                                   HDMI_P0_CH_SR_b_rw_water_lv_mask
#define  HDMI_CH_SR_rgb_de_align_flag(data)                              HDMI_P0_CH_SR_rgb_de_align_flag(data)
#define  HDMI_CH_SR_r_rwclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_r_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_r_wrclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_r_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_g_rwclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_g_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_g_wrclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_g_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_b_rwclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_b_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_b_wrclk_det_timeout_flag(data)                       HDMI_P0_CH_SR_b_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_r_rudflow_flag(data)                                 HDMI_P0_CH_SR_r_rudflow_flag(data)
#define  HDMI_CH_SR_r_wovflow_flag(data)                                 HDMI_P0_CH_SR_r_wovflow_flag(data)
#define  HDMI_CH_SR_r_rflush_flag(data)                                  HDMI_P0_CH_SR_r_rflush_flag(data)
#define  HDMI_CH_SR_r_rw_water_lv(data)                                  HDMI_P0_CH_SR_r_rw_water_lv(data)
#define  HDMI_CH_SR_g_rudflow_flag(data)                                 HDMI_P0_CH_SR_g_rudflow_flag(data)
#define  HDMI_CH_SR_g_wovflow_flag(data)                                 HDMI_P0_CH_SR_g_wovflow_flag(data)
#define  HDMI_CH_SR_g_rflush_flag(data)                                  HDMI_P0_CH_SR_g_rflush_flag(data)
#define  HDMI_CH_SR_g_rw_water_lv(data)                                  HDMI_P0_CH_SR_g_rw_water_lv(data)
#define  HDMI_CH_SR_b_rudflow_flag(data)                                 HDMI_P0_CH_SR_b_rudflow_flag(data)
#define  HDMI_CH_SR_b_wovflow_flag(data)                                 HDMI_P0_CH_SR_b_wovflow_flag(data)
#define  HDMI_CH_SR_b_rflush_flag(data)                                  HDMI_P0_CH_SR_b_rflush_flag(data)
#define  HDMI_CH_SR_b_rw_water_lv(data)                                  HDMI_P0_CH_SR_b_rw_water_lv(data)
#define  HDMI_CH_SR_get_rgb_de_align_flag(data)                          HDMI_P0_CH_SR_get_rgb_de_align_flag(data)
#define  HDMI_CH_SR_get_r_rwclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_r_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_r_wrclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_r_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_g_rwclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_g_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_g_wrclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_g_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_b_rwclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_b_rwclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_b_wrclk_det_timeout_flag(data)                   HDMI_P0_CH_SR_get_b_wrclk_det_timeout_flag(data)
#define  HDMI_CH_SR_get_r_rudflow_flag(data)                             HDMI_P0_CH_SR_get_r_rudflow_flag(data)
#define  HDMI_CH_SR_get_r_wovflow_flag(data)                             HDMI_P0_CH_SR_get_r_wovflow_flag(data)
#define  HDMI_CH_SR_get_r_rflush_flag(data)                              HDMI_P0_CH_SR_get_r_rflush_flag(data)
#define  HDMI_CH_SR_get_r_rw_water_lv(data)                              HDMI_P0_CH_SR_get_r_rw_water_lv(data)
#define  HDMI_CH_SR_get_g_rudflow_flag(data)                             HDMI_P0_CH_SR_get_g_rudflow_flag(data)
#define  HDMI_CH_SR_get_g_wovflow_flag(data)                             HDMI_P0_CH_SR_get_g_wovflow_flag(data)
#define  HDMI_CH_SR_get_g_rflush_flag(data)                              HDMI_P0_CH_SR_get_g_rflush_flag(data)
#define  HDMI_CH_SR_get_g_rw_water_lv(data)                              HDMI_P0_CH_SR_get_g_rw_water_lv(data)
#define  HDMI_CH_SR_get_b_rudflow_flag(data)                             HDMI_P0_CH_SR_get_b_rudflow_flag(data)
#define  HDMI_CH_SR_get_b_wovflow_flag(data)                             HDMI_P0_CH_SR_get_b_wovflow_flag(data)
#define  HDMI_CH_SR_get_b_rflush_flag(data)                              HDMI_P0_CH_SR_get_b_rflush_flag(data)
#define  HDMI_CH_SR_get_b_rw_water_lv(data)                              HDMI_P0_CH_SR_get_b_rw_water_lv(data)


#define  HDMI_HDMI_2p0_CR_lfsr0_mask                                     HDMI_P0_HDMI_2p0_CR_lfsr0_mask
#define  HDMI_HDMI_2p0_CR_gating_en_mask                                 HDMI_P0_HDMI_2p0_CR_gating_en_mask
#define  HDMI_HDMI_2p0_CR_ctr_char_num_mask                              HDMI_P0_HDMI_2p0_CR_ctr_char_num_mask
#define  HDMI_HDMI_2p0_CR_hdmi_2p0_en_mask                               HDMI_P0_HDMI_2p0_CR_hdmi_2p0_en_mask
#define  HDMI_HDMI_2p0_CR_lfsr0(data)                                    HDMI_P0_HDMI_2p0_CR_lfsr0(data)
#define  HDMI_HDMI_2p0_CR_gating_en(data)                                HDMI_P0_HDMI_2p0_CR_gating_en(data)
#define  HDMI_HDMI_2p0_CR_ctr_char_num(data)                             HDMI_P0_HDMI_2p0_CR_ctr_char_num(data)
#define  HDMI_HDMI_2p0_CR_hdmi_2p0_en(data)                              HDMI_P0_HDMI_2p0_CR_hdmi_2p0_en(data)
#define  HDMI_HDMI_2p0_CR_get_lfsr0(data)                                HDMI_P0_HDMI_2p0_CR_get_lfsr0(data)
#define  HDMI_HDMI_2p0_CR_get_gating_en(data)                            HDMI_P0_HDMI_2p0_CR_get_gating_en(data)
#define  HDMI_HDMI_2p0_CR_get_ctr_char_num(data)                         HDMI_P0_HDMI_2p0_CR_get_ctr_char_num(data)
#define  HDMI_HDMI_2p0_CR_get_hdmi_2p0_en(data)                          HDMI_P0_HDMI_2p0_CR_get_hdmi_2p0_en(data)


#define  HDMI_HDMI_2p0_CR1_lfsr2_mask                                    HDMI_P0_HDMI_2p0_CR1_lfsr2_mask
#define  HDMI_HDMI_2p0_CR1_lfsr1_mask                                    HDMI_P0_HDMI_2p0_CR1_lfsr1_mask
#define  HDMI_HDMI_2p0_CR1_lfsr2(data)                                   HDMI_P0_HDMI_2p0_CR1_lfsr2(data)
#define  HDMI_HDMI_2p0_CR1_lfsr1(data)                                   HDMI_P0_HDMI_2p0_CR1_lfsr1(data)
#define  HDMI_HDMI_2p0_CR1_get_lfsr2(data)                               HDMI_P0_HDMI_2p0_CR1_get_lfsr2(data)
#define  HDMI_HDMI_2p0_CR1_get_lfsr1(data)                               HDMI_P0_HDMI_2p0_CR1_get_lfsr1(data)


#define  HDMI_SCR_CR_char_lock_mask                                      HDMI_P0_SCR_CR_char_lock_mask
#define  HDMI_SCR_CR_uscr_per_glitch_r_mask                              HDMI_P0_SCR_CR_uscr_per_glitch_r_mask
#define  HDMI_SCR_CR_uscr_per_glitch_g_mask                              HDMI_P0_SCR_CR_uscr_per_glitch_g_mask
#define  HDMI_SCR_CR_uscr_per_glitch_b_mask                              HDMI_P0_SCR_CR_uscr_per_glitch_b_mask
#define  HDMI_SCR_CR_uscr_char_flag_r_mask                               HDMI_P0_SCR_CR_uscr_char_flag_r_mask
#define  HDMI_SCR_CR_uscr_char_flag_g_mask                               HDMI_P0_SCR_CR_uscr_char_flag_g_mask
#define  HDMI_SCR_CR_uscr_char_flag_b_mask                               HDMI_P0_SCR_CR_uscr_char_flag_b_mask
#define  HDMI_SCR_CR_uscr_num_mask                                       HDMI_P0_SCR_CR_uscr_num_mask
#define  HDMI_SCR_CR_dummy_3_mask                                        HDMI_P0_SCR_CR_dummy_3_mask
#define  HDMI_SCR_CR_bc_sel_mask                                         HDMI_P0_SCR_CR_bc_sel_mask
#define  HDMI_SCR_CR_scr_en_fw_mask                                      HDMI_P0_SCR_CR_scr_en_fw_mask
#define  HDMI_SCR_CR_scr_auto_mask                                       HDMI_P0_SCR_CR_scr_auto_mask
#define  HDMI_SCR_CR_char_lock(data)                                     HDMI_P0_SCR_CR_char_lock(data)
#define  HDMI_SCR_CR_uscr_per_glitch_r(data)                             HDMI_P0_SCR_CR_uscr_per_glitch_r(data)
#define  HDMI_SCR_CR_uscr_per_glitch_g(data)                             HDMI_P0_SCR_CR_uscr_per_glitch_g(data)
#define  HDMI_SCR_CR_uscr_per_glitch_b(data)                             HDMI_P0_SCR_CR_uscr_per_glitch_b(data)
#define  HDMI_SCR_CR_uscr_char_flag_r(data)                              HDMI_P0_SCR_CR_uscr_char_flag_r(data)
#define  HDMI_SCR_CR_uscr_char_flag_g(data)                              HDMI_P0_SCR_CR_uscr_char_flag_g(data)
#define  HDMI_SCR_CR_uscr_char_flag_b(data)                              HDMI_P0_SCR_CR_uscr_char_flag_b(data)
#define  HDMI_SCR_CR_uscr_num(data)                                      HDMI_P0_SCR_CR_uscr_num(data)
#define  HDMI_SCR_CR_dummy_3(data)                                       HDMI_P0_SCR_CR_dummy_3(data)
#define  HDMI_SCR_CR_bc_sel(data)                                        HDMI_P0_SCR_CR_bc_sel(data)
#define  HDMI_SCR_CR_scr_en_fw(data)                                     HDMI_P0_SCR_CR_scr_en_fw(data)
#define  HDMI_SCR_CR_scr_auto(data)                                      HDMI_P0_SCR_CR_scr_auto(data)
#define  HDMI_SCR_CR_get_char_lock(data)                                 HDMI_P0_SCR_CR_get_char_lock(data)
#define  HDMI_SCR_CR_get_uscr_per_glitch_r(data)                         HDMI_P0_SCR_CR_get_uscr_per_glitch_r(data)
#define  HDMI_SCR_CR_get_uscr_per_glitch_g(data)                         HDMI_P0_SCR_CR_get_uscr_per_glitch_g(data)
#define  HDMI_SCR_CR_get_uscr_per_glitch_b(data)                         HDMI_P0_SCR_CR_get_uscr_per_glitch_b(data)
#define  HDMI_SCR_CR_get_uscr_char_flag_r(data)                          HDMI_P0_SCR_CR_get_uscr_char_flag_r(data)
#define  HDMI_SCR_CR_get_uscr_char_flag_g(data)                          HDMI_P0_SCR_CR_get_uscr_char_flag_g(data)
#define  HDMI_SCR_CR_get_uscr_char_flag_b(data)                          HDMI_P0_SCR_CR_get_uscr_char_flag_b(data)
#define  HDMI_SCR_CR_get_uscr_num(data)                                  HDMI_P0_SCR_CR_get_uscr_num(data)
#define  HDMI_SCR_CR_get_dummy_3(data)                                   HDMI_P0_SCR_CR_get_dummy_3(data)
#define  HDMI_SCR_CR_get_bc_sel(data)                                    HDMI_P0_SCR_CR_get_bc_sel(data)
#define  HDMI_SCR_CR_get_scr_en_fw(data)                                 HDMI_P0_SCR_CR_get_scr_en_fw(data)
#define  HDMI_SCR_CR_get_scr_auto(data)                                  HDMI_P0_SCR_CR_get_scr_auto(data)


#define  HDMI_CERCR_err_cnt_sel_mask                                     HDMI_P0_CERCR_err_cnt_sel_mask
#define  HDMI_CERCR_scdc_ced_en_mask                                     HDMI_P0_CERCR_scdc_ced_en_mask
#define  HDMI_CERCR_ch_locked_reset_mask                                 HDMI_P0_CERCR_ch_locked_reset_mask
#define  HDMI_CERCR_ch2_over_max_err_num_mask                            HDMI_P0_CERCR_ch2_over_max_err_num_mask
#define  HDMI_CERCR_ch1_over_max_err_num_mask                            HDMI_P0_CERCR_ch1_over_max_err_num_mask
#define  HDMI_CERCR_ch0_over_max_err_num_mask                            HDMI_P0_CERCR_ch0_over_max_err_num_mask
#define  HDMI_CERCR_max_err_num_mask                                     HDMI_P0_CERCR_max_err_num_mask
#define  HDMI_CERCR_valid_reset_mask                                     HDMI_P0_CERCR_valid_reset_mask
#define  HDMI_CERCR_reset_err_det_mask                                   HDMI_P0_CERCR_reset_err_det_mask
#define  HDMI_CERCR_keep_err_det_mask                                    HDMI_P0_CERCR_keep_err_det_mask
#define  HDMI_CERCR_refer_implem_mask                                    HDMI_P0_CERCR_refer_implem_mask
#define  HDMI_CERCR_reset_mask                                           HDMI_P0_CERCR_reset_mask
#define  HDMI_CERCR_period_mask                                          HDMI_P0_CERCR_period_mask
#define  HDMI_CERCR_ced_upd_num_mask                                     HDMI_P0_CERCR_ced_upd_num_mask
#define  HDMI_CERCR_mode_mask                                            HDMI_P0_CERCR_mode_mask
#define  HDMI_CERCR_en_mask                                              HDMI_P0_CERCR_en_mask
#define  HDMI_CERCR_err_cnt_sel(data)                                    HDMI_P0_CERCR_err_cnt_sel(data)
#define  HDMI_CERCR_scdc_ced_en(data)                                    HDMI_P0_CERCR_scdc_ced_en(data)
#define  HDMI_CERCR_ch_locked_reset(data)                                HDMI_P0_CERCR_ch_locked_reset(data)
#define  HDMI_CERCR_ch2_over_max_err_num(data)                           HDMI_P0_CERCR_ch2_over_max_err_num(data)
#define  HDMI_CERCR_ch1_over_max_err_num(data)                           HDMI_P0_CERCR_ch1_over_max_err_num(data)
#define  HDMI_CERCR_ch0_over_max_err_num(data)                           HDMI_P0_CERCR_ch0_over_max_err_num(data)
#define  HDMI_CERCR_max_err_num(data)                                    HDMI_P0_CERCR_max_err_num(data)
#define  HDMI_CERCR_valid_reset(data)                                    HDMI_P0_CERCR_valid_reset(data)
#define  HDMI_CERCR_reset_err_det(data)                                  HDMI_P0_CERCR_reset_err_det(data)
#define  HDMI_CERCR_keep_err_det(data)                                   HDMI_P0_CERCR_keep_err_det(data)
#define  HDMI_CERCR_refer_implem(data)                                   HDMI_P0_CERCR_refer_implem(data)
#define  HDMI_CERCR_reset(data)                                          HDMI_P0_CERCR_reset(data)
#define  HDMI_CERCR_period(data)                                         HDMI_P0_CERCR_period(data)
#define  HDMI_CERCR_ced_upd_num(data)                                    HDMI_P0_CERCR_ced_upd_num(data)
#define  HDMI_CERCR_mode(data)                                           HDMI_P0_CERCR_mode(data)
#define  HDMI_CERCR_en(data)                                             HDMI_P0_CERCR_en(data)
#define  HDMI_CERCR_get_err_cnt_sel(data)                                HDMI_P0_CERCR_get_err_cnt_sel(data)
#define  HDMI_CERCR_get_scdc_ced_en(data)                                HDMI_P0_CERCR_get_scdc_ced_en(data)
#define  HDMI_CERCR_get_ch_locked_reset(data)                            HDMI_P0_CERCR_get_ch_locked_reset(data)
#define  HDMI_CERCR_get_ch2_over_max_err_num(data)                       HDMI_P0_CERCR_get_ch2_over_max_err_num(data)
#define  HDMI_CERCR_get_ch1_over_max_err_num(data)                       HDMI_P0_CERCR_get_ch1_over_max_err_num(data)
#define  HDMI_CERCR_get_ch0_over_max_err_num(data)                       HDMI_P0_CERCR_get_ch0_over_max_err_num(data)
#define  HDMI_CERCR_get_max_err_num(data)                                HDMI_P0_CERCR_get_max_err_num(data)
#define  HDMI_CERCR_get_valid_reset(data)                                HDMI_P0_CERCR_get_valid_reset(data)
#define  HDMI_CERCR_get_reset_err_det(data)                              HDMI_P0_CERCR_get_reset_err_det(data)
#define  HDMI_CERCR_get_keep_err_det(data)                               HDMI_P0_CERCR_get_keep_err_det(data)
#define  HDMI_CERCR_get_refer_implem(data)                               HDMI_P0_CERCR_get_refer_implem(data)
#define  HDMI_CERCR_get_reset(data)                                      HDMI_P0_CERCR_get_reset(data)
#define  HDMI_CERCR_get_period(data)                                     HDMI_P0_CERCR_get_period(data)
#define  HDMI_CERCR_get_ced_upd_num(data)                                HDMI_P0_CERCR_get_ced_upd_num(data)
#define  HDMI_CERCR_get_mode(data)                                       HDMI_P0_CERCR_get_mode(data)
#define  HDMI_CERCR_get_en(data)                                         HDMI_P0_CERCR_get_en(data)


#define  HDMI_CERSR0_err_cnt1_video_mask                                 HDMI_P0_CERSR0_err_cnt1_video_mask
#define  HDMI_CERSR0_err_cnt0_video_mask                                 HDMI_P0_CERSR0_err_cnt0_video_mask
#define  HDMI_CERSR0_err_cnt1_video(data)                                HDMI_P0_CERSR0_err_cnt1_video(data)
#define  HDMI_CERSR0_err_cnt0_video(data)                                HDMI_P0_CERSR0_err_cnt0_video(data)
#define  HDMI_CERSR0_get_err_cnt1_video(data)                            HDMI_P0_CERSR0_get_err_cnt1_video(data)
#define  HDMI_CERSR0_get_err_cnt0_video(data)                            HDMI_P0_CERSR0_get_err_cnt0_video(data)


#define  HDMI_CERSR1_err_cnt0_pkt_mask                                   HDMI_P0_CERSR1_err_cnt0_pkt_mask
#define  HDMI_CERSR1_err_cnt2_video_mask                                 HDMI_P0_CERSR1_err_cnt2_video_mask
#define  HDMI_CERSR1_err_cnt0_pkt(data)                                  HDMI_P0_CERSR1_err_cnt0_pkt(data)
#define  HDMI_CERSR1_err_cnt2_video(data)                                HDMI_P0_CERSR1_err_cnt2_video(data)
#define  HDMI_CERSR1_get_err_cnt0_pkt(data)                              HDMI_P0_CERSR1_get_err_cnt0_pkt(data)
#define  HDMI_CERSR1_get_err_cnt2_video(data)                            HDMI_P0_CERSR1_get_err_cnt2_video(data)


#define  HDMI_CERSR2_err_cnt2_pkt_mask                                   HDMI_P0_CERSR2_err_cnt2_pkt_mask
#define  HDMI_CERSR2_err_cnt1_pkt_mask                                   HDMI_P0_CERSR2_err_cnt1_pkt_mask
#define  HDMI_CERSR2_err_cnt2_pkt(data)                                  HDMI_P0_CERSR2_err_cnt2_pkt(data)
#define  HDMI_CERSR2_err_cnt1_pkt(data)                                  HDMI_P0_CERSR2_err_cnt1_pkt(data)
#define  HDMI_CERSR2_get_err_cnt2_pkt(data)                              HDMI_P0_CERSR2_get_err_cnt2_pkt(data)
#define  HDMI_CERSR2_get_err_cnt1_pkt(data)                              HDMI_P0_CERSR2_get_err_cnt1_pkt(data)


#define  HDMI_CERSR3_err_cnt1_ctr_mask                                   HDMI_P0_CERSR3_err_cnt1_ctr_mask
#define  HDMI_CERSR3_err_cnt0_ctr_mask                                   HDMI_P0_CERSR3_err_cnt0_ctr_mask
#define  HDMI_CERSR3_err_cnt1_ctr(data)                                  HDMI_P0_CERSR3_err_cnt1_ctr(data)
#define  HDMI_CERSR3_err_cnt0_ctr(data)                                  HDMI_P0_CERSR3_err_cnt0_ctr(data)
#define  HDMI_CERSR3_get_err_cnt1_ctr(data)                              HDMI_P0_CERSR3_get_err_cnt1_ctr(data)
#define  HDMI_CERSR3_get_err_cnt0_ctr(data)                              HDMI_P0_CERSR3_get_err_cnt0_ctr(data)


#define  HDMI_CERSR4_err_cnt2_ctr_mask                                   HDMI_P0_CERSR4_err_cnt2_ctr_mask
#define  HDMI_CERSR4_err_cnt2_ctr(data)                                  HDMI_P0_CERSR4_err_cnt2_ctr(data)
#define  HDMI_CERSR4_get_err_cnt2_ctr(data)                              HDMI_P0_CERSR4_get_err_cnt2_ctr(data)

#define  HDMI_YUV420_CR_fw_htotal_mask                                   HDMI_P0_YUV420_CR_fw_htotal_mask//HDMI_P0_YUV420_CR_fw_htotal_mask
#define  HDMI_YUV420_CR_fw_hblank_mask                                   HDMI_P0_YUV420_CR_fw_hblank_mask
#define  HDMI_YUV420_CR_fw_set_time_mask                                 HDMI_P0_YUV420_CR_fw_set_time_mask
#define  HDMI_YUV420_CR_dummy_2_mask                                     HDMI_P0_YUV420_CR_dummy_2_mask // no use
#define  HDMI_YUV420_CR_clkgen_en_mask                                   HDMI_P0_YUV420_CR_clkgen_en_mask //no use
#define  HDMI_YUV420_CR_en_mask                                          HDMI_P0_YUV420_CR_en_mask
#define  HDMI_YUV420_CR_fw_htotal(data)                                  HDMI_P0_YUV420_CR_fw_htotal(data)
#define  HDMI_YUV420_CR_fw_hblank(data)                                  HDMI_P0_YUV420_CR_fw_hblank(data)
#define  HDMI_YUV420_CR_fw_set_time(data)                                HDMI_P0_YUV420_CR_fw_set_time(data)
#define  HDMI_YUV420_CR_dummy_2(data)                                    HDMI_P0_YUV420_CR_dummy_2(data)
#define  HDMI_YUV420_CR_clkgen_en(data)                                  HDMI_P0_YUV420_CR_clkgen_en(data)
#define  HDMI_YUV420_CR_en(data)                                         HDMI_P0_YUV420_CR_en(data)
#define  HDMI_YUV420_CR_get_fw_htotal(data)                              HDMI_P0_YUV420_CR_get_fw_htotal(data)
#define  HDMI_YUV420_CR_get_fw_hblank(data)                              HDMI_P0_YUV420_CR_get_fw_hblank(data)
#define  HDMI_YUV420_CR_get_fw_set_time(data)                            HDMI_P0_YUV420_CR_get_fw_set_time(data)
#define  HDMI_YUV420_CR_get_dummy_2(data)                                HDMI_P0_YUV420_CR_get_dummy_2(data)
#define  HDMI_YUV420_CR_get_clkgen_en(data)                              HDMI_P0_YUV420_CR_get_clkgen_en(data)
#define  HDMI_YUV420_CR_get_en(data)                                     HDMI_P0_YUV420_CR_get_en(data)


#define  HDMI_YUV420_CR1_yuv420_clk_valid_en_mask                 COMMON_RX_420_HDMIRX_YUV420_CTRL_1_yuv420_clk_valid_en_mask
#define  HDMI_YUV420_CR1_fva_ds_mode_mask                                HDMI_P0_YUV420_CR1_fva_ds_mode_mask
#define  HDMI_YUV420_CR1_yuv4k120_mode_mask                              HDMI_P0_YUV420_CR1_yuv4k120_mode_mask //no use
#define  HDMI_YUV420_CR1_vactive_mask                                    HDMI_P0_YUV420_CR1_vactive_mask
#define  HDMI_YUV420_CR1_mode_mask                                       HDMI_P0_YUV420_CR1_mode_mask
#define  HDMI_YUV420_CR1_fva_ds_mode(data)                               HDMI_P0_YUV420_CR1_fva_ds_mode(data)
#define  HDMI_YUV420_CR1_yuv4k120_mode(data)                             HDMI_P0_YUV420_CR1_yuv4k120_mode(data)
#define  HDMI_YUV420_CR1_vactive(data)                                   HDMI_P0_YUV420_CR1_vactive(data)
#define  HDMI_YUV420_CR1_mode(data)                                      HDMI_P0_YUV420_CR1_mode(data)
#define  HDMI_YUV420_CR1_get_fva_ds_mode(data)                           HDMI_P0_YUV420_CR1_get_fva_ds_mode(data)
#define  HDMI_YUV420_CR1_get_yuv4k120_mode(data)                         HDMI_P0_YUV420_CR1_get_yuv4k120_mode(data)
#define  HDMI_YUV420_CR1_get_vactive(data)                               HDMI_P0_YUV420_CR1_get_vactive(data)
#define  HDMI_YUV420_CR1_get_mode(data)                                  HDMI_P0_YUV420_CR1_get_mode(data)


#define  HDMI_fapa_ct_tmp_all_mode_mask                                  HDMI_P0_fapa_ct_tmp_all_mode_mask
#define  HDMI_fapa_ct_tmp_sta_mask                                       HDMI_P0_fapa_ct_tmp_sta_mask
#define  HDMI_fapa_ct_tmp_all_mode(data)                                 HDMI_P0_fapa_ct_tmp_all_mode(data)
#define  HDMI_fapa_ct_tmp_sta(data)                                      HDMI_P0_fapa_ct_tmp_sta(data)
#define  HDMI_fapa_ct_tmp_get_all_mode(data)                             HDMI_P0_fapa_ct_tmp_get_all_mode(data)
#define  HDMI_fapa_ct_tmp_get_sta(data)                                  HDMI_P0_fapa_ct_tmp_get_sta(data)


#define  HDMI_HDR_EM_CT2_sync_mask                                       HDMI_P0_HDR_EM_CT2_sync_mask
#define  HDMI_HDR_EM_CT2_recognize_oui_en_mask                           HDMI_P0_HDR_EM_CT2_recognize_oui_en_mask
#define  HDMI_HDR_EM_CT2_oui_3rd_mask                                    HDMI_P0_HDR_EM_CT2_oui_3rd_mask
#define  HDMI_HDR_EM_CT2_oui_2nd_mask                                    HDMI_P0_HDR_EM_CT2_oui_2nd_mask
#define  HDMI_HDR_EM_CT2_oui_1st_mask                                    HDMI_P0_HDR_EM_CT2_oui_1st_mask
#define  HDMI_HDR_EM_CT2_sync(data)                                      HDMI_P0_HDR_EM_CT2_sync(data)
#define  HDMI_HDR_EM_CT2_recognize_oui_en(data)                          HDMI_P0_HDR_EM_CT2_recognize_oui_en(data)
#define  HDMI_HDR_EM_CT2_oui_3rd(data)                                   HDMI_P0_HDR_EM_CT2_oui_3rd(data)
#define  HDMI_HDR_EM_CT2_oui_2nd(data)                                   HDMI_P0_HDR_EM_CT2_oui_2nd(data)
#define  HDMI_HDR_EM_CT2_oui_1st(data)                                   HDMI_P0_HDR_EM_CT2_oui_1st(data)
#define  HDMI_HDR_EM_CT2_get_sync(data)                                  HDMI_P0_HDR_EM_CT2_get_sync(data)
#define  HDMI_HDR_EM_CT2_get_recognize_oui_en(data)                      HDMI_P0_HDR_EM_CT2_get_recognize_oui_en(data)
#define  HDMI_HDR_EM_CT2_get_oui_3rd(data)                               HDMI_P0_HDR_EM_CT2_get_oui_3rd(data)
#define  HDMI_HDR_EM_CT2_get_oui_2nd(data)                               HDMI_P0_HDR_EM_CT2_get_oui_2nd(data)
#define  HDMI_HDR_EM_CT2_get_oui_1st(data)                               HDMI_P0_HDR_EM_CT2_get_oui_1st(data)


#define  HDMI_HDMI_VSEM_EMC_set_tag_msb_mask                             HDMI_P0_HDMI_VSEM_EMC_set_tag_msb_mask
#define  HDMI_HDMI_VSEM_EMC_set_tag_lsb_mask                             HDMI_P0_HDMI_VSEM_EMC_set_tag_lsb_mask
#define  HDMI_HDMI_VSEM_EMC_recognize_tag_en_mask                        HDMI_P0_HDMI_VSEM_EMC_recognize_tag_en_mask
#define  HDMI_HDMI_VSEM_EMC_vsem_clr_mask                                HDMI_P0_HDMI_VSEM_EMC_vsem_clr_mask
#define  HDMI_HDMI_VSEM_EMC_rec_em_vsem_irq_en_mask                      HDMI_P0_HDMI_VSEM_EMC_rec_em_vsem_irq_en_mask
#define  HDMI_HDMI_VSEM_EMC_set_tag_msb(data)                            HDMI_P0_HDMI_VSEM_EMC_set_tag_msb(data)
#define  HDMI_HDMI_VSEM_EMC_set_tag_lsb(data)                            HDMI_P0_HDMI_VSEM_EMC_set_tag_lsb(data)
#define  HDMI_HDMI_VSEM_EMC_recognize_tag_en(data)                       HDMI_P0_HDMI_VSEM_EMC_recognize_tag_en(data)
#define  HDMI_HDMI_VSEM_EMC_vsem_clr(data)                               HDMI_P0_HDMI_VSEM_EMC_vsem_clr(data)
#define  HDMI_HDMI_VSEM_EMC_rec_em_vsem_irq_en(data)                     HDMI_P0_HDMI_VSEM_EMC_rec_em_vsem_irq_en(data)
#define  HDMI_HDMI_VSEM_EMC_get_set_tag_msb(data)                        HDMI_P0_HDMI_VSEM_EMC_get_set_tag_msb(data)
#define  HDMI_HDMI_VSEM_EMC_get_set_tag_lsb(data)                        HDMI_P0_HDMI_VSEM_EMC_get_set_tag_lsb(data)
#define  HDMI_HDMI_VSEM_EMC_get_recognize_tag_en(data)                   HDMI_P0_HDMI_VSEM_EMC_get_recognize_tag_en(data)
#define  HDMI_HDMI_VSEM_EMC_get_vsem_clr(data)                           HDMI_P0_HDMI_VSEM_EMC_get_vsem_clr(data)
#define  HDMI_HDMI_VSEM_EMC_get_rec_em_vsem_irq_en(data)                 HDMI_P0_HDMI_VSEM_EMC_get_rec_em_vsem_irq_en(data)


#define  HDMI_HDMI_VSEM_EMC2_sync_mask                                   HDMI_P0_HDMI_VSEM_EMC2_sync_mask
#define  HDMI_HDMI_VSEM_EMC2_recognize_oui_en_mask                       HDMI_P0_HDMI_VSEM_EMC2_recognize_oui_en_mask
#define  HDMI_HDMI_VSEM_EMC2_oui_3rd_mask                                HDMI_P0_HDMI_VSEM_EMC2_oui_3rd_mask
#define  HDMI_HDMI_VSEM_EMC2_oui_2nd_mask                                HDMI_P0_HDMI_VSEM_EMC2_oui_2nd_mask
#define  HDMI_HDMI_VSEM_EMC2_oui_1st_mask                                HDMI_P0_HDMI_VSEM_EMC2_oui_1st_mask
#define  HDMI_HDMI_VSEM_EMC2_sync(data)                                  HDMI_P0_HDMI_VSEM_EMC2_sync(data)
#define  HDMI_HDMI_VSEM_EMC2_recognize_oui_en(data)                      HDMI_P0_HDMI_VSEM_EMC2_recognize_oui_en(data)
#define  HDMI_HDMI_VSEM_EMC2_oui_3rd(data)                               HDMI_P0_HDMI_VSEM_EMC2_oui_3rd(data)
#define  HDMI_HDMI_VSEM_EMC2_oui_2nd(data)                               HDMI_P0_HDMI_VSEM_EMC2_oui_2nd(data)
#define  HDMI_HDMI_VSEM_EMC2_oui_1st(data)                               HDMI_P0_HDMI_VSEM_EMC2_oui_1st(data)
#define  HDMI_HDMI_VSEM_EMC2_get_sync(data)                              HDMI_P0_HDMI_VSEM_EMC2_get_sync(data)
#define  HDMI_HDMI_VSEM_EMC2_get_recognize_oui_en(data)                  HDMI_P0_HDMI_VSEM_EMC2_get_recognize_oui_en(data)
#define  HDMI_HDMI_VSEM_EMC2_get_oui_3rd(data)                           HDMI_P0_HDMI_VSEM_EMC2_get_oui_3rd(data)
#define  HDMI_HDMI_VSEM_EMC2_get_oui_2nd(data)                           HDMI_P0_HDMI_VSEM_EMC2_get_oui_2nd(data)
#define  HDMI_HDMI_VSEM_EMC2_get_oui_1st(data)                           HDMI_P0_HDMI_VSEM_EMC2_get_oui_1st(data)


#define  HDMI_VSEM_ST_em_vsem_no_mask                                    HDMI_P0_VSEM_ST_em_vsem_no_mask
#define  HDMI_VSEM_ST_em_vsem_timeout_mask                               HDMI_P0_VSEM_ST_em_vsem_timeout_mask
#define  HDMI_VSEM_ST_em_vsem_discont_mask                               HDMI_P0_VSEM_ST_em_vsem_discont_mask
#define  HDMI_VSEM_ST_em_vsem_end_mask                                   HDMI_P0_VSEM_ST_em_vsem_end_mask
#define  HDMI_VSEM_ST_em_vsem_new_mask                                   HDMI_P0_VSEM_ST_em_vsem_new_mask
#define  HDMI_VSEM_ST_em_vsem_first_mask                                 HDMI_P0_VSEM_ST_em_vsem_first_mask
#define  HDMI_VSEM_ST_em_vsem_last_mask                                  HDMI_P0_VSEM_ST_em_vsem_last_mask
#define  HDMI_VSEM_ST_em_vsem_no(data)                                   HDMI_P0_VSEM_ST_em_vsem_no(data)
#define  HDMI_VSEM_ST_em_vsem_timeout(data)                              HDMI_P0_VSEM_ST_em_vsem_timeout(data)
#define  HDMI_VSEM_ST_em_vsem_discont(data)                              HDMI_P0_VSEM_ST_em_vsem_discont(data)
#define  HDMI_VSEM_ST_em_vsem_end(data)                                  HDMI_P0_VSEM_ST_em_vsem_end(data)
#define  HDMI_VSEM_ST_em_vsem_new(data)                                  HDMI_P0_VSEM_ST_em_vsem_new(data)
#define  HDMI_VSEM_ST_em_vsem_first(data)                                HDMI_P0_VSEM_ST_em_vsem_first(data)
#define  HDMI_VSEM_ST_em_vsem_last(data)                                 HDMI_P0_VSEM_ST_em_vsem_last(data)
#define  HDMI_VSEM_ST_get_em_vsem_no(data)                               HDMI_P0_VSEM_ST_get_em_vsem_no(data)
#define  HDMI_VSEM_ST_get_em_vsem_timeout(data)                          HDMI_P0_VSEM_ST_get_em_vsem_timeout(data)
#define  HDMI_VSEM_ST_get_em_vsem_discont(data)                          HDMI_P0_VSEM_ST_get_em_vsem_discont(data)
#define  HDMI_VSEM_ST_get_em_vsem_end(data)                              HDMI_P0_VSEM_ST_get_em_vsem_end(data)
#define  HDMI_VSEM_ST_get_em_vsem_new(data)                              HDMI_P0_VSEM_ST_get_em_vsem_new(data)
#define  HDMI_VSEM_ST_get_em_vsem_first(data)                            HDMI_P0_VSEM_ST_get_em_vsem_first(data)
#define  HDMI_VSEM_ST_get_em_vsem_last(data)                             HDMI_P0_VSEM_ST_get_em_vsem_last(data)


#define  HDMI_HD20_ps_ct_mac_out_en_mask                                 HDMI_P0_HD20_ps_ct_mac_out_en_mask
#define  HDMI_HD20_ps_ct_mac_on_sel_mask                                 HDMI_P0_HD20_ps_ct_mac_on_sel_mask
#define  HDMI_HD20_ps_ct_ps_bg_mode_mask                                 HDMI_P0_HD20_ps_ct_ps_bg_mode_mask
#define  HDMI_HD20_ps_ct_timer_cnt_end_mask                              HDMI_P0_HD20_ps_ct_timer_cnt_end_mask
#define  HDMI_HD20_ps_ct_hdcp_mode_mask                                  HDMI_P0_HD20_ps_ct_hdcp_mode_mask
#define  HDMI_HD20_ps_ct_fw_mode_mask                                    HDMI_P0_HD20_ps_ct_fw_mode_mask
#define  HDMI_HD20_ps_ct_ps_mode_mask                                    HDMI_P0_HD20_ps_ct_ps_mode_mask
#define  HDMI_HD20_ps_ct_mac_out_en(data)                                HDMI_P0_HD20_ps_ct_mac_out_en(data)
#define  HDMI_HD20_ps_ct_mac_on_sel(data)                                HDMI_P0_HD20_ps_ct_mac_on_sel(data)
#define  HDMI_HD20_ps_ct_ps_bg_mode(data)                                HDMI_P0_HD20_ps_ct_ps_bg_mode(data)
#define  HDMI_HD20_ps_ct_timer_cnt_end(data)                             HDMI_P0_HD20_ps_ct_timer_cnt_end(data)
#define  HDMI_HD20_ps_ct_hdcp_mode(data)                                 HDMI_P0_HD20_ps_ct_hdcp_mode(data)
#define  HDMI_HD20_ps_ct_fw_mode(data)                                   HDMI_P0_HD20_ps_ct_fw_mode(data)
#define  HDMI_HD20_ps_ct_ps_mode(data)                                   HDMI_P0_HD20_ps_ct_ps_mode(data)
#define  HDMI_HD20_ps_ct_get_mac_out_en(data)                            HDMI_P0_HD20_ps_ct_get_mac_out_en(data)
#define  HDMI_HD20_ps_ct_get_mac_on_sel(data)                            HDMI_P0_HD20_ps_ct_get_mac_on_sel(data)
#define  HDMI_HD20_ps_ct_get_ps_bg_mode(data)                            HDMI_P0_HD20_ps_ct_get_ps_bg_mode(data)
#define  HDMI_HD20_ps_ct_get_timer_cnt_end(data)                         HDMI_P0_HD20_ps_ct_get_timer_cnt_end(data)
#define  HDMI_HD20_ps_ct_get_hdcp_mode(data)                             HDMI_P0_HD20_ps_ct_get_hdcp_mode(data)
#define  HDMI_HD20_ps_ct_get_fw_mode(data)                               HDMI_P0_HD20_ps_ct_get_fw_mode(data)
#define  HDMI_HD20_ps_ct_get_ps_mode(data)                               HDMI_P0_HD20_ps_ct_get_ps_mode(data)


#define  HDMI_HD20_ps_st_unscr_ctr_flag_mask                             HDMI_P0_HD20_ps_st_unscr_ctr_flag_mask
#define  HDMI_HD20_ps_st_win_opp_detect_mask                             HDMI_P0_HD20_ps_st_win_opp_detect_mask
#define  HDMI_HD20_ps_st_irq_en_pwron_mask                               HDMI_P0_HD20_ps_st_irq_en_pwron_mask
#define  HDMI_HD20_ps_st_irq_en_pwroff_mask                              HDMI_P0_HD20_ps_st_irq_en_pwroff_mask
#define  HDMI_HD20_ps_st_phy_power_on_flag_mask                          HDMI_P0_HD20_ps_st_phy_power_on_flag_mask
#define  HDMI_HD20_ps_st_pwron_mask                                      HDMI_P0_HD20_ps_st_pwron_mask
#define  HDMI_HD20_ps_st_pwroff_mask                                     HDMI_P0_HD20_ps_st_pwroff_mask
#define  HDMI_HD20_ps_st_unscr_ctr_flag(data)                            HDMI_P0_HD20_ps_st_unscr_ctr_flag(data)
#define  HDMI_HD20_ps_st_win_opp_detect(data)                            HDMI_P0_HD20_ps_st_win_opp_detect(data)
#define  HDMI_HD20_ps_st_irq_en_pwron(data)                              HDMI_P0_HD20_ps_st_irq_en_pwron(data)
#define  HDMI_HD20_ps_st_irq_en_pwroff(data)                             HDMI_P0_HD20_ps_st_irq_en_pwroff(data)
#define  HDMI_HD20_ps_st_phy_power_on_flag(data)                         HDMI_P0_HD20_ps_st_phy_power_on_flag(data)
#define  HDMI_HD20_ps_st_pwron(data)                                     HDMI_P0_HD20_ps_st_pwron(data)
#define  HDMI_HD20_ps_st_pwroff(data)                                    HDMI_P0_HD20_ps_st_pwroff(data)
#define  HDMI_HD20_ps_st_get_unscr_ctr_flag(data)                        HDMI_P0_HD20_ps_st_get_unscr_ctr_flag(data)
#define  HDMI_HD20_ps_st_get_win_opp_detect(data)                        HDMI_P0_HD20_ps_st_get_win_opp_detect(data)
#define  HDMI_HD20_ps_st_get_irq_en_pwron(data)                          HDMI_P0_HD20_ps_st_get_irq_en_pwron(data)
#define  HDMI_HD20_ps_st_get_irq_en_pwroff(data)                         HDMI_P0_HD20_ps_st_get_irq_en_pwroff(data)
#define  HDMI_HD20_ps_st_get_phy_power_on_flag(data)                     HDMI_P0_HD20_ps_st_get_phy_power_on_flag(data)
#define  HDMI_HD20_ps_st_get_pwron(data)                                 HDMI_P0_HD20_ps_st_get_pwron(data)
#define  HDMI_HD20_ps_st_get_pwroff(data)                                HDMI_P0_HD20_ps_st_get_pwroff(data)
#define  HDMI_irq_all_status_airq_fw_all_0_mask                          HDMI_P0_irq_all_status_airq_fw_all_0_mask
#define  HDMI_irq_all_status_irq_fw_all_0_mask                           HDMI_P0_irq_all_status_irq_fw_all_0_mask
#define  HDMI_irq_all_status_airq_fw_all_0(data)                         HDMI_P0_irq_all_status_airq_fw_all_0(data)
#define  HDMI_irq_all_status_irq_fw_all_0(data)                          HDMI_P0_irq_all_status_irq_fw_all_0(data)
#define  HDMI_irq_all_status_get_airq_fw_all_0(data)                     HDMI_P0_irq_all_status_get_airq_fw_all_0(data)
#define  HDMI_irq_all_status_get_irq_fw_all_0(data)                      HDMI_P0_irq_all_status_get_irq_fw_all_0(data)


#define  HDMI_xtal_1ms_cnt_mask                                          HDMI_P0_xtal_1ms_cnt_mask
#define  HDMI_xtal_1ms_cnt(data)                                         HDMI_P0_xtal_1ms_cnt(data)
#define  HDMI_xtal_1ms_get_cnt(data)                                     HDMI_P0_xtal_1ms_get_cnt(data)


#define  HDMI_xtal_10ms_cnt_mask                                         HDMI_P0_xtal_10ms_cnt_mask
#define  HDMI_xtal_10ms_cnt(data)                                        HDMI_P0_xtal_10ms_cnt(data)
#define  HDMI_xtal_10ms_get_cnt(data)                                    HDMI_P0_xtal_10ms_get_cnt(data)


#define  HDMI_ps_measure_ctrl_en_mask                                    HDMI_P0_ps_measure_ctrl_en_mask
#define  HDMI_ps_measure_ctrl_en(data)                                   HDMI_P0_ps_measure_ctrl_en(data)
#define  HDMI_ps_measure_ctrl_get_en(data)                               HDMI_P0_ps_measure_ctrl_get_en(data)


#define  HDMI_ps_measure_xtal_clk_vs2vs_cycle_count_mask                 HDMI_P0_ps_measure_xtal_clk_vs2vs_cycle_count_mask
#define  HDMI_ps_measure_xtal_clk_vs2vs_cycle_count(data)                HDMI_P0_ps_measure_xtal_clk_vs2vs_cycle_count(data)
#define  HDMI_ps_measure_xtal_clk_get_vs2vs_cycle_count(data)            HDMI_P0_ps_measure_xtal_clk_get_vs2vs_cycle_count(data)


#define  HDMI_ps_measure_tmds_clk_last_pixel2vs_count_mask               HDMI_P0_ps_measure_tmds_clk_last_pixel2vs_count_mask
#define  HDMI_ps_measure_tmds_clk_last_pixel2vs_count(data)              HDMI_P0_ps_measure_tmds_clk_last_pixel2vs_count(data)
#define  HDMI_ps_measure_tmds_clk_get_last_pixel2vs_count(data)          HDMI_P0_ps_measure_tmds_clk_get_last_pixel2vs_count(data)


#define HDMI_HDMI_PA_IRQ_EN_hfvs_absence_irq_en_mask                     HDMI_P0_HDMI_PA_IRQ_EN_hfvs_absence_irq_en_mask
#define HDMI_HDMI_PA_IRQ_EN_hfvs_absence_irq_en(data)                    HDMI_P0_HDMI_PA_IRQ_EN_hfvs_absence_irq_en(data)
#define HDMI_HDMI_PA_IRQ_EN_get_hfvs_absence_irq_en(data)                HDMI_P0_HDMI_PA_IRQ_EN_get_hfvs_absence_irq_en(data)


#define HDMI_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta_mask                   HDMI_P0_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta_mask
#define HDMI_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta(data)                  HDMI_P0_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta(data)
#define HDMI_HDMI_PA_IRQ_STA_get_hfvs_absence_irq_sta(data)              HDMI_P0_HDMI_PA_IRQ_STA_get_hfvs_absence_irq_sta(data)


#define HDMI_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt_mask               HDMI_P0_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt_mask
#define HDMI_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt(data)              HDMI_P0_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt(data)
#define HDMI_HDMI_PA_FRAME_CNT_get_hfvs_absence_frame_cnt(data)          HDMI_P0_HDMI_PA_FRAME_CNT_get_hfvs_absence_frame_cnt(data)
#define  HDMI_SC_ALIGN_B_lock_mask                                       HDMI_P0_SC_ALIGN_B_lock_mask
#define  HDMI_SC_ALIGN_B_en_mask                                         HDMI_P0_SC_ALIGN_B_en_mask
#define  HDMI_SC_ALIGN_B_sc_th_mask                                      HDMI_P0_SC_ALIGN_B_sc_th_mask
#define  HDMI_SC_ALIGN_B_pop_out_mask                                    HDMI_P0_SC_ALIGN_B_pop_out_mask
#define  HDMI_SC_ALIGN_B_choose_counter_mask                             HDMI_P0_SC_ALIGN_B_choose_counter_mask
#define  HDMI_SC_ALIGN_B_sc_counter_mask                                 HDMI_P0_SC_ALIGN_B_sc_counter_mask
#define  HDMI_SC_ALIGN_B_lock(data)                                      HDMI_P0_SC_ALIGN_B_lock(data)
#define  HDMI_SC_ALIGN_B_en(data)                                        HDMI_P0_SC_ALIGN_B_en(data)
#define  HDMI_SC_ALIGN_B_sc_th(data)                                     HDMI_P0_SC_ALIGN_B_sc_th(data)
#define  HDMI_SC_ALIGN_B_pop_out(data)                                   HDMI_P0_SC_ALIGN_B_pop_out(data)
#define  HDMI_SC_ALIGN_B_choose_counter(data)                            HDMI_P0_SC_ALIGN_B_choose_counter(data)
#define  HDMI_SC_ALIGN_B_sc_counter(data)                                HDMI_P0_SC_ALIGN_B_sc_counter(data)
#define  HDMI_SC_ALIGN_B_get_lock(data)                                  HDMI_P0_SC_ALIGN_B_get_lock(data)
#define  HDMI_SC_ALIGN_B_get_en(data)                                    HDMI_P0_SC_ALIGN_B_get_en(data)
#define  HDMI_SC_ALIGN_B_get_sc_th(data)                                 HDMI_P0_SC_ALIGN_B_get_sc_th(data)
#define  HDMI_SC_ALIGN_B_get_pop_out(data)                               HDMI_P0_SC_ALIGN_B_get_pop_out(data)
#define  HDMI_SC_ALIGN_B_get_choose_counter(data)                        HDMI_P0_SC_ALIGN_B_get_choose_counter(data)
#define  HDMI_SC_ALIGN_B_get_sc_counter(data)                            HDMI_P0_SC_ALIGN_B_get_sc_counter(data)
#define  HDMI_SC_ALIGN_R_lock_mask                                       HDMI_P0_SC_ALIGN_R_lock_mask
#define  HDMI_SC_ALIGN_R_en_mask                                         HDMI_P0_SC_ALIGN_R_en_mask
#define  HDMI_SC_ALIGN_R_sc_th_mask                                      HDMI_P0_SC_ALIGN_R_sc_th_mask
#define  HDMI_SC_ALIGN_R_pop_out_mask                                    HDMI_P0_SC_ALIGN_R_pop_out_mask
#define  HDMI_SC_ALIGN_R_choose_counter_mask                             HDMI_P0_SC_ALIGN_R_choose_counter_mask
#define  HDMI_SC_ALIGN_R_sc_counter_mask                                 HDMI_P0_SC_ALIGN_R_sc_counter_mask
#define  HDMI_SC_ALIGN_R_lock(data)                                      HDMI_P0_SC_ALIGN_R_lock(data)
#define  HDMI_SC_ALIGN_R_en(data)                                        HDMI_P0_SC_ALIGN_R_en(data)
#define  HDMI_SC_ALIGN_R_sc_th(data)                                     HDMI_P0_SC_ALIGN_R_sc_th(data)
#define  HDMI_SC_ALIGN_R_pop_out(data)                                   HDMI_P0_SC_ALIGN_R_pop_out(data)
#define  HDMI_SC_ALIGN_R_choose_counter(data)                            HDMI_P0_SC_ALIGN_R_choose_counter(data)
#define  HDMI_SC_ALIGN_R_sc_counter(data)                                HDMI_P0_SC_ALIGN_R_sc_counter(data)
#define  HDMI_SC_ALIGN_R_get_lock(data)                                  HDMI_P0_SC_ALIGN_R_get_lock(data)
#define  HDMI_SC_ALIGN_R_get_en(data)                                    HDMI_P0_SC_ALIGN_R_get_en(data)
#define  HDMI_SC_ALIGN_R_get_sc_th(data)                                 HDMI_P0_SC_ALIGN_R_get_sc_th(data)
#define  HDMI_SC_ALIGN_R_get_pop_out(data)                               HDMI_P0_SC_ALIGN_R_get_pop_out(data)
#define  HDMI_SC_ALIGN_R_get_choose_counter(data)                        HDMI_P0_SC_ALIGN_R_get_choose_counter(data)
#define  HDMI_SC_ALIGN_R_get_sc_counter(data)                            HDMI_P0_SC_ALIGN_R_get_sc_counter(data)
#define  HDMI_SCR_CTR_PER_LFSR_match_cnt_th_mask                         HDMI_P0_SCR_CTR_PER_LFSR_match_cnt_th_mask
#define  HDMI_SCR_CTR_PER_LFSR_ignore_cycle_cnt_mask                     HDMI_P0_SCR_CTR_PER_LFSR_ignore_cycle_cnt_mask
#define  HDMI_SCR_CTR_PER_LFSR_cmp_cycle_cnt_mask                        HDMI_P0_SCR_CTR_PER_LFSR_cmp_cycle_cnt_mask
#define  HDMI_SCR_CTR_PER_LFSR_fw_act_br_vs_win_opp_en_mask              HDMI_P0_SCR_CTR_PER_LFSR_fw_act_br_vs_win_opp_en_mask
#define  HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en_mask                     HDMI_P0_SCR_CTR_PER_LFSR_br_vs_win_opp_en_mask
#define  HDMI_SCR_CTR_PER_LFSR_rec_uscr_ctr_per_r_mask                   HDMI_P0_SCR_CTR_PER_LFSR_rec_uscr_ctr_per_r_mask
#define  HDMI_SCR_CTR_PER_LFSR_de_video_with_scr_ctr_cnt_ok_r_mask       HDMI_P0_SCR_CTR_PER_LFSR_de_video_with_scr_ctr_cnt_ok_r_mask
#define  HDMI_SCR_CTR_PER_LFSR_vs_win_opp_mask                           HDMI_P0_SCR_CTR_PER_LFSR_vs_win_opp_mask
#define  HDMI_SCR_CTR_PER_LFSR_vs_dec_des_mask                           HDMI_P0_SCR_CTR_PER_LFSR_vs_dec_des_mask
#define  HDMI_SCR_CTR_PER_LFSR_br_scr_align_done_mask                    HDMI_P0_SCR_CTR_PER_LFSR_br_scr_align_done_mask
#define  HDMI_SCR_CTR_PER_LFSR_align_diff_mask                           HDMI_P0_SCR_CTR_PER_LFSR_align_diff_mask
#define  HDMI_SCR_CTR_PER_LFSR_match_cnt_th(data)                        HDMI_P0_SCR_CTR_PER_LFSR_match_cnt_th(data)
#define  HDMI_SCR_CTR_PER_LFSR_ignore_cycle_cnt(data)                    HDMI_P0_SCR_CTR_PER_LFSR_ignore_cycle_cnt(data)
#define  HDMI_SCR_CTR_PER_LFSR_cmp_cycle_cnt(data)                       HDMI_P0_SCR_CTR_PER_LFSR_cmp_cycle_cnt(data)
#define  HDMI_SCR_CTR_PER_LFSR_fw_act_br_vs_win_opp_en(data)             HDMI_P0_SCR_CTR_PER_LFSR_fw_act_br_vs_win_opp_en(data)
#define  HDMI_SCR_CTR_PER_LFSR_br_vs_win_opp_en(data)                    HDMI_P0_SCR_CTR_PER_LFSR_br_vs_win_opp_en(data)
#define  HDMI_SCR_CTR_PER_LFSR_rec_uscr_ctr_per_r(data)                  HDMI_P0_SCR_CTR_PER_LFSR_rec_uscr_ctr_per_r(data)
#define  HDMI_SCR_CTR_PER_LFSR_de_video_with_scr_ctr_cnt_ok_r(data)      HDMI_P0_SCR_CTR_PER_LFSR_de_video_with_scr_ctr_cnt_ok_r(data)
#define  HDMI_SCR_CTR_PER_LFSR_vs_win_opp(data)                          HDMI_P0_SCR_CTR_PER_LFSR_vs_win_opp(data)
#define  HDMI_SCR_CTR_PER_LFSR_vs_dec_des(data)                          HDMI_P0_SCR_CTR_PER_LFSR_vs_dec_des(data)
#define  HDMI_SCR_CTR_PER_LFSR_br_scr_align_done(data)                   HDMI_P0_SCR_CTR_PER_LFSR_br_scr_align_done(data)
#define  HDMI_SCR_CTR_PER_LFSR_align_diff(data)                          HDMI_P0_SCR_CTR_PER_LFSR_align_diff(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_match_cnt_th(data)                    HDMI_P0_SCR_CTR_PER_LFSR_get_match_cnt_th(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_ignore_cycle_cnt(data)                HDMI_P0_SCR_CTR_PER_LFSR_get_ignore_cycle_cnt(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_cmp_cycle_cnt(data)                   HDMI_P0_SCR_CTR_PER_LFSR_get_cmp_cycle_cnt(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_fw_act_br_vs_win_opp_en(data)         HDMI_P0_SCR_CTR_PER_LFSR_get_fw_act_br_vs_win_opp_en(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_br_vs_win_opp_en(data)                HDMI_P0_SCR_CTR_PER_LFSR_get_br_vs_win_opp_en(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_rec_uscr_ctr_per_r(data)              HDMI_P0_SCR_CTR_PER_LFSR_get_rec_uscr_ctr_per_r(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_de_video_with_scr_ctr_cnt_ok_r(data)  HDMI_P0_SCR_CTR_PER_LFSR_get_de_video_with_scr_ctr_cnt_ok_r(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_vs_win_opp(data)                      HDMI_P0_SCR_CTR_PER_LFSR_get_vs_win_opp(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_vs_dec_des(data)                      HDMI_P0_SCR_CTR_PER_LFSR_get_vs_dec_des(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_br_scr_align_done(data)               HDMI_P0_SCR_CTR_PER_LFSR_get_br_scr_align_done(data)
#define  HDMI_SCR_CTR_PER_LFSR_get_align_diff(data)                      HDMI_P0_SCR_CTR_PER_LFSR_get_align_diff(data)
#define  HDMI_HDMI_EMAP_PP_apss_mask                                     HDMI_P0_HDMI_EMAP_PP_apss_mask
#define  HDMI_HDMI_EMAP_PP_apss(data)                                    HDMI_P0_HDMI_EMAP_PP_apss(data)
#define  HDMI_HDMI_EMAP_PP_get_apss(data)                                HDMI_P0_HDMI_EMAP_PP_get_apss(data)
#define  HDMI_HDMI_EMDP_PP_dpss_mask                                     HDMI_P0_HDMI_EMDP_PP_dpss_mask
#define  HDMI_HDMI_EMDP_PP_dpss(data)                                    HDMI_P0_HDMI_EMDP_PP_dpss(data)
#define  HDMI_HDMI_EMDP_PP_get_dpss(data)                                HDMI_P0_HDMI_EMDP_PP_get_dpss(data)
#define  HDMI_HDMI_SBTM_EMC_dummy1_mask                                  HDMI_P0_HDMI_SBTM_EMC_dummy1_mask
#define  HDMI_HDMI_SBTM_EMC_dummy2_mask                                  HDMI_P0_HDMI_SBTM_EMC_dummy2_mask
#define  HDMI_HDMI_SBTM_EMC_dummy3_mask                                  HDMI_P0_HDMI_SBTM_EMC_dummy3_mask
#define  HDMI_HDMI_SBTM_EMC_sbtm_clr_mask                                HDMI_P0_HDMI_SBTM_EMC_sbtm_clr_mask
#define  HDMI_HDMI_SBTM_EMC_rec_em_sbtm_irq_en_mask                      HDMI_P0_HDMI_SBTM_EMC_rec_em_sbtm_irq_en_mask
#define  HDMI_HDMI_SBTM_EMC_dummy1(data)                                 HDMI_P0_HDMI_SBTM_EMC_dummy1(data)
#define  HDMI_HDMI_SBTM_EMC_dummy2(data)                                 HDMI_P0_HDMI_SBTM_EMC_dummy2(data)
#define  HDMI_HDMI_SBTM_EMC_dummy3(data)                                 HDMI_P0_HDMI_SBTM_EMC_dummy3(data)
#define  HDMI_HDMI_SBTM_EMC_sbtm_clr(data)                               HDMI_P0_HDMI_SBTM_EMC_sbtm_clr(data)
#define  HDMI_HDMI_SBTM_EMC_rec_em_sbtm_irq_en(data)                     HDMI_P0_HDMI_SBTM_EMC_rec_em_sbtm_irq_en(data)
#define  HDMI_HDMI_SBTM_EMC_get_dummy1(data)                             HDMI_P0_HDMI_SBTM_EMC_get_dummy1(data)
#define  HDMI_HDMI_SBTM_EMC_get_dummy2(data)                             HDMI_P0_HDMI_SBTM_EMC_get_dummy2(data)
#define  HDMI_HDMI_SBTM_EMC_get_dummy3(data)                             HDMI_P0_HDMI_SBTM_EMC_get_dummy3(data)
#define  HDMI_HDMI_SBTM_EMC_get_sbtm_clr(data)                           HDMI_P0_HDMI_SBTM_EMC_get_sbtm_clr(data)
#define  HDMI_HDMI_SBTM_EMC_get_rec_em_sbtm_irq_en(data)                 HDMI_P0_HDMI_SBTM_EMC_get_rec_em_sbtm_irq_en(data)
#define  HDMI_SBTM_ST_em_sbtm_no_mask                                    HDMI_P0_SBTM_ST_em_sbtm_no_mask
#define  HDMI_SBTM_ST_em_sbtm_timeout_mask                               HDMI_P0_SBTM_ST_em_sbtm_timeout_mask
#define  HDMI_SBTM_ST_em_sbtm_discont_mask                               HDMI_P0_SBTM_ST_em_sbtm_discont_mask
#define  HDMI_SBTM_ST_em_sbtm_end_mask                                   HDMI_P0_SBTM_ST_em_sbtm_end_mask
#define  HDMI_SBTM_ST_em_sbtm_new_mask                                   HDMI_P0_SBTM_ST_em_sbtm_new_mask
#define  HDMI_SBTM_ST_em_sbtm_first_mask                                 HDMI_P0_SBTM_ST_em_sbtm_first_mask
#define  HDMI_SBTM_ST_em_sbtm_last_mask                                  HDMI_P0_SBTM_ST_em_sbtm_last_mask
#define  HDMI_SBTM_ST_em_sbtm_no(data)                                   HDMI_P0_SBTM_ST_em_sbtm_no(data)
#define  HDMI_SBTM_ST_em_sbtm_timeout(data)                              HDMI_P0_SBTM_ST_em_sbtm_timeout(data)
#define  HDMI_SBTM_ST_em_sbtm_discont(data)                              HDMI_P0_SBTM_ST_em_sbtm_discont(data)
#define  HDMI_SBTM_ST_em_sbtm_end(data)                                  HDMI_P0_SBTM_ST_em_sbtm_end(data)
#define  HDMI_SBTM_ST_em_sbtm_new(data)                                  HDMI_P0_SBTM_ST_em_sbtm_new(data)
#define  HDMI_SBTM_ST_em_sbtm_first(data)                                HDMI_P0_SBTM_ST_em_sbtm_first(data)
#define  HDMI_SBTM_ST_em_sbtm_last(data)                                 HDMI_P0_SBTM_ST_em_sbtm_last(data)
#define  HDMI_SBTM_ST_get_em_sbtm_no(data)                               HDMI_P0_SBTM_ST_get_em_sbtm_no(data)
#define  HDMI_SBTM_ST_get_em_sbtm_timeout(data)                          HDMI_P0_SBTM_ST_get_em_sbtm_timeout(data)
#define  HDMI_SBTM_ST_get_em_sbtm_discont(data)                          HDMI_P0_SBTM_ST_get_em_sbtm_discont(data)
#define  HDMI_SBTM_ST_get_em_sbtm_end(data)                              HDMI_P0_SBTM_ST_get_em_sbtm_end(data)
#define  HDMI_SBTM_ST_get_em_sbtm_new(data)                              HDMI_P0_SBTM_ST_get_em_sbtm_new(data)
#define  HDMI_SBTM_ST_get_em_sbtm_first(data)                            HDMI_P0_SBTM_ST_get_em_sbtm_first(data)
#define  HDMI_SBTM_ST_get_em_sbtm_last(data)                             HDMI_P0_SBTM_ST_get_em_sbtm_last(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_no_mask                        HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_no_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_timeout_mask                   HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_timeout_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_discont_mask                   HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_discont_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_end_mask                       HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_end_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_new_mask                       HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_new_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_first_mask                     HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_first_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_last_mask                      HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_last_mask
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_no(data)                       HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_no(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_timeout(data)                  HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_timeout(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_discont(data)                  HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_discont(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_end(data)                      HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_end(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_new(data)                      HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_new(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_first(data)                    HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_first(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_em_rsv0_last(data)                     HDMI_P0_HD20_RSV0_FOR_EM_ST_em_rsv0_last(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_no(data)                   HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_no(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_timeout(data)              HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_timeout(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_discont(data)              HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_discont(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_end(data)                  HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_end(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_new(data)                  HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_new(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_first(data)                HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_first(data)
#define  HDMI_HD20_RSV0_FOR_EM_ST_get_em_rsv0_last(data)                 HDMI_P0_HD20_RSV0_FOR_EM_ST_get_em_rsv0_last(data)
#define  HDMI_HD20_RSV0_EM_CT_org_id_mask                                HDMI_P0_HD20_RSV0_EM_CT_org_id_mask
#define  HDMI_HD20_RSV0_EM_CT_set_tag_msb_mask                           HDMI_P0_HD20_RSV0_EM_CT_set_tag_msb_mask
#define  HDMI_HD20_RSV0_EM_CT_set_tag_lsb_mask                           HDMI_P0_HD20_RSV0_EM_CT_set_tag_lsb_mask
#define  HDMI_HD20_RSV0_EM_CT_recognize_tag_en_mask                      HDMI_P0_HD20_RSV0_EM_CT_recognize_tag_en_mask
#define  HDMI_HD20_RSV0_EM_CT_rsv0_em_clr_mask                           HDMI_P0_HD20_RSV0_EM_CT_rsv0_em_clr_mask
#define  HDMI_HD20_RSV0_EM_CT_rec_em_rsv0_irq_en_mask                    HDMI_P0_HD20_RSV0_EM_CT_rec_em_rsv0_irq_en_mask
#define  HDMI_HD20_RSV0_EM_CT_org_id(data)                               HDMI_P0_HD20_RSV0_EM_CT_org_id(data)
#define  HDMI_HD20_RSV0_EM_CT_set_tag_msb(data)                          HDMI_P0_HD20_RSV0_EM_CT_set_tag_msb(data)
#define  HDMI_HD20_RSV0_EM_CT_set_tag_lsb(data)                          HDMI_P0_HD20_RSV0_EM_CT_set_tag_lsb(data)
#define  HDMI_HD20_RSV0_EM_CT_recognize_tag_en(data)                     HDMI_P0_HD20_RSV0_EM_CT_recognize_tag_en(data)
#define  HDMI_HD20_RSV0_EM_CT_rsv0_em_clr(data)                          HDMI_P0_HD20_RSV0_EM_CT_rsv0_em_clr(data)
#define  HDMI_HD20_RSV0_EM_CT_rec_em_rsv0_irq_en(data)                   HDMI_P0_HD20_RSV0_EM_CT_rec_em_rsv0_irq_en(data)
#define  HDMI_HD20_RSV0_EM_CT_get_org_id(data)                           HDMI_P0_HD20_RSV0_EM_CT_get_org_id(data)
#define  HDMI_HD20_RSV0_EM_CT_get_set_tag_msb(data)                      HDMI_P0_HD20_RSV0_EM_CT_get_set_tag_msb(data)
#define  HDMI_HD20_RSV0_EM_CT_get_set_tag_lsb(data)                      HDMI_P0_HD20_RSV0_EM_CT_get_set_tag_lsb(data)
#define  HDMI_HD20_RSV0_EM_CT_get_recognize_tag_en(data)                 HDMI_P0_HD20_RSV0_EM_CT_get_recognize_tag_en(data)
#define  HDMI_HD20_RSV0_EM_CT_get_rsv0_em_clr(data)                      HDMI_P0_HD20_RSV0_EM_CT_get_rsv0_em_clr(data)
#define  HDMI_HD20_RSV0_EM_CT_get_rec_em_rsv0_irq_en(data)               HDMI_P0_HD20_RSV0_EM_CT_get_rec_em_rsv0_irq_en(data)
#define  HDMI_HD20_RSV0_EM_CT2_sync_mask                                 HDMI_P0_HD20_RSV0_EM_CT2_sync_mask
#define  HDMI_HD20_RSV0_EM_CT2_recognize_oui_en_mask                     HDMI_P0_HD20_RSV0_EM_CT2_recognize_oui_en_mask
#define  HDMI_HD20_RSV0_EM_CT2_oui_3rd_mask                              HDMI_P0_HD20_RSV0_EM_CT2_oui_3rd_mask
#define  HDMI_HD20_RSV0_EM_CT2_oui_2nd_mask                              HDMI_P0_HD20_RSV0_EM_CT2_oui_2nd_mask
#define  HDMI_HD20_RSV0_EM_CT2_oui_1st_mask                              HDMI_P0_HD20_RSV0_EM_CT2_oui_1st_mask
#define  HDMI_HD20_RSV0_EM_CT2_sync(data)                                HDMI_P0_HD20_RSV0_EM_CT2_sync(data)
#define  HDMI_HD20_RSV0_EM_CT2_recognize_oui_en(data)                    HDMI_P0_HD20_RSV0_EM_CT2_recognize_oui_en(data)
#define  HDMI_HD20_RSV0_EM_CT2_oui_3rd(data)                             HDMI_P0_HD20_RSV0_EM_CT2_oui_3rd(data)
#define  HDMI_HD20_RSV0_EM_CT2_oui_2nd(data)                             HDMI_P0_HD20_RSV0_EM_CT2_oui_2nd(data)
#define  HDMI_HD20_RSV0_EM_CT2_oui_1st(data)                             HDMI_P0_HD20_RSV0_EM_CT2_oui_1st(data)
#define  HDMI_HD20_RSV0_EM_CT2_get_sync(data)                            HDMI_P0_HD20_RSV0_EM_CT2_get_sync(data)
#define  HDMI_HD20_RSV0_EM_CT2_get_recognize_oui_en(data)                HDMI_P0_HD20_RSV0_EM_CT2_get_recognize_oui_en(data)
#define  HDMI_HD20_RSV0_EM_CT2_get_oui_3rd(data)                         HDMI_P0_HD20_RSV0_EM_CT2_get_oui_3rd(data)
#define  HDMI_HD20_RSV0_EM_CT2_get_oui_2nd(data)                         HDMI_P0_HD20_RSV0_EM_CT2_get_oui_2nd(data)
#define  HDMI_HD20_RSV0_EM_CT2_get_oui_1st(data)                         HDMI_P0_HD20_RSV0_EM_CT2_get_oui_1st(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_no_mask                        HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_no_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_timeout_mask                   HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_timeout_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_discont_mask                   HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_discont_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_end_mask                       HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_end_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_new_mask                       HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_new_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_first_mask                     HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_first_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_last_mask                      HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_last_mask
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_no(data)                       HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_no(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_timeout(data)                  HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_timeout(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_discont(data)                  HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_discont(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_end(data)                      HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_end(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_new(data)                      HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_new(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_first(data)                    HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_first(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_em_rsv1_last(data)                     HDMI_P0_HD20_RSV1_FOR_EM_ST_em_rsv1_last(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_no(data)                   HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_no(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_timeout(data)              HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_timeout(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_discont(data)              HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_discont(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_end(data)                  HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_end(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_new(data)                  HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_new(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_first(data)                HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_first(data)
#define  HDMI_HD20_RSV1_FOR_EM_ST_get_em_rsv1_last(data)                 HDMI_P0_HD20_RSV1_FOR_EM_ST_get_em_rsv1_last(data)
#define  HDMI_HD20_RSV1_EM_CT_org_id_mask                                HDMI_P0_HD20_RSV1_EM_CT_org_id_mask
#define  HDMI_HD20_RSV1_EM_CT_set_tag_msb_mask                           HDMI_P0_HD20_RSV1_EM_CT_set_tag_msb_mask
#define  HDMI_HD20_RSV1_EM_CT_set_tag_lsb_mask                           HDMI_P0_HD20_RSV1_EM_CT_set_tag_lsb_mask
#define  HDMI_HD20_RSV1_EM_CT_recognize_tag_en_mask                      HDMI_P0_HD20_RSV1_EM_CT_recognize_tag_en_mask
#define  HDMI_HD20_RSV1_EM_CT_rsv1_em_clr_mask                           HDMI_P0_HD20_RSV1_EM_CT_rsv1_em_clr_mask
#define  HDMI_HD20_RSV1_EM_CT_rec_em_rsv1_irq_en_mask                    HDMI_P0_HD20_RSV1_EM_CT_rec_em_rsv1_irq_en_mask
#define  HDMI_HD20_RSV1_EM_CT_org_id(data)                               HDMI_P0_HD20_RSV1_EM_CT_org_id(data)
#define  HDMI_HD20_RSV1_EM_CT_set_tag_msb(data)                          HDMI_P0_HD20_RSV1_EM_CT_set_tag_msb(data)
#define  HDMI_HD20_RSV1_EM_CT_set_tag_lsb(data)                          HDMI_P0_HD20_RSV1_EM_CT_set_tag_lsb(data)
#define  HDMI_HD20_RSV1_EM_CT_recognize_tag_en(data)                     HDMI_P0_HD20_RSV1_EM_CT_recognize_tag_en(data)
#define  HDMI_HD20_RSV1_EM_CT_rsv1_em_clr(data)                          HDMI_P0_HD20_RSV1_EM_CT_rsv1_em_clr(data)
#define  HDMI_HD20_RSV1_EM_CT_rec_em_rsv1_irq_en(data)                   HDMI_P0_HD20_RSV1_EM_CT_rec_em_rsv1_irq_en(data)
#define  HDMI_HD20_RSV1_EM_CT_get_org_id(data)                           HDMI_P0_HD20_RSV1_EM_CT_get_org_id(data)
#define  HDMI_HD20_RSV1_EM_CT_get_set_tag_msb(data)                      HDMI_P0_HD20_RSV1_EM_CT_get_set_tag_msb(data)
#define  HDMI_HD20_RSV1_EM_CT_get_set_tag_lsb(data)                      HDMI_P0_HD20_RSV1_EM_CT_get_set_tag_lsb(data)
#define  HDMI_HD20_RSV1_EM_CT_get_recognize_tag_en(data)                 HDMI_P0_HD20_RSV1_EM_CT_get_recognize_tag_en(data)
#define  HDMI_HD20_RSV1_EM_CT_get_rsv1_em_clr(data)                      HDMI_P0_HD20_RSV1_EM_CT_get_rsv1_em_clr(data)
#define  HDMI_HD20_RSV1_EM_CT_get_rec_em_rsv1_irq_en(data)               HDMI_P0_HD20_RSV1_EM_CT_get_rec_em_rsv1_irq_en(data)
#define  HDMI_HD20_RSV1_EM_CT2_sync_mask                                 HDMI_P0_HD20_RSV1_EM_CT2_sync_mask
#define  HDMI_HD20_RSV1_EM_CT2_recognize_oui_en_mask                     HDMI_P0_HD20_RSV1_EM_CT2_recognize_oui_en_mask
#define  HDMI_HD20_RSV1_EM_CT2_oui_3rd_mask                              HDMI_P0_HD20_RSV1_EM_CT2_oui_3rd_mask
#define  HDMI_HD20_RSV1_EM_CT2_oui_2nd_mask                              HDMI_P0_HD20_RSV1_EM_CT2_oui_2nd_mask
#define  HDMI_HD20_RSV1_EM_CT2_oui_1st_mask                              HDMI_P0_HD20_RSV1_EM_CT2_oui_1st_mask
#define  HDMI_HD20_RSV1_EM_CT2_sync(data)                                HDMI_P0_HD20_RSV1_EM_CT2_sync(data)
#define  HDMI_HD20_RSV1_EM_CT2_recognize_oui_en(data)                    HDMI_P0_HD20_RSV1_EM_CT2_recognize_oui_en(data)
#define  HDMI_HD20_RSV1_EM_CT2_oui_3rd(data)                             HDMI_P0_HD20_RSV1_EM_CT2_oui_3rd(data)
#define  HDMI_HD20_RSV1_EM_CT2_oui_2nd(data)                             HDMI_P0_HD20_RSV1_EM_CT2_oui_2nd(data)
#define  HDMI_HD20_RSV1_EM_CT2_oui_1st(data)                             HDMI_P0_HD20_RSV1_EM_CT2_oui_1st(data)
#define  HDMI_HD20_RSV1_EM_CT2_get_sync(data)                            HDMI_P0_HD20_RSV1_EM_CT2_get_sync(data)
#define  HDMI_HD20_RSV1_EM_CT2_get_recognize_oui_en(data)                HDMI_P0_HD20_RSV1_EM_CT2_get_recognize_oui_en(data)
#define  HDMI_HD20_RSV1_EM_CT2_get_oui_3rd(data)                         HDMI_P0_HD20_RSV1_EM_CT2_get_oui_3rd(data)
#define  HDMI_HD20_RSV1_EM_CT2_get_oui_2nd(data)                         HDMI_P0_HD20_RSV1_EM_CT2_get_oui_2nd(data)
#define  HDMI_HD20_RSV1_EM_CT2_get_oui_1st(data)                         HDMI_P0_HD20_RSV1_EM_CT2_get_oui_1st(data)
#define  HDMI_HDMI_PTRSV4_pt3_oui_2st_mask                               HDMI_P0_HDMI_PTRSV4_pt3_oui_2st_mask
#define  HDMI_HDMI_PTRSV4_pt2_oui_2st_mask                               HDMI_P0_HDMI_PTRSV4_pt2_oui_2st_mask
#define  HDMI_HDMI_PTRSV4_pt1_oui_2st_mask                               HDMI_P0_HDMI_PTRSV4_pt1_oui_2st_mask
#define  HDMI_HDMI_PTRSV4_pt0_oui_2st_mask                               HDMI_P0_HDMI_PTRSV4_pt0_oui_2st_mask
#define  HDMI_HDMI_PTRSV4_pt3_oui_2st(data)                              HDMI_P0_HDMI_PTRSV4_pt3_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_pt2_oui_2st(data)                              HDMI_P0_HDMI_PTRSV4_pt2_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_pt1_oui_2st(data)                              HDMI_P0_HDMI_PTRSV4_pt1_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_pt0_oui_2st(data)                              HDMI_P0_HDMI_PTRSV4_pt0_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_get_pt3_oui_2st(data)                          HDMI_P0_HDMI_PTRSV4_get_pt3_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_get_pt2_oui_2st(data)                          HDMI_P0_HDMI_PTRSV4_get_pt2_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_get_pt1_oui_2st(data)                          HDMI_P0_HDMI_PTRSV4_get_pt1_oui_2st(data)
#define  HDMI_HDMI_PTRSV4_get_pt0_oui_2st(data)                          HDMI_P0_HDMI_PTRSV4_get_pt0_oui_2st(data)
#define  HDMI_HDMI_PTRSV5_pt3_oui_3st_mask                               HDMI_P0_HDMI_PTRSV5_pt3_oui_3st_mask
#define  HDMI_HDMI_PTRSV5_pt2_oui_3st_mask                               HDMI_P0_HDMI_PTRSV5_pt2_oui_3st_mask
#define  HDMI_HDMI_PTRSV5_pt1_oui_3st_mask                               HDMI_P0_HDMI_PTRSV5_pt1_oui_3st_mask
#define  HDMI_HDMI_PTRSV5_pt0_oui_3st_mask                               HDMI_P0_HDMI_PTRSV5_pt0_oui_3st_mask
#define  HDMI_HDMI_PTRSV5_pt3_oui_3st(data)                              HDMI_P0_HDMI_PTRSV5_pt3_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_pt2_oui_3st(data)                              HDMI_P0_HDMI_PTRSV5_pt2_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_pt1_oui_3st(data)                              HDMI_P0_HDMI_PTRSV5_pt1_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_pt0_oui_3st(data)                              HDMI_P0_HDMI_PTRSV5_pt0_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_get_pt3_oui_3st(data)                          HDMI_P0_HDMI_PTRSV5_get_pt3_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_get_pt2_oui_3st(data)                          HDMI_P0_HDMI_PTRSV5_get_pt2_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_get_pt1_oui_3st(data)                          HDMI_P0_HDMI_PTRSV5_get_pt1_oui_3st(data)
#define  HDMI_HDMI_PTRSV5_get_pt0_oui_3st(data)                          HDMI_P0_HDMI_PTRSV5_get_pt0_oui_3st(data)
#define  HDMI_HDMI_PTRSV6_pt3_recognize_oui23_en_mask                    HDMI_P0_HDMI_PTRSV6_pt3_recognize_oui23_en_mask
#define  HDMI_HDMI_PTRSV6_pt2_recognize_oui23_en_mask                    HDMI_P0_HDMI_PTRSV6_pt2_recognize_oui23_en_mask
#define  HDMI_HDMI_PTRSV6_pt1_recognize_oui23_en_mask                    HDMI_P0_HDMI_PTRSV6_pt1_recognize_oui23_en_mask
#define  HDMI_HDMI_PTRSV6_pt0_recognize_oui23_en_mask                    HDMI_P0_HDMI_PTRSV6_pt0_recognize_oui23_en_mask
#define  HDMI_HDMI_PTRSV6_pt3_recognize_oui23_en(data)                   HDMI_P0_HDMI_PTRSV6_pt3_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_pt2_recognize_oui23_en(data)                   HDMI_P0_HDMI_PTRSV6_pt2_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_pt1_recognize_oui23_en(data)                   HDMI_P0_HDMI_PTRSV6_pt1_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_pt0_recognize_oui23_en(data)                   HDMI_P0_HDMI_PTRSV6_pt0_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_get_pt3_recognize_oui23_en(data)               HDMI_P0_HDMI_PTRSV6_get_pt3_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_get_pt2_recognize_oui23_en(data)               HDMI_P0_HDMI_PTRSV6_get_pt2_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_get_pt1_recognize_oui23_en(data)               HDMI_P0_HDMI_PTRSV6_get_pt1_recognize_oui23_en(data)
#define  HDMI_HDMI_PTRSV6_get_pt0_recognize_oui23_en(data)               HDMI_P0_HDMI_PTRSV6_get_pt0_recognize_oui23_en(data)
#define  HDMI_HDCP_KEEPOUT_non_ctl_err_mask                              HDMI_P0_HDCP_KEEPOUT_non_ctl_err_mask
#define  HDMI_HDCP_KEEPOUT_rc_err_mask                                   HDMI_P0_HDCP_KEEPOUT_rc_err_mask
#define  HDMI_HDCP_KEEPOUT_keepout_window_end_mask                       HDMI_P0_HDCP_KEEPOUT_keepout_window_end_mask
#define  HDMI_HDCP_KEEPOUT_keepout_window_start_mask                     HDMI_P0_HDCP_KEEPOUT_keepout_window_start_mask
#define  HDMI_HDCP_KEEPOUT_non_ctl_err(data)                             HDMI_P0_HDCP_KEEPOUT_non_ctl_err(data)
#define  HDMI_HDCP_KEEPOUT_rc_err(data)                                  HDMI_P0_HDCP_KEEPOUT_rc_err(data)
#define  HDMI_HDCP_KEEPOUT_keepout_window_end(data)                      HDMI_P0_HDCP_KEEPOUT_keepout_window_end(data)
#define  HDMI_HDCP_KEEPOUT_keepout_window_start(data)                    HDMI_P0_HDCP_KEEPOUT_keepout_window_start(data)
#define  HDMI_HDCP_KEEPOUT_get_non_ctl_err(data)                         HDMI_P0_HDCP_KEEPOUT_get_non_ctl_err(data)
#define  HDMI_HDCP_KEEPOUT_get_rc_err(data)                              HDMI_P0_HDCP_KEEPOUT_get_rc_err(data)
#define  HDMI_HDCP_KEEPOUT_get_keepout_window_end(data)                  HDMI_P0_HDCP_KEEPOUT_get_keepout_window_end(data)
#define  HDMI_HDCP_KEEPOUT_get_keepout_window_start(data)                HDMI_P0_HDCP_KEEPOUT_get_keepout_window_start(data)


#define  HDMI_HDCP_CR_enc_window_end_mask                                HDMI_P0_HDCP_CR_enc_window_end_mask
#define  HDMI_HDCP_CR_enc_window_start_mask                              HDMI_P0_HDCP_CR_enc_window_start_mask
#define  HDMI_HDCP_CR_hdcp_clk_switch_auto_mask                          HDMI_P0_HDCP_CR_hdcp_clk_switch_auto_mask
#define  HDMI_HDCP_CR_hdcp_clk_sel_fw_mask                               HDMI_P0_HDCP_CR_hdcp_clk_sel_fw_mask
#define  HDMI_HDCP_CR_namfe_mask                                         HDMI_P0_HDCP_CR_namfe_mask
#define  HDMI_HDCP_CR_rpt_mask                                           HDMI_P0_HDCP_CR_rpt_mask
#define  HDMI_HDCP_CR_ivsp_mask                                          HDMI_P0_HDCP_CR_ivsp_mask
#define  HDMI_HDCP_CR_invvs_mask                                         HDMI_P0_HDCP_CR_invvs_mask
#define  HDMI_HDCP_CR_ivspm_mask                                         HDMI_P0_HDCP_CR_ivspm_mask
#define  HDMI_HDCP_CR_enc_window_end(data)                               HDMI_P0_HDCP_CR_enc_window_end(data)
#define  HDMI_HDCP_CR_enc_window_start(data)                             HDMI_P0_HDCP_CR_enc_window_start(data)
#define  HDMI_HDCP_CR_hdcp_clk_switch_auto(data)                         HDMI_P0_HDCP_CR_hdcp_clk_switch_auto(data)
#define  HDMI_HDCP_CR_hdcp_clk_sel_fw(data)                              HDMI_P0_HDCP_CR_hdcp_clk_sel_fw(data)
#define  HDMI_HDCP_CR_namfe(data)                                        HDMI_P0_HDCP_CR_namfe(data)
#define  HDMI_HDCP_CR_rpt(data)                                          HDMI_P0_HDCP_CR_rpt(data)
#define  HDMI_HDCP_CR_ivsp(data)                                         HDMI_P0_HDCP_CR_ivsp(data)
#define  HDMI_HDCP_CR_invvs(data)                                        HDMI_P0_HDCP_CR_invvs(data)
#define  HDMI_HDCP_CR_ivspm(data)                                        HDMI_P0_HDCP_CR_ivspm(data)
#define  HDMI_HDCP_CR_get_enc_window_end(data)                           HDMI_P0_HDCP_CR_get_enc_window_end(data)
#define  HDMI_HDCP_CR_get_enc_window_start(data)                         HDMI_P0_HDCP_CR_get_enc_window_start(data)
#define  HDMI_HDCP_CR_get_hdcp_clk_switch_auto(data)                     HDMI_P0_HDCP_CR_get_hdcp_clk_switch_auto(data)
#define  HDMI_HDCP_CR_get_hdcp_clk_sel_fw(data)                          HDMI_P0_HDCP_CR_get_hdcp_clk_sel_fw(data)
#define  HDMI_HDCP_CR_get_namfe(data)                                    HDMI_P0_HDCP_CR_get_namfe(data)
#define  HDMI_HDCP_CR_get_rpt(data)                                      HDMI_P0_HDCP_CR_get_rpt(data)
#define  HDMI_HDCP_CR_get_ivsp(data)                                     HDMI_P0_HDCP_CR_get_ivsp(data)
#define  HDMI_HDCP_CR_get_invvs(data)                                    HDMI_P0_HDCP_CR_get_invvs(data)
#define  HDMI_HDCP_CR_get_ivspm(data)                                    HDMI_P0_HDCP_CR_get_ivspm(data)


#define  HDMI_HDCP_CR2_enc_window_de2vs_mask                             HDMI_P0_HDCP_CR2_enc_window_de2vs_mask
#define  HDMI_HDCP_CR2_win_opp_mode_mask                                 HDMI_P0_HDCP_CR2_win_opp_mode_mask
#define  HDMI_HDCP_CR2_enc_window_de2vs(data)                            HDMI_P0_HDCP_CR2_enc_window_de2vs(data)
#define  HDMI_HDCP_CR2_win_opp_mode(data)                                HDMI_P0_HDCP_CR2_win_opp_mode(data)
#define  HDMI_HDCP_CR2_get_enc_window_de2vs(data)                        HDMI_P0_HDCP_CR2_get_enc_window_de2vs(data)
#define  HDMI_HDCP_CR2_get_win_opp_mode(data)                            HDMI_P0_HDCP_CR2_get_win_opp_mode(data)


#define  HDMI_HDCP_PCR_dummy_18_mask                                     HDMI_P0_HDCP_PCR_dummy_18_mask
#define  HDMI_HDCP_PCR_tune_up_down_mask                                 HDMI_P0_HDCP_PCR_tune_up_down_mask
#define  HDMI_HDCP_PCR_tune_range_mask                                   HDMI_P0_HDCP_PCR_tune_range_mask
#define  HDMI_HDCP_PCR_dvi_enc_mode_mask                                 HDMI_P0_HDCP_PCR_dvi_enc_mode_mask
#define  HDMI_HDCP_PCR_hdcp_vs_sel_mask                                  HDMI_P0_HDCP_PCR_hdcp_vs_sel_mask
#define  HDMI_HDCP_PCR_enc_tog_mask                                      HDMI_P0_HDCP_PCR_enc_tog_mask
#define  HDMI_HDCP_PCR_avmute_dis_mask                                   HDMI_P0_HDCP_PCR_avmute_dis_mask
#define  HDMI_HDCP_PCR_dummy_18(data)                                    HDMI_P0_HDCP_PCR_dummy_18(data)
#define  HDMI_HDCP_PCR_tune_up_down(data)                                HDMI_P0_HDCP_PCR_tune_up_down(data)
#define  HDMI_HDCP_PCR_tune_range(data)                                  HDMI_P0_HDCP_PCR_tune_range(data)
#define  HDMI_HDCP_PCR_dvi_enc_mode(data)                                HDMI_P0_HDCP_PCR_dvi_enc_mode(data)
#define  HDMI_HDCP_PCR_hdcp_vs_sel(data)                                 HDMI_P0_HDCP_PCR_hdcp_vs_sel(data)
#define  HDMI_HDCP_PCR_enc_tog(data)                                     HDMI_P0_HDCP_PCR_enc_tog(data)
#define  HDMI_HDCP_PCR_avmute_dis(data)                                  HDMI_P0_HDCP_PCR_avmute_dis(data)
#define  HDMI_HDCP_PCR_get_dummy_18(data)                                HDMI_P0_HDCP_PCR_get_dummy_18(data)
#define  HDMI_HDCP_PCR_get_tune_up_down(data)                            HDMI_P0_HDCP_PCR_get_tune_up_down(data)
#define  HDMI_HDCP_PCR_get_tune_range(data)                              HDMI_P0_HDCP_PCR_get_tune_range(data)
#define  HDMI_HDCP_PCR_get_dvi_enc_mode(data)                            HDMI_P0_HDCP_PCR_get_dvi_enc_mode(data)
#define  HDMI_HDCP_PCR_get_hdcp_vs_sel(data)                             HDMI_P0_HDCP_PCR_get_hdcp_vs_sel(data)
#define  HDMI_HDCP_PCR_get_enc_tog(data)                                 HDMI_P0_HDCP_PCR_get_enc_tog(data)
#define  HDMI_HDCP_PCR_get_avmute_dis(data)                              HDMI_P0_HDCP_PCR_get_avmute_dis(data)


#define  HDMI_HDCP_FLAG1_dummy_0_mask                                    HDMI_P0_HDCP_FLAG1_dummy_0_mask
#define  HDMI_HDCP_FLAG1_dummy_0(data)                                   HDMI_P0_HDCP_FLAG1_dummy_0(data)
#define  HDMI_HDCP_FLAG1_get_dummy_0(data)                               HDMI_P0_HDCP_FLAG1_get_dummy_0(data)
#define  HDMI_HDCP_FLAG2_win_opp_compare_num_mask                        HDMI_P0_HDCP_FLAG2_win_opp_compare_num_mask
#define  HDMI_HDCP_FLAG2_win_opp_diff_mask                               HDMI_P0_HDCP_FLAG2_win_opp_diff_mask
#define  HDMI_HDCP_FLAG2_frame_cnt_no_upd_mask                           HDMI_P0_HDCP_FLAG2_frame_cnt_no_upd_mask
#define  HDMI_HDCP_FLAG2_frame_cnt_err_upd_mask                          HDMI_P0_HDCP_FLAG2_frame_cnt_err_upd_mask
#define  HDMI_HDCP_FLAG2_dummy_0_mask                                    HDMI_P0_HDCP_FLAG2_dummy_0_mask
#define  HDMI_HDCP_FLAG2_win_opp_compare_num(data)                       HDMI_P0_HDCP_FLAG2_win_opp_compare_num(data)
#define  HDMI_HDCP_FLAG2_win_opp_diff(data)                              HDMI_P0_HDCP_FLAG2_win_opp_diff(data)
#define  HDMI_HDCP_FLAG2_frame_cnt_no_upd(data)                          HDMI_P0_HDCP_FLAG2_frame_cnt_no_upd(data)
#define  HDMI_HDCP_FLAG2_frame_cnt_err_upd(data)                         HDMI_P0_HDCP_FLAG2_frame_cnt_err_upd(data)
#define  HDMI_HDCP_FLAG2_dummy_0(data)                                   HDMI_P0_HDCP_FLAG2_dummy_0(data)
#define  HDMI_HDCP_FLAG2_get_win_opp_compare_num(data)                   HDMI_P0_HDCP_FLAG2_get_win_opp_compare_num(data)
#define  HDMI_HDCP_FLAG2_get_win_opp_diff(data)                          HDMI_P0_HDCP_FLAG2_get_win_opp_diff(data)
#define  HDMI_HDCP_FLAG2_get_frame_cnt_no_upd(data)                      HDMI_P0_HDCP_FLAG2_get_frame_cnt_no_upd(data)
#define  HDMI_HDCP_FLAG2_get_frame_cnt_err_upd(data)                     HDMI_P0_HDCP_FLAG2_get_frame_cnt_err_upd(data)
#define  HDMI_HDCP_FLAG2_get_dummy_0(data)                               HDMI_P0_HDCP_FLAG2_get_dummy_0(data)


#define  HDMI_HDCP_2p2_CR_dummy_31_29_mask                               HDMI_P0_HDCP_2p2_CR_dummy_31_29_mask
#define  HDMI_HDCP_2p2_CR_aes_engine_sel_mask                            HDMI_P0_HDCP_2p2_CR_aes_engine_sel_mask
#define  HDMI_HDCP_2p2_CR_fn_auto_inc_mask                               HDMI_P0_HDCP_2p2_CR_fn_auto_inc_mask
#define  HDMI_HDCP_2p2_CR_apply_cp_fn_mask                               HDMI_P0_HDCP_2p2_CR_apply_cp_fn_mask
#define  HDMI_HDCP_2p2_CR_ks_encoded_mask                                HDMI_P0_HDCP_2p2_CR_ks_encoded_mask
#define  HDMI_HDCP_2p2_CR_dummy_31_29(data)                              HDMI_P0_HDCP_2p2_CR_dummy_31_29(data)
#define  HDMI_HDCP_2p2_CR_aes_engine_sel(data)                           HDMI_P0_HDCP_2p2_CR_aes_engine_sel(data)
#define  HDMI_HDCP_2p2_CR_fn_auto_inc(data)                              HDMI_P0_HDCP_2p2_CR_fn_auto_inc(data)
#define  HDMI_HDCP_2p2_CR_apply_cp_fn(data)                              HDMI_P0_HDCP_2p2_CR_apply_cp_fn(data)
#define  HDMI_HDCP_2p2_CR_ks_encoded(data)                               HDMI_P0_HDCP_2p2_CR_ks_encoded(data)
#define  HDMI_HDCP_2p2_CR_get_dummy_31_29(data)                          HDMI_P0_HDCP_2p2_CR_get_dummy_31_29(data)
#define  HDMI_HDCP_2p2_CR_get_aes_engine_sel(data)                       HDMI_P0_HDCP_2p2_CR_get_aes_engine_sel(data)
#define  HDMI_HDCP_2p2_CR_get_fn_auto_inc(data)                          HDMI_P0_HDCP_2p2_CR_get_fn_auto_inc(data)
#define  HDMI_HDCP_2p2_CR_get_apply_cp_fn(data)                          HDMI_P0_HDCP_2p2_CR_get_apply_cp_fn(data)
#define  HDMI_HDCP_2p2_CR_get_ks_encoded(data)                           HDMI_P0_HDCP_2p2_CR_get_ks_encoded(data)


#define  HDMI_HDCP_2p2_ks0_ks_mask                                       HDMI_P0_HDCP_2p2_ks0_ks_mask
#define  HDMI_HDCP_2p2_ks0_ks(data)                                      HDMI_P0_HDCP_2p2_ks0_ks(data)
#define  HDMI_HDCP_2p2_ks0_get_ks(data)                                  HDMI_P0_HDCP_2p2_ks0_get_ks(data)


#define  HDMI_HDCP_2p2_ks1_ks_mask                                       HDMI_P0_HDCP_2p2_ks1_ks_mask
#define  HDMI_HDCP_2p2_ks1_ks(data)                                      HDMI_P0_HDCP_2p2_ks1_ks(data)
#define  HDMI_HDCP_2p2_ks1_get_ks(data)                                  HDMI_P0_HDCP_2p2_ks1_get_ks(data)


#define  HDMI_HDCP_2p2_ks2_ks_mask                                       HDMI_P0_HDCP_2p2_ks2_ks_mask
#define  HDMI_HDCP_2p2_ks2_ks(data)                                      HDMI_P0_HDCP_2p2_ks2_ks(data)
#define  HDMI_HDCP_2p2_ks2_get_ks(data)                                  HDMI_P0_HDCP_2p2_ks2_get_ks(data)


#define  HDMI_HDCP_2p2_ks3_ks_mask                                       HDMI_P0_HDCP_2p2_ks3_ks_mask
#define  HDMI_HDCP_2p2_ks3_ks(data)                                      HDMI_P0_HDCP_2p2_ks3_ks(data)
#define  HDMI_HDCP_2p2_ks3_get_ks(data)                                  HDMI_P0_HDCP_2p2_ks3_get_ks(data)


#define  HDMI_HDCP_2p2_lc0_lc_mask                                       HDMI_P0_HDCP_2p2_lc0_lc_mask
#define  HDMI_HDCP_2p2_lc0_lc(data)                                      HDMI_P0_HDCP_2p2_lc0_lc(data)
#define  HDMI_HDCP_2p2_lc0_get_lc(data)                                  HDMI_P0_HDCP_2p2_lc0_get_lc(data)


#define  HDMI_HDCP_2p2_lc1_lc_mask                                       HDMI_P0_HDCP_2p2_lc1_lc_mask
#define  HDMI_HDCP_2p2_lc1_lc(data)                                      HDMI_P0_HDCP_2p2_lc1_lc(data)
#define  HDMI_HDCP_2p2_lc1_get_lc(data)                                  HDMI_P0_HDCP_2p2_lc1_get_lc(data)


#define  HDMI_HDCP_2p2_lc2_lc_mask                                       HDMI_P0_HDCP_2p2_lc2_lc_mask
#define  HDMI_HDCP_2p2_lc2_lc(data)                                      HDMI_P0_HDCP_2p2_lc2_lc(data)
#define  HDMI_HDCP_2p2_lc2_get_lc(data)                                  HDMI_P0_HDCP_2p2_lc2_get_lc(data)


#define  HDMI_HDCP_2p2_lc3_lc_mask                                       HDMI_P0_HDCP_2p2_lc3_lc_mask
#define  HDMI_HDCP_2p2_lc3_lc(data)                                      HDMI_P0_HDCP_2p2_lc3_lc(data)
#define  HDMI_HDCP_2p2_lc3_get_lc(data)                                  HDMI_P0_HDCP_2p2_lc3_get_lc(data)


#define  HDMI_HDCP_2p2_riv0_riv_mask                                     HDMI_P0_HDCP_2p2_riv0_riv_mask
#define  HDMI_HDCP_2p2_riv0_riv(data)                                    HDMI_P0_HDCP_2p2_riv0_riv(data)
#define  HDMI_HDCP_2p2_riv0_get_riv(data)                                HDMI_P0_HDCP_2p2_riv0_get_riv(data)


#define  HDMI_HDCP_2p2_riv1_riv_mask                                     HDMI_P0_HDCP_2p2_riv1_riv_mask
#define  HDMI_HDCP_2p2_riv1_riv(data)                                    HDMI_P0_HDCP_2p2_riv1_riv(data)
#define  HDMI_HDCP_2p2_riv1_get_riv(data)                                HDMI_P0_HDCP_2p2_riv1_get_riv(data)


#define  HDMI_HDCP_2p2_SR0_frame_number_mask                             HDMI_P0_HDCP_2p2_SR0_frame_number_mask
#define  HDMI_HDCP_2p2_SR0_irq_fn_compare_fail_mask                      HDMI_P0_HDCP_2p2_SR0_irq_fn_compare_fail_mask
#define  HDMI_HDCP_2p2_SR0_irq_crc_fail_mask                             HDMI_P0_HDCP_2p2_SR0_irq_crc_fail_mask
#define  HDMI_HDCP_2p2_SR0_irq_no_cp_packet_mask                         HDMI_P0_HDCP_2p2_SR0_irq_no_cp_packet_mask
#define  HDMI_HDCP_2p2_SR0_aes_uf_mask                                   HDMI_P0_HDCP_2p2_SR0_aes_uf_mask
#define  HDMI_HDCP_2p2_SR0_frame_number(data)                            HDMI_P0_HDCP_2p2_SR0_frame_number(data)
#define  HDMI_HDCP_2p2_SR0_irq_fn_compare_fail(data)                     HDMI_P0_HDCP_2p2_SR0_irq_fn_compare_fail(data)
#define  HDMI_HDCP_2p2_SR0_irq_crc_fail(data)                            HDMI_P0_HDCP_2p2_SR0_irq_crc_fail(data)
#define  HDMI_HDCP_2p2_SR0_irq_no_cp_packet(data)                        HDMI_P0_HDCP_2p2_SR0_irq_no_cp_packet(data)
#define  HDMI_HDCP_2p2_SR0_aes_uf(data)                                  HDMI_P0_HDCP_2p2_SR0_aes_uf(data)
#define  HDMI_HDCP_2p2_SR0_get_frame_number(data)                        HDMI_P0_HDCP_2p2_SR0_get_frame_number(data)
#define  HDMI_HDCP_2p2_SR0_get_irq_fn_compare_fail(data)                 HDMI_P0_HDCP_2p2_SR0_get_irq_fn_compare_fail(data)
#define  HDMI_HDCP_2p2_SR0_get_irq_crc_fail(data)                        HDMI_P0_HDCP_2p2_SR0_get_irq_crc_fail(data)
#define  HDMI_HDCP_2p2_SR0_get_irq_no_cp_packet(data)                    HDMI_P0_HDCP_2p2_SR0_get_irq_no_cp_packet(data)
#define  HDMI_HDCP_2p2_SR0_get_aes_uf(data)                              HDMI_P0_HDCP_2p2_SR0_get_aes_uf(data)


#define  HDMI_HDCP_2p2_SR1_bch_err2_cnt_mask                             HDMI_P0_HDCP_2p2_SR1_bch_err2_cnt_mask
#define  HDMI_HDCP_2p2_SR1_irq_fn_compare_fail_en_mask                   HDMI_P0_HDCP_2p2_SR1_irq_fn_compare_fail_en_mask
#define  HDMI_HDCP_2p2_SR1_irq_crc_fail_en_mask                          HDMI_P0_HDCP_2p2_SR1_irq_crc_fail_en_mask
#define  HDMI_HDCP_2p2_SR1_irq_no_cp_packet_en_mask                      HDMI_P0_HDCP_2p2_SR1_irq_no_cp_packet_en_mask
#define  HDMI_HDCP_2p2_SR1_bch_err2_cnt(data)                            HDMI_P0_HDCP_2p2_SR1_bch_err2_cnt(data)
#define  HDMI_HDCP_2p2_SR1_irq_fn_compare_fail_en(data)                  HDMI_P0_HDCP_2p2_SR1_irq_fn_compare_fail_en(data)
#define  HDMI_HDCP_2p2_SR1_irq_crc_fail_en(data)                         HDMI_P0_HDCP_2p2_SR1_irq_crc_fail_en(data)
#define  HDMI_HDCP_2p2_SR1_irq_no_cp_packet_en(data)                     HDMI_P0_HDCP_2p2_SR1_irq_no_cp_packet_en(data)
#define  HDMI_HDCP_2p2_SR1_get_bch_err2_cnt(data)                        HDMI_P0_HDCP_2p2_SR1_get_bch_err2_cnt(data)
#define  HDMI_HDCP_2p2_SR1_get_irq_fn_compare_fail_en(data)              HDMI_P0_HDCP_2p2_SR1_get_irq_fn_compare_fail_en(data)
#define  HDMI_HDCP_2p2_SR1_get_irq_crc_fail_en(data)                     HDMI_P0_HDCP_2p2_SR1_get_irq_crc_fail_en(data)
#define  HDMI_HDCP_2p2_SR1_get_irq_no_cp_packet_en(data)                 HDMI_P0_HDCP_2p2_SR1_get_irq_no_cp_packet_en(data)


#define  HDMI_HDCP_HOA_enc_method_mask                                   HDMI_P0_HDCP_HOA_enc_method_mask
#define  HDMI_HDCP_HOA_fc_mask                                           HDMI_P0_HDCP_HOA_fc_mask
#define  HDMI_HDCP_HOA_enc_method(data)                                  HDMI_P0_HDCP_HOA_enc_method(data)
#define  HDMI_HDCP_HOA_fc(data)                                          HDMI_P0_HDCP_HOA_fc(data)
#define  HDMI_HDCP_HOA_get_enc_method(data)                              HDMI_P0_HDCP_HOA_get_enc_method(data)
#define  HDMI_HDCP_HOA_get_fc(data)                                      HDMI_P0_HDCP_HOA_get_fc(data)

#define  HDMI_HDCP_POWER_SAVING_CTRL_win_opp_all_mode_mask               HDMI_P0_HDCP_POWER_SAVING_CTRL_win_opp_all_mode_mask
#define  HDMI_HDCP_POWER_SAVING_CTRL_opp_blk_vfront_min_mask             HDMI_P0_HDCP_POWER_SAVING_CTRL_opp_blk_vfront_min_mask
#define  HDMI_HDCP_POWER_SAVING_CTRL_win_opp_all_mode(data)              HDMI_P0_HDCP_POWER_SAVING_CTRL_win_opp_all_mode(data)
#define  HDMI_HDCP_POWER_SAVING_CTRL_opp_blk_vfront_min(data)            HDMI_P0_HDCP_POWER_SAVING_CTRL_opp_blk_vfront_min(data)
#define  HDMI_HDCP_POWER_SAVING_CTRL_get_win_opp_all_mode(data)          HDMI_P0_HDCP_POWER_SAVING_CTRL_get_win_opp_all_mode(data)
#define  HDMI_HDCP_POWER_SAVING_CTRL_get_opp_blk_vfront_min(data)        HDMI_P0_HDCP_POWER_SAVING_CTRL_get_opp_blk_vfront_min(data)

#define  HDMI_WIN_OPP_CTRL_ctrl_r_only_sel_mask                          HDMI_P0_WIN_OPP_CTRL_ctrl_r_only_sel_mask
#define  HDMI_WIN_OPP_CTRL_opp_vs_debounce_en_mask                       HDMI_P0_WIN_OPP_CTRL_opp_vs_debounce_en_mask
#define  HDMI_WIN_OPP_CTRL_opp_vs_debounce_num_mask                      HDMI_P0_WIN_OPP_CTRL_opp_vs_debounce_num_mask
#define  HDMI_WIN_OPP_CTRL_ctrl_r_only_sel(data)                         HDMI_P0_WIN_OPP_CTRL_ctrl_r_only_sel(data)
#define  HDMI_WIN_OPP_CTRL_opp_vs_debounce_en(data)                      HDMI_P0_WIN_OPP_CTRL_opp_vs_debounce_en(data)
#define  HDMI_WIN_OPP_CTRL_opp_vs_debounce_num(data)                     HDMI_P0_WIN_OPP_CTRL_opp_vs_debounce_num(data)
#define  HDMI_WIN_OPP_CTRL_get_ctrl_r_only_sel(data)                     HDMI_P0_WIN_OPP_CTRL_get_ctrl_r_only_sel(data)
#define  HDMI_WIN_OPP_CTRL_get_opp_vs_debounce_en(data)                  HDMI_P0_WIN_OPP_CTRL_get_opp_vs_debounce_en(data)
#define  HDMI_WIN_OPP_CTRL_get_opp_vs_debounce_num(data)                 HDMI_P0_WIN_OPP_CTRL_get_opp_vs_debounce_num(data)

#define  HDMI_HDMI_VPLLCR0_dpll_reg_mask                                 HDMI_P0_HDMI_VPLLCR0_dpll_reg_mask
#define  HDMI_HDMI_VPLLCR0_dpll_m_mask                                   HDMI_P0_HDMI_VPLLCR0_dpll_m_mask
#define  HDMI_HDMI_VPLLCR0_dpll_reser_mask                               HDMI_P0_HDMI_VPLLCR0_dpll_reser_mask
#define  HDMI_HDMI_VPLLCR0_dpll_bpsin_mask                               HDMI_P0_HDMI_VPLLCR0_dpll_bpsin_mask
#define  HDMI_HDMI_VPLLCR0_dpll_o_mask                                   HDMI_P0_HDMI_VPLLCR0_dpll_o_mask
#define  HDMI_HDMI_VPLLCR0_dpll_n_mask                                   HDMI_P0_HDMI_VPLLCR0_dpll_n_mask
#define  HDMI_HDMI_VPLLCR0_dpll_rs_mask                                  HDMI_P0_HDMI_VPLLCR0_dpll_rs_mask
#define  HDMI_HDMI_VPLLCR0_dpll_ip_mask                                  HDMI_P0_HDMI_VPLLCR0_dpll_ip_mask
#define  HDMI_HDMI_VPLLCR0_dpll_reg(data)                                HDMI_P0_HDMI_VPLLCR0_dpll_reg(data)
#define  HDMI_HDMI_VPLLCR0_dpll_m(data)                                  HDMI_P0_HDMI_VPLLCR0_dpll_m(data)
#define  HDMI_HDMI_VPLLCR0_dpll_reser(data)                              HDMI_P0_HDMI_VPLLCR0_dpll_reser(data)
#define  HDMI_HDMI_VPLLCR0_dpll_bpsin(data)                              HDMI_P0_HDMI_VPLLCR0_dpll_bpsin(data)
#define  HDMI_HDMI_VPLLCR0_dpll_o(data)                                  HDMI_P0_HDMI_VPLLCR0_dpll_o(data)
#define  HDMI_HDMI_VPLLCR0_dpll_n(data)                                  HDMI_P0_HDMI_VPLLCR0_dpll_n(data)
#define  HDMI_HDMI_VPLLCR0_dpll_rs(data)                                 HDMI_P0_HDMI_VPLLCR0_dpll_rs(data)
#define  HDMI_HDMI_VPLLCR0_dpll_ip(data)                                 HDMI_P0_HDMI_VPLLCR0_dpll_ip(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_reg(data)                            HDMI_P0_HDMI_VPLLCR0_get_dpll_reg(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_m(data)                              HDMI_P0_HDMI_VPLLCR0_get_dpll_m(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_reser(data)                          HDMI_P0_HDMI_VPLLCR0_get_dpll_reser(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_bpsin(data)                          HDMI_P0_HDMI_VPLLCR0_get_dpll_bpsin(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_o(data)                              HDMI_P0_HDMI_VPLLCR0_get_dpll_o(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_n(data)                              HDMI_P0_HDMI_VPLLCR0_get_dpll_n(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_rs(data)                             HDMI_P0_HDMI_VPLLCR0_get_dpll_rs(data)
#define  HDMI_HDMI_VPLLCR0_get_dpll_ip(data)                             HDMI_P0_HDMI_VPLLCR0_get_dpll_ip(data)


#define  HDMI_HDMI_VPLLCR1_dpll_cp_mask                                  HDMI_P0_HDMI_VPLLCR1_dpll_cp_mask
#define  HDMI_HDMI_VPLLCR1_dpll_seltst_mask                              HDMI_P0_HDMI_VPLLCR1_dpll_seltst_mask
#define  HDMI_HDMI_VPLLCR1_dpll_vrefsel_mask                             HDMI_P0_HDMI_VPLLCR1_dpll_vrefsel_mask
#define  HDMI_HDMI_VPLLCR1_dpll_freeze_dsc_mask                          HDMI_P0_HDMI_VPLLCR1_dpll_freeze_dsc_mask
#define  HDMI_HDMI_VPLLCR1_dpll_ldo_pow_mask                             HDMI_P0_HDMI_VPLLCR1_dpll_ldo_pow_mask
#define  HDMI_HDMI_VPLLCR1_dpll_ldo_sel_mask                             HDMI_P0_HDMI_VPLLCR1_dpll_ldo_sel_mask
#define  HDMI_HDMI_VPLLCR1_dpll_wdrst_mask                               HDMI_P0_HDMI_VPLLCR1_dpll_wdrst_mask
#define  HDMI_HDMI_VPLLCR1_dpll_wdset_mask                               HDMI_P0_HDMI_VPLLCR1_dpll_wdset_mask
#define  HDMI_HDMI_VPLLCR1_dpll_rstb_mask                                HDMI_P0_HDMI_VPLLCR1_dpll_rstb_mask
#define  HDMI_HDMI_VPLLCR1_dpll_freeze_mask                              HDMI_P0_HDMI_VPLLCR1_dpll_freeze_mask
#define  HDMI_HDMI_VPLLCR1_dpll_pow_mask                                 HDMI_P0_HDMI_VPLLCR1_dpll_pow_mask
#define  HDMI_HDMI_VPLLCR1_dpll_cp(data)                                 HDMI_P0_HDMI_VPLLCR1_dpll_cp(data)
#define  HDMI_HDMI_VPLLCR1_dpll_seltst(data)                             HDMI_P0_HDMI_VPLLCR1_dpll_seltst(data)
#define  HDMI_HDMI_VPLLCR1_dpll_vrefsel(data)                            HDMI_P0_HDMI_VPLLCR1_dpll_vrefsel(data)
#define  HDMI_HDMI_VPLLCR1_dpll_freeze_dsc(data)                         HDMI_P0_HDMI_VPLLCR1_dpll_freeze_dsc(data)
#define  HDMI_HDMI_VPLLCR1_dpll_ldo_pow(data)                            HDMI_P0_HDMI_VPLLCR1_dpll_ldo_pow(data)
#define  HDMI_HDMI_VPLLCR1_dpll_ldo_sel(data)                            HDMI_P0_HDMI_VPLLCR1_dpll_ldo_sel(data)
#define  HDMI_HDMI_VPLLCR1_dpll_wdrst(data)                              HDMI_P0_HDMI_VPLLCR1_dpll_wdrst(data)
#define  HDMI_HDMI_VPLLCR1_dpll_wdset(data)                              HDMI_P0_HDMI_VPLLCR1_dpll_wdset(data)
#define  HDMI_HDMI_VPLLCR1_dpll_rstb(data)                               HDMI_P0_HDMI_VPLLCR1_dpll_rstb(data)
#define  HDMI_HDMI_VPLLCR1_dpll_freeze(data)                             HDMI_P0_HDMI_VPLLCR1_dpll_freeze(data)
#define  HDMI_HDMI_VPLLCR1_dpll_pow(data)                                HDMI_P0_HDMI_VPLLCR1_dpll_pow(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_cp(data)                             HDMI_P0_HDMI_VPLLCR1_get_dpll_cp(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_seltst(data)                         HDMI_P0_HDMI_VPLLCR1_get_dpll_seltst(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_vrefsel(data)                        HDMI_P0_HDMI_VPLLCR1_get_dpll_vrefsel(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_freeze_dsc(data)                     HDMI_P0_HDMI_VPLLCR1_get_dpll_freeze_dsc(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_ldo_pow(data)                        HDMI_P0_HDMI_VPLLCR1_get_dpll_ldo_pow(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_ldo_sel(data)                        HDMI_P0_HDMI_VPLLCR1_get_dpll_ldo_sel(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_wdrst(data)                          HDMI_P0_HDMI_VPLLCR1_get_dpll_wdrst(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_wdset(data)                          HDMI_P0_HDMI_VPLLCR1_get_dpll_wdset(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_rstb(data)                           HDMI_P0_HDMI_VPLLCR1_get_dpll_rstb(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_freeze(data)                         HDMI_P0_HDMI_VPLLCR1_get_dpll_freeze(data)
#define  HDMI_HDMI_VPLLCR1_get_dpll_pow(data)                            HDMI_P0_HDMI_VPLLCR1_get_dpll_pow(data)


#define  HDMI_HDMI_VPLLCR2_dpll_wdo_mask                                 HDMI_P0_HDMI_VPLLCR2_dpll_wdo_mask
#define  HDMI_HDMI_VPLLCR2_dpll_reloadm_mask                             HDMI_P0_HDMI_VPLLCR2_dpll_reloadm_mask
#define  HDMI_HDMI_VPLLCR2_dpll_wdo(data)                                HDMI_P0_HDMI_VPLLCR2_dpll_wdo(data)
#define  HDMI_HDMI_VPLLCR2_dpll_reloadm(data)                            HDMI_P0_HDMI_VPLLCR2_dpll_reloadm(data)
#define  HDMI_HDMI_VPLLCR2_get_dpll_wdo(data)                            HDMI_P0_HDMI_VPLLCR2_get_dpll_wdo(data)
#define  HDMI_HDMI_VPLLCR2_get_dpll_reloadm(data)                        HDMI_P0_HDMI_VPLLCR2_get_dpll_reloadm(data)


#define  HDMI_MN_SCLKG_CTRL0_sclkg_pll_in_sel_mask                       HDMI_P0_MN_SCLKG_CTRL0_sclkg_pll_in_sel_mask
#define  HDMI_MN_SCLKG_CTRL0_sclkg_dbuf_mask                             HDMI_P0_MN_SCLKG_CTRL0_sclkg_dbuf_mask
#define  HDMI_MN_SCLKG_CTRL0_dummy_3_0_mask                              HDMI_P0_MN_SCLKG_CTRL0_dummy_3_0_mask
#define  HDMI_MN_SCLKG_CTRL0_sclkg_pll_in_sel(data)                      HDMI_P0_MN_SCLKG_CTRL0_sclkg_pll_in_sel(data)
#define  HDMI_MN_SCLKG_CTRL0_sclkg_dbuf(data)                            HDMI_P0_MN_SCLKG_CTRL0_sclkg_dbuf(data)
#define  HDMI_MN_SCLKG_CTRL0_dummy_3_0(data)                             HDMI_P0_MN_SCLKG_CTRL0_dummy_3_0(data)
#define  HDMI_MN_SCLKG_CTRL0_get_sclkg_pll_in_sel(data)                  HDMI_P0_MN_SCLKG_CTRL0_get_sclkg_pll_in_sel(data)
#define  HDMI_MN_SCLKG_CTRL0_get_sclkg_dbuf(data)                        HDMI_P0_MN_SCLKG_CTRL0_get_sclkg_dbuf(data)
#define  HDMI_MN_SCLKG_CTRL0_get_dummy_3_0(data)                         HDMI_P0_MN_SCLKG_CTRL0_get_dummy_3_0(data)

#define  HDMI_MDD_CR_pending_mask                                        HDMI_P0_MDD_CR_pending_mask
#define  HDMI_MDD_CR_dummy_24_16_mask                                    HDMI_P0_MDD_CR_dummy_24_16_mask
#define  HDMI_MDD_CR_eco_hd21_timing_info_gb_cnt_mask                    HDMI_P0_MDD_CR_eco_hd21_timing_info_gb_cnt_mask
#define  HDMI_MDD_CR_no_md_check_mask                                    HDMI_P0_MDD_CR_no_md_check_mask
#define  HDMI_MDD_CR_metadata_version_check_mask                         HDMI_P0_MDD_CR_metadata_version_check_mask
#define  HDMI_MDD_CR_metadata_type_check_mask                            HDMI_P0_MDD_CR_metadata_type_check_mask
#define  HDMI_MDD_CR_eco_hd21_sbtm_mask                                  HDMI_P0_MDD_CR_eco_hd21_sbtm_mask
#define  HDMI_MDD_CR_det_con_enter_mask                                  HDMI_P0_MDD_CR_det_con_enter_mask
#define  HDMI_MDD_CR_eco_hd20_sbtm_mask                                  HDMI_P0_MDD_CR_eco_hd20_sbtm_mask
#define  HDMI_MDD_CR_det_con_leave_mask                                  HDMI_P0_MDD_CR_det_con_leave_mask
#define  HDMI_MDD_CR_irq_det_chg_en_mask                                 HDMI_P0_MDD_CR_irq_det_chg_en_mask
#define  HDMI_MDD_CR_dummy_2_1_mask                                      HDMI_P0_MDD_CR_dummy_2_1_mask
#define  HDMI_MDD_CR_en_mask                                             HDMI_P0_MDD_CR_en_mask
#define  HDMI_MDD_CR_pending(data)                                       HDMI_P0_MDD_CR_pending(data)
#define  HDMI_MDD_CR_dummy_24_16(data)                                   HDMI_P0_MDD_CR_dummy_24_16(data)
#define  HDMI_MDD_CR_eco_hd21_timing_info_gb_cnt(data)                   HDMI_P0_MDD_CR_eco_hd21_timing_info_gb_cnt(data)
#define  HDMI_MDD_CR_no_md_check(data)                                   HDMI_P0_MDD_CR_no_md_check(data)
#define  HDMI_MDD_CR_metadata_version_check(data)                        HDMI_P0_MDD_CR_metadata_version_check(data)
#define  HDMI_MDD_CR_metadata_type_check(data)                           HDMI_P0_MDD_CR_metadata_type_check(data)
#define  HDMI_MDD_CR_eco_hd21_sbtm(data)                                 HDMI_P0_MDD_CR_eco_hd21_sbtm(data)
#define  HDMI_MDD_CR_det_con_enter(data)                                 HDMI_P0_MDD_CR_det_con_enter(data)
#define  HDMI_MDD_CR_eco_hd20_sbtm(data)                                 HDMI_P0_MDD_CR_eco_hd20_sbtm(data)
#define  HDMI_MDD_CR_det_con_leave(data)                                 HDMI_P0_MDD_CR_det_con_leave(data)
#define  HDMI_MDD_CR_irq_det_chg_en(data)                                HDMI_P0_MDD_CR_irq_det_chg_en(data)
#define  HDMI_MDD_CR_dummy_2_1(data)                                     HDMI_P0_MDD_CR_dummy_2_1(data)
#define  HDMI_MDD_CR_en(data)                                            HDMI_P0_MDD_CR_en(data)
#define  HDMI_MDD_CR_get_pending(data)                                   HDMI_P0_MDD_CR_get_pending(data)
#define  HDMI_MDD_CR_get_dummy_24_16(data)                               HDMI_P0_MDD_CR_get_dummy_24_16(data)
#define  HDMI_MDD_CR_get_eco_hd21_timing_info_gb_cnt(data)               HDMI_P0_MDD_CR_get_eco_hd21_timing_info_gb_cnt(data)
#define  HDMI_MDD_CR_get_no_md_check(data)                               HDMI_P0_MDD_CR_get_no_md_check(data)
#define  HDMI_MDD_CR_get_metadata_version_check(data)                    HDMI_P0_MDD_CR_get_metadata_version_check(data)
#define  HDMI_MDD_CR_get_metadata_type_check(data)                       HDMI_P0_MDD_CR_get_metadata_type_check(data)
#define  HDMI_MDD_CR_get_eco_hd21_sbtm(data)                             HDMI_P0_MDD_CR_get_eco_hd21_sbtm(data)
#define  HDMI_MDD_CR_get_det_con_enter(data)                             HDMI_P0_MDD_CR_get_det_con_enter(data)
#define  HDMI_MDD_CR_get_eco_hd20_sbtm(data)                             HDMI_P0_MDD_CR_get_eco_hd20_sbtm(data)
#define  HDMI_MDD_CR_get_det_con_leave(data)                             HDMI_P0_MDD_CR_get_det_con_leave(data)
#define  HDMI_MDD_CR_get_irq_det_chg_en(data)                            HDMI_P0_MDD_CR_get_irq_det_chg_en(data)
#define  HDMI_MDD_CR_get_dummy_2_1(data)                                 HDMI_P0_MDD_CR_get_dummy_2_1(data)
#define  HDMI_MDD_CR_get_en(data)                                        HDMI_P0_MDD_CR_get_en(data)


#define  HDMI_MDD_SR_frame_crc_result_mask                               HDMI_P0_MDD_SR_frame_crc_result_mask
#define  HDMI_MDD_SR_det_result_chg_mask                                 HDMI_P0_MDD_SR_det_result_chg_mask
#define  HDMI_MDD_SR_det_result_mask                                     HDMI_P0_MDD_SR_det_result_mask
#define  HDMI_MDD_SR_frame_crc_result(data)                              HDMI_P0_MDD_SR_frame_crc_result(data)
#define  HDMI_MDD_SR_det_result_chg(data)                                HDMI_P0_MDD_SR_det_result_chg(data)
#define  HDMI_MDD_SR_det_result(data)                                    HDMI_P0_MDD_SR_det_result(data)
#define  HDMI_MDD_SR_get_frame_crc_result(data)                          HDMI_P0_MDD_SR_get_frame_crc_result(data)
#define  HDMI_MDD_SR_get_det_result_chg(data)                            HDMI_P0_MDD_SR_get_det_result_chg(data)
#define  HDMI_MDD_SR_get_det_result(data)                                HDMI_P0_MDD_SR_get_det_result(data)


#define  HDMI_FW_FUNC_application_mask                                   HDMI_P0_FW_FUNC_application_mask
#define  HDMI_FW_FUNC_application(data)                                  HDMI_P0_FW_FUNC_application(data)
#define  HDMI_FW_FUNC_get_application(data)                              HDMI_P0_FW_FUNC_get_application(data)


#define  HDMI_PORT_SWITCH_offms_hd21_sel_mask                            HDMI_P0_PORT_SWITCH_offms_hd21_sel_mask
#define  HDMI_PORT_SWITCH_port_sel_mask                                  HDMI_P0_PORT_SWITCH_port_sel_mask
#define  HDMI_PORT_SWITCH_offms_port_sel_mask                            HDMI_P0_PORT_SWITCH_offms_port_sel_mask
#define  HDMI_PORT_SWITCH_offms_hd21_sel(data)                           HDMI_P0_PORT_SWITCH_offms_hd21_sel(data)
#define  HDMI_PORT_SWITCH_port_sel(data)                                 HDMI_P0_PORT_SWITCH_port_sel(data)
#define  HDMI_PORT_SWITCH_offms_port_sel(data)                           HDMI_P0_PORT_SWITCH_offms_port_sel(data)
#define  HDMI_PORT_SWITCH_get_offms_hd21_sel(data)                       HDMI_P0_PORT_SWITCH_get_offms_hd21_sel(data)
#define  HDMI_PORT_SWITCH_get_port_sel(data)                             HDMI_P0_PORT_SWITCH_get_port_sel(data)
#define  HDMI_PORT_SWITCH_get_offms_port_sel(data)                       HDMI_P0_PORT_SWITCH_get_offms_port_sel(data)


#define  HDMI_POWER_SAVING_ptg_clken_mask                                HDMI_P0_POWER_SAVING_ptg_clken_mask
#define  HDMI_POWER_SAVING_ptg_clken(data)                               HDMI_P0_POWER_SAVING_ptg_clken(data)
#define  HDMI_POWER_SAVING_get_ptg_clken(data)                           HDMI_P0_POWER_SAVING_get_ptg_clken(data)


#define  HDMI_CRC_Ctrl_crc_res_sel_mask                                  HDMI_P0_CRC_Ctrl_crc_res_sel_mask
#define  HDMI_CRC_Ctrl_crc_auto_cmp_en_mask                              HDMI_P0_CRC_Ctrl_crc_auto_cmp_en_mask
#define  HDMI_CRC_Ctrl_crc_conti_mask                                    HDMI_P0_CRC_Ctrl_crc_conti_mask
#define  HDMI_CRC_Ctrl_crc_start_mask                                    HDMI_P0_CRC_Ctrl_crc_start_mask
#define  HDMI_CRC_Ctrl_crc_res_sel(data)                                 HDMI_P0_CRC_Ctrl_crc_res_sel(data)
#define  HDMI_CRC_Ctrl_crc_auto_cmp_en(data)                             HDMI_P0_CRC_Ctrl_crc_auto_cmp_en(data)
#define  HDMI_CRC_Ctrl_crc_conti(data)                                   HDMI_P0_CRC_Ctrl_crc_conti(data)
#define  HDMI_CRC_Ctrl_crc_start(data)                                   HDMI_P0_CRC_Ctrl_crc_start(data)
#define  HDMI_CRC_Ctrl_get_crc_res_sel(data)                             HDMI_P0_CRC_Ctrl_get_crc_res_sel(data)
#define  HDMI_CRC_Ctrl_get_crc_auto_cmp_en(data)                         HDMI_P0_CRC_Ctrl_get_crc_auto_cmp_en(data)
#define  HDMI_CRC_Ctrl_get_crc_conti(data)                               HDMI_P0_CRC_Ctrl_get_crc_conti(data)
#define  HDMI_CRC_Ctrl_get_crc_start(data)                               HDMI_P0_CRC_Ctrl_get_crc_start(data)


#define  HDMI_CRC_Result_crc_result_mask                                 HDMI_P0_CRC_Result_crc_result_mask
#define  HDMI_CRC_Result_crc_result(data)                                HDMI_P0_CRC_Result_crc_result(data)
#define  HDMI_CRC_Result_get_crc_result(data)                            HDMI_P0_CRC_Result_get_crc_result(data)


#define  HDMI_DES_CRC_crc_des_mask                                       HDMI_P0_DES_CRC_crc_des_mask
#define  HDMI_DES_CRC_crc_des(data)                                      HDMI_P0_DES_CRC_crc_des(data)
#define  HDMI_DES_CRC_get_crc_des(data)                                  HDMI_P0_DES_CRC_get_crc_des(data)


#define  HDMI_CRC_ERR_CNT0_crc_err_cnt_mask                              HDMI_P0_CRC_ERR_CNT0_crc_err_cnt_mask
#define  HDMI_CRC_ERR_CNT0_crc_err_cnt(data)                             HDMI_P0_CRC_ERR_CNT0_crc_err_cnt(data)
#define  HDMI_CRC_ERR_CNT0_get_crc_err_cnt(data)                         HDMI_P0_CRC_ERR_CNT0_get_crc_err_cnt(data)


#define  HDMI_hdmi_20_21_ctrl_common_irq_keep_mask                       HDMI_P0_hdmi_20_21_ctrl_common_irq_keep_mask
#define  HDMI_hdmi_20_21_ctrl_end_phase_mask                             HDMI_P0_hdmi_20_21_ctrl_end_phase_mask
#define  HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2_mask                    HDMI_P0_hdmi_20_21_ctrl_out_pixel_clk_div2_mask
#define  HDMI_hdmi_20_21_ctrl_ho_mask                                    HDMI_P0_hdmi_20_21_ctrl_ho_mask
#define  HDMI_hdmi_20_21_ctrl_yo_mask                                    HDMI_P0_hdmi_20_21_ctrl_yo_mask
#define  HDMI_hdmi_20_21_ctrl_pixel_mode_mask                            HDMI_P0_hdmi_20_21_ctrl_pixel_mode_mask
#define  HDMI_hdmi_20_21_ctrl_common_irq_keep(data)                      HDMI_P0_hdmi_20_21_ctrl_common_irq_keep(data)
#define  HDMI_hdmi_20_21_ctrl_end_phase(data)                            HDMI_P0_hdmi_20_21_ctrl_end_phase(data)
#define  HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2(data)                   HDMI_P0_hdmi_20_21_ctrl_out_pixel_clk_div2(data)
#define  HDMI_hdmi_20_21_ctrl_ho(data)                                   HDMI_P0_hdmi_20_21_ctrl_ho(data)
#define  HDMI_hdmi_20_21_ctrl_yo(data)                                   HDMI_P0_hdmi_20_21_ctrl_yo(data)
#define  HDMI_hdmi_20_21_ctrl_pixel_mode(data)                           HDMI_P0_hdmi_20_21_ctrl_pixel_mode(data)
#define  HDMI_hdmi_20_21_ctrl_get_common_irq_keep(data)                  HDMI_P0_hdmi_20_21_ctrl_get_common_irq_keep(data)
#define  HDMI_hdmi_20_21_ctrl_get_end_phase(data)                        HDMI_P0_hdmi_20_21_ctrl_get_end_phase(data)
#define  HDMI_hdmi_20_21_ctrl_get_out_pixel_clk_div2(data)               HDMI_P0_hdmi_20_21_ctrl_get_out_pixel_clk_div2(data)
#define  HDMI_hdmi_20_21_ctrl_get_ho(data)                               HDMI_P0_hdmi_20_21_ctrl_get_ho(data)
#define  HDMI_hdmi_20_21_ctrl_get_yo(data)                               HDMI_P0_hdmi_20_21_ctrl_get_yo(data)
#define  HDMI_hdmi_20_21_ctrl_get_pixel_mode(data)                       HDMI_P0_hdmi_20_21_ctrl_get_pixel_mode(data)

#define  HDMI_YUV422_CD_frame_cnt_thr_mask                               HDMI_P0_YUV422_CD_frame_cnt_thr_mask
#define  HDMI_YUV422_CD_pixel_cnt_thr_mask                               HDMI_P0_YUV422_CD_pixel_cnt_thr_mask
#define  HDMI_YUV422_CD_det_12b_flag_mask                                HDMI_P0_YUV422_CD_det_12b_flag_mask
#define  HDMI_YUV422_CD_det_10b_flag_mask                                HDMI_P0_YUV422_CD_det_10b_flag_mask
#define  HDMI_YUV422_CD_det_en_mask                                      HDMI_P0_YUV422_CD_det_en_mask
#define  HDMI_YUV422_CD_frame_cnt_thr(data)                              HDMI_P0_YUV422_CD_frame_cnt_thr(data)
#define  HDMI_YUV422_CD_pixel_cnt_thr(data)                              HDMI_P0_YUV422_CD_pixel_cnt_thr(data)
#define  HDMI_YUV422_CD_det_12b_flag(data)                               HDMI_P0_YUV422_CD_det_12b_flag(data)
#define  HDMI_YUV422_CD_det_10b_flag(data)                               HDMI_P0_YUV422_CD_det_10b_flag(data)
#define  HDMI_YUV422_CD_det_en(data)                                     HDMI_P0_YUV422_CD_det_en(data)
#define  HDMI_YUV422_CD_get_frame_cnt_thr(data)                          HDMI_P0_YUV422_CD_get_frame_cnt_thr(data)
#define  HDMI_YUV422_CD_get_pixel_cnt_thr(data)                          HDMI_P0_YUV422_CD_get_pixel_cnt_thr(data)
#define  HDMI_YUV422_CD_get_det_12b_flag(data)                           HDMI_P0_YUV422_CD_get_det_12b_flag(data)
#define  HDMI_YUV422_CD_get_det_10b_flag(data)                           HDMI_P0_YUV422_CD_get_det_10b_flag(data)
#define  HDMI_YUV422_CD_get_det_en(data)                                 HDMI_P0_YUV422_CD_get_det_en(data)

#define  HDMI_MN_SCLKG_CTRL1_sclkg_oclk_sel_mask                         HDMI_P0_MN_SCLKG_CTRL1_sclkg_oclk_sel_mask
#define  HDMI_MN_SCLKG_CTRL1_dummy_4_0_mask                              HDMI_P0_MN_SCLKG_CTRL1_dummy_4_0_mask
#define  HDMI_MN_SCLKG_CTRL1_sclkg_oclk_sel(data)                        HDMI_P0_MN_SCLKG_CTRL1_sclkg_oclk_sel(data)
#define  HDMI_MN_SCLKG_CTRL1_dummy_4_0(data)                             HDMI_P0_MN_SCLKG_CTRL1_dummy_4_0(data)
#define  HDMI_MN_SCLKG_CTRL1_get_sclkg_oclk_sel(data)                    HDMI_P0_MN_SCLKG_CTRL1_get_sclkg_oclk_sel(data)
#define  HDMI_MN_SCLKG_CTRL1_get_dummy_4_0(data)                         HDMI_P0_MN_SCLKG_CTRL1_get_dummy_4_0(data)

#define  HDMI_MN_SCLKG_DIVS1_sclkg_pll_div2_en_mask                      HDMI_P0_MN_SCLKG_DIVS1_sclkg_pll_div2_en_mask
#define  HDMI_MN_SCLKG_DIVS1_sclkg_pll_divs_mask                         HDMI_P0_MN_SCLKG_DIVS1_sclkg_pll_divs_mask
#define  HDMI_MN_SCLKG_DIVS1_sclkg_pll_div2_en(data)                     HDMI_P0_MN_SCLKG_DIVS1_sclkg_pll_div2_en(data)
#define  HDMI_MN_SCLKG_DIVS1_sclkg_pll_divs(data)                        HDMI_P0_MN_SCLKG_DIVS1_sclkg_pll_divs(data)
#define  HDMI_MN_SCLKG_DIVS1_get_sclkg_pll_div2_en(data)                 HDMI_P0_MN_SCLKG_DIVS1_get_sclkg_pll_div2_en(data)
#define  HDMI_MN_SCLKG_DIVS1_get_sclkg_pll_divs(data)                    HDMI_P0_MN_SCLKG_DIVS1_get_sclkg_pll_divs(data)

#define  HDMI_ot_p3_scdc_sel_mask                                        HDMI_P0_ot_p3_scdc_sel_mask
#define  HDMI_ot_p2_scdc_sel_mask                                        HDMI_P0_ot_p2_scdc_sel_mask
#define  HDMI_ot_p1_scdc_sel_mask                                        HDMI_P0_ot_p1_scdc_sel_mask
#define  HDMI_ot_p0_scdc_sel_mask                                        HDMI_P0_ot_p0_scdc_sel_mask
#define  HDMI_ot_p3_scdc_sel(data)                                       HDMI_P0_ot_p3_scdc_sel(data)
#define  HDMI_ot_p2_scdc_sel(data)                                       HDMI_P0_ot_p2_scdc_sel(data)
#define  HDMI_ot_p1_scdc_sel(data)                                       HDMI_P0_ot_p1_scdc_sel(data)
#define  HDMI_ot_p0_scdc_sel(data)                                       HDMI_P0_ot_p0_scdc_sel(data)
#define  HDMI_ot_get_p3_scdc_sel(data)                                   HDMI_P0_ot_get_p3_scdc_sel(data)
#define  HDMI_ot_get_p2_scdc_sel(data)                                   HDMI_P0_ot_get_p2_scdc_sel(data)
#define  HDMI_ot_get_p1_scdc_sel(data)                                   HDMI_P0_ot_get_p1_scdc_sel(data)
#define  HDMI_ot_get_p0_scdc_sel(data)                                   HDMI_P0_ot_get_p0_scdc_sel(data)
#define  HDMI_IRQ_auto_gen_pulse_mode_mask                               HDMI_P0_IRQ_auto_gen_pulse_mode_mask
#define  HDMI_IRQ_auto_gen_pulse_mode(data)                              HDMI_P0_IRQ_auto_gen_pulse_mode(data)
#define  HDMI_IRQ_get_auto_gen_pulse_mode(data)                          HDMI_P0_IRQ_get_auto_gen_pulse_mode(data)


#define  HDMI_mp_bist_mode_mask                                          HDMI_P0_mp_bist_mode_mask
#define  HDMI_mp_bist_mode(data)                                         HDMI_P0_mp_bist_mode(data)
#define  HDMI_mp_get_bist_mode(data)                                     HDMI_P0_mp_get_bist_mode(data)


#define  HDMI_hdmi_v_bypass_fw_mode_mask                                 HDMI_P0_hdmi_v_bypass_fw_mode_mask
#define  HDMI_hdmi_v_bypass_data_mask                                    HDMI_P0_hdmi_v_bypass_data_mask
#define  HDMI_hdmi_v_bypass_reg_hd21_eco_option_mask                     HDMI_P0_hdmi_v_bypass_reg_hd21_eco_option_mask
#define  HDMI_hdmi_v_bypass_fw_mode(data)                                HDMI_P0_hdmi_v_bypass_fw_mode(data)
#define  HDMI_hdmi_v_bypass_data(data)                                   HDMI_P0_hdmi_v_bypass_data(data)
#define  HDMI_hdmi_v_bypass_reg_hd21_eco_option(data)                    HDMI_P0_hdmi_v_bypass_reg_hd21_eco_option(data)
#define  HDMI_hdmi_v_bypass_get_fw_mode(data)                            HDMI_P0_hdmi_v_bypass_get_fw_mode(data)
#define  HDMI_hdmi_v_bypass_get_data(data)                               HDMI_P0_hdmi_v_bypass_get_data(data)
#define  HDMI_hdmi_v_bypass_get_reg_hd21_eco_option(data)                HDMI_P0_hdmi_v_bypass_get_reg_hd21_eco_option(data)
#define  HDMI_hdmi_mp_common_mp_check_and_mask                           HDMI_P0_hdmi_mp_common_mp_check_and_mask
#define  HDMI_hdmi_mp_common_mp_check_or_mask                            HDMI_P0_hdmi_mp_common_mp_check_or_mask
#define  HDMI_hdmi_mp_p3_mp_check_and_mask                               HDMI_P0_hdmi_mp_p3_mp_check_and_mask
#define  HDMI_hdmi_mp_p3_mp_check_or_mask                                HDMI_P0_hdmi_mp_p3_mp_check_or_mask
#define  HDMI_hdmi_mp_p2_mp_check_and_mask                               HDMI_P0_hdmi_mp_p2_mp_check_and_mask
#define  HDMI_hdmi_mp_p2_mp_check_or_mask                                HDMI_P0_hdmi_mp_p2_mp_check_or_mask
#define  HDMI_hdmi_mp_p1_mp_check_and_mask                               HDMI_P0_hdmi_mp_p1_mp_check_and_mask
#define  HDMI_hdmi_mp_p1_mp_check_or_mask                                HDMI_P0_hdmi_mp_p1_mp_check_or_mask
#define  HDMI_hdmi_mp_mp_data_out_mask                                   HDMI_P0_hdmi_mp_mp_data_out_mask
#define  HDMI_hdmi_mp_mp_check_and_mask                                  HDMI_P0_hdmi_mp_mp_check_and_mask
#define  HDMI_hdmi_mp_mp_check_or_mask                                   HDMI_P0_hdmi_mp_mp_check_or_mask
#define  HDMI_hdmi_mp_mp_mode_mask                                       HDMI_P0_hdmi_mp_mp_mode_mask
#define  HDMI_hdmi_mp_common_mp_check_and(data)                          HDMI_P0_hdmi_mp_common_mp_check_and(data)
#define  HDMI_hdmi_mp_common_mp_check_or(data)                           HDMI_P0_hdmi_mp_common_mp_check_or(data)
#define  HDMI_hdmi_mp_p3_mp_check_and(data)                              HDMI_P0_hdmi_mp_p3_mp_check_and(data)
#define  HDMI_hdmi_mp_p3_mp_check_or(data)                               HDMI_P0_hdmi_mp_p3_mp_check_or(data)
#define  HDMI_hdmi_mp_p2_mp_check_and(data)                              HDMI_P0_hdmi_mp_p2_mp_check_and(data)
#define  HDMI_hdmi_mp_p2_mp_check_or(data)                               HDMI_P0_hdmi_mp_p2_mp_check_or(data)
#define  HDMI_hdmi_mp_p1_mp_check_and(data)                              HDMI_P0_hdmi_mp_p1_mp_check_and(data)
#define  HDMI_hdmi_mp_p1_mp_check_or(data)                               HDMI_P0_hdmi_mp_p1_mp_check_or(data)
#define  HDMI_hdmi_mp_mp_data_out(data)                                  HDMI_P0_hdmi_mp_mp_data_out(data)
#define  HDMI_hdmi_mp_mp_check_and(data)                                 HDMI_P0_hdmi_mp_mp_check_and(data)
#define  HDMI_hdmi_mp_mp_check_or(data)                                  HDMI_P0_hdmi_mp_mp_check_or(data)
#define  HDMI_hdmi_mp_mp_mode(data)                                      HDMI_P0_hdmi_mp_mp_mode(data)
#define  HDMI_hdmi_mp_get_common_mp_check_and(data)                      HDMI_P0_hdmi_mp_get_common_mp_check_and(data)
#define  HDMI_hdmi_mp_get_common_mp_check_or(data)                       HDMI_P0_hdmi_mp_get_common_mp_check_or(data)
#define  HDMI_hdmi_mp_get_p3_mp_check_and(data)                          HDMI_P0_hdmi_mp_get_p3_mp_check_and(data)
#define  HDMI_hdmi_mp_get_p3_mp_check_or(data)                           HDMI_P0_hdmi_mp_get_p3_mp_check_or(data)
#define  HDMI_hdmi_mp_get_p2_mp_check_and(data)                          HDMI_P0_hdmi_mp_get_p2_mp_check_and(data)
#define  HDMI_hdmi_mp_get_p2_mp_check_or(data)                           HDMI_P0_hdmi_mp_get_p2_mp_check_or(data)
#define  HDMI_hdmi_mp_get_p1_mp_check_and(data)                          HDMI_P0_hdmi_mp_get_p1_mp_check_and(data)
#define  HDMI_hdmi_mp_get_p1_mp_check_or(data)                           HDMI_P0_hdmi_mp_get_p1_mp_check_or(data)
#define  HDMI_hdmi_mp_get_mp_data_out(data)                              HDMI_P0_hdmi_mp_get_mp_data_out(data)
#define  HDMI_hdmi_mp_get_mp_check_and(data)                             HDMI_P0_hdmi_mp_get_mp_check_and(data)
#define  HDMI_hdmi_mp_get_mp_check_or(data)                              HDMI_P0_hdmi_mp_get_mp_check_or(data)
#define  HDMI_hdmi_mp_get_mp_mode(data)                                  HDMI_P0_hdmi_mp_get_mp_mode(data)


#define  HDMI_debug_port_otpin_mask                                      HDMI_P0_debug_port_otpin_mask
#define  HDMI_debug_port_otpin(data)                                     HDMI_P0_debug_port_otpin(data)
#define  HDMI_debug_port_get_otpin(data)                                 HDMI_P0_debug_port_get_otpin(data)
#define  HDMI_debug_port_ctrl_common_test_sel_mask                       HDMI_P0_debug_port_ctrl_common_test_sel_mask
#define  HDMI_debug_port_ctrl_hdmi_test_sel_mask                         HDMI_P0_debug_port_ctrl_hdmi_test_sel_mask
#define  HDMI_debug_port_ctrl_debug_port_sel_mask                        HDMI_P0_debug_port_ctrl_debug_port_sel_mask
#define  HDMI_debug_port_ctrl_common_test_sel(data)                      HDMI_P0_debug_port_ctrl_common_test_sel(data)
#define  HDMI_debug_port_ctrl_hdmi_test_sel(data)                        HDMI_P0_debug_port_ctrl_hdmi_test_sel(data)
#define  HDMI_debug_port_ctrl_debug_port_sel(data)                       HDMI_P0_debug_port_ctrl_debug_port_sel(data)
#define  HDMI_debug_port_ctrl_get_common_test_sel(data)                  HDMI_P0_debug_port_ctrl_get_common_test_sel(data)
#define  HDMI_debug_port_ctrl_get_hdmi_test_sel(data)                    HDMI_P0_debug_port_ctrl_get_hdmi_test_sel(data)
#define  HDMI_debug_port_ctrl_get_debug_port_sel(data)                   HDMI_P0_debug_port_ctrl_get_debug_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud_to_hdmitx_common_port_sel_mask  HDMI_P0_AUD_COMMON_PORT_SWITCH_aud_to_hdmitx_common_port_sel_mask
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud2_common_port_sel_mask           HDMI_P0_AUD_COMMON_PORT_SWITCH_aud2_common_port_sel_mask
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud_common_port_sel_mask            HDMI_P0_AUD_COMMON_PORT_SWITCH_aud_common_port_sel_mask
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud_to_hdmitx_common_port_sel(data) HDMI_P0_AUD_COMMON_PORT_SWITCH_aud_to_hdmitx_common_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud2_common_port_sel(data)          HDMI_P0_AUD_COMMON_PORT_SWITCH_aud2_common_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_aud_common_port_sel(data)           HDMI_P0_AUD_COMMON_PORT_SWITCH_aud_common_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_get_aud_to_hdmitx_common_port_sel(data) HDMI_P0_AUD_COMMON_PORT_SWITCH_get_aud_to_hdmitx_common_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_get_aud2_common_port_sel(data)      HDMI_P0_AUD_COMMON_PORT_SWITCH_get_aud2_common_port_sel(data)
#define  HDMI_AUD_COMMON_PORT_SWITCH_get_aud_common_port_sel(data)       HDMI_P0_AUD_COMMON_PORT_SWITCH_get_aud_common_port_sel(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_common_mask                            HDMI_P0_hdmi_rst0_rstn_hdmi_common_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_pre3_mask                              HDMI_P0_hdmi_rst0_rstn_hdmi_pre3_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_pre2_mask                              HDMI_P0_hdmi_rst0_rstn_hdmi_pre2_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_pre1_mask                              HDMI_P0_hdmi_rst0_rstn_hdmi_pre1_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_pre0_mask                              HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre3_mask                         HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre3_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre2_mask                         HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre2_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre1_mask                         HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre1_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask                         HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_common2_mask                           HDMI_P0_hdmi_rst0_rstn_hdmi_common2_mask
#define  HDMI_hdmi_rst0_rstn_hdmi_common(data)                           HDMI_P0_hdmi_rst0_rstn_hdmi_common(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_pre3(data)                             HDMI_P0_hdmi_rst0_rstn_hdmi_pre3(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_pre2(data)                             HDMI_P0_hdmi_rst0_rstn_hdmi_pre2(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_pre1(data)                             HDMI_P0_hdmi_rst0_rstn_hdmi_pre1(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_pre0(data)                             HDMI_P0_hdmi_rst0_rstn_hdmi_pre0(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre3(data)                        HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre3(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre2(data)                        HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre2(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre1(data)                        HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre1(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_hdcp_pre0(data)                        HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0(data)
#define  HDMI_hdmi_rst0_rstn_hdmi_common2(data)                          HDMI_P0_hdmi_rst0_rstn_hdmi_common2(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_common(data)                       HDMI_P0_hdmi_rst0_get_rstn_hdmi_common(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_pre3(data)                         HDMI_P0_hdmi_rst0_get_rstn_hdmi_pre3(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_pre2(data)                         HDMI_P0_hdmi_rst0_get_rstn_hdmi_pre2(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_pre1(data)                         HDMI_P0_hdmi_rst0_get_rstn_hdmi_pre1(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_pre0(data)                         HDMI_P0_hdmi_rst0_get_rstn_hdmi_pre0(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_hdcp_pre3(data)                    HDMI_P0_hdmi_rst0_get_rstn_hdmi_hdcp_pre3(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_hdcp_pre2(data)                    HDMI_P0_hdmi_rst0_get_rstn_hdmi_hdcp_pre2(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_hdcp_pre1(data)                    HDMI_P0_hdmi_rst0_get_rstn_hdmi_hdcp_pre1(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_hdcp_pre0(data)                    HDMI_P0_hdmi_rst0_get_rstn_hdmi_hdcp_pre0(data)
#define  HDMI_hdmi_rst0_get_rstn_hdmi_common2(data)                      HDMI_P0_hdmi_rst0_get_rstn_hdmi_common2(data)

#define  HDMI_hdmi_rst3_rstn_hd21_common_mask                            HDMI_P0_hdmi_rst3_rstn_hd21_common_mask
#define  HDMI_hdmi_rst3_rstn_hd21_pre3_mask                              HDMI_P0_hdmi_rst3_rstn_hd21_pre3_mask
#define  HDMI_hdmi_rst3_rstn_hd21_pre2_mask                              HDMI_P0_hdmi_rst3_rstn_hd21_pre2_mask
#define  HDMI_hdmi_rst3_rstn_hd21_pre1_mask                              HDMI_P0_hdmi_rst3_rstn_hd21_pre1_mask
#define  HDMI_hdmi_rst3_rstn_hd21_pre0_mask                              HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre3_mask                         HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre3_mask
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre2_mask                         HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre2_mask
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre1_mask                         HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre1_mask
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre0_mask                         HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask
#define  HDMI_hdmi_rst3_rstn_hd21_common2_mask                           HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask
#define  HDMI_hdmi_rst3_rstn_hd21_common(data)                           HDMI_P0_hdmi_rst3_rstn_hd21_common(data)
#define  HDMI_hdmi_rst3_rstn_hd21_pre3(data)                             HDMI_P0_hdmi_rst3_rstn_hd21_pre3(data)
#define  HDMI_hdmi_rst3_rstn_hd21_pre2(data)                             HDMI_P0_hdmi_rst3_rstn_hd21_pre2(data)
#define  HDMI_hdmi_rst3_rstn_hd21_pre1(data)                             HDMI_P0_hdmi_rst3_rstn_hd21_pre1(data)
#define  HDMI_hdmi_rst3_rstn_hd21_pre0(data)                             HDMI_P0_hdmi_rst3_rstn_hd21_pre0(data)
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre3(data)                        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre3(data)
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre2(data)                        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre2(data)
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre1(data)                        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre1(data)
#define  HDMI_hdmi_rst3_rstn_hd21_hdcp_pre0(data)                        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0(data)
#define  HDMI_hdmi_rst3_rstn_hd21_common2(data)                          HDMI_P0_hdmi_rst3_rstn_hd21_common2(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_common(data)                       HDMI_P0_hdmi_rst3_get_rstn_hd21_common(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_pre3(data)                         HDMI_P0_hdmi_rst3_get_rstn_hd21_pre3(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_pre2(data)                         HDMI_P0_hdmi_rst3_get_rstn_hd21_pre2(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_pre1(data)                         HDMI_P0_hdmi_rst3_get_rstn_hd21_pre1(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_pre0(data)                         HDMI_P0_hdmi_rst3_get_rstn_hd21_pre0(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_hdcp_pre3(data)                    HDMI_P0_hdmi_rst3_get_rstn_hd21_hdcp_pre3(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_hdcp_pre2(data)                    HDMI_P0_hdmi_rst3_get_rstn_hd21_hdcp_pre2(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_hdcp_pre1(data)                    HDMI_P0_hdmi_rst3_get_rstn_hd21_hdcp_pre1(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_hdcp_pre0(data)                    HDMI_P0_hdmi_rst3_get_rstn_hd21_hdcp_pre0(data)
#define  HDMI_hdmi_rst3_get_rstn_hd21_common2(data)                      HDMI_P0_hdmi_rst3_get_rstn_hd21_common2(data)

#define  HDMI_hdmi_clken0_clken_hdmi_common_mask                         HDMI_P0_hdmi_clken0_clken_hdmi_common_mask
#define  HDMI_hdmi_clken0_clken_hdmi_pre3_mask                           HDMI_P0_hdmi_clken0_clken_hdmi_pre3_mask
#define  HDMI_hdmi_clken0_clken_hdmi_pre2_mask                           HDMI_P0_hdmi_clken0_clken_hdmi_pre2_mask
#define  HDMI_hdmi_clken0_clken_hdmi_pre1_mask                           HDMI_P0_hdmi_clken0_clken_hdmi_pre1_mask
#define  HDMI_hdmi_clken0_clken_hdmi_pre0_mask                           HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre3_mask                      HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre3_mask
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre2_mask                      HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre2_mask
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre1_mask                      HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre1_mask
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre0_mask                      HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0_mask
#define  HDMI_hdmi_clken0_clken_hdmi_common2_mask                        HDMI_P0_hdmi_clken0_clken_hdmi_common2_mask
#define  HDMI_hdmi_clken0_clken_hdmi_common(data)                        HDMI_P0_hdmi_clken0_clken_hdmi_common(data)
#define  HDMI_hdmi_clken0_clken_hdmi_pre3(data)                          HDMI_P0_hdmi_clken0_clken_hdmi_pre3(data)
#define  HDMI_hdmi_clken0_clken_hdmi_pre2(data)                          HDMI_P0_hdmi_clken0_clken_hdmi_pre2(data)
#define  HDMI_hdmi_clken0_clken_hdmi_pre1(data)                          HDMI_P0_hdmi_clken0_clken_hdmi_pre1(data)
#define  HDMI_hdmi_clken0_clken_hdmi_pre0(data)                          HDMI_P0_hdmi_clken0_clken_hdmi_pre0(data)
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre3(data)                     HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre3(data)
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre2(data)                     HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre2(data)
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre1(data)                     HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre1(data)
#define  HDMI_hdmi_clken0_clken_hdmi_hdcp_pre0(data)                     HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0(data)
#define  HDMI_hdmi_clken0_clken_hdmi_common2(data)                       HDMI_P0_hdmi_clken0_clken_hdmi_common2(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_common(data)                    HDMI_P0_hdmi_clken0_get_clken_hdmi_common(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_pre3(data)                      HDMI_P0_hdmi_clken0_get_clken_hdmi_pre3(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_pre2(data)                      HDMI_P0_hdmi_clken0_get_clken_hdmi_pre2(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_pre1(data)                      HDMI_P0_hdmi_clken0_get_clken_hdmi_pre1(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_pre0(data)                      HDMI_P0_hdmi_clken0_get_clken_hdmi_pre0(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_hdcp_pre3(data)                 HDMI_P0_hdmi_clken0_get_clken_hdmi_hdcp_pre3(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_hdcp_pre2(data)                 HDMI_P0_hdmi_clken0_get_clken_hdmi_hdcp_pre2(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_hdcp_pre1(data)                 HDMI_P0_hdmi_clken0_get_clken_hdmi_hdcp_pre1(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_hdcp_pre0(data)                 HDMI_P0_hdmi_clken0_get_clken_hdmi_hdcp_pre0(data)
#define  HDMI_hdmi_clken0_get_clken_hdmi_common2(data)                   HDMI_P0_hdmi_clken0_get_clken_hdmi_common2(data)

#define  HDMI_hdmi_clken3_clken_hd21_common_mask                         HDMI_P0_hdmi_clken3_clken_hd21_common_mask
#define  HDMI_hdmi_clken3_clken_hd21_pre3_mask                           HDMI_P0_hdmi_clken3_clken_hd21_pre3_mask
#define  HDMI_hdmi_clken3_clken_hd21_pre2_mask                           HDMI_P0_hdmi_clken3_clken_hd21_pre2_mask
#define  HDMI_hdmi_clken3_clken_hd21_pre1_mask                           HDMI_P0_hdmi_clken3_clken_hd21_pre1_mask
#define  HDMI_hdmi_clken3_clken_hd21_pre0_mask                           HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre3_mask                      HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre3_mask
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre2_mask                      HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre2_mask
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre1_mask                      HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre1_mask
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre0_mask                      HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask
#define  HDMI_hdmi_clken3_clken_hd21_common2_mask                        HDMI_P0_hdmi_clken3_clken_hd21_common2_mask
#define  HDMI_hdmi_clken3_clken_hd21_common(data)                        HDMI_P0_hdmi_clken3_clken_hd21_common(data)
#define  HDMI_hdmi_clken3_clken_hd21_pre3(data)                          HDMI_P0_hdmi_clken3_clken_hd21_pre3(data)
#define  HDMI_hdmi_clken3_clken_hd21_pre2(data)                          HDMI_P0_hdmi_clken3_clken_hd21_pre2(data)
#define  HDMI_hdmi_clken3_clken_hd21_pre1(data)                          HDMI_P0_hdmi_clken3_clken_hd21_pre1(data)
#define  HDMI_hdmi_clken3_clken_hd21_pre0(data)                          HDMI_P0_hdmi_clken3_clken_hd21_pre0(data)
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre3(data)                     HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre3(data)
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre2(data)                     HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre2(data)
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre1(data)                     HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre1(data)
#define  HDMI_hdmi_clken3_clken_hd21_hdcp_pre0(data)                     HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0(data)
#define  HDMI_hdmi_clken3_clken_hd21_common2(data)                       HDMI_P0_hdmi_clken3_clken_hd21_common2(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_common(data)                    HDMI_P0_hdmi_clken3_get_clken_hd21_common(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_pre3(data)                      HDMI_P0_hdmi_clken3_get_clken_hd21_pre3(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_pre2(data)                      HDMI_P0_hdmi_clken3_get_clken_hd21_pre2(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_pre1(data)                      HDMI_P0_hdmi_clken3_get_clken_hd21_pre1(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_pre0(data)                      HDMI_P0_hdmi_clken3_get_clken_hd21_pre0(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_hdcp_pre3(data)                 HDMI_P0_hdmi_clken3_get_clken_hd21_hdcp_pre3(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_hdcp_pre2(data)                 HDMI_P0_hdmi_clken3_get_clken_hd21_hdcp_pre2(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_hdcp_pre1(data)                 HDMI_P0_hdmi_clken3_get_clken_hd21_hdcp_pre1(data)
#define  HDMI_hdmi_clken3_get_clken_hd21_hdcp_pre0(data)                 HDMI_P0_hdmi_clken3_get_clken_hd21_hdcp_pre0(data)

#define  HDMI_hdmi_clken3_get_clken_hd21_common2(data)                   HDMI_P0_hdmi_clken3_get_clken_hd21_common2(data)

#define  HDMI_BIST_ALL_SET1_linebuf_test_rnm_0_mask                      HDMI_P0_BIST_ALL_SET1_linebuf_test_rnm_0_mask
#define  HDMI_BIST_ALL_SET1_linebuf_wa_0_mask                            HDMI_P0_BIST_ALL_SET1_linebuf_wa_0_mask
#define  HDMI_BIST_ALL_SET1_linebuf_wpulse_0_mask                        HDMI_P0_BIST_ALL_SET1_linebuf_wpulse_0_mask
#define  HDMI_BIST_ALL_SET1_hdr_test_rnma_0_mask                         HDMI_P0_BIST_ALL_SET1_hdr_test_rnma_0_mask
#define  HDMI_BIST_ALL_SET1_hdr_test_rnma_1_mask                         HDMI_P0_BIST_ALL_SET1_hdr_test_rnma_1_mask
#define  HDMI_BIST_ALL_SET1_audio_test_rnm_0_mask                        HDMI_P0_BIST_ALL_SET1_audio_test_rnm_0_mask
#define  HDMI_BIST_ALL_SET1_audio_wa_0_mask                              HDMI_P0_BIST_ALL_SET1_audio_wa_0_mask
#define  HDMI_BIST_ALL_SET1_audio_wpulse_0_mask                          HDMI_P0_BIST_ALL_SET1_audio_wpulse_0_mask
#define  HDMI_BIST_ALL_SET1_linebuf_test_rnm_0(data)                     HDMI_P0_BIST_ALL_SET1_linebuf_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET1_linebuf_wa_0(data)                           HDMI_P0_BIST_ALL_SET1_linebuf_wa_0(data)
#define  HDMI_BIST_ALL_SET1_linebuf_wpulse_0(data)                       HDMI_P0_BIST_ALL_SET1_linebuf_wpulse_0(data)
#define  HDMI_BIST_ALL_SET1_hdr_test_rnma_0(data)                        HDMI_P0_BIST_ALL_SET1_hdr_test_rnma_0(data)
#define  HDMI_BIST_ALL_SET1_hdr_test_rnma_1(data)                        HDMI_P0_BIST_ALL_SET1_hdr_test_rnma_1(data)
#define  HDMI_BIST_ALL_SET1_audio_test_rnm_0(data)                       HDMI_P0_BIST_ALL_SET1_audio_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET1_audio_wa_0(data)                             HDMI_P0_BIST_ALL_SET1_audio_wa_0(data)
#define  HDMI_BIST_ALL_SET1_audio_wpulse_0(data)                         HDMI_P0_BIST_ALL_SET1_audio_wpulse_0(data)
#define  HDMI_BIST_ALL_SET1_get_linebuf_test_rnm_0(data)                 HDMI_P0_BIST_ALL_SET1_get_linebuf_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET1_get_linebuf_wa_0(data)                       HDMI_P0_BIST_ALL_SET1_get_linebuf_wa_0(data)
#define  HDMI_BIST_ALL_SET1_get_linebuf_wpulse_0(data)                   HDMI_P0_BIST_ALL_SET1_get_linebuf_wpulse_0(data)
#define  HDMI_BIST_ALL_SET1_get_hdr_test_rnma_0(data)                    HDMI_P0_BIST_ALL_SET1_get_hdr_test_rnma_0(data)
#define  HDMI_BIST_ALL_SET1_get_hdr_test_rnma_1(data)                    HDMI_P0_BIST_ALL_SET1_get_hdr_test_rnma_1(data)
#define  HDMI_BIST_ALL_SET1_get_audio_test_rnm_0(data)                   HDMI_P0_BIST_ALL_SET1_get_audio_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET1_get_audio_wa_0(data)                         HDMI_P0_BIST_ALL_SET1_get_audio_wa_0(data)
#define  HDMI_BIST_ALL_SET1_get_audio_wpulse_0(data)                     HDMI_P0_BIST_ALL_SET1_get_audio_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_0_mask                         HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_0_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_0_mask                          HDMI_P0_BIST_YUV420_SET1_yuv420_wa_0_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_0_mask                      HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_0_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_1_mask                         HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_1_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_1_mask                          HDMI_P0_BIST_YUV420_SET1_yuv420_wa_1_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_1_mask                      HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_1_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_2_mask                         HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_2_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_2_mask                          HDMI_P0_BIST_YUV420_SET1_yuv420_wa_2_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_2_mask                      HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_2_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_3_mask                         HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_3_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_3_mask                          HDMI_P0_BIST_YUV420_SET1_yuv420_wa_3_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_3_mask                      HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_3_mask
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_0(data)                        HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_0(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_0(data)                         HDMI_P0_BIST_YUV420_SET1_yuv420_wa_0(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_0(data)                     HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_1(data)                        HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_1(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_1(data)                         HDMI_P0_BIST_YUV420_SET1_yuv420_wa_1(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_1(data)                     HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_1(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_2(data)                        HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_2(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_2(data)                         HDMI_P0_BIST_YUV420_SET1_yuv420_wa_2(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_2(data)                     HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_2(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_rnm_3(data)                        HDMI_P0_BIST_YUV420_SET1_yuv420_rnm_3(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wa_3(data)                         HDMI_P0_BIST_YUV420_SET1_yuv420_wa_3(data)
#define  HDMI_BIST_YUV420_SET1_yuv420_wpulse_3(data)                     HDMI_P0_BIST_YUV420_SET1_yuv420_wpulse_3(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_rnm_0(data)                    HDMI_P0_BIST_YUV420_SET1_get_yuv420_rnm_0(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wa_0(data)                     HDMI_P0_BIST_YUV420_SET1_get_yuv420_wa_0(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wpulse_0(data)                 HDMI_P0_BIST_YUV420_SET1_get_yuv420_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_rnm_1(data)                    HDMI_P0_BIST_YUV420_SET1_get_yuv420_rnm_1(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wa_1(data)                     HDMI_P0_BIST_YUV420_SET1_get_yuv420_wa_1(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wpulse_1(data)                 HDMI_P0_BIST_YUV420_SET1_get_yuv420_wpulse_1(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_rnm_2(data)                    HDMI_P0_BIST_YUV420_SET1_get_yuv420_rnm_2(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wa_2(data)                     HDMI_P0_BIST_YUV420_SET1_get_yuv420_wa_2(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wpulse_2(data)                 HDMI_P0_BIST_YUV420_SET1_get_yuv420_wpulse_2(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_rnm_3(data)                    HDMI_P0_BIST_YUV420_SET1_get_yuv420_rnm_3(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wa_3(data)                     HDMI_P0_BIST_YUV420_SET1_get_yuv420_wa_3(data)
#define  HDMI_BIST_YUV420_SET1_get_yuv420_wpulse_3(data)                 HDMI_P0_BIST_YUV420_SET1_get_yuv420_wpulse_3(data)
#define  HDMI_BIST_ALL_SET2_linebuf_bc1_0_mask                           HDMI_P0_BIST_ALL_SET2_linebuf_bc1_0_mask
#define  HDMI_BIST_ALL_SET2_linebuf_bc2_0_mask                           HDMI_P0_BIST_ALL_SET2_linebuf_bc2_0_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_0_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc1_0_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_0_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc2_0_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_1_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc1_1_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_1_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc2_1_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_2_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc1_2_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_2_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc2_2_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_3_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc1_3_mask
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_3_mask                            HDMI_P0_BIST_ALL_SET2_yuv420_bc2_3_mask
#define  HDMI_BIST_ALL_SET2_audio_bc1_mask                               HDMI_P0_BIST_ALL_SET2_audio_bc1_mask
#define  HDMI_BIST_ALL_SET2_audio_bc2_mask                               HDMI_P0_BIST_ALL_SET2_audio_bc2_mask
#define  HDMI_BIST_ALL_SET2_linebuf_bc1_0(data)                          HDMI_P0_BIST_ALL_SET2_linebuf_bc1_0(data)
#define  HDMI_BIST_ALL_SET2_linebuf_bc2_0(data)                          HDMI_P0_BIST_ALL_SET2_linebuf_bc2_0(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_0(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc1_0(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_0(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc2_0(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_1(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc1_1(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_1(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc2_1(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_2(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc1_2(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_2(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc2_2(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc1_3(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc1_3(data)
#define  HDMI_BIST_ALL_SET2_yuv420_bc2_3(data)                           HDMI_P0_BIST_ALL_SET2_yuv420_bc2_3(data)
#define  HDMI_BIST_ALL_SET2_audio_bc1(data)                              HDMI_P0_BIST_ALL_SET2_audio_bc1(data)
#define  HDMI_BIST_ALL_SET2_audio_bc2(data)                              HDMI_P0_BIST_ALL_SET2_audio_bc2(data)
#define  HDMI_BIST_ALL_SET2_get_linebuf_bc1_0(data)                      HDMI_P0_BIST_ALL_SET2_get_linebuf_bc1_0(data)
#define  HDMI_BIST_ALL_SET2_get_linebuf_bc2_0(data)                      HDMI_P0_BIST_ALL_SET2_get_linebuf_bc2_0(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc1_0(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc1_0(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc2_0(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc2_0(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc1_1(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc1_1(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc2_1(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc2_1(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc1_2(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc1_2(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc2_2(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc2_2(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc1_3(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc1_3(data)
#define  HDMI_BIST_ALL_SET2_get_yuv420_bc2_3(data)                       HDMI_P0_BIST_ALL_SET2_get_yuv420_bc2_3(data)
#define  HDMI_BIST_ALL_SET2_get_audio_bc1(data)                          HDMI_P0_BIST_ALL_SET2_get_audio_bc1(data)
#define  HDMI_BIST_ALL_SET2_get_audio_bc2(data)                          HDMI_P0_BIST_ALL_SET2_get_audio_bc2(data)
#define  HDMI_BIST_ALL_SET3_linebuf_test_rnm_0_mask                      HDMI_P0_BIST_ALL_SET3_linebuf_test_rnm_0_mask
#define  HDMI_BIST_ALL_SET3_linebuf_wa_0_mask                            HDMI_P0_BIST_ALL_SET3_linebuf_wa_0_mask
#define  HDMI_BIST_ALL_SET3_linebuf_wpulse_0_mask                        HDMI_P0_BIST_ALL_SET3_linebuf_wpulse_0_mask
#define  HDMI_BIST_ALL_SET3_hdr_test_rnma_0_mask                         HDMI_P0_BIST_ALL_SET3_hdr_test_rnma_0_mask
#define  HDMI_BIST_ALL_SET3_hdr_test_rnma_1_mask                         HDMI_P0_BIST_ALL_SET3_hdr_test_rnma_1_mask
#define  HDMI_BIST_ALL_SET3_audio_test_rnm_0_mask                        HDMI_P0_BIST_ALL_SET3_audio_test_rnm_0_mask
#define  HDMI_BIST_ALL_SET3_audio_wa_0_mask                              HDMI_P0_BIST_ALL_SET3_audio_wa_0_mask
#define  HDMI_BIST_ALL_SET3_audio_wpulse_0_mask                          HDMI_P0_BIST_ALL_SET3_audio_wpulse_0_mask
#define  HDMI_BIST_ALL_SET3_linebuf_test_rnm_0(data)                     HDMI_P0_BIST_ALL_SET3_linebuf_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET3_linebuf_wa_0(data)                           HDMI_P0_BIST_ALL_SET3_linebuf_wa_0(data)
#define  HDMI_BIST_ALL_SET3_linebuf_wpulse_0(data)                       HDMI_P0_BIST_ALL_SET3_linebuf_wpulse_0(data)
#define  HDMI_BIST_ALL_SET3_hdr_test_rnma_0(data)                        HDMI_P0_BIST_ALL_SET3_hdr_test_rnma_0(data)
#define  HDMI_BIST_ALL_SET3_hdr_test_rnma_1(data)                        HDMI_P0_BIST_ALL_SET3_hdr_test_rnma_1(data)
#define  HDMI_BIST_ALL_SET3_audio_test_rnm_0(data)                       HDMI_P0_BIST_ALL_SET3_audio_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET3_audio_wa_0(data)                             HDMI_P0_BIST_ALL_SET3_audio_wa_0(data)
#define  HDMI_BIST_ALL_SET3_audio_wpulse_0(data)                         HDMI_P0_BIST_ALL_SET3_audio_wpulse_0(data)
#define  HDMI_BIST_ALL_SET3_get_linebuf_test_rnm_0(data)                 HDMI_P0_BIST_ALL_SET3_get_linebuf_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET3_get_linebuf_wa_0(data)                       HDMI_P0_BIST_ALL_SET3_get_linebuf_wa_0(data)
#define  HDMI_BIST_ALL_SET3_get_linebuf_wpulse_0(data)                   HDMI_P0_BIST_ALL_SET3_get_linebuf_wpulse_0(data)
#define  HDMI_BIST_ALL_SET3_get_hdr_test_rnma_0(data)                    HDMI_P0_BIST_ALL_SET3_get_hdr_test_rnma_0(data)
#define  HDMI_BIST_ALL_SET3_get_hdr_test_rnma_1(data)                    HDMI_P0_BIST_ALL_SET3_get_hdr_test_rnma_1(data)
#define  HDMI_BIST_ALL_SET3_get_audio_test_rnm_0(data)                   HDMI_P0_BIST_ALL_SET3_get_audio_test_rnm_0(data)
#define  HDMI_BIST_ALL_SET3_get_audio_wa_0(data)                         HDMI_P0_BIST_ALL_SET3_get_audio_wa_0(data)
#define  HDMI_BIST_ALL_SET3_get_audio_wpulse_0(data)                     HDMI_P0_BIST_ALL_SET3_get_audio_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_0_mask                         HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_0_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_0_mask                          HDMI_P0_BIST_YUV420_SET2_yuv420_wa_0_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_0_mask                      HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_0_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_1_mask                         HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_1_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_1_mask                          HDMI_P0_BIST_YUV420_SET2_yuv420_wa_1_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_1_mask                      HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_1_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_2_mask                         HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_2_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_2_mask                          HDMI_P0_BIST_YUV420_SET2_yuv420_wa_2_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_2_mask                      HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_2_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_3_mask                         HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_3_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_3_mask                          HDMI_P0_BIST_YUV420_SET2_yuv420_wa_3_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_3_mask                      HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_3_mask
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_0(data)                        HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_0(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_0(data)                         HDMI_P0_BIST_YUV420_SET2_yuv420_wa_0(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_0(data)                     HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_1(data)                        HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_1(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_1(data)                         HDMI_P0_BIST_YUV420_SET2_yuv420_wa_1(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_1(data)                     HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_1(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_2(data)                        HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_2(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_2(data)                         HDMI_P0_BIST_YUV420_SET2_yuv420_wa_2(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_2(data)                     HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_2(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_rnm_3(data)                        HDMI_P0_BIST_YUV420_SET2_yuv420_rnm_3(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wa_3(data)                         HDMI_P0_BIST_YUV420_SET2_yuv420_wa_3(data)
#define  HDMI_BIST_YUV420_SET2_yuv420_wpulse_3(data)                     HDMI_P0_BIST_YUV420_SET2_yuv420_wpulse_3(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_rnm_0(data)                    HDMI_P0_BIST_YUV420_SET2_get_yuv420_rnm_0(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wa_0(data)                     HDMI_P0_BIST_YUV420_SET2_get_yuv420_wa_0(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wpulse_0(data)                 HDMI_P0_BIST_YUV420_SET2_get_yuv420_wpulse_0(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_rnm_1(data)                    HDMI_P0_BIST_YUV420_SET2_get_yuv420_rnm_1(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wa_1(data)                     HDMI_P0_BIST_YUV420_SET2_get_yuv420_wa_1(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wpulse_1(data)                 HDMI_P0_BIST_YUV420_SET2_get_yuv420_wpulse_1(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_rnm_2(data)                    HDMI_P0_BIST_YUV420_SET2_get_yuv420_rnm_2(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wa_2(data)                     HDMI_P0_BIST_YUV420_SET2_get_yuv420_wa_2(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wpulse_2(data)                 HDMI_P0_BIST_YUV420_SET2_get_yuv420_wpulse_2(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_rnm_3(data)                    HDMI_P0_BIST_YUV420_SET2_get_yuv420_rnm_3(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wa_3(data)                     HDMI_P0_BIST_YUV420_SET2_get_yuv420_wa_3(data)
#define  HDMI_BIST_YUV420_SET2_get_yuv420_wpulse_3(data)                 HDMI_P0_BIST_YUV420_SET2_get_yuv420_wpulse_3(data)
#define  HDMI_BIST_ALL_SET4_linebuf_bc1_0_mask                           HDMI_P0_BIST_ALL_SET4_linebuf_bc1_0_mask
#define  HDMI_BIST_ALL_SET4_linebuf_bc2_0_mask                           HDMI_P0_BIST_ALL_SET4_linebuf_bc2_0_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_0_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc1_0_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_0_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc2_0_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_1_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc1_1_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_1_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc2_1_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_2_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc1_2_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_2_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc2_2_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_3_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc1_3_mask
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_3_mask                            HDMI_P0_BIST_ALL_SET4_yuv420_bc2_3_mask
#define  HDMI_BIST_ALL_SET4_audio_bc1_mask                               HDMI_P0_BIST_ALL_SET4_audio_bc1_mask
#define  HDMI_BIST_ALL_SET4_audio_bc2_mask                               HDMI_P0_BIST_ALL_SET4_audio_bc2_mask
#define  HDMI_BIST_ALL_SET4_linebuf_bc1_0(data)                          HDMI_P0_BIST_ALL_SET4_linebuf_bc1_0(data)
#define  HDMI_BIST_ALL_SET4_linebuf_bc2_0(data)                          HDMI_P0_BIST_ALL_SET4_linebuf_bc2_0(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_0(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc1_0(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_0(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc2_0(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_1(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc1_1(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_1(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc2_1(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_2(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc1_2(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_2(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc2_2(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc1_3(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc1_3(data)
#define  HDMI_BIST_ALL_SET4_yuv420_bc2_3(data)                           HDMI_P0_BIST_ALL_SET4_yuv420_bc2_3(data)
#define  HDMI_BIST_ALL_SET4_audio_bc1(data)                              HDMI_P0_BIST_ALL_SET4_audio_bc1(data)
#define  HDMI_BIST_ALL_SET4_audio_bc2(data)                              HDMI_P0_BIST_ALL_SET4_audio_bc2(data)
#define  HDMI_BIST_ALL_SET4_get_linebuf_bc1_0(data)                      HDMI_P0_BIST_ALL_SET4_get_linebuf_bc1_0(data)
#define  HDMI_BIST_ALL_SET4_get_linebuf_bc2_0(data)                      HDMI_P0_BIST_ALL_SET4_get_linebuf_bc2_0(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc1_0(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc1_0(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc2_0(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc2_0(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc1_1(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc1_1(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc2_1(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc2_1(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc1_2(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc1_2(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc2_2(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc2_2(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc1_3(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc1_3(data)
#define  HDMI_BIST_ALL_SET4_get_yuv420_bc2_3(data)                       HDMI_P0_BIST_ALL_SET4_get_yuv420_bc2_3(data)
#define  HDMI_BIST_ALL_SET4_get_audio_bc1(data)                          HDMI_P0_BIST_ALL_SET4_get_audio_bc1(data)
#define  HDMI_BIST_ALL_SET4_get_audio_bc2(data)                          HDMI_P0_BIST_ALL_SET4_get_audio_bc2(data)
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls2_mask                       HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls2_mask
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls1_mask                       HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls1_mask
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls0_mask                       HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls0_mask
#define  HDMI_common2_LIGHT_SLEEP_hdr_ls_1_mask                          HDMI_P0_common2_LIGHT_SLEEP_hdr_ls_1_mask
#define  HDMI_common2_LIGHT_SLEEP_hdr_ls_0_mask                          HDMI_P0_common2_LIGHT_SLEEP_hdr_ls_0_mask
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls3_mask                        HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls3_mask
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls2_mask                        HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls2_mask
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls1_mask                        HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls1_mask
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls0_mask                        HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls0_mask
#define  HDMI_common2_LIGHT_SLEEP_audio_ls_mask                          HDMI_P0_common2_LIGHT_SLEEP_audio_ls_mask
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls2(data)                      HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls2(data)
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls1(data)                      HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls1(data)
#define  HDMI_common2_LIGHT_SLEEP_linebuf_ls0(data)                      HDMI_P0_common2_LIGHT_SLEEP_linebuf_ls0(data)
#define  HDMI_common2_LIGHT_SLEEP_hdr_ls_1(data)                         HDMI_P0_common2_LIGHT_SLEEP_hdr_ls_1(data)
#define  HDMI_common2_LIGHT_SLEEP_hdr_ls_0(data)                         HDMI_P0_common2_LIGHT_SLEEP_hdr_ls_0(data)
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls3(data)                       HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls3(data)
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls2(data)                       HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls2(data)
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls1(data)                       HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls1(data)
#define  HDMI_common2_LIGHT_SLEEP_yuv420_ls0(data)                       HDMI_P0_common2_LIGHT_SLEEP_yuv420_ls0(data)
#define  HDMI_common2_LIGHT_SLEEP_audio_ls(data)                         HDMI_P0_common2_LIGHT_SLEEP_audio_ls(data)
#define  HDMI_common2_LIGHT_SLEEP_get_linebuf_ls2(data)                  HDMI_P0_common2_LIGHT_SLEEP_get_linebuf_ls2(data)
#define  HDMI_common2_LIGHT_SLEEP_get_linebuf_ls1(data)                  HDMI_P0_common2_LIGHT_SLEEP_get_linebuf_ls1(data)
#define  HDMI_common2_LIGHT_SLEEP_get_linebuf_ls0(data)                  HDMI_P0_common2_LIGHT_SLEEP_get_linebuf_ls0(data)
#define  HDMI_common2_LIGHT_SLEEP_get_hdr_ls_1(data)                     HDMI_P0_common2_LIGHT_SLEEP_get_hdr_ls_1(data)
#define  HDMI_common2_LIGHT_SLEEP_get_hdr_ls_0(data)                     HDMI_P0_common2_LIGHT_SLEEP_get_hdr_ls_0(data)
#define  HDMI_common2_LIGHT_SLEEP_get_yuv420_ls3(data)                   HDMI_P0_common2_LIGHT_SLEEP_get_yuv420_ls3(data)
#define  HDMI_common2_LIGHT_SLEEP_get_yuv420_ls2(data)                   HDMI_P0_common2_LIGHT_SLEEP_get_yuv420_ls2(data)
#define  HDMI_common2_LIGHT_SLEEP_get_yuv420_ls1(data)                   HDMI_P0_common2_LIGHT_SLEEP_get_yuv420_ls1(data)
#define  HDMI_common2_LIGHT_SLEEP_get_yuv420_ls0(data)                   HDMI_P0_common2_LIGHT_SLEEP_get_yuv420_ls0(data)
#define  HDMI_common2_LIGHT_SLEEP_get_audio_ls(data)                     HDMI_P0_common2_LIGHT_SLEEP_get_audio_ls(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme2_mask               HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme2_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme1_mask               HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme1_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme0_mask               HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme0_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmeb_1_mask                 HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmeb_1_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmeb_0_mask                 HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmeb_0_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmea_1_mask                 HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmea_1_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmea_0_mask                 HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmea_0_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme3_mask                HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme3_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme2_mask                HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme2_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme1_mask                HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme1_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme0_mask                HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme0_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_audio_rme_mask                  HDMI_P0_common2_READ_MARGIN_ENABLE_audio_rme_mask
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme2(data)              HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme2(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme1(data)              HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_linebuf_rme0(data)              HDMI_P0_common2_READ_MARGIN_ENABLE_linebuf_rme0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmeb_1(data)                HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmeb_1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmeb_0(data)                HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmeb_0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmea_1(data)                HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmea_1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_hdr_rmea_0(data)                HDMI_P0_common2_READ_MARGIN_ENABLE_hdr_rmea_0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme3(data)               HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme3(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme2(data)               HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme2(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme1(data)               HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_yuv420_rme0(data)               HDMI_P0_common2_READ_MARGIN_ENABLE_yuv420_rme0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_audio_rme(data)                 HDMI_P0_common2_READ_MARGIN_ENABLE_audio_rme(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_linebuf_rme2(data)          HDMI_P0_common2_READ_MARGIN_ENABLE_get_linebuf_rme2(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_linebuf_rme1(data)          HDMI_P0_common2_READ_MARGIN_ENABLE_get_linebuf_rme1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_linebuf_rme0(data)          HDMI_P0_common2_READ_MARGIN_ENABLE_get_linebuf_rme0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_hdr_rmeb_1(data)            HDMI_P0_common2_READ_MARGIN_ENABLE_get_hdr_rmeb_1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_hdr_rmeb_0(data)            HDMI_P0_common2_READ_MARGIN_ENABLE_get_hdr_rmeb_0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_hdr_rmea_1(data)            HDMI_P0_common2_READ_MARGIN_ENABLE_get_hdr_rmea_1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_hdr_rmea_0(data)            HDMI_P0_common2_READ_MARGIN_ENABLE_get_hdr_rmea_0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_yuv420_rme3(data)           HDMI_P0_common2_READ_MARGIN_ENABLE_get_yuv420_rme3(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_yuv420_rme2(data)           HDMI_P0_common2_READ_MARGIN_ENABLE_get_yuv420_rme2(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_yuv420_rme1(data)           HDMI_P0_common2_READ_MARGIN_ENABLE_get_yuv420_rme1(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_yuv420_rme0(data)           HDMI_P0_common2_READ_MARGIN_ENABLE_get_yuv420_rme0(data)
#define  HDMI_common2_READ_MARGIN_ENABLE_get_audio_rme(data)             HDMI_P0_common2_READ_MARGIN_ENABLE_get_audio_rme(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_2_mask                  HDMI_P0_common2_READ_MARGIN3_linebuf_test1_2_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_1_mask                  HDMI_P0_common2_READ_MARGIN3_linebuf_test1_1_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_0_mask                  HDMI_P0_common2_READ_MARGIN3_linebuf_test1_0_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_rm2_mask                      HDMI_P0_common2_READ_MARGIN3_linebuf_rm2_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_rm1_mask                      HDMI_P0_common2_READ_MARGIN3_linebuf_rm1_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_rm0_mask                      HDMI_P0_common2_READ_MARGIN3_linebuf_rm0_mask
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_2(data)                 HDMI_P0_common2_READ_MARGIN3_linebuf_test1_2(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_1(data)                 HDMI_P0_common2_READ_MARGIN3_linebuf_test1_1(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_test1_0(data)                 HDMI_P0_common2_READ_MARGIN3_linebuf_test1_0(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_rm2(data)                     HDMI_P0_common2_READ_MARGIN3_linebuf_rm2(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_rm1(data)                     HDMI_P0_common2_READ_MARGIN3_linebuf_rm1(data)
#define  HDMI_common2_READ_MARGIN3_linebuf_rm0(data)                     HDMI_P0_common2_READ_MARGIN3_linebuf_rm0(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_test1_2(data)             HDMI_P0_common2_READ_MARGIN3_get_linebuf_test1_2(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_test1_1(data)             HDMI_P0_common2_READ_MARGIN3_get_linebuf_test1_1(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_test1_0(data)             HDMI_P0_common2_READ_MARGIN3_get_linebuf_test1_0(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_rm2(data)                 HDMI_P0_common2_READ_MARGIN3_get_linebuf_rm2(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_rm1(data)                 HDMI_P0_common2_READ_MARGIN3_get_linebuf_rm1(data)
#define  HDMI_common2_READ_MARGIN3_get_linebuf_rm0(data)                 HDMI_P0_common2_READ_MARGIN3_get_linebuf_rm0(data)
#define  HDMI_common2_READ_MARGIN1_hdr_test1b_1_mask                     HDMI_P0_common2_READ_MARGIN1_hdr_test1b_1_mask
#define  HDMI_common2_READ_MARGIN1_hdr_test1a_1_mask                     HDMI_P0_common2_READ_MARGIN1_hdr_test1a_1_mask
#define  HDMI_common2_READ_MARGIN1_hdr_test1b_0_mask                     HDMI_P0_common2_READ_MARGIN1_hdr_test1b_0_mask
#define  HDMI_common2_READ_MARGIN1_hdr_test1a_0_mask                     HDMI_P0_common2_READ_MARGIN1_hdr_test1a_0_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_3_mask                   HDMI_P0_common2_READ_MARGIN1_yuv420_test1_3_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_2_mask                   HDMI_P0_common2_READ_MARGIN1_yuv420_test1_2_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_1_mask                   HDMI_P0_common2_READ_MARGIN1_yuv420_test1_1_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_0_mask                   HDMI_P0_common2_READ_MARGIN1_yuv420_test1_0_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_rm3_mask                       HDMI_P0_common2_READ_MARGIN1_yuv420_rm3_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_rm2_mask                       HDMI_P0_common2_READ_MARGIN1_yuv420_rm2_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_rm1_mask                       HDMI_P0_common2_READ_MARGIN1_yuv420_rm1_mask
#define  HDMI_common2_READ_MARGIN1_yuv420_rm0_mask                       HDMI_P0_common2_READ_MARGIN1_yuv420_rm0_mask
#define  HDMI_common2_READ_MARGIN1_hdr_test1b_1(data)                    HDMI_P0_common2_READ_MARGIN1_hdr_test1b_1(data)
#define  HDMI_common2_READ_MARGIN1_hdr_test1a_1(data)                    HDMI_P0_common2_READ_MARGIN1_hdr_test1a_1(data)
#define  HDMI_common2_READ_MARGIN1_hdr_test1b_0(data)                    HDMI_P0_common2_READ_MARGIN1_hdr_test1b_0(data)
#define  HDMI_common2_READ_MARGIN1_hdr_test1a_0(data)                    HDMI_P0_common2_READ_MARGIN1_hdr_test1a_0(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_3(data)                  HDMI_P0_common2_READ_MARGIN1_yuv420_test1_3(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_2(data)                  HDMI_P0_common2_READ_MARGIN1_yuv420_test1_2(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_1(data)                  HDMI_P0_common2_READ_MARGIN1_yuv420_test1_1(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_test1_0(data)                  HDMI_P0_common2_READ_MARGIN1_yuv420_test1_0(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_rm3(data)                      HDMI_P0_common2_READ_MARGIN1_yuv420_rm3(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_rm2(data)                      HDMI_P0_common2_READ_MARGIN1_yuv420_rm2(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_rm1(data)                      HDMI_P0_common2_READ_MARGIN1_yuv420_rm1(data)
#define  HDMI_common2_READ_MARGIN1_yuv420_rm0(data)                      HDMI_P0_common2_READ_MARGIN1_yuv420_rm0(data)
#define  HDMI_common2_READ_MARGIN1_get_hdr_test1b_1(data)                HDMI_P0_common2_READ_MARGIN1_get_hdr_test1b_1(data)
#define  HDMI_common2_READ_MARGIN1_get_hdr_test1a_1(data)                HDMI_P0_common2_READ_MARGIN1_get_hdr_test1a_1(data)
#define  HDMI_common2_READ_MARGIN1_get_hdr_test1b_0(data)                HDMI_P0_common2_READ_MARGIN1_get_hdr_test1b_0(data)
#define  HDMI_common2_READ_MARGIN1_get_hdr_test1a_0(data)                HDMI_P0_common2_READ_MARGIN1_get_hdr_test1a_0(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_test1_3(data)              HDMI_P0_common2_READ_MARGIN1_get_yuv420_test1_3(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_test1_2(data)              HDMI_P0_common2_READ_MARGIN1_get_yuv420_test1_2(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_test1_1(data)              HDMI_P0_common2_READ_MARGIN1_get_yuv420_test1_1(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_test1_0(data)              HDMI_P0_common2_READ_MARGIN1_get_yuv420_test1_0(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_rm3(data)                  HDMI_P0_common2_READ_MARGIN1_get_yuv420_rm3(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_rm2(data)                  HDMI_P0_common2_READ_MARGIN1_get_yuv420_rm2(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_rm1(data)                  HDMI_P0_common2_READ_MARGIN1_get_yuv420_rm1(data)
#define  HDMI_common2_READ_MARGIN1_get_yuv420_rm0(data)                  HDMI_P0_common2_READ_MARGIN1_get_yuv420_rm0(data)
#define  HDMI_common2_READ_MARGIN0_hdr_rmb_1_mask                        HDMI_P0_common2_READ_MARGIN0_hdr_rmb_1_mask
#define  HDMI_common2_READ_MARGIN0_hdr_rmb_0_mask                        HDMI_P0_common2_READ_MARGIN0_hdr_rmb_0_mask
#define  HDMI_common2_READ_MARGIN0_hdr_rma_1_mask                        HDMI_P0_common2_READ_MARGIN0_hdr_rma_1_mask
#define  HDMI_common2_READ_MARGIN0_hdr_rma_0_mask                        HDMI_P0_common2_READ_MARGIN0_hdr_rma_0_mask
#define  HDMI_common2_READ_MARGIN0_audio_test1_0_mask                    HDMI_P0_common2_READ_MARGIN0_audio_test1_0_mask
#define  HDMI_common2_READ_MARGIN0_audio_rm_mask                         HDMI_P0_common2_READ_MARGIN0_audio_rm_mask
#define  HDMI_common2_READ_MARGIN0_hdr_rmb_1(data)                       HDMI_P0_common2_READ_MARGIN0_hdr_rmb_1(data)
#define  HDMI_common2_READ_MARGIN0_hdr_rmb_0(data)                       HDMI_P0_common2_READ_MARGIN0_hdr_rmb_0(data)
#define  HDMI_common2_READ_MARGIN0_hdr_rma_1(data)                       HDMI_P0_common2_READ_MARGIN0_hdr_rma_1(data)
#define  HDMI_common2_READ_MARGIN0_hdr_rma_0(data)                       HDMI_P0_common2_READ_MARGIN0_hdr_rma_0(data)
#define  HDMI_common2_READ_MARGIN0_audio_test1_0(data)                   HDMI_P0_common2_READ_MARGIN0_audio_test1_0(data)
#define  HDMI_common2_READ_MARGIN0_audio_rm(data)                        HDMI_P0_common2_READ_MARGIN0_audio_rm(data)
#define  HDMI_common2_READ_MARGIN0_get_hdr_rmb_1(data)                   HDMI_P0_common2_READ_MARGIN0_get_hdr_rmb_1(data)
#define  HDMI_common2_READ_MARGIN0_get_hdr_rmb_0(data)                   HDMI_P0_common2_READ_MARGIN0_get_hdr_rmb_0(data)
#define  HDMI_common2_READ_MARGIN0_get_hdr_rma_1(data)                   HDMI_P0_common2_READ_MARGIN0_get_hdr_rma_1(data)
#define  HDMI_common2_READ_MARGIN0_get_hdr_rma_0(data)                   HDMI_P0_common2_READ_MARGIN0_get_hdr_rma_0(data)
#define  HDMI_common2_READ_MARGIN0_get_audio_test1_0(data)               HDMI_P0_common2_READ_MARGIN0_get_audio_test1_0(data)
#define  HDMI_common2_READ_MARGIN0_get_audio_rm(data)                    HDMI_P0_common2_READ_MARGIN0_get_audio_rm(data)
#define  HDMI_common2_BIST_MODE_linebuf_bist_en_mask                     HDMI_P0_common2_BIST_MODE_linebuf_bist_en_mask
#define  HDMI_common2_BIST_MODE_hdr_bist_en_mask                         HDMI_P0_common2_BIST_MODE_hdr_bist_en_mask
#define  HDMI_common2_BIST_MODE_yuv420_bist_en_mask                      HDMI_P0_common2_BIST_MODE_yuv420_bist_en_mask
#define  HDMI_common2_BIST_MODE_audio_bist_en_mask                       HDMI_P0_common2_BIST_MODE_audio_bist_en_mask
#define  HDMI_common2_BIST_MODE_linebuf_bist_en(data)                    HDMI_P0_common2_BIST_MODE_linebuf_bist_en(data)
#define  HDMI_common2_BIST_MODE_hdr_bist_en(data)                        HDMI_P0_common2_BIST_MODE_hdr_bist_en(data)
#define  HDMI_common2_BIST_MODE_yuv420_bist_en(data)                     HDMI_P0_common2_BIST_MODE_yuv420_bist_en(data)
#define  HDMI_common2_BIST_MODE_audio_bist_en(data)                      HDMI_P0_common2_BIST_MODE_audio_bist_en(data)
#define  HDMI_common2_BIST_MODE_get_linebuf_bist_en(data)                HDMI_P0_common2_BIST_MODE_get_linebuf_bist_en(data)
#define  HDMI_common2_BIST_MODE_get_hdr_bist_en(data)                    HDMI_P0_common2_BIST_MODE_get_hdr_bist_en(data)
#define  HDMI_common2_BIST_MODE_get_yuv420_bist_en(data)                 HDMI_P0_common2_BIST_MODE_get_yuv420_bist_en(data)
#define  HDMI_common2_BIST_MODE_get_audio_bist_en(data)                  HDMI_P0_common2_BIST_MODE_get_audio_bist_en(data)
#define  HDMI_common2_BIST_DONE_linebuf_bist_done_mask                   HDMI_P0_common2_BIST_DONE_linebuf_bist_done_mask
#define  HDMI_common2_BIST_DONE_hdr_bist_done_mask                       HDMI_P0_common2_BIST_DONE_hdr_bist_done_mask
#define  HDMI_common2_BIST_DONE_yuv420_bist_done_mask                    HDMI_P0_common2_BIST_DONE_yuv420_bist_done_mask
#define  HDMI_common2_BIST_DONE_audio_bist_done_mask                     HDMI_P0_common2_BIST_DONE_audio_bist_done_mask
#define  HDMI_common2_BIST_DONE_linebuf_bist_done(data)                  HDMI_P0_common2_BIST_DONE_linebuf_bist_done(data)
#define  HDMI_common2_BIST_DONE_hdr_bist_done(data)                      HDMI_P0_common2_BIST_DONE_hdr_bist_done(data)
#define  HDMI_common2_BIST_DONE_yuv420_bist_done(data)                   HDMI_P0_common2_BIST_DONE_yuv420_bist_done(data)
#define  HDMI_common2_BIST_DONE_audio_bist_done(data)                    HDMI_P0_common2_BIST_DONE_audio_bist_done(data)
#define  HDMI_common2_BIST_DONE_get_linebuf_bist_done(data)              HDMI_P0_common2_BIST_DONE_get_linebuf_bist_done(data)
#define  HDMI_common2_BIST_DONE_get_hdr_bist_done(data)                  HDMI_P0_common2_BIST_DONE_get_hdr_bist_done(data)
#define  HDMI_common2_BIST_DONE_get_yuv420_bist_done(data)               HDMI_P0_common2_BIST_DONE_get_yuv420_bist_done(data)
#define  HDMI_common2_BIST_DONE_get_audio_bist_done(data)                HDMI_P0_common2_BIST_DONE_get_audio_bist_done(data)
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail_mask                   HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail_mask
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail0_mask                  HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail0_mask
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail1_mask                  HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail1_mask
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail2_mask                  HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail2_mask
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail_mask                       HDMI_P0_common2_BIST_FAIL_hdr_bist_fail_mask
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail0_mask                      HDMI_P0_common2_BIST_FAIL_hdr_bist_fail0_mask
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail1_mask                      HDMI_P0_common2_BIST_FAIL_hdr_bist_fail1_mask
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail_mask                    HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail_mask
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail0_mask                   HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail0_mask
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail1_mask                   HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail1_mask
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail2_mask                   HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail2_mask
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail3_mask                   HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail3_mask
#define  HDMI_common2_BIST_FAIL_audio_bist_fail_mask                     HDMI_P0_common2_BIST_FAIL_audio_bist_fail_mask
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail(data)                  HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail0(data)                 HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail1(data)                 HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_linebuf_bist_fail2(data)                 HDMI_P0_common2_BIST_FAIL_linebuf_bist_fail2(data)
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail(data)                      HDMI_P0_common2_BIST_FAIL_hdr_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail0(data)                     HDMI_P0_common2_BIST_FAIL_hdr_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_hdr_bist_fail1(data)                     HDMI_P0_common2_BIST_FAIL_hdr_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail(data)                   HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail0(data)                  HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail1(data)                  HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail2(data)                  HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail2(data)
#define  HDMI_common2_BIST_FAIL_yuv420_bist_fail3(data)                  HDMI_P0_common2_BIST_FAIL_yuv420_bist_fail3(data)
#define  HDMI_common2_BIST_FAIL_audio_bist_fail(data)                    HDMI_P0_common2_BIST_FAIL_audio_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_get_linebuf_bist_fail(data)              HDMI_P0_common2_BIST_FAIL_get_linebuf_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_get_linebuf_bist_fail0(data)             HDMI_P0_common2_BIST_FAIL_get_linebuf_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_get_linebuf_bist_fail1(data)             HDMI_P0_common2_BIST_FAIL_get_linebuf_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_get_linebuf_bist_fail2(data)             HDMI_P0_common2_BIST_FAIL_get_linebuf_bist_fail2(data)
#define  HDMI_common2_BIST_FAIL_get_hdr_bist_fail(data)                  HDMI_P0_common2_BIST_FAIL_get_hdr_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_get_hdr_bist_fail0(data)                 HDMI_P0_common2_BIST_FAIL_get_hdr_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_get_hdr_bist_fail1(data)                 HDMI_P0_common2_BIST_FAIL_get_hdr_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_get_yuv420_bist_fail(data)               HDMI_P0_common2_BIST_FAIL_get_yuv420_bist_fail(data)
#define  HDMI_common2_BIST_FAIL_get_yuv420_bist_fail0(data)              HDMI_P0_common2_BIST_FAIL_get_yuv420_bist_fail0(data)
#define  HDMI_common2_BIST_FAIL_get_yuv420_bist_fail1(data)              HDMI_P0_common2_BIST_FAIL_get_yuv420_bist_fail1(data)
#define  HDMI_common2_BIST_FAIL_get_yuv420_bist_fail2(data)              HDMI_P0_common2_BIST_FAIL_get_yuv420_bist_fail2(data)
#define  HDMI_common2_BIST_FAIL_get_yuv420_bist_fail3(data)              HDMI_P0_common2_BIST_FAIL_get_yuv420_bist_fail3(data)
#define  HDMI_common2_BIST_FAIL_get_audio_bist_fail(data)                HDMI_P0_common2_BIST_FAIL_get_audio_bist_fail(data)
#define  HDMI_common2_DRF_MODE_linebuf_drf_mode_mask                     HDMI_P0_common2_DRF_MODE_linebuf_drf_mode_mask
#define  HDMI_common2_DRF_MODE_hdr_drf_mode_mask                         HDMI_P0_common2_DRF_MODE_hdr_drf_mode_mask
#define  HDMI_common2_DRF_MODE_yuv420_drf_mode_mask                      HDMI_P0_common2_DRF_MODE_yuv420_drf_mode_mask
#define  HDMI_common2_DRF_MODE_audio_drf_mode_mask                       HDMI_P0_common2_DRF_MODE_audio_drf_mode_mask
#define  HDMI_common2_DRF_MODE_linebuf_drf_mode(data)                    HDMI_P0_common2_DRF_MODE_linebuf_drf_mode(data)
#define  HDMI_common2_DRF_MODE_hdr_drf_mode(data)                        HDMI_P0_common2_DRF_MODE_hdr_drf_mode(data)
#define  HDMI_common2_DRF_MODE_yuv420_drf_mode(data)                     HDMI_P0_common2_DRF_MODE_yuv420_drf_mode(data)
#define  HDMI_common2_DRF_MODE_audio_drf_mode(data)                      HDMI_P0_common2_DRF_MODE_audio_drf_mode(data)
#define  HDMI_common2_DRF_MODE_get_linebuf_drf_mode(data)                HDMI_P0_common2_DRF_MODE_get_linebuf_drf_mode(data)
#define  HDMI_common2_DRF_MODE_get_hdr_drf_mode(data)                    HDMI_P0_common2_DRF_MODE_get_hdr_drf_mode(data)
#define  HDMI_common2_DRF_MODE_get_yuv420_drf_mode(data)                 HDMI_P0_common2_DRF_MODE_get_yuv420_drf_mode(data)
#define  HDMI_common2_DRF_MODE_get_audio_drf_mode(data)                  HDMI_P0_common2_DRF_MODE_get_audio_drf_mode(data)
#define  HDMI_common2_DRF_RESUME_linebuf_drf_resume_mask                 HDMI_P0_common2_DRF_RESUME_linebuf_drf_resume_mask
#define  HDMI_common2_DRF_RESUME_hdr_drf_resume_mask                     HDMI_P0_common2_DRF_RESUME_hdr_drf_resume_mask
#define  HDMI_common2_DRF_RESUME_yuv420_drf_resume_mask                  HDMI_P0_common2_DRF_RESUME_yuv420_drf_resume_mask
#define  HDMI_common2_DRF_RESUME_audio_drf_resume_mask                   HDMI_P0_common2_DRF_RESUME_audio_drf_resume_mask
#define  HDMI_common2_DRF_RESUME_linebuf_drf_resume(data)                HDMI_P0_common2_DRF_RESUME_linebuf_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_hdr_drf_resume(data)                    HDMI_P0_common2_DRF_RESUME_hdr_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_yuv420_drf_resume(data)                 HDMI_P0_common2_DRF_RESUME_yuv420_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_audio_drf_resume(data)                  HDMI_P0_common2_DRF_RESUME_audio_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_get_linebuf_drf_resume(data)            HDMI_P0_common2_DRF_RESUME_get_linebuf_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_get_hdr_drf_resume(data)                HDMI_P0_common2_DRF_RESUME_get_hdr_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_get_yuv420_drf_resume(data)             HDMI_P0_common2_DRF_RESUME_get_yuv420_drf_resume(data)
#define  HDMI_common2_DRF_RESUME_get_audio_drf_resume(data)              HDMI_P0_common2_DRF_RESUME_get_audio_drf_resume(data)
#define  HDMI_common2_DRF_DONE_linebuf_drf_done_mask                     HDMI_P0_common2_DRF_DONE_linebuf_drf_done_mask
#define  HDMI_common2_DRF_DONE_hdr_drf_done_mask                         HDMI_P0_common2_DRF_DONE_hdr_drf_done_mask
#define  HDMI_common2_DRF_DONE_yuv420_drf_done_mask                      HDMI_P0_common2_DRF_DONE_yuv420_drf_done_mask
#define  HDMI_common2_DRF_DONE_audio_drf_done_mask                       HDMI_P0_common2_DRF_DONE_audio_drf_done_mask
#define  HDMI_common2_DRF_DONE_linebuf_drf_done(data)                    HDMI_P0_common2_DRF_DONE_linebuf_drf_done(data)
#define  HDMI_common2_DRF_DONE_hdr_drf_done(data)                        HDMI_P0_common2_DRF_DONE_hdr_drf_done(data)
#define  HDMI_common2_DRF_DONE_yuv420_drf_done(data)                     HDMI_P0_common2_DRF_DONE_yuv420_drf_done(data)
#define  HDMI_common2_DRF_DONE_audio_drf_done(data)                      HDMI_P0_common2_DRF_DONE_audio_drf_done(data)
#define  HDMI_common2_DRF_DONE_get_linebuf_drf_done(data)                HDMI_P0_common2_DRF_DONE_get_linebuf_drf_done(data)
#define  HDMI_common2_DRF_DONE_get_hdr_drf_done(data)                    HDMI_P0_common2_DRF_DONE_get_hdr_drf_done(data)
#define  HDMI_common2_DRF_DONE_get_yuv420_drf_done(data)                 HDMI_P0_common2_DRF_DONE_get_yuv420_drf_done(data)
#define  HDMI_common2_DRF_DONE_get_audio_drf_done(data)                  HDMI_P0_common2_DRF_DONE_get_audio_drf_done(data)
#define  HDMI_common2_DRF_PAUSE_linebuf_drf_pause_mask                   HDMI_P0_common2_DRF_PAUSE_linebuf_drf_pause_mask
#define  HDMI_common2_DRF_PAUSE_hdr_drf_pause_mask                       HDMI_P0_common2_DRF_PAUSE_hdr_drf_pause_mask
#define  HDMI_common2_DRF_PAUSE_yuv420_drf_pause_mask                    HDMI_P0_common2_DRF_PAUSE_yuv420_drf_pause_mask
#define  HDMI_common2_DRF_PAUSE_audio_drf_pause_mask                     HDMI_P0_common2_DRF_PAUSE_audio_drf_pause_mask
#define  HDMI_common2_DRF_PAUSE_linebuf_drf_pause(data)                  HDMI_P0_common2_DRF_PAUSE_linebuf_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_hdr_drf_pause(data)                      HDMI_P0_common2_DRF_PAUSE_hdr_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_yuv420_drf_pause(data)                   HDMI_P0_common2_DRF_PAUSE_yuv420_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_audio_drf_pause(data)                    HDMI_P0_common2_DRF_PAUSE_audio_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_get_linebuf_drf_pause(data)              HDMI_P0_common2_DRF_PAUSE_get_linebuf_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_get_hdr_drf_pause(data)                  HDMI_P0_common2_DRF_PAUSE_get_hdr_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_get_yuv420_drf_pause(data)               HDMI_P0_common2_DRF_PAUSE_get_yuv420_drf_pause(data)
#define  HDMI_common2_DRF_PAUSE_get_audio_drf_pause(data)                HDMI_P0_common2_DRF_PAUSE_get_audio_drf_pause(data)
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail_mask                     HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail_mask
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail0_mask                    HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail0_mask
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail1_mask                    HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail1_mask
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail2_mask                    HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail2_mask
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail_mask                         HDMI_P0_common2_DRF_FAIL_hdr_drf_fail_mask
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail0_mask                        HDMI_P0_common2_DRF_FAIL_hdr_drf_fail0_mask
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail1_mask                        HDMI_P0_common2_DRF_FAIL_hdr_drf_fail1_mask
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail_mask                      HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail_mask
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail0_mask                     HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail0_mask
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail1_mask                     HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail1_mask
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail2_mask                     HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail2_mask
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail3_mask                     HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail3_mask
#define  HDMI_common2_DRF_FAIL_audio_drf_fail_mask                       HDMI_P0_common2_DRF_FAIL_audio_drf_fail_mask
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail(data)                    HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail0(data)                   HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail1(data)                   HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_linebuf_drf_fail2(data)                   HDMI_P0_common2_DRF_FAIL_linebuf_drf_fail2(data)
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail(data)                        HDMI_P0_common2_DRF_FAIL_hdr_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail0(data)                       HDMI_P0_common2_DRF_FAIL_hdr_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_hdr_drf_fail1(data)                       HDMI_P0_common2_DRF_FAIL_hdr_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail(data)                     HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail0(data)                    HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail1(data)                    HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail2(data)                    HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail2(data)
#define  HDMI_common2_DRF_FAIL_yuv420_drf_fail3(data)                    HDMI_P0_common2_DRF_FAIL_yuv420_drf_fail3(data)
#define  HDMI_common2_DRF_FAIL_audio_drf_fail(data)                      HDMI_P0_common2_DRF_FAIL_audio_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_get_linebuf_drf_fail(data)                HDMI_P0_common2_DRF_FAIL_get_linebuf_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_get_linebuf_drf_fail0(data)               HDMI_P0_common2_DRF_FAIL_get_linebuf_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_get_linebuf_drf_fail1(data)               HDMI_P0_common2_DRF_FAIL_get_linebuf_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_get_linebuf_drf_fail2(data)               HDMI_P0_common2_DRF_FAIL_get_linebuf_drf_fail2(data)
#define  HDMI_common2_DRF_FAIL_get_hdr_drf_fail(data)                    HDMI_P0_common2_DRF_FAIL_get_hdr_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_get_hdr_drf_fail0(data)                   HDMI_P0_common2_DRF_FAIL_get_hdr_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_get_hdr_drf_fail1(data)                   HDMI_P0_common2_DRF_FAIL_get_hdr_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_get_yuv420_drf_fail(data)                 HDMI_P0_common2_DRF_FAIL_get_yuv420_drf_fail(data)
#define  HDMI_common2_DRF_FAIL_get_yuv420_drf_fail0(data)                HDMI_P0_common2_DRF_FAIL_get_yuv420_drf_fail0(data)
#define  HDMI_common2_DRF_FAIL_get_yuv420_drf_fail1(data)                HDMI_P0_common2_DRF_FAIL_get_yuv420_drf_fail1(data)
#define  HDMI_common2_DRF_FAIL_get_yuv420_drf_fail2(data)                HDMI_P0_common2_DRF_FAIL_get_yuv420_drf_fail2(data)
#define  HDMI_common2_DRF_FAIL_get_yuv420_drf_fail3(data)                HDMI_P0_common2_DRF_FAIL_get_yuv420_drf_fail3(data)
#define  HDMI_common2_DRF_FAIL_get_audio_drf_fail(data)                  HDMI_P0_common2_DRF_FAIL_get_audio_drf_fail(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_done_mask                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_done_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_fail_mask                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_fail_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_done_mask                     HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_done_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_pause_mask                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_pause_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_fail_mask                     HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_fail_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bc1_mask                          HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bc1_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bc2_mask                          HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bc2_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_test_rnm_0_mask                   HDMI_P0_COMMON_MSBUF_MBIST_msbuf_test_rnm_0_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_wa_0_mask                         HDMI_P0_COMMON_MSBUF_MBIST_msbuf_wa_0_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_wpulse_0_mask                     HDMI_P0_COMMON_MSBUF_MBIST_msbuf_wpulse_0_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_test1_mask                        HDMI_P0_COMMON_MSBUF_MBIST_msbuf_test1_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_rm_mask                           HDMI_P0_COMMON_MSBUF_MBIST_msbuf_rm_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_rme_mask                          HDMI_P0_COMMON_MSBUF_MBIST_msbuf_rme_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_resume_mask                   HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_resume_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_mode_mask                     HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_mode_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_en_mask                      HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_en_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_ls_mask                           HDMI_P0_COMMON_MSBUF_MBIST_msbuf_ls_mask
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_done(data)                   HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_done(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_fail(data)                   HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_fail(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_done(data)                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_done(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_pause(data)                   HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_pause(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_fail(data)                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_fail(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bc1(data)                         HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bc1(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bc2(data)                         HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bc2(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_test_rnm_0(data)                  HDMI_P0_COMMON_MSBUF_MBIST_msbuf_test_rnm_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_wa_0(data)                        HDMI_P0_COMMON_MSBUF_MBIST_msbuf_wa_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_wpulse_0(data)                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_wpulse_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_test1(data)                       HDMI_P0_COMMON_MSBUF_MBIST_msbuf_test1(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_rm(data)                          HDMI_P0_COMMON_MSBUF_MBIST_msbuf_rm(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_rme(data)                         HDMI_P0_COMMON_MSBUF_MBIST_msbuf_rme(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_resume(data)                  HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_resume(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_drf_mode(data)                    HDMI_P0_COMMON_MSBUF_MBIST_msbuf_drf_mode(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_bist_en(data)                     HDMI_P0_COMMON_MSBUF_MBIST_msbuf_bist_en(data)
#define  HDMI_COMMON_MSBUF_MBIST_msbuf_ls(data)                          HDMI_P0_COMMON_MSBUF_MBIST_msbuf_ls(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_bist_done(data)               HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_bist_done(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_bist_fail(data)               HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_bist_fail(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_drf_done(data)                HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_drf_done(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_drf_pause(data)               HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_drf_pause(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_drf_fail(data)                HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_drf_fail(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_bc1(data)                     HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_bc1(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_bc2(data)                     HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_bc2(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_test_rnm_0(data)              HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_test_rnm_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_wa_0(data)                    HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_wa_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_wpulse_0(data)                HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_wpulse_0(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_test1(data)                   HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_test1(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_rm(data)                      HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_rm(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_rme(data)                     HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_rme(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_drf_resume(data)              HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_drf_resume(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_drf_mode(data)                HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_drf_mode(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_bist_en(data)                 HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_bist_en(data)
#define  HDMI_COMMON_MSBUF_MBIST_get_msbuf_ls(data)                      HDMI_P0_COMMON_MSBUF_MBIST_get_msbuf_ls(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_done_mask                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_done_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_fail_mask                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_fail_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_done_mask                    HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_done_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_pause_mask                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_pause_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_fail_mask                    HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_fail_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bc1_mask                         HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bc1_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bc2_mask                         HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bc2_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_test_rnm_0_mask                  HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_test_rnm_0_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_wa_0_mask                        HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_wa_0_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_wpulse_0_mask                    HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_wpulse_0_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_test1_mask                       HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_test1_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_rm_mask                          HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_rm_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_rme_mask                         HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_rme_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_resume_mask                  HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_resume_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_mode_mask                    HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_mode_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_en_mask                     HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_en_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_ls_mask                          HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_ls_mask
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_done(data)                  HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_done(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_fail(data)                  HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_fail(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_done(data)                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_done(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_pause(data)                  HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_pause(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_fail(data)                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_fail(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bc1(data)                        HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bc1(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bc2(data)                        HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bc2(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_test_rnm_0(data)                 HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_test_rnm_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_wa_0(data)                       HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_wa_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_wpulse_0(data)                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_wpulse_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_test1(data)                      HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_test1(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_rm(data)                         HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_rm(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_rme(data)                        HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_rme(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_resume(data)                 HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_resume(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_drf_mode(data)                   HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_drf_mode(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_bist_en(data)                    HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_bist_en(data)
#define  HDMI_COMMON2_MSBUF_MBIST_msbuf_ls(data)                         HDMI_P0_COMMON2_MSBUF_MBIST_msbuf_ls(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_bist_done(data)              HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_bist_done(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_bist_fail(data)              HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_bist_fail(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_drf_done(data)               HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_drf_done(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_drf_pause(data)              HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_drf_pause(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_drf_fail(data)               HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_drf_fail(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_bc1(data)                    HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_bc1(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_bc2(data)                    HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_bc2(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_test_rnm_0(data)             HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_test_rnm_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_wa_0(data)                   HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_wa_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_wpulse_0(data)               HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_wpulse_0(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_test1(data)                  HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_test1(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_rm(data)                     HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_rm(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_rme(data)                    HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_rme(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_drf_resume(data)             HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_drf_resume(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_drf_mode(data)               HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_drf_mode(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_bist_en(data)                HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_bist_en(data)
#define  HDMI_COMMON2_MSBUF_MBIST_get_msbuf_ls(data)                     HDMI_P0_COMMON2_MSBUF_MBIST_get_msbuf_ls(data)


//---------------------------------------------------------------------------------
// HDMI2.0 MAC (STB) for wake up by HDMI
//---------------------------------------------------------------------------------
#include <rbus/hdmi_stb_p0_reg.h>
#include <rbus/hdmi_stb_p1_reg.h>
#include <rbus/hdmi_stb_p2_reg.h>
#include <rbus/hdmi_stb_p3_reg.h>

struct hdmi_stb_mac_reg_st
{
    // SCDC related
    uint32_t  scdc_cr;
    uint32_t  scdc_pcr;
    uint32_t  scdc_ap;
    uint32_t  scdc_dp;

    // HDCP 1.4
    uint32_t  hdcp_cr;
    uint32_t  hdcp_dkap;
    uint32_t  hdcp_pcr;
    uint32_t  hdcp_flag1;
    uint32_t  hdcp_flag2;
    uint32_t  hdcp_ap;
    uint32_t  hdcp_dp;

    // HDCP 2.2
    uint32_t  hdcp_2p2_cr;
    uint32_t  hdcp_2p2_sr0;
    uint32_t  hdcp_2p2_sr1;
    uint32_t  hdcp_msap;
    uint32_t  hdcp_msdp;
    uint32_t  hdcp_pend;

    // BIST
    uint32_t  mbist_st0;
    uint32_t  mbist_st1;
    uint32_t  mbist_st2;
    uint32_t  mbist_st3;
    uint32_t  stb_xtal_1ms;
    uint32_t  stb_xtal_4p7us;
    uint32_t  hdmi_stb_dbgbox_ctl;
    uint32_t  clkdetsr ;
    uint32_t  gdi_tmdsclk_setting_00;
    uint32_t  gdi_tmdsclk_setting_01;
    uint32_t  gdi_tmdsclk_setting_02;
    uint32_t  ot;
    uint32_t  hdmi_stb_dbgbox_port_sel;
    uint32_t  clken_hdmi_stb;
    uint32_t  rst_n_hdmi_stb;
    uint32_t  hdmi_mp;
};

extern const struct hdmi_stb_mac_reg_st stb_mac[4];

// Registe Wrapper

#define  HDMI_STB_SCDC_CR_reg                              (stb_mac[nport].scdc_cr)
#define  HDMI_STB_SCDC_PCR_reg                             (stb_mac[nport].scdc_pcr)
#define  HDMI_STB_SCDC_AP_reg                              (stb_mac[nport].scdc_ap)
#define  HDMI_STB_SCDC_DP_reg                              (stb_mac[nport].scdc_dp)
#define  HDMI_STB_HDCP_CR_reg                              (stb_mac[nport].hdcp_cr)
#define  HDMI_STB_HDCP_DKAP_reg                            (stb_mac[nport].hdcp_dkap)
#define  HDMI_STB_HDCP_PCR_reg                             (stb_mac[nport].hdcp_pcr)
#define  HDMI_STB_HDCP_FLAG1_reg                           (stb_mac[nport].hdcp_flag1)
#define  HDMI_STB_HDCP_FLAG2_reg                           (stb_mac[nport].hdcp_flag2)
#define  HDMI_STB_HDCP_AP_reg                              (stb_mac[nport].hdcp_ap)
#define  HDMI_STB_HDCP_DP_reg                              (stb_mac[nport].hdcp_dp)
#define  HDMI_STB_HDCP_2p2_CR_reg                          (stb_mac[nport].hdcp_2p2_cr)
#define  HDMI_STB_HDCP_2p2_SR0_reg                         (stb_mac[nport].hdcp_2p2_sr0)
#define  HDMI_STB_HDCP_2p2_SR1_reg                         (stb_mac[nport].hdcp_2p2_sr1)
#define  HDMI_STB_HDCP_MSAP_reg                            (stb_mac[nport].hdcp_msap)
#define  HDMI_STB_HDCP_MSDP_reg                            (stb_mac[nport].hdcp_msdp)
#define  HDMI_STB_HDCP_PEND_reg                            (stb_mac[nport].hdcp_pend)
#define  HDMI_STB_MBIST_ST0_reg                            (stb_mac[nport].mbist_st0)
#define  HDMI_STB_MBIST_ST1_reg                            (stb_mac[nport].mbist_st1)
#define  HDMI_STB_MBIST_ST2_reg                            (stb_mac[nport].mbist_st2)
#define  HDMI_STB_MBIST_ST3_reg                            (stb_mac[nport].mbist_st3)
#define  HDMI_STB_stb_xtal_1ms                             (stb_mac[nport].stb_xtal_1ms)
#define  HDMI_STB_stb_xtal_4p7us                           (stb_mac[nport].stb_xtal_4p7us)
#define  HDMI_STB_HDMI_STB_DBGBOX_CTL_reg                  (stb_mac[nport].hdmi_stb_dbgbox_ctl)
#define  HDMI_STB_CLKDETSR_reg                             (stb_mac[nport].clkdetsr)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_reg               (stb_mac[nport].gdi_tmdsclk_setting_00)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_reg               (stb_mac[nport].gdi_tmdsclk_setting_01)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_reg               (stb_mac[nport].gdi_tmdsclk_setting_02)

// the following register is one time register (that only available on for MAC0)
#define  HDMI_STB_ot_reg                                   (stb_mac[0].ot)
#define  HDMI_STB_HDMI_STB_DBGBOX_PORT_SEL_reg             (stb_mac[0].hdmi_stb_dbgbox_port_sel)
#define  HDMI_STB_clken_hdmi_stb_reg                       (stb_mac[0].clken_hdmi_stb)
#define  HDMI_STB_rst_n_hdmi_stb_reg                       (stb_mac[0].rst_n_hdmi_stb)
#define  HDMI_STB_hdmi_mp_reg                              (stb_mac[0].hdmi_mp)


// Bit Field Wrapper

#define  HDMI_STB_SCDC_CR_update11_hwauto_en_mask                            HDMI_STB_P0_SCDC_CR_update11_hwauto_en_mask
#define  HDMI_STB_SCDC_CR_update10_hwauto_en_mask                            HDMI_STB_P0_SCDC_CR_update10_hwauto_en_mask
#define  HDMI_STB_SCDC_CR_status_flags_chg_irq_mask                          HDMI_STB_P0_SCDC_CR_status_flags_chg_irq_mask
#define  HDMI_STB_SCDC_CR_hw_mask_en_mask                                    HDMI_STB_P0_SCDC_CR_hw_mask_en_mask
#define  HDMI_STB_SCDC_CR_irq_cpu_gen_mask                                   HDMI_STB_P0_SCDC_CR_irq_cpu_gen_mask
#define  HDMI_STB_SCDC_CR_write20_det_irq_flag_mask                          HDMI_STB_P0_SCDC_CR_write20_det_irq_flag_mask
#define  HDMI_STB_SCDC_CR_write20_det_irq_en_mask                            HDMI_STB_P0_SCDC_CR_write20_det_irq_en_mask
#define  HDMI_STB_SCDC_CR_pending_mask                                       HDMI_STB_P0_SCDC_CR_pending_mask
#define  HDMI_STB_SCDC_CR_scdc_err_det_sum_port_mask                         HDMI_STB_P0_SCDC_CR_scdc_err_det_sum_port_mask
#define  HDMI_STB_SCDC_CR_scdc_sel_mask                                      HDMI_STB_P0_SCDC_CR_scdc_sel_mask
#define  HDMI_STB_SCDC_CR_scdc_reset_mask                                    HDMI_STB_P0_SCDC_CR_scdc_reset_mask
#define  HDMI_STB_SCDC_CR_write31_det_irq_flag_mask                          HDMI_STB_P0_SCDC_CR_write31_det_irq_flag_mask
#define  HDMI_STB_SCDC_CR_write31_det_irq_en_mask                            HDMI_STB_P0_SCDC_CR_write31_det_irq_en_mask
#define  HDMI_STB_SCDC_CR_frl_start_src_clr_en_mask                          HDMI_STB_P0_SCDC_CR_frl_start_src_clr_en_mask
#define  HDMI_STB_SCDC_CR_maddf_mask                                         HDMI_STB_P0_SCDC_CR_maddf_mask
#define  HDMI_STB_SCDC_CR_scdc_en_mask                                       HDMI_STB_P0_SCDC_CR_scdc_en_mask
//-------------if you want to do write 1 clear, you have to use this mask----------------
#define  HDMI_STB_SCDC_CR_write_1_clr_mask    (HDMI_STB_SCDC_CR_write31_det_irq_flag_mask|HDMI_STB_SCDC_CR_write20_det_irq_flag_mask|HDMI_STB_SCDC_CR_pending_mask) 
//----------------------------------------------------------------------------
#define  HDMI_STB_SCDC_CR_update11_hwauto_en(data)                           HDMI_STB_P0_SCDC_CR_update11_hwauto_en(data)
#define  HDMI_STB_SCDC_CR_update10_hwauto_en(data)                           HDMI_STB_P0_SCDC_CR_update10_hwauto_en(data)
#define  HDMI_STB_SCDC_CR_status_flags_chg_irq(data)                         HDMI_STB_P0_SCDC_CR_status_flags_chg_irq(data)
#define  HDMI_STB_SCDC_CR_hw_mask_en(data)                                   HDMI_STB_P0_SCDC_CR_hw_mask_en(data)
#define  HDMI_STB_SCDC_CR_irq_cpu_gen(data)                                  HDMI_STB_P0_SCDC_CR_irq_cpu_gen(data)
#define  HDMI_STB_SCDC_CR_write20_det_irq_flag(data)                         HDMI_STB_P0_SCDC_CR_write20_det_irq_flag(data)
#define  HDMI_STB_SCDC_CR_write20_det_irq_en(data)                           HDMI_STB_P0_SCDC_CR_write20_det_irq_en(data)
#define  HDMI_STB_SCDC_CR_pending(data)                                      HDMI_STB_P0_SCDC_CR_pending(data)
#define  HDMI_STB_SCDC_CR_scdc_err_det_sum_port(data)                        HDMI_STB_P0_SCDC_CR_scdc_err_det_sum_port(data)
#define  HDMI_STB_SCDC_CR_scdc_sel(data)                                     HDMI_STB_P0_SCDC_CR_scdc_sel(data)
#define  HDMI_STB_SCDC_CR_scdc_reset(data)                                   HDMI_STB_P0_SCDC_CR_scdc_reset(data)
#define  HDMI_STB_SCDC_CR_write31_det_irq_flag(data)                         HDMI_STB_P0_SCDC_CR_write31_det_irq_flag(data)
#define  HDMI_STB_SCDC_CR_write31_det_irq_en(data)                           HDMI_STB_P0_SCDC_CR_write31_det_irq_en(data)
#define  HDMI_STB_SCDC_CR_frl_start_src_clr_en(data)                         HDMI_STB_P0_SCDC_CR_frl_start_src_clr_en(data)
#define  HDMI_STB_SCDC_CR_maddf(data)                                        HDMI_STB_P0_SCDC_CR_maddf(data)
#define  HDMI_STB_SCDC_CR_scdc_en(data)                                      HDMI_STB_P0_SCDC_CR_scdc_en(data)
#define  HDMI_STB_SCDC_CR_get_update11_hwauto_en(data)                       HDMI_STB_P0_SCDC_CR_get_update11_hwauto_en(data)
#define  HDMI_STB_SCDC_CR_get_update10_hwauto_en(data)                       HDMI_STB_P0_SCDC_CR_get_update10_hwauto_en(data)
#define  HDMI_STB_SCDC_CR_get_status_flags_chg_irq(data)                     HDMI_STB_P0_SCDC_CR_get_status_flags_chg_irq(data)
#define  HDMI_STB_SCDC_CR_get_hw_mask_en(data)                               HDMI_STB_P0_SCDC_CR_get_hw_mask_en(data)
#define  HDMI_STB_SCDC_CR_get_irq_cpu_gen(data)                              HDMI_STB_P0_SCDC_CR_get_irq_cpu_gen(data)
#define  HDMI_STB_SCDC_CR_get_write20_det_irq_flag(data)                     HDMI_STB_P0_SCDC_CR_get_write20_det_irq_flag(data)
#define  HDMI_STB_SCDC_CR_get_write20_det_irq_en(data)                       HDMI_STB_P0_SCDC_CR_get_write20_det_irq_en(data)
#define  HDMI_STB_SCDC_CR_get_pending(data)                                  HDMI_STB_P0_SCDC_CR_get_pending(data)
#define  HDMI_STB_SCDC_CR_get_scdc_err_det_sum_port(data)                    HDMI_STB_P0_SCDC_CR_get_scdc_err_det_sum_port(data)
#define  HDMI_STB_SCDC_CR_get_scdc_sel(data)                                 HDMI_STB_P0_SCDC_CR_get_scdc_sel(data)
#define  HDMI_STB_SCDC_CR_get_scdc_reset(data)                               HDMI_STB_P0_SCDC_CR_get_scdc_reset(data)
#define  HDMI_STB_SCDC_CR_get_write31_det_irq_flag(data)                     HDMI_STB_P0_SCDC_CR_get_write31_det_irq_flag(data)
#define  HDMI_STB_SCDC_CR_get_write31_det_irq_en(data)                       HDMI_STB_P0_SCDC_CR_get_write31_det_irq_en(data)
#define  HDMI_STB_SCDC_CR_get_frl_start_src_clr_en(data)                     HDMI_STB_P0_SCDC_CR_get_frl_start_src_clr_en(data)
#define  HDMI_STB_SCDC_CR_get_maddf(data)                                    HDMI_STB_P0_SCDC_CR_get_maddf(data)
#define  HDMI_STB_SCDC_CR_get_scdc_en(data)                                  HDMI_STB_P0_SCDC_CR_get_scdc_en(data)

#define  HDMI_STB_SCDC_PCR_timeout_flag_mask                                 HDMI_STB_P0_SCDC_PCR_timeout_flag_mask
#define  HDMI_STB_SCDC_PCR_timeout_sel_mask                                  HDMI_STB_P0_SCDC_PCR_timeout_sel_mask
#define  HDMI_STB_SCDC_PCR_status_flag_mask                                  HDMI_STB_P0_SCDC_PCR_status_flag_mask
#define  HDMI_STB_SCDC_PCR_config_flag_mask                                  HDMI_STB_P0_SCDC_PCR_config_flag_mask
#define  HDMI_STB_SCDC_PCR_scrambler_status_flag_mask                        HDMI_STB_P0_SCDC_PCR_scrambler_status_flag_mask
#define  HDMI_STB_SCDC_PCR_tmds_config_flag_mask                             HDMI_STB_P0_SCDC_PCR_tmds_config_flag_mask
#define  HDMI_STB_SCDC_PCR_i2c_scl_dly_sel_mask                              HDMI_STB_P0_SCDC_PCR_i2c_scl_dly_sel_mask
#define  HDMI_STB_SCDC_PCR_i2c_sda_dly_sel_mask                              HDMI_STB_P0_SCDC_PCR_i2c_sda_dly_sel_mask
#define  HDMI_STB_SCDC_PCR_i2c_free_num_mask                                 HDMI_STB_P0_SCDC_PCR_i2c_free_num_mask
#define  HDMI_STB_SCDC_PCR_rr_retry_sel_mask                                 HDMI_STB_P0_SCDC_PCR_rr_retry_sel_mask
#define  HDMI_STB_SCDC_PCR_dbnc_level_sel_mask                               HDMI_STB_P0_SCDC_PCR_dbnc_level_sel_mask
#define  HDMI_STB_SCDC_PCR_fifo_mode_mask                                    HDMI_STB_P0_SCDC_PCR_fifo_mode_mask
#define  HDMI_STB_SCDC_PCR_fifo_mode_fw_mask                                 HDMI_STB_P0_SCDC_PCR_fifo_mode_fw_mask
#define  HDMI_STB_SCDC_PCR_fifo_switch_mode_mask                             HDMI_STB_P0_SCDC_PCR_fifo_switch_mode_mask
#define  HDMI_STB_SCDC_PCR_dummy_3_mask                                      HDMI_STB_P0_SCDC_PCR_dummy_3_mask
#define  HDMI_STB_SCDC_PCR_test_rr_added_delay_sel_mask                      HDMI_STB_P0_SCDC_PCR_test_rr_added_delay_sel_mask
#define  HDMI_STB_SCDC_PCR_apai_mask                                         HDMI_STB_P0_SCDC_PCR_apai_mask
#define  HDMI_STB_SCDC_PCR_timeout_flag(data)                                HDMI_STB_P0_SCDC_PCR_timeout_flag(data)
#define  HDMI_STB_SCDC_PCR_timeout_sel(data)                                 HDMI_STB_P0_SCDC_PCR_timeout_sel(data)
#define  HDMI_STB_SCDC_PCR_status_flag(data)                                 HDMI_STB_P0_SCDC_PCR_status_flag(data)
#define  HDMI_STB_SCDC_PCR_config_flag(data)                                 HDMI_STB_P0_SCDC_PCR_config_flag(data)
#define  HDMI_STB_SCDC_PCR_scrambler_status_flag(data)                       HDMI_STB_P0_SCDC_PCR_scrambler_status_flag(data)
#define  HDMI_STB_SCDC_PCR_tmds_config_flag(data)                            HDMI_STB_P0_SCDC_PCR_tmds_config_flag(data)
#define  HDMI_STB_SCDC_PCR_i2c_scl_dly_sel(data)                             HDMI_STB_P0_SCDC_PCR_i2c_scl_dly_sel(data)
#define  HDMI_STB_SCDC_PCR_i2c_sda_dly_sel(data)                             HDMI_STB_P0_SCDC_PCR_i2c_sda_dly_sel(data)
#define  HDMI_STB_SCDC_PCR_i2c_free_num(data)                                HDMI_STB_P0_SCDC_PCR_i2c_free_num(data)
#define  HDMI_STB_SCDC_PCR_rr_retry_sel(data)                                HDMI_STB_P0_SCDC_PCR_rr_retry_sel(data)
#define  HDMI_STB_SCDC_PCR_dbnc_level_sel(data)                              HDMI_STB_P0_SCDC_PCR_dbnc_level_sel(data)
#define  HDMI_STB_SCDC_PCR_fifo_mode(data)                                   HDMI_STB_P0_SCDC_PCR_fifo_mode(data)
#define  HDMI_STB_SCDC_PCR_fifo_mode_fw(data)                                HDMI_STB_P0_SCDC_PCR_fifo_mode_fw(data)
#define  HDMI_STB_SCDC_PCR_fifo_switch_mode(data)                            HDMI_STB_P0_SCDC_PCR_fifo_switch_mode(data)
#define  HDMI_STB_SCDC_PCR_dummy_3(data)                                     HDMI_STB_P0_SCDC_PCR_dummy_3(data)
#define  HDMI_STB_SCDC_PCR_test_rr_added_delay_sel(data)                     HDMI_STB_P0_SCDC_PCR_test_rr_added_delay_sel(data)
#define  HDMI_STB_SCDC_PCR_apai(data)                                        HDMI_STB_P0_SCDC_PCR_apai(data)
#define  HDMI_STB_SCDC_PCR_get_timeout_flag(data)                            HDMI_STB_P0_SCDC_PCR_get_timeout_flag(data)
#define  HDMI_STB_SCDC_PCR_get_timeout_sel(data)                             HDMI_STB_P0_SCDC_PCR_get_timeout_sel(data)
#define  HDMI_STB_SCDC_PCR_get_status_flag(data)                             HDMI_STB_P0_SCDC_PCR_get_status_flag(data)
#define  HDMI_STB_SCDC_PCR_get_config_flag(data)                             HDMI_STB_P0_SCDC_PCR_get_config_flag(data)
#define  HDMI_STB_SCDC_PCR_get_scrambler_status_flag(data)                   HDMI_STB_P0_SCDC_PCR_get_scrambler_status_flag(data)
#define  HDMI_STB_SCDC_PCR_get_tmds_config_flag(data)                        HDMI_STB_P0_SCDC_PCR_get_tmds_config_flag(data)
#define  HDMI_STB_SCDC_PCR_get_i2c_scl_dly_sel(data)                         HDMI_STB_P0_SCDC_PCR_get_i2c_scl_dly_sel(data)
#define  HDMI_STB_SCDC_PCR_get_i2c_sda_dly_sel(data)                         HDMI_STB_P0_SCDC_PCR_get_i2c_sda_dly_sel(data)
#define  HDMI_STB_SCDC_PCR_get_i2c_free_num(data)                            HDMI_STB_P0_SCDC_PCR_get_i2c_free_num(data)
#define  HDMI_STB_SCDC_PCR_get_rr_retry_sel(data)                            HDMI_STB_P0_SCDC_PCR_get_rr_retry_sel(data)
#define  HDMI_STB_SCDC_PCR_get_dbnc_level_sel(data)                          HDMI_STB_P0_SCDC_PCR_get_dbnc_level_sel(data)
#define  HDMI_STB_SCDC_PCR_get_dummy_3(data)                                 HDMI_STB_P0_SCDC_PCR_get_dummy_3(data)
#define  HDMI_STB_SCDC_PCR_get_test_rr_added_delay_sel(data)                 HDMI_STB_P0_SCDC_PCR_get_test_rr_added_delay_sel(data)
#define  HDMI_STB_SCDC_PCR_get_apai(data)                                    HDMI_STB_P0_SCDC_PCR_get_apai(data)

#define  HDMI_STB_SCDC_AP_ap1_mask                                           HDMI_STB_P0_SCDC_AP_ap1_mask
#define  HDMI_STB_SCDC_AP_ap1(data)                                          HDMI_STB_P0_SCDC_AP_ap1(data)
#define  HDMI_STB_SCDC_AP_get_ap1(data)                                      HDMI_STB_P0_SCDC_AP_get_ap1(data)

#define  HDMI_STB_SCDC_DP_dp1_rwmask_mask                                    HDMI_STB_P0_SCDC_DP_dp1_rwmask_mask
#define  HDMI_STB_SCDC_DP_dp1_mask                                           HDMI_STB_P0_SCDC_DP_dp1_mask
#define  HDMI_STB_SCDC_DP_dp1_rwmask(data)                                   HDMI_STB_P0_SCDC_DP_dp1_rwmask(data)
#define  HDMI_STB_SCDC_DP_dp1(data)                                          HDMI_STB_P0_SCDC_DP_dp1(data)
#define  HDMI_STB_SCDC_DP_get_dp1_rwmask(data)                               HDMI_STB_P0_SCDC_DP_get_dp1_rwmask(data)
#define  HDMI_STB_SCDC_DP_get_dp1(data)                                      HDMI_STB_P0_SCDC_DP_get_dp1(data)

#define  HDMI_STB_HDCP_CR_clr_ri_mask                                        HDMI_STB_P0_HDCP_CR_clr_ri_mask
#define  HDMI_STB_HDCP_CR_cypher_dis_mask                                    HDMI_STB_P0_HDCP_CR_cypher_dis_mask
#define  HDMI_STB_HDCP_CR_maddf_mask                                         HDMI_STB_P0_HDCP_CR_maddf_mask
#define  HDMI_STB_HDCP_CR_dkapde_mask                                        HDMI_STB_P0_HDCP_CR_dkapde_mask
#define  HDMI_STB_HDCP_CR_hdcp_en_mask                                       HDMI_STB_P0_HDCP_CR_hdcp_en_mask
#define  HDMI_STB_HDCP_CR_clr_ri(data)                                       HDMI_STB_P0_HDCP_CR_clr_ri(data)
#define  HDMI_STB_HDCP_CR_cypher_dis(data)                                   HDMI_STB_P0_HDCP_CR_cypher_dis(data)
#define  HDMI_STB_HDCP_CR_maddf(data)                                        HDMI_STB_P0_HDCP_CR_maddf(data)
#define  HDMI_STB_HDCP_CR_dkapde(data)                                       HDMI_STB_P0_HDCP_CR_dkapde(data)
#define  HDMI_STB_HDCP_CR_hdcp_en(data)                                      HDMI_STB_P0_HDCP_CR_hdcp_en(data)
#define  HDMI_STB_HDCP_CR_get_clr_ri(data)                                   HDMI_STB_P0_HDCP_CR_get_clr_ri(data)
#define  HDMI_STB_HDCP_CR_get_cypher_dis(data)                               HDMI_STB_P0_HDCP_CR_get_cypher_dis(data)
#define  HDMI_STB_HDCP_CR_get_maddf(data)                                    HDMI_STB_P0_HDCP_CR_get_maddf(data)
#define  HDMI_STB_HDCP_CR_get_dkapde(data)                                   HDMI_STB_P0_HDCP_CR_get_dkapde(data)
#define  HDMI_STB_HDCP_CR_get_hdcp_en(data)                                  HDMI_STB_P0_HDCP_CR_get_hdcp_en(data)

#define  HDMI_STB_HDCP_DKAP_dkap_mask                                        HDMI_STB_P0_HDCP_DKAP_dkap_mask
#define  HDMI_STB_HDCP_DKAP_dkap(data)                                       HDMI_STB_P0_HDCP_DKAP_dkap(data)
#define  HDMI_STB_HDCP_DKAP_get_dkap(data)                                   HDMI_STB_P0_HDCP_DKAP_get_dkap(data)

#define  HDMI_STB_HDCP_PCR_iic_st_mask                                       HDMI_STB_P0_HDCP_PCR_iic_st_mask
#define  HDMI_STB_HDCP_PCR_i2c_scl_dly_sel_mask                              HDMI_STB_P0_HDCP_PCR_i2c_scl_dly_sel_mask
#define  HDMI_STB_HDCP_PCR_i2c_sda_dly_sel_mask                              HDMI_STB_P0_HDCP_PCR_i2c_sda_dly_sel_mask
#define  HDMI_STB_HDCP_PCR_ddcdbnc_mask                                      HDMI_STB_P0_HDCP_PCR_ddcdbnc_mask
#define  HDMI_STB_HDCP_PCR_dvi_enc_mode_mask                                 HDMI_STB_P0_HDCP_PCR_dvi_enc_mode_mask
#define  HDMI_STB_HDCP_PCR_dbnc_level_sel_mask                               HDMI_STB_P0_HDCP_PCR_dbnc_level_sel_mask
#define  HDMI_STB_HDCP_PCR_km_start_sel_mask                                 HDMI_STB_P0_HDCP_PCR_km_start_sel_mask
#define  HDMI_STB_HDCP_PCR_apai_type_mask                                    HDMI_STB_P0_HDCP_PCR_apai_type_mask
#define  HDMI_STB_HDCP_PCR_apai_mask                                         HDMI_STB_P0_HDCP_PCR_apai_mask
#define  HDMI_STB_HDCP_PCR_iic_st(data)                                      HDMI_STB_P0_HDCP_PCR_iic_st(data)
#define  HDMI_STB_HDCP_PCR_i2c_scl_dly_sel(data)                             HDMI_STB_P0_HDCP_PCR_i2c_scl_dly_sel(data)
#define  HDMI_STB_HDCP_PCR_i2c_sda_dly_sel(data)                             HDMI_STB_P0_HDCP_PCR_i2c_sda_dly_sel(data)
#define  HDMI_STB_HDCP_PCR_ddcdbnc(data)                                     HDMI_STB_P0_HDCP_PCR_ddcdbnc(data)
#define  HDMI_STB_HDCP_PCR_dvi_enc_mode(data)                                HDMI_STB_P0_HDCP_PCR_dvi_enc_mode(data)
#define  HDMI_STB_HDCP_PCR_dbnc_level_sel(data)                              HDMI_STB_P0_HDCP_PCR_dbnc_level_sel(data)
#define  HDMI_STB_HDCP_PCR_km_start_sel(data)                                HDMI_STB_P0_HDCP_PCR_km_start_sel(data)
#define  HDMI_STB_HDCP_PCR_apai_type(data)                                   HDMI_STB_P0_HDCP_PCR_apai_type(data)
#define  HDMI_STB_HDCP_PCR_apai(data)                                        HDMI_STB_P0_HDCP_PCR_apai(data)
#define  HDMI_STB_HDCP_PCR_get_iic_st(data)                                  HDMI_STB_P0_HDCP_PCR_get_iic_st(data)
#define  HDMI_STB_HDCP_PCR_get_i2c_scl_dly_sel(data)                         HDMI_STB_P0_HDCP_PCR_get_i2c_scl_dly_sel(data)
#define  HDMI_STB_HDCP_PCR_get_i2c_sda_dly_sel(data)                         HDMI_STB_P0_HDCP_PCR_get_i2c_sda_dly_sel(data)
#define  HDMI_STB_HDCP_PCR_get_ddcdbnc(data)                                 HDMI_STB_P0_HDCP_PCR_get_ddcdbnc(data)
#define  HDMI_STB_HDCP_PCR_get_dvi_enc_mode(data)                            HDMI_STB_P0_HDCP_PCR_get_dvi_enc_mode(data)
#define  HDMI_STB_HDCP_PCR_get_dbnc_level_sel(data)                          HDMI_STB_P0_HDCP_PCR_get_dbnc_level_sel(data)
#define  HDMI_STB_HDCP_PCR_get_km_start_sel(data)                            HDMI_STB_P0_HDCP_PCR_get_km_start_sel(data)
#define  HDMI_STB_HDCP_PCR_get_apai_type(data)                               HDMI_STB_P0_HDCP_PCR_get_apai_type(data)
#define  HDMI_STB_HDCP_PCR_get_apai(data)                                    HDMI_STB_P0_HDCP_PCR_get_apai(data)

#define  HDMI_STB_HDCP_FLAG1_akm_flag_mask                                   HDMI_STB_P0_HDCP_FLAG1_akm_flag_mask
#define  HDMI_STB_HDCP_FLAG1_adne_flag_mask                                  HDMI_STB_P0_HDCP_FLAG1_adne_flag_mask
#define  HDMI_STB_HDCP_FLAG1_ence_flag_mask                                  HDMI_STB_P0_HDCP_FLAG1_ence_flag_mask
#define  HDMI_STB_HDCP_FLAG1_nc_flag_mask                                    HDMI_STB_P0_HDCP_FLAG1_nc_flag_mask
#define  HDMI_STB_HDCP_FLAG1_wr_aksv_flag_mask                               HDMI_STB_P0_HDCP_FLAG1_wr_aksv_flag_mask
#define  HDMI_STB_HDCP_FLAG1_rd_bcap_flag_mask                               HDMI_STB_P0_HDCP_FLAG1_rd_bcap_flag_mask
#define  HDMI_STB_HDCP_FLAG1_rd_ri_flag_mask                                 HDMI_STB_P0_HDCP_FLAG1_rd_ri_flag_mask
#define  HDMI_STB_HDCP_FLAG1_rd_bksv_flag_mask                               HDMI_STB_P0_HDCP_FLAG1_rd_bksv_flag_mask
#define  HDMI_STB_HDCP_FLAG1_dummy_0_mask                                    HDMI_STB_P0_HDCP_FLAG1_dummy_0_mask
#define  HDMI_STB_HDCP_FLAG1_akm_flag(data)                                  HDMI_STB_P0_HDCP_FLAG1_akm_flag(data)
#define  HDMI_STB_HDCP_FLAG1_adne_flag(data)                                 HDMI_STB_P0_HDCP_FLAG1_adne_flag(data)
#define  HDMI_STB_HDCP_FLAG1_ence_flag(data)                                 HDMI_STB_P0_HDCP_FLAG1_ence_flag(data)
#define  HDMI_STB_HDCP_FLAG1_nc_flag(data)                                   HDMI_STB_P0_HDCP_FLAG1_nc_flag(data)
#define  HDMI_STB_HDCP_FLAG1_wr_aksv_flag(data)                              HDMI_STB_P0_HDCP_FLAG1_wr_aksv_flag(data)
#define  HDMI_STB_HDCP_FLAG1_rd_bcap_flag(data)                              HDMI_STB_P0_HDCP_FLAG1_rd_bcap_flag(data)
#define  HDMI_STB_HDCP_FLAG1_rd_ri_flag(data)                                HDMI_STB_P0_HDCP_FLAG1_rd_ri_flag(data)
#define  HDMI_STB_HDCP_FLAG1_rd_bksv_flag(data)                              HDMI_STB_P0_HDCP_FLAG1_rd_bksv_flag(data)
#define  HDMI_STB_HDCP_FLAG1_dummy_0(data)                                   HDMI_STB_P0_HDCP_FLAG1_dummy_0(data)
#define  HDMI_STB_HDCP_FLAG1_get_akm_flag(data)                              HDMI_STB_P0_HDCP_FLAG1_get_akm_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_adne_flag(data)                             HDMI_STB_P0_HDCP_FLAG1_get_adne_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_ence_flag(data)                             HDMI_STB_P0_HDCP_FLAG1_get_ence_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_nc_flag(data)                               HDMI_STB_P0_HDCP_FLAG1_get_nc_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_wr_aksv_flag(data)                          HDMI_STB_P0_HDCP_FLAG1_get_wr_aksv_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_rd_bcap_flag(data)                          HDMI_STB_P0_HDCP_FLAG1_get_rd_bcap_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_rd_ri_flag(data)                            HDMI_STB_P0_HDCP_FLAG1_get_rd_ri_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_rd_bksv_flag(data)                          HDMI_STB_P0_HDCP_FLAG1_get_rd_bksv_flag(data)
#define  HDMI_STB_HDCP_FLAG1_get_dummy_0(data)                               HDMI_STB_P0_HDCP_FLAG1_get_dummy_0(data)

#define  HDMI_STB_HDCP_FLAG2_irq_akm_en_mask                                 HDMI_STB_P0_HDCP_FLAG2_irq_akm_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_adne_en_mask                                HDMI_STB_P0_HDCP_FLAG2_irq_adne_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_ence_en_mask                                HDMI_STB_P0_HDCP_FLAG2_irq_ence_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_nc_en_mask                                  HDMI_STB_P0_HDCP_FLAG2_irq_nc_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_aksv_en_mask                                HDMI_STB_P0_HDCP_FLAG2_irq_aksv_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_bcap_en_mask                                HDMI_STB_P0_HDCP_FLAG2_irq_bcap_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_ri_en_mask                                  HDMI_STB_P0_HDCP_FLAG2_irq_ri_en_mask
#define  HDMI_STB_HDCP_FLAG2_irq_bksv_en_mask                                HDMI_STB_P0_HDCP_FLAG2_irq_bksv_en_mask
#define  HDMI_STB_HDCP_FLAG2_dummy_0_mask                                    HDMI_STB_P0_HDCP_FLAG2_dummy_0_mask
#define  HDMI_STB_HDCP_FLAG2_irq_akm_en(data)                                HDMI_STB_P0_HDCP_FLAG2_irq_akm_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_adne_en(data)                               HDMI_STB_P0_HDCP_FLAG2_irq_adne_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_ence_en(data)                               HDMI_STB_P0_HDCP_FLAG2_irq_ence_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_nc_en(data)                                 HDMI_STB_P0_HDCP_FLAG2_irq_nc_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_aksv_en(data)                               HDMI_STB_P0_HDCP_FLAG2_irq_aksv_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_bcap_en(data)                               HDMI_STB_P0_HDCP_FLAG2_irq_bcap_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_ri_en(data)                                 HDMI_STB_P0_HDCP_FLAG2_irq_ri_en(data)
#define  HDMI_STB_HDCP_FLAG2_irq_bksv_en(data)                               HDMI_STB_P0_HDCP_FLAG2_irq_bksv_en(data)
#define  HDMI_STB_HDCP_FLAG2_dummy_0(data)                                   HDMI_STB_P0_HDCP_FLAG2_dummy_0(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_akm_en(data)                            HDMI_STB_P0_HDCP_FLAG2_get_irq_akm_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_adne_en(data)                           HDMI_STB_P0_HDCP_FLAG2_get_irq_adne_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_ence_en(data)                           HDMI_STB_P0_HDCP_FLAG2_get_irq_ence_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_nc_en(data)                             HDMI_STB_P0_HDCP_FLAG2_get_irq_nc_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_aksv_en(data)                           HDMI_STB_P0_HDCP_FLAG2_get_irq_aksv_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_bcap_en(data)                           HDMI_STB_P0_HDCP_FLAG2_get_irq_bcap_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_ri_en(data)                             HDMI_STB_P0_HDCP_FLAG2_get_irq_ri_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_irq_bksv_en(data)                           HDMI_STB_P0_HDCP_FLAG2_get_irq_bksv_en(data)
#define  HDMI_STB_HDCP_FLAG2_get_dummy_0(data)                               HDMI_STB_P0_HDCP_FLAG2_get_dummy_0(data)

#define  HDMI_STB_HDCP_AP_ksvfifo_status_mask                                HDMI_STB_P0_HDCP_AP_ksvfifo_status_mask
#define  HDMI_STB_HDCP_AP_dp_ksvfifo_ptr_mask                                HDMI_STB_P0_HDCP_AP_dp_ksvfifo_ptr_mask
#define  HDMI_STB_HDCP_AP_ap1_mask                                           HDMI_STB_P0_HDCP_AP_ap1_mask
#define  HDMI_STB_HDCP_AP_ksvfifo_status(data)                               HDMI_STB_P0_HDCP_AP_ksvfifo_status(data)
#define  HDMI_STB_HDCP_AP_dp_ksvfifo_ptr(data)                               HDMI_STB_P0_HDCP_AP_dp_ksvfifo_ptr(data)
#define  HDMI_STB_HDCP_AP_ap1(data)                                          HDMI_STB_P0_HDCP_AP_ap1(data)
#define  HDMI_STB_HDCP_AP_get_ksvfifo_status(data)                           HDMI_STB_P0_HDCP_AP_get_ksvfifo_status(data)
#define  HDMI_STB_HDCP_AP_get_dp_ksvfifo_ptr(data)                           HDMI_STB_P0_HDCP_AP_get_dp_ksvfifo_ptr(data)
#define  HDMI_STB_HDCP_AP_get_ap1(data)                                      HDMI_STB_P0_HDCP_AP_get_ap1(data)

#define  HDMI_STB_HDCP_DP_dp1_mask                                           HDMI_STB_P0_HDCP_DP_dp1_mask
#define  HDMI_STB_HDCP_DP_dp1(data)                                          HDMI_STB_P0_HDCP_DP_dp1(data)
#define  HDMI_STB_HDCP_DP_get_dp1(data)                                      HDMI_STB_P0_HDCP_DP_get_dp1(data)

#define  HDMI_STB_HDCP_2p2_CR_enable_reauth_mask                             HDMI_STB_P0_HDCP_2p2_CR_enable_reauth_mask
#define  HDMI_STB_HDCP_2p2_CR_switch_state_mask                              HDMI_STB_P0_HDCP_2p2_CR_switch_state_mask
#define  HDMI_STB_HDCP_2p2_CR_rpt_thr_unauth_mask                            HDMI_STB_P0_HDCP_2p2_CR_rpt_thr_unauth_mask
#define  HDMI_STB_HDCP_2p2_CR_apply_state_mask                               HDMI_STB_P0_HDCP_2p2_CR_apply_state_mask
#define  HDMI_STB_HDCP_2p2_CR_mspai_mask                                     HDMI_STB_P0_HDCP_2p2_CR_mspai_mask
#define  HDMI_STB_HDCP_2p2_CR_new_auth_device_mask                           HDMI_STB_P0_HDCP_2p2_CR_new_auth_device_mask
#define  HDMI_STB_HDCP_2p2_CR_verify_id_pass_mask                            HDMI_STB_P0_HDCP_2p2_CR_verify_id_pass_mask
#define  HDMI_STB_HDCP_2p2_CR_verify_id_done_mask                            HDMI_STB_P0_HDCP_2p2_CR_verify_id_done_mask
#define  HDMI_STB_HDCP_2p2_CR_assemble_id_fail_mask                          HDMI_STB_P0_HDCP_2p2_CR_assemble_id_fail_mask
#define  HDMI_STB_HDCP_2p2_CR_downstream_done_mask                           HDMI_STB_P0_HDCP_2p2_CR_downstream_done_mask
#define  HDMI_STB_HDCP_2p2_CR_ks_done_mask                                   HDMI_STB_P0_HDCP_2p2_CR_ks_done_mask
#define  HDMI_STB_HDCP_2p2_CR_switch_unauth_mask                             HDMI_STB_P0_HDCP_2p2_CR_switch_unauth_mask
#define  HDMI_STB_HDCP_2p2_CR_hdcp_2p2_en_mask                               HDMI_STB_P0_HDCP_2p2_CR_hdcp_2p2_en_mask
#define  HDMI_STB_HDCP_2p2_CR_enable_reauth(data)                            HDMI_STB_P0_HDCP_2p2_CR_enable_reauth(data)
#define  HDMI_STB_HDCP_2p2_CR_switch_state(data)                             HDMI_STB_P0_HDCP_2p2_CR_switch_state(data)
#define  HDMI_STB_HDCP_2p2_CR_rpt_thr_unauth(data)                           HDMI_STB_P0_HDCP_2p2_CR_rpt_thr_unauth(data)
#define  HDMI_STB_HDCP_2p2_CR_apply_state(data)                              HDMI_STB_P0_HDCP_2p2_CR_apply_state(data)
#define  HDMI_STB_HDCP_2p2_CR_mspai(data)                                    HDMI_STB_P0_HDCP_2p2_CR_mspai(data)
#define  HDMI_STB_HDCP_2p2_CR_new_auth_device(data)                          HDMI_STB_P0_HDCP_2p2_CR_new_auth_device(data)
#define  HDMI_STB_HDCP_2p2_CR_verify_id_pass(data)                           HDMI_STB_P0_HDCP_2p2_CR_verify_id_pass(data)
#define  HDMI_STB_HDCP_2p2_CR_verify_id_done(data)                           HDMI_STB_P0_HDCP_2p2_CR_verify_id_done(data)
#define  HDMI_STB_HDCP_2p2_CR_assemble_id_fail(data)                         HDMI_STB_P0_HDCP_2p2_CR_assemble_id_fail(data)
#define  HDMI_STB_HDCP_2p2_CR_downstream_done(data)                          HDMI_STB_P0_HDCP_2p2_CR_downstream_done(data)
#define  HDMI_STB_HDCP_2p2_CR_ks_done(data)                                  HDMI_STB_P0_HDCP_2p2_CR_ks_done(data)
#define  HDMI_STB_HDCP_2p2_CR_switch_unauth(data)                            HDMI_STB_P0_HDCP_2p2_CR_switch_unauth(data)
#define  HDMI_STB_HDCP_2p2_CR_hdcp_2p2_en(data)                              HDMI_STB_P0_HDCP_2p2_CR_hdcp_2p2_en(data)
#define  HDMI_STB_HDCP_2p2_CR_get_enable_reauth(data)                        HDMI_STB_P0_HDCP_2p2_CR_get_enable_reauth(data)
#define  HDMI_STB_HDCP_2p2_CR_get_switch_state(data)                         HDMI_STB_P0_HDCP_2p2_CR_get_switch_state(data)
#define  HDMI_STB_HDCP_2p2_CR_get_rpt_thr_unauth(data)                       HDMI_STB_P0_HDCP_2p2_CR_get_rpt_thr_unauth(data)
#define  HDMI_STB_HDCP_2p2_CR_get_apply_state(data)                          HDMI_STB_P0_HDCP_2p2_CR_get_apply_state(data)
#define  HDMI_STB_HDCP_2p2_CR_get_mspai(data)                                HDMI_STB_P0_HDCP_2p2_CR_get_mspai(data)
#define  HDMI_STB_HDCP_2p2_CR_get_new_auth_device(data)                      HDMI_STB_P0_HDCP_2p2_CR_get_new_auth_device(data)
#define  HDMI_STB_HDCP_2p2_CR_get_verify_id_pass(data)                       HDMI_STB_P0_HDCP_2p2_CR_get_verify_id_pass(data)
#define  HDMI_STB_HDCP_2p2_CR_get_verify_id_done(data)                       HDMI_STB_P0_HDCP_2p2_CR_get_verify_id_done(data)
#define  HDMI_STB_HDCP_2p2_CR_get_assemble_id_fail(data)                     HDMI_STB_P0_HDCP_2p2_CR_get_assemble_id_fail(data)
#define  HDMI_STB_HDCP_2p2_CR_get_downstream_done(data)                      HDMI_STB_P0_HDCP_2p2_CR_get_downstream_done(data)
#define  HDMI_STB_HDCP_2p2_CR_get_ks_done(data)                              HDMI_STB_P0_HDCP_2p2_CR_get_ks_done(data)
#define  HDMI_STB_HDCP_2p2_CR_get_switch_unauth(data)                        HDMI_STB_P0_HDCP_2p2_CR_get_switch_unauth(data)
#define  HDMI_STB_HDCP_2p2_CR_get_hdcp_2p2_en(data)                          HDMI_STB_P0_HDCP_2p2_CR_get_hdcp_2p2_en(data)

#define  HDMI_STB_HDCP_2p2_SR0_irq_msg_overwrite_mask                        HDMI_STB_P0_HDCP_2p2_SR0_irq_msg_overwrite_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_unauth_chg_mask                           HDMI_STB_P0_HDCP_2p2_SR0_irq_unauth_chg_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_state_chg_mask                            HDMI_STB_P0_HDCP_2p2_SR0_irq_state_chg_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done_mask                          HDMI_STB_P0_HDCP_2p2_SR0_irq_rd_msg_done_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_done_mask                          HDMI_STB_P0_HDCP_2p2_SR0_irq_wr_msg_done_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_start_mask                         HDMI_STB_P0_HDCP_2p2_SR0_irq_rd_msg_start_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_start_mask                         HDMI_STB_P0_HDCP_2p2_SR0_irq_wr_msg_start_mask
#define  HDMI_STB_HDCP_2p2_SR0_state_mask                                    HDMI_STB_P0_HDCP_2p2_SR0_state_mask
#define  HDMI_STB_HDCP_2p2_SR0_irq_msg_overwrite(data)                       HDMI_STB_P0_HDCP_2p2_SR0_irq_msg_overwrite(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_unauth_chg(data)                          HDMI_STB_P0_HDCP_2p2_SR0_irq_unauth_chg(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_state_chg(data)                           HDMI_STB_P0_HDCP_2p2_SR0_irq_state_chg(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_done(data)                         HDMI_STB_P0_HDCP_2p2_SR0_irq_rd_msg_done(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_done(data)                         HDMI_STB_P0_HDCP_2p2_SR0_irq_wr_msg_done(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_rd_msg_start(data)                        HDMI_STB_P0_HDCP_2p2_SR0_irq_rd_msg_start(data)
#define  HDMI_STB_HDCP_2p2_SR0_irq_wr_msg_start(data)                        HDMI_STB_P0_HDCP_2p2_SR0_irq_wr_msg_start(data)
#define  HDMI_STB_HDCP_2p2_SR0_state(data)                                   HDMI_STB_P0_HDCP_2p2_SR0_state(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_msg_overwrite(data)                   HDMI_STB_P0_HDCP_2p2_SR0_get_irq_msg_overwrite(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_unauth_chg(data)                      HDMI_STB_P0_HDCP_2p2_SR0_get_irq_unauth_chg(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_state_chg(data)                       HDMI_STB_P0_HDCP_2p2_SR0_get_irq_state_chg(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_rd_msg_done(data)                     HDMI_STB_P0_HDCP_2p2_SR0_get_irq_rd_msg_done(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_wr_msg_done(data)                     HDMI_STB_P0_HDCP_2p2_SR0_get_irq_wr_msg_done(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_rd_msg_start(data)                    HDMI_STB_P0_HDCP_2p2_SR0_get_irq_rd_msg_start(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_irq_wr_msg_start(data)                    HDMI_STB_P0_HDCP_2p2_SR0_get_irq_wr_msg_start(data)
#define  HDMI_STB_HDCP_2p2_SR0_get_state(data)                               HDMI_STB_P0_HDCP_2p2_SR0_get_state(data)


#define  HDMI_STB_HDCP_2p2_SR1_irq_msg_overwrite_en_mask                     HDMI_STB_P0_HDCP_2p2_SR1_irq_msg_overwrite_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_unauth_chg_en_mask                        HDMI_STB_P0_HDCP_2p2_SR1_irq_unauth_chg_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_state_chg_en_mask                         HDMI_STB_P0_HDCP_2p2_SR1_irq_state_chg_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_done_en_mask                       HDMI_STB_P0_HDCP_2p2_SR1_irq_rd_msg_done_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en_mask                       HDMI_STB_P0_HDCP_2p2_SR1_irq_wr_msg_done_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_start_en_mask                      HDMI_STB_P0_HDCP_2p2_SR1_irq_rd_msg_start_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_start_en_mask                      HDMI_STB_P0_HDCP_2p2_SR1_irq_wr_msg_start_en_mask
#define  HDMI_STB_HDCP_2p2_SR1_irq_msg_overwrite_en(data)                    HDMI_STB_P0_HDCP_2p2_SR1_irq_msg_overwrite_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_unauth_chg_en(data)                       HDMI_STB_P0_HDCP_2p2_SR1_irq_unauth_chg_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_state_chg_en(data)                        HDMI_STB_P0_HDCP_2p2_SR1_irq_state_chg_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_done_en(data)                      HDMI_STB_P0_HDCP_2p2_SR1_irq_rd_msg_done_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_done_en(data)                      HDMI_STB_P0_HDCP_2p2_SR1_irq_wr_msg_done_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_rd_msg_start_en(data)                     HDMI_STB_P0_HDCP_2p2_SR1_irq_rd_msg_start_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_irq_wr_msg_start_en(data)                     HDMI_STB_P0_HDCP_2p2_SR1_irq_wr_msg_start_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_msg_overwrite_en(data)                HDMI_STB_P0_HDCP_2p2_SR1_get_irq_msg_overwrite_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_unauth_chg_en(data)                   HDMI_STB_P0_HDCP_2p2_SR1_get_irq_unauth_chg_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_state_chg_en(data)                    HDMI_STB_P0_HDCP_2p2_SR1_get_irq_state_chg_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_rd_msg_done_en(data)                  HDMI_STB_P0_HDCP_2p2_SR1_get_irq_rd_msg_done_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_wr_msg_done_en(data)                  HDMI_STB_P0_HDCP_2p2_SR1_get_irq_wr_msg_done_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_rd_msg_start_en(data)                 HDMI_STB_P0_HDCP_2p2_SR1_get_irq_rd_msg_start_en(data)
#define  HDMI_STB_HDCP_2p2_SR1_get_irq_wr_msg_start_en(data)                 HDMI_STB_P0_HDCP_2p2_SR1_get_irq_wr_msg_start_en(data)

#define  HDMI_STB_HDCP_MSAP_ap1_mask                                         HDMI_STB_P0_HDCP_MSAP_ap1_mask
#define  HDMI_STB_HDCP_MSAP_ap1(data)                                        HDMI_STB_P0_HDCP_MSAP_ap1(data)
#define  HDMI_STB_HDCP_MSAP_get_ap1(data)                                    HDMI_STB_P0_HDCP_MSAP_get_ap1(data)

#define  HDMI_STB_HDCP_MSDP_dp1_mask                                         HDMI_STB_P0_HDCP_MSDP_dp1_mask
#define  HDMI_STB_HDCP_MSDP_dp1(data)                                        HDMI_STB_P0_HDCP_MSDP_dp1(data)
#define  HDMI_STB_HDCP_MSDP_get_dp1(data)                                    HDMI_STB_P0_HDCP_MSDP_get_dp1(data)

#define  HDMI_STB_HDCP_PEND_hdcp1p4_mask                                     HDMI_STB_P0_HDCP_PEND_hdcp1p4_mask
#define  HDMI_STB_HDCP_PEND_hdcp2p2_mask                                     HDMI_STB_P0_HDCP_PEND_hdcp2p2_mask
#define  HDMI_STB_HDCP_PEND_hdcp1p4(data)                                    HDMI_STB_P0_HDCP_PEND_hdcp1p4(data)
#define  HDMI_STB_HDCP_PEND_hdcp2p2(data)                                    HDMI_STB_P0_HDCP_PEND_hdcp2p2(data)
#define  HDMI_STB_HDCP_PEND_get_hdcp1p4(data)                                HDMI_STB_P0_HDCP_PEND_get_hdcp1p4(data)
#define  HDMI_STB_HDCP_PEND_get_hdcp2p2(data)                                HDMI_STB_P0_HDCP_PEND_get_hdcp2p2(data)

#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail_mask                           HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail0_mask                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail0_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail1_mask                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail1_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_fail_mask                       HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_fail_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_pause_mask                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_pause_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_pause_mask                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_pause_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_done_mask                           HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_done_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_done_mask                       HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_done_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_resume_mask                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_resume_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_resume_mask                     HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_resume_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_mode_mask                           HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_mode_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_mode_mask                       HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_mode_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail_mask                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail0_mask                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail0_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail1_mask                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail1_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_fail_mask                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_fail_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_done_mask                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_done_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_done_mask                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_done_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_en_mask                            HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_en_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_en_mask                        HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_en_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_rme1_mask                               HDMI_STB_P0_MBIST_ST0_hdcp_2p2_rme1_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_rme0_mask                               HDMI_STB_P0_MBIST_ST0_hdcp_2p2_rme0_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_rme_mask                            HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_rme_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_ls1_mask                                HDMI_STB_P0_MBIST_ST0_hdcp_2p2_ls1_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_ls0_mask                                HDMI_STB_P0_MBIST_ST0_hdcp_2p2_ls0_mask
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_ls_mask                             HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_ls_mask
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail(data)                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail0(data)                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail0(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_fail1(data)                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_fail1(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_fail(data)                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_fail(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_pause(data)                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_pause(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_pause(data)                     HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_pause(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_done(data)                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_done(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_done(data)                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_done(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_resume(data)                        HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_resume(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_resume(data)                    HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_resume(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_drf_mode(data)                          HDMI_STB_P0_MBIST_ST0_hdcp_2p2_drf_mode(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_drf_mode(data)                      HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_drf_mode(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail(data)                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail0(data)                        HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail0(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_fail1(data)                        HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_fail1(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_fail(data)                     HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_fail(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_done(data)                         HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_done(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_done(data)                     HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_done(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_bist_en(data)                           HDMI_STB_P0_MBIST_ST0_hdcp_2p2_bist_en(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_bist_en(data)                       HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_bist_en(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_rme1(data)                              HDMI_STB_P0_MBIST_ST0_hdcp_2p2_rme1(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_rme0(data)                              HDMI_STB_P0_MBIST_ST0_hdcp_2p2_rme0(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_rme(data)                           HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_rme(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_ls1(data)                               HDMI_STB_P0_MBIST_ST0_hdcp_2p2_ls1(data)
#define  HDMI_STB_MBIST_ST0_hdcp_2p2_ls0(data)                               HDMI_STB_P0_MBIST_ST0_hdcp_2p2_ls0(data)
#define  HDMI_STB_MBIST_ST0_hdcp_ksvfifo_ls(data)                            HDMI_STB_P0_MBIST_ST0_hdcp_ksvfifo_ls(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_fail(data)                      HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_fail(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_fail0(data)                     HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_fail0(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_fail1(data)                     HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_fail1(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_drf_fail(data)                  HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_drf_fail(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_pause(data)                     HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_pause(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_drf_pause(data)                 HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_drf_pause(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_done(data)                      HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_done(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_drf_done(data)                  HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_drf_done(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_resume(data)                    HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_resume(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_drf_resume(data)                HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_drf_resume(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_drf_mode(data)                      HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_drf_mode(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_drf_mode(data)                  HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_drf_mode(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_bist_fail(data)                     HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_bist_fail(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_bist_fail0(data)                    HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_bist_fail0(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_bist_fail1(data)                    HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_bist_fail1(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_bist_fail(data)                 HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_bist_fail(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_bist_done(data)                     HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_bist_done(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_bist_done(data)                 HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_bist_done(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_bist_en(data)                       HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_bist_en(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_bist_en(data)                   HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_bist_en(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_rme1(data)                          HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_rme1(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_rme0(data)                          HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_rme0(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_rme(data)                       HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_rme(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_ls1(data)                           HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_ls1(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_2p2_ls0(data)                           HDMI_STB_P0_MBIST_ST0_get_hdcp_2p2_ls0(data)
#define  HDMI_STB_MBIST_ST0_get_hdcp_ksvfifo_ls(data)                        HDMI_STB_P0_MBIST_ST0_get_hdcp_ksvfifo_ls(data)

#define  HDMI_STB_MBIST_ST1_hdcp_2p2_test1_1_mask                            HDMI_STB_P0_MBIST_ST1_hdcp_2p2_test1_1_mask
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_test1_0_mask                            HDMI_STB_P0_MBIST_ST1_hdcp_2p2_test1_0_mask
#define  HDMI_STB_MBIST_ST1_hdcp_ksvfifo_test1_0_mask                        HDMI_STB_P0_MBIST_ST1_hdcp_ksvfifo_test1_0_mask
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_rm1_mask                                HDMI_STB_P0_MBIST_ST1_hdcp_2p2_rm1_mask
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_rm0_mask                                HDMI_STB_P0_MBIST_ST1_hdcp_2p2_rm0_mask
#define  HDMI_STB_MBIST_ST1_hdcp_ksvfifo_rm_mask                             HDMI_STB_P0_MBIST_ST1_hdcp_ksvfifo_rm_mask
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_test1_1(data)                           HDMI_STB_P0_MBIST_ST1_hdcp_2p2_test1_1(data)
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_test1_0(data)                           HDMI_STB_P0_MBIST_ST1_hdcp_2p2_test1_0(data)
#define  HDMI_STB_MBIST_ST1_hdcp_ksvfifo_test1_0(data)                       HDMI_STB_P0_MBIST_ST1_hdcp_ksvfifo_test1_0(data)
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_rm1(data)                               HDMI_STB_P0_MBIST_ST1_hdcp_2p2_rm1(data)
#define  HDMI_STB_MBIST_ST1_hdcp_2p2_rm0(data)                               HDMI_STB_P0_MBIST_ST1_hdcp_2p2_rm0(data)
#define  HDMI_STB_MBIST_ST1_hdcp_ksvfifo_rm(data)                            HDMI_STB_P0_MBIST_ST1_hdcp_ksvfifo_rm(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_2p2_test1_1(data)                       HDMI_STB_P0_MBIST_ST1_get_hdcp_2p2_test1_1(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_2p2_test1_0(data)                       HDMI_STB_P0_MBIST_ST1_get_hdcp_2p2_test1_0(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_ksvfifo_test1_0(data)                   HDMI_STB_P0_MBIST_ST1_get_hdcp_ksvfifo_test1_0(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_2p2_rm1(data)                           HDMI_STB_P0_MBIST_ST1_get_hdcp_2p2_rm1(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_2p2_rm0(data)                           HDMI_STB_P0_MBIST_ST1_get_hdcp_2p2_rm0(data)
#define  HDMI_STB_MBIST_ST1_get_hdcp_ksvfifo_rm(data)                        HDMI_STB_P0_MBIST_ST1_get_hdcp_ksvfifo_rm(data)

#define  HDMI_STB_MBIST_ST2_hdcp_drf_fail_mask                               HDMI_STB_P0_MBIST_ST2_hdcp_drf_fail_mask
#define  HDMI_STB_MBIST_ST2_hdcp_drf_pause_mask                              HDMI_STB_P0_MBIST_ST2_hdcp_drf_pause_mask
#define  HDMI_STB_MBIST_ST2_hdcp_drf_done_mask                               HDMI_STB_P0_MBIST_ST2_hdcp_drf_done_mask
#define  HDMI_STB_MBIST_ST2_hdcp_drf_resume_mask                             HDMI_STB_P0_MBIST_ST2_hdcp_drf_resume_mask
#define  HDMI_STB_MBIST_ST2_hdcp_drf_mode_mask                               HDMI_STB_P0_MBIST_ST2_hdcp_drf_mode_mask
#define  HDMI_STB_MBIST_ST2_hdcp_bist_fail_mask                              HDMI_STB_P0_MBIST_ST2_hdcp_bist_fail_mask
#define  HDMI_STB_MBIST_ST2_hdcp_bist_done_mask                              HDMI_STB_P0_MBIST_ST2_hdcp_bist_done_mask
#define  HDMI_STB_MBIST_ST2_hdcp_bist_en_mask                                HDMI_STB_P0_MBIST_ST2_hdcp_bist_en_mask
#define  HDMI_STB_MBIST_ST2_hdcp_rme_mask                                    HDMI_STB_P0_MBIST_ST2_hdcp_rme_mask
#define  HDMI_STB_MBIST_ST2_hdcp_ls_mask                                     HDMI_STB_P0_MBIST_ST2_hdcp_ls_mask
#define  HDMI_STB_MBIST_ST2_hdcp_drf_fail(data)                              HDMI_STB_P0_MBIST_ST2_hdcp_drf_fail(data)
#define  HDMI_STB_MBIST_ST2_hdcp_drf_pause(data)                             HDMI_STB_P0_MBIST_ST2_hdcp_drf_pause(data)
#define  HDMI_STB_MBIST_ST2_hdcp_drf_done(data)                              HDMI_STB_P0_MBIST_ST2_hdcp_drf_done(data)
#define  HDMI_STB_MBIST_ST2_hdcp_drf_resume(data)                            HDMI_STB_P0_MBIST_ST2_hdcp_drf_resume(data)
#define  HDMI_STB_MBIST_ST2_hdcp_drf_mode(data)                              HDMI_STB_P0_MBIST_ST2_hdcp_drf_mode(data)
#define  HDMI_STB_MBIST_ST2_hdcp_bist_fail(data)                             HDMI_STB_P0_MBIST_ST2_hdcp_bist_fail(data)
#define  HDMI_STB_MBIST_ST2_hdcp_bist_done(data)                             HDMI_STB_P0_MBIST_ST2_hdcp_bist_done(data)
#define  HDMI_STB_MBIST_ST2_hdcp_bist_en(data)                               HDMI_STB_P0_MBIST_ST2_hdcp_bist_en(data)
#define  HDMI_STB_MBIST_ST2_hdcp_rme(data)                                   HDMI_STB_P0_MBIST_ST2_hdcp_rme(data)
#define  HDMI_STB_MBIST_ST2_hdcp_ls(data)                                    HDMI_STB_P0_MBIST_ST2_hdcp_ls(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_drf_fail(data)                          HDMI_STB_P0_MBIST_ST2_get_hdcp_drf_fail(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_drf_pause(data)                         HDMI_STB_P0_MBIST_ST2_get_hdcp_drf_pause(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_drf_done(data)                          HDMI_STB_P0_MBIST_ST2_get_hdcp_drf_done(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_drf_resume(data)                        HDMI_STB_P0_MBIST_ST2_get_hdcp_drf_resume(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_drf_mode(data)                          HDMI_STB_P0_MBIST_ST2_get_hdcp_drf_mode(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_bist_fail(data)                         HDMI_STB_P0_MBIST_ST2_get_hdcp_bist_fail(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_bist_done(data)                         HDMI_STB_P0_MBIST_ST2_get_hdcp_bist_done(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_bist_en(data)                           HDMI_STB_P0_MBIST_ST2_get_hdcp_bist_en(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_rme(data)                               HDMI_STB_P0_MBIST_ST2_get_hdcp_rme(data)
#define  HDMI_STB_MBIST_ST2_get_hdcp_ls(data)                                HDMI_STB_P0_MBIST_ST2_get_hdcp_ls(data)

#define  HDMI_STB_MBIST_ST3_hdcp_test1_0_mask                                HDMI_STB_P0_MBIST_ST3_hdcp_test1_0_mask
#define  HDMI_STB_MBIST_ST3_hdcp_rm_mask                                     HDMI_STB_P0_MBIST_ST3_hdcp_rm_mask
#define  HDMI_STB_MBIST_ST3_hdcp_test1_0(data)                               HDMI_STB_P0_MBIST_ST3_hdcp_test1_0(data)
#define  HDMI_STB_MBIST_ST3_hdcp_rm(data)                                    HDMI_STB_P0_MBIST_ST3_hdcp_rm(data)
#define  HDMI_STB_MBIST_ST3_get_hdcp_test1_0(data)                           HDMI_STB_P0_MBIST_ST3_get_hdcp_test1_0(data)
#define  HDMI_STB_MBIST_ST3_get_hdcp_rm(data)                                HDMI_STB_P0_MBIST_ST3_get_hdcp_rm(data)

#define  HDMI_STB_stb_xtal_1ms_cnt_mask                                      HDMI_STB_P0_stb_xtal_1ms_cnt_mask
#define  HDMI_STB_stb_xtal_1ms_cnt(data)                                     HDMI_STB_P0_stb_xtal_1ms_cnt(data)
#define  HDMI_STB_stb_xtal_1ms_get_cnt(data)                                 HDMI_STB_P0_stb_xtal_1ms_get_cnt(data)

#define  HDMI_STB_stb_xtal_4p7us_cnt_mask                                    HDMI_STB_P0_stb_xtal_4p7us_cnt_mask
#define  HDMI_STB_stb_xtal_4p7us_cnt(data)                                   HDMI_STB_P0_stb_xtal_4p7us_cnt(data)
#define  HDMI_STB_stb_xtal_4p7us_get_cnt(data)                               HDMI_STB_P0_stb_xtal_4p7us_get_cnt(data)

#define  HDMI_STB_HDMI_STB_DBGBOX_CTL_dbg_sel_mask                           HDMI_STB_P0_HDMI_STB_DBGBOX_CTL_dbg_sel_mask
#define  HDMI_STB_HDMI_STB_DBGBOX_CTL_dbg_sel(data)                          HDMI_STB_P0_HDMI_STB_DBGBOX_CTL_dbg_sel(data)
#define  HDMI_STB_HDMI_STB_DBGBOX_CTL_get_dbg_sel(data)                      HDMI_STB_P0_HDMI_STB_DBGBOX_CTL_get_dbg_sel(data)

#define  HDMI_STB_CLKDETSR_reset_counter_mask                                HDMI_STB_P0_CLKDETSR_reset_counter_mask
#define  HDMI_STB_CLKDETSR_pop_out_mask                                      HDMI_STB_P0_CLKDETSR_pop_out_mask
#define  HDMI_STB_CLKDETSR_clk_counter_mask                                  HDMI_STB_P0_CLKDETSR_clk_counter_mask
#define  HDMI_STB_CLKDETSR_clock_det_en_mask                                 HDMI_STB_P0_CLKDETSR_clock_det_en_mask
#define  HDMI_STB_CLKDETSR_en_tmds_chg_irq_mask                              HDMI_STB_P0_CLKDETSR_en_tmds_chg_irq_mask
#define  HDMI_STB_CLKDETSR_tmds_chg_irq_mask                                 HDMI_STB_P0_CLKDETSR_tmds_chg_irq_mask
#define  HDMI_STB_CLKDETSR_dummy_3_2_mask                                    HDMI_STB_P0_CLKDETSR_dummy_3_2_mask
#define  HDMI_STB_CLKDETSR_clk_in_target_irq_en_mask                         HDMI_STB_P0_CLKDETSR_clk_in_target_irq_en_mask
#define  HDMI_STB_CLKDETSR_clk_in_target_mask                                HDMI_STB_P0_CLKDETSR_clk_in_target_mask
#define  HDMI_STB_CLKDETSR_reset_counter(data)                               HDMI_STB_P0_CLKDETSR_reset_counter(data)
#define  HDMI_STB_CLKDETSR_pop_out(data)                                     HDMI_STB_P0_CLKDETSR_pop_out(data)
#define  HDMI_STB_CLKDETSR_clk_counter(data)                                 HDMI_STB_P0_CLKDETSR_clk_counter(data)
#define  HDMI_STB_CLKDETSR_clock_det_en(data)                                HDMI_STB_P0_CLKDETSR_clock_det_en(data)
#define  HDMI_STB_CLKDETSR_en_tmds_chg_irq(data)                             HDMI_STB_P0_CLKDETSR_en_tmds_chg_irq(data)
#define  HDMI_STB_CLKDETSR_tmds_chg_irq(data)                                HDMI_STB_P0_CLKDETSR_tmds_chg_irq(data)
#define  HDMI_STB_CLKDETSR_dummy_3_2(data)                                   HDMI_STB_P0_CLKDETSR_dummy_3_2(data)
#define  HDMI_STB_CLKDETSR_clk_in_target_irq_en(data)                        HDMI_STB_P0_CLKDETSR_clk_in_target_irq_en(data)
#define  HDMI_STB_CLKDETSR_clk_in_target(data)                               HDMI_STB_P0_CLKDETSR_clk_in_target(data)
#define  HDMI_STB_CLKDETSR_get_reset_counter(data)                           HDMI_STB_P0_CLKDETSR_get_reset_counter(data)
#define  HDMI_STB_CLKDETSR_get_pop_out(data)                                 HDMI_STB_P0_CLKDETSR_get_pop_out(data)
#define  HDMI_STB_CLKDETSR_get_clk_counter(data)                             HDMI_STB_P0_CLKDETSR_get_clk_counter(data)
#define  HDMI_STB_CLKDETSR_get_clock_det_en(data)                            HDMI_STB_P0_CLKDETSR_get_clock_det_en(data)
#define  HDMI_STB_CLKDETSR_get_en_tmds_chg_irq(data)                         HDMI_STB_P0_CLKDETSR_get_en_tmds_chg_irq(data)
#define  HDMI_STB_CLKDETSR_get_tmds_chg_irq(data)                            HDMI_STB_P0_CLKDETSR_get_tmds_chg_irq(data)
#define  HDMI_STB_CLKDETSR_get_dummy_3_2(data)                               HDMI_STB_P0_CLKDETSR_get_dummy_3_2(data)
#define  HDMI_STB_CLKDETSR_get_clk_in_target_irq_en(data)                    HDMI_STB_P0_CLKDETSR_get_clk_in_target_irq_en(data)
#define  HDMI_STB_CLKDETSR_get_clk_in_target(data)                           HDMI_STB_P0_CLKDETSR_get_clk_in_target(data)

#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_dclk_cnt_start_mask                 HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_dclk_cnt_start_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_dclk_cnt_end_mask                   HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_dclk_cnt_end_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_dclk_cnt_start(data)                HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_dclk_cnt_start(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_dclk_cnt_end(data)                  HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_dclk_cnt_end(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_get_dclk_cnt_start(data)            HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_get_dclk_cnt_start(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_00_get_dclk_cnt_end(data)              HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_get_dclk_cnt_end(data)

#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_target_for_maximum_clk_counter_mask        HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_target_for_maximum_clk_counter_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_target_for_minimum_clk_counter_mask        HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_target_for_minimum_clk_counter_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_target_for_maximum_clk_counter(data)       HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_target_for_maximum_clk_counter(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_target_for_minimum_clk_counter(data)       HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_target_for_minimum_clk_counter(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_get_target_for_maximum_clk_counter(data)   HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_get_target_for_maximum_clk_counter(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_01_get_target_for_minimum_clk_counter(data)   HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_get_target_for_minimum_clk_counter(data)

#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_clk_counter_err_threshold_mask      HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_clk_counter_err_threshold_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_clk_counter_debounce_mask           HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_clk_counter_debounce_mask
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_clk_counter_err_threshold(data)     HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_clk_counter_err_threshold(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_clk_counter_debounce(data)          HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_clk_counter_debounce(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_get_clk_counter_err_threshold(data) HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_get_clk_counter_err_threshold(data)
#define  HDMI_STB_GDI_TMDSCLK_SETTING_02_get_clk_counter_debounce(data)      HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_get_clk_counter_debounce(data)

#define  HDMI_STB_ot_port_sel_mask                                           HDMI_STB_P0_ot_port_sel_mask
#define  HDMI_STB_ot_ddc_ch3_sel_mask                                        HDMI_STB_P0_ot_ddc_ch3_sel_mask
#define  HDMI_STB_ot_ddc_ch2_sel_mask                                        HDMI_STB_P0_ot_ddc_ch2_sel_mask
#define  HDMI_STB_ot_ddc_ch1_sel_mask                                        HDMI_STB_P0_ot_ddc_ch1_sel_mask
#define  HDMI_STB_ot_ddc_ch0_sel_mask                                        HDMI_STB_P0_ot_ddc_ch0_sel_mask
#define  HDMI_STB_ot_port_sel(data)                                          HDMI_STB_P0_ot_port_sel(data)
#define  HDMI_STB_ot_ddc_ch3_sel(data)                                       HDMI_STB_P0_ot_ddc_ch3_sel(data)
#define  HDMI_STB_ot_ddc_ch2_sel(data)                                       HDMI_STB_P0_ot_ddc_ch2_sel(data)
#define  HDMI_STB_ot_ddc_ch1_sel(data)                                       HDMI_STB_P0_ot_ddc_ch1_sel(data)
#define  HDMI_STB_ot_ddc_ch0_sel(data)                                       HDMI_STB_P0_ot_ddc_ch0_sel(data)
#define  HDMI_STB_ot_get_port_sel(data)                                      HDMI_STB_P0_ot_get_port_sel(data)
#define  HDMI_STB_ot_get_ddc_ch3_sel(data)                                   HDMI_STB_P0_ot_get_ddc_ch3_sel(data)
#define  HDMI_STB_ot_get_ddc_ch2_sel(data)                                   HDMI_STB_P0_ot_get_ddc_ch2_sel(data)
#define  HDMI_STB_ot_get_ddc_ch1_sel(data)                                   HDMI_STB_P0_ot_get_ddc_ch1_sel(data)
#define  HDMI_STB_ot_get_ddc_ch0_sel(data)                                   HDMI_STB_P0_ot_get_ddc_ch0_sel(data)

#define  HDMI_STB_HDMI_STB_DBGBOX_PORT_SEL_hdmi_stb_test_sel_mask            HDMI_STB_P0_HDMI_STB_DBGBOX_PORT_SEL_hdmi_stb_test_sel_mask
#define  HDMI_STB_HDMI_STB_DBGBOX_PORT_SEL_hdmi_stb_test_sel(data)           HDMI_STB_P0_HDMI_STB_DBGBOX_PORT_SEL_hdmi_stb_test_sel(data)
#define  HDMI_STB_HDMI_STB_DBGBOX_PORT_SEL_get_hdmi_stb_test_sel(data)       HDMI_STB_P0_HDMI_STB_DBGBOX_PORT_SEL_get_hdmi_stb_test_sel(data)

#define  HDMI_STB_clken_hdmi_stb_port3_mask                                  HDMI_STB_P0_clken_hdmi_stb_port3_mask
#define  HDMI_STB_clken_hdmi_stb_port2_mask                                  HDMI_STB_P0_clken_hdmi_stb_port2_mask
#define  HDMI_STB_clken_hdmi_stb_port1_mask                                  HDMI_STB_P0_clken_hdmi_stb_port1_mask
#define  HDMI_STB_clken_hdmi_stb_port0_mask                                  HDMI_STB_P0_clken_hdmi_stb_port0_mask
#define  HDMI_STB_clken_hdmi_stb_port3(data)                                 HDMI_STB_P0_clken_hdmi_stb_port3(data)
#define  HDMI_STB_clken_hdmi_stb_port2(data)                                 HDMI_STB_P0_clken_hdmi_stb_port2(data)
#define  HDMI_STB_clken_hdmi_stb_port1(data)                                 HDMI_STB_P0_clken_hdmi_stb_port1(data)
#define  HDMI_STB_clken_hdmi_stb_port0(data)                                 HDMI_STB_P0_clken_hdmi_stb_port0(data)
#define  HDMI_STB_clken_hdmi_stb_get_port3(data)                             HDMI_STB_P0_clken_hdmi_stb_get_port3(data)
#define  HDMI_STB_clken_hdmi_stb_get_port2(data)                             HDMI_STB_P0_clken_hdmi_stb_get_port2(data)
#define  HDMI_STB_clken_hdmi_stb_get_port1(data)                             HDMI_STB_P0_clken_hdmi_stb_get_port1(data)
#define  HDMI_STB_clken_hdmi_stb_get_port0(data)                             HDMI_STB_P0_clken_hdmi_stb_get_port0(data)

#define  HDMI_STB_rst_n_hdmi_stb_port3_mask                                  HDMI_STB_P0_rst_n_hdmi_stb_port3_mask
#define  HDMI_STB_rst_n_hdmi_stb_port2_mask                                  HDMI_STB_P0_rst_n_hdmi_stb_port2_mask
#define  HDMI_STB_rst_n_hdmi_stb_port1_mask                                  HDMI_STB_P0_rst_n_hdmi_stb_port1_mask
#define  HDMI_STB_rst_n_hdmi_stb_port0_mask                                  HDMI_STB_P0_rst_n_hdmi_stb_port0_mask
#define  HDMI_STB_rst_n_hdmi_stb_port3(data)                                 HDMI_STB_P0_rst_n_hdmi_stb_port3(data)
#define  HDMI_STB_rst_n_hdmi_stb_port2(data)                                 HDMI_STB_P0_rst_n_hdmi_stb_port2(data)
#define  HDMI_STB_rst_n_hdmi_stb_port1(data)                                 HDMI_STB_P0_rst_n_hdmi_stb_port1(data)
#define  HDMI_STB_rst_n_hdmi_stb_port0(data)                                 HDMI_STB_P0_rst_n_hdmi_stb_port0(data)
#define  HDMI_STB_rst_n_hdmi_stb_get_port3(data)                             HDMI_STB_P0_rst_n_hdmi_stb_get_port3(data)
#define  HDMI_STB_rst_n_hdmi_stb_get_port2(data)                             HDMI_STB_P0_rst_n_hdmi_stb_get_port2(data)
#define  HDMI_STB_rst_n_hdmi_stb_get_port1(data)                             HDMI_STB_P0_rst_n_hdmi_stb_get_port1(data)
#define  HDMI_STB_rst_n_hdmi_stb_get_port0(data)                             HDMI_STB_P0_rst_n_hdmi_stb_get_port0(data)

#define  HDMI_STB_hdmi_mp_mp_data_out_mask                                   HDMI_STB_P0_hdmi_mp_mp_data_out_mask
#define  HDMI_STB_hdmi_mp_mp_check_and_mask                                  HDMI_STB_P0_hdmi_mp_mp_check_and_mask
#define  HDMI_STB_hdmi_mp_mp_check_or_mask                                   HDMI_STB_P0_hdmi_mp_mp_check_or_mask
#define  HDMI_STB_hdmi_mp_sda_i_mask                                         HDMI_STB_P0_hdmi_mp_sda_i_mask
#define  HDMI_STB_hdmi_mp_scl_i_mask                                         HDMI_STB_P0_hdmi_mp_scl_i_mask
#define  HDMI_STB_hdmi_mp_sda_oe_mask                                        HDMI_STB_P0_hdmi_mp_sda_oe_mask
#define  HDMI_STB_hdmi_mp_ddc_port_sel_mask                                  HDMI_STB_P0_hdmi_mp_ddc_port_sel_mask
#define  HDMI_STB_hdmi_mp_mp_mode_mask                                       HDMI_STB_P0_hdmi_mp_mp_mode_mask
#define  HDMI_STB_hdmi_mp_mp_data_out(data)                                  HDMI_STB_P0_hdmi_mp_mp_data_out(data)
#define  HDMI_STB_hdmi_mp_mp_check_and(data)                                 HDMI_STB_P0_hdmi_mp_mp_check_and(data)
#define  HDMI_STB_hdmi_mp_mp_check_or(data)                                  HDMI_STB_P0_hdmi_mp_mp_check_or(data)
#define  HDMI_STB_hdmi_mp_sda_i(data)                                        HDMI_STB_P0_hdmi_mp_sda_i(data)
#define  HDMI_STB_hdmi_mp_scl_i(data)                                        HDMI_STB_P0_hdmi_mp_scl_i(data)
#define  HDMI_STB_hdmi_mp_sda_oe(data)                                       HDMI_STB_P0_hdmi_mp_sda_oe(data)
#define  HDMI_STB_hdmi_mp_ddc_port_sel(data)                                 HDMI_STB_P0_hdmi_mp_ddc_port_sel(data)
#define  HDMI_STB_hdmi_mp_mp_mode(data)                                      HDMI_STB_P0_hdmi_mp_mp_mode(data)
#define  HDMI_STB_hdmi_mp_get_mp_data_out(data)                              HDMI_STB_P0_hdmi_mp_get_mp_data_out(data)
#define  HDMI_STB_hdmi_mp_get_mp_check_and(data)                             HDMI_STB_P0_hdmi_mp_get_mp_check_and(data)
#define  HDMI_STB_hdmi_mp_get_mp_check_or(data)                              HDMI_STB_P0_hdmi_mp_get_mp_check_or(data)
#define  HDMI_STB_hdmi_mp_get_sda_i(data)                                    HDMI_STB_P0_hdmi_mp_get_sda_i(data)
#define  HDMI_STB_hdmi_mp_get_scl_i(data)                                    HDMI_STB_P0_hdmi_mp_get_scl_i(data)
#define  HDMI_STB_hdmi_mp_get_sda_oe(data)                                   HDMI_STB_P0_hdmi_mp_get_sda_oe(data)
#define  HDMI_STB_hdmi_mp_get_ddc_port_sel(data)                             HDMI_STB_P0_hdmi_mp_get_ddc_port_sel(data)
#define  HDMI_STB_hdmi_mp_get_mp_mode(data)                                  HDMI_STB_P0_hdmi_mp_get_mp_mode(data)


//---------------------------------------------------------------------------------
// HDMI2.1 MAC
//---------------------------------------------------------------------------------

#include <rbus/hdmi21_p0_reg.h>
#include <rbus/hdmi21_p1_reg.h>
#include <rbus/hdmi21_p2_reg.h>
#include <rbus/hdmi21_p3_reg.h>
#include <rbus/hdmi21_p0_meas_reg.h>
#include <rbus/hdmi21_p1_meas_reg.h>
#include <rbus/hdmi21_p2_meas_reg.h>
#include <rbus/hdmi21_p3_meas_reg.h>
#include <rbus/hdmi21_p0_fec_reg.h>
#include <rbus/hdmi21_p1_fec_reg.h>
#include <rbus/hdmi21_p2_fec_reg.h>
#include <rbus/hdmi21_p3_fec_reg.h>

#include <rbus/hdmi_ex_p0_reg.h>
#include <rbus/hdmi_ex_p1_reg.h>
#include <rbus/hdmi_ex_p2_reg.h>
#include <rbus/hdmi_ex_p3_reg.h>

#define HDMI21_MAC_NUM    4    // By IC

#define HDMI21_MAC0_BASE           (HDMI21_P0_HD21_CR_reg)
#define HDMI21_MAC1_BASE           (HDMI21_P0_HD21_CR_reg+0x2000)

struct hdmi21_mac_reg_st
{
    // HDMI_2p1 exist or not
    uint32_t  valid;

    // HDMI_2p1_control_register
    uint32_t  cr;

    // Video Part
    uint32_t  phy_fifo_cr0;
    uint32_t  phy_fifo_cr1;
    uint32_t  phy_fifo_sr0;
    uint32_t  phy_fifo_sr1;

    // Data Align
    uint32_t  channel_align_c;
    uint32_t  channel_align_r;
    uint32_t  channel_align_g;
    uint32_t  channel_align_b;

    // Channel Syn
    uint32_t  ch_sync_ctrl;
    uint32_t  ch_sync_status;
    uint32_t  ch_sync_r;
    uint32_t  ch_sync_g;
    uint32_t  ch_sync_b;
    uint32_t  ch_sync_c;

    // Error Detect Function
    uint32_t  errdet_r_ctrl;
    uint32_t  errdet_g_ctrl;
    uint32_t  errdet_b_ctrl;
    uint32_t  errdet_c_ctrl;    // 48
    uint32_t  errdet_r_disparity;    // 5C
    uint32_t  errdet_g_disparity;    // 60
    uint32_t  errdet_b_disparity;    // 64
    uint32_t  errdet_c_disparity ;   // 68
    uint32_t  errdet_r_char;    // 6C
    uint32_t  errdet_g_char;    // 70
    uint32_t  errdet_b_char;    // 74
    uint32_t  errdet_c_char;    // 78
    uint32_t  disp_ctrl;    // 7C

    // FRL Status
    uint32_t  mm_cr;    // 80
    uint32_t  mm_sr;    // 84

    // Extended Metadata Packet
    uint32_t  hdmi_hdrap;    // B8
    uint32_t  hdmi_hdrdp;    // BC
    uint32_t  fapa_ct2;    // C0
    uint32_t  hdr_em_st;    // C4
    uint32_t  hdr_em_ct;    // C8
    uint32_t  hdmi_emap;    // CC
    uint32_t  hdmi_emdp;    // D0
    uint32_t  hdmi_emap_pp;    // B0
    uint32_t  hdmi_emdp_pp;    // B4
    uint32_t  fapa_ct;    // D8
    uint32_t  hdr_em_ct2;    // DC

    // Error DET
    uint32_t  cercr;    // F0
    uint32_t  cersr0;    // F4
    uint32_t  cersr1;    // F8
    uint32_t  cersr2;    // FC

    uint32_t  rmzp;    // EC

    // Extended Metadata Packet 2
    uint32_t  vsem_st;    // 100
    uint32_t  vrrem_st;    // 104
    uint32_t  cvtem_st;    // 108
    uint32_t  hdmi_vsem_emc;    // 110
    uint32_t  hdmi_vrr_emc;    // 114
    uint32_t  hdmi_cvtem_emc;    // 118
    uint32_t  hdmi_vsem_emc2;    // 11C

    // CRC Control Register
    uint32_t crc_ctrl;          // 120
    uint32_t crc_result;        // 124
    uint32_t des_crc;           // 128
    uint32_t crc_err_cnt0;      // 12C

    // Video Timing Generator
    uint32_t  timing_gen_cr;    // 14C
    uint32_t  timing_gen_sr;    // 150

    // Channel Status
    uint32_t  ch_st;    // 1D0

    // PRBS
    uint32_t  prbs_r_ctrl;    // 1D4
    uint32_t  prbs_g_ctrl;    // 1D8
    uint32_t  prbs_b_ctrl;    // 1DC
    uint32_t  prbs_c_ctrl;    // 1E0
    uint32_t  prbs_r_status;    // 1E4
    uint32_t  prbs_g_status;    // 1E8
    uint32_t  prbs_b_status;    // 1EC
    uint32_t  prbs_c_status;    // 1F0

    // DSCD_DBG
    uint32_t  dscd_dbg;    // 1F4
    uint32_t  dscd_dbg_ap;    // 1F8
    uint32_t  dscd_dbg_dp;    // 1FC

    // LINK TRAINING
    uint32_t  lt_r;    // 200
    uint32_t  lt_g;    // 204
    uint32_t  lt_b;    // 208
    uint32_t  lt_c;    // 20C
    uint32_t  lt_all;    // 210

    // Debug Selection
    uint32_t  udc;    // 230

    //Deep Color
    uint32_t  tmds_dpc0;    // 234
    uint32_t  tmds_dpc1;    // 238
    uint32_t  tmds_dpc_set0;    // 23C

    // Packet Status
    uint32_t  hdmi_sr;    // 244
    uint32_t  hdmi_gpvs;    // 248
    uint32_t  hdmi_gpvs1;    // 24C
    uint32_t  hdmi_psap;    // 250
    uint32_t  hdmi_psdp;    // 254

    // Mode Control
    uint32_t  hdmi_scr;    // 258

    // BCH Control
    uint32_t  hdmi_bchcr;    // 25C

    // AV Mute Control
    uint32_t  hdmi_avmcr;    // 260
    uint32_t  hdmi_wdcr;

    // Packet Variation
    uint32_t  hdmi_pamicr;    // 264
    uint32_t  hdmi_ptrsv1;    // 268
    uint32_t  hdmi_ptrsv2;    // 26C
    uint32_t  hdmi_ptrsv3;    // 270
    uint32_t  hdmi_pvgcr0;    // 274
    uint32_t  hdmi_pvsr0;    // 278

    // Video Flow control
    uint32_t  hdmi_vcr;    // 27C

    // Audio Clock Regeneration Packet
    uint32_t  hdmi_acrcr;    // 280
    uint32_t  hdmi_acrsr0;    // 284
    uint32_t  hdmi_acrsr1;    // 288

    // IRQ
    uint32_t  hdmi_intcr;    // 28C

    //Packet/Guard Band Error
    uint32_t  hdmi_bcsr;    // 290

    // Packet Content
    uint32_t  hdmi_asr0;    // 294
    uint32_t  hdmi_asr1;    // 298
    uint32_t  hdmi_video_format;    // 29C
    uint32_t  hdmi_3d_format;    // 2A0
    uint32_t  hdmi_fvs;    // 2A4
    uint32_t  hdmi_drm;    // 2A8
    uint32_t  hdmi_pcmc;    // 2AC

    // Audio Control
    uint32_t  audio_sample_rate_change_irq;    // 2B0
    uint32_t  high_bit_rate_audio_packet;    // 2B4
    uint32_t  hdmi_audcr;    // 2B8
    uint32_t  hdmi_audsr;    // 2BC
    uint32_t  audio_cts_up_bound;    // 2C0
    uint32_t  audio_cts_low_bound;    // 2C4
    uint32_t  audio_n_up_bound;    // 2C8
    uint32_t  audio_n_low_bound;    // 2CC

    // CLKDET part
    uint32_t  clkdetcr;    // 2D0
    uint32_t  clkdetsr;    // 2D4
    uint32_t  clk_setting_00;    // 2D8
    uint32_t  clk_setting_01;    // 2DC
    uint32_t  clk_setting_02;    // 2E0
    uint32_t  clkdet2cr;    // 2E4
    uint32_t  clkdet2sr;    // 2E8
    uint32_t  clk2_setting_00;    // 2EC
    uint32_t  clk2_setting_01;    // 2F0
    uint32_t  clk2_setting_02;    // 2F4

    // CLK High Width Measure
    uint32_t  clk_hwm_setting_00;    // 2F8
    uint32_t  clk_hwm_setting_01;    // 2FC
    uint32_t  clk_hwm_setting_02;    // 300
    uint32_t  clk_hwn_one_st_00;    // 304
    uint32_t  clk_hwn_cont_st_01;    // 308

    // HDMI TMP part
    uint32_t  fapa_ct_tmp;    // 30C
    uint32_t  ps_ct;    // 310
    uint32_t  ps_st;    // 314
    uint32_t  irq_all_status;    // 318

    // XTAL CNT
    uint32_t  xtal_1s;    // 340
    uint32_t  xtal_280ms;    // 344
    uint32_t  xtal_10ms;    // 348

    // Power Saving Measure
    uint32_t  ps_measure_ctrl;    // 350
    uint32_t  ps_measure_xtal_clk0;    // 354
    uint32_t  ps_measure_xtal_clk1;    // 358
    uint32_t  ps_measure_xtal_clk2;    // 35C

    // Packet Absence
    uint32_t hdmi_pa_irq_en;    // 370
    uint32_t hdmi_pa_irq_sta;    // 374
    uint32_t hdmi_pa_frame_cnt;    // 378
};

extern const struct hdmi21_mac_reg_st hd21_mac[HDMI21_MAC_NUM];

//---------------------------
// Registe Wrapper
//---------------------------
#define  HD21_VALID_reg                                    (hd21_mac[nport].valid)
#define  HD21_CR_reg                                       (hd21_mac[nport].cr)

// PHY FIFO
#define  HD21_PHY_FIFO_CR0_reg                             (hd21_mac[nport].phy_fifo_cr0)
#define  HD21_PHY_FIFO_CR1_reg                             (hd21_mac[nport].phy_fifo_cr1)
#define  HD21_PHY_FIFO_SR0_reg                             (hd21_mac[nport].phy_fifo_sr0)
#define  HD21_PHY_FIFO_SR1_reg                             (hd21_mac[nport].phy_fifo_sr1)

// DATA ALIGN
#define  HD21_channel_align_c_reg                          (hd21_mac[nport].channel_align_c)
#define  HD21_channel_align_r_reg                          (hd21_mac[nport].channel_align_r)
#define  HD21_channel_align_g_reg                          (hd21_mac[nport].channel_align_g)
#define  HD21_channel_align_b_reg                          (hd21_mac[nport].channel_align_b)
#define  HD21_ch_sync_ctrl_reg                             (hd21_mac[nport].ch_sync_ctrl)
#define  HD21_ch_sync_status_reg                           (hd21_mac[nport].ch_sync_status)
#define  HD21_ch_sync_r_reg                                (hd21_mac[nport].ch_sync_r)
#define  HD21_ch_sync_g_reg                                (hd21_mac[nport].ch_sync_g)
#define  HD21_ch_sync_b_reg                                (hd21_mac[nport].ch_sync_b)
#define  HD21_ch_sync_c_reg                                (hd21_mac[nport].ch_sync_c)

// Error Detect
#define  HD21_errdet_r_ctrl_reg                            (hd21_mac[nport].errdet_r_ctrl)
#define  HD21_errdet_g_ctrl_reg                            (hd21_mac[nport].errdet_g_ctrl)
#define  HD21_errdet_b_ctrl_reg                            (hd21_mac[nport].errdet_b_ctrl)
#define  HD21_errdet_c_ctrl_reg                            (hd21_mac[nport].errdet_c_ctrl)
#define  HD21_errdet_r_disparity_reg                       (hd21_mac[nport].errdet_r_disparity)
#define  HD21_errdet_g_disparity_reg                       (hd21_mac[nport].errdet_g_disparity)
#define  HD21_errdet_b_disparity_reg                       (hd21_mac[nport].errdet_b_disparity)
#define  HD21_errdet_c_disparity_reg                       (hd21_mac[nport].errdet_c_disparity)
#define  HD21_errdet_r_char_reg                            (hd21_mac[nport].errdet_r_char)
#define  HD21_errdet_g_char_reg                            (hd21_mac[nport].errdet_g_char)
#define  HD21_errdet_b_char_reg                            (hd21_mac[nport].errdet_b_char)
#define  HD21_errdet_c_char_reg                            (hd21_mac[nport].errdet_c_char)
#define  HD21_disp_ctrl_reg                                (hd21_mac[nport].disp_ctrl)

// FRL
#define  HD21_MM_CR_reg                                    (hd21_mac[nport].mm_cr)
#define  HD21_MM_SR_reg                                    (hd21_mac[nport].mm_sr)

// EMP
#define  HD21_HDMI_HDRAP_reg                               (hd21_mac[nport].hdmi_hdrap)
#define  HD21_HDMI_HDRDP_reg                               (hd21_mac[nport].hdmi_hdrdp)
#define  HD21_HDR_EM_ST_reg                                (hd21_mac[nport].hdr_em_st)
#define  HD21_HDR_EM_CT_reg                                (hd21_mac[nport].hdr_em_ct)
#define  HD21_HDR_EM_CT2_reg                               (hd21_mac[nport].hdr_em_ct2)
#define  HD21_HDMI_EMAP_reg                                (hd21_mac[nport].hdmi_emap)
#define  HD21_HDMI_EMDP_reg                                (hd21_mac[nport].hdmi_emdp)
#define  HD21_HDMI_EMAP_PP_reg                             (hd21_mac[nport].hdmi_emap_pp)
#define  HD21_HDMI_EMDP_PP_reg                             (hd21_mac[nport].hdmi_emdp_pp)
#define  HD21_fapa_ct_reg                                  (hd21_mac[nport].fapa_ct)
#define  HD21_fapa_ct2_reg                                  (hd21_mac[0].fapa_ct2)
#define  HD21_VSEM_ST_reg                                  (hd21_mac[nport].vsem_st)
#define  HD21_VRREM_ST_reg                                 (hd21_mac[nport].vrrem_st)
#define  HD21_CVTEM_ST_reg                                 (hd21_mac[nport].cvtem_st)
#define  HD21_HDMI_VSEM_EMC_reg                            (hd21_mac[nport].hdmi_vsem_emc)
#define  HD21_HDMI_VRR_EMC_reg                             (hd21_mac[nport].hdmi_vrr_emc)
#define  HD21_HDMI_CVTEM_EMC_reg                           (hd21_mac[nport].hdmi_cvtem_emc)
#define  HD21_HDMI_VSEM_EMC2_reg                           (hd21_mac[nport].hdmi_vsem_emc2)

// RM Zero
#define  HD21_rmzp_reg                                     (hd21_mac[nport].rmzp)

// Error Detect
#define  HD21_cercr_reg                                    (hd21_mac[nport].cercr)
#define  HD21_CERSR0_reg                                   (hd21_mac[nport].cersr0)
#define  HD21_CERSR1_reg                                   (hd21_mac[nport].cersr1)
#define  HD21_CERSR2_reg                                   (hd21_mac[nport].cersr2)

// CRC Control Register
#define  HD21_CRC_Ctrl_reg                                 (hd21_mac[nport].crc_ctrl)
#define  HD21_CRC_Result_reg                               (hd21_mac[nport].crc_result)
#define  HD21_DES_CRC_reg                                  (hd21_mac[nport].des_crc)
#define  HD21_CRC_ERR_CNT0_reg                             (hd21_mac[nport].crc_err_cnt0)

// Video Timing Generator
#define  HD21_TIMING_GEN_CR_reg                            (hd21_mac[nport].timing_gen_cr)
#define  HD21_TIMING_GEN_SR_reg                            (hd21_mac[nport].timing_gen_sr)

// Channel Status
#define  HD21_CH_ST_reg                                    (hd21_mac[nport].ch_st)
#define  HD21_PRBS_R_CTRL_reg                              (hd21_mac[nport].prbs_r_ctrl)
#define  HD21_PRBS_G_CTRL_reg                              (hd21_mac[nport].prbs_g_ctrl)
#define  HD21_PRBS_B_CTRL_reg                              (hd21_mac[nport].prbs_b_ctrl)
#define  HD21_PRBS_C_CTRL_reg                              (hd21_mac[nport].prbs_c_ctrl)
#define  HD21_PRBS_R_STATUS_reg                            (hd21_mac[nport].prbs_r_status)
#define  HD21_PRBS_G_STATUS_reg                            (hd21_mac[nport].prbs_g_status)
#define  HD21_PRBS_B_STATUS_reg                            (hd21_mac[nport].prbs_b_status)
#define  HD21_PRBS_C_STATUS_reg                            (hd21_mac[nport].prbs_c_status)

// DCSD
#define  HD21_DSCD_DBG_reg                                 (hd21_mac[nport].dscd_dbg)
#define  HD21_DSCD_DBG_AP_reg                              (hd21_mac[nport].dscd_dbg_ap)
#define  HD21_DSCD_DBG_DP_reg                              (hd21_mac[nport].dscd_dbg_dp)

// Link Training
#define  HD21_LT_R_reg                                      (hd21_mac[nport].lt_r)
#define  HD21_LT_G_reg                                     (hd21_mac[nport].lt_g)
#define  HD21_LT_B_reg                                     (hd21_mac[nport].lt_b)
#define  HD21_LT_C_reg                                     (hd21_mac[nport].lt_c)
#define  HD21_LT_ALL_reg                                   (hd21_mac[nport].lt_all)
#define  HD21_UDC_reg                                      (hd21_mac[nport].udc)
#define  HD21_TMDS_DPC0_reg                                (hd21_mac[nport].tmds_dpc0)
#define  HD21_TMDS_DPC1_reg                                (hd21_mac[nport].tmds_dpc1)
#define  HD21_TMDS_DPC_SET0_reg                            (hd21_mac[nport].tmds_dpc_set0)

// Packet Status
#define  HD21_HDMI_SR_reg                                  (hd21_mac[nport].hdmi_sr)
#define  HD21_HDMI_GPVS_reg                                (hd21_mac[nport].hdmi_gpvs)
#define  HD21_HDMI_GPVS1_reg                               (hd21_mac[nport].hdmi_gpvs1)
#define  HD21_HDMI_PSAP_reg                                (hd21_mac[nport].hdmi_psap)
#define  HD21_HDMI_PSDP_reg                                (hd21_mac[nport].hdmi_psdp)
#define  HD21_HDMI_SCR_reg                                 (hd21_mac[nport].hdmi_scr)
#define  HD21_HDMI_BCHCR_reg                               (hd21_mac[nport].hdmi_bchcr)
#define  HD21_HDMI_AVMCR_reg                               (hd21_mac[nport].hdmi_avmcr)
#define  HD21_HDMI_WDCR_reg                                (hd21_mac[nport].hdmi_wdcr)
#define  HD21_HDMI_PAMICR_reg                              (hd21_mac[nport].hdmi_pamicr)
#define  HD21_HDMI_PTRSV1_reg                              (hd21_mac[nport].hdmi_ptrsv1)
#define  HD21_HDMI_PTRSV2_reg                              (hd21_mac[nport].hdmi_ptrsv2)
#define  HD21_HDMI_PTRSV3_reg                              (hd21_mac[nport].hdmi_ptrsv3)
#define  HD21_HDMI_PVGCR0_reg                              (hd21_mac[nport].hdmi_pvgcr0)
#define  HD21_HDMI_PVSR0_reg                               (hd21_mac[nport].hdmi_pvsr0)

// Video Flow
#define  HD21_HDMI_VCR_reg                                 (hd21_mac[nport].hdmi_vcr)
#define  HD21_HDMI_ACRCR_reg                               (hd21_mac[nport].hdmi_acrcr)
#define  HD21_HDMI_ACRSR0_reg                              (hd21_mac[nport].hdmi_acrsr0)
#define  HD21_HDMI_ACRSR1_reg                              (hd21_mac[nport].hdmi_acrsr1)
#define  HD21_HDMI_INTCR_reg                               (hd21_mac[nport].hdmi_intcr)
#define  HD21_HDMI_BCSR_reg                                (hd21_mac[nport].hdmi_bcsr)
#define  HD21_HDMI_ASR0_reg                                (hd21_mac[nport].hdmi_asr0)
#define  HD21_HDMI_ASR1_reg                                (hd21_mac[nport].hdmi_asr1)
#define  HD21_HDMI_VIDEO_FORMAT_reg                        (hd21_mac[nport].hdmi_video_format)
#define  HD21_HDMI_3D_FORMAT_reg                            (hd21_mac[nport].hdmi_3d_format)
#define  HD21_HDMI_FVS_reg                                 (hd21_mac[nport].hdmi_fvs)
#define  HD21_HDMI_DRM_reg                                 (hd21_mac[nport].hdmi_drm)
#define  HD21_HDMI_PCMC_reg                                (hd21_mac[nport].hdmi_pcmc)
#define  HD21_Audio_Sample_Rate_Change_IRQ_reg             (hd21_mac[nport].audio_sample_rate_change_irq)
#define  HD21_High_Bit_Rate_Audio_Packet_reg               (hd21_mac[nport].high_bit_rate_audio_packet)
#define  HD21_HDMI_AUDCR_reg                               (hd21_mac[nport].hdmi_audcr)
#define  HD21_HDMI_AUDSR_reg                               (hd21_mac[nport].hdmi_audsr)
#define  HD21_AUDIO_CTS_UP_BOUND_reg                       (hd21_mac[nport].audio_cts_up_bound)
#define  HD21_AUDIO_CTS_LOW_BOUND_reg                      (hd21_mac[nport].audio_cts_low_bound)
#define  HD21_AUDIO_N_UP_BOUND_reg                         (hd21_mac[nport].audio_n_up_bound)
#define  HD21_AUDIO_N_LOW_BOUND_reg                        (hd21_mac[nport].audio_n_low_bound)
#define  HD21_CLKDETCR_reg                                 (hd21_mac[nport].clkdetcr)
#define  HD21_CLKDETSR_reg                                 (hd21_mac[nport].clkdetsr)
#define  HD21_CLK_SETTING_00_reg                           (hd21_mac[nport].clk_setting_00)
#define  HD21_CLK_SETTING_01_reg                           (hd21_mac[nport].clk_setting_01)
#define  HD21_CLK_SETTING_02_reg                           (hd21_mac[nport].clk_setting_02)
#define  HD21_CLKDET2CR_reg                                (hd21_mac[nport].clkdet2cr)
#define  HD21_CLKDET2SR_reg                                (hd21_mac[nport].clkdet2sr)
#define  HD21_CLK2_SETTING_00_reg                          (hd21_mac[nport].clk2_setting_00)
#define  HD21_CLK2_SETTING_01_reg                          (hd21_mac[nport].clk2_setting_01)
#define  HD21_CLK2_SETTING_02_reg                          (hd21_mac[nport].clk2_setting_02)
#define  HD21_CLK_HWM_SETTING_00_reg                       (hd21_mac[nport].clk_hwm_setting_00)
#define  HD21_CLK_HWM_SETTING_01_reg                       (hd21_mac[nport].clk_hwm_setting_01)
#define  HD21_CLK_HWM_SETTING_02_reg                       (hd21_mac[nport].clk_hwm_setting_02)
#define  HD21_CLK_HWN_ONE_ST_00_reg                        (hd21_mac[nport].clk_hwn_one_st_00)
#define  HD21_CLK_HWN_CONT_ST_01_reg                       (hd21_mac[nport].clk_hwn_cont_st_01)
#define  HD21_fapa_ct_tmp_reg                              (hd21_mac[nport].fapa_ct_tmp)
#define  HD21_ps_ct_reg                                    (hd21_mac[nport].ps_ct)
#define  HD21_ps_st_reg                                    (hd21_mac[nport].ps_st)
#define  HD21_irq_all_status_reg                           (hd21_mac[nport].irq_all_status)
#define  HD21_xtal_1s_reg                                  (hd21_mac[nport].xtal_1s)
#define  HD21_xtal_280ms_reg                               (hd21_mac[nport].xtal_280ms)
#define  HD21_xtal_10ms_reg                                (hd21_mac[nport].xtal_10ms)
#define  HD21_ps_measure_ctrl_reg                          (hd21_mac[nport].ps_measure_ctrl)
#define  HD21_ps_measure_xtal_clk0_reg                     (hd21_mac[nport].ps_measure_xtal_clk0)
#define  HD21_ps_measure_xtal_clk1_reg                     (hd21_mac[nport].ps_measure_xtal_clk1)
#define  HD21_ps_measure_xtal_clk2_reg                     (hd21_mac[nport].ps_measure_xtal_clk2)         

// packet absence
#define  HD21_HDMI_PA_IRQ_EN_reg               (hd21_mac[nport].hdmi_pa_irq_en)
#define  HD21_HDMI_PA_IRQ_STA_reg              (hd21_mac[nport].hdmi_pa_irq_sta)
#define  HD21_HDMI_PA_FRAME_CNT_reg            (hd21_mac[nport].hdmi_pa_frame_cnt)

// reg warpper
#define  HD21_CR_clk_div_sel_mask                                       HDMI21_P0_HD21_CR_clk_div_sel_mask
#define  HD21_CR_clk_div_en_mask                                       HDMI21_P0_HD21_CR_clk_div_en_mask
#define  HD21_CR_iclk_sel_mask                                          HDMI21_P0_HD21_CR_iclk_sel_mask
#define  HD21_CR_dsc_mode_fw_mask                                       HDMI21_P0_HD21_CR_dsc_mode_fw_mask
#define  HD21_CR_lane_mode_fw_mask                                      HDMI21_P0_HD21_CR_lane_mode_fw_mask
#define  HD21_CR_hd21_en_mask                                           HDMI21_P0_HD21_CR_hd21_en_mask
#define  HD21_CR_clk_div_sel(data)                                      HDMI21_P0_HD21_CR_clk_div_sel(data)
#define  HD21_CR_clk_div_en(data)                                      HDMI21_P0_HD21_CR_clk_div_en(data)
#define  HD21_CR_iclk_sel(data)                                         HDMI21_P0_HD21_CR_iclk_sel(data)
#define  HD21_CR_dsc_mode_fw(data)                                      HDMI21_P0_HD21_CR_dsc_mode_fw(data)
#define  HD21_CR_lane_mode_fw(data)                                     HDMI21_P0_HD21_CR_lane_mode_fw(data)
#define  HD21_CR_hd21_en(data)                                          HDMI21_P0_HD21_CR_hd21_en(data)
#define  HD21_CR_get_clk_div_sel(data)                                      HDMI21_P0_HD21_CR_get_clk_div_sel(data)
#define  HD21_CR_get_clk_div_en(data)                                  HDMI21_P0_HD21_CR_get_clk_div_en(data)
#define  HD21_CR_get_iclk_sel(data)                                     HDMI21_P0_HD21_CR_get_iclk_sel(data)
#define  HD21_CR_get_dsc_mode_fw(data)                                  HDMI21_P0_HD21_CR_get_dsc_mode_fw(data)
#define  HD21_CR_get_lane_mode_fw(data)                                 HDMI21_P0_HD21_CR_get_lane_mode_fw(data)
#define  HD21_CR_get_hd21_en(data)                                      HDMI21_P0_HD21_CR_get_hd21_en(data)


#define  HD21_PHY_FIFO_CR0_port0_c_flush_mask                           HDMI21_P0_HD21_PHY_FIFO_CR0_port0_c_flush_mask
#define  HD21_PHY_FIFO_CR0_port0_c_afifo_en_mask                        HDMI21_P0_HD21_PHY_FIFO_CR0_port0_c_afifo_en_mask
#define  HD21_PHY_FIFO_CR0_port2_c_flush_mask                           HDMI21_P0_HD21_PHY_FIFO_CR0_port2_c_flush_mask
#define  HD21_PHY_FIFO_CR0_port2_c_afifo_en_mask                        HDMI21_P0_HD21_PHY_FIFO_CR0_port2_c_afifo_en_mask
#define  HD21_PHY_FIFO_CR0_port1_c_flush_mask                           HDMI21_P0_HD21_PHY_FIFO_CR0_port1_c_flush_mask
#define  HD21_PHY_FIFO_CR0_port1_c_afifo_en_mask                        HDMI21_P0_HD21_PHY_FIFO_CR0_port1_c_afifo_en_mask
#define  HD21_PHY_FIFO_CR0_port3_c_flush_mask                           HDMI21_P0_HD21_PHY_FIFO_CR0_port3_c_flush_mask
#define  HD21_PHY_FIFO_CR0_port3_c_afifo_en_mask                        HDMI21_P0_HD21_PHY_FIFO_CR0_port3_c_afifo_en_mask
#define  HD21_PHY_FIFO_CR0_port0_c_flush(data)                          HDMI21_P0_HD21_PHY_FIFO_CR0_port0_c_flush(data)
#define  HD21_PHY_FIFO_CR0_port0_c_afifo_en(data)                       HDMI21_P0_HD21_PHY_FIFO_CR0_port0_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_port2_c_flush(data)                          HDMI21_P0_HD21_PHY_FIFO_CR0_port2_c_flush(data)
#define  HD21_PHY_FIFO_CR0_port2_c_afifo_en(data)                       HDMI21_P0_HD21_PHY_FIFO_CR0_port2_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_port1_c_flush(data)                          HDMI21_P0_HD21_PHY_FIFO_CR0_port1_c_flush(data)
#define  HD21_PHY_FIFO_CR0_port1_c_afifo_en(data)                       HDMI21_P0_HD21_PHY_FIFO_CR0_port1_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_port3_c_flush(data)                          HDMI21_P0_HD21_PHY_FIFO_CR0_port3_c_flush(data)
#define  HD21_PHY_FIFO_CR0_port3_c_afifo_en(data)                       HDMI21_P0_HD21_PHY_FIFO_CR0_port3_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_get_port0_c_flush(data)                      HDMI21_P0_HD21_PHY_FIFO_CR0_get_port0_c_flush(data)
#define  HD21_PHY_FIFO_CR0_get_port0_c_afifo_en(data)                   HDMI21_P0_HD21_PHY_FIFO_CR0_get_port0_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_get_port2_c_flush(data)                      HDMI21_P0_HD21_PHY_FIFO_CR0_get_port2_c_flush(data)
#define  HD21_PHY_FIFO_CR0_get_port2_c_afifo_en(data)                   HDMI21_P0_HD21_PHY_FIFO_CR0_get_port2_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_get_port1_c_flush(data)                      HDMI21_P0_HD21_PHY_FIFO_CR0_get_port1_c_flush(data)
#define  HD21_PHY_FIFO_CR0_get_port1_c_afifo_en(data)                   HDMI21_P0_HD21_PHY_FIFO_CR0_get_port1_c_afifo_en(data)
#define  HD21_PHY_FIFO_CR0_get_port3_c_flush(data)                      HDMI21_P0_HD21_PHY_FIFO_CR0_get_port3_c_flush(data)
#define  HD21_PHY_FIFO_CR0_get_port3_c_afifo_en(data)                   HDMI21_P0_HD21_PHY_FIFO_CR0_get_port3_c_afifo_en(data)


#define  HD21_PHY_FIFO_CR1_port0_ckclk_inv_mask                         HDMI21_P0_HD21_PHY_FIFO_CR1_port0_ckclk_inv_mask
#define  HD21_PHY_FIFO_CR1_port1_ckclk_inv_mask                         HDMI21_P0_HD21_PHY_FIFO_CR1_port1_ckclk_inv_mask
#define  HD21_PHY_FIFO_CR1_port2_ckclk_inv_mask                         HDMI21_P0_HD21_PHY_FIFO_CR1_port2_ckclk_inv_mask
#define  HD21_PHY_FIFO_CR1_port3_ckclk_inv_mask                         HDMI21_P0_HD21_PHY_FIFO_CR1_port3_ckclk_inv_mask
#define  HD21_PHY_FIFO_CR1_port0_ckclk_inv(data)                        HDMI21_P0_HD21_PHY_FIFO_CR1_port0_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_port1_ckclk_inv(data)                        HDMI21_P0_HD21_PHY_FIFO_CR1_port1_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_port2_ckclk_inv(data)                        HDMI21_P0_HD21_PHY_FIFO_CR1_port2_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_port3_ckclk_inv(data)                        HDMI21_P0_HD21_PHY_FIFO_CR1_port3_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_get_port0_ckclk_inv(data)                    HDMI21_P0_HD21_PHY_FIFO_CR1_get_port0_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_get_port1_ckclk_inv(data)                    HDMI21_P0_HD21_PHY_FIFO_CR1_get_port1_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_get_port2_ckclk_inv(data)                    HDMI21_P0_HD21_PHY_FIFO_CR1_get_port2_ckclk_inv(data)
#define  HD21_PHY_FIFO_CR1_get_port3_ckclk_inv(data)                    HDMI21_P0_HD21_PHY_FIFO_CR1_get_port3_ckclk_inv(data)


#define  HD21_PHY_FIFO_SR0_port0_c_wrclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_wrclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR0_port0_c_rwclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rwclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR0_port0_c_wovflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_wovflow_flag_mask
#define  HD21_PHY_FIFO_SR0_port0_c_rudflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rudflow_flag_mask
#define  HD21_PHY_FIFO_SR0_port0_c_rflush_flag_mask                     HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rflush_flag_mask
#define  HD21_PHY_FIFO_SR0_port1_c_wrclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_wrclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR0_port1_c_rwclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rwclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR0_port1_c_wovflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_wovflow_flag_mask
#define  HD21_PHY_FIFO_SR0_port1_c_rudflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rudflow_flag_mask
#define  HD21_PHY_FIFO_SR0_port1_c_rflush_flag_mask                     HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rflush_flag_mask
#define  HD21_PHY_FIFO_SR0_port0_c_wrclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_port0_c_rwclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_port0_c_wovflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR0_port0_c_rudflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR0_port0_c_rflush_flag(data)                    HDMI21_P0_HD21_PHY_FIFO_SR0_port0_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR0_port1_c_wrclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_port1_c_rwclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_port1_c_wovflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR0_port1_c_rudflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR0_port1_c_rflush_flag(data)                    HDMI21_P0_HD21_PHY_FIFO_SR0_port1_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port0_c_wrclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR0_get_port0_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port0_c_rwclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR0_get_port0_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port0_c_wovflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR0_get_port0_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port0_c_rudflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR0_get_port0_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port0_c_rflush_flag(data)                HDMI21_P0_HD21_PHY_FIFO_SR0_get_port0_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port1_c_wrclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR0_get_port1_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port1_c_rwclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR0_get_port1_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port1_c_wovflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR0_get_port1_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port1_c_rudflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR0_get_port1_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR0_get_port1_c_rflush_flag(data)                HDMI21_P0_HD21_PHY_FIFO_SR0_get_port1_c_rflush_flag(data)


#define  HD21_PHY_FIFO_SR1_port2_c_wrclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_wrclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR1_port2_c_rwclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rwclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR1_port2_c_wovflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_wovflow_flag_mask
#define  HD21_PHY_FIFO_SR1_port2_c_rudflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rudflow_flag_mask
#define  HD21_PHY_FIFO_SR1_port2_c_rflush_flag_mask                     HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rflush_flag_mask
#define  HD21_PHY_FIFO_SR1_port3_c_wrclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_wrclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR1_port3_c_rwclk_det_timeout_flag_mask          HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rwclk_det_timeout_flag_mask
#define  HD21_PHY_FIFO_SR1_port3_c_wovflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_wovflow_flag_mask
#define  HD21_PHY_FIFO_SR1_port3_c_rudflow_flag_mask                    HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rudflow_flag_mask
#define  HD21_PHY_FIFO_SR1_port3_c_rflush_flag_mask                     HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rflush_flag_mask
#define  HD21_PHY_FIFO_SR1_port2_c_wrclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_port2_c_rwclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_port2_c_wovflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR1_port2_c_rudflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR1_port2_c_rflush_flag(data)                    HDMI21_P0_HD21_PHY_FIFO_SR1_port2_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR1_port3_c_wrclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_port3_c_rwclk_det_timeout_flag(data)         HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_port3_c_wovflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR1_port3_c_rudflow_flag(data)                   HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR1_port3_c_rflush_flag(data)                    HDMI21_P0_HD21_PHY_FIFO_SR1_port3_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port2_c_wrclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR1_get_port2_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port2_c_rwclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR1_get_port2_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port2_c_wovflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR1_get_port2_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port2_c_rudflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR1_get_port2_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port2_c_rflush_flag(data)                HDMI21_P0_HD21_PHY_FIFO_SR1_get_port2_c_rflush_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port3_c_wrclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR1_get_port3_c_wrclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port3_c_rwclk_det_timeout_flag(data)     HDMI21_P0_HD21_PHY_FIFO_SR1_get_port3_c_rwclk_det_timeout_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port3_c_wovflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR1_get_port3_c_wovflow_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port3_c_rudflow_flag(data)               HDMI21_P0_HD21_PHY_FIFO_SR1_get_port3_c_rudflow_flag(data)
#define  HD21_PHY_FIFO_SR1_get_port3_c_rflush_flag(data)                HDMI21_P0_HD21_PHY_FIFO_SR1_get_port3_c_rflush_flag(data)


#define  HD21_channel_align_c_scdc_force_realign_mask                   HDMI21_P0_hd21_channel_align_c_scdc_force_realign_mask
#define  HD21_channel_align_c_scdc_lock_sel_mask                        HDMI21_P0_hd21_channel_align_c_scdc_lock_sel_mask
#define  HD21_channel_align_c_scdc_unlock_num_mask                      HDMI21_P0_hd21_channel_align_c_scdc_unlock_num_mask
#define  HD21_channel_align_c_scdc_lock_bound_mask                      HDMI21_P0_hd21_channel_align_c_scdc_lock_bound_mask
#define  HD21_channel_align_c_scdc_align_mode_mask                      HDMI21_P0_hd21_channel_align_c_scdc_align_mode_mask
#define  HD21_channel_align_c_align_mode_mask                           HDMI21_P0_hd21_channel_align_c_align_mode_mask
#define  HD21_channel_align_c_force_realign_mask                        HDMI21_P0_hd21_channel_align_c_force_realign_mask
#define  HD21_channel_align_c_lock_sel_mask                             HDMI21_P0_hd21_channel_align_c_lock_sel_mask
#define  HD21_channel_align_c_unlock_num_mask                           HDMI21_P0_hd21_channel_align_c_unlock_num_mask
#define  HD21_channel_align_c_unex_comma_mask                           HDMI21_P0_hd21_channel_align_c_unex_comma_mask
#define  HD21_channel_align_c_position_error_flag_mask                  HDMI21_P0_hd21_channel_align_c_position_error_flag_mask
#define  HD21_channel_align_c_com_seq_lock_mask                         HDMI21_P0_hd21_channel_align_c_com_seq_lock_mask
#define  HD21_channel_align_c_sr_detect_mask                            HDMI21_P0_hd21_channel_align_c_sr_detect_mask
#define  HD21_channel_align_c_position_error_cnt_mask                   HDMI21_P0_hd21_channel_align_c_position_error_cnt_mask
#define  HD21_channel_align_c_lock_bound_mask                           HDMI21_P0_hd21_channel_align_c_lock_bound_mask
#define  HD21_channel_align_c_scdc_force_realign(data)                  HDMI21_P0_hd21_channel_align_c_scdc_force_realign(data)
#define  HD21_channel_align_c_scdc_lock_sel(data)                       HDMI21_P0_hd21_channel_align_c_scdc_lock_sel(data)
#define  HD21_channel_align_c_scdc_unlock_num(data)                     HDMI21_P0_hd21_channel_align_c_scdc_unlock_num(data)
#define  HD21_channel_align_c_scdc_lock_bound(data)                     HDMI21_P0_hd21_channel_align_c_scdc_lock_bound(data)
#define  HD21_channel_align_c_scdc_align_mode(data)                     HDMI21_P0_hd21_channel_align_c_scdc_align_mode(data)
#define  HD21_channel_align_c_align_mode(data)                          HDMI21_P0_hd21_channel_align_c_align_mode(data)
#define  HD21_channel_align_c_force_realign(data)                       HDMI21_P0_hd21_channel_align_c_force_realign(data)
#define  HD21_channel_align_c_lock_sel(data)                            HDMI21_P0_hd21_channel_align_c_lock_sel(data)
#define  HD21_channel_align_c_unlock_num(data)                          HDMI21_P0_hd21_channel_align_c_unlock_num(data)
#define  HD21_channel_align_c_unex_comma(data)                          HDMI21_P0_hd21_channel_align_c_unex_comma(data)
#define  HD21_channel_align_c_position_error_flag(data)                 HDMI21_P0_hd21_channel_align_c_position_error_flag(data)
#define  HD21_channel_align_c_com_seq_lock(data)                        HDMI21_P0_hd21_channel_align_c_com_seq_lock(data)
#define  HD21_channel_align_c_sr_detect(data)                           HDMI21_P0_hd21_channel_align_c_sr_detect(data)
#define  HD21_channel_align_c_position_error_cnt(data)                  HDMI21_P0_hd21_channel_align_c_position_error_cnt(data)
#define  HD21_channel_align_c_lock_bound(data)                          HDMI21_P0_hd21_channel_align_c_lock_bound(data)
#define  HD21_channel_align_c_get_scdc_force_realign(data)              HDMI21_P0_hd21_channel_align_c_get_scdc_force_realign(data)
#define  HD21_channel_align_c_get_scdc_lock_sel(data)                   HDMI21_P0_hd21_channel_align_c_get_scdc_lock_sel(data)
#define  HD21_channel_align_c_get_scdc_unlock_num(data)                 HDMI21_P0_hd21_channel_align_c_get_scdc_unlock_num(data)
#define  HD21_channel_align_c_get_scdc_lock_bound(data)                 HDMI21_P0_hd21_channel_align_c_get_scdc_lock_bound(data)
#define  HD21_channel_align_c_get_scdc_align_mode(data)                 HDMI21_P0_hd21_channel_align_c_get_scdc_align_mode(data)
#define  HD21_channel_align_c_get_align_mode(data)                      HDMI21_P0_hd21_channel_align_c_get_align_mode(data)
#define  HD21_channel_align_c_get_force_realign(data)                   HDMI21_P0_hd21_channel_align_c_get_force_realign(data)
#define  HD21_channel_align_c_get_lock_sel(data)                        HDMI21_P0_hd21_channel_align_c_get_lock_sel(data)
#define  HD21_channel_align_c_get_unlock_num(data)                      HDMI21_P0_hd21_channel_align_c_get_unlock_num(data)
#define  HD21_channel_align_c_get_unex_comma(data)                      HDMI21_P0_hd21_channel_align_c_get_unex_comma(data)
#define  HD21_channel_align_c_get_position_error_flag(data)             HDMI21_P0_hd21_channel_align_c_get_position_error_flag(data)
#define  HD21_channel_align_c_get_com_seq_lock(data)                    HDMI21_P0_hd21_channel_align_c_get_com_seq_lock(data)
#define  HD21_channel_align_c_get_sr_detect(data)                       HDMI21_P0_hd21_channel_align_c_get_sr_detect(data)
#define  HD21_channel_align_c_get_position_error_cnt(data)              HDMI21_P0_hd21_channel_align_c_get_position_error_cnt(data)
#define  HD21_channel_align_c_get_lock_bound(data)                      HDMI21_P0_hd21_channel_align_c_get_lock_bound(data)


#define  HD21_channel_align_r_scdc_force_realign_mask                   HDMI21_P0_hd21_channel_align_r_scdc_force_realign_mask
#define  HD21_channel_align_r_scdc_lock_sel_mask                        HDMI21_P0_hd21_channel_align_r_scdc_lock_sel_mask
#define  HD21_channel_align_r_scdc_unlock_num_mask                      HDMI21_P0_hd21_channel_align_r_scdc_unlock_num_mask
#define  HD21_channel_align_r_scdc_lock_bound_mask                      HDMI21_P0_hd21_channel_align_r_scdc_lock_bound_mask
#define  HD21_channel_align_r_scdc_align_mode_mask                      HDMI21_P0_hd21_channel_align_r_scdc_align_mode_mask
#define  HD21_channel_align_r_align_mode_mask                           HDMI21_P0_hd21_channel_align_r_align_mode_mask
#define  HD21_channel_align_r_force_realign_mask                        HDMI21_P0_hd21_channel_align_r_force_realign_mask
#define  HD21_channel_align_r_lock_sel_mask                             HDMI21_P0_hd21_channel_align_r_lock_sel_mask
#define  HD21_channel_align_r_unlock_num_mask                           HDMI21_P0_hd21_channel_align_r_unlock_num_mask
#define  HD21_channel_align_r_unex_comma_mask                           HDMI21_P0_hd21_channel_align_r_unex_comma_mask
#define  HD21_channel_align_r_position_error_flag_mask                  HDMI21_P0_hd21_channel_align_r_position_error_flag_mask
#define  HD21_channel_align_r_com_seq_lock_mask                         HDMI21_P0_hd21_channel_align_r_com_seq_lock_mask
#define  HD21_channel_align_r_sr_detect_mask                            HDMI21_P0_hd21_channel_align_r_sr_detect_mask
#define  HD21_channel_align_r_position_error_cnt_mask                   HDMI21_P0_hd21_channel_align_r_position_error_cnt_mask
#define  HD21_channel_align_r_lock_bound_mask                           HDMI21_P0_hd21_channel_align_r_lock_bound_mask
#define  HD21_channel_align_r_scdc_force_realign(data)                  HDMI21_P0_hd21_channel_align_r_scdc_force_realign(data)
#define  HD21_channel_align_r_scdc_lock_sel(data)                       HDMI21_P0_hd21_channel_align_r_scdc_lock_sel(data)
#define  HD21_channel_align_r_scdc_unlock_num(data)                     HDMI21_P0_hd21_channel_align_r_scdc_unlock_num(data)
#define  HD21_channel_align_r_scdc_lock_bound(data)                     HDMI21_P0_hd21_channel_align_r_scdc_lock_bound(data)
#define  HD21_channel_align_r_scdc_align_mode(data)                     HDMI21_P0_hd21_channel_align_r_scdc_align_mode(data)
#define  HD21_channel_align_r_align_mode(data)                          HDMI21_P0_hd21_channel_align_r_align_mode(data)
#define  HD21_channel_align_r_force_realign(data)                       HDMI21_P0_hd21_channel_align_r_force_realign(data)
#define  HD21_channel_align_r_lock_sel(data)                            HDMI21_P0_hd21_channel_align_r_lock_sel(data)
#define  HD21_channel_align_r_unlock_num(data)                          HDMI21_P0_hd21_channel_align_r_unlock_num(data)
#define  HD21_channel_align_r_unex_comma(data)                          HDMI21_P0_hd21_channel_align_r_unex_comma(data)
#define  HD21_channel_align_r_position_error_flag(data)                 HDMI21_P0_hd21_channel_align_r_position_error_flag(data)
#define  HD21_channel_align_r_com_seq_lock(data)                        HDMI21_P0_hd21_channel_align_r_com_seq_lock(data)
#define  HD21_channel_align_r_sr_detect(data)                           HDMI21_P0_hd21_channel_align_r_sr_detect(data)
#define  HD21_channel_align_r_position_error_cnt(data)                  HDMI21_P0_hd21_channel_align_r_position_error_cnt(data)
#define  HD21_channel_align_r_lock_bound(data)                          HDMI21_P0_hd21_channel_align_r_lock_bound(data)
#define  HD21_channel_align_r_get_scdc_force_realign(data)              HDMI21_P0_hd21_channel_align_r_get_scdc_force_realign(data)
#define  HD21_channel_align_r_get_scdc_lock_sel(data)                   HDMI21_P0_hd21_channel_align_r_get_scdc_lock_sel(data)
#define  HD21_channel_align_r_get_scdc_unlock_num(data)                 HDMI21_P0_hd21_channel_align_r_get_scdc_unlock_num(data)
#define  HD21_channel_align_r_get_scdc_lock_bound(data)                 HDMI21_P0_hd21_channel_align_r_get_scdc_lock_bound(data)
#define  HD21_channel_align_r_get_scdc_align_mode(data)                 HDMI21_P0_hd21_channel_align_r_get_scdc_align_mode(data)
#define  HD21_channel_align_r_get_align_mode(data)                      HDMI21_P0_hd21_channel_align_r_get_align_mode(data)
#define  HD21_channel_align_r_get_force_realign(data)                   HDMI21_P0_hd21_channel_align_r_get_force_realign(data)
#define  HD21_channel_align_r_get_lock_sel(data)                        HDMI21_P0_hd21_channel_align_r_get_lock_sel(data)
#define  HD21_channel_align_r_get_unlock_num(data)                      HDMI21_P0_hd21_channel_align_r_get_unlock_num(data)
#define  HD21_channel_align_r_get_unex_comma(data)                      HDMI21_P0_hd21_channel_align_r_get_unex_comma(data)
#define  HD21_channel_align_r_get_position_error_flag(data)             HDMI21_P0_hd21_channel_align_r_get_position_error_flag(data)
#define  HD21_channel_align_r_get_com_seq_lock(data)                    HDMI21_P0_hd21_channel_align_r_get_com_seq_lock(data)
#define  HD21_channel_align_r_get_sr_detect(data)                       HDMI21_P0_hd21_channel_align_r_get_sr_detect(data)
#define  HD21_channel_align_r_get_position_error_cnt(data)              HDMI21_P0_hd21_channel_align_r_get_position_error_cnt(data)
#define  HD21_channel_align_r_get_lock_bound(data)                      HDMI21_P0_hd21_channel_align_r_get_lock_bound(data)


#define  HD21_channel_align_g_scdc_force_realign_mask                   HDMI21_P0_hd21_channel_align_g_scdc_force_realign_mask
#define  HD21_channel_align_g_scdc_lock_sel_mask                        HDMI21_P0_hd21_channel_align_g_scdc_lock_sel_mask
#define  HD21_channel_align_g_scdc_unlock_num_mask                      HDMI21_P0_hd21_channel_align_g_scdc_unlock_num_mask
#define  HD21_channel_align_g_scdc_lock_bound_mask                      HDMI21_P0_hd21_channel_align_g_scdc_lock_bound_mask
#define  HD21_channel_align_g_scdc_align_mode_mask                      HDMI21_P0_hd21_channel_align_g_scdc_align_mode_mask
#define  HD21_channel_align_g_align_mode_mask                           HDMI21_P0_hd21_channel_align_g_align_mode_mask
#define  HD21_channel_align_g_force_realign_mask                        HDMI21_P0_hd21_channel_align_g_force_realign_mask
#define  HD21_channel_align_g_lock_sel_mask                             HDMI21_P0_hd21_channel_align_g_lock_sel_mask
#define  HD21_channel_align_g_unlock_num_mask                           HDMI21_P0_hd21_channel_align_g_unlock_num_mask
#define  HD21_channel_align_g_unex_comma_mask                           HDMI21_P0_hd21_channel_align_g_unex_comma_mask
#define  HD21_channel_align_g_position_error_flag_mask                  HDMI21_P0_hd21_channel_align_g_position_error_flag_mask
#define  HD21_channel_align_g_com_seq_lock_mask                         HDMI21_P0_hd21_channel_align_g_com_seq_lock_mask
#define  HD21_channel_align_g_sr_detect_mask                            HDMI21_P0_hd21_channel_align_g_sr_detect_mask
#define  HD21_channel_align_g_position_error_cnt_mask                   HDMI21_P0_hd21_channel_align_g_position_error_cnt_mask
#define  HD21_channel_align_g_lock_bound_mask                           HDMI21_P0_hd21_channel_align_g_lock_bound_mask
#define  HD21_channel_align_g_scdc_force_realign(data)                  HDMI21_P0_hd21_channel_align_g_scdc_force_realign(data)
#define  HD21_channel_align_g_scdc_lock_sel(data)                       HDMI21_P0_hd21_channel_align_g_scdc_lock_sel(data)
#define  HD21_channel_align_g_scdc_unlock_num(data)                     HDMI21_P0_hd21_channel_align_g_scdc_unlock_num(data)
#define  HD21_channel_align_g_scdc_lock_bound(data)                     HDMI21_P0_hd21_channel_align_g_scdc_lock_bound(data)
#define  HD21_channel_align_g_scdc_align_mode(data)                     HDMI21_P0_hd21_channel_align_g_scdc_align_mode(data)
#define  HD21_channel_align_g_align_mode(data)                          HDMI21_P0_hd21_channel_align_g_align_mode(data)
#define  HD21_channel_align_g_force_realign(data)                       HDMI21_P0_hd21_channel_align_g_force_realign(data)
#define  HD21_channel_align_g_lock_sel(data)                            HDMI21_P0_hd21_channel_align_g_lock_sel(data)
#define  HD21_channel_align_g_unlock_num(data)                          HDMI21_P0_hd21_channel_align_g_unlock_num(data)
#define  HD21_channel_align_g_unex_comma(data)                          HDMI21_P0_hd21_channel_align_g_unex_comma(data)
#define  HD21_channel_align_g_position_error_flag(data)                 HDMI21_P0_hd21_channel_align_g_position_error_flag(data)
#define  HD21_channel_align_g_com_seq_lock(data)                        HDMI21_P0_hd21_channel_align_g_com_seq_lock(data)
#define  HD21_channel_align_g_sr_detect(data)                           HDMI21_P0_hd21_channel_align_g_sr_detect(data)
#define  HD21_channel_align_g_position_error_cnt(data)                  HDMI21_P0_hd21_channel_align_g_position_error_cnt(data)
#define  HD21_channel_align_g_lock_bound(data)                          HDMI21_P0_hd21_channel_align_g_lock_bound(data)
#define  HD21_channel_align_g_get_scdc_force_realign(data)              HDMI21_P0_hd21_channel_align_g_get_scdc_force_realign(data)
#define  HD21_channel_align_g_get_scdc_lock_sel(data)                   HDMI21_P0_hd21_channel_align_g_get_scdc_lock_sel(data)
#define  HD21_channel_align_g_get_scdc_unlock_num(data)                 HDMI21_P0_hd21_channel_align_g_get_scdc_unlock_num(data)
#define  HD21_channel_align_g_get_scdc_lock_bound(data)                 HDMI21_P0_hd21_channel_align_g_get_scdc_lock_bound(data)
#define  HD21_channel_align_g_get_scdc_align_mode(data)                 HDMI21_P0_hd21_channel_align_g_get_scdc_align_mode(data)
#define  HD21_channel_align_g_get_align_mode(data)                      HDMI21_P0_hd21_channel_align_g_get_align_mode(data)
#define  HD21_channel_align_g_get_force_realign(data)                   HDMI21_P0_hd21_channel_align_g_get_force_realign(data)
#define  HD21_channel_align_g_get_lock_sel(data)                        HDMI21_P0_hd21_channel_align_g_get_lock_sel(data)
#define  HD21_channel_align_g_get_unlock_num(data)                      HDMI21_P0_hd21_channel_align_g_get_unlock_num(data)
#define  HD21_channel_align_g_get_unex_comma(data)                      HDMI21_P0_hd21_channel_align_g_get_unex_comma(data)
#define  HD21_channel_align_g_get_position_error_flag(data)             HDMI21_P0_hd21_channel_align_g_get_position_error_flag(data)
#define  HD21_channel_align_g_get_com_seq_lock(data)                    HDMI21_P0_hd21_channel_align_g_get_com_seq_lock(data)
#define  HD21_channel_align_g_get_sr_detect(data)                       HDMI21_P0_hd21_channel_align_g_get_sr_detect(data)
#define  HD21_channel_align_g_get_position_error_cnt(data)              HDMI21_P0_hd21_channel_align_g_get_position_error_cnt(data)
#define  HD21_channel_align_g_get_lock_bound(data)                      HDMI21_P0_hd21_channel_align_g_get_lock_bound(data)


#define  HD21_channel_align_b_scdc_force_realign_mask                   HDMI21_P0_hd21_channel_align_b_scdc_force_realign_mask
#define  HD21_channel_align_b_scdc_lock_sel_mask                        HDMI21_P0_hd21_channel_align_b_scdc_lock_sel_mask
#define  HD21_channel_align_b_scdc_unlock_num_mask                      HDMI21_P0_hd21_channel_align_b_scdc_unlock_num_mask
#define  HD21_channel_align_b_scdc_lock_bound_mask                      HDMI21_P0_hd21_channel_align_b_scdc_lock_bound_mask
#define  HD21_channel_align_b_scdc_align_mode_mask                      HDMI21_P0_hd21_channel_align_b_scdc_align_mode_mask
#define  HD21_channel_align_b_align_mode_mask                           HDMI21_P0_hd21_channel_align_b_align_mode_mask
#define  HD21_channel_align_b_force_realign_mask                        HDMI21_P0_hd21_channel_align_b_force_realign_mask
#define  HD21_channel_align_b_lock_sel_mask                             HDMI21_P0_hd21_channel_align_b_lock_sel_mask
#define  HD21_channel_align_b_unlock_num_mask                           HDMI21_P0_hd21_channel_align_b_unlock_num_mask
#define  HD21_channel_align_b_unex_comma_mask                           HDMI21_P0_hd21_channel_align_b_unex_comma_mask
#define  HD21_channel_align_b_position_error_flag_mask                  HDMI21_P0_hd21_channel_align_b_position_error_flag_mask
#define  HD21_channel_align_b_com_seq_lock_mask                         HDMI21_P0_hd21_channel_align_b_com_seq_lock_mask
#define  HD21_channel_align_b_sr_detect_mask                            HDMI21_P0_hd21_channel_align_b_sr_detect_mask
#define  HD21_channel_align_b_position_error_cnt_mask                   HDMI21_P0_hd21_channel_align_b_position_error_cnt_mask
#define  HD21_channel_align_b_lock_bound_mask                           HDMI21_P0_hd21_channel_align_b_lock_bound_mask
#define  HD21_channel_align_b_scdc_force_realign(data)                  HDMI21_P0_hd21_channel_align_b_scdc_force_realign(data)
#define  HD21_channel_align_b_scdc_lock_sel(data)                       HDMI21_P0_hd21_channel_align_b_scdc_lock_sel(data)
#define  HD21_channel_align_b_scdc_unlock_num(data)                     HDMI21_P0_hd21_channel_align_b_scdc_unlock_num(data)
#define  HD21_channel_align_b_scdc_lock_bound(data)                     HDMI21_P0_hd21_channel_align_b_scdc_lock_bound(data)
#define  HD21_channel_align_b_scdc_align_mode(data)                     HDMI21_P0_hd21_channel_align_b_scdc_align_mode(data)
#define  HD21_channel_align_b_align_mode(data)                          HDMI21_P0_hd21_channel_align_b_align_mode(data)
#define  HD21_channel_align_b_force_realign(data)                       HDMI21_P0_hd21_channel_align_b_force_realign(data)
#define  HD21_channel_align_b_lock_sel(data)                            HDMI21_P0_hd21_channel_align_b_lock_sel(data)
#define  HD21_channel_align_b_unlock_num(data)                          HDMI21_P0_hd21_channel_align_b_unlock_num(data)
#define  HD21_channel_align_b_unex_comma(data)                          HDMI21_P0_hd21_channel_align_b_unex_comma(data)
#define  HD21_channel_align_b_position_error_flag(data)                 HDMI21_P0_hd21_channel_align_b_position_error_flag(data)
#define  HD21_channel_align_b_com_seq_lock(data)                        HDMI21_P0_hd21_channel_align_b_com_seq_lock(data)
#define  HD21_channel_align_b_sr_detect(data)                           HDMI21_P0_hd21_channel_align_b_sr_detect(data)
#define  HD21_channel_align_b_position_error_cnt(data)                  HDMI21_P0_hd21_channel_align_b_position_error_cnt(data)
#define  HD21_channel_align_b_lock_bound(data)                          HDMI21_P0_hd21_channel_align_b_lock_bound(data)
#define  HD21_channel_align_b_get_scdc_force_realign(data)              HDMI21_P0_hd21_channel_align_b_get_scdc_force_realign(data)
#define  HD21_channel_align_b_get_scdc_lock_sel(data)                   HDMI21_P0_hd21_channel_align_b_get_scdc_lock_sel(data)
#define  HD21_channel_align_b_get_scdc_unlock_num(data)                 HDMI21_P0_hd21_channel_align_b_get_scdc_unlock_num(data)
#define  HD21_channel_align_b_get_scdc_lock_bound(data)                 HDMI21_P0_hd21_channel_align_b_get_scdc_lock_bound(data)
#define  HD21_channel_align_b_get_scdc_align_mode(data)                 HDMI21_P0_hd21_channel_align_b_get_scdc_align_mode(data)
#define  HD21_channel_align_b_get_align_mode(data)                      HDMI21_P0_hd21_channel_align_b_get_align_mode(data)
#define  HD21_channel_align_b_get_force_realign(data)                   HDMI21_P0_hd21_channel_align_b_get_force_realign(data)
#define  HD21_channel_align_b_get_lock_sel(data)                        HDMI21_P0_hd21_channel_align_b_get_lock_sel(data)
#define  HD21_channel_align_b_get_unlock_num(data)                      HDMI21_P0_hd21_channel_align_b_get_unlock_num(data)
#define  HD21_channel_align_b_get_unex_comma(data)                      HDMI21_P0_hd21_channel_align_b_get_unex_comma(data)
#define  HD21_channel_align_b_get_position_error_flag(data)             HDMI21_P0_hd21_channel_align_b_get_position_error_flag(data)
#define  HD21_channel_align_b_get_com_seq_lock(data)                    HDMI21_P0_hd21_channel_align_b_get_com_seq_lock(data)
#define  HD21_channel_align_b_get_sr_detect(data)                       HDMI21_P0_hd21_channel_align_b_get_sr_detect(data)
#define  HD21_channel_align_b_get_position_error_cnt(data)              HDMI21_P0_hd21_channel_align_b_get_position_error_cnt(data)
#define  HD21_channel_align_b_get_lock_bound(data)                      HDMI21_P0_hd21_channel_align_b_get_lock_bound(data)


#define  HD21_ch_sync_ctrl_ch_sync_sel_mask                             HDMI21_P0_hd21_ch_sync_ctrl_ch_sync_sel_mask
#define  HD21_ch_sync_ctrl_dummy_19_17_mask                             HDMI21_P0_hd21_ch_sync_ctrl_dummy_19_17_mask
#define  HD21_ch_sync_ctrl_rden_thr_mask                                HDMI21_P0_hd21_ch_sync_ctrl_rden_thr_mask
#define  HD21_ch_sync_ctrl_dummy_11_9_mask                              HDMI21_P0_hd21_ch_sync_ctrl_dummy_11_9_mask
#define  HD21_ch_sync_ctrl_udwater_thr_mask                             HDMI21_P0_hd21_ch_sync_ctrl_udwater_thr_mask
#define  HD21_ch_sync_ctrl_dummy_3_mask                                 HDMI21_P0_hd21_ch_sync_ctrl_dummy_3_mask
#define  HD21_ch_sync_ctrl_flush_mask                                   HDMI21_P0_hd21_ch_sync_ctrl_flush_mask
#define  HD21_ch_sync_ctrl_ch_afifo_airq_en_mask                        HDMI21_P0_hd21_ch_sync_ctrl_ch_afifo_airq_en_mask
#define  HD21_ch_sync_ctrl_ch_afifo_irq_en_mask                         HDMI21_P0_hd21_ch_sync_ctrl_ch_afifo_irq_en_mask
#define  HD21_ch_sync_ctrl_ch_sync_sel(data)                            HDMI21_P0_hd21_ch_sync_ctrl_ch_sync_sel(data)
#define  HD21_ch_sync_ctrl_dummy_19_17(data)                            HDMI21_P0_hd21_ch_sync_ctrl_dummy_19_17(data)
#define  HD21_ch_sync_ctrl_rden_thr(data)                               HDMI21_P0_hd21_ch_sync_ctrl_rden_thr(data)
#define  HD21_ch_sync_ctrl_dummy_11_9(data)                             HDMI21_P0_hd21_ch_sync_ctrl_dummy_11_9(data)
#define  HD21_ch_sync_ctrl_udwater_thr(data)                            HDMI21_P0_hd21_ch_sync_ctrl_udwater_thr(data)
#define  HD21_ch_sync_ctrl_dummy_3(data)                                HDMI21_P0_hd21_ch_sync_ctrl_dummy_3(data)
#define  HD21_ch_sync_ctrl_flush(data)                                  HDMI21_P0_hd21_ch_sync_ctrl_flush(data)
#define  HD21_ch_sync_ctrl_ch_afifo_airq_en(data)                       HDMI21_P0_hd21_ch_sync_ctrl_ch_afifo_airq_en(data)
#define  HD21_ch_sync_ctrl_ch_afifo_irq_en(data)                        HDMI21_P0_hd21_ch_sync_ctrl_ch_afifo_irq_en(data)
#define  HD21_ch_sync_ctrl_get_ch_sync_sel(data)                        HDMI21_P0_hd21_ch_sync_ctrl_get_ch_sync_sel(data)
#define  HD21_ch_sync_ctrl_get_dummy_19_17(data)                        HDMI21_P0_hd21_ch_sync_ctrl_get_dummy_19_17(data)
#define  HD21_ch_sync_ctrl_get_rden_thr(data)                           HDMI21_P0_hd21_ch_sync_ctrl_get_rden_thr(data)
#define  HD21_ch_sync_ctrl_get_dummy_11_9(data)                         HDMI21_P0_hd21_ch_sync_ctrl_get_dummy_11_9(data)
#define  HD21_ch_sync_ctrl_get_udwater_thr(data)                        HDMI21_P0_hd21_ch_sync_ctrl_get_udwater_thr(data)
#define  HD21_ch_sync_ctrl_get_dummy_3(data)                            HDMI21_P0_hd21_ch_sync_ctrl_get_dummy_3(data)
#define  HD21_ch_sync_ctrl_get_flush(data)                              HDMI21_P0_hd21_ch_sync_ctrl_get_flush(data)
#define  HD21_ch_sync_ctrl_get_ch_afifo_airq_en(data)                   HDMI21_P0_hd21_ch_sync_ctrl_get_ch_afifo_airq_en(data)
#define  HD21_ch_sync_ctrl_get_ch_afifo_irq_en(data)                    HDMI21_P0_hd21_ch_sync_ctrl_get_ch_afifo_irq_en(data)


#define  HD21_ch_sync_status_rgbc_de_align_flag_mask                    HDMI21_P0_hd21_ch_sync_status_rgbc_de_align_flag_mask
#define  HD21_ch_sync_status_rgbc_de_align_flag(data)                   HDMI21_P0_hd21_ch_sync_status_rgbc_de_align_flag(data)
#define  HD21_ch_sync_status_get_rgbc_de_align_flag(data)               HDMI21_P0_hd21_ch_sync_status_get_rgbc_de_align_flag(data)


#define  HD21_ch_sync_r_rw_water_lv_mask                                HDMI21_P0_hd21_ch_sync_r_rw_water_lv_mask
#define  HD21_ch_sync_r_rflush_flag_mask                                HDMI21_P0_hd21_ch_sync_r_rflush_flag_mask
#define  HD21_ch_sync_r_rwclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_r_rwclk_det_timeout_flag_mask
#define  HD21_ch_sync_r_wrclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_r_wrclk_det_timeout_flag_mask
#define  HD21_ch_sync_r_rudflow_flag_mask                               HDMI21_P0_hd21_ch_sync_r_rudflow_flag_mask
#define  HD21_ch_sync_r_wovflow_flag_mask                               HDMI21_P0_hd21_ch_sync_r_wovflow_flag_mask
#define  HD21_ch_sync_r_ch_afifo_en_mask                                HDMI21_P0_hd21_ch_sync_r_ch_afifo_en_mask
#define  HD21_ch_sync_r_rw_water_lv(data)                               HDMI21_P0_hd21_ch_sync_r_rw_water_lv(data)
#define  HD21_ch_sync_r_rflush_flag(data)                               HDMI21_P0_hd21_ch_sync_r_rflush_flag(data)
#define  HD21_ch_sync_r_rwclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_r_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_r_wrclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_r_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_r_rudflow_flag(data)                              HDMI21_P0_hd21_ch_sync_r_rudflow_flag(data)
#define  HD21_ch_sync_r_wovflow_flag(data)                              HDMI21_P0_hd21_ch_sync_r_wovflow_flag(data)
#define  HD21_ch_sync_r_ch_afifo_en(data)                               HDMI21_P0_hd21_ch_sync_r_ch_afifo_en(data)
#define  HD21_ch_sync_r_get_rw_water_lv(data)                           HDMI21_P0_hd21_ch_sync_r_get_rw_water_lv(data)
#define  HD21_ch_sync_r_get_rflush_flag(data)                           HDMI21_P0_hd21_ch_sync_r_get_rflush_flag(data)
#define  HD21_ch_sync_r_get_rwclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_r_get_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_r_get_wrclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_r_get_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_r_get_rudflow_flag(data)                          HDMI21_P0_hd21_ch_sync_r_get_rudflow_flag(data)
#define  HD21_ch_sync_r_get_wovflow_flag(data)                          HDMI21_P0_hd21_ch_sync_r_get_wovflow_flag(data)
#define  HD21_ch_sync_r_get_ch_afifo_en(data)                           HDMI21_P0_hd21_ch_sync_r_get_ch_afifo_en(data)


#define  HD21_ch_sync_g_rw_water_lv_mask                                HDMI21_P0_hd21_ch_sync_g_rw_water_lv_mask
#define  HD21_ch_sync_g_rflush_flag_mask                                HDMI21_P0_hd21_ch_sync_g_rflush_flag_mask
#define  HD21_ch_sync_g_rwclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_g_rwclk_det_timeout_flag_mask
#define  HD21_ch_sync_g_wrclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_g_wrclk_det_timeout_flag_mask
#define  HD21_ch_sync_g_rudflow_flag_mask                               HDMI21_P0_hd21_ch_sync_g_rudflow_flag_mask
#define  HD21_ch_sync_g_wovflow_flag_mask                               HDMI21_P0_hd21_ch_sync_g_wovflow_flag_mask
#define  HD21_ch_sync_g_ch_afifo_en_mask                                HDMI21_P0_hd21_ch_sync_g_ch_afifo_en_mask
#define  HD21_ch_sync_g_rw_water_lv(data)                               HDMI21_P0_hd21_ch_sync_g_rw_water_lv(data)
#define  HD21_ch_sync_g_rflush_flag(data)                               HDMI21_P0_hd21_ch_sync_g_rflush_flag(data)
#define  HD21_ch_sync_g_rwclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_g_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_g_wrclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_g_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_g_rudflow_flag(data)                              HDMI21_P0_hd21_ch_sync_g_rudflow_flag(data)
#define  HD21_ch_sync_g_wovflow_flag(data)                              HDMI21_P0_hd21_ch_sync_g_wovflow_flag(data)
#define  HD21_ch_sync_g_ch_afifo_en(data)                               HDMI21_P0_hd21_ch_sync_g_ch_afifo_en(data)
#define  HD21_ch_sync_g_get_rw_water_lv(data)                           HDMI21_P0_hd21_ch_sync_g_get_rw_water_lv(data)
#define  HD21_ch_sync_g_get_rflush_flag(data)                           HDMI21_P0_hd21_ch_sync_g_get_rflush_flag(data)
#define  HD21_ch_sync_g_get_rwclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_g_get_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_g_get_wrclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_g_get_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_g_get_rudflow_flag(data)                          HDMI21_P0_hd21_ch_sync_g_get_rudflow_flag(data)
#define  HD21_ch_sync_g_get_wovflow_flag(data)                          HDMI21_P0_hd21_ch_sync_g_get_wovflow_flag(data)
#define  HD21_ch_sync_g_get_ch_afifo_en(data)                           HDMI21_P0_hd21_ch_sync_g_get_ch_afifo_en(data)


#define  HD21_ch_sync_b_rw_water_lv_mask                                HDMI21_P0_hd21_ch_sync_b_rw_water_lv_mask
#define  HD21_ch_sync_b_rflush_flag_mask                                HDMI21_P0_hd21_ch_sync_b_rflush_flag_mask
#define  HD21_ch_sync_b_rwclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_b_rwclk_det_timeout_flag_mask
#define  HD21_ch_sync_b_wrclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_b_wrclk_det_timeout_flag_mask
#define  HD21_ch_sync_b_rudflow_flag_mask                               HDMI21_P0_hd21_ch_sync_b_rudflow_flag_mask
#define  HD21_ch_sync_b_wovflow_flag_mask                               HDMI21_P0_hd21_ch_sync_b_wovflow_flag_mask
#define  HD21_ch_sync_b_ch_afifo_en_mask                                HDMI21_P0_hd21_ch_sync_b_ch_afifo_en_mask
#define  HD21_ch_sync_b_rw_water_lv(data)                               HDMI21_P0_hd21_ch_sync_b_rw_water_lv(data)
#define  HD21_ch_sync_b_rflush_flag(data)                               HDMI21_P0_hd21_ch_sync_b_rflush_flag(data)
#define  HD21_ch_sync_b_rwclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_b_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_b_wrclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_b_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_b_rudflow_flag(data)                              HDMI21_P0_hd21_ch_sync_b_rudflow_flag(data)
#define  HD21_ch_sync_b_wovflow_flag(data)                              HDMI21_P0_hd21_ch_sync_b_wovflow_flag(data)
#define  HD21_ch_sync_b_ch_afifo_en(data)                               HDMI21_P0_hd21_ch_sync_b_ch_afifo_en(data)
#define  HD21_ch_sync_b_get_rw_water_lv(data)                           HDMI21_P0_hd21_ch_sync_b_get_rw_water_lv(data)
#define  HD21_ch_sync_b_get_rflush_flag(data)                           HDMI21_P0_hd21_ch_sync_b_get_rflush_flag(data)
#define  HD21_ch_sync_b_get_rwclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_b_get_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_b_get_wrclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_b_get_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_b_get_rudflow_flag(data)                          HDMI21_P0_hd21_ch_sync_b_get_rudflow_flag(data)
#define  HD21_ch_sync_b_get_wovflow_flag(data)                          HDMI21_P0_hd21_ch_sync_b_get_wovflow_flag(data)
#define  HD21_ch_sync_b_get_ch_afifo_en(data)                           HDMI21_P0_hd21_ch_sync_b_get_ch_afifo_en(data)


#define  HD21_ch_sync_c_rw_water_lv_mask                                HDMI21_P0_hd21_ch_sync_c_rw_water_lv_mask
#define  HD21_ch_sync_c_rflush_flag_mask                                HDMI21_P0_hd21_ch_sync_c_rflush_flag_mask
#define  HD21_ch_sync_c_rwclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_c_rwclk_det_timeout_flag_mask
#define  HD21_ch_sync_c_wrclk_det_timeout_flag_mask                     HDMI21_P0_hd21_ch_sync_c_wrclk_det_timeout_flag_mask
#define  HD21_ch_sync_c_rudflow_flag_mask                               HDMI21_P0_hd21_ch_sync_c_rudflow_flag_mask
#define  HD21_ch_sync_c_wovflow_flag_mask                               HDMI21_P0_hd21_ch_sync_c_wovflow_flag_mask
#define  HD21_ch_sync_c_ch_afifo_en_mask                                HDMI21_P0_hd21_ch_sync_c_ch_afifo_en_mask
#define  HD21_ch_sync_c_rw_water_lv(data)                               HDMI21_P0_hd21_ch_sync_c_rw_water_lv(data)
#define  HD21_ch_sync_c_rflush_flag(data)                               HDMI21_P0_hd21_ch_sync_c_rflush_flag(data)
#define  HD21_ch_sync_c_rwclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_c_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_c_wrclk_det_timeout_flag(data)                    HDMI21_P0_hd21_ch_sync_c_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_c_rudflow_flag(data)                              HDMI21_P0_hd21_ch_sync_c_rudflow_flag(data)
#define  HD21_ch_sync_c_wovflow_flag(data)                              HDMI21_P0_hd21_ch_sync_c_wovflow_flag(data)
#define  HD21_ch_sync_c_ch_afifo_en(data)                               HDMI21_P0_hd21_ch_sync_c_ch_afifo_en(data)
#define  HD21_ch_sync_c_get_rw_water_lv(data)                           HDMI21_P0_hd21_ch_sync_c_get_rw_water_lv(data)
#define  HD21_ch_sync_c_get_rflush_flag(data)                           HDMI21_P0_hd21_ch_sync_c_get_rflush_flag(data)
#define  HD21_ch_sync_c_get_rwclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_c_get_rwclk_det_timeout_flag(data)
#define  HD21_ch_sync_c_get_wrclk_det_timeout_flag(data)                HDMI21_P0_hd21_ch_sync_c_get_wrclk_det_timeout_flag(data)
#define  HD21_ch_sync_c_get_rudflow_flag(data)                          HDMI21_P0_hd21_ch_sync_c_get_rudflow_flag(data)
#define  HD21_ch_sync_c_get_wovflow_flag(data)                          HDMI21_P0_hd21_ch_sync_c_get_wovflow_flag(data)
#define  HD21_ch_sync_c_get_ch_afifo_en(data)                           HDMI21_P0_hd21_ch_sync_c_get_ch_afifo_en(data)


#define  HD21_errdet_r_ctrl_char_en_mask                                HDMI21_P0_hd21_errdet_r_ctrl_char_en_mask
#define  HD21_errdet_r_ctrl_disp_en_mask                                HDMI21_P0_hd21_errdet_r_ctrl_disp_en_mask
#define  HD21_errdet_r_ctrl_disp_thr_mask                               HDMI21_P0_hd21_errdet_r_ctrl_disp_thr_mask
#define  HD21_errdet_r_ctrl_char_thr_mask                               HDMI21_P0_hd21_errdet_r_ctrl_char_thr_mask
#define  HD21_errdet_r_ctrl_char_en(data)                               HDMI21_P0_hd21_errdet_r_ctrl_char_en(data)
#define  HD21_errdet_r_ctrl_disp_en(data)                               HDMI21_P0_hd21_errdet_r_ctrl_disp_en(data)
#define  HD21_errdet_r_ctrl_disp_thr(data)                              HDMI21_P0_hd21_errdet_r_ctrl_disp_thr(data)
#define  HD21_errdet_r_ctrl_char_thr(data)                              HDMI21_P0_hd21_errdet_r_ctrl_char_thr(data)
#define  HD21_errdet_r_ctrl_get_char_en(data)                           HDMI21_P0_hd21_errdet_r_ctrl_get_char_en(data)
#define  HD21_errdet_r_ctrl_get_disp_en(data)                           HDMI21_P0_hd21_errdet_r_ctrl_get_disp_en(data)
#define  HD21_errdet_r_ctrl_get_disp_thr(data)                          HDMI21_P0_hd21_errdet_r_ctrl_get_disp_thr(data)
#define  HD21_errdet_r_ctrl_get_char_thr(data)                          HDMI21_P0_hd21_errdet_r_ctrl_get_char_thr(data)


#define  HD21_errdet_g_ctrl_char_en_mask                                HDMI21_P0_hd21_errdet_g_ctrl_char_en_mask
#define  HD21_errdet_g_ctrl_disp_en_mask                                HDMI21_P0_hd21_errdet_g_ctrl_disp_en_mask
#define  HD21_errdet_g_ctrl_disp_thr_mask                               HDMI21_P0_hd21_errdet_g_ctrl_disp_thr_mask
#define  HD21_errdet_g_ctrl_char_thr_mask                               HDMI21_P0_hd21_errdet_g_ctrl_char_thr_mask
#define  HD21_errdet_g_ctrl_char_en(data)                               HDMI21_P0_hd21_errdet_g_ctrl_char_en(data)
#define  HD21_errdet_g_ctrl_disp_en(data)                               HDMI21_P0_hd21_errdet_g_ctrl_disp_en(data)
#define  HD21_errdet_g_ctrl_disp_thr(data)                              HDMI21_P0_hd21_errdet_g_ctrl_disp_thr(data)
#define  HD21_errdet_g_ctrl_char_thr(data)                              HDMI21_P0_hd21_errdet_g_ctrl_char_thr(data)
#define  HD21_errdet_g_ctrl_get_char_en(data)                           HDMI21_P0_hd21_errdet_g_ctrl_get_char_en(data)
#define  HD21_errdet_g_ctrl_get_disp_en(data)                           HDMI21_P0_hd21_errdet_g_ctrl_get_disp_en(data)
#define  HD21_errdet_g_ctrl_get_disp_thr(data)                          HDMI21_P0_hd21_errdet_g_ctrl_get_disp_thr(data)
#define  HD21_errdet_g_ctrl_get_char_thr(data)                          HDMI21_P0_hd21_errdet_g_ctrl_get_char_thr(data)


#define  HD21_errdet_b_ctrl_char_en_mask                                HDMI21_P0_hd21_errdet_b_ctrl_char_en_mask
#define  HD21_errdet_b_ctrl_disp_en_mask                                HDMI21_P0_hd21_errdet_b_ctrl_disp_en_mask
#define  HD21_errdet_b_ctrl_disp_thr_mask                               HDMI21_P0_hd21_errdet_b_ctrl_disp_thr_mask
#define  HD21_errdet_b_ctrl_char_thr_mask                               HDMI21_P0_hd21_errdet_b_ctrl_char_thr_mask
#define  HD21_errdet_b_ctrl_char_en(data)                               HDMI21_P0_hd21_errdet_b_ctrl_char_en(data)
#define  HD21_errdet_b_ctrl_disp_en(data)                               HDMI21_P0_hd21_errdet_b_ctrl_disp_en(data)
#define  HD21_errdet_b_ctrl_disp_thr(data)                              HDMI21_P0_hd21_errdet_b_ctrl_disp_thr(data)
#define  HD21_errdet_b_ctrl_char_thr(data)                              HDMI21_P0_hd21_errdet_b_ctrl_char_thr(data)
#define  HD21_errdet_b_ctrl_get_char_en(data)                           HDMI21_P0_hd21_errdet_b_ctrl_get_char_en(data)
#define  HD21_errdet_b_ctrl_get_disp_en(data)                           HDMI21_P0_hd21_errdet_b_ctrl_get_disp_en(data)
#define  HD21_errdet_b_ctrl_get_disp_thr(data)                          HDMI21_P0_hd21_errdet_b_ctrl_get_disp_thr(data)
#define  HD21_errdet_b_ctrl_get_char_thr(data)                          HDMI21_P0_hd21_errdet_b_ctrl_get_char_thr(data)


#define  HD21_errdet_c_ctrl_char_en_mask                                HDMI21_P0_hd21_errdet_c_ctrl_char_en_mask
#define  HD21_errdet_c_ctrl_disp_en_mask                                HDMI21_P0_hd21_errdet_c_ctrl_disp_en_mask
#define  HD21_errdet_c_ctrl_disp_thr_mask                               HDMI21_P0_hd21_errdet_c_ctrl_disp_thr_mask
#define  HD21_errdet_c_ctrl_char_thr_mask                               HDMI21_P0_hd21_errdet_c_ctrl_char_thr_mask
#define  HD21_errdet_c_ctrl_char_en(data)                               HDMI21_P0_hd21_errdet_c_ctrl_char_en(data)
#define  HD21_errdet_c_ctrl_disp_en(data)                               HDMI21_P0_hd21_errdet_c_ctrl_disp_en(data)
#define  HD21_errdet_c_ctrl_disp_thr(data)                              HDMI21_P0_hd21_errdet_c_ctrl_disp_thr(data)
#define  HD21_errdet_c_ctrl_char_thr(data)                              HDMI21_P0_hd21_errdet_c_ctrl_char_thr(data)
#define  HD21_errdet_c_ctrl_get_char_en(data)                           HDMI21_P0_hd21_errdet_c_ctrl_get_char_en(data)
#define  HD21_errdet_c_ctrl_get_disp_en(data)                           HDMI21_P0_hd21_errdet_c_ctrl_get_disp_en(data)
#define  HD21_errdet_c_ctrl_get_disp_thr(data)                          HDMI21_P0_hd21_errdet_c_ctrl_get_disp_thr(data)
#define  HD21_errdet_c_ctrl_get_char_thr(data)                          HDMI21_P0_hd21_errdet_c_ctrl_get_char_thr(data)


#define  HD21_errdet_r_disparity_error_flag_mask                        HDMI21_P0_hd21_errdet_r_disparity_error_flag_mask
#define  HD21_errdet_r_disparity_error_cnt_sb_max_mask                  HDMI21_P0_hd21_errdet_r_disparity_error_cnt_sb_max_mask
#define  HD21_errdet_r_disparity_error_cnt_acc_mask                     HDMI21_P0_hd21_errdet_r_disparity_error_cnt_acc_mask
#define  HD21_errdet_r_disparity_error_flag(data)                       HDMI21_P0_hd21_errdet_r_disparity_error_flag(data)
#define  HD21_errdet_r_disparity_error_cnt_sb_max(data)                 HDMI21_P0_hd21_errdet_r_disparity_error_cnt_sb_max(data)
#define  HD21_errdet_r_disparity_error_cnt_acc(data)                    HDMI21_P0_hd21_errdet_r_disparity_error_cnt_acc(data)
#define  HD21_errdet_r_disparity_get_error_flag(data)                   HDMI21_P0_hd21_errdet_r_disparity_get_error_flag(data)
#define  HD21_errdet_r_disparity_get_error_cnt_sb_max(data)             HDMI21_P0_hd21_errdet_r_disparity_get_error_cnt_sb_max(data)
#define  HD21_errdet_r_disparity_get_error_cnt_acc(data)                HDMI21_P0_hd21_errdet_r_disparity_get_error_cnt_acc(data)


#define  HD21_errdet_g_disparity_error_flag_mask                        HDMI21_P0_hd21_errdet_g_disparity_error_flag_mask
#define  HD21_errdet_g_disparity_error_cnt_sb_max_mask                  HDMI21_P0_hd21_errdet_g_disparity_error_cnt_sb_max_mask
#define  HD21_errdet_g_disparity_error_cnt_acc_mask                     HDMI21_P0_hd21_errdet_g_disparity_error_cnt_acc_mask
#define  HD21_errdet_g_disparity_error_flag(data)                       HDMI21_P0_hd21_errdet_g_disparity_error_flag(data)
#define  HD21_errdet_g_disparity_error_cnt_sb_max(data)                 HDMI21_P0_hd21_errdet_g_disparity_error_cnt_sb_max(data)
#define  HD21_errdet_g_disparity_error_cnt_acc(data)                    HDMI21_P0_hd21_errdet_g_disparity_error_cnt_acc(data)
#define  HD21_errdet_g_disparity_get_error_flag(data)                   HDMI21_P0_hd21_errdet_g_disparity_get_error_flag(data)
#define  HD21_errdet_g_disparity_get_error_cnt_sb_max(data)             HDMI21_P0_hd21_errdet_g_disparity_get_error_cnt_sb_max(data)
#define  HD21_errdet_g_disparity_get_error_cnt_acc(data)                HDMI21_P0_hd21_errdet_g_disparity_get_error_cnt_acc(data)


#define  HD21_errdet_b_disparity_error_flag_mask                        HDMI21_P0_hd21_errdet_b_disparity_error_flag_mask
#define  HD21_errdet_b_disparity_error_cnt_sb_max_mask                  HDMI21_P0_hd21_errdet_b_disparity_error_cnt_sb_max_mask
#define  HD21_errdet_b_disparity_error_cnt_acc_mask                     HDMI21_P0_hd21_errdet_b_disparity_error_cnt_acc_mask
#define  HD21_errdet_b_disparity_error_flag(data)                       HDMI21_P0_hd21_errdet_b_disparity_error_flag(data)
#define  HD21_errdet_b_disparity_error_cnt_sb_max(data)                 HDMI21_P0_hd21_errdet_b_disparity_error_cnt_sb_max(data)
#define  HD21_errdet_b_disparity_error_cnt_acc(data)                    HDMI21_P0_hd21_errdet_b_disparity_error_cnt_acc(data)
#define  HD21_errdet_b_disparity_get_error_flag(data)                   HDMI21_P0_hd21_errdet_b_disparity_get_error_flag(data)
#define  HD21_errdet_b_disparity_get_error_cnt_sb_max(data)             HDMI21_P0_hd21_errdet_b_disparity_get_error_cnt_sb_max(data)
#define  HD21_errdet_b_disparity_get_error_cnt_acc(data)                HDMI21_P0_hd21_errdet_b_disparity_get_error_cnt_acc(data)


#define  HD21_errdet_c_disparity_error_flag_mask                        HDMI21_P0_hd21_errdet_c_disparity_error_flag_mask
#define  HD21_errdet_c_disparity_error_cnt_sb_max_mask                  HDMI21_P0_hd21_errdet_c_disparity_error_cnt_sb_max_mask
#define  HD21_errdet_c_disparity_error_cnt_acc_mask                     HDMI21_P0_hd21_errdet_c_disparity_error_cnt_acc_mask
#define  HD21_errdet_c_disparity_error_flag(data)                       HDMI21_P0_hd21_errdet_c_disparity_error_flag(data)
#define  HD21_errdet_c_disparity_error_cnt_sb_max(data)                 HDMI21_P0_hd21_errdet_c_disparity_error_cnt_sb_max(data)
#define  HD21_errdet_c_disparity_error_cnt_acc(data)                    HDMI21_P0_hd21_errdet_c_disparity_error_cnt_acc(data)
#define  HD21_errdet_c_disparity_get_error_flag(data)                   HDMI21_P0_hd21_errdet_c_disparity_get_error_flag(data)
#define  HD21_errdet_c_disparity_get_error_cnt_sb_max(data)             HDMI21_P0_hd21_errdet_c_disparity_get_error_cnt_sb_max(data)
#define  HD21_errdet_c_disparity_get_error_cnt_acc(data)                HDMI21_P0_hd21_errdet_c_disparity_get_error_cnt_acc(data)


#define  HD21_errdet_r_char_error_flag_mask                             HDMI21_P0_hd21_errdet_r_char_error_flag_mask
#define  HD21_errdet_r_char_error_cnt_sb_max_mask                       HDMI21_P0_hd21_errdet_r_char_error_cnt_sb_max_mask
#define  HD21_errdet_r_char_error_cnt_acc_mask                          HDMI21_P0_hd21_errdet_r_char_error_cnt_acc_mask
#define  HD21_errdet_r_char_error_flag(data)                            HDMI21_P0_hd21_errdet_r_char_error_flag(data)
#define  HD21_errdet_r_char_error_cnt_sb_max(data)                      HDMI21_P0_hd21_errdet_r_char_error_cnt_sb_max(data)
#define  HD21_errdet_r_char_error_cnt_acc(data)                         HDMI21_P0_hd21_errdet_r_char_error_cnt_acc(data)
#define  HD21_errdet_r_char_get_error_flag(data)                        HDMI21_P0_hd21_errdet_r_char_get_error_flag(data)
#define  HD21_errdet_r_char_get_error_cnt_sb_max(data)                  HDMI21_P0_hd21_errdet_r_char_get_error_cnt_sb_max(data)
#define  HD21_errdet_r_char_get_error_cnt_acc(data)                     HDMI21_P0_hd21_errdet_r_char_get_error_cnt_acc(data)


#define  HD21_errdet_g_char_error_flag_mask                             HDMI21_P0_hd21_errdet_g_char_error_flag_mask
#define  HD21_errdet_g_char_error_cnt_sb_max_mask                       HDMI21_P0_hd21_errdet_g_char_error_cnt_sb_max_mask
#define  HD21_errdet_g_char_error_cnt_acc_mask                          HDMI21_P0_hd21_errdet_g_char_error_cnt_acc_mask
#define  HD21_errdet_g_char_error_flag(data)                            HDMI21_P0_hd21_errdet_g_char_error_flag(data)
#define  HD21_errdet_g_char_error_cnt_sb_max(data)                      HDMI21_P0_hd21_errdet_g_char_error_cnt_sb_max(data)
#define  HD21_errdet_g_char_error_cnt_acc(data)                         HDMI21_P0_hd21_errdet_g_char_error_cnt_acc(data)
#define  HD21_errdet_g_char_get_error_flag(data)                        HDMI21_P0_hd21_errdet_g_char_get_error_flag(data)
#define  HD21_errdet_g_char_get_error_cnt_sb_max(data)                  HDMI21_P0_hd21_errdet_g_char_get_error_cnt_sb_max(data)
#define  HD21_errdet_g_char_get_error_cnt_acc(data)                     HDMI21_P0_hd21_errdet_g_char_get_error_cnt_acc(data)


#define  HD21_errdet_b_char_error_flag_mask                             HDMI21_P0_hd21_errdet_b_char_error_flag_mask
#define  HD21_errdet_b_char_error_cnt_sb_max_mask                       HDMI21_P0_hd21_errdet_b_char_error_cnt_sb_max_mask
#define  HD21_errdet_b_char_error_cnt_acc_mask                          HDMI21_P0_hd21_errdet_b_char_error_cnt_acc_mask
#define  HD21_errdet_b_char_error_flag(data)                            HDMI21_P0_hd21_errdet_b_char_error_flag(data)
#define  HD21_errdet_b_char_error_cnt_sb_max(data)                      HDMI21_P0_hd21_errdet_b_char_error_cnt_sb_max(data)
#define  HD21_errdet_b_char_error_cnt_acc(data)                         HDMI21_P0_hd21_errdet_b_char_error_cnt_acc(data)
#define  HD21_errdet_b_char_get_error_flag(data)                        HDMI21_P0_hd21_errdet_b_char_get_error_flag(data)
#define  HD21_errdet_b_char_get_error_cnt_sb_max(data)                  HDMI21_P0_hd21_errdet_b_char_get_error_cnt_sb_max(data)
#define  HD21_errdet_b_char_get_error_cnt_acc(data)                     HDMI21_P0_hd21_errdet_b_char_get_error_cnt_acc(data)


#define  HD21_errdet_c_char_error_flag_mask                             HDMI21_P0_hd21_errdet_c_char_error_flag_mask
#define  HD21_errdet_c_char_error_cnt_sb_max_mask                       HDMI21_P0_hd21_errdet_c_char_error_cnt_sb_max_mask
#define  HD21_errdet_c_char_error_cnt_acc_mask                          HDMI21_P0_hd21_errdet_c_char_error_cnt_acc_mask
#define  HD21_errdet_c_char_error_flag(data)                            HDMI21_P0_hd21_errdet_c_char_error_flag(data)
#define  HD21_errdet_c_char_error_cnt_sb_max(data)                      HDMI21_P0_hd21_errdet_c_char_error_cnt_sb_max(data)
#define  HD21_errdet_c_char_error_cnt_acc(data)                         HDMI21_P0_hd21_errdet_c_char_error_cnt_acc(data)
#define  HD21_errdet_c_char_get_error_flag(data)                        HDMI21_P0_hd21_errdet_c_char_get_error_flag(data)
#define  HD21_errdet_c_char_get_error_cnt_sb_max(data)                  HDMI21_P0_hd21_errdet_c_char_get_error_cnt_sb_max(data)
#define  HD21_errdet_c_char_get_error_cnt_acc(data)                     HDMI21_P0_hd21_errdet_c_char_get_error_cnt_acc(data)


#define  HD21_disp_ctrl_mask_sb_cnt_mask                          HDMI21_P0_hd21_disp_ctrl_mask_sb_cnt_mask
#define  HD21_disp_ctrl_mask_en_mask                              HDMI21_P0_hd21_disp_ctrl_mask_en_mask
#define  HD21_disp_ctrl_disp_upbound_mask                         HDMI21_P0_hd21_disp_ctrl_disp_upbound_mask
#define  HD21_disp_ctrl_init_disp_value_mask                      HDMI21_P0_hd21_disp_ctrl_init_disp_value_mask
#define  HD21_disp_ctrl_mask_sb_cnt(data)                         HDMI21_P0_hd21_disp_ctrl_mask_sb_cnt(data)
#define  HD21_disp_ctrl_mask_en(data)                             HDMI21_P0_hd21_disp_ctrl_mask_en(data)
#define  HD21_disp_ctrl_disp_upbound(data)                        HDMI21_P0_hd21_disp_ctrl_disp_upbound(data)
#define  HD21_disp_ctrl_init_disp_value(data)                     HDMI21_P0_hd21_disp_ctrl_init_disp_value(data)
#define  HD21_disp_ctrl_get_mask_sb_cnt(data)                     HDMI21_P0_hd21_disp_ctrl_get_mask_sb_cnt(data)
#define  HD21_disp_ctrl_get_mask_en(data)                         HDMI21_P0_hd21_disp_ctrl_get_mask_en(data)
#define  HD21_disp_ctrl_get_disp_upbound(data)                    HDMI21_P0_hd21_disp_ctrl_get_disp_upbound(data)
#define  HD21_disp_ctrl_get_init_disp_value(data)                 HDMI21_P0_hd21_disp_ctrl_get_init_disp_value(data)

#define  HD21_MM_CR_tb_diff_mask                                        HDMI21_P0_HD21_MM_CR_tb_diff_mask
#define  HD21_MM_CR_fix_rate_hact_mask                                  HDMI21_P0_HD21_MM_CR_fix_rate_hact_mask
#define  HD21_MM_CR_tb_diff(data)                                       HDMI21_P0_HD21_MM_CR_tb_diff(data)
#define  HD21_MM_CR_fix_rate_hact(data)                                 HDMI21_P0_HD21_MM_CR_fix_rate_hact(data)
#define  HD21_MM_CR_get_tb_diff(data)                                   HDMI21_P0_HD21_MM_CR_get_tb_diff(data)
#define  HD21_MM_CR_get_fix_rate_hact(data)                             HDMI21_P0_HD21_MM_CR_get_fix_rate_hact(data)


#define  HD21_MM_SR_tb_diff_over_mask                                   HDMI21_P0_HD21_MM_SR_tb_diff_over_mask
#define  HD21_MM_SR_tb_diff_over(data)                                  HDMI21_P0_HD21_MM_SR_tb_diff_over(data)
#define  HD21_MM_SR_get_tb_diff_over(data)                              HDMI21_P0_HD21_MM_SR_get_tb_diff_over(data)


#define  HD21_HDMI_HDRAP_apss_mask                                      HDMI21_P0_hd21_HDMI_HDRAP_apss_mask
#define  HD21_HDMI_HDRAP_apss(data)                                     HDMI21_P0_hd21_HDMI_HDRAP_apss(data)
#define  HD21_HDMI_HDRAP_get_apss(data)                                 HDMI21_P0_hd21_HDMI_HDRAP_get_apss(data)


#define  HD21_HDMI_HDRDP_dpss_mask                                      HDMI21_P0_hd21_HDMI_HDRDP_dpss_mask
#define  HD21_HDMI_HDRDP_dpss(data)                                     HDMI21_P0_hd21_HDMI_HDRDP_dpss(data)
#define  HD21_HDMI_HDRDP_get_dpss(data)                                 HDMI21_P0_hd21_HDMI_HDRDP_get_dpss(data)


#define  HD21_HDR_EM_ST_em_hdr_no_mask                                  HDMI21_P0_HD21_HDR_EM_ST_em_hdr_no_mask
#define  HD21_HDR_EM_ST_em_hdr_done_block_mask                          HDMI21_P0_HD21_HDR_EM_ST_em_hdr_done_block_mask
#define  HD21_HDR_EM_ST_em_hdr_timeout_mask                             HDMI21_P0_HD21_HDR_EM_ST_em_hdr_timeout_mask
#define  HD21_HDR_EM_ST_em_hdr_discont_mask                             HDMI21_P0_HD21_HDR_EM_ST_em_hdr_discont_mask
#define  HD21_HDR_EM_ST_em_hdr_end_mask                                 HDMI21_P0_HD21_HDR_EM_ST_em_hdr_end_mask
#define  HD21_HDR_EM_ST_em_hdr_new_mask                                 HDMI21_P0_HD21_HDR_EM_ST_em_hdr_new_mask
#define  HD21_HDR_EM_ST_em_hdr_first_mask                               HDMI21_P0_HD21_HDR_EM_ST_em_hdr_first_mask
#define  HD21_HDR_EM_ST_em_hdr_last_mask                                HDMI21_P0_HD21_HDR_EM_ST_em_hdr_last_mask
#define  HD21_HDR_EM_ST_em_hdr_no(data)                                 HDMI21_P0_HD21_HDR_EM_ST_em_hdr_no(data)
#define  HD21_HDR_EM_ST_em_hdr_done_block(data)                         HDMI21_P0_HD21_HDR_EM_ST_em_hdr_done_block(data)
#define  HD21_HDR_EM_ST_em_hdr_timeout(data)                            HDMI21_P0_HD21_HDR_EM_ST_em_hdr_timeout(data)
#define  HD21_HDR_EM_ST_em_hdr_discont(data)                            HDMI21_P0_HD21_HDR_EM_ST_em_hdr_discont(data)
#define  HD21_HDR_EM_ST_em_hdr_end(data)                                HDMI21_P0_HD21_HDR_EM_ST_em_hdr_end(data)
#define  HD21_HDR_EM_ST_em_hdr_new(data)                                HDMI21_P0_HD21_HDR_EM_ST_em_hdr_new(data)
#define  HD21_HDR_EM_ST_em_hdr_first(data)                              HDMI21_P0_HD21_HDR_EM_ST_em_hdr_first(data)
#define  HD21_HDR_EM_ST_em_hdr_last(data)                               HDMI21_P0_HD21_HDR_EM_ST_em_hdr_last(data)
#define  HD21_HDR_EM_ST_get_em_hdr_no(data)                             HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_no(data)
#define  HD21_HDR_EM_ST_get_em_hdr_done_block(data)                     HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_done_block(data)
#define  HD21_HDR_EM_ST_get_em_hdr_timeout(data)                        HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_timeout(data)
#define  HD21_HDR_EM_ST_get_em_hdr_discont(data)                        HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_discont(data)
#define  HD21_HDR_EM_ST_get_em_hdr_end(data)                            HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_end(data)
#define  HD21_HDR_EM_ST_get_em_hdr_new(data)                            HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_new(data)
#define  HD21_HDR_EM_ST_get_em_hdr_first(data)                          HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_first(data)
#define  HD21_HDR_EM_ST_get_em_hdr_last(data)                           HDMI21_P0_HD21_HDR_EM_ST_get_em_hdr_last(data)


#define  HD21_HDR_EM_CT_org_id_mask                                     HDMI21_P0_HD21_HDR_EM_CT_org_id_mask
#define  HD21_HDR_EM_CT_set_tag_msb_mask                                HDMI21_P0_HD21_HDR_EM_CT_set_tag_msb_mask
#define  HD21_HDR_EM_CT_set_tag_lsb_mask                                HDMI21_P0_HD21_HDR_EM_CT_set_tag_lsb_mask
#define  HD21_HDR_EM_CT_recognize_tag_en_mask                           HDMI21_P0_HD21_HDR_EM_CT_recognize_tag_en_mask
#define  HD21_HDR_EM_CT_hdr_pkt_cnt_clr_mask                            HDMI21_P0_HD21_HDR_EM_CT_hdr_pkt_cnt_clr_mask
#define  HD21_HDR_EM_CT_hdr_block_sel_mask                              HDMI21_P0_HD21_HDR_EM_CT_hdr_block_sel_mask
#define  HD21_HDR_EM_CT_em_err2_proc_mask                               HDMI21_P0_HD21_HDR_EM_CT_em_err2_proc_mask
#define  HD21_HDR_EM_CT_rec_em_hdr_irq_en_mask                          HDMI21_P0_HD21_HDR_EM_CT_rec_em_hdr_irq_en_mask
#define  HD21_HDR_EM_CT_org_id(data)                                    HDMI21_P0_HD21_HDR_EM_CT_org_id(data)
#define  HD21_HDR_EM_CT_set_tag_msb(data)                               HDMI21_P0_HD21_HDR_EM_CT_set_tag_msb(data)
#define  HD21_HDR_EM_CT_set_tag_lsb(data)                               HDMI21_P0_HD21_HDR_EM_CT_set_tag_lsb(data)
#define  HD21_HDR_EM_CT_recognize_tag_en(data)                          HDMI21_P0_HD21_HDR_EM_CT_recognize_tag_en(data)
#define  HD21_HDR_EM_CT_hdr_pkt_cnt_clr(data)                           HDMI21_P0_HD21_HDR_EM_CT_hdr_pkt_cnt_clr(data)
#define  HD21_HDR_EM_CT_hdr_block_sel(data)                             HDMI21_P0_HD21_HDR_EM_CT_hdr_block_sel(data)
#define  HD21_HDR_EM_CT_em_err2_proc(data)                              HDMI21_P0_HD21_HDR_EM_CT_em_err2_proc(data)
#define  HD21_HDR_EM_CT_rec_em_hdr_irq_en(data)                         HDMI21_P0_HD21_HDR_EM_CT_rec_em_hdr_irq_en(data)
#define  HD21_HDR_EM_CT_get_org_id(data)                                HDMI21_P0_HD21_HDR_EM_CT_get_org_id(data)
#define  HD21_HDR_EM_CT_get_set_tag_msb(data)                           HDMI21_P0_HD21_HDR_EM_CT_get_set_tag_msb(data)
#define  HD21_HDR_EM_CT_get_set_tag_lsb(data)                           HDMI21_P0_HD21_HDR_EM_CT_get_set_tag_lsb(data)
#define  HD21_HDR_EM_CT_get_recognize_tag_en(data)                      HDMI21_P0_HD21_HDR_EM_CT_get_recognize_tag_en(data)
#define  HD21_HDR_EM_CT_get_hdr_pkt_cnt_clr(data)                       HDMI21_P0_HD21_HDR_EM_CT_get_hdr_pkt_cnt_clr(data)
#define  HD21_HDR_EM_CT_get_hdr_block_sel(data)                         HDMI21_P0_HD21_HDR_EM_CT_get_hdr_block_sel(data)
#define  HD21_HDR_EM_CT_get_em_err2_proc(data)                          HDMI21_P0_HD21_HDR_EM_CT_get_em_err2_proc(data)
#define  HD21_HDR_EM_CT_get_rec_em_hdr_irq_en(data)                     HDMI21_P0_HD21_HDR_EM_CT_get_rec_em_hdr_irq_en(data)


#define  HD21_HDR_EM_CT2_sync_mask                                      HDMI21_P0_HD21_HDR_EM_CT2_sync_mask
#define  HD21_HDR_EM_CT2_recognize_oui_en_mask                          HDMI21_P0_HD21_HDR_EM_CT2_recognize_oui_en_mask
#define  HD21_HDR_EM_CT2_oui_3rd_mask                                   HDMI21_P0_HD21_HDR_EM_CT2_oui_3rd_mask
#define  HD21_HDR_EM_CT2_oui_2nd_mask                                   HDMI21_P0_HD21_HDR_EM_CT2_oui_2nd_mask
#define  HD21_HDR_EM_CT2_oui_1st_mask                                   HDMI21_P0_HD21_HDR_EM_CT2_oui_1st_mask
#define  HD21_HDR_EM_CT2_sync(data)                                     HDMI21_P0_HD21_HDR_EM_CT2_sync(data)
#define  HD21_HDR_EM_CT2_recognize_oui_en(data)                         HDMI21_P0_HD21_HDR_EM_CT2_recognize_oui_en(data)
#define  HD21_HDR_EM_CT2_oui_3rd(data)                                  HDMI21_P0_HD21_HDR_EM_CT2_oui_3rd(data)
#define  HD21_HDR_EM_CT2_oui_2nd(data)                                  HDMI21_P0_HD21_HDR_EM_CT2_oui_2nd(data)
#define  HD21_HDR_EM_CT2_oui_1st(data)                                  HDMI21_P0_HD21_HDR_EM_CT2_oui_1st(data)
#define  HD21_HDR_EM_CT2_get_sync(data)                                 HDMI21_P0_HD21_HDR_EM_CT2_get_sync(data)
#define  HD21_HDR_EM_CT2_get_recognize_oui_en(data)                     HDMI21_P0_HD21_HDR_EM_CT2_get_recognize_oui_en(data)
#define  HD21_HDR_EM_CT2_get_oui_3rd(data)                              HDMI21_P0_HD21_HDR_EM_CT2_get_oui_3rd(data)
#define  HD21_HDR_EM_CT2_get_oui_2nd(data)                              HDMI21_P0_HD21_HDR_EM_CT2_get_oui_2nd(data)
#define  HD21_HDR_EM_CT2_get_oui_1st(data)                              HDMI21_P0_HD21_HDR_EM_CT2_get_oui_1st(data)


#define  HD21_HDMI_EMAP_apss_mask                                       HDMI21_P0_HD21_HDMI_EMAP_apss_mask
#define  HD21_HDMI_EMAP_apss(data)                                      HDMI21_P0_HD21_HDMI_EMAP_apss(data)
#define  HD21_HDMI_EMAP_get_apss(data)                                  HDMI21_P0_HD21_HDMI_EMAP_get_apss(data)


#define  HD21_HDMI_EMDP_dpss_mask                                       HDMI21_P0_hd21_HDMI_EMDP_dpss_mask
#define  HD21_HDMI_EMDP_dpss(data)                                      HDMI21_P0_hd21_HDMI_EMDP_dpss(data)
#define  HD21_HDMI_EMDP_get_dpss(data)                                  HDMI21_P0_hd21_HDMI_EMDP_get_dpss(data)


#define  HD21_fapa_ct_region_cnt_mask                                   HDMI21_P0_HD21_fapa_ct_region_cnt_mask
#define  HD21_fapa_ct_em_timeout_frame_mask                             HDMI21_P0_HD21_fapa_ct_em_timeout_frame_mask
#define  HD21_fapa_ct_mode_mask                                         HDMI21_P0_HD21_fapa_ct_mode_mask
#define  HD21_fapa_ct_region_cnt(data)                                  HDMI21_P0_HD21_fapa_ct_region_cnt(data)
#define  HD21_fapa_ct_em_timeout_frame(data)                            HDMI21_P0_HD21_fapa_ct_em_timeout_frame(data)
#define  HD21_fapa_ct_mode(data)                                        HDMI21_P0_HD21_fapa_ct_mode(data)
#define  HD21_fapa_ct_get_region_cnt(data)                              HDMI21_P0_HD21_fapa_ct_get_region_cnt(data)
#define  HD21_fapa_ct_get_em_timeout_frame(data)                        HDMI21_P0_HD21_fapa_ct_get_em_timeout_frame(data)
#define  HD21_fapa_ct_get_mode(data)                                    HDMI21_P0_HD21_fapa_ct_get_mode(data)


#define  HD21_fapa_ct2_region_cnt_msb_mask                        HDMI21_P0_HD21_fapa_ct2_region_cnt_msb_mask
#define  HD21_fapa_ct2_region_cnt_msb(data)                       HDMI21_P0_HD21_fapa_ct2_region_cnt_msb(data)
#define  HD21_fapa_ct2_get_region_cnt_msb(data)                   HDMI21_P0_HD21_fapa_ct2_get_region_cnt_msb(data)


#define  HD21_VSEM_ST_em_vsem_no_mask                                   HDMI21_P0_HD21_VSEM_ST_em_vsem_no_mask
#define  HD21_VSEM_ST_em_vsem_timeout_mask                              HDMI21_P0_HD21_VSEM_ST_em_vsem_timeout_mask
#define  HD21_VSEM_ST_em_vsem_discont_mask                              HDMI21_P0_HD21_VSEM_ST_em_vsem_discont_mask
#define  HD21_VSEM_ST_em_vsem_end_mask                                  HDMI21_P0_HD21_VSEM_ST_em_vsem_end_mask
#define  HD21_VSEM_ST_em_vsem_new_mask                                  HDMI21_P0_HD21_VSEM_ST_em_vsem_new_mask
#define  HD21_VSEM_ST_em_vsem_first_mask                                HDMI21_P0_HD21_VSEM_ST_em_vsem_first_mask
#define  HD21_VSEM_ST_em_vsem_last_mask                                 HDMI21_P0_HD21_VSEM_ST_em_vsem_last_mask
#define  HD21_VSEM_ST_em_vsem_no(data)                                  HDMI21_P0_HD21_VSEM_ST_em_vsem_no(data)
#define  HD21_VSEM_ST_em_vsem_timeout(data)                             HDMI21_P0_HD21_VSEM_ST_em_vsem_timeout(data)
#define  HD21_VSEM_ST_em_vsem_discont(data)                             HDMI21_P0_HD21_VSEM_ST_em_vsem_discont(data)
#define  HD21_VSEM_ST_em_vsem_end(data)                                 HDMI21_P0_HD21_VSEM_ST_em_vsem_end(data)
#define  HD21_VSEM_ST_em_vsem_new(data)                                 HDMI21_P0_HD21_VSEM_ST_em_vsem_new(data)
#define  HD21_VSEM_ST_em_vsem_first(data)                               HDMI21_P0_HD21_VSEM_ST_em_vsem_first(data)
#define  HD21_VSEM_ST_em_vsem_last(data)                                HDMI21_P0_HD21_VSEM_ST_em_vsem_last(data)
#define  HD21_VSEM_ST_get_em_vsem_no(data)                              HDMI21_P0_HD21_VSEM_ST_get_em_vsem_no(data)
#define  HD21_VSEM_ST_get_em_vsem_timeout(data)                         HDMI21_P0_HD21_VSEM_ST_get_em_vsem_timeout(data)
#define  HD21_VSEM_ST_get_em_vsem_discont(data)                         HDMI21_P0_HD21_VSEM_ST_get_em_vsem_discont(data)
#define  HD21_VSEM_ST_get_em_vsem_end(data)                             HDMI21_P0_HD21_VSEM_ST_get_em_vsem_end(data)
#define  HD21_VSEM_ST_get_em_vsem_new(data)                             HDMI21_P0_HD21_VSEM_ST_get_em_vsem_new(data)
#define  HD21_VSEM_ST_get_em_vsem_first(data)                           HDMI21_P0_HD21_VSEM_ST_get_em_vsem_first(data)
#define  HD21_VSEM_ST_get_em_vsem_last(data)                            HDMI21_P0_HD21_VSEM_ST_get_em_vsem_last(data)


#define  HD21_VRREM_ST_em_vrr_no_mask                                   HDMI21_P0_HD21_VRREM_ST_em_vrr_no_mask
#define  HD21_VRREM_ST_em_vrr_timeout_mask                              HDMI21_P0_HD21_VRREM_ST_em_vrr_timeout_mask
#define  HD21_VRREM_ST_em_vrr_discont_mask                              HDMI21_P0_HD21_VRREM_ST_em_vrr_discont_mask
#define  HD21_VRREM_ST_em_vrr_end_mask                                  HDMI21_P0_HD21_VRREM_ST_em_vrr_end_mask
#define  HD21_VRREM_ST_em_vrr_new_mask                                  HDMI21_P0_HD21_VRREM_ST_em_vrr_new_mask
#define  HD21_VRREM_ST_em_vrr_first_mask                                HDMI21_P0_HD21_VRREM_ST_em_vrr_first_mask
#define  HD21_VRREM_ST_em_vrr_last_mask                                 HDMI21_P0_HD21_VRREM_ST_em_vrr_last_mask
#define  HD21_VRREM_ST_em_vrr_no(data)                                  HDMI21_P0_HD21_VRREM_ST_em_vrr_no(data)
#define  HD21_VRREM_ST_em_vrr_timeout(data)                             HDMI21_P0_HD21_VRREM_ST_em_vrr_timeout(data)
#define  HD21_VRREM_ST_em_vrr_discont(data)                             HDMI21_P0_HD21_VRREM_ST_em_vrr_discont(data)
#define  HD21_VRREM_ST_em_vrr_end(data)                                 HDMI21_P0_HD21_VRREM_ST_em_vrr_end(data)
#define  HD21_VRREM_ST_em_vrr_new(data)                                 HDMI21_P0_HD21_VRREM_ST_em_vrr_new(data)
#define  HD21_VRREM_ST_em_vrr_first(data)                               HDMI21_P0_HD21_VRREM_ST_em_vrr_first(data)
#define  HD21_VRREM_ST_em_vrr_last(data)                                HDMI21_P0_HD21_VRREM_ST_em_vrr_last(data)
#define  HD21_VRREM_ST_get_em_vrr_no(data)                              HDMI21_P0_HD21_VRREM_ST_get_em_vrr_no(data)
#define  HD21_VRREM_ST_get_em_vrr_timeout(data)                         HDMI21_P0_HD21_VRREM_ST_get_em_vrr_timeout(data)
#define  HD21_VRREM_ST_get_em_vrr_discont(data)                         HDMI21_P0_HD21_VRREM_ST_get_em_vrr_discont(data)
#define  HD21_VRREM_ST_get_em_vrr_end(data)                             HDMI21_P0_HD21_VRREM_ST_get_em_vrr_end(data)
#define  HD21_VRREM_ST_get_em_vrr_new(data)                             HDMI21_P0_HD21_VRREM_ST_get_em_vrr_new(data)
#define  HD21_VRREM_ST_get_em_vrr_first(data)                           HDMI21_P0_HD21_VRREM_ST_get_em_vrr_first(data)
#define  HD21_VRREM_ST_get_em_vrr_last(data)                            HDMI21_P0_HD21_VRREM_ST_get_em_vrr_last(data)


#define  HD21_CVTEM_ST_em_cvtem_no_mask                                 HDMI21_P0_HD21_CVTEM_ST_em_cvtem_no_mask
#define  HD21_CVTEM_ST_em_cvtem_timeout_mask                            HDMI21_P0_HD21_CVTEM_ST_em_cvtem_timeout_mask
#define  HD21_CVTEM_ST_em_cvtem_discont_mask                            HDMI21_P0_HD21_CVTEM_ST_em_cvtem_discont_mask
#define  HD21_CVTEM_ST_em_cvtem_end_mask                                HDMI21_P0_HD21_CVTEM_ST_em_cvtem_end_mask
#define  HD21_CVTEM_ST_em_cvtem_new_mask                                HDMI21_P0_HD21_CVTEM_ST_em_cvtem_new_mask
#define  HD21_CVTEM_ST_em_cvtem_first_mask                              HDMI21_P0_HD21_CVTEM_ST_em_cvtem_first_mask
#define  HD21_CVTEM_ST_em_cvtem_last_mask                               HDMI21_P0_HD21_CVTEM_ST_em_cvtem_last_mask
#define  HD21_CVTEM_ST_em_cvtem_no(data)                                HDMI21_P0_HD21_CVTEM_ST_em_cvtem_no(data)
#define  HD21_CVTEM_ST_em_cvtem_timeout(data)                           HDMI21_P0_HD21_CVTEM_ST_em_cvtem_timeout(data)
#define  HD21_CVTEM_ST_em_cvtem_discont(data)                           HDMI21_P0_HD21_CVTEM_ST_em_cvtem_discont(data)
#define  HD21_CVTEM_ST_em_cvtem_end(data)                               HDMI21_P0_HD21_CVTEM_ST_em_cvtem_end(data)
#define  HD21_CVTEM_ST_em_cvtem_new(data)                               HDMI21_P0_HD21_CVTEM_ST_em_cvtem_new(data)
#define  HD21_CVTEM_ST_em_cvtem_first(data)                             HDMI21_P0_HD21_CVTEM_ST_em_cvtem_first(data)
#define  HD21_CVTEM_ST_em_cvtem_last(data)                              HDMI21_P0_HD21_CVTEM_ST_em_cvtem_last(data)
#define  HD21_CVTEM_ST_get_em_cvtem_no(data)                            HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_no(data)
#define  HD21_CVTEM_ST_get_em_cvtem_timeout(data)                       HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_timeout(data)
#define  HD21_CVTEM_ST_get_em_cvtem_discont(data)                       HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_discont(data)
#define  HD21_CVTEM_ST_get_em_cvtem_end(data)                           HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_end(data)
#define  HD21_CVTEM_ST_get_em_cvtem_new(data)                           HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_new(data)
#define  HD21_CVTEM_ST_get_em_cvtem_first(data)                         HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_first(data)
#define  HD21_CVTEM_ST_get_em_cvtem_last(data)                          HDMI21_P0_HD21_CVTEM_ST_get_em_cvtem_last(data)


#define  HD21_HDMI_VSEM_EMC_set_tag_msb_mask                            HDMI21_P0_HD21_HDMI_VSEM_EMC_set_tag_msb_mask
#define  HD21_HDMI_VSEM_EMC_set_tag_lsb_mask                            HDMI21_P0_HD21_HDMI_VSEM_EMC_set_tag_lsb_mask
#define  HD21_HDMI_VSEM_EMC_recognize_tag_en_mask                       HDMI21_P0_HD21_HDMI_VSEM_EMC_recognize_tag_en_mask
#define  HD21_HDMI_VSEM_EMC_vsem_clr_mask                               HDMI21_P0_HD21_HDMI_VSEM_EMC_vsem_clr_mask
#define  HD21_HDMI_VSEM_EMC_rec_em_vsem_irq_en_mask                     HDMI21_P0_HD21_HDMI_VSEM_EMC_rec_em_vsem_irq_en_mask
#define  HD21_HDMI_VSEM_EMC_set_tag_msb(data)                           HDMI21_P0_HD21_HDMI_VSEM_EMC_set_tag_msb(data)
#define  HD21_HDMI_VSEM_EMC_set_tag_lsb(data)                           HDMI21_P0_HD21_HDMI_VSEM_EMC_set_tag_lsb(data)
#define  HD21_HDMI_VSEM_EMC_recognize_tag_en(data)                      HDMI21_P0_HD21_HDMI_VSEM_EMC_recognize_tag_en(data)
#define  HD21_HDMI_VSEM_EMC_vsem_clr(data)                              HDMI21_P0_HD21_HDMI_VSEM_EMC_vsem_clr(data)
#define  HD21_HDMI_VSEM_EMC_rec_em_vsem_irq_en(data)                    HDMI21_P0_HD21_HDMI_VSEM_EMC_rec_em_vsem_irq_en(data)
#define  HD21_HDMI_VSEM_EMC_get_set_tag_msb(data)                       HDMI21_P0_HD21_HDMI_VSEM_EMC_get_set_tag_msb(data)
#define  HD21_HDMI_VSEM_EMC_get_set_tag_lsb(data)                       HDMI21_P0_HD21_HDMI_VSEM_EMC_get_set_tag_lsb(data)
#define  HD21_HDMI_VSEM_EMC_get_recognize_tag_en(data)                  HDMI21_P0_HD21_HDMI_VSEM_EMC_get_recognize_tag_en(data)
#define  HD21_HDMI_VSEM_EMC_get_vsem_clr(data)                          HDMI21_P0_HD21_HDMI_VSEM_EMC_get_vsem_clr(data)
#define  HD21_HDMI_VSEM_EMC_get_rec_em_vsem_irq_en(data)                HDMI21_P0_HD21_HDMI_VSEM_EMC_get_rec_em_vsem_irq_en(data)


#define  HD21_HDMI_VRR_EMC_vrr_clr_mask                                 HDMI21_P0_HD21_HDMI_VRR_EMC_vrr_clr_mask
#define  HD21_HDMI_VRR_EMC_rec_em_vrr_irq_en_mask                       HDMI21_P0_HD21_HDMI_VRR_EMC_rec_em_vrr_irq_en_mask
#define  HD21_HDMI_VRR_EMC_vrr_clr(data)                                HDMI21_P0_HD21_HDMI_VRR_EMC_vrr_clr(data)
#define  HD21_HDMI_VRR_EMC_rec_em_vrr_irq_en(data)                      HDMI21_P0_HD21_HDMI_VRR_EMC_rec_em_vrr_irq_en(data)
#define  HD21_HDMI_VRR_EMC_get_vrr_clr(data)                            HDMI21_P0_HD21_HDMI_VRR_EMC_get_vrr_clr(data)
#define  HD21_HDMI_VRR_EMC_get_rec_em_vrr_irq_en(data)                  HDMI21_P0_HD21_HDMI_VRR_EMC_get_rec_em_vrr_irq_en(data)


#define  HD21_HDMI_CVTEM_EMC_cvtem_clr_mask                             HDMI21_P0_HD21_HDMI_CVTEM_EMC_cvtem_clr_mask
#define  HD21_HDMI_CVTEM_EMC_rec_em_cvtem_irq_en_mask                   HDMI21_P0_HD21_HDMI_CVTEM_EMC_rec_em_cvtem_irq_en_mask
#define  HD21_HDMI_CVTEM_EMC_cvtem_clr(data)                            HDMI21_P0_HD21_HDMI_CVTEM_EMC_cvtem_clr(data)
#define  HD21_HDMI_CVTEM_EMC_rec_em_cvtem_irq_en(data)                  HDMI21_P0_HD21_HDMI_CVTEM_EMC_rec_em_cvtem_irq_en(data)
#define  HD21_HDMI_CVTEM_EMC_get_cvtem_clr(data)                        HDMI21_P0_HD21_HDMI_CVTEM_EMC_get_cvtem_clr(data)
#define  HD21_HDMI_CVTEM_EMC_get_rec_em_cvtem_irq_en(data)              HDMI21_P0_HD21_HDMI_CVTEM_EMC_get_rec_em_cvtem_irq_en(data)


#define  HD21_HDMI_VSEM_EMC2_sync_mask                                  HDMI21_P0_HD21_HDMI_VSEM_EMC2_sync_mask
#define  HD21_HDMI_VSEM_EMC2_recognize_oui_en_mask                      HDMI21_P0_HD21_HDMI_VSEM_EMC2_recognize_oui_en_mask
#define  HD21_HDMI_VSEM_EMC2_oui_3rd_mask                               HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_3rd_mask
#define  HD21_HDMI_VSEM_EMC2_oui_2nd_mask                               HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_2nd_mask
#define  HD21_HDMI_VSEM_EMC2_oui_1st_mask                               HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_1st_mask
#define  HD21_HDMI_VSEM_EMC2_sync(data)                                 HDMI21_P0_HD21_HDMI_VSEM_EMC2_sync(data)
#define  HD21_HDMI_VSEM_EMC2_recognize_oui_en(data)                     HDMI21_P0_HD21_HDMI_VSEM_EMC2_recognize_oui_en(data)
#define  HD21_HDMI_VSEM_EMC2_oui_3rd(data)                              HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_3rd(data)
#define  HD21_HDMI_VSEM_EMC2_oui_2nd(data)                              HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_2nd(data)
#define  HD21_HDMI_VSEM_EMC2_oui_1st(data)                              HDMI21_P0_HD21_HDMI_VSEM_EMC2_oui_1st(data)
#define  HD21_HDMI_VSEM_EMC2_get_sync(data)                             HDMI21_P0_HD21_HDMI_VSEM_EMC2_get_sync(data)
#define  HD21_HDMI_VSEM_EMC2_get_recognize_oui_en(data)                 HDMI21_P0_HD21_HDMI_VSEM_EMC2_get_recognize_oui_en(data)
#define  HD21_HDMI_VSEM_EMC2_get_oui_3rd(data)                          HDMI21_P0_HD21_HDMI_VSEM_EMC2_get_oui_3rd(data)
#define  HD21_HDMI_VSEM_EMC2_get_oui_2nd(data)                          HDMI21_P0_HD21_HDMI_VSEM_EMC2_get_oui_2nd(data)
#define  HD21_HDMI_VSEM_EMC2_get_oui_1st(data)                          HDMI21_P0_HD21_HDMI_VSEM_EMC2_get_oui_1st(data)


#define  HD21_rmzp_hcbyte_fw_mask                                       HDMI21_P0_HD21_rmzp_hcbyte_fw_mask
#define  HD21_rmzp_fw_apply_en_mask                                     HDMI21_P0_HD21_rmzp_fw_apply_en_mask
#define  HD21_rmzp_less_empacket_err_mask                               HDMI21_P0_HD21_rmzp_less_empacket_err_mask
#define  HD21_rmzp_more_empacket_err_mask                               HDMI21_P0_HD21_rmzp_more_empacket_err_mask
#define  HD21_rmzp_hcbyte_fw(data)                                      HDMI21_P0_HD21_rmzp_hcbyte_fw(data)
#define  HD21_rmzp_fw_apply_en(data)                                    HDMI21_P0_HD21_rmzp_fw_apply_en(data)
#define  HD21_rmzp_less_empacket_err(data)                              HDMI21_P0_HD21_rmzp_less_empacket_err(data)
#define  HD21_rmzp_more_empacket_err(data)                              HDMI21_P0_HD21_rmzp_more_empacket_err(data)
#define  HD21_rmzp_get_hcbyte_fw(data)                                  HDMI21_P0_HD21_rmzp_get_hcbyte_fw(data)
#define  HD21_rmzp_get_fw_apply_en(data)                                HDMI21_P0_HD21_rmzp_get_fw_apply_en(data)
#define  HD21_rmzp_get_less_empacket_err(data)                          HDMI21_P0_HD21_rmzp_get_less_empacket_err(data)
#define  HD21_rmzp_get_more_empacket_err(data)                          HDMI21_P0_HD21_rmzp_get_more_empacket_err(data)


#define  HD21_cercr_err_cnt_sel_mask                                    HDMI21_P0_hd21_cercr_err_cnt_sel_mask
#define  HD21_cercr_scdc_ced_en_mask                                    HDMI21_P0_hd21_cercr_scdc_ced_en_mask
#define  HD21_cercr_valid_reset_mask                                    HDMI21_P0_hd21_cercr_valid_reset_mask
#define  HD21_cercr_reset_err_det_mask                                  HDMI21_P0_hd21_cercr_reset_err_det_mask
#define  HD21_cercr_keep_err_det_mask                                   HDMI21_P0_hd21_cercr_keep_err_det_mask
#define  HD21_cercr_reset_mask                                          HDMI21_P0_hd21_cercr_reset_mask
#define  HD21_cercr_period_mask                                         HDMI21_P0_hd21_cercr_period_mask
#define  HD21_cercr_ced_upd_num_mask                                    HDMI21_P0_hd21_cercr_ced_upd_num_mask
#define  HD21_cercr_mode_mask                                           HDMI21_P0_hd21_cercr_mode_mask
#define  HD21_cercr_en_mask                                             HDMI21_P0_hd21_cercr_en_mask
#define  HD21_cercr_err_cnt_sel(data)                                   HDMI21_P0_hd21_cercr_err_cnt_sel(data)
#define  HD21_cercr_scdc_ced_en(data)                                   HDMI21_P0_hd21_cercr_scdc_ced_en(data)
#define  HD21_cercr_valid_reset(data)                                   HDMI21_P0_hd21_cercr_valid_reset(data)
#define  HD21_cercr_reset_err_det(data)                                 HDMI21_P0_hd21_cercr_reset_err_det(data)
#define  HD21_cercr_keep_err_det(data)                                  HDMI21_P0_hd21_cercr_keep_err_det(data)
#define  HD21_cercr_reset(data)                                         HDMI21_P0_hd21_cercr_reset(data)
#define  HD21_cercr_period(data)                                        HDMI21_P0_hd21_cercr_period(data)
#define  HD21_cercr_ced_upd_num(data)                                   HDMI21_P0_hd21_cercr_ced_upd_num(data)
#define  HD21_cercr_mode(data)                                          HDMI21_P0_hd21_cercr_mode(data)
#define  HD21_cercr_en(data)                                            HDMI21_P0_hd21_cercr_en(data)
#define  HD21_cercr_get_err_cnt_sel(data)                               HDMI21_P0_hd21_cercr_get_err_cnt_sel(data)
#define  HD21_cercr_get_scdc_ced_en(data)                               HDMI21_P0_hd21_cercr_get_scdc_ced_en(data)
#define  HD21_cercr_get_valid_reset(data)                               HDMI21_P0_hd21_cercr_get_valid_reset(data)
#define  HD21_cercr_get_reset_err_det(data)                             HDMI21_P0_hd21_cercr_get_reset_err_det(data)
#define  HD21_cercr_get_keep_err_det(data)                              HDMI21_P0_hd21_cercr_get_keep_err_det(data)
#define  HD21_cercr_get_reset(data)                                     HDMI21_P0_hd21_cercr_get_reset(data)
#define  HD21_cercr_get_period(data)                                    HDMI21_P0_hd21_cercr_get_period(data)
#define  HD21_cercr_get_ced_upd_num(data)                               HDMI21_P0_hd21_cercr_get_ced_upd_num(data)
#define  HD21_cercr_get_mode(data)                                      HDMI21_P0_hd21_cercr_get_mode(data)
#define  HD21_cercr_get_en(data)                                        HDMI21_P0_hd21_cercr_get_en(data)


#define  HD21_CERSR0_err_cnt1_mask                                      HDMI21_P0_HD21_CERSR0_err_cnt1_mask
#define  HD21_CERSR0_err_cnt0_mask                                      HDMI21_P0_HD21_CERSR0_err_cnt0_mask
#define  HD21_CERSR0_err_cnt1(data)                                     HDMI21_P0_HD21_CERSR0_err_cnt1(data)
#define  HD21_CERSR0_err_cnt0(data)                                     HDMI21_P0_HD21_CERSR0_err_cnt0(data)
#define  HD21_CERSR0_get_err_cnt1(data)                                 HDMI21_P0_HD21_CERSR0_get_err_cnt1(data)
#define  HD21_CERSR0_get_err_cnt0(data)                                 HDMI21_P0_HD21_CERSR0_get_err_cnt0(data)


#define  HD21_CERSR1_err_cnt3_mask                                      HDMI21_P0_HD21_CERSR1_err_cnt3_mask
#define  HD21_CERSR1_err_cnt2_mask                                      HDMI21_P0_HD21_CERSR1_err_cnt2_mask
#define  HD21_CERSR1_err_cnt3(data)                                     HDMI21_P0_HD21_CERSR1_err_cnt3(data)
#define  HD21_CERSR1_err_cnt2(data)                                     HDMI21_P0_HD21_CERSR1_err_cnt2(data)
#define  HD21_CERSR1_get_err_cnt3(data)                                 HDMI21_P0_HD21_CERSR1_get_err_cnt3(data)
#define  HD21_CERSR1_get_err_cnt2(data)                                 HDMI21_P0_HD21_CERSR1_get_err_cnt2(data)


#define  HD21_CERSR2_rs_err_cnt_mask                                    HDMI21_P0_HD21_CERSR2_rs_err_cnt_mask
#define  HD21_CERSR2_rs_err_cnt(data)                                   HDMI21_P0_HD21_CERSR2_rs_err_cnt(data)
#define  HD21_CERSR2_get_rs_err_cnt(data)                               HDMI21_P0_HD21_CERSR2_get_rs_err_cnt(data)

#define  HD21_CRC_Ctrl_crc_res_sel_mask                                  HDMI21_P0_HD21_CRC_Ctrl_crc_res_sel_mask
#define  HD21_CRC_Ctrl_crc_auto_cmp_en_mask                              HDMI21_P0_HD21_CRC_Ctrl_crc_auto_cmp_en_mask
#define  HD21_CRC_Ctrl_crc_conti_mask                                    HDMI21_P0_HD21_CRC_Ctrl_crc_conti_mask
#define  HD21_CRC_Ctrl_crc_start_mask                                    HDMI21_P0_HD21_CRC_Ctrl_crc_start_mask
#define  HD21_CRC_Ctrl_crc_res_sel(data)                                 HDMI21_P0_HD21_CRC_Ctrl_crc_res_sel(data)
#define  HD21_CRC_Ctrl_crc_auto_cmp_en(data)                             HDMI21_P0_HD21_CRC_Ctrl_crc_auto_cmp_en(data)
#define  HD21_CRC_Ctrl_crc_conti(data)                                   HDMI21_P0_HD21_CRC_Ctrl_crc_conti(data)
#define  HD21_CRC_Ctrl_crc_start(data)                                   HDMI21_P0_HD21_CRC_Ctrl_crc_start(data)
#define  HD21_CRC_Ctrl_get_crc_res_sel(data)                             HDMI21_P0_HD21_CRC_Ctrl_get_crc_res_sel(data)
#define  HD21_CRC_Ctrl_get_crc_auto_cmp_en(data)                         HDMI21_P0_HD21_CRC_Ctrl_get_crc_auto_cmp_en(data)
#define  HD21_CRC_Ctrl_get_crc_conti(data)                               HDMI21_P0_HD21_CRC_Ctrl_get_crc_conti(data)
#define  HD21_CRC_Ctrl_get_crc_start(data)                               HDMI21_P0_HD21_CRC_Ctrl_get_crc_start(data)


#define  HD21_CRC_Result_crc_result_mask                                 HDMI21_P0_HD21_CRC_Result_crc_result_mask
#define  HD21_CRC_Result_crc_result(data)                                HDMI21_P0_HD21_CRC_Result_crc_result(data)
#define  HD21_CRC_Result_get_crc_result(data)                            HDMI21_P0_HD21_CRC_Result_get_crc_result(data)


#define  HD21_DES_CRC_crc_des_mask                                       HDMI21_P0_HD21_DES_CRC_crc_des_mask
#define  HD21_DES_CRC_crc_des(data)                                      HDMI21_P0_HD21_DES_CRC_crc_des(data)
#define  HD21_DES_CRC_get_crc_des(data)                                  HDMI21_P0_HD21_DES_CRC_get_crc_des(data)


#define  HD21_CRC_ERR_CNT0_crc_err_cnt_mask                              HDMI21_P0_HD21_CRC_ERR_CNT0_crc_err_cnt_mask
#define  HD21_CRC_ERR_CNT0_crc_err_cnt(data)                             HDMI21_P0_HD21_CRC_ERR_CNT0_crc_err_cnt(data)
#define  HD21_CRC_ERR_CNT0_get_crc_err_cnt(data)                         HDMI21_P0_HD21_CRC_ERR_CNT0_get_crc_err_cnt(data)

#define  HD21_TIMING_GEN_CR_tg_window_mask                              HDMI21_P0_HD21_TIMING_GEN_CR_tg_window_mask
#define  HD21_TIMING_GEN_CR_tg_en_mask                                  HDMI21_P0_HD21_TIMING_GEN_CR_tg_en_mask
#define  HD21_TIMING_GEN_CR_tg_window(data)                             HDMI21_P0_HD21_TIMING_GEN_CR_tg_window(data)
#define  HD21_TIMING_GEN_CR_tg_en(data)                                 HDMI21_P0_HD21_TIMING_GEN_CR_tg_en(data)
#define  HD21_TIMING_GEN_CR_get_tg_window(data)                         HDMI21_P0_HD21_TIMING_GEN_CR_get_tg_window(data)
#define  HD21_TIMING_GEN_CR_get_tg_en(data)                             HDMI21_P0_HD21_TIMING_GEN_CR_get_tg_en(data)

#define  HD21_TIMING_GEN_SR_de_data_mismatch_mask                       HDMI21_P0_HD21_TIMING_GEN_SR_de_data_mismatch_mask
#define  HD21_TIMING_GEN_SR_dpc2tg_linebuff_mask                        HDMI21_P0_HD21_TIMING_GEN_SR_dpc2tg_linebuff_mask
#define  HD21_TIMING_GEN_SR_ms2tg_fifof_mask                            HDMI21_P0_HD21_TIMING_GEN_SR_ms2tg_fifof_mask
#define  HD21_TIMING_GEN_SR_video_timing_tog_err_mask                   HDMI21_P0_HD21_TIMING_GEN_SR_video_timing_tog_err_mask
#define  HD21_TIMING_GEN_SR_de_data_mismatch(data)                      HDMI21_P0_HD21_TIMING_GEN_SR_de_data_mismatch(data)
#define  HD21_TIMING_GEN_SR_dpc2tg_linebuff(data)                       HDMI21_P0_HD21_TIMING_GEN_SR_dpc2tg_linebuff(data)
#define  HD21_TIMING_GEN_SR_ms2tg_fifof(data)                           HDMI21_P0_HD21_TIMING_GEN_SR_ms2tg_fifof(data)
#define  HD21_TIMING_GEN_SR_video_timing_tog_err(data)                  HDMI21_P0_HD21_TIMING_GEN_SR_video_timing_tog_err(data)
#define  HD21_TIMING_GEN_SR_get_de_data_mismatch(data)                  HDMI21_P0_HD21_TIMING_GEN_SR_get_de_data_mismatch(data)
#define  HD21_TIMING_GEN_SR_get_dpc2tg_linebuff(data)                   HDMI21_P0_HD21_TIMING_GEN_SR_get_dpc2tg_linebuff(data)
#define  HD21_TIMING_GEN_SR_get_ms2tg_fifof(data)                       HDMI21_P0_HD21_TIMING_GEN_SR_get_ms2tg_fifof(data)
#define  HD21_TIMING_GEN_SR_get_video_timing_tog_err(data)              HDMI21_P0_HD21_TIMING_GEN_SR_get_video_timing_tog_err(data)


#define  HD21_CH_ST_dummy_1_0_mask                                      HDMI21_P0_HD21_CH_ST_dummy_1_0_mask
#define  HD21_CH_ST_dummy_1_0(data)                                     HDMI21_P0_HD21_CH_ST_dummy_1_0(data)
#define  HD21_CH_ST_get_dummy_1_0(data)                                 HDMI21_P0_HD21_CH_ST_get_dummy_1_0(data)


#define  HD21_PRBS_R_CTRL_prbs_rxbist_err_cnt_mask                      HDMI21_P0_HD21_PRBS_R_CTRL_prbs_rxbist_err_cnt_mask
#define  HD21_PRBS_R_CTRL_prbs_lock_mask                                HDMI21_P0_HD21_PRBS_R_CTRL_prbs_lock_mask
#define  HD21_PRBS_R_CTRL_prbs_err_cnt_clr_mask                         HDMI21_P0_HD21_PRBS_R_CTRL_prbs_err_cnt_clr_mask
#define  HD21_PRBS_R_CTRL_prbs_reverse_mask                             HDMI21_P0_HD21_PRBS_R_CTRL_prbs_reverse_mask
#define  HD21_PRBS_R_CTRL_prbs15_rxen_mask                              HDMI21_P0_HD21_PRBS_R_CTRL_prbs15_rxen_mask
#define  HD21_PRBS_R_CTRL_prbs11_rxen_mask                              HDMI21_P0_HD21_PRBS_R_CTRL_prbs11_rxen_mask
#define  HD21_PRBS_R_CTRL_prbs7_rxen_mask                               HDMI21_P0_HD21_PRBS_R_CTRL_prbs7_rxen_mask
#define  HD21_PRBS_R_CTRL_prbs_rxbist_err_cnt(data)                     HDMI21_P0_HD21_PRBS_R_CTRL_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_R_CTRL_prbs_lock(data)                               HDMI21_P0_HD21_PRBS_R_CTRL_prbs_lock(data)
#define  HD21_PRBS_R_CTRL_prbs_err_cnt_clr(data)                        HDMI21_P0_HD21_PRBS_R_CTRL_prbs_err_cnt_clr(data)
#define  HD21_PRBS_R_CTRL_prbs_reverse(data)                            HDMI21_P0_HD21_PRBS_R_CTRL_prbs_reverse(data)
#define  HD21_PRBS_R_CTRL_prbs15_rxen(data)                             HDMI21_P0_HD21_PRBS_R_CTRL_prbs15_rxen(data)
#define  HD21_PRBS_R_CTRL_prbs11_rxen(data)                             HDMI21_P0_HD21_PRBS_R_CTRL_prbs11_rxen(data)
#define  HD21_PRBS_R_CTRL_prbs7_rxen(data)                              HDMI21_P0_HD21_PRBS_R_CTRL_prbs7_rxen(data)
#define  HD21_PRBS_R_CTRL_get_prbs_rxbist_err_cnt(data)                 HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_R_CTRL_get_prbs_lock(data)                           HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs_lock(data)
#define  HD21_PRBS_R_CTRL_get_prbs_err_cnt_clr(data)                    HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs_err_cnt_clr(data)
#define  HD21_PRBS_R_CTRL_get_prbs_reverse(data)                        HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs_reverse(data)
#define  HD21_PRBS_R_CTRL_get_prbs15_rxen(data)                         HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs15_rxen(data)
#define  HD21_PRBS_R_CTRL_get_prbs11_rxen(data)                         HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs11_rxen(data)
#define  HD21_PRBS_R_CTRL_get_prbs7_rxen(data)                          HDMI21_P0_HD21_PRBS_R_CTRL_get_prbs7_rxen(data)

#define  HD21_PRBS_G_CTRL_prbs_rxbist_err_cnt_mask                      HDMI21_P0_HD21_PRBS_G_CTRL_prbs_rxbist_err_cnt_mask
#define  HD21_PRBS_G_CTRL_prbs_lock_mask                                HDMI21_P0_HD21_PRBS_G_CTRL_prbs_lock_mask
#define  HD21_PRBS_G_CTRL_prbs_err_cnt_clr_mask                         HDMI21_P0_HD21_PRBS_G_CTRL_prbs_err_cnt_clr_mask
#define  HD21_PRBS_G_CTRL_prbs_reverse_mask                             HDMI21_P0_HD21_PRBS_G_CTRL_prbs_reverse_mask
#define  HD21_PRBS_G_CTRL_prbs15_rxen_mask                              HDMI21_P0_HD21_PRBS_G_CTRL_prbs15_rxen_mask
#define  HD21_PRBS_G_CTRL_prbs11_rxen_mask                              HDMI21_P0_HD21_PRBS_G_CTRL_prbs11_rxen_mask
#define  HD21_PRBS_G_CTRL_prbs7_rxen_mask                               HDMI21_P0_HD21_PRBS_G_CTRL_prbs7_rxen_mask
#define  HD21_PRBS_G_CTRL_prbs_rxbist_err_cnt(data)                     HDMI21_P0_HD21_PRBS_G_CTRL_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_G_CTRL_prbs_lock(data)                               HDMI21_P0_HD21_PRBS_G_CTRL_prbs_lock(data)
#define  HD21_PRBS_G_CTRL_prbs_err_cnt_clr(data)                        HDMI21_P0_HD21_PRBS_G_CTRL_prbs_err_cnt_clr(data)
#define  HD21_PRBS_G_CTRL_prbs_reverse(data)                            HDMI21_P0_HD21_PRBS_G_CTRL_prbs_reverse(data)
#define  HD21_PRBS_G_CTRL_prbs15_rxen(data)                             HDMI21_P0_HD21_PRBS_G_CTRL_prbs15_rxen(data)
#define  HD21_PRBS_G_CTRL_prbs11_rxen(data)                             HDMI21_P0_HD21_PRBS_G_CTRL_prbs11_rxen(data)
#define  HD21_PRBS_G_CTRL_prbs7_rxen(data)                              HDMI21_P0_HD21_PRBS_G_CTRL_prbs7_rxen(data)
#define  HD21_PRBS_G_CTRL_get_prbs_rxbist_err_cnt(data)                 HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_G_CTRL_get_prbs_lock(data)                           HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs_lock(data)
#define  HD21_PRBS_G_CTRL_get_prbs_err_cnt_clr(data)                    HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs_err_cnt_clr(data)
#define  HD21_PRBS_G_CTRL_get_prbs_reverse(data)                        HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs_reverse(data)
#define  HD21_PRBS_G_CTRL_get_prbs15_rxen(data)                         HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs15_rxen(data)
#define  HD21_PRBS_G_CTRL_get_prbs11_rxen(data)                         HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs11_rxen(data)
#define  HD21_PRBS_G_CTRL_get_prbs7_rxen(data)                          HDMI21_P0_HD21_PRBS_G_CTRL_get_prbs7_rxen(data)


#define  HD21_PRBS_B_CTRL_prbs_rxbist_err_cnt_mask                      HDMI21_P0_HD21_PRBS_B_CTRL_prbs_rxbist_err_cnt_mask
#define  HD21_PRBS_B_CTRL_prbs_lock_mask                                HDMI21_P0_HD21_PRBS_B_CTRL_prbs_lock_mask
#define  HD21_PRBS_B_CTRL_prbs_err_cnt_clr_mask                         HDMI21_P0_HD21_PRBS_B_CTRL_prbs_err_cnt_clr_mask
#define  HD21_PRBS_B_CTRL_prbs_reverse_mask                             HDMI21_P0_HD21_PRBS_B_CTRL_prbs_reverse_mask
#define  HD21_PRBS_B_CTRL_prbs15_rxen_mask                              HDMI21_P0_HD21_PRBS_B_CTRL_prbs15_rxen_mask
#define  HD21_PRBS_B_CTRL_prbs11_rxen_mask                              HDMI21_P0_HD21_PRBS_B_CTRL_prbs11_rxen_mask
#define  HD21_PRBS_B_CTRL_prbs7_rxen_mask                               HDMI21_P0_HD21_PRBS_B_CTRL_prbs7_rxen_mask
#define  HD21_PRBS_B_CTRL_prbs_rxbist_err_cnt(data)                     HDMI21_P0_HD21_PRBS_B_CTRL_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_B_CTRL_prbs_lock(data)                               HDMI21_P0_HD21_PRBS_B_CTRL_prbs_lock(data)
#define  HD21_PRBS_B_CTRL_prbs_err_cnt_clr(data)                        HDMI21_P0_HD21_PRBS_B_CTRL_prbs_err_cnt_clr(data)
#define  HD21_PRBS_B_CTRL_prbs_reverse(data)                            HDMI21_P0_HD21_PRBS_B_CTRL_prbs_reverse(data)
#define  HD21_PRBS_B_CTRL_prbs15_rxen(data)                             HDMI21_P0_HD21_PRBS_B_CTRL_prbs15_rxen(data)
#define  HD21_PRBS_B_CTRL_prbs11_rxen(data)                             HDMI21_P0_HD21_PRBS_B_CTRL_prbs11_rxen(data)
#define  HD21_PRBS_B_CTRL_prbs7_rxen(data)                              HDMI21_P0_HD21_PRBS_B_CTRL_prbs7_rxen(data)
#define  HD21_PRBS_B_CTRL_get_prbs_rxbist_err_cnt(data)                 HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_B_CTRL_get_prbs_lock(data)                           HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs_lock(data)
#define  HD21_PRBS_B_CTRL_get_prbs_err_cnt_clr(data)                    HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs_err_cnt_clr(data)
#define  HD21_PRBS_B_CTRL_get_prbs_reverse(data)                        HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs_reverse(data)
#define  HD21_PRBS_B_CTRL_get_prbs15_rxen(data)                         HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs15_rxen(data)
#define  HD21_PRBS_B_CTRL_get_prbs11_rxen(data)                         HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs11_rxen(data)
#define  HD21_PRBS_B_CTRL_get_prbs7_rxen(data)                          HDMI21_P0_HD21_PRBS_B_CTRL_get_prbs7_rxen(data)


#define  HD21_PRBS_C_CTRL_prbs_rxbist_err_cnt_mask                      HDMI21_P0_HD21_PRBS_C_CTRL_prbs_rxbist_err_cnt_mask
#define  HD21_PRBS_C_CTRL_prbs_lock_mask                                HDMI21_P0_HD21_PRBS_C_CTRL_prbs_lock_mask
#define  HD21_PRBS_C_CTRL_prbs_err_cnt_clr_mask                         HDMI21_P0_HD21_PRBS_C_CTRL_prbs_err_cnt_clr_mask
#define  HD21_PRBS_C_CTRL_prbs_reverse_mask                             HDMI21_P0_HD21_PRBS_C_CTRL_prbs_reverse_mask
#define  HD21_PRBS_C_CTRL_prbs15_rxen_mask                              HDMI21_P0_HD21_PRBS_C_CTRL_prbs15_rxen_mask
#define  HD21_PRBS_C_CTRL_prbs11_rxen_mask                              HDMI21_P0_HD21_PRBS_C_CTRL_prbs11_rxen_mask
#define  HD21_PRBS_C_CTRL_prbs7_rxen_mask                               HDMI21_P0_HD21_PRBS_C_CTRL_prbs7_rxen_mask
#define  HD21_PRBS_C_CTRL_prbs_rxbist_err_cnt(data)                     HDMI21_P0_HD21_PRBS_C_CTRL_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_C_CTRL_prbs_lock(data)                               HDMI21_P0_HD21_PRBS_C_CTRL_prbs_lock(data)
#define  HD21_PRBS_C_CTRL_prbs_err_cnt_clr(data)                        HDMI21_P0_HD21_PRBS_C_CTRL_prbs_err_cnt_clr(data)
#define  HD21_PRBS_C_CTRL_prbs_reverse(data)                            HDMI21_P0_HD21_PRBS_C_CTRL_prbs_reverse(data)
#define  HD21_PRBS_C_CTRL_prbs15_rxen(data)                             HDMI21_P0_HD21_PRBS_C_CTRL_prbs15_rxen(data)
#define  HD21_PRBS_C_CTRL_prbs11_rxen(data)                             HDMI21_P0_HD21_PRBS_C_CTRL_prbs11_rxen(data)
#define  HD21_PRBS_C_CTRL_prbs7_rxen(data)                              HDMI21_P0_HD21_PRBS_C_CTRL_prbs7_rxen(data)
#define  HD21_PRBS_C_CTRL_get_prbs_rxbist_err_cnt(data)                 HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs_rxbist_err_cnt(data)
#define  HD21_PRBS_C_CTRL_get_prbs_lock(data)                           HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs_lock(data)
#define  HD21_PRBS_C_CTRL_get_prbs_err_cnt_clr(data)                    HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs_err_cnt_clr(data)
#define  HD21_PRBS_C_CTRL_get_prbs_reverse(data)                        HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs_reverse(data)
#define  HD21_PRBS_C_CTRL_get_prbs15_rxen(data)                         HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs15_rxen(data)
#define  HD21_PRBS_C_CTRL_get_prbs11_rxen(data)                         HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs11_rxen(data)
#define  HD21_PRBS_C_CTRL_get_prbs7_rxen(data)                          HDMI21_P0_HD21_PRBS_C_CTRL_get_prbs7_rxen(data)


#define  HD21_PRBS_R_STATUS_prbs_err_cnt_mask                           HDMI21_P0_HD21_PRBS_R_STATUS_prbs_err_cnt_mask
#define  HD21_PRBS_R_STATUS_prbs_bit_err_mask                           HDMI21_P0_HD21_PRBS_R_STATUS_prbs_bit_err_mask
#define  HD21_PRBS_R_STATUS_prbs_err_cnt(data)                          HDMI21_P0_HD21_PRBS_R_STATUS_prbs_err_cnt(data)
#define  HD21_PRBS_R_STATUS_prbs_bit_err(data)                          HDMI21_P0_HD21_PRBS_R_STATUS_prbs_bit_err(data)
#define  HD21_PRBS_R_STATUS_get_prbs_err_cnt(data)                      HDMI21_P0_HD21_PRBS_R_STATUS_get_prbs_err_cnt(data)
#define  HD21_PRBS_R_STATUS_get_prbs_bit_err(data)                      HDMI21_P0_HD21_PRBS_R_STATUS_get_prbs_bit_err(data)


#define  HD21_PRBS_G_STATUS_prbs_err_cnt_mask                           HDMI21_P0_HD21_PRBS_G_STATUS_prbs_err_cnt_mask
#define  HD21_PRBS_G_STATUS_prbs_bit_err_mask                           HDMI21_P0_HD21_PRBS_G_STATUS_prbs_bit_err_mask
#define  HD21_PRBS_G_STATUS_prbs_err_cnt(data)                          HDMI21_P0_HD21_PRBS_G_STATUS_prbs_err_cnt(data)
#define  HD21_PRBS_G_STATUS_prbs_bit_err(data)                          HDMI21_P0_HD21_PRBS_G_STATUS_prbs_bit_err(data)
#define  HD21_PRBS_G_STATUS_get_prbs_err_cnt(data)                      HDMI21_P0_HD21_PRBS_G_STATUS_get_prbs_err_cnt(data)
#define  HD21_PRBS_G_STATUS_get_prbs_bit_err(data)                      HDMI21_P0_HD21_PRBS_G_STATUS_get_prbs_bit_err(data)


#define  HD21_PRBS_B_STATUS_prbs_err_cnt_mask                           HDMI21_P0_HD21_PRBS_B_STATUS_prbs_err_cnt_mask
#define  HD21_PRBS_B_STATUS_prbs_bit_err_mask                           HDMI21_P0_HD21_PRBS_B_STATUS_prbs_bit_err_mask
#define  HD21_PRBS_B_STATUS_prbs_err_cnt(data)                          HDMI21_P0_HD21_PRBS_B_STATUS_prbs_err_cnt(data)
#define  HD21_PRBS_B_STATUS_prbs_bit_err(data)                          HDMI21_P0_HD21_PRBS_B_STATUS_prbs_bit_err(data)
#define  HD21_PRBS_B_STATUS_get_prbs_err_cnt(data)                      HDMI21_P0_HD21_PRBS_B_STATUS_get_prbs_err_cnt(data)
#define  HD21_PRBS_B_STATUS_get_prbs_bit_err(data)                      HDMI21_P0_HD21_PRBS_B_STATUS_get_prbs_bit_err(data)


#define  HD21_PRBS_C_STATUS_prbs_err_cnt_mask                           HDMI21_P0_HD21_PRBS_C_STATUS_prbs_err_cnt_mask
#define  HD21_PRBS_C_STATUS_prbs_bit_err_mask                           HDMI21_P0_HD21_PRBS_C_STATUS_prbs_bit_err_mask
#define  HD21_PRBS_C_STATUS_prbs_err_cnt(data)                          HDMI21_P0_HD21_PRBS_C_STATUS_prbs_err_cnt(data)
#define  HD21_PRBS_C_STATUS_prbs_bit_err(data)                          HDMI21_P0_HD21_PRBS_C_STATUS_prbs_bit_err(data)
#define  HD21_PRBS_C_STATUS_get_prbs_err_cnt(data)                      HDMI21_P0_HD21_PRBS_C_STATUS_get_prbs_err_cnt(data)
#define  HD21_PRBS_C_STATUS_get_prbs_bit_err(data)                      HDMI21_P0_HD21_PRBS_C_STATUS_get_prbs_bit_err(data)


#define  HD21_DSCD_DBG_en_mask                                          HDMI21_P0_hd21_DSCD_DBG_en_mask
#define  HD21_DSCD_DBG_rd_ch_mask                                       HDMI21_P0_hd21_DSCD_DBG_rd_ch_mask
#define  HD21_DSCD_DBG_wr_last_addr_mask                                HDMI21_P0_hd21_DSCD_DBG_wr_last_addr_mask
#define  HD21_DSCD_DBG_one_lane_sel_mask                                HDMI21_P0_hd21_DSCD_DBG_one_lane_sel_mask
#define  HD21_DSCD_DBG_four_lane_sel_mask                               HDMI21_P0_hd21_DSCD_DBG_four_lane_sel_mask
#define  HD21_DSCD_DBG_lane_dbg_mode_mask                               HDMI21_P0_hd21_DSCD_DBG_lane_dbg_mode_mask
#define  HD21_DSCD_DBG_wr_en_mask                                       HDMI21_P0_hd21_DSCD_DBG_wr_en_mask
#define  HD21_DSCD_DBG_en(data)                                         HDMI21_P0_hd21_DSCD_DBG_en(data)
#define  HD21_DSCD_DBG_rd_ch(data)                                      HDMI21_P0_hd21_DSCD_DBG_rd_ch(data)
#define  HD21_DSCD_DBG_wr_last_addr(data)                               HDMI21_P0_hd21_DSCD_DBG_wr_last_addr(data)
#define  HD21_DSCD_DBG_one_lane_sel(data)                               HDMI21_P0_hd21_DSCD_DBG_one_lane_sel(data)
#define  HD21_DSCD_DBG_four_lane_sel(data)                              HDMI21_P0_hd21_DSCD_DBG_four_lane_sel(data)
#define  HD21_DSCD_DBG_lane_dbg_mode(data)                              HDMI21_P0_hd21_DSCD_DBG_lane_dbg_mode(data)
#define  HD21_DSCD_DBG_wr_en(data)                                      HDMI21_P0_hd21_DSCD_DBG_wr_en(data)
#define  HD21_DSCD_DBG_get_en(data)                                     HDMI21_P0_hd21_DSCD_DBG_get_en(data)
#define  HD21_DSCD_DBG_get_rd_ch(data)                                  HDMI21_P0_hd21_DSCD_DBG_get_rd_ch(data)
#define  HD21_DSCD_DBG_get_wr_last_addr(data)                           HDMI21_P0_hd21_DSCD_DBG_get_wr_last_addr(data)
#define  HD21_DSCD_DBG_get_one_lane_sel(data)                           HDMI21_P0_hd21_DSCD_DBG_get_one_lane_sel(data)
#define  HD21_DSCD_DBG_get_four_lane_sel(data)                          HDMI21_P0_hd21_DSCD_DBG_get_four_lane_sel(data)
#define  HD21_DSCD_DBG_get_lane_dbg_mode(data)                          HDMI21_P0_hd21_DSCD_DBG_get_lane_dbg_mode(data)
#define  HD21_DSCD_DBG_get_wr_en(data)                                  HDMI21_P0_hd21_DSCD_DBG_get_wr_en(data)


#define  HD21_DSCD_DBG_AP_apss_mask                                     HDMI21_P0_HD21_DSCD_DBG_AP_apss_mask
#define  HD21_DSCD_DBG_AP_apss(data)                                    HDMI21_P0_HD21_DSCD_DBG_AP_apss(data)
#define  HD21_DSCD_DBG_AP_get_apss(data)                                HDMI21_P0_HD21_DSCD_DBG_AP_get_apss(data)


#define  HD21_DSCD_DBG_DP_dpss_mask                                     HDMI21_P0_hd21_DSCD_DBG_DP_dpss_mask
#define  HD21_DSCD_DBG_DP_dpss(data)                                    HDMI21_P0_hd21_DSCD_DBG_DP_dpss(data)
#define  HD21_DSCD_DBG_DP_get_dpss(data)                                HDMI21_P0_hd21_DSCD_DBG_DP_get_dpss(data)


#define  HD21_LT_R_ln_req_mask                                          HDMI21_P0_HD21_LT_R_ln_req_mask
#define  HD21_LT_R_lock_sel_mask                                        HDMI21_P0_HD21_LT_R_lock_sel_mask
#define  HD21_LT_R_popup_mask                                           HDMI21_P0_HD21_LT_R_popup_mask
#define  HD21_LT_R_reset_mask                                           HDMI21_P0_HD21_LT_R_reset_mask
#define  HD21_LT_R_timeout_flag_mask                                    HDMI21_P0_HD21_LT_R_timeout_flag_mask
#define  HD21_LT_R_err_cnt_mask                                         HDMI21_P0_HD21_LT_R_err_cnt_mask
#define  HD21_LT_R_lock_valid_mask                                      HDMI21_P0_HD21_LT_R_lock_valid_mask
#define  HD21_LT_R_ln_req(data)                                         HDMI21_P0_HD21_LT_R_ln_req(data)
#define  HD21_LT_R_lock_sel(data)                                       HDMI21_P0_HD21_LT_R_lock_sel(data)
#define  HD21_LT_R_popup(data)                                          HDMI21_P0_HD21_LT_R_popup(data)
#define  HD21_LT_R_reset(data)                                          HDMI21_P0_HD21_LT_R_reset(data)
#define  HD21_LT_R_timeout_flag(data)                                   HDMI21_P0_HD21_LT_R_timeout_flag(data)
#define  HD21_LT_R_err_cnt(data)                                        HDMI21_P0_HD21_LT_R_err_cnt(data)
#define  HD21_LT_R_lock_valid(data)                                     HDMI21_P0_HD21_LT_R_lock_valid(data)
#define  HD21_LT_R_get_ln_req(data)                                     HDMI21_P0_HD21_LT_R_get_ln_req(data)
#define  HD21_LT_R_get_lock_sel(data)                                   HDMI21_P0_HD21_LT_R_get_lock_sel(data)
#define  HD21_LT_R_get_popup(data)                                      HDMI21_P0_HD21_LT_R_get_popup(data)
#define  HD21_LT_R_get_reset(data)                                      HDMI21_P0_HD21_LT_R_get_reset(data)
#define  HD21_LT_R_get_timeout_flag(data)                               HDMI21_P0_HD21_LT_R_get_timeout_flag(data)
#define  HD21_LT_R_get_err_cnt(data)                                    HDMI21_P0_HD21_LT_R_get_err_cnt(data)
#define  HD21_LT_R_get_lock_valid(data)                                 HDMI21_P0_HD21_LT_R_get_lock_valid(data)


#define  HD21_LT_G_ln_req_mask                                          HDMI21_P0_HD21_LT_G_ln_req_mask
#define  HD21_LT_G_lock_sel_mask                                        HDMI21_P0_HD21_LT_G_lock_sel_mask
#define  HD21_LT_G_popup_mask                                           HDMI21_P0_HD21_LT_G_popup_mask
#define  HD21_LT_G_reset_mask                                           HDMI21_P0_HD21_LT_G_reset_mask
#define  HD21_LT_G_timeout_flag_mask                                    HDMI21_P0_HD21_LT_G_timeout_flag_mask
#define  HD21_LT_G_err_cnt_mask                                         HDMI21_P0_HD21_LT_G_err_cnt_mask
#define  HD21_LT_G_lock_valid_mask                                      HDMI21_P0_HD21_LT_G_lock_valid_mask
#define  HD21_LT_G_ln_req(data)                                         HDMI21_P0_HD21_LT_G_ln_req(data)
#define  HD21_LT_G_lock_sel(data)                                       HDMI21_P0_HD21_LT_G_lock_sel(data)
#define  HD21_LT_G_popup(data)                                          HDMI21_P0_HD21_LT_G_popup(data)
#define  HD21_LT_G_reset(data)                                          HDMI21_P0_HD21_LT_G_reset(data)
#define  HD21_LT_G_timeout_flag(data)                                   HDMI21_P0_HD21_LT_G_timeout_flag(data)
#define  HD21_LT_G_err_cnt(data)                                        HDMI21_P0_HD21_LT_G_err_cnt(data)
#define  HD21_LT_G_lock_valid(data)                                     HDMI21_P0_HD21_LT_G_lock_valid(data)
#define  HD21_LT_G_get_ln_req(data)                                     HDMI21_P0_HD21_LT_G_get_ln_req(data)
#define  HD21_LT_G_get_lock_sel(data)                                   HDMI21_P0_HD21_LT_G_get_lock_sel(data)
#define  HD21_LT_G_get_popup(data)                                      HDMI21_P0_HD21_LT_G_get_popup(data)
#define  HD21_LT_G_get_reset(data)                                      HDMI21_P0_HD21_LT_G_get_reset(data)
#define  HD21_LT_G_get_timeout_flag(data)                               HDMI21_P0_HD21_LT_G_get_timeout_flag(data)
#define  HD21_LT_G_get_err_cnt(data)                                    HDMI21_P0_HD21_LT_G_get_err_cnt(data)
#define  HD21_LT_G_get_lock_valid(data)                                 HDMI21_P0_HD21_LT_G_get_lock_valid(data)


#define  HD21_LT_B_ln_req_mask                                          HDMI21_P0_HD21_LT_B_ln_req_mask
#define  HD21_LT_B_lock_sel_mask                                        HDMI21_P0_HD21_LT_B_lock_sel_mask
#define  HD21_LT_B_popup_mask                                           HDMI21_P0_HD21_LT_B_popup_mask
#define  HD21_LT_B_reset_mask                                           HDMI21_P0_HD21_LT_B_reset_mask
#define  HD21_LT_B_timeout_flag_mask                                    HDMI21_P0_HD21_LT_B_timeout_flag_mask
#define  HD21_LT_B_err_cnt_mask                                         HDMI21_P0_HD21_LT_B_err_cnt_mask
#define  HD21_LT_B_lock_valid_mask                                      HDMI21_P0_HD21_LT_B_lock_valid_mask
#define  HD21_LT_B_ln_req(data)                                         HDMI21_P0_HD21_LT_B_ln_req(data)
#define  HD21_LT_B_lock_sel(data)                                       HDMI21_P0_HD21_LT_B_lock_sel(data)
#define  HD21_LT_B_popup(data)                                          HDMI21_P0_HD21_LT_B_popup(data)
#define  HD21_LT_B_reset(data)                                          HDMI21_P0_HD21_LT_B_reset(data)
#define  HD21_LT_B_timeout_flag(data)                                   HDMI21_P0_HD21_LT_B_timeout_flag(data)
#define  HD21_LT_B_err_cnt(data)                                        HDMI21_P0_HD21_LT_B_err_cnt(data)
#define  HD21_LT_B_lock_valid(data)                                     HDMI21_P0_HD21_LT_B_lock_valid(data)
#define  HD21_LT_B_get_ln_req(data)                                     HDMI21_P0_HD21_LT_B_get_ln_req(data)
#define  HD21_LT_B_get_lock_sel(data)                                   HDMI21_P0_HD21_LT_B_get_lock_sel(data)
#define  HD21_LT_B_get_popup(data)                                      HDMI21_P0_HD21_LT_B_get_popup(data)
#define  HD21_LT_B_get_reset(data)                                      HDMI21_P0_HD21_LT_B_get_reset(data)
#define  HD21_LT_B_get_timeout_flag(data)                               HDMI21_P0_HD21_LT_B_get_timeout_flag(data)
#define  HD21_LT_B_get_err_cnt(data)                                    HDMI21_P0_HD21_LT_B_get_err_cnt(data)
#define  HD21_LT_B_get_lock_valid(data)                                 HDMI21_P0_HD21_LT_B_get_lock_valid(data)


#define  HD21_LT_C_ln_req_mask                                          HDMI21_P0_HD21_LT_C_ln_req_mask
#define  HD21_LT_C_lock_sel_mask                                        HDMI21_P0_HD21_LT_C_lock_sel_mask
#define  HD21_LT_C_popup_mask                                           HDMI21_P0_HD21_LT_C_popup_mask
#define  HD21_LT_C_reset_mask                                           HDMI21_P0_HD21_LT_C_reset_mask
#define  HD21_LT_C_timeout_flag_mask                                    HDMI21_P0_HD21_LT_C_timeout_flag_mask
#define  HD21_LT_C_err_cnt_mask                                         HDMI21_P0_HD21_LT_C_err_cnt_mask
#define  HD21_LT_C_lock_valid_mask                                      HDMI21_P0_HD21_LT_C_lock_valid_mask
#define  HD21_LT_C_ln_req(data)                                         HDMI21_P0_HD21_LT_C_ln_req(data)
#define  HD21_LT_C_lock_sel(data)                                       HDMI21_P0_HD21_LT_C_lock_sel(data)
#define  HD21_LT_C_popup(data)                                          HDMI21_P0_HD21_LT_C_popup(data)
#define  HD21_LT_C_reset(data)                                          HDMI21_P0_HD21_LT_C_reset(data)
#define  HD21_LT_C_timeout_flag(data)                                   HDMI21_P0_HD21_LT_C_timeout_flag(data)
#define  HD21_LT_C_err_cnt(data)                                        HDMI21_P0_HD21_LT_C_err_cnt(data)
#define  HD21_LT_C_lock_valid(data)                                     HDMI21_P0_HD21_LT_C_lock_valid(data)
#define  HD21_LT_C_get_ln_req(data)                                     HDMI21_P0_HD21_LT_C_get_ln_req(data)
#define  HD21_LT_C_get_lock_sel(data)                                   HDMI21_P0_HD21_LT_C_get_lock_sel(data)
#define  HD21_LT_C_get_popup(data)                                      HDMI21_P0_HD21_LT_C_get_popup(data)
#define  HD21_LT_C_get_reset(data)                                      HDMI21_P0_HD21_LT_C_get_reset(data)
#define  HD21_LT_C_get_timeout_flag(data)                               HDMI21_P0_HD21_LT_C_get_timeout_flag(data)
#define  HD21_LT_C_get_err_cnt(data)                                    HDMI21_P0_HD21_LT_C_get_err_cnt(data)
#define  HD21_LT_C_get_lock_valid(data)                                 HDMI21_P0_HD21_LT_C_get_lock_valid(data)


#define  HD21_LT_ALL_ltp_en_mask                                        HDMI21_P0_HD21_LT_ALL_ltp_en_mask
#define  HD21_LT_ALL_ltp3_sel_mask                                      HDMI21_P0_HD21_LT_ALL_ltp3_sel_mask
#define  HD21_LT_ALL_timeout_set_mask                                   HDMI21_P0_HD21_LT_ALL_timeout_set_mask
#define  HD21_LT_ALL_ltp_en(data)                                       HDMI21_P0_HD21_LT_ALL_ltp_en(data)
#define  HD21_LT_ALL_ltp3_sel(data)                                     HDMI21_P0_HD21_LT_ALL_ltp3_sel(data)
#define  HD21_LT_ALL_timeout_set(data)                                  HDMI21_P0_HD21_LT_ALL_timeout_set(data)
#define  HD21_LT_ALL_get_ltp_en(data)                                   HDMI21_P0_HD21_LT_ALL_get_ltp_en(data)
#define  HD21_LT_ALL_get_ltp3_sel(data)                                 HDMI21_P0_HD21_LT_ALL_get_ltp3_sel(data)
#define  HD21_LT_ALL_get_timeout_set(data)                              HDMI21_P0_HD21_LT_ALL_get_timeout_set(data)


#define  HD21_UDC_debug_sel_mask                                        HDMI21_P0_HD21_UDC_debug_sel_mask
#define  HD21_UDC_dummy_3_mask                                          HDMI21_P0_HD21_UDC_dummy_3_mask
#define  HD21_UDC_debug_sel(data)                                       HDMI21_P0_HD21_UDC_debug_sel(data)
#define  HD21_UDC_dummy_3(data)                                         HDMI21_P0_HD21_UDC_dummy_3(data)
#define  HD21_UDC_get_debug_sel(data)                                   HDMI21_P0_HD21_UDC_get_debug_sel(data)
#define  HD21_UDC_get_dummy_3(data)                                     HDMI21_P0_HD21_UDC_get_dummy_3(data)


#define  HD21_TMDS_DPC0_dpc_pp_valid_mask                               HDMI21_P0_HD21_TMDS_DPC0_dpc_pp_valid_mask
#define  HD21_TMDS_DPC0_dpc_default_ph_mask                             HDMI21_P0_HD21_TMDS_DPC0_dpc_default_ph_mask
#define  HD21_TMDS_DPC0_dpc_pp_mask                                     HDMI21_P0_HD21_TMDS_DPC0_dpc_pp_mask
#define  HD21_TMDS_DPC0_dpc_cd_mask                                     HDMI21_P0_HD21_TMDS_DPC0_dpc_cd_mask
#define  HD21_TMDS_DPC0_dpc_pp_valid(data)                              HDMI21_P0_HD21_TMDS_DPC0_dpc_pp_valid(data)
#define  HD21_TMDS_DPC0_dpc_default_ph(data)                            HDMI21_P0_HD21_TMDS_DPC0_dpc_default_ph(data)
#define  HD21_TMDS_DPC0_dpc_pp(data)                                    HDMI21_P0_HD21_TMDS_DPC0_dpc_pp(data)
#define  HD21_TMDS_DPC0_dpc_cd(data)                                    HDMI21_P0_HD21_TMDS_DPC0_dpc_cd(data)
#define  HD21_TMDS_DPC0_get_dpc_pp_valid(data)                          HDMI21_P0_HD21_TMDS_DPC0_get_dpc_pp_valid(data)
#define  HD21_TMDS_DPC0_get_dpc_default_ph(data)                        HDMI21_P0_HD21_TMDS_DPC0_get_dpc_default_ph(data)
#define  HD21_TMDS_DPC0_get_dpc_pp(data)                                HDMI21_P0_HD21_TMDS_DPC0_get_dpc_pp(data)
#define  HD21_TMDS_DPC0_get_dpc_cd(data)                                HDMI21_P0_HD21_TMDS_DPC0_get_dpc_cd(data)


#define  HD21_TMDS_DPC1_dpc_cd_chg_flag_mask                            HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_chg_flag_mask
#define  HD21_TMDS_DPC1_dpc_cd_chg_int_en_mask                          HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_chg_int_en_mask
#define  HD21_TMDS_DPC1_dummy_15_11_mask                                HDMI21_P0_HD21_TMDS_DPC1_dummy_15_11_mask
#define  HD21_TMDS_DPC1_dpc_auto_mask                                   HDMI21_P0_HD21_TMDS_DPC1_dpc_auto_mask
#define  HD21_TMDS_DPC1_dpc_pp_valid_fw_mask                            HDMI21_P0_HD21_TMDS_DPC1_dpc_pp_valid_fw_mask
#define  HD21_TMDS_DPC1_dpc_default_ph_fw_mask                          HDMI21_P0_HD21_TMDS_DPC1_dpc_default_ph_fw_mask
#define  HD21_TMDS_DPC1_dpc_pp_fw_mask                                  HDMI21_P0_HD21_TMDS_DPC1_dpc_pp_fw_mask
#define  HD21_TMDS_DPC1_dpc_cd_fw_mask                                  HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_fw_mask
#define  HD21_TMDS_DPC1_dpc_cd_chg_flag(data)                           HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_chg_flag(data)
#define  HD21_TMDS_DPC1_dpc_cd_chg_int_en(data)                         HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_chg_int_en(data)
#define  HD21_TMDS_DPC1_dummy_15_11(data)                               HDMI21_P0_HD21_TMDS_DPC1_dummy_15_11(data)
#define  HD21_TMDS_DPC1_dpc_auto(data)                                  HDMI21_P0_HD21_TMDS_DPC1_dpc_auto(data)
#define  HD21_TMDS_DPC1_dpc_pp_valid_fw(data)                           HDMI21_P0_HD21_TMDS_DPC1_dpc_pp_valid_fw(data)
#define  HD21_TMDS_DPC1_dpc_default_ph_fw(data)                         HDMI21_P0_HD21_TMDS_DPC1_dpc_default_ph_fw(data)
#define  HD21_TMDS_DPC1_dpc_pp_fw(data)                                 HDMI21_P0_HD21_TMDS_DPC1_dpc_pp_fw(data)
#define  HD21_TMDS_DPC1_dpc_cd_fw(data)                                 HDMI21_P0_HD21_TMDS_DPC1_dpc_cd_fw(data)
#define  HD21_TMDS_DPC1_get_dpc_cd_chg_flag(data)                       HDMI21_P0_HD21_TMDS_DPC1_get_dpc_cd_chg_flag(data)
#define  HD21_TMDS_DPC1_get_dpc_cd_chg_int_en(data)                     HDMI21_P0_HD21_TMDS_DPC1_get_dpc_cd_chg_int_en(data)
#define  HD21_TMDS_DPC1_get_dummy_15_11(data)                           HDMI21_P0_HD21_TMDS_DPC1_get_dummy_15_11(data)
#define  HD21_TMDS_DPC1_get_dpc_auto(data)                              HDMI21_P0_HD21_TMDS_DPC1_get_dpc_auto(data)
#define  HD21_TMDS_DPC1_get_dpc_pp_valid_fw(data)                       HDMI21_P0_HD21_TMDS_DPC1_get_dpc_pp_valid_fw(data)
#define  HD21_TMDS_DPC1_get_dpc_default_ph_fw(data)                     HDMI21_P0_HD21_TMDS_DPC1_get_dpc_default_ph_fw(data)
#define  HD21_TMDS_DPC1_get_dpc_pp_fw(data)                             HDMI21_P0_HD21_TMDS_DPC1_get_dpc_pp_fw(data)
#define  HD21_TMDS_DPC1_get_dpc_cd_fw(data)                             HDMI21_P0_HD21_TMDS_DPC1_get_dpc_cd_fw(data)


#define  HD21_TMDS_DPC_SET0_dpc_en_mask                                 HDMI21_P0_HD21_TMDS_DPC_SET0_dpc_en_mask
#define  HD21_TMDS_DPC_SET0_pp_phase_measure_err_mask                   HDMI21_P0_HD21_TMDS_DPC_SET0_pp_phase_measure_err_mask
#define  HD21_TMDS_DPC_SET0_dpc_en(data)                                HDMI21_P0_HD21_TMDS_DPC_SET0_dpc_en(data)
#define  HD21_TMDS_DPC_SET0_pp_phase_measure_err(data)                  HDMI21_P0_HD21_TMDS_DPC_SET0_pp_phase_measure_err(data)
#define  HD21_TMDS_DPC_SET0_get_dpc_en(data)                            HDMI21_P0_HD21_TMDS_DPC_SET0_get_dpc_en(data)
#define  HD21_TMDS_DPC_SET0_get_pp_phase_measure_err(data)              HDMI21_P0_HD21_TMDS_DPC_SET0_get_pp_phase_measure_err(data)


#define  HD21_HDMI_SR_avmute_fw_mask                                    HDMI21_P0_HD21_HDMI_SR_avmute_fw_mask
#define  HD21_HDMI_SR_avmute_bg_mask                                    HDMI21_P0_HD21_HDMI_SR_avmute_bg_mask
#define  HD21_HDMI_SR_avmute_mask                                       HDMI21_P0_HD21_HDMI_SR_avmute_mask
#define  HD21_HDMI_SR_avmute_fw(data)                                   HDMI21_P0_HD21_HDMI_SR_avmute_fw(data)
#define  HD21_HDMI_SR_avmute_bg(data)                                   HDMI21_P0_HD21_HDMI_SR_avmute_bg(data)
#define  HD21_HDMI_SR_avmute(data)                                      HDMI21_P0_HD21_HDMI_SR_avmute(data)
#define  HD21_HDMI_SR_get_avmute_fw(data)                               HDMI21_P0_HD21_HDMI_SR_get_avmute_fw(data)
#define  HD21_HDMI_SR_get_avmute_bg(data)                               HDMI21_P0_HD21_HDMI_SR_get_avmute_bg(data)
#define  HD21_HDMI_SR_get_avmute(data)                                  HDMI21_P0_HD21_HDMI_SR_get_avmute(data)


#define  HD21_HDMI_GPVS_dummy_31_mask                                   HDMI21_P0_HD21_HDMI_GPVS_dummy_31_mask
#define  HD21_HDMI_GPVS_audps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_audps_mask
#define  HD21_HDMI_GPVS_hbr_audps_mask                                  HDMI21_P0_HD21_HDMI_GPVS_hbr_audps_mask
#define  HD21_HDMI_GPVS_drmps_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_drmps_v_mask
#define  HD21_HDMI_GPVS_mpegps_v_mask                                   HDMI21_P0_HD21_HDMI_GPVS_mpegps_v_mask
#define  HD21_HDMI_GPVS_aps_v_mask                                      HDMI21_P0_HD21_HDMI_GPVS_aps_v_mask
#define  HD21_HDMI_GPVS_spdps_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_spdps_v_mask
#define  HD21_HDMI_GPVS_avips_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_avips_v_mask
#define  HD21_HDMI_GPVS_hdr10pvsps_v_mask                               HDMI21_P0_HD21_HDMI_GPVS_hdr10pvsps_v_mask
#define  HD21_HDMI_GPVS_dvsps_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_dvsps_v_mask
#define  HD21_HDMI_GPVS_fvsps_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_fvsps_v_mask
#define  HD21_HDMI_GPVS_vsps_v_mask                                     HDMI21_P0_HD21_HDMI_GPVS_vsps_v_mask
#define  HD21_HDMI_GPVS_gmps_v_mask                                     HDMI21_P0_HD21_HDMI_GPVS_gmps_v_mask
#define  HD21_HDMI_GPVS_isrc1ps_v_mask                                  HDMI21_P0_HD21_HDMI_GPVS_isrc1ps_v_mask
#define  HD21_HDMI_GPVS_acpps_v_mask                                    HDMI21_P0_HD21_HDMI_GPVS_acpps_v_mask
#define  HD21_HDMI_GPVS_drmps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_drmps_mask
#define  HD21_HDMI_GPVS_mpegps_mask                                     HDMI21_P0_HD21_HDMI_GPVS_mpegps_mask
#define  HD21_HDMI_GPVS_aps_mask                                        HDMI21_P0_HD21_HDMI_GPVS_aps_mask
#define  HD21_HDMI_GPVS_spdps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_spdps_mask
#define  HD21_HDMI_GPVS_avips_mask                                      HDMI21_P0_HD21_HDMI_GPVS_avips_mask
#define  HD21_HDMI_GPVS_hdr10pvsps_mask                                 HDMI21_P0_HD21_HDMI_GPVS_hdr10pvsps_mask
#define  HD21_HDMI_GPVS_dvsps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_dvsps_mask
#define  HD21_HDMI_GPVS_fvsps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_fvsps_mask
#define  HD21_HDMI_GPVS_vsps_mask                                       HDMI21_P0_HD21_HDMI_GPVS_vsps_mask
#define  HD21_HDMI_GPVS_gmps_mask                                       HDMI21_P0_HD21_HDMI_GPVS_gmps_mask
#define  HD21_HDMI_GPVS_isrc1ps_mask                                    HDMI21_P0_HD21_HDMI_GPVS_isrc1ps_mask
#define  HD21_HDMI_GPVS_acpps_mask                                      HDMI21_P0_HD21_HDMI_GPVS_acpps_mask
#define  HD21_HDMI_GPVS_nps_mask                                        HDMI21_P0_HD21_HDMI_GPVS_nps_mask
#define  HD21_HDMI_GPVS_rsv3ps_mask                                     HDMI21_P0_HD21_HDMI_GPVS_rsv3ps_mask
#define  HD21_HDMI_GPVS_rsv2ps_mask                                     HDMI21_P0_HD21_HDMI_GPVS_rsv2ps_mask
#define  HD21_HDMI_GPVS_rsv1ps_mask                                     HDMI21_P0_HD21_HDMI_GPVS_rsv1ps_mask
#define  HD21_HDMI_GPVS_rsv0ps_mask                                     HDMI21_P0_HD21_HDMI_GPVS_rsv0ps_mask
#define  HD21_HDMI_GPVS_dummy_31(data)                                  HDMI21_P0_HD21_HDMI_GPVS_dummy_31(data)
#define  HD21_HDMI_GPVS_audps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_audps(data)
#define  HD21_HDMI_GPVS_hbr_audps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_hbr_audps(data)
#define  HD21_HDMI_GPVS_drmps_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_drmps_v(data)
#define  HD21_HDMI_GPVS_mpegps_v(data)                                  HDMI21_P0_HD21_HDMI_GPVS_mpegps_v(data)
#define  HD21_HDMI_GPVS_aps_v(data)                                     HDMI21_P0_HD21_HDMI_GPVS_aps_v(data)
#define  HD21_HDMI_GPVS_spdps_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_spdps_v(data)
#define  HD21_HDMI_GPVS_avips_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_avips_v(data)
#define  HD21_HDMI_GPVS_hdr10pvsps_v(data)                              HDMI21_P0_HD21_HDMI_GPVS_hdr10pvsps_v(data)
#define  HD21_HDMI_GPVS_dvsps_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_dvsps_v(data)
#define  HD21_HDMI_GPVS_fvsps_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_fvsps_v(data)
#define  HD21_HDMI_GPVS_vsps_v(data)                                    HDMI21_P0_HD21_HDMI_GPVS_vsps_v(data)
#define  HD21_HDMI_GPVS_gmps_v(data)                                    HDMI21_P0_HD21_HDMI_GPVS_gmps_v(data)
#define  HD21_HDMI_GPVS_isrc1ps_v(data)                                 HDMI21_P0_HD21_HDMI_GPVS_isrc1ps_v(data)
#define  HD21_HDMI_GPVS_acpps_v(data)                                   HDMI21_P0_HD21_HDMI_GPVS_acpps_v(data)
#define  HD21_HDMI_GPVS_drmps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_drmps(data)
#define  HD21_HDMI_GPVS_mpegps(data)                                    HDMI21_P0_HD21_HDMI_GPVS_mpegps(data)
#define  HD21_HDMI_GPVS_aps(data)                                       HDMI21_P0_HD21_HDMI_GPVS_aps(data)
#define  HD21_HDMI_GPVS_spdps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_spdps(data)
#define  HD21_HDMI_GPVS_avips(data)                                     HDMI21_P0_HD21_HDMI_GPVS_avips(data)
#define  HD21_HDMI_GPVS_hdr10pvsps(data)                                HDMI21_P0_HD21_HDMI_GPVS_hdr10pvsps(data)
#define  HD21_HDMI_GPVS_dvsps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_dvsps(data)
#define  HD21_HDMI_GPVS_fvsps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_fvsps(data)
#define  HD21_HDMI_GPVS_vsps(data)                                      HDMI21_P0_HD21_HDMI_GPVS_vsps(data)
#define  HD21_HDMI_GPVS_gmps(data)                                      HDMI21_P0_HD21_HDMI_GPVS_gmps(data)
#define  HD21_HDMI_GPVS_isrc1ps(data)                                   HDMI21_P0_HD21_HDMI_GPVS_isrc1ps(data)
#define  HD21_HDMI_GPVS_acpps(data)                                     HDMI21_P0_HD21_HDMI_GPVS_acpps(data)
#define  HD21_HDMI_GPVS_nps(data)                                       HDMI21_P0_HD21_HDMI_GPVS_nps(data)
#define  HD21_HDMI_GPVS_rsv3ps(data)                                    HDMI21_P0_HD21_HDMI_GPVS_rsv3ps(data)
#define  HD21_HDMI_GPVS_rsv2ps(data)                                    HDMI21_P0_HD21_HDMI_GPVS_rsv2ps(data)
#define  HD21_HDMI_GPVS_rsv1ps(data)                                    HDMI21_P0_HD21_HDMI_GPVS_rsv1ps(data)
#define  HD21_HDMI_GPVS_rsv0ps(data)                                    HDMI21_P0_HD21_HDMI_GPVS_rsv0ps(data)
#define  HD21_HDMI_GPVS_get_dummy_31(data)                              HDMI21_P0_HD21_HDMI_GPVS_get_dummy_31(data)
#define  HD21_HDMI_GPVS_get_audps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_audps(data)
#define  HD21_HDMI_GPVS_get_hbr_audps(data)                             HDMI21_P0_HD21_HDMI_GPVS_get_hbr_audps(data)
#define  HD21_HDMI_GPVS_get_drmps_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_drmps_v(data)
#define  HD21_HDMI_GPVS_get_mpegps_v(data)                              HDMI21_P0_HD21_HDMI_GPVS_get_mpegps_v(data)
#define  HD21_HDMI_GPVS_get_aps_v(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_aps_v(data)
#define  HD21_HDMI_GPVS_get_spdps_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_spdps_v(data)
#define  HD21_HDMI_GPVS_get_avips_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_avips_v(data)
#define  HD21_HDMI_GPVS_get_hdr10pvsps_v(data)                          HDMI21_P0_HD21_HDMI_GPVS_get_hdr10pvsps_v(data)
#define  HD21_HDMI_GPVS_get_dvsps_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_dvsps_v(data)
#define  HD21_HDMI_GPVS_get_fvsps_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_fvsps_v(data)
#define  HD21_HDMI_GPVS_get_vsps_v(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_vsps_v(data)
#define  HD21_HDMI_GPVS_get_gmps_v(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_gmps_v(data)
#define  HD21_HDMI_GPVS_get_isrc1ps_v(data)                             HDMI21_P0_HD21_HDMI_GPVS_get_isrc1ps_v(data)
#define  HD21_HDMI_GPVS_get_acpps_v(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_acpps_v(data)
#define  HD21_HDMI_GPVS_get_drmps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_drmps(data)
#define  HD21_HDMI_GPVS_get_mpegps(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_mpegps(data)
#define  HD21_HDMI_GPVS_get_aps(data)                                   HDMI21_P0_HD21_HDMI_GPVS_get_aps(data)
#define  HD21_HDMI_GPVS_get_spdps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_spdps(data)
#define  HD21_HDMI_GPVS_get_avips(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_avips(data)
#define  HD21_HDMI_GPVS_get_hdr10pvsps(data)                            HDMI21_P0_HD21_HDMI_GPVS_get_hdr10pvsps(data)
#define  HD21_HDMI_GPVS_get_dvsps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_dvsps(data)
#define  HD21_HDMI_GPVS_get_fvsps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_fvsps(data)
#define  HD21_HDMI_GPVS_get_vsps(data)                                  HDMI21_P0_HD21_HDMI_GPVS_get_vsps(data)
#define  HD21_HDMI_GPVS_get_gmps(data)                                  HDMI21_P0_HD21_HDMI_GPVS_get_gmps(data)
#define  HD21_HDMI_GPVS_get_isrc1ps(data)                               HDMI21_P0_HD21_HDMI_GPVS_get_isrc1ps(data)
#define  HD21_HDMI_GPVS_get_acpps(data)                                 HDMI21_P0_HD21_HDMI_GPVS_get_acpps(data)
#define  HD21_HDMI_GPVS_get_nps(data)                                   HDMI21_P0_HD21_HDMI_GPVS_get_nps(data)
#define  HD21_HDMI_GPVS_get_rsv3ps(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_rsv3ps(data)
#define  HD21_HDMI_GPVS_get_rsv2ps(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_rsv2ps(data)
#define  HD21_HDMI_GPVS_get_rsv1ps(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_rsv1ps(data)
#define  HD21_HDMI_GPVS_get_rsv0ps(data)                                HDMI21_P0_HD21_HDMI_GPVS_get_rsv0ps(data)


#define  HD21_HDMI_GPVS1_dummy_31_4_mask                                HDMI21_P0_HD21_HDMI_GPVS1_dummy_31_4_mask
#define  HD21_HDMI_GPVS1_drmps_rv_mask                                  HDMI21_P0_HD21_HDMI_GPVS1_drmps_rv_mask
#define  HD21_HDMI_GPVS1_hdr10pvsps_rv_mask                             HDMI21_P0_HD21_HDMI_GPVS1_hdr10pvsps_rv_mask
#define  HD21_HDMI_GPVS1_dvsps_rv_mask                                  HDMI21_P0_HD21_HDMI_GPVS1_dvsps_rv_mask
#define  HD21_HDMI_GPVS1_vsps_rv_mask                                   HDMI21_P0_HD21_HDMI_GPVS1_vsps_rv_mask
#define  HD21_HDMI_GPVS1_dummy_31_4(data)                               HDMI21_P0_HD21_HDMI_GPVS1_dummy_31_4(data)
#define  HD21_HDMI_GPVS1_drmps_rv(data)                                 HDMI21_P0_HD21_HDMI_GPVS1_drmps_rv(data)
#define  HD21_HDMI_GPVS1_hdr10pvsps_rv(data)                            HDMI21_P0_HD21_HDMI_GPVS1_hdr10pvsps_rv(data)
#define  HD21_HDMI_GPVS1_dvsps_rv(data)                                 HDMI21_P0_HD21_HDMI_GPVS1_dvsps_rv(data)
#define  HD21_HDMI_GPVS1_vsps_rv(data)                                  HDMI21_P0_HD21_HDMI_GPVS1_vsps_rv(data)
#define  HD21_HDMI_GPVS1_get_dummy_31_4(data)                           HDMI21_P0_HD21_HDMI_GPVS1_get_dummy_31_4(data)
#define  HD21_HDMI_GPVS1_get_drmps_rv(data)                             HDMI21_P0_HD21_HDMI_GPVS1_get_drmps_rv(data)
#define  HD21_HDMI_GPVS1_get_hdr10pvsps_rv(data)                        HDMI21_P0_HD21_HDMI_GPVS1_get_hdr10pvsps_rv(data)
#define  HD21_HDMI_GPVS1_get_dvsps_rv(data)                             HDMI21_P0_HD21_HDMI_GPVS1_get_dvsps_rv(data)
#define  HD21_HDMI_GPVS1_get_vsps_rv(data)                              HDMI21_P0_HD21_HDMI_GPVS1_get_vsps_rv(data)


#define  HD21_HDMI_PSAP_apss_mask                                       HDMI21_P0_HD21_HDMI_PSAP_apss_mask
#define  HD21_HDMI_PSAP_apss(data)                                      HDMI21_P0_HD21_HDMI_PSAP_apss(data)
#define  HD21_HDMI_PSAP_get_apss(data)                                  HDMI21_P0_HD21_HDMI_PSAP_get_apss(data)


#define  HD21_HDMI_PSDP_dpss_mask                                       HDMI21_P0_HD21_HDMI_PSDP_dpss_mask
#define  HD21_HDMI_PSDP_dpss(data)                                      HDMI21_P0_HD21_HDMI_PSDP_dpss(data)
#define  HD21_HDMI_PSDP_get_dpss(data)                                  HDMI21_P0_HD21_HDMI_PSDP_get_dpss(data)


#define  HD21_HDMI_SCR_packet_header_parsing_mode_mask                  HDMI21_P0_HD21_HDMI_SCR_packet_header_parsing_mode_mask
#define  HD21_HDMI_SCR_packet_header_parsing_mode(data)                 HDMI21_P0_HD21_HDMI_SCR_packet_header_parsing_mode(data)
#define  HD21_HDMI_SCR_get_packet_header_parsing_mode(data)             HDMI21_P0_HD21_HDMI_SCR_get_packet_header_parsing_mode(data)


#define  HD21_HDMI_BCHCR_bches2_airq_en_mask                            HDMI21_P0_HD21_HDMI_BCHCR_bches2_airq_en_mask
#define  HD21_HDMI_BCHCR_bches2_irq_en_mask                             HDMI21_P0_HD21_HDMI_BCHCR_bches2_irq_en_mask
#define  HD21_HDMI_BCHCR_bche_mask                                      HDMI21_P0_HD21_HDMI_BCHCR_bche_mask
#define  HD21_HDMI_BCHCR_bches_mask                                     HDMI21_P0_HD21_HDMI_BCHCR_bches_mask
#define  HD21_HDMI_BCHCR_bches2_mask                                    HDMI21_P0_HD21_HDMI_BCHCR_bches2_mask
#define  HD21_HDMI_BCHCR_pe_mask                                        HDMI21_P0_HD21_HDMI_BCHCR_pe_mask
#define  HD21_HDMI_BCHCR_bches2_airq_en(data)                           HDMI21_P0_HD21_HDMI_BCHCR_bches2_airq_en(data)
#define  HD21_HDMI_BCHCR_bches2_irq_en(data)                            HDMI21_P0_HD21_HDMI_BCHCR_bches2_irq_en(data)
#define  HD21_HDMI_BCHCR_bche(data)                                     HDMI21_P0_HD21_HDMI_BCHCR_bche(data)
#define  HD21_HDMI_BCHCR_bches(data)                                    HDMI21_P0_HD21_HDMI_BCHCR_bches(data)
#define  HD21_HDMI_BCHCR_bches2(data)                                   HDMI21_P0_HD21_HDMI_BCHCR_bches2(data)
#define  HD21_HDMI_BCHCR_pe(data)                                       HDMI21_P0_HD21_HDMI_BCHCR_pe(data)
#define  HD21_HDMI_BCHCR_get_bches2_airq_en(data)                       HDMI21_P0_HD21_HDMI_BCHCR_get_bches2_airq_en(data)
#define  HD21_HDMI_BCHCR_get_bches2_irq_en(data)                        HDMI21_P0_HD21_HDMI_BCHCR_get_bches2_irq_en(data)
#define  HD21_HDMI_BCHCR_get_bche(data)                                 HDMI21_P0_HD21_HDMI_BCHCR_get_bche(data)
#define  HD21_HDMI_BCHCR_get_bches(data)                                HDMI21_P0_HD21_HDMI_BCHCR_get_bches(data)
#define  HD21_HDMI_BCHCR_get_bches2(data)                               HDMI21_P0_HD21_HDMI_BCHCR_get_bches2(data)
#define  HD21_HDMI_BCHCR_get_pe(data)                                   HDMI21_P0_HD21_HDMI_BCHCR_get_pe(data)


#define  HD21_HDMI_AVMCR_avmute_flag_mask                               HDMI21_P0_HD21_HDMI_AVMCR_avmute_flag_mask
#define  HD21_HDMI_AVMCR_avmute_win_en_mask                             HDMI21_P0_HD21_HDMI_AVMCR_avmute_win_en_mask
#define  HD21_HDMI_AVMCR_avmute_flag(data)                              HDMI21_P0_HD21_HDMI_AVMCR_avmute_flag(data)
#define  HD21_HDMI_AVMCR_avmute_win_en(data)                            HDMI21_P0_HD21_HDMI_AVMCR_avmute_win_en(data)
#define  HD21_HDMI_AVMCR_get_avmute_flag(data)                          HDMI21_P0_HD21_HDMI_AVMCR_get_avmute_flag(data)
#define  HD21_HDMI_AVMCR_get_avmute_win_en(data)                        HDMI21_P0_HD21_HDMI_AVMCR_get_avmute_win_en(data)


#define  HD21_HDMI_WDCR_hdmi_wd_debug_mode_mask                         HDMI21_P0_HD21_HDMI_WDCR_hdmi_wd_debug_mode_mask
#define  HD21_HDMI_WDCR_hdmi_wd_debug_target_mask                       HDMI21_P0_HD21_HDMI_WDCR_hdmi_wd_debug_target_mask
#define  HD21_HDMI_WDCR_avmute_wd_en_mask                               HDMI21_P0_HD21_HDMI_WDCR_avmute_wd_en_mask
#define  HD21_HDMI_WDCR_hdmi_wd_debug_mode(data)                        HDMI21_P0_HD21_HDMI_WDCR_hdmi_wd_debug_mode(data)
#define  HD21_HDMI_WDCR_hdmi_wd_debug_target(data)                      HDMI21_P0_HD21_HDMI_WDCR_hdmi_wd_debug_target(data)
#define  HD21_HDMI_WDCR_avmute_wd_en(data)                              HDMI21_P0_HD21_HDMI_WDCR_avmute_wd_en(data)
#define  HD21_HDMI_WDCR_get_hdmi_wd_debug_mode(data)                    HDMI21_P0_HD21_HDMI_WDCR_get_hdmi_wd_debug_mode(data)
#define  HD21_HDMI_WDCR_get_hdmi_wd_debug_target(data)                  HDMI21_P0_HD21_HDMI_WDCR_get_hdmi_wd_debug_target(data)
#define  HD21_HDMI_WDCR_get_avmute_wd_en(data)                          HDMI21_P0_HD21_HDMI_WDCR_get_avmute_wd_en(data)


#define  HD21_HDMI_PAMICR_aicpvsb_mask                                  HDMI21_P0_HD21_HDMI_PAMICR_aicpvsb_mask
#define  HD21_HDMI_PAMICR_icpvsb_mask                                   HDMI21_P0_HD21_HDMI_PAMICR_icpvsb_mask
#define  HD21_HDMI_PAMICR_aicpvsb(data)                                 HDMI21_P0_HD21_HDMI_PAMICR_aicpvsb(data)
#define  HD21_HDMI_PAMICR_icpvsb(data)                                  HDMI21_P0_HD21_HDMI_PAMICR_icpvsb(data)
#define  HD21_HDMI_PAMICR_get_aicpvsb(data)                             HDMI21_P0_HD21_HDMI_PAMICR_get_aicpvsb(data)
#define  HD21_HDMI_PAMICR_get_icpvsb(data)                              HDMI21_P0_HD21_HDMI_PAMICR_get_icpvsb(data)


#define  HD21_HDMI_PTRSV1_pt3_mask                                      HDMI21_P0_HD21_HDMI_PTRSV1_pt3_mask
#define  HD21_HDMI_PTRSV1_pt2_mask                                      HDMI21_P0_HD21_HDMI_PTRSV1_pt2_mask
#define  HD21_HDMI_PTRSV1_pt1_mask                                      HDMI21_P0_HD21_HDMI_PTRSV1_pt1_mask
#define  HD21_HDMI_PTRSV1_pt0_mask                                      HDMI21_P0_HD21_HDMI_PTRSV1_pt0_mask
#define  HD21_HDMI_PTRSV1_pt3(data)                                     HDMI21_P0_HD21_HDMI_PTRSV1_pt3(data)
#define  HD21_HDMI_PTRSV1_pt2(data)                                     HDMI21_P0_HD21_HDMI_PTRSV1_pt2(data)
#define  HD21_HDMI_PTRSV1_pt1(data)                                     HDMI21_P0_HD21_HDMI_PTRSV1_pt1(data)
#define  HD21_HDMI_PTRSV1_pt0(data)                                     HDMI21_P0_HD21_HDMI_PTRSV1_pt0(data)
#define  HD21_HDMI_PTRSV1_get_pt3(data)                                 HDMI21_P0_HD21_HDMI_PTRSV1_get_pt3(data)
#define  HD21_HDMI_PTRSV1_get_pt2(data)                                 HDMI21_P0_HD21_HDMI_PTRSV1_get_pt2(data)
#define  HD21_HDMI_PTRSV1_get_pt1(data)                                 HDMI21_P0_HD21_HDMI_PTRSV1_get_pt1(data)
#define  HD21_HDMI_PTRSV1_get_pt0(data)                                 HDMI21_P0_HD21_HDMI_PTRSV1_get_pt0(data)


#define  HD21_HDMI_PTRSV2_pt3_oui_1st_mask                              HDMI21_P0_HD21_HDMI_PTRSV2_pt3_oui_1st_mask
#define  HD21_HDMI_PTRSV2_pt2_oui_1st_mask                              HDMI21_P0_HD21_HDMI_PTRSV2_pt2_oui_1st_mask
#define  HD21_HDMI_PTRSV2_pt1_oui_1st_mask                              HDMI21_P0_HD21_HDMI_PTRSV2_pt1_oui_1st_mask
#define  HD21_HDMI_PTRSV2_pt0_oui_1st_mask                              HDMI21_P0_HD21_HDMI_PTRSV2_pt0_oui_1st_mask
#define  HD21_HDMI_PTRSV2_pt3_oui_1st(data)                             HDMI21_P0_HD21_HDMI_PTRSV2_pt3_oui_1st(data)
#define  HD21_HDMI_PTRSV2_pt2_oui_1st(data)                             HDMI21_P0_HD21_HDMI_PTRSV2_pt2_oui_1st(data)
#define  HD21_HDMI_PTRSV2_pt1_oui_1st(data)                             HDMI21_P0_HD21_HDMI_PTRSV2_pt1_oui_1st(data)
#define  HD21_HDMI_PTRSV2_pt0_oui_1st(data)                             HDMI21_P0_HD21_HDMI_PTRSV2_pt0_oui_1st(data)
#define  HD21_HDMI_PTRSV2_get_pt3_oui_1st(data)                         HDMI21_P0_HD21_HDMI_PTRSV2_get_pt3_oui_1st(data)
#define  HD21_HDMI_PTRSV2_get_pt2_oui_1st(data)                         HDMI21_P0_HD21_HDMI_PTRSV2_get_pt2_oui_1st(data)
#define  HD21_HDMI_PTRSV2_get_pt1_oui_1st(data)                         HDMI21_P0_HD21_HDMI_PTRSV2_get_pt1_oui_1st(data)
#define  HD21_HDMI_PTRSV2_get_pt0_oui_1st(data)                         HDMI21_P0_HD21_HDMI_PTRSV2_get_pt0_oui_1st(data)


#define  HD21_HDMI_PTRSV3_pt3_recognize_oui_en_mask                     HDMI21_P0_HD21_HDMI_PTRSV3_pt3_recognize_oui_en_mask
#define  HD21_HDMI_PTRSV3_pt2_recognize_oui_en_mask                     HDMI21_P0_HD21_HDMI_PTRSV3_pt2_recognize_oui_en_mask
#define  HD21_HDMI_PTRSV3_pt1_recognize_oui_en_mask                     HDMI21_P0_HD21_HDMI_PTRSV3_pt1_recognize_oui_en_mask
#define  HD21_HDMI_PTRSV3_pt0_recognize_oui_en_mask                     HDMI21_P0_HD21_HDMI_PTRSV3_pt0_recognize_oui_en_mask
#define  HD21_HDMI_PTRSV3_pt3_recognize_oui_en(data)                    HDMI21_P0_HD21_HDMI_PTRSV3_pt3_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_pt2_recognize_oui_en(data)                    HDMI21_P0_HD21_HDMI_PTRSV3_pt2_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_pt1_recognize_oui_en(data)                    HDMI21_P0_HD21_HDMI_PTRSV3_pt1_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_pt0_recognize_oui_en(data)                    HDMI21_P0_HD21_HDMI_PTRSV3_pt0_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_get_pt3_recognize_oui_en(data)                HDMI21_P0_HD21_HDMI_PTRSV3_get_pt3_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_get_pt2_recognize_oui_en(data)                HDMI21_P0_HD21_HDMI_PTRSV3_get_pt2_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_get_pt1_recognize_oui_en(data)                HDMI21_P0_HD21_HDMI_PTRSV3_get_pt1_recognize_oui_en(data)
#define  HD21_HDMI_PTRSV3_get_pt0_recognize_oui_en(data)                HDMI21_P0_HD21_HDMI_PTRSV3_get_pt0_recognize_oui_en(data)


#define  HD21_HDMI_PVGCR0_pvsef_mask                                    HDMI21_P0_HD21_HDMI_PVGCR0_pvsef_mask
#define  HD21_HDMI_PVGCR0_pvsef(data)                                   HDMI21_P0_HD21_HDMI_PVGCR0_pvsef(data)
#define  HD21_HDMI_PVGCR0_get_pvsef(data)                               HDMI21_P0_HD21_HDMI_PVGCR0_get_pvsef(data)


#define  HD21_HDMI_PVSR0_pvs_mask                                       HDMI21_P0_HD21_HDMI_PVSR0_pvs_mask
#define  HD21_HDMI_PVSR0_pvs(data)                                      HDMI21_P0_HD21_HDMI_PVSR0_pvs(data)
#define  HD21_HDMI_PVSR0_get_pvs(data)                                  HDMI21_P0_HD21_HDMI_PVSR0_get_pvs(data)


#define  HD21_HDMI_VCR_hdmi_field_mask                                  HDMI21_P0_HD21_HDMI_VCR_hdmi_field_mask
#define  HD21_HDMI_VCR_csc_r_mask                                       HDMI21_P0_HD21_HDMI_VCR_csc_r_mask
#define  HD21_HDMI_VCR_csam_mask                                        HDMI21_P0_HD21_HDMI_VCR_csam_mask
#define  HD21_HDMI_VCR_csc_mask                                         HDMI21_P0_HD21_HDMI_VCR_csc_mask
#define  HD21_HDMI_VCR_prdsam_mask                                      HDMI21_P0_HD21_HDMI_VCR_prdsam_mask
#define  HD21_HDMI_VCR_dsc_r_mask                                       HDMI21_P0_HD21_HDMI_VCR_dsc_r_mask
#define  HD21_HDMI_VCR_eoi_mask                                         HDMI21_P0_HD21_HDMI_VCR_eoi_mask
#define  HD21_HDMI_VCR_ho_mask                                          HDMI21_P0_HD21_HDMI_VCR_ho_mask
#define  HD21_HDMI_VCR_yo_mask                                          HDMI21_P0_HD21_HDMI_VCR_yo_mask
#define  HD21_HDMI_VCR_rs_mask                                          HDMI21_P0_HD21_HDMI_VCR_rs_mask
#define  HD21_HDMI_VCR_dsc_mask                                         HDMI21_P0_HD21_HDMI_VCR_dsc_mask
#define  HD21_HDMI_VCR_hdmi_field(data)                                 HDMI21_P0_HD21_HDMI_VCR_hdmi_field(data)
#define  HD21_HDMI_VCR_csc_r(data)                                      HDMI21_P0_HD21_HDMI_VCR_csc_r(data)
#define  HD21_HDMI_VCR_csam(data)                                       HDMI21_P0_HD21_HDMI_VCR_csam(data)
#define  HD21_HDMI_VCR_csc(data)                                        HDMI21_P0_HD21_HDMI_VCR_csc(data)
#define  HD21_HDMI_VCR_prdsam(data)                                     HDMI21_P0_HD21_HDMI_VCR_prdsam(data)
#define  HD21_HDMI_VCR_dsc_r(data)                                      HDMI21_P0_HD21_HDMI_VCR_dsc_r(data)
#define  HD21_HDMI_VCR_eoi(data)                                        HDMI21_P0_HD21_HDMI_VCR_eoi(data)
#define  HD21_HDMI_VCR_ho(data)                                         HDMI21_P0_HD21_HDMI_VCR_ho(data)
#define  HD21_HDMI_VCR_yo(data)                                         HDMI21_P0_HD21_HDMI_VCR_yo(data)
#define  HD21_HDMI_VCR_rs(data)                                         HDMI21_P0_HD21_HDMI_VCR_rs(data)
#define  HD21_HDMI_VCR_dsc(data)                                        HDMI21_P0_HD21_HDMI_VCR_dsc(data)
#define  HD21_HDMI_VCR_get_hdmi_field(data)                             HDMI21_P0_HD21_HDMI_VCR_get_hdmi_field(data)
#define  HD21_HDMI_VCR_get_csc_r(data)                                  HDMI21_P0_HD21_HDMI_VCR_get_csc_r(data)
#define  HD21_HDMI_VCR_get_csam(data)                                   HDMI21_P0_HD21_HDMI_VCR_get_csam(data)
#define  HD21_HDMI_VCR_get_csc(data)                                    HDMI21_P0_HD21_HDMI_VCR_get_csc(data)
#define  HD21_HDMI_VCR_get_prdsam(data)                                 HDMI21_P0_HD21_HDMI_VCR_get_prdsam(data)
#define  HD21_HDMI_VCR_get_dsc_r(data)                                  HDMI21_P0_HD21_HDMI_VCR_get_dsc_r(data)
#define  HD21_HDMI_VCR_get_eoi(data)                                    HDMI21_P0_HD21_HDMI_VCR_get_eoi(data)
#define  HD21_HDMI_VCR_get_ho(data)                                     HDMI21_P0_HD21_HDMI_VCR_get_ho(data)
#define  HD21_HDMI_VCR_get_yo(data)                                     HDMI21_P0_HD21_HDMI_VCR_get_yo(data)
#define  HD21_HDMI_VCR_get_rs(data)                                     HDMI21_P0_HD21_HDMI_VCR_get_rs(data)
#define  HD21_HDMI_VCR_get_dsc(data)                                    HDMI21_P0_HD21_HDMI_VCR_get_dsc(data)


#define  HD21_HDMI_ACRCR_pucnr_mask                                     HDMI21_P0_HD21_HDMI_ACRCR_pucnr_mask
#define  HD21_HDMI_ACRCR_mode_mask                                    HDMI21_P0_HD21_HDMI_ACRCR_mode_mask
#define  HD21_HDMI_ACRCR_pucnr(data)                                    HDMI21_P0_HD21_HDMI_ACRCR_pucnr(data)
#define  HD21_HDMI_ACRCR_mode(data)                                   HDMI21_P0_HD21_HDMI_ACRCR_mode(data)
#define  HD21_HDMI_ACRCR_get_pucnr(data)                                HDMI21_P0_HD21_HDMI_ACRCR_get_pucnr(data)
#define  HD21_HDMI_ACRCR_get_mode(data)                               HDMI21_P0_HD21_HDMI_ACRCR_get_mode(data)


#define  HD21_HDMI_ACRSR0_cts_mask                                      HDMI21_P0_HD21_HDMI_ACRSR0_cts_mask
#define  HD21_HDMI_ACRSR0_cts(data)                                     HDMI21_P0_HD21_HDMI_ACRSR0_cts(data)
#define  HD21_HDMI_ACRSR0_get_cts(data)                                 HDMI21_P0_HD21_HDMI_ACRSR0_get_cts(data)


#define  HD21_HDMI_ACRSR1_n_mask                                        HDMI21_P0_HD21_HDMI_ACRSR1_n_mask
#define  HD21_HDMI_ACRSR1_n(data)                                       HDMI21_P0_HD21_HDMI_ACRSR1_n(data)
#define  HD21_HDMI_ACRSR1_get_n(data)                                   HDMI21_P0_HD21_HDMI_ACRSR1_get_n(data)


#define  HD21_HDMI_INTCR_apending_mask                                  HDMI21_P0_HD21_HDMI_INTCR_apending_mask
#define  HD21_HDMI_INTCR_pending_mask                                   HDMI21_P0_HD21_HDMI_INTCR_pending_mask
#define  HD21_HDMI_INTCR_aavmute_mask                                   HDMI21_P0_HD21_HDMI_INTCR_aavmute_mask
#define  HD21_HDMI_INTCR_avmute_mask                                    HDMI21_P0_HD21_HDMI_INTCR_avmute_mask
#define  HD21_HDMI_INTCR_apending(data)                                 HDMI21_P0_HD21_HDMI_INTCR_apending(data)
#define  HD21_HDMI_INTCR_pending(data)                                  HDMI21_P0_HD21_HDMI_INTCR_pending(data)
#define  HD21_HDMI_INTCR_aavmute(data)                                  HDMI21_P0_HD21_HDMI_INTCR_aavmute(data)
#define  HD21_HDMI_INTCR_avmute(data)                                   HDMI21_P0_HD21_HDMI_INTCR_avmute(data)
#define  HD21_HDMI_INTCR_get_apending(data)                             HDMI21_P0_HD21_HDMI_INTCR_get_apending(data)
#define  HD21_HDMI_INTCR_get_pending(data)                              HDMI21_P0_HD21_HDMI_INTCR_get_pending(data)
#define  HD21_HDMI_INTCR_get_aavmute(data)                              HDMI21_P0_HD21_HDMI_INTCR_get_aavmute(data)
#define  HD21_HDMI_INTCR_get_avmute(data)                               HDMI21_P0_HD21_HDMI_INTCR_get_avmute(data)

#define  HD21_HDMI_BCSR_bch_err2_mask                                   HDMI21_P0_HD21_HDMI_BCSR_bch_err2_mask
#define  HD21_HDMI_BCSR_gcp_bch_err2_mask                               HDMI21_P0_HD21_HDMI_BCSR_gcp_bch_err2_mask
#define  HD21_HDMI_BCSR_pe_mask                                         HDMI21_P0_HD21_HDMI_BCSR_pe_mask
#define  HD21_HDMI_BCSR_gcp_mask                                        HDMI21_P0_HD21_HDMI_BCSR_gcp_mask
#define  HD21_HDMI_BCSR_bch_err2(data)                                  HDMI21_P0_HD21_HDMI_BCSR_bch_err2(data)
#define  HD21_HDMI_BCSR_gcp_bch_err2(data)                              HDMI21_P0_HD21_HDMI_BCSR_gcp_bch_err2(data)
#define  HD21_HDMI_BCSR_pe(data)                                        HDMI21_P0_HD21_HDMI_BCSR_pe(data)
#define  HD21_HDMI_BCSR_gcp(data)                                       HDMI21_P0_HD21_HDMI_BCSR_gcp(data)
#define  HD21_HDMI_BCSR_get_bch_err2(data)                              HDMI21_P0_HD21_HDMI_BCSR_get_bch_err2(data)
#define  HD21_HDMI_BCSR_get_gcp_bch_err2(data)                          HDMI21_P0_HD21_HDMI_BCSR_get_gcp_bch_err2(data)
#define  HD21_HDMI_BCSR_get_pe(data)                                    HDMI21_P0_HD21_HDMI_BCSR_get_pe(data)
#define  HD21_HDMI_BCSR_get_gcp(data)                                   HDMI21_P0_HD21_HDMI_BCSR_get_gcp(data)


#define  HD21_HDMI_ASR0_fsre_mask                                       HDMI21_P0_HD21_HDMI_ASR0_fsre_mask
#define  HD21_HDMI_ASR0_fsif_mask                                       HDMI21_P0_HD21_HDMI_ASR0_fsif_mask
#define  HD21_HDMI_ASR0_fsre(data)                                      HDMI21_P0_HD21_HDMI_ASR0_fsre(data)
#define  HD21_HDMI_ASR0_fsif(data)                                      HDMI21_P0_HD21_HDMI_ASR0_fsif(data)
#define  HD21_HDMI_ASR0_get_fsre(data)                                  HDMI21_P0_HD21_HDMI_ASR0_get_fsre(data)
#define  HD21_HDMI_ASR0_get_fsif(data)                                  HDMI21_P0_HD21_HDMI_ASR0_get_fsif(data)


#define  HD21_HDMI_ASR1_fbif_mask                                       HDMI21_P0_HD21_HDMI_ASR1_fbif_mask
#define  HD21_HDMI_ASR1_fbif(data)                                      HDMI21_P0_HD21_HDMI_ASR1_fbif(data)
#define  HD21_HDMI_ASR1_get_fbif(data)                                  HDMI21_P0_HD21_HDMI_ASR1_get_fbif(data)


#define  HD21_HDMI_VIDEO_FORMAT_hvf_mask                                HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_hvf_mask
#define  HD21_HDMI_VIDEO_FORMAT_hdmi_vic_mask                           HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_hdmi_vic_mask
#define  HD21_HDMI_VIDEO_FORMAT_hvf(data)                               HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_hvf(data)
#define  HD21_HDMI_VIDEO_FORMAT_hdmi_vic(data)                          HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_hdmi_vic(data)
#define  HD21_HDMI_VIDEO_FORMAT_get_hvf(data)                           HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_get_hvf(data)
#define  HD21_HDMI_VIDEO_FORMAT_get_hdmi_vic(data)                      HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_get_hdmi_vic(data)


#define  HD21_HDMI_3D_FORMAT_hdmi_3d_ext_data_mask                      HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_ext_data_mask
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_metadata_type_mask                 HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_metadata_type_mask
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_structure_mask                     HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_structure_mask
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_meta_present_mask                  HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_meta_present_mask
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_ext_data(data)                     HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_ext_data(data)
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_metadata_type(data)                HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_metadata_type(data)
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_structure(data)                    HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_structure(data)
#define  HD21_HDMI_3D_FORMAT_hdmi_3d_meta_present(data)                 HDMI21_P0_HD21_HDMI_3D_FORMAT_hdmi_3d_meta_present(data)
#define  HD21_HDMI_3D_FORMAT_get_hdmi_3d_ext_data(data)                 HDMI21_P0_HD21_HDMI_3D_FORMAT_get_hdmi_3d_ext_data(data)
#define  HD21_HDMI_3D_FORMAT_get_hdmi_3d_metadata_type(data)            HDMI21_P0_HD21_HDMI_3D_FORMAT_get_hdmi_3d_metadata_type(data)
#define  HD21_HDMI_3D_FORMAT_get_hdmi_3d_structure(data)                HDMI21_P0_HD21_HDMI_3D_FORMAT_get_hdmi_3d_structure(data)
#define  HD21_HDMI_3D_FORMAT_get_hdmi_3d_meta_present(data)             HDMI21_P0_HD21_HDMI_3D_FORMAT_get_hdmi_3d_meta_present(data)


#define  HD21_HDMI_FVS_allm_mode_mask                                   HDMI21_P0_HD21_HDMI_FVS_allm_mode_mask
#define  HD21_HDMI_FVS_ccbpc_mask                                       HDMI21_P0_HD21_HDMI_FVS_ccbpc_mask
#define  HD21_HDMI_FVS_allm_mode(data)                                  HDMI21_P0_HD21_HDMI_FVS_allm_mode(data)
#define  HD21_HDMI_FVS_ccbpc(data)                                      HDMI21_P0_HD21_HDMI_FVS_ccbpc(data)
#define  HD21_HDMI_FVS_get_allm_mode(data)                              HDMI21_P0_HD21_HDMI_FVS_get_allm_mode(data)
#define  HD21_HDMI_FVS_get_ccbpc(data)                                  HDMI21_P0_HD21_HDMI_FVS_get_ccbpc(data)


#define  HD21_HDMI_DRM_static_metadata_descriptor_id_mask               HDMI21_P0_HD21_HDMI_DRM_static_metadata_descriptor_id_mask
#define  HD21_HDMI_DRM_eotf_mask                                        HDMI21_P0_HD21_HDMI_DRM_eotf_mask
#define  HD21_HDMI_DRM_static_metadata_descriptor_id(data)              HDMI21_P0_HD21_HDMI_DRM_static_metadata_descriptor_id(data)
#define  HD21_HDMI_DRM_eotf(data)                                       HDMI21_P0_HD21_HDMI_DRM_eotf(data)
#define  HD21_HDMI_DRM_get_static_metadata_descriptor_id(data)          HDMI21_P0_HD21_HDMI_DRM_get_static_metadata_descriptor_id(data)
#define  HD21_HDMI_DRM_get_eotf(data)                                   HDMI21_P0_HD21_HDMI_DRM_get_eotf(data)


#define  HD21_HDMI_PCMC_drm_info_clr_mask                               HDMI21_P0_HD21_HDMI_PCMC_drm_info_clr_mask
#define  HD21_HDMI_PCMC_mpeg_info_clr_mask                              HDMI21_P0_HD21_HDMI_PCMC_mpeg_info_clr_mask
#define  HD21_HDMI_PCMC_audio_info_clr_mask                             HDMI21_P0_HD21_HDMI_PCMC_audio_info_clr_mask
#define  HD21_HDMI_PCMC_spd_info_clr_mask                               HDMI21_P0_HD21_HDMI_PCMC_spd_info_clr_mask
#define  HD21_HDMI_PCMC_avi_info_clr_mask                               HDMI21_P0_HD21_HDMI_PCMC_avi_info_clr_mask
#define  HD21_HDMI_PCMC_hdr10pvs_info_clr_mask                          HDMI21_P0_HD21_HDMI_PCMC_hdr10pvs_info_clr_mask
#define  HD21_HDMI_PCMC_dvs_info_clr_mask                               HDMI21_P0_HD21_HDMI_PCMC_dvs_info_clr_mask
#define  HD21_HDMI_PCMC_fvs_info_clr_mask                               HDMI21_P0_HD21_HDMI_PCMC_fvs_info_clr_mask
#define  HD21_HDMI_PCMC_vs_info_clr_mask                                HDMI21_P0_HD21_HDMI_PCMC_vs_info_clr_mask
#define  HD21_HDMI_PCMC_gm_clr_mask                                     HDMI21_P0_HD21_HDMI_PCMC_gm_clr_mask
#define  HD21_HDMI_PCMC_isrc2_clr_mask                                  HDMI21_P0_HD21_HDMI_PCMC_isrc2_clr_mask
#define  HD21_HDMI_PCMC_isrc1_clr_mask                                  HDMI21_P0_HD21_HDMI_PCMC_isrc1_clr_mask
#define  HD21_HDMI_PCMC_acp_clr_mask                                    HDMI21_P0_HD21_HDMI_PCMC_acp_clr_mask
#define  HD21_HDMI_PCMC_ackg_clr_mask                                   HDMI21_P0_HD21_HDMI_PCMC_ackg_clr_mask
#define  HD21_HDMI_PCMC_drm_info_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_drm_info_clr(data)
#define  HD21_HDMI_PCMC_mpeg_info_clr(data)                             HDMI21_P0_HD21_HDMI_PCMC_mpeg_info_clr(data)
#define  HD21_HDMI_PCMC_audio_info_clr(data)                            HDMI21_P0_HD21_HDMI_PCMC_audio_info_clr(data)
#define  HD21_HDMI_PCMC_spd_info_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_spd_info_clr(data)
#define  HD21_HDMI_PCMC_avi_info_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_avi_info_clr(data)
#define  HD21_HDMI_PCMC_hdr10pvs_info_clr(data)                         HDMI21_P0_HD21_HDMI_PCMC_hdr10pvs_info_clr(data)
#define  HD21_HDMI_PCMC_dvs_info_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_dvs_info_clr(data)
#define  HD21_HDMI_PCMC_fvs_info_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_fvs_info_clr(data)
#define  HD21_HDMI_PCMC_vs_info_clr(data)                               HDMI21_P0_HD21_HDMI_PCMC_vs_info_clr(data)
#define  HD21_HDMI_PCMC_gm_clr(data)                                    HDMI21_P0_HD21_HDMI_PCMC_gm_clr(data)
#define  HD21_HDMI_PCMC_isrc2_clr(data)                                 HDMI21_P0_HD21_HDMI_PCMC_isrc2_clr(data)
#define  HD21_HDMI_PCMC_isrc1_clr(data)                                 HDMI21_P0_HD21_HDMI_PCMC_isrc1_clr(data)
#define  HD21_HDMI_PCMC_acp_clr(data)                                   HDMI21_P0_HD21_HDMI_PCMC_acp_clr(data)
#define  HD21_HDMI_PCMC_ackg_clr(data)                                  HDMI21_P0_HD21_HDMI_PCMC_ackg_clr(data)
#define  HD21_HDMI_PCMC_get_drm_info_clr(data)                          HDMI21_P0_HD21_HDMI_PCMC_get_drm_info_clr(data)
#define  HD21_HDMI_PCMC_get_mpeg_info_clr(data)                         HDMI21_P0_HD21_HDMI_PCMC_get_mpeg_info_clr(data)
#define  HD21_HDMI_PCMC_get_audio_info_clr(data)                        HDMI21_P0_HD21_HDMI_PCMC_get_audio_info_clr(data)
#define  HD21_HDMI_PCMC_get_spd_info_clr(data)                          HDMI21_P0_HD21_HDMI_PCMC_get_spd_info_clr(data)
#define  HD21_HDMI_PCMC_get_avi_info_clr(data)                          HDMI21_P0_HD21_HDMI_PCMC_get_avi_info_clr(data)
#define  HD21_HDMI_PCMC_get_hdr10pvs_info_clr(data)                     HDMI21_P0_HD21_HDMI_PCMC_get_hdr10pvs_info_clr(data)
#define  HD21_HDMI_PCMC_get_dvs_info_clr(data)                          HDMI21_P0_HD21_HDMI_PCMC_get_dvs_info_clr(data)
#define  HD21_HDMI_PCMC_get_fvs_info_clr(data)                          HDMI21_P0_HD21_HDMI_PCMC_get_fvs_info_clr(data)
#define  HD21_HDMI_PCMC_get_vs_info_clr(data)                           HDMI21_P0_HD21_HDMI_PCMC_get_vs_info_clr(data)
#define  HD21_HDMI_PCMC_get_gm_clr(data)                                HDMI21_P0_HD21_HDMI_PCMC_get_gm_clr(data)
#define  HD21_HDMI_PCMC_get_isrc2_clr(data)                             HDMI21_P0_HD21_HDMI_PCMC_get_isrc2_clr(data)
#define  HD21_HDMI_PCMC_get_isrc1_clr(data)                             HDMI21_P0_HD21_HDMI_PCMC_get_isrc1_clr(data)
#define  HD21_HDMI_PCMC_get_acp_clr(data)                               HDMI21_P0_HD21_HDMI_PCMC_get_acp_clr(data)
#define  HD21_HDMI_PCMC_get_ackg_clr(data)                              HDMI21_P0_HD21_HDMI_PCMC_get_ackg_clr(data)


#define  HD21_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq_mask         HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq_mask
#define  HD21_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq_mask            HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq_mask
#define  HD21_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq(data)        HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_en_ncts_chg_irq(data)
#define  HD21_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq(data)           HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_ncts_chg_irq(data)
#define  HD21_Audio_Sample_Rate_Change_IRQ_get_en_ncts_chg_irq(data)    HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_get_en_ncts_chg_irq(data)
#define  HD21_Audio_Sample_Rate_Change_IRQ_get_ncts_chg_irq(data)       HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_get_ncts_chg_irq(data)


#define  HD21_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask            HDMI21_P0_HD21_High_Bit_Rate_Audio_Packet_hbr_audio_mode_mask
#define  HD21_High_Bit_Rate_Audio_Packet_hbr_audio_mode(data)           HDMI21_P0_HD21_High_Bit_Rate_Audio_Packet_hbr_audio_mode(data)
#define  HD21_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(data)       HDMI21_P0_HD21_High_Bit_Rate_Audio_Packet_get_hbr_audio_mode(data)


#define  HD21_HDMI_AUDCR_aud_data_fw_mask                               HDMI21_P0_HD21_HDMI_AUDCR_aud_data_fw_mask
#define  HD21_HDMI_AUDCR_pck_hbr_aud_gated_en_mask                      HDMI21_P0_HD21_HDMI_AUDCR_pck_hbr_aud_gated_en_mask
#define  HD21_HDMI_AUDCR_pck_aud_gated_en_mask                          HDMI21_P0_HD21_HDMI_AUDCR_pck_aud_gated_en_mask
#define  HD21_HDMI_AUDCR_aud_cptest_en_mask                             HDMI21_P0_HD21_HDMI_AUDCR_aud_cptest_en_mask
#define  HD21_HDMI_AUDCR_sine_replace_en_mask                           HDMI21_P0_HD21_HDMI_AUDCR_sine_replace_en_mask
#define  HD21_HDMI_AUDCR_bch2_repeat_en_mask                            HDMI21_P0_HD21_HDMI_AUDCR_bch2_repeat_en_mask
#define  HD21_HDMI_AUDCR_flat_en_mask                                   HDMI21_P0_HD21_HDMI_AUDCR_flat_en_mask
#define  HD21_HDMI_AUDCR_aud_en_mask                                    HDMI21_P0_HD21_HDMI_AUDCR_aud_en_mask
#define  HD21_HDMI_AUDCR_aud_data_fw(data)                              HDMI21_P0_HD21_HDMI_AUDCR_aud_data_fw(data)
#define  HD21_HDMI_AUDCR_pck_hbr_aud_gated_en(data)                     HDMI21_P0_HD21_HDMI_AUDCR_pck_hbr_aud_gated_en(data)
#define  HD21_HDMI_AUDCR_pck_aud_gated_en(data)                         HDMI21_P0_HD21_HDMI_AUDCR_pck_aud_gated_en(data)
#define  HD21_HDMI_AUDCR_aud_cptest_en(data)                            HDMI21_P0_HD21_HDMI_AUDCR_aud_cptest_en(data)
#define  HD21_HDMI_AUDCR_sine_replace_en(data)                          HDMI21_P0_HD21_HDMI_AUDCR_sine_replace_en(data)
#define  HD21_HDMI_AUDCR_bch2_repeat_en(data)                           HDMI21_P0_HD21_HDMI_AUDCR_bch2_repeat_en(data)
#define  HD21_HDMI_AUDCR_flat_en(data)                                  HDMI21_P0_HD21_HDMI_AUDCR_flat_en(data)
#define  HD21_HDMI_AUDCR_aud_en(data)                                   HDMI21_P0_HD21_HDMI_AUDCR_aud_en(data)
#define  HD21_HDMI_AUDCR_get_aud_data_fw(data)                          HDMI21_P0_HD21_HDMI_AUDCR_get_aud_data_fw(data)
#define  HD21_HDMI_AUDCR_get_pck_hbr_aud_gated_en(data)                 HDMI21_P0_HD21_HDMI_AUDCR_get_pck_hbr_aud_gated_en(data)
#define  HD21_HDMI_AUDCR_get_pck_aud_gated_en(data)                     HDMI21_P0_HD21_HDMI_AUDCR_get_pck_aud_gated_en(data)
#define  HD21_HDMI_AUDCR_get_aud_cptest_en(data)                        HDMI21_P0_HD21_HDMI_AUDCR_get_aud_cptest_en(data)
#define  HD21_HDMI_AUDCR_get_sine_replace_en(data)                      HDMI21_P0_HD21_HDMI_AUDCR_get_sine_replace_en(data)
#define  HD21_HDMI_AUDCR_get_bch2_repeat_en(data)                       HDMI21_P0_HD21_HDMI_AUDCR_get_bch2_repeat_en(data)
#define  HD21_HDMI_AUDCR_get_flat_en(data)                              HDMI21_P0_HD21_HDMI_AUDCR_get_flat_en(data)
#define  HD21_HDMI_AUDCR_get_aud_en(data)                               HDMI21_P0_HD21_HDMI_AUDCR_get_aud_en(data)


#define  HD21_HDMI_AUDSR_aud_ch_mode_mask                               HDMI21_P0_HD21_HDMI_AUDSR_aud_ch_mode_mask
#define  HD21_HDMI_AUDSR_aud_fifof_mask                                 HDMI21_P0_HD21_HDMI_AUDSR_aud_fifof_mask
#define  HD21_HDMI_AUDSR_aud_ch_mode(data)                              HDMI21_P0_HD21_HDMI_AUDSR_aud_ch_mode(data)
#define  HD21_HDMI_AUDSR_aud_fifof(data)                                HDMI21_P0_HD21_HDMI_AUDSR_aud_fifof(data)
#define  HD21_HDMI_AUDSR_get_aud_ch_mode(data)                          HDMI21_P0_HD21_HDMI_AUDSR_get_aud_ch_mode(data)
#define  HD21_HDMI_AUDSR_get_aud_fifof(data)                            HDMI21_P0_HD21_HDMI_AUDSR_get_aud_fifof(data)


#define  HD21_AUDIO_CTS_UP_BOUND_cts_over_flag_mask                     HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_over_flag_mask
#define  HD21_AUDIO_CTS_UP_BOUND_cts_up_bound_mask                      HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_up_bound_mask
#define  HD21_AUDIO_CTS_UP_BOUND_cts_over_flag(data)                    HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_over_flag(data)
#define  HD21_AUDIO_CTS_UP_BOUND_cts_up_bound(data)                     HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_cts_up_bound(data)
#define  HD21_AUDIO_CTS_UP_BOUND_get_cts_over_flag(data)                HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_get_cts_over_flag(data)
#define  HD21_AUDIO_CTS_UP_BOUND_get_cts_up_bound(data)                 HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_get_cts_up_bound(data)


#define  HD21_AUDIO_CTS_LOW_BOUND_cts_under_flag_mask                   HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_under_flag_mask
#define  HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask                    HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound_mask
#define  HD21_AUDIO_CTS_LOW_BOUND_cts_under_flag(data)                  HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_under_flag(data)
#define  HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound(data)                   HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_cts_low_bound(data)
#define  HD21_AUDIO_CTS_LOW_BOUND_get_cts_under_flag(data)              HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_get_cts_under_flag(data)
#define  HD21_AUDIO_CTS_LOW_BOUND_get_cts_low_bound(data)               HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_get_cts_low_bound(data)


#define  HD21_AUDIO_N_UP_BOUND_n_over_flag_mask                         HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_over_flag_mask
#define  HD21_AUDIO_N_UP_BOUND_n_up_bound_mask                          HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_up_bound_mask
#define  HD21_AUDIO_N_UP_BOUND_n_over_flag(data)                        HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_over_flag(data)
#define  HD21_AUDIO_N_UP_BOUND_n_up_bound(data)                         HDMI21_P0_HD21_AUDIO_N_UP_BOUND_n_up_bound(data)
#define  HD21_AUDIO_N_UP_BOUND_get_n_over_flag(data)                    HDMI21_P0_HD21_AUDIO_N_UP_BOUND_get_n_over_flag(data)
#define  HD21_AUDIO_N_UP_BOUND_get_n_up_bound(data)                     HDMI21_P0_HD21_AUDIO_N_UP_BOUND_get_n_up_bound(data)


#define  HD21_AUDIO_N_LOW_BOUND_n_under_flag_mask                       HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_under_flag_mask
#define  HD21_AUDIO_N_LOW_BOUND_n_low_bound_mask                        HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_low_bound_mask
#define  HD21_AUDIO_N_LOW_BOUND_n_under_flag(data)                      HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_under_flag(data)
#define  HD21_AUDIO_N_LOW_BOUND_n_low_bound(data)                       HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_n_low_bound(data)
#define  HD21_AUDIO_N_LOW_BOUND_get_n_under_flag(data)                  HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_get_n_under_flag(data)
#define  HD21_AUDIO_N_LOW_BOUND_get_n_low_bound(data)                   HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_get_n_low_bound(data)


#define  HD21_CLKDETCR_trans_meas_sel_mask                              HDMI21_P0_HD21_CLKDETCR_trans_meas_sel_mask
#define  HD21_CLKDETCR_det_sel_mask                                     HDMI21_P0_HD21_CLKDETCR_det_sel_mask
#define  HD21_CLKDETCR_clkv_meas_sel_mask                               HDMI21_P0_HD21_CLKDETCR_clkv_meas_sel_mask
#define  HD21_CLKDETCR_clock_det_en_mask                                HDMI21_P0_HD21_CLKDETCR_clock_det_en_mask
#define  HD21_CLKDETCR_trans_meas_sel(data)                             HDMI21_P0_HD21_CLKDETCR_trans_meas_sel(data)
#define  HD21_CLKDETCR_det_sel(data)                                    HDMI21_P0_HD21_CLKDETCR_det_sel(data)
#define  HD21_CLKDETCR_clkv_meas_sel(data)                              HDMI21_P0_HD21_CLKDETCR_clkv_meas_sel(data)
#define  HD21_CLKDETCR_clock_det_en(data)                               HDMI21_P0_HD21_CLKDETCR_clock_det_en(data)
#define  HD21_CLKDETCR_get_trans_meas_sel(data)                         HDMI21_P0_HD21_CLKDETCR_get_trans_meas_sel(data)
#define  HD21_CLKDETCR_get_det_sel(data)                                HDMI21_P0_HD21_CLKDETCR_get_det_sel(data)
#define  HD21_CLKDETCR_get_clkv_meas_sel(data)                          HDMI21_P0_HD21_CLKDETCR_get_clkv_meas_sel(data)
#define  HD21_CLKDETCR_get_clock_det_en(data)                           HDMI21_P0_HD21_CLKDETCR_get_clock_det_en(data)


#define  HD21_CLKDETSR_reset_counter_mask                               HDMI21_P0_HD21_CLKDETSR_reset_counter_mask
#define  HD21_CLKDETSR_pop_out_mask                                     HDMI21_P0_HD21_CLKDETSR_pop_out_mask
#define  HD21_CLKDETSR_clk_counter_mask                                 HDMI21_P0_HD21_CLKDETSR_clk_counter_mask
#define  HD21_CLKDETSR_en_clk_chg_irq_mask                              HDMI21_P0_HD21_CLKDETSR_en_clk_chg_irq_mask
#define  HD21_CLKDETSR_clk_chg_irq_mask                                 HDMI21_P0_HD21_CLKDETSR_clk_chg_irq_mask
#define  HD21_CLKDETSR_dummy_3_2_mask                                   HDMI21_P0_HD21_CLKDETSR_dummy_3_2_mask
#define  HD21_CLKDETSR_clk_in_target_irq_en_mask                        HDMI21_P0_HD21_CLKDETSR_clk_in_target_irq_en_mask
#define  HD21_CLKDETSR_clk_in_target_mask                               HDMI21_P0_HD21_CLKDETSR_clk_in_target_mask
#define  HD21_CLKDETSR_reset_counter(data)                              HDMI21_P0_HD21_CLKDETSR_reset_counter(data)
#define  HD21_CLKDETSR_pop_out(data)                                    HDMI21_P0_HD21_CLKDETSR_pop_out(data)
#define  HD21_CLKDETSR_clk_counter(data)                                HDMI21_P0_HD21_CLKDETSR_clk_counter(data)
#define  HD21_CLKDETSR_en_clk_chg_irq(data)                             HDMI21_P0_HD21_CLKDETSR_en_clk_chg_irq(data)
#define  HD21_CLKDETSR_clk_chg_irq(data)                                HDMI21_P0_HD21_CLKDETSR_clk_chg_irq(data)
#define  HD21_CLKDETSR_dummy_3_2(data)                                  HDMI21_P0_HD21_CLKDETSR_dummy_3_2(data)
#define  HD21_CLKDETSR_clk_in_target_irq_en(data)                       HDMI21_P0_HD21_CLKDETSR_clk_in_target_irq_en(data)
#define  HD21_CLKDETSR_clk_in_target(data)                              HDMI21_P0_HD21_CLKDETSR_clk_in_target(data)
#define  HD21_CLKDETSR_get_reset_counter(data)                          HDMI21_P0_HD21_CLKDETSR_get_reset_counter(data)
#define  HD21_CLKDETSR_get_pop_out(data)                                HDMI21_P0_HD21_CLKDETSR_get_pop_out(data)
#define  HD21_CLKDETSR_get_clk_counter(data)                            HDMI21_P0_HD21_CLKDETSR_get_clk_counter(data)
#define  HD21_CLKDETSR_get_en_clk_chg_irq(data)                         HDMI21_P0_HD21_CLKDETSR_get_en_clk_chg_irq(data)
#define  HD21_CLKDETSR_get_clk_chg_irq(data)                            HDMI21_P0_HD21_CLKDETSR_get_clk_chg_irq(data)
#define  HD21_CLKDETSR_get_dummy_3_2(data)                              HDMI21_P0_HD21_CLKDETSR_get_dummy_3_2(data)
#define  HD21_CLKDETSR_get_clk_in_target_irq_en(data)                   HDMI21_P0_HD21_CLKDETSR_get_clk_in_target_irq_en(data)
#define  HD21_CLKDETSR_get_clk_in_target(data)                          HDMI21_P0_HD21_CLKDETSR_get_clk_in_target(data)


#define  HD21_CLK_SETTING_00_dclk_cnt_start_mask                        HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_start_mask
#define  HD21_CLK_SETTING_00_dclk_cnt_end_mask                          HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_end_mask
#define  HD21_CLK_SETTING_00_dclk_cnt_start(data)                       HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_start(data)
#define  HD21_CLK_SETTING_00_dclk_cnt_end(data)                         HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_end(data)
#define  HD21_CLK_SETTING_00_get_dclk_cnt_start(data)                   HDMI21_P0_HD21_CLK_SETTING_00_get_dclk_cnt_start(data)
#define  HD21_CLK_SETTING_00_get_dclk_cnt_end(data)                     HDMI21_P0_HD21_CLK_SETTING_00_get_dclk_cnt_end(data)


#define  HD21_CLK_SETTING_01_target_for_maximum_clk_counter_mask        HDMI21_P0_HD21_CLK_SETTING_01_target_for_maximum_clk_counter_mask
#define  HD21_CLK_SETTING_01_target_for_minimum_clk_counter_mask        HDMI21_P0_HD21_CLK_SETTING_01_target_for_minimum_clk_counter_mask
#define  HD21_CLK_SETTING_01_target_for_maximum_clk_counter(data)       HDMI21_P0_HD21_CLK_SETTING_01_target_for_maximum_clk_counter(data)
#define  HD21_CLK_SETTING_01_target_for_minimum_clk_counter(data)       HDMI21_P0_HD21_CLK_SETTING_01_target_for_minimum_clk_counter(data)
#define  HD21_CLK_SETTING_01_get_target_for_maximum_clk_counter(data)   HDMI21_P0_HD21_CLK_SETTING_01_get_target_for_maximum_clk_counter(data)
#define  HD21_CLK_SETTING_01_get_target_for_minimum_clk_counter(data)   HDMI21_P0_HD21_CLK_SETTING_01_get_target_for_minimum_clk_counter(data)


#define  HD21_CLK_SETTING_02_clk_counter_err_threshold_mask             HDMI21_P0_HD21_CLK_SETTING_02_clk_counter_err_threshold_mask
#define  HD21_CLK_SETTING_02_clk_counter_debounce_mask                  HDMI21_P0_HD21_CLK_SETTING_02_clk_counter_debounce_mask
#define  HD21_CLK_SETTING_02_clk_counter_err_threshold(data)            HDMI21_P0_HD21_CLK_SETTING_02_clk_counter_err_threshold(data)
#define  HD21_CLK_SETTING_02_clk_counter_debounce(data)                 HDMI21_P0_HD21_CLK_SETTING_02_clk_counter_debounce(data)
#define  HD21_CLK_SETTING_02_get_clk_counter_err_threshold(data)        HDMI21_P0_HD21_CLK_SETTING_02_get_clk_counter_err_threshold(data)
#define  HD21_CLK_SETTING_02_get_clk_counter_debounce(data)             HDMI21_P0_HD21_CLK_SETTING_02_get_clk_counter_debounce(data)


#define  HD21_CLKDET2CR_trans_meas_sel_mask                             HDMI21_P0_HD21_CLKDET2CR_trans_meas_sel_mask
#define  HD21_CLKDET2CR_clock_det_en_mask                               HDMI21_P0_HD21_CLKDET2CR_clock_det_en_mask
#define  HD21_CLKDET2CR_trans_meas_sel(data)                            HDMI21_P0_HD21_CLKDET2CR_trans_meas_sel(data)
#define  HD21_CLKDET2CR_clock_det_en(data)                              HDMI21_P0_HD21_CLKDET2CR_clock_det_en(data)
#define  HD21_CLKDET2CR_get_trans_meas_sel(data)                        HDMI21_P0_HD21_CLKDET2CR_get_trans_meas_sel(data)
#define  HD21_CLKDET2CR_get_clock_det_en(data)                          HDMI21_P0_HD21_CLKDET2CR_get_clock_det_en(data)


#define  HD21_CLKDET2SR_reset_counter_mask                              HDMI21_P0_HD21_CLKDET2SR_reset_counter_mask
#define  HD21_CLKDET2SR_pop_out_mask                                    HDMI21_P0_HD21_CLKDET2SR_pop_out_mask
#define  HD21_CLKDET2SR_clk_counter_mask                                HDMI21_P0_HD21_CLKDET2SR_clk_counter_mask
#define  HD21_CLKDET2SR_en_clk_chg_irq_mask                             HDMI21_P0_HD21_CLKDET2SR_en_clk_chg_irq_mask
#define  HD21_CLKDET2SR_clk_chg_irq_mask                                HDMI21_P0_HD21_CLKDET2SR_clk_chg_irq_mask
#define  HD21_CLKDET2SR_dummy_3_2_mask                                  HDMI21_P0_HD21_CLKDET2SR_dummy_3_2_mask
#define  HD21_CLKDET2SR_clk_in_target_irq_en_mask                       HDMI21_P0_HD21_CLKDET2SR_clk_in_target_irq_en_mask
#define  HD21_CLKDET2SR_clk_in_target_mask                              HDMI21_P0_HD21_CLKDET2SR_clk_in_target_mask
#define  HD21_CLKDET2SR_reset_counter(data)                             HDMI21_P0_HD21_CLKDET2SR_reset_counter(data)
#define  HD21_CLKDET2SR_pop_out(data)                                   HDMI21_P0_HD21_CLKDET2SR_pop_out(data)
#define  HD21_CLKDET2SR_clk_counter(data)                               HDMI21_P0_HD21_CLKDET2SR_clk_counter(data)
#define  HD21_CLKDET2SR_en_clk_chg_irq(data)                            HDMI21_P0_HD21_CLKDET2SR_en_clk_chg_irq(data)
#define  HD21_CLKDET2SR_clk_chg_irq(data)                               HDMI21_P0_HD21_CLKDET2SR_clk_chg_irq(data)
#define  HD21_CLKDET2SR_dummy_3_2(data)                                 HDMI21_P0_HD21_CLKDET2SR_dummy_3_2(data)
#define  HD21_CLKDET2SR_clk_in_target_irq_en(data)                      HDMI21_P0_HD21_CLKDET2SR_clk_in_target_irq_en(data)
#define  HD21_CLKDET2SR_clk_in_target(data)                             HDMI21_P0_HD21_CLKDET2SR_clk_in_target(data)
#define  HD21_CLKDET2SR_get_reset_counter(data)                         HDMI21_P0_HD21_CLKDET2SR_get_reset_counter(data)
#define  HD21_CLKDET2SR_get_pop_out(data)                               HDMI21_P0_HD21_CLKDET2SR_get_pop_out(data)
#define  HD21_CLKDET2SR_get_clk_counter(data)                           HDMI21_P0_HD21_CLKDET2SR_get_clk_counter(data)
#define  HD21_CLKDET2SR_get_en_clk_chg_irq(data)                        HDMI21_P0_HD21_CLKDET2SR_get_en_clk_chg_irq(data)
#define  HD21_CLKDET2SR_get_clk_chg_irq(data)                           HDMI21_P0_HD21_CLKDET2SR_get_clk_chg_irq(data)
#define  HD21_CLKDET2SR_get_dummy_3_2(data)                             HDMI21_P0_HD21_CLKDET2SR_get_dummy_3_2(data)
#define  HD21_CLKDET2SR_get_clk_in_target_irq_en(data)                  HDMI21_P0_HD21_CLKDET2SR_get_clk_in_target_irq_en(data)
#define  HD21_CLKDET2SR_get_clk_in_target(data)                         HDMI21_P0_HD21_CLKDET2SR_get_clk_in_target(data)


#define  HD21_CLK2_SETTING_00_dclk_cnt_start_mask                       HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_start_mask
#define  HD21_CLK2_SETTING_00_dclk_cnt_end_mask                         HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_end_mask
#define  HD21_CLK2_SETTING_00_dclk_cnt_start(data)                      HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_start(data)
#define  HD21_CLK2_SETTING_00_dclk_cnt_end(data)                        HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_end(data)
#define  HD21_CLK2_SETTING_00_get_dclk_cnt_start(data)                  HDMI21_P0_HD21_CLK2_SETTING_00_get_dclk_cnt_start(data)
#define  HD21_CLK2_SETTING_00_get_dclk_cnt_end(data)                    HDMI21_P0_HD21_CLK2_SETTING_00_get_dclk_cnt_end(data)


#define  HD21_CLK2_SETTING_01_target_for_maximum_clk_counter_mask       HDMI21_P0_HD21_CLK2_SETTING_01_target_for_maximum_clk_counter_mask
#define  HD21_CLK2_SETTING_01_target_for_minimum_clk_counter_mask       HDMI21_P0_HD21_CLK2_SETTING_01_target_for_minimum_clk_counter_mask
#define  HD21_CLK2_SETTING_01_target_for_maximum_clk_counter(data)      HDMI21_P0_HD21_CLK2_SETTING_01_target_for_maximum_clk_counter(data)
#define  HD21_CLK2_SETTING_01_target_for_minimum_clk_counter(data)      HDMI21_P0_HD21_CLK2_SETTING_01_target_for_minimum_clk_counter(data)
#define  HD21_CLK2_SETTING_01_get_target_for_maximum_clk_counter(data)  HDMI21_P0_HD21_CLK2_SETTING_01_get_target_for_maximum_clk_counter(data)
#define  HD21_CLK2_SETTING_01_get_target_for_minimum_clk_counter(data)  HDMI21_P0_HD21_CLK2_SETTING_01_get_target_for_minimum_clk_counter(data)


#define  HD21_CLK2_SETTING_02_clk_counter_err_threshold_mask            HDMI21_P0_HD21_CLK2_SETTING_02_clk_counter_err_threshold_mask
#define  HD21_CLK2_SETTING_02_clk_counter_debounce_mask                 HDMI21_P0_HD21_CLK2_SETTING_02_clk_counter_debounce_mask
#define  HD21_CLK2_SETTING_02_clk_counter_err_threshold(data)           HDMI21_P0_HD21_CLK2_SETTING_02_clk_counter_err_threshold(data)
#define  HD21_CLK2_SETTING_02_clk_counter_debounce(data)                HDMI21_P0_HD21_CLK2_SETTING_02_clk_counter_debounce(data)
#define  HD21_CLK2_SETTING_02_get_clk_counter_err_threshold(data)       HDMI21_P0_HD21_CLK2_SETTING_02_get_clk_counter_err_threshold(data)
#define  HD21_CLK2_SETTING_02_get_clk_counter_debounce(data)            HDMI21_P0_HD21_CLK2_SETTING_02_get_clk_counter_debounce(data)


#define  HD21_CLK_HWM_SETTING_00_one_time_n_mask                        HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_n_mask
#define  HD21_CLK_HWM_SETTING_00_one_time_en_mask                       HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_en_mask
#define  HD21_CLK_HWM_SETTING_00_one_time_n(data)                       HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_n(data)
#define  HD21_CLK_HWM_SETTING_00_one_time_en(data)                      HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_en(data)
#define  HD21_CLK_HWM_SETTING_00_get_one_time_n(data)                   HDMI21_P0_HD21_CLK_HWM_SETTING_00_get_one_time_n(data)
#define  HD21_CLK_HWM_SETTING_00_get_one_time_en(data)                  HDMI21_P0_HD21_CLK_HWM_SETTING_00_get_one_time_en(data)


#define  HD21_CLK_HWM_SETTING_01_cont_offset_n_mask                     HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_offset_n_mask
#define  HD21_CLK_HWM_SETTING_01_cont_x_mask                            HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_x_mask
#define  HD21_CLK_HWM_SETTING_01_cont_en_mask                           HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en_mask
#define  HD21_CLK_HWM_SETTING_01_cont_offset_n(data)                    HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_offset_n(data)
#define  HD21_CLK_HWM_SETTING_01_cont_x(data)                           HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_x(data)
#define  HD21_CLK_HWM_SETTING_01_cont_en(data)                          HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en(data)
#define  HD21_CLK_HWM_SETTING_01_get_cont_offset_n(data)                HDMI21_P0_HD21_CLK_HWM_SETTING_01_get_cont_offset_n(data)
#define  HD21_CLK_HWM_SETTING_01_get_cont_x(data)                       HDMI21_P0_HD21_CLK_HWM_SETTING_01_get_cont_x(data)
#define  HD21_CLK_HWM_SETTING_01_get_cont_en(data)                      HDMI21_P0_HD21_CLK_HWM_SETTING_01_get_cont_en(data)


#define  HD21_CLK_HWM_SETTING_02_cont_err_th_mask                       HDMI21_P0_HD21_CLK_HWM_SETTING_02_cont_err_th_mask
#define  HD21_CLK_HWM_SETTING_02_real_mode_mask                         HDMI21_P0_HD21_CLK_HWM_SETTING_02_real_mode_mask
#define  HD21_CLK_HWM_SETTING_02_tr_weight_mask                         HDMI21_P0_HD21_CLK_HWM_SETTING_02_tr_weight_mask
#define  HD21_CLK_HWM_SETTING_02_no_tr_weight_mask                      HDMI21_P0_HD21_CLK_HWM_SETTING_02_no_tr_weight_mask
#define  HD21_CLK_HWM_SETTING_02_cont_err_th(data)                      HDMI21_P0_HD21_CLK_HWM_SETTING_02_cont_err_th(data)
#define  HD21_CLK_HWM_SETTING_02_real_mode(data)                        HDMI21_P0_HD21_CLK_HWM_SETTING_02_real_mode(data)
#define  HD21_CLK_HWM_SETTING_02_tr_weight(data)                        HDMI21_P0_HD21_CLK_HWM_SETTING_02_tr_weight(data)
#define  HD21_CLK_HWM_SETTING_02_no_tr_weight(data)                     HDMI21_P0_HD21_CLK_HWM_SETTING_02_no_tr_weight(data)
#define  HD21_CLK_HWM_SETTING_02_get_cont_err_th(data)                  HDMI21_P0_HD21_CLK_HWM_SETTING_02_get_cont_err_th(data)
#define  HD21_CLK_HWM_SETTING_02_get_real_mode(data)                    HDMI21_P0_HD21_CLK_HWM_SETTING_02_get_real_mode(data)
#define  HD21_CLK_HWM_SETTING_02_get_tr_weight(data)                    HDMI21_P0_HD21_CLK_HWM_SETTING_02_get_tr_weight(data)
#define  HD21_CLK_HWM_SETTING_02_get_no_tr_weight(data)                 HDMI21_P0_HD21_CLK_HWM_SETTING_02_get_no_tr_weight(data)


#define  HD21_CLK_HWN_ONE_ST_00_one_time_done_flag_mask                 HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_done_flag_mask
#define  HD21_CLK_HWN_ONE_ST_00_one_time_1_num_mask                     HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_1_num_mask
#define  HD21_CLK_HWN_ONE_ST_00_one_time_0_num_mask                     HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_0_num_mask
#define  HD21_CLK_HWN_ONE_ST_00_one_time_done_flag(data)                HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_done_flag(data)
#define  HD21_CLK_HWN_ONE_ST_00_one_time_1_num(data)                    HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_1_num(data)
#define  HD21_CLK_HWN_ONE_ST_00_one_time_0_num(data)                    HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_0_num(data)
#define  HD21_CLK_HWN_ONE_ST_00_get_one_time_done_flag(data)            HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_get_one_time_done_flag(data)
#define  HD21_CLK_HWN_ONE_ST_00_get_one_time_1_num(data)                HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_get_one_time_1_num(data)
#define  HD21_CLK_HWN_ONE_ST_00_get_one_time_0_num(data)                HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_get_one_time_0_num(data)


#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_going_flag_mask              HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_going_flag_mask
#define  HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask                  HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask
#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_err_cnt_mask                 HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_err_cnt_mask
#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_1_num_mask                   HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_1_num_mask
#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_going_flag(data)             HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_going_flag(data)
#define  HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag(data)                 HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag(data)
#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_err_cnt(data)                HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_err_cnt(data)
#define  HD21_CLK_HWN_CONT_ST_01_cont_mode_1_num(data)                  HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_mode_1_num(data)
#define  HD21_CLK_HWN_CONT_ST_01_get_cont_mode_going_flag(data)         HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_mode_going_flag(data)
#define  HD21_CLK_HWN_CONT_ST_01_get_cont_err_ov_flag(data)             HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_err_ov_flag(data)
#define  HD21_CLK_HWN_CONT_ST_01_get_cont_mode_err_cnt(data)            HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_mode_err_cnt(data)
#define  HD21_CLK_HWN_CONT_ST_01_get_cont_mode_1_num(data)              HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_mode_1_num(data)


#define  HD21_fapa_ct_tmp_all_mode_mask                                 HDMI21_P0_HD21_fapa_ct_tmp_all_mode_mask
#define  HD21_fapa_ct_tmp_sta_mask                                      HDMI21_P0_HD21_fapa_ct_tmp_sta_mask
#define  HD21_fapa_ct_tmp_all_mode(data)                                HDMI21_P0_HD21_fapa_ct_tmp_all_mode(data)
#define  HD21_fapa_ct_tmp_sta(data)                                     HDMI21_P0_HD21_fapa_ct_tmp_sta(data)
#define  HD21_fapa_ct_tmp_get_all_mode(data)                            HDMI21_P0_HD21_fapa_ct_tmp_get_all_mode(data)
#define  HD21_fapa_ct_tmp_get_sta(data)                                 HDMI21_P0_HD21_fapa_ct_tmp_get_sta(data)

#define  HD21_ps_ct_mac_out_en_mask                                     HDMI21_P0_HD21_ps_ct_mac_out_en_mask
#define  HD21_ps_ct_mac_on_sel_mask                                     HDMI21_P0_HD21_ps_ct_mac_on_sel_mask
#define  HD21_ps_ct_timer_cnt_end_mask                                  HDMI21_P0_HD21_ps_ct_timer_cnt_end_mask
#define  HD21_ps_ct_hdcp_mode_mask                                      HDMI21_P0_HD21_ps_ct_hdcp_mode_mask
#define  HD21_ps_ct_fw_mode_mask                                        HDMI21_P0_HD21_ps_ct_fw_mode_mask
#define  HD21_ps_ct_ps_mode_mask                                        HDMI21_P0_HD21_ps_ct_ps_mode_mask
#define  HD21_ps_ct_mac_out_en(data)                                    HDMI21_P0_HD21_ps_ct_mac_out_en(data)
#define  HD21_ps_ct_mac_on_sel(data)                                    HDMI21_P0_HD21_ps_ct_mac_on_sel(data)
#define  HD21_ps_ct_timer_cnt_end(data)                                 HDMI21_P0_HD21_ps_ct_timer_cnt_end(data)
#define  HD21_ps_ct_hdcp_mode(data)                                     HDMI21_P0_HD21_ps_ct_hdcp_mode(data)
#define  HD21_ps_ct_fw_mode(data)                                       HDMI21_P0_HD21_ps_ct_fw_mode(data)
#define  HD21_ps_ct_ps_mode(data)                                       HDMI21_P0_HD21_ps_ct_ps_mode(data)
#define  HD21_ps_ct_get_mac_out_en(data)                                HDMI21_P0_HD21_ps_ct_get_mac_out_en(data)
#define  HD21_ps_ct_get_mac_on_sel(data)                                HDMI21_P0_HD21_ps_ct_get_mac_on_sel(data)
#define  HD21_ps_ct_get_timer_cnt_end(data)                             HDMI21_P0_HD21_ps_ct_get_timer_cnt_end(data)
#define  HD21_ps_ct_get_hdcp_mode(data)                                 HDMI21_P0_HD21_ps_ct_get_hdcp_mode(data)
#define  HD21_ps_ct_get_fw_mode(data)                                   HDMI21_P0_HD21_ps_ct_get_fw_mode(data)
#define  HD21_ps_ct_get_ps_mode(data)                                   HDMI21_P0_HD21_ps_ct_get_ps_mode(data)

#define  HD21_ps_st_win_opp_detect_mask                                 HDMI21_P0_HD21_ps_st_win_opp_detect_mask
#define  HD21_ps_st_irq_en_pwron_mask                                   HDMI21_P0_HD21_ps_st_irq_en_pwron_mask
#define  HD21_ps_st_irq_en_pwroff_mask                                  HDMI21_P0_HD21_ps_st_irq_en_pwroff_mask
#define  HD21_ps_st_phy_power_on_flag_mask                              HDMI21_P0_HD21_ps_st_phy_power_on_flag_mask
#define  HD21_ps_st_pwron_mask                                          HDMI21_P0_HD21_ps_st_pwron_mask
#define  HD21_ps_st_pwroff_mask                                         HDMI21_P0_HD21_ps_st_pwroff_mask
#define  HD21_ps_st_win_opp_detect(data)                                HDMI21_P0_HD21_ps_st_win_opp_detect(data)
#define  HD21_ps_st_irq_en_pwron(data)                                  HDMI21_P0_HD21_ps_st_irq_en_pwron(data)
#define  HD21_ps_st_irq_en_pwroff(data)                                 HDMI21_P0_HD21_ps_st_irq_en_pwroff(data)
#define  HD21_ps_st_phy_power_on_flag(data)                             HDMI21_P0_HD21_ps_st_phy_power_on_flag(data)
#define  HD21_ps_st_pwron(data)                                         HDMI21_P0_HD21_ps_st_pwron(data)
#define  HD21_ps_st_pwroff(data)                                        HDMI21_P0_HD21_ps_st_pwroff(data)
#define  HD21_ps_st_get_win_opp_detect(data)                            HDMI21_P0_HD21_ps_st_get_win_opp_detect(data)
#define  HD21_ps_st_get_irq_en_pwron(data)                              HDMI21_P0_HD21_ps_st_get_irq_en_pwron(data)
#define  HD21_ps_st_get_irq_en_pwroff(data)                             HDMI21_P0_HD21_ps_st_get_irq_en_pwroff(data)
#define  HD21_ps_st_get_phy_power_on_flag(data)                         HDMI21_P0_HD21_ps_st_get_phy_power_on_flag(data)
#define  HD21_ps_st_get_pwron(data)                                     HDMI21_P0_HD21_ps_st_get_pwron(data)
#define  HD21_ps_st_get_pwroff(data)                                    HDMI21_P0_HD21_ps_st_get_pwroff(data)

#define  HD21_ps_measure_ctrl_en_mask                                   HDMI21_P0_hd21_ps_measure_ctrl_en_mask
#define  HD21_ps_measure_ctrl_en(data)                                  HDMI21_P0_hd21_ps_measure_ctrl_en(data)
#define  HD21_ps_measure_ctrl_get_en(data)                              HDMI21_P0_hd21_ps_measure_ctrl_get_en(data)

#define  HD21_ps_measure_xtal_clk0_vs2first_pixel_count_mask            HDMI21_P0_hd21_ps_measure_xtal_clk0_vs2first_pixel_count_mask
#define  HD21_ps_measure_xtal_clk0_vs2first_pixel_count(data)           HDMI21_P0_hd21_ps_measure_xtal_clk0_vs2first_pixel_count(data)
#define  HD21_ps_measure_xtal_clk0_get_vs2first_pixel_count(data)       HDMI21_P0_hd21_ps_measure_xtal_clk0_get_vs2first_pixel_count(data)
                                                                        
#define  HD21_ps_measure_xtal_clk1_vs2last_pixel_count_mask             HDMI21_P0_hd21_ps_measure_xtal_clk1_vs2last_pixel_count_mask
#define  HD21_ps_measure_xtal_clk1_vs2last_pixel_count(data)            HDMI21_P0_hd21_ps_measure_xtal_clk1_vs2last_pixel_count(data)
#define  HD21_ps_measure_xtal_clk1_get_vs2last_pixel_count(data)        HDMI21_P0_hd21_ps_measure_xtal_clk1_get_vs2last_pixel_count(data)
                                                                        
#define  HD21_ps_measure_xtal_clk2_vs2vs_count_mask                     HDMI21_P0_hd21_ps_measure_xtal_clk2_vs2vs_count_mask
#define  HD21_ps_measure_xtal_clk2_vs2vs_count(data)                    HDMI21_P0_hd21_ps_measure_xtal_clk2_vs2vs_count(data)
#define  HD21_ps_measure_xtal_clk2_get_vs2vs_count(data)                HDMI21_P0_hd21_ps_measure_xtal_clk2_get_vs2vs_count(data)

#define  HD21_HDMI_PA_IRQ_EN_hfvs_absence_irq_en_mask                   HDMI21_P0_HD21_HDMI_PA_IRQ_EN_hfvs_absence_irq_en_mask
#define  HD21_HDMI_PA_IRQ_EN_hfvs_absence_irq_en(data)                  HDMI21_P0_HD21_HDMI_PA_IRQ_EN_hfvs_absence_irq_en(data)
#define  HD21_HDMI_PA_IRQ_EN_get_hfvs_absence_irq_en(data)              HDMI21_P0_HD21_HDMI_PA_IRQ_EN_get_hfvs_absence_irq_en(data)

#define  HD21_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta_mask                 HDMI21_P0_HD21_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta_mask
#define  HD21_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta(data)                HDMI21_P0_HD21_HDMI_PA_IRQ_STA_hfvs_absence_irq_sta(data)
#define  HD21_HDMI_PA_IRQ_STA_get_hfvs_absence_irq_sta(data)            HDMI21_P0_HD21_HDMI_PA_IRQ_STA_get_hfvs_absence_irq_sta(data)

#define  HD21_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt_mask             HDMI21_P0_HD21_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt_mask
#define  HD21_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt(data)            HDMI21_P0_HD21_HDMI_PA_FRAME_CNT_hfvs_absence_frame_cnt(data)
#define  HD21_HDMI_PA_FRAME_CNT_get_hfvs_absence_frame_cnt(data)        HDMI21_P0_HD21_HDMI_PA_FRAME_CNT_get_hfvs_absence_frame_cnt(data)

// Measure

#define HDMI21_MEAS0_BASE           (HDMI21_P0_MEAS_FRL_MEASURE_0_reg)
#define HDMI21_MEAS1_BASE           (HDMI21_P0_MEAS_FRL_MEASURE_0_reg+0x2000)

struct hdmi21_meas_reg_st
{
    uint32_t  frl_measure_0;    // 00
    uint32_t  frl_measure_1;    // 04
    uint32_t  frl_measure_3;    // 0C
    uint32_t  frl_measure_4;    // 10
    uint32_t  frl_measure_5;    // 14
    uint32_t  frl_measure_7;    // 1C
    uint32_t  frl_measure_9;    // 24
    uint32_t  frl_measure_a;    // 28
    uint32_t  frl_measure_b;    // 2C
    uint32_t  frl_measure_d;    // 34
    uint32_t  frl_measure_f;    // 3C
    uint32_t  frl_measure_11;    // 44
    uint32_t  frl_measure_13;    // 4C
    uint32_t  frl_measure_14;    // 50
    uint32_t  frl_measure_16;    // 58
    uint32_t  frl_measure_18;    // 64
};

extern const struct hdmi21_meas_reg_st hd21_meas[HDMI21_MAC_NUM];

#define  HD21_MEAS_FRL_MEASURE_0_reg                                    (hd21_meas[nport].frl_measure_0)
#define  HD21_MEAS_FRL_MEASURE_1_reg                                    (hd21_meas[nport].frl_measure_1)
#define  HD21_MEAS_FRL_MEASURE_3_reg                                    (hd21_meas[nport].frl_measure_3)
#define  HD21_MEAS_FRL_MEASURE_4_reg                                    (hd21_meas[nport].frl_measure_4)
#define  HD21_MEAS_FRL_MEASURE_5_reg                                    (hd21_meas[nport].frl_measure_5)
#define  HD21_MEAS_FRL_MEASURE_7_reg                                    (hd21_meas[nport].frl_measure_7)
#define  HD21_MEAS_FRL_MEASURE_9_reg                                    (hd21_meas[nport].frl_measure_9)
#define  HD21_MEAS_FRL_MEASURE_A_reg                                    (hd21_meas[nport].frl_measure_a)
#define  HD21_MEAS_FRL_MEASURE_B_reg                                    (hd21_meas[nport].frl_measure_b)
#define  HD21_MEAS_FRL_MEASURE_D_reg                                    (hd21_meas[nport].frl_measure_d)
#define  HD21_MEAS_FRL_MEASURE_F_reg                                    (hd21_meas[nport].frl_measure_f)
#define  HD21_MEAS_FRL_MEASURE_11_reg                                   (hd21_meas[nport].frl_measure_11)
#define  HD21_MEAS_FRL_MEASURE_13_reg                                   (hd21_meas[nport].frl_measure_13)
#define  HD21_MEAS_FRL_MEASURE_14_reg                                   (hd21_meas[nport].frl_measure_14)
#define  HD21_MEAS_FRL_MEASURE_16_reg                                   (hd21_meas[nport].frl_measure_16)
#define  HD21_MEAS_FRL_MEASURE_18_reg                                   (hd21_meas[nport].frl_measure_18)

#define  HD21_MEAS_FRL_MEASURE_0_measure_en_mask               HDMI21_P0_MEAS_FRL_MEASURE_0_measure_en_mask
#define  HD21_MEAS_FRL_MEASURE_0_dummy_6_0_mask                HDMI21_P0_MEAS_FRL_MEASURE_0_dummy_6_0_mask
#define  HD21_MEAS_FRL_MEASURE_0_measure_en(data)              HDMI21_P0_MEAS_FRL_MEASURE_0_measure_en(data)
#define  HD21_MEAS_FRL_MEASURE_0_dummy_6_0(data)               HDMI21_P0_MEAS_FRL_MEASURE_0_dummy_6_0(data)
#define  HD21_MEAS_FRL_MEASURE_0_get_measure_en(data)          HDMI21_P0_MEAS_FRL_MEASURE_0_get_measure_en(data)
#define  HD21_MEAS_FRL_MEASURE_0_get_dummy_6_0(data)           HDMI21_P0_MEAS_FRL_MEASURE_0_get_dummy_6_0(data)


#define  HD21_MEAS_FRL_MEASURE_1_ht_clk_cnt_mask               HDMI21_P0_MEAS_FRL_MEASURE_1_ht_clk_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_1_ht_clk_cnt(data)              HDMI21_P0_MEAS_FRL_MEASURE_1_ht_clk_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_1_get_ht_clk_cnt(data)          HDMI21_P0_MEAS_FRL_MEASURE_1_get_ht_clk_cnt(data)

#define  HD21_MEAS_FRL_MEASURE_3_hs_sel_mask                   HDMI21_P0_MEAS_FRL_MEASURE_3_hs_sel_mask
#define  HD21_MEAS_FRL_MEASURE_3_ht_meas_sel_mask              HDMI21_P0_MEAS_FRL_MEASURE_3_ht_meas_sel_mask
#define  HD21_MEAS_FRL_MEASURE_3_meas_skip_line_mask           HDMI21_P0_MEAS_FRL_MEASURE_3_meas_skip_line_mask
#define  HD21_MEAS_FRL_MEASURE_3_avg_line_num_mask             HDMI21_P0_MEAS_FRL_MEASURE_3_avg_line_num_mask
#define  HD21_MEAS_FRL_MEASURE_3_dummy_0_mask                  HDMI21_P0_MEAS_FRL_MEASURE_3_dummy_0_mask
#define  HD21_MEAS_FRL_MEASURE_3_hs_sel(data)                  HDMI21_P0_MEAS_FRL_MEASURE_3_hs_sel(data)
#define  HD21_MEAS_FRL_MEASURE_3_ht_meas_sel(data)             HDMI21_P0_MEAS_FRL_MEASURE_3_ht_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_3_meas_skip_line(data)          HDMI21_P0_MEAS_FRL_MEASURE_3_meas_skip_line(data)
#define  HD21_MEAS_FRL_MEASURE_3_avg_line_num(data)            HDMI21_P0_MEAS_FRL_MEASURE_3_avg_line_num(data)
#define  HD21_MEAS_FRL_MEASURE_3_dummy_0(data)                 HDMI21_P0_MEAS_FRL_MEASURE_3_dummy_0(data)
#define  HD21_MEAS_FRL_MEASURE_3_get_hs_sel(data)              HDMI21_P0_MEAS_FRL_MEASURE_3_get_hs_sel(data)
#define  HD21_MEAS_FRL_MEASURE_3_get_ht_meas_sel(data)         HDMI21_P0_MEAS_FRL_MEASURE_3_get_ht_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_3_get_meas_skip_line(data)      HDMI21_P0_MEAS_FRL_MEASURE_3_get_meas_skip_line(data)
#define  HD21_MEAS_FRL_MEASURE_3_get_avg_line_num(data)        HDMI21_P0_MEAS_FRL_MEASURE_3_get_avg_line_num(data)
#define  HD21_MEAS_FRL_MEASURE_3_get_dummy_0(data)             HDMI21_P0_MEAS_FRL_MEASURE_3_get_dummy_0(data)


#define  HD21_MEAS_FRL_MEASURE_4_line_bd_sel_mask              HDMI21_P0_MEAS_FRL_MEASURE_4_line_bd_sel_mask
#define  HD21_MEAS_FRL_MEASURE_4_vt_meas_sel_mask              HDMI21_P0_MEAS_FRL_MEASURE_4_vt_meas_sel_mask
#define  HD21_MEAS_FRL_MEASURE_4_vact_meas_sel_mask            HDMI21_P0_MEAS_FRL_MEASURE_4_vact_meas_sel_mask
#define  HD21_MEAS_FRL_MEASURE_4_dummy_3_0_mask                HDMI21_P0_MEAS_FRL_MEASURE_4_dummy_3_0_mask
#define  HD21_MEAS_FRL_MEASURE_4_line_bd_sel(data)             HDMI21_P0_MEAS_FRL_MEASURE_4_line_bd_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_vt_meas_sel(data)             HDMI21_P0_MEAS_FRL_MEASURE_4_vt_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_vact_meas_sel(data)           HDMI21_P0_MEAS_FRL_MEASURE_4_vact_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_dummy_3_0(data)               HDMI21_P0_MEAS_FRL_MEASURE_4_dummy_3_0(data)
#define  HD21_MEAS_FRL_MEASURE_4_get_line_bd_sel(data)         HDMI21_P0_MEAS_FRL_MEASURE_4_get_line_bd_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_get_vt_meas_sel(data)         HDMI21_P0_MEAS_FRL_MEASURE_4_get_vt_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_get_vact_meas_sel(data)       HDMI21_P0_MEAS_FRL_MEASURE_4_get_vact_meas_sel(data)
#define  HD21_MEAS_FRL_MEASURE_4_get_dummy_3_0(data)           HDMI21_P0_MEAS_FRL_MEASURE_4_get_dummy_3_0(data)


#define  HD21_MEAS_FRL_MEASURE_5_vt_line_cnt1_mask             HDMI21_P0_MEAS_FRL_MEASURE_5_vt_line_cnt1_mask
#define  HD21_MEAS_FRL_MEASURE_5_vt_line_cnt1(data)            HDMI21_P0_MEAS_FRL_MEASURE_5_vt_line_cnt1(data)
#define  HD21_MEAS_FRL_MEASURE_5_get_vt_line_cnt1(data)        HDMI21_P0_MEAS_FRL_MEASURE_5_get_vt_line_cnt1(data)


#define  HD21_MEAS_FRL_MEASURE_7_vt_line_cnt2_mask             HDMI21_P0_MEAS_FRL_MEASURE_7_vt_line_cnt2_mask
#define  HD21_MEAS_FRL_MEASURE_7_vt_line_cnt2(data)            HDMI21_P0_MEAS_FRL_MEASURE_7_vt_line_cnt2(data)
#define  HD21_MEAS_FRL_MEASURE_7_get_vt_line_cnt2(data)        HDMI21_P0_MEAS_FRL_MEASURE_7_get_vt_line_cnt2(data)


#define  HD21_MEAS_FRL_MEASURE_9_pol_det_mode_mask             HDMI21_P0_MEAS_FRL_MEASURE_9_pol_det_mode_mask
#define  HD21_MEAS_FRL_MEASURE_9_hs_polarity_mask              HDMI21_P0_MEAS_FRL_MEASURE_9_hs_polarity_mask
#define  HD21_MEAS_FRL_MEASURE_9_vs_polarity_mask              HDMI21_P0_MEAS_FRL_MEASURE_9_vs_polarity_mask
#define  HD21_MEAS_FRL_MEASURE_9_dummy_4_0_mask                HDMI21_P0_MEAS_FRL_MEASURE_9_dummy_4_0_mask
#define  HD21_MEAS_FRL_MEASURE_9_pol_det_mode(data)            HDMI21_P0_MEAS_FRL_MEASURE_9_pol_det_mode(data)
#define  HD21_MEAS_FRL_MEASURE_9_hs_polarity(data)             HDMI21_P0_MEAS_FRL_MEASURE_9_hs_polarity(data)
#define  HD21_MEAS_FRL_MEASURE_9_vs_polarity(data)             HDMI21_P0_MEAS_FRL_MEASURE_9_vs_polarity(data)
#define  HD21_MEAS_FRL_MEASURE_9_dummy_4_0(data)               HDMI21_P0_MEAS_FRL_MEASURE_9_dummy_4_0(data)
#define  HD21_MEAS_FRL_MEASURE_9_get_pol_det_mode(data)        HDMI21_P0_MEAS_FRL_MEASURE_9_get_pol_det_mode(data)
#define  HD21_MEAS_FRL_MEASURE_9_get_hs_polarity(data)         HDMI21_P0_MEAS_FRL_MEASURE_9_get_hs_polarity(data)
#define  HD21_MEAS_FRL_MEASURE_9_get_vs_polarity(data)         HDMI21_P0_MEAS_FRL_MEASURE_9_get_vs_polarity(data)
#define  HD21_MEAS_FRL_MEASURE_9_get_dummy_4_0(data)           HDMI21_P0_MEAS_FRL_MEASURE_9_get_dummy_4_0(data)


#define  HD21_MEAS_FRL_MEASURE_A_interlaced_vtotal_mask        HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_vtotal_mask
#define  HD21_MEAS_FRL_MEASURE_A_vtotal_diff_mask              HDMI21_P0_MEAS_FRL_MEASURE_A_vtotal_diff_mask
#define  HD21_MEAS_FRL_MEASURE_A_field_tog_mask                HDMI21_P0_MEAS_FRL_MEASURE_A_field_tog_mask
#define  HD21_MEAS_FRL_MEASURE_A_interlaced_field_mask         HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_field_mask
#define  HD21_MEAS_FRL_MEASURE_A_interlaced_err_mask           HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_err_mask
#define  HD21_MEAS_FRL_MEASURE_A_dummy_1_0_mask                HDMI21_P0_MEAS_FRL_MEASURE_A_dummy_1_0_mask
#define  HD21_MEAS_FRL_MEASURE_A_interlaced_vtotal(data)       HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_vtotal(data)
#define  HD21_MEAS_FRL_MEASURE_A_vtotal_diff(data)             HDMI21_P0_MEAS_FRL_MEASURE_A_vtotal_diff(data)
#define  HD21_MEAS_FRL_MEASURE_A_field_tog(data)               HDMI21_P0_MEAS_FRL_MEASURE_A_field_tog(data)
#define  HD21_MEAS_FRL_MEASURE_A_interlaced_field(data)        HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_field(data)
#define  HD21_MEAS_FRL_MEASURE_A_interlaced_err(data)          HDMI21_P0_MEAS_FRL_MEASURE_A_interlaced_err(data)
#define  HD21_MEAS_FRL_MEASURE_A_dummy_1_0(data)               HDMI21_P0_MEAS_FRL_MEASURE_A_dummy_1_0(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_interlaced_vtotal(data)   HDMI21_P0_MEAS_FRL_MEASURE_A_get_interlaced_vtotal(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_vtotal_diff(data)         HDMI21_P0_MEAS_FRL_MEASURE_A_get_vtotal_diff(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_field_tog(data)           HDMI21_P0_MEAS_FRL_MEASURE_A_get_field_tog(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_interlaced_field(data)    HDMI21_P0_MEAS_FRL_MEASURE_A_get_interlaced_field(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_interlaced_err(data)      HDMI21_P0_MEAS_FRL_MEASURE_A_get_interlaced_err(data)
#define  HD21_MEAS_FRL_MEASURE_A_get_dummy_1_0(data)           HDMI21_P0_MEAS_FRL_MEASURE_A_get_dummy_1_0(data)


#define  HD21_MEAS_FRL_MEASURE_B_hs_width_mask                 HDMI21_P0_MEAS_FRL_MEASURE_B_hs_width_mask
#define  HD21_MEAS_FRL_MEASURE_B_hs_width(data)                HDMI21_P0_MEAS_FRL_MEASURE_B_hs_width(data)
#define  HD21_MEAS_FRL_MEASURE_B_get_hs_width(data)            HDMI21_P0_MEAS_FRL_MEASURE_B_get_hs_width(data)


#define  HD21_MEAS_FRL_MEASURE_D_hback_cnt_mask                HDMI21_P0_MEAS_FRL_MEASURE_D_hback_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_D_hback_cnt(data)               HDMI21_P0_MEAS_FRL_MEASURE_D_hback_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_D_get_hback_cnt(data)           HDMI21_P0_MEAS_FRL_MEASURE_D_get_hback_cnt(data)


#define  HD21_MEAS_FRL_MEASURE_F_hact_cnt_mask                 HDMI21_P0_MEAS_FRL_MEASURE_F_hact_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_F_hact_cnt(data)                HDMI21_P0_MEAS_FRL_MEASURE_F_hact_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_F_get_hact_cnt(data)            HDMI21_P0_MEAS_FRL_MEASURE_F_get_hact_cnt(data)


#define  HD21_MEAS_FRL_MEASURE_11_htotal_cnt_mask              HDMI21_P0_MEAS_FRL_MEASURE_11_htotal_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_11_htotal_cnt(data)             HDMI21_P0_MEAS_FRL_MEASURE_11_htotal_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_11_get_htotal_cnt(data)         HDMI21_P0_MEAS_FRL_MEASURE_11_get_htotal_cnt(data)


#define  HD21_MEAS_FRL_MEASURE_13_vs_width_mask                HDMI21_P0_MEAS_FRL_MEASURE_13_vs_width_mask
#define  HD21_MEAS_FRL_MEASURE_13_vs_width(data)               HDMI21_P0_MEAS_FRL_MEASURE_13_vs_width(data)
#define  HD21_MEAS_FRL_MEASURE_13_get_vs_width(data)           HDMI21_P0_MEAS_FRL_MEASURE_13_get_vs_width(data)


#define  HD21_MEAS_FRL_MEASURE_14_vback_cnt_mask               HDMI21_P0_MEAS_FRL_MEASURE_14_vback_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_14_vback_cnt(data)              HDMI21_P0_MEAS_FRL_MEASURE_14_vback_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_14_get_vback_cnt(data)          HDMI21_P0_MEAS_FRL_MEASURE_14_get_vback_cnt(data)


#define  HD21_MEAS_FRL_MEASURE_16_vs_hs_rp_align_mask          HDMI21_P0_MEAS_FRL_MEASURE_16_vs_hs_rp_align_mask
#define  HD21_MEAS_FRL_MEASURE_16_vs_hs_fp_align_mask          HDMI21_P0_MEAS_FRL_MEASURE_16_vs_hs_fp_align_mask
#define  HD21_MEAS_FRL_MEASURE_16_last_line_ov_thd_mask        HDMI21_P0_MEAS_FRL_MEASURE_16_last_line_ov_thd_mask
#define  HD21_MEAS_FRL_MEASURE_16_vact_cnt_mask                HDMI21_P0_MEAS_FRL_MEASURE_16_vact_cnt_mask
#define  HD21_MEAS_FRL_MEASURE_16_vs_hs_rp_align(data)         HDMI21_P0_MEAS_FRL_MEASURE_16_vs_hs_rp_align(data)
#define  HD21_MEAS_FRL_MEASURE_16_vs_hs_fp_align(data)         HDMI21_P0_MEAS_FRL_MEASURE_16_vs_hs_fp_align(data)
#define  HD21_MEAS_FRL_MEASURE_16_last_line_ov_thd(data)       HDMI21_P0_MEAS_FRL_MEASURE_16_last_line_ov_thd(data)
#define  HD21_MEAS_FRL_MEASURE_16_vact_cnt(data)               HDMI21_P0_MEAS_FRL_MEASURE_16_vact_cnt(data)
#define  HD21_MEAS_FRL_MEASURE_16_get_vs_hs_rp_align(data)     HDMI21_P0_MEAS_FRL_MEASURE_16_get_vs_hs_rp_align(data)
#define  HD21_MEAS_FRL_MEASURE_16_get_vs_hs_fp_align(data)     HDMI21_P0_MEAS_FRL_MEASURE_16_get_vs_hs_fp_align(data)
#define  HD21_MEAS_FRL_MEASURE_16_get_last_line_ov_thd(data)   HDMI21_P0_MEAS_FRL_MEASURE_16_get_last_line_ov_thd(data)
#define  HD21_MEAS_FRL_MEASURE_16_get_vact_cnt(data)           HDMI21_P0_MEAS_FRL_MEASURE_16_get_vact_cnt(data)

#define  HD21_MEAS_FRL_MEASURE_18_hs_period_mask               HDMI21_P0_MEAS_FRL_MEASURE_18_hs_period_mask
#define  HD21_MEAS_FRL_MEASURE_18_hs_period(data)              HDMI21_P0_MEAS_FRL_MEASURE_18_hs_period(data)
#define  HD21_MEAS_FRL_MEASURE_18_get_hs_period(data)          HDMI21_P0_MEAS_FRL_MEASURE_18_get_hs_period(data)

//---------------------------------------------------------------------------------
// HDMI2.1 FEC
//---------------------------------------------------------------------------------

#define HDMI21_FEC0_BASE           (HDMI21_P0_FEC_FEC_HDMI21_RS_0_reg)
#define HDMI21_FEC1_BASE           (HDMI21_P0_FEC_FEC_HDMI21_RS_0_reg+0x2000)

struct hdmi21_fec_reg_st
{
    uint32_t  hdmi21_rs_0;    // 00
    uint32_t  hdmi21_rs_1;    // 04
    uint32_t  hdmi21_rs_2;    // 08
    uint32_t  hdmi21_rs_3;    // 0C

    uint32_t  hdmi21_fw_rs_0;    // 10
    uint32_t  hdmi21_fw_rs_1;    // 14
    uint32_t  hdmi21_fw_rs_2;    // 18
    uint32_t  hdmi21_fw_rs_3;    // 1C
    uint32_t  hdmi21_fw_rs_4;    // 20
    uint32_t  hdmi21_fw_rs_5;    // 24
    uint32_t  hdmi21_fw_rs_6;    // 28

    uint32_t  hdmi21_lfsr_1;    // 2C
    uint32_t  hdmi21_lfsr_2;    // 30
    uint32_t  hdmi21_lfsr_3;    // 34
    uint32_t  hdmi21_lfsr_4;    // 38
    uint32_t  hdmi21_lfsr_5;    // 3C
    uint32_t  hdmi21_lfsr_6;    // 40
    uint32_t  hdmi21_lfsr_7;    // 44
    uint32_t  hdmi21_lfsr_8;    // 48
    uint32_t  hdmi21_lfsr_9;    // 4C
    uint32_t  hdmi21_lfsr_10;    // 50
    uint32_t  hdmi21_lfsr_11;    // 54

    uint32_t  hdmi21_di_0;    // 58

    uint32_t  hdmi21_rs_sram_0;    // 5C
    uint32_t  hdmi21_rs_sram_1;    // 60
    uint32_t  hdmi21_rs_sram_2;    // 64
    uint32_t  hdmi21_rs_sram_bist_0;    // 6C
    uint32_t  hdmi21_rs_sram_bist_1;    // 70
    uint32_t  hdmi21_rs_sram_bist_2;    // 74

    uint32_t  hdmi21_rs_read_margin ;   // 78
    uint32_t  hdmi21_rs_read_margin_enable;    // 7C

    uint32_t  hdmi21_frl_0;    // 80
    uint32_t  hdmi21_frl_1;    // 84
    uint32_t  hdmi21_frl_D1;    // 88
    uint32_t  hdmi21_frl_D2;    // 8C

    uint32_t  hdmi21_test_sel;    // 90

    uint32_t  hdmi21_lt_9;    // 94

    uint32_t  port_page73_hdmi_scr;    // 98
    uint32_t  hdmi_sr;    // 9C
    uint32_t  port_page73_hdmi_bcsr;    // A0
};

extern const struct hdmi21_fec_reg_st hd21_fec[HDMI21_MAC_NUM];

// FEC
#define  HD21_FEC_FEC_HDMI21_RS_0_reg                                   (hd21_fec[nport].hdmi21_rs_0)
#define  HD21_FEC_FEC_HDMI21_RS_1_reg                                   (hd21_fec[nport].hdmi21_rs_1)
#define  HD21_FEC_FEC_HDMI21_RS_2_reg                                   (hd21_fec[nport].hdmi21_rs_2)
#define  HD21_FEC_FEC_HDMI21_RS_3_reg                                   (hd21_fec[nport].hdmi21_rs_3)
#define  HD21_FEC_FEC_HDMI21_FW_RS_0_reg                                (hd21_fec[nport].hdmi21_fw_rs_0)
#define  HD21_FEC_FEC_HDMI21_FW_RS_1_reg                                (hd21_fec[nport].hdmi21_fw_rs_1)
#define  HD21_FEC_FEC_HDMI21_FW_RS_2_reg                                (hd21_fec[nport].hdmi21_fw_rs_2)
#define  HD21_FEC_FEC_HDMI21_FW_RS_3_reg                                (hd21_fec[nport].hdmi21_fw_rs_3)
#define  HD21_FEC_FEC_HDMI21_FW_RS_4_reg                                (hd21_fec[nport].hdmi21_fw_rs_4)
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_reg                                (hd21_fec[nport].hdmi21_fw_rs_5)
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_reg                                (hd21_fec[nport].hdmi21_fw_rs_6)
#define  HD21_FEC_FEC_HDMI21_LFSR_1_reg                                 (hd21_fec[nport].hdmi21_lfsr_1)
#define  HD21_FEC_FEC_HDMI21_LFSR_2_reg                                 (hd21_fec[nport].hdmi21_lfsr_2)
#define  HD21_FEC_FEC_HDMI21_LFSR_3_reg                                 (hd21_fec[nport].hdmi21_lfsr_3)
#define  HD21_FEC_FEC_HDMI21_LFSR_4_reg                                 (hd21_fec[nport].hdmi21_lfsr_4)
#define  HD21_FEC_FEC_HDMI21_LFSR_5_reg                                 (hd21_fec[nport].hdmi21_lfsr_5)
#define  HD21_FEC_FEC_HDMI21_LFSR_6_reg                                 (hd21_fec[nport].hdmi21_lfsr_6)
#define  HD21_FEC_FEC_HDMI21_LFSR_7_reg                                 (hd21_fec[nport].hdmi21_lfsr_7)
#define  HD21_FEC_FEC_HDMI21_LFSR_8_reg                                 (hd21_fec[nport].hdmi21_lfsr_8)
#define  HD21_FEC_FEC_HDMI21_LFSR_9_reg                                 (hd21_fec[nport].hdmi21_lfsr_9)
#define  HD21_FEC_FEC_HDMI21_LFSR_10_reg                                (hd21_fec[nport].hdmi21_lfsr_10)
#define  HD21_FEC_FEC_HDMI21_LFSR_11_reg                                (hd21_fec[nport].hdmi21_lfsr_11)
#define  HD21_FEC_FEC_HDMI21_DI_0_reg                                   (hd21_fec[nport].hdmi21_di_0)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_reg                              (hd21_fec[nport].hdmi21_rs_sram_0)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_1_reg                              (hd21_fec[nport].hdmi21_rs_sram_1)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_2_reg                              (hd21_fec[nport].hdmi21_rs_sram_2)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_reg                         (hd21_fec[nport].hdmi21_rs_sram_bist_0)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_reg                         (hd21_fec[nport].hdmi21_rs_sram_bist_1)
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_reg                         (hd21_fec[nport].hdmi21_rs_sram_bist_2)
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_reg                         (hd21_fec[nport].hdmi21_rs_read_margin)
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_reg                  (hd21_fec[nport].hdmi21_rs_read_margin_enable)
#define  HD21_FEC_FEC_HDMI21_FRL_0_reg                                  (hd21_fec[nport].hdmi21_frl_0)
#define  HD21_FEC_FEC_HDMI21_FRL_1_reg                                  (hd21_fec[nport].hdmi21_frl_1)
#define  HD21_FEC_FEC_HDMI21_FRL_D1_reg                                 (hd21_fec[nport].hdmi21_frl_d1)
#define  HD21_FEC_FEC_HDMI21_FRL_D2_reg                                 (hd21_fec[nport].hdmi21_frl_d2)
#define  HD21_FEC_FEC_HDMI21_TEST_SEL_reg                               (hd21_fec[nport].hdmi21_test_sel)
#define  HD21_FEC_FEC_HDMI21_LT_9_reg                                   (hd21_fec[nport].hdmi21_lt_9)
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg                          (hd21_fec[nport].port_page73_hdmi_scr)
#define  HD21_FEC_FEC_HDMI_SR_reg                                       (hd21_fec[nport].hdmi_sr)
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_reg                         (hd21_fec[nport].port_page73_hdmi_bcsr)

#define  HD21_FEC_FEC_HDMI21_RS_0_rs_dc_en_mask                           HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_dc_en_mask                             
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_bypass_mask                          HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_bypass_mask                            
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_en_mask                          HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_en_mask                            
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_keep_mask                        HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_keep_mask                          
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_valid_mask                       HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_valid_mask                         
#define  HD21_FEC_FEC_HDMI21_RS_0_dummy_2_0_mask                          HDMI21_P0_FEC_FEC_HDMI21_RS_0_dummy_2_0_mask                            
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_dc_en(data)                          HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_dc_en(data)                            
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_bypass(data)                         HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_bypass(data)                           
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_en(data)                         HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_en(data)                           
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_keep(data)                       HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_keep(data)                         
#define  HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_valid(data)                      HDMI21_P0_FEC_FEC_HDMI21_RS_0_rs_cnt_valid(data)                        
#define  HD21_FEC_FEC_HDMI21_RS_0_dummy_2_0(data)                         HDMI21_P0_FEC_FEC_HDMI21_RS_0_dummy_2_0(data)                           
#define  HD21_FEC_FEC_HDMI21_RS_0_get_rs_dc_en(data)                      HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_rs_dc_en(data)                        
#define  HD21_FEC_FEC_HDMI21_RS_0_get_rs_bypass(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_rs_bypass(data)                       
#define  HD21_FEC_FEC_HDMI21_RS_0_get_rs_cnt_en(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_rs_cnt_en(data)                       
#define  HD21_FEC_FEC_HDMI21_RS_0_get_rs_cnt_keep(data)                   HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_rs_cnt_keep(data)                     
#define  HD21_FEC_FEC_HDMI21_RS_0_get_rs_cnt_valid(data)                  HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_rs_cnt_valid(data)                    
#define  HD21_FEC_FEC_HDMI21_RS_0_get_dummy_2_0(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_0_get_dummy_2_0(data)                       
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_1_rs_clk_1s_sel_mask                      HDMI21_P0_FEC_FEC_HDMI21_RS_1_rs_clk_1s_sel_mask                        
#define  HD21_FEC_FEC_HDMI21_RS_1_rs_clk_1m_div_mask                      HDMI21_P0_FEC_FEC_HDMI21_RS_1_rs_clk_1m_div_mask                        
#define  HD21_FEC_FEC_HDMI21_RS_1_dummy_2_0_mask                          HDMI21_P0_FEC_FEC_HDMI21_RS_1_dummy_2_0_mask                            
#define  HD21_FEC_FEC_HDMI21_RS_1_rs_clk_1s_sel(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_1_rs_clk_1s_sel(data)                       
#define  HD21_FEC_FEC_HDMI21_RS_1_rs_clk_1m_div(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_1_rs_clk_1m_div(data)                       
#define  HD21_FEC_FEC_HDMI21_RS_1_dummy_2_0(data)                         HDMI21_P0_FEC_FEC_HDMI21_RS_1_dummy_2_0(data)                           
#define  HD21_FEC_FEC_HDMI21_RS_1_get_rs_clk_1s_sel(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_1_get_rs_clk_1s_sel(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_1_get_rs_clk_1m_div(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_1_get_rs_clk_1m_div(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_1_get_dummy_2_0(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_1_get_dummy_2_0(data)                       
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_2_dummy_7_0_mask                          HDMI21_P0_FEC_FEC_HDMI21_RS_2_dummy_7_0_mask                            
#define  HD21_FEC_FEC_HDMI21_RS_2_dummy_7_0(data)                         HDMI21_P0_FEC_FEC_HDMI21_RS_2_dummy_7_0(data)                           
#define  HD21_FEC_FEC_HDMI21_RS_2_get_dummy_7_0(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_2_get_dummy_7_0(data)                       
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_en_mask                        HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_en_mask                          
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_num_mask                       HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_num_mask                         
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_once_mask                      HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_once_mask                        
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_ie_mask                        HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_ie_mask                          
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_mask                           HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_mask                             
#define  HD21_FEC_FEC_HDMI21_RS_3_dummy_0_mask                            HDMI21_P0_FEC_FEC_HDMI21_RS_3_dummy_0_mask                              
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_en(data)                       HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_en(data)                         
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_num(data)                      HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_num(data)                        
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_once(data)                     HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_once(data)                       
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_ie(data)                       HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd_ie(data)                         
#define  HD21_FEC_FEC_HDMI21_RS_3_rsed_upd(data)                          HDMI21_P0_FEC_FEC_HDMI21_RS_3_rsed_upd(data)                            
#define  HD21_FEC_FEC_HDMI21_RS_3_dummy_0(data)                           HDMI21_P0_FEC_FEC_HDMI21_RS_3_dummy_0(data)                             
#define  HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd_en(data)                   HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_rsed_upd_en(data)                     
#define  HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd_num(data)                  HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_rsed_upd_num(data)                    
#define  HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd_once(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_rsed_upd_once(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd_ie(data)                   HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_rsed_upd_ie(data)                     
#define  HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd(data)                      HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_rsed_upd(data)                        
#define  HD21_FEC_FEC_HDMI21_RS_3_get_dummy_0(data)                       HDMI21_P0_FEC_FEC_HDMI21_RS_3_get_dummy_0(data)                         
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_0_fw_rs_corr_cnt_l0_mask               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_0_fw_rs_corr_cnt_l0_mask                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_0_fw_rs_corr_cnt_l0(data)              HDMI21_P0_FEC_FEC_HDMI21_FW_RS_0_fw_rs_corr_cnt_l0(data)                
#define  HD21_FEC_FEC_HDMI21_FW_RS_0_get_fw_rs_corr_cnt_l0(data)          HDMI21_P0_FEC_FEC_HDMI21_FW_RS_0_get_fw_rs_corr_cnt_l0(data)            
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_1_fw_rs_corr_cnt_l1_mask               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_1_fw_rs_corr_cnt_l1_mask                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_1_fw_rs_corr_cnt_l1(data)              HDMI21_P0_FEC_FEC_HDMI21_FW_RS_1_fw_rs_corr_cnt_l1(data)                
#define  HD21_FEC_FEC_HDMI21_FW_RS_1_get_fw_rs_corr_cnt_l1(data)          HDMI21_P0_FEC_FEC_HDMI21_FW_RS_1_get_fw_rs_corr_cnt_l1(data)            
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_2_fw_rs_corr_cnt_l2_mask               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_2_fw_rs_corr_cnt_l2_mask                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_2_fw_rs_corr_cnt_l2(data)              HDMI21_P0_FEC_FEC_HDMI21_FW_RS_2_fw_rs_corr_cnt_l2(data)                
#define  HD21_FEC_FEC_HDMI21_FW_RS_2_get_fw_rs_corr_cnt_l2(data)          HDMI21_P0_FEC_FEC_HDMI21_FW_RS_2_get_fw_rs_corr_cnt_l2(data)            
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_3_fw_rs_corr_cnt_l3_mask               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_3_fw_rs_corr_cnt_l3_mask                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_3_fw_rs_corr_cnt_l3(data)              HDMI21_P0_FEC_FEC_HDMI21_FW_RS_3_fw_rs_corr_cnt_l3(data)                
#define  HD21_FEC_FEC_HDMI21_FW_RS_3_get_fw_rs_corr_cnt_l3(data)          HDMI21_P0_FEC_FEC_HDMI21_FW_RS_3_get_fw_rs_corr_cnt_l3(data)            
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_4_fw_rs_corr_th_mask                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_4_fw_rs_corr_th_mask                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_4_fw_rs_corr_th(data)                  HDMI21_P0_FEC_FEC_HDMI21_FW_RS_4_fw_rs_corr_th(data)                    
#define  HD21_FEC_FEC_HDMI21_FW_RS_4_get_fw_rs_corr_th(data)              HDMI21_P0_FEC_FEC_HDMI21_FW_RS_4_get_fw_rs_corr_th(data)                
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_en_mask                    HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_en_mask                      
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_ie_mask                    HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_ie_mask                      
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l0_mask                        HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l0_mask                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l1_mask                        HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l1_mask                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l2_mask                        HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l2_mask                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l3_mask                        HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l3_mask                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_md_mask                    HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_md_mask                      
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_dummy_0_mask                         HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_dummy_0_mask                           
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_en(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_en(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_ie(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_ie(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l0(data)                       HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l0(data)                         
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l1(data)                       HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l1(data)                         
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l2(data)                       HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l2(data)                         
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l3(data)                       HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_l3(data)                         
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_md(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_fw_rs_cnt_md(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_dummy_0(data)                        HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_dummy_0(data)                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_en(data)               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_en(data)                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_ie(data)               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_ie(data)                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l0(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l0(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l1(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l1(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l2(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l2(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l3(data)                   HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_l3(data)                     
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_md(data)               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_fw_rs_cnt_md(data)                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_5_get_dummy_0(data)                    HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_get_dummy_0(data)                      
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_dummy_7_3_mask                       HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_dummy_7_3_mask                         
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_done_mask                  HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_done_mask                    
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_period_mask                HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_period_mask                  
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_dummy_7_3(data)                      HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_dummy_7_3(data)                        
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_done(data)                 HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_done(data)                   
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_period(data)               HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_fw_rs_cnt_period(data)                 
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_get_dummy_7_3(data)                  HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_get_dummy_7_3(data)                    
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_get_fw_rs_cnt_done(data)             HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_get_fw_rs_cnt_done(data)               
#define  HD21_FEC_FEC_HDMI21_FW_RS_6_get_fw_rs_cnt_period(data)           HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_get_fw_rs_cnt_period(data)             
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_1_lfsr_chk_en_mask                      HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_lfsr_chk_en_mask                        
#define  HD21_FEC_FEC_HDMI21_LFSR_1_lfsr_fail_mask                        HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_lfsr_fail_mask                          
#define  HD21_FEC_FEC_HDMI21_LFSR_1_map_type_mask                         HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_map_type_mask                           
#define  HD21_FEC_FEC_HDMI21_LFSR_1_lfsr_chk_en(data)                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_lfsr_chk_en(data)                       
#define  HD21_FEC_FEC_HDMI21_LFSR_1_lfsr_fail(data)                       HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_lfsr_fail(data)                         
#define  HD21_FEC_FEC_HDMI21_LFSR_1_map_type(data)                        HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_map_type(data)                          
#define  HD21_FEC_FEC_HDMI21_LFSR_1_get_lfsr_chk_en(data)                 HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_get_lfsr_chk_en(data)                   
#define  HD21_FEC_FEC_HDMI21_LFSR_1_get_lfsr_fail(data)                   HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_get_lfsr_fail(data)                     
#define  HD21_FEC_FEC_HDMI21_LFSR_1_get_map_type(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_get_map_type(data)                      
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_done_mask                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_done_mask                      
#define  HD21_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_num_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_num_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_2_dummy_3_0_mask                        HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_dummy_3_0_mask                          
#define  HD21_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_done(data)                   HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_done(data)                     
#define  HD21_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_num(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_lfsr_chk_num(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_2_dummy_3_0(data)                       HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_dummy_3_0(data)                         
#define  HD21_FEC_FEC_HDMI21_LFSR_2_get_lfsr_chk_done(data)               HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_get_lfsr_chk_done(data)                 
#define  HD21_FEC_FEC_HDMI21_LFSR_2_get_lfsr_chk_num(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_get_lfsr_chk_num(data)                  
#define  HD21_FEC_FEC_HDMI21_LFSR_2_get_dummy_3_0(data)                   HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_get_dummy_3_0(data)                     
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_3_lfsr_err_cnt_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_3_lfsr_err_cnt_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_3_lfsr_err_cnt(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_3_lfsr_err_cnt(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_3_get_lfsr_err_cnt(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_3_get_lfsr_err_cnt(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_4_lfsr1_seed_l_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_4_lfsr1_seed_l_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_4_lfsr1_seed_l(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_4_lfsr1_seed_l(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_4_get_lfsr1_seed_l(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_4_get_lfsr1_seed_l(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_5_lfsr1_seed_h_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_5_lfsr1_seed_h_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_5_lfsr1_seed_h(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_5_lfsr1_seed_h(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_5_get_lfsr1_seed_h(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_5_get_lfsr1_seed_h(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_6_lfsr2_seed_l_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_6_lfsr2_seed_l_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_6_lfsr2_seed_l(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_6_lfsr2_seed_l(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_6_get_lfsr2_seed_l(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_6_get_lfsr2_seed_l(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_7_lfsr2_seed_h_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_7_lfsr2_seed_h_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_7_lfsr2_seed_h(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_7_lfsr2_seed_h(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_7_get_lfsr2_seed_h(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_7_get_lfsr2_seed_h(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_8_lfsr3_seed_l_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_8_lfsr3_seed_l_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_8_lfsr3_seed_l(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_8_lfsr3_seed_l(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_8_get_lfsr3_seed_l(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_8_get_lfsr3_seed_l(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_9_lfsr3_seed_h_mask                     HDMI21_P0_FEC_FEC_HDMI21_LFSR_9_lfsr3_seed_h_mask                       
#define  HD21_FEC_FEC_HDMI21_LFSR_9_lfsr3_seed_h(data)                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_9_lfsr3_seed_h(data)                      
#define  HD21_FEC_FEC_HDMI21_LFSR_9_get_lfsr3_seed_h(data)                HDMI21_P0_FEC_FEC_HDMI21_LFSR_9_get_lfsr3_seed_h(data)                  
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_10_lfsr4_seed_l_mask                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_10_lfsr4_seed_l_mask                      
#define  HD21_FEC_FEC_HDMI21_LFSR_10_lfsr4_seed_l(data)                   HDMI21_P0_FEC_FEC_HDMI21_LFSR_10_lfsr4_seed_l(data)                     
#define  HD21_FEC_FEC_HDMI21_LFSR_10_get_lfsr4_seed_l(data)               HDMI21_P0_FEC_FEC_HDMI21_LFSR_10_get_lfsr4_seed_l(data)                 
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LFSR_11_lfsr4_seed_h_mask                    HDMI21_P0_FEC_FEC_HDMI21_LFSR_11_lfsr4_seed_h_mask                      
#define  HD21_FEC_FEC_HDMI21_LFSR_11_lfsr4_seed_h(data)                   HDMI21_P0_FEC_FEC_HDMI21_LFSR_11_lfsr4_seed_h(data)                     
#define  HD21_FEC_FEC_HDMI21_LFSR_11_get_lfsr4_seed_h(data)               HDMI21_P0_FEC_FEC_HDMI21_LFSR_11_get_lfsr4_seed_h(data)                 
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_DI_0_digb_judge_mask                         HDMI21_P0_FEC_FEC_HDMI21_DI_0_digb_judge_mask                           
#define  HD21_FEC_FEC_HDMI21_DI_0_di_start_judge_mask                     HDMI21_P0_FEC_FEC_HDMI21_DI_0_di_start_judge_mask                       
#define  HD21_FEC_FEC_HDMI21_DI_0_hs_polar_invert_mask                    HDMI21_P0_FEC_FEC_HDMI21_DI_0_hs_polar_invert_mask                      
#define  HD21_FEC_FEC_HDMI21_DI_0_vs_polar_invert_mask                    HDMI21_P0_FEC_FEC_HDMI21_DI_0_vs_polar_invert_mask                      
#define  HD21_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode_mask                   HDMI21_P0_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode_mask                     
#define  HD21_FEC_FEC_HDMI21_DI_0_dummy_2_0_mask                          HDMI21_P0_FEC_FEC_HDMI21_DI_0_dummy_2_0_mask                            
#define  HD21_FEC_FEC_HDMI21_DI_0_digb_judge(data)                        HDMI21_P0_FEC_FEC_HDMI21_DI_0_digb_judge(data)                          
#define  HD21_FEC_FEC_HDMI21_DI_0_di_start_judge(data)                    HDMI21_P0_FEC_FEC_HDMI21_DI_0_di_start_judge(data)                      
#define  HD21_FEC_FEC_HDMI21_DI_0_hs_polar_invert(data)                   HDMI21_P0_FEC_FEC_HDMI21_DI_0_hs_polar_invert(data)                     
#define  HD21_FEC_FEC_HDMI21_DI_0_vs_polar_invert(data)                   HDMI21_P0_FEC_FEC_HDMI21_DI_0_vs_polar_invert(data)                     
#define  HD21_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode(data)                  HDMI21_P0_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode(data)                    
#define  HD21_FEC_FEC_HDMI21_DI_0_dummy_2_0(data)                         HDMI21_P0_FEC_FEC_HDMI21_DI_0_dummy_2_0(data)                           
#define  HD21_FEC_FEC_HDMI21_DI_0_get_digb_judge(data)                    HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_digb_judge(data)                      
#define  HD21_FEC_FEC_HDMI21_DI_0_get_di_start_judge(data)                HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_di_start_judge(data)                  
#define  HD21_FEC_FEC_HDMI21_DI_0_get_hs_polar_invert(data)               HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_hs_polar_invert(data)                 
#define  HD21_FEC_FEC_HDMI21_DI_0_get_vs_polar_invert(data)               HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_vs_polar_invert(data)                 
#define  HD21_FEC_FEC_HDMI21_DI_0_get_vs_polar_fw_mode(data)              HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_vs_polar_fw_mode(data)                
#define  HD21_FEC_FEC_HDMI21_DI_0_get_dummy_2_0(data)                     HDMI21_P0_FEC_FEC_HDMI21_DI_0_get_dummy_2_0(data)                       
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_save_mask                  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_save_mask                    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_ch_mask                    HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_ch_mask                      
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_dummy_4_1_mask                     HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_dummy_4_1_mask                       
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_save_done_mask                  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_save_done_mask                    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_save(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_save(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_ch(data)                   HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_fifo_ch(data)                     
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_dummy_4_1(data)                    HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_dummy_4_1(data)                      
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_rs_save_done(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_rs_save_done(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_fifo_save(data)             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_fifo_save(data)               
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_fifo_ch(data)               HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_fifo_ch(data)                 
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_get_dummy_4_1(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_get_dummy_4_1(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_save_done(data)             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_get_rs_save_done(data)               
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_1_rs_sram_addr_mask                  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_1_rs_sram_addr_mask                    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_1_rs_sram_addr(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_1_rs_sram_addr(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_1_get_rs_sram_addr(data)             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_1_get_rs_sram_addr(data)               
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_2_rs_sram_data_mask                  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_2_rs_sram_data_mask                    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_2_rs_sram_data(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_2_rs_sram_data(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_2_get_rs_sram_data(data)             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_2_get_rs_sram_data(data)               
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_rst_n_rs_mask            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_rst_n_rs_mask              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_mode_rs_mask             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_mode_rs_mask               
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_bist_mode_rs_mask         HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_bist_mode_rs_mask           
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_start_pause_rs_mask       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_start_pause_rs_mask         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_test_resume_rs_mask       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_test_resume_rs_mask         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_bist_clk_en_mask           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_bist_clk_en_mask             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_cp_clk_en_mask             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_cp_clk_en_mask               
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_dummy_0_mask                  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_dummy_0_mask                    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_rst_n_rs(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_rst_n_rs(data)             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_mode_rs(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_bist_mode_rs(data)              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_bist_mode_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_bist_mode_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_start_pause_rs(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_start_pause_rs(data)        
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_test_resume_rs(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_drf_test_resume_rs(data)        
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_bist_clk_en(data)          HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_bist_clk_en(data)            
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_cp_clk_en(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_rs_cp_clk_en(data)              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_dummy_0(data)                 HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_dummy_0(data)                   
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_bist_rst_n_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_bist_rst_n_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_bist_mode_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_bist_mode_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_bist_mode_rs(data)    HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_bist_mode_rs(data)      
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_start_pause_rs(data)  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_start_pause_rs(data)    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_test_resume_rs(data)  HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_drf_test_resume_rs(data)    
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_rs_bist_clk_en(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_rs_bist_clk_en(data)        
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_rs_cp_clk_en(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_rs_cp_clk_en(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_dummy_0(data)             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_get_dummy_0(data)               
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_done_rs_mask             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_done_rs_mask               
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail_rs_mask             HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail_rs_mask               
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail0_rs_mask            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail0_rs_mask              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail1_rs_mask            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail1_rs_mask              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail2_rs_mask            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail2_rs_mask              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail3_rs_mask            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail3_rs_mask              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_dummy_2_0_mask                HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_dummy_2_0_mask                  
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_done_rs(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_done_rs(data)              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail_rs(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail_rs(data)              
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail0_rs(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail0_rs(data)             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail1_rs(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail1_rs(data)             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail2_rs(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail2_rs(data)             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail3_rs(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_bist_fail3_rs(data)             
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_dummy_2_0(data)               HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_dummy_2_0(data)                 
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_done_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_done_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail0_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail0_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail1_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail1_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail2_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail2_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail3_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_bist_fail3_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_dummy_2_0(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_get_dummy_2_0(data)             
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_done_rs_mask         HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_done_rs_mask           
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail_rs_mask         HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail_rs_mask           
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail0_rs_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail0_rs_mask          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail1_rs_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail1_rs_mask          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail2_rs_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail2_rs_mask          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail3_rs_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail3_rs_mask          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_dummy_2_0_mask                HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_dummy_2_0_mask                  
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_done_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_done_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail_rs(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail_rs(data)          
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail0_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail0_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail1_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail1_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail2_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail2_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail3_rs(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_drf_bist_fail3_rs(data)         
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_dummy_2_0(data)               HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_dummy_2_0(data)                 
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_done_rs(data)    HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_done_rs(data)      
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail_rs(data)    HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail_rs(data)      
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail0_rs(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail0_rs(data)     
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail1_rs(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail1_rs(data)     
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail2_rs(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail2_rs(data)     
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail3_rs(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_drf_bist_fail3_rs(data)     
#define  HD21_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_dummy_2_0(data)           HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_get_dummy_2_0(data)             
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb3_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb3_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb2_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb2_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb1_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb1_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb0_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb0_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma3_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma3_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma2_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma2_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma1_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma1_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma0_mask                 HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma0_mask                   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb3(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb3(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb2(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb2(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb1(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb1(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb0(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rmb0(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma3(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma3(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma2(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma2(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma1(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma1(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma0(data)                HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_fec_rma0(data)                  
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb3(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb3(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb2(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb2(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb1(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb1(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb0(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rmb0(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma3(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma3(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma2(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma2(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma1(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma1(data)              
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma0(data)            HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_get_fec_rma0(data)              
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_3_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_3_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_3_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_3_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_2_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_2_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_2_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_2_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_1_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_1_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_1_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_1_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_0_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_0_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_0_mask      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_0_mask        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls3_mask           HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls3_mask             
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls2_mask           HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls2_mask             
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls1_mask           HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls1_mask             
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls0_mask           HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls0_mask             
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_3_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_3_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_2_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_2_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_1_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_1_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_0_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_0_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_3_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_3_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_2_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_2_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_1_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_1_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_0_mask        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_0_mask          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_dummy_7_0_mask         HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_dummy_7_0_mask           
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_3(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_3(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_3(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_3(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_2(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_2(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_2(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_2(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_1(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_1(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_1(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_1(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_0(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1b_0(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_0(data)     HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_test1a_0(data)       
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls3(data)          HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls3(data)            
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls2(data)          HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls2(data)            
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls1(data)          HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls1(data)            
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls0(data)          HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_ls0(data)            
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_3(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_3(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_2(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_2(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_1(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_1(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_0(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmeb_0(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_3(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_3(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_2(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_2(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_1(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_1(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_0(data)       HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_fec_rmea_0(data)         
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_dummy_7_0(data)        HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_dummy_7_0(data)          
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_3(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_3(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_3(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_3(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_2(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_2(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_2(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_2(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_1(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_1(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_1(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_1(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_0(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1b_0(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_0(data) HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_test1a_0(data)   
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls3(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls3(data)        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls2(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls2(data)        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls1(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls1(data)        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls0(data)      HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_ls0(data)        
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_3(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_3(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_2(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_2(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_1(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_1(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_0(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmeb_0(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_3(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_3(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_2(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_2(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_1(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_1(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_0(data)   HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_fec_rmea_0(data)     
#define  HD21_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_dummy_7_0(data)    HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_get_dummy_7_0(data)      
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_det_en_mask                        HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_det_en_mask                          
#define  HD21_FEC_FEC_HDMI21_FRL_0_v_gb_det_mask                          HDMI21_P0_FEC_FEC_HDMI21_FRL_0_v_gb_det_mask                            
#define  HD21_FEC_FEC_HDMI21_FRL_0_v_pkt_det_mask                         HDMI21_P0_FEC_FEC_HDMI21_FRL_0_v_pkt_det_mask                           
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_hs_mask                            HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_hs_mask                              
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_vs_mask                            HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_vs_mask                              
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_data_byte_swap_mask                HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_data_byte_swap_mask                  
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_data_bit_rev_mask                  HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_data_bit_rev_mask                    
#define  HD21_FEC_FEC_HDMI21_FRL_0_dummy_0_mask                           HDMI21_P0_FEC_FEC_HDMI21_FRL_0_dummy_0_mask                             
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_det_en(data)                       HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_det_en(data)                         
#define  HD21_FEC_FEC_HDMI21_FRL_0_v_gb_det(data)                         HDMI21_P0_FEC_FEC_HDMI21_FRL_0_v_gb_det(data)                           
#define  HD21_FEC_FEC_HDMI21_FRL_0_v_pkt_det(data)                        HDMI21_P0_FEC_FEC_HDMI21_FRL_0_v_pkt_det(data)                          
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_hs(data)                           HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_hs(data)                             
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_vs(data)                           HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_vs(data)                             
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_data_byte_swap(data)               HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_data_byte_swap(data)                 
#define  HD21_FEC_FEC_HDMI21_FRL_0_frl_data_bit_rev(data)                 HDMI21_P0_FEC_FEC_HDMI21_FRL_0_frl_data_bit_rev(data)                   
#define  HD21_FEC_FEC_HDMI21_FRL_0_dummy_0(data)                          HDMI21_P0_FEC_FEC_HDMI21_FRL_0_dummy_0(data)                            
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_frl_det_en(data)                   HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_frl_det_en(data)                     
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_v_gb_det(data)                     HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_v_gb_det(data)                       
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_v_pkt_det(data)                    HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_v_pkt_det(data)                      
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_frl_hs(data)                       HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_frl_hs(data)                         
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_frl_vs(data)                       HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_frl_vs(data)                         
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_frl_data_byte_swap(data)           HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_frl_data_byte_swap(data)             
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_frl_data_bit_rev(data)             HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_frl_data_bit_rev(data)               
#define  HD21_FEC_FEC_HDMI21_FRL_0_get_dummy_0(data)                      HDMI21_P0_FEC_FEC_HDMI21_FRL_0_get_dummy_0(data)                        
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_wd_en_mask                         HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_wd_en_mask                           
#define  HD21_FEC_FEC_HDMI21_FRL_1_data_island_err_mask                   HDMI21_P0_FEC_FEC_HDMI21_FRL_1_data_island_err_mask                     
#define  HD21_FEC_FEC_HDMI21_FRL_1_di_err_ie_mask                         HDMI21_P0_FEC_FEC_HDMI21_FRL_1_di_err_ie_mask                           
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_pkt_len_err_mask                   HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_pkt_len_err_mask                     
#define  HD21_FEC_FEC_HDMI21_FRL_1_gap_judge_mask                         HDMI21_P0_FEC_FEC_HDMI21_FRL_1_gap_judge_mask                           
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_map_err_mask                       HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_map_err_mask                         
#define  HD21_FEC_FEC_HDMI21_FRL_1_dummy_1_0_mask                         HDMI21_P0_FEC_FEC_HDMI21_FRL_1_dummy_1_0_mask                           
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_wd_en(data)                        HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_wd_en(data)                          
#define  HD21_FEC_FEC_HDMI21_FRL_1_data_island_err(data)                  HDMI21_P0_FEC_FEC_HDMI21_FRL_1_data_island_err(data)                    
#define  HD21_FEC_FEC_HDMI21_FRL_1_di_err_ie(data)                        HDMI21_P0_FEC_FEC_HDMI21_FRL_1_di_err_ie(data)                          
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_pkt_len_err(data)                  HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_pkt_len_err(data)                    
#define  HD21_FEC_FEC_HDMI21_FRL_1_gap_judge(data)                        HDMI21_P0_FEC_FEC_HDMI21_FRL_1_gap_judge(data)                          
#define  HD21_FEC_FEC_HDMI21_FRL_1_frl_map_err(data)                      HDMI21_P0_FEC_FEC_HDMI21_FRL_1_frl_map_err(data)                        
#define  HD21_FEC_FEC_HDMI21_FRL_1_dummy_1_0(data)                        HDMI21_P0_FEC_FEC_HDMI21_FRL_1_dummy_1_0(data)                          
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_frl_wd_en(data)                    HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_frl_wd_en(data)                      
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_data_island_err(data)              HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_data_island_err(data)                
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_di_err_ie(data)                    HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_di_err_ie(data)                      
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_frl_pkt_len_err(data)              HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_frl_pkt_len_err(data)                
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_gap_judge(data)                    HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_gap_judge(data)                      
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_frl_map_err(data)                  HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_frl_map_err(data)                    
#define  HD21_FEC_FEC_HDMI21_FRL_1_get_dummy_1_0(data)                    HDMI21_P0_FEC_FEC_HDMI21_FRL_1_get_dummy_1_0(data)                      
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FRL_D1_dummy_7_0_mask                        HDMI21_P0_FEC_FEC_HDMI21_FRL_D1_dummy_7_0_mask                          
#define  HD21_FEC_FEC_HDMI21_FRL_D1_dummy_7_0(data)                       HDMI21_P0_FEC_FEC_HDMI21_FRL_D1_dummy_7_0(data)                         
#define  HD21_FEC_FEC_HDMI21_FRL_D1_get_dummy_7_0(data)                   HDMI21_P0_FEC_FEC_HDMI21_FRL_D1_get_dummy_7_0(data)                     
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_FRL_D2_dummy_7_0_mask                        HDMI21_P0_FEC_FEC_HDMI21_FRL_D2_dummy_7_0_mask                          
#define  HD21_FEC_FEC_HDMI21_FRL_D2_dummy_7_0(data)                       HDMI21_P0_FEC_FEC_HDMI21_FRL_D2_dummy_7_0(data)                         
#define  HD21_FEC_FEC_HDMI21_FRL_D2_get_dummy_7_0(data)                   HDMI21_P0_FEC_FEC_HDMI21_FRL_D2_get_dummy_7_0(data)                     
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_TEST_SEL_hdmi21_tst_sel_mask                 HDMI21_P0_FEC_FEC_HDMI21_TEST_SEL_hdmi21_tst_sel_mask                   
#define  HD21_FEC_FEC_HDMI21_TEST_SEL_hdmi21_tst_sel(data)                HDMI21_P0_FEC_FEC_HDMI21_TEST_SEL_hdmi21_tst_sel(data)                  
#define  HD21_FEC_FEC_HDMI21_TEST_SEL_get_hdmi21_tst_sel(data)            HDMI21_P0_FEC_FEC_HDMI21_TEST_SEL_get_hdmi21_tst_sel(data)              
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_en_mask                          HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_en_mask                            
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_err_mask                         HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_err_mask                           
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_err_ie_mask                      HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_err_ie_mask                        
#define  HD21_FEC_FEC_HDMI21_LT_9_dummy_4_0_mask                          HDMI21_P0_FEC_FEC_HDMI21_LT_9_dummy_4_0_mask                            
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_en(data)                         HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_en(data)                           
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_err(data)                        HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_err(data)                          
#define  HD21_FEC_FEC_HDMI21_LT_9_lt_gap_err_ie(data)                     HDMI21_P0_FEC_FEC_HDMI21_LT_9_lt_gap_err_ie(data)                       
#define  HD21_FEC_FEC_HDMI21_LT_9_dummy_4_0(data)                         HDMI21_P0_FEC_FEC_HDMI21_LT_9_dummy_4_0(data)                           
#define  HD21_FEC_FEC_HDMI21_LT_9_get_lt_gap_en(data)                     HDMI21_P0_FEC_FEC_HDMI21_LT_9_get_lt_gap_en(data)                       
#define  HD21_FEC_FEC_HDMI21_LT_9_get_lt_gap_err(data)                    HDMI21_P0_FEC_FEC_HDMI21_LT_9_get_lt_gap_err(data)                      
#define  HD21_FEC_FEC_HDMI21_LT_9_get_lt_gap_err_ie(data)                 HDMI21_P0_FEC_FEC_HDMI21_LT_9_get_lt_gap_err_ie(data)                   
#define  HD21_FEC_FEC_HDMI21_LT_9_get_dummy_4_0(data)                     HDMI21_P0_FEC_FEC_HDMI21_LT_9_get_dummy_4_0(data)                       
                                                                          
                                                                          
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_en_mask        HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_en_mask          
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_mask           HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_mask             
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_cond_mask               HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_cond_mask                 
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_a_time_mask               HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_a_time_mask                 
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_b_time_mask               HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_b_time_mask                 
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl_mask               HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl_mask                 
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_mask                    HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_mask                      
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_dummy_1_0_mask                 HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_dummy_1_0_mask                   
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_en(data)       HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det_en(data)         
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det(data)          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_chg_hdmi_md_det(data)            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_cond(data)              HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_cond(data)                
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_a_time(data)              HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_a_time(data)                
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_b_time(data)              HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_cond_b_time(data)                
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl(data)              HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl(data)                
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel(data)                   HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel(data)                     
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_dummy_1_0(data)                HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_dummy_1_0(data)                  
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_chg_hdmi_md_det_en(data)   HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_chg_hdmi_md_det_en(data)     
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_chg_hdmi_md_det(data)      HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_chg_hdmi_md_det(data)        
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel_cond(data)          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel_cond(data)            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_cond_a_time(data)          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_cond_a_time(data)            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_cond_b_time(data)          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_cond_b_time(data)            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel_ctrl(data)          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel_ctrl(data)            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel(data)               HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_md_sel(data)                 
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_dummy_1_0(data)            HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_get_dummy_1_0(data)              
                                                                          
                                                                          
#define  HD21_FEC_FEC_HDMI_SR_hdmi_md_det_mask                            HDMI21_P0_FEC_FEC_HDMI_SR_hdmi_md_det_mask                              
#define  HD21_FEC_FEC_HDMI_SR_hdmi_md_det(data)                           HDMI21_P0_FEC_FEC_HDMI_SR_hdmi_md_det(data)                             
#define  HD21_FEC_FEC_HDMI_SR_get_hdmi_md_det(data)                       HDMI21_P0_FEC_FEC_HDMI_SR_get_hdmi_md_det(data)                         
                                                                          
                                                                          
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_vgb_aft_vpre_mask          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_vgb_aft_vpre_mask            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_agb_aft_apre_mask          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_agb_aft_apre_mask            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_trailing_agb_mask          HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_trailing_agb_mask            
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_vgb_aft_vpre(data)         HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_vgb_aft_vpre(data)           
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_agb_aft_apre(data)         HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_agb_aft_apre(data)           
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_trailing_agb(data)         HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_no_trailing_agb(data)           
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_vgb_aft_vpre(data)     HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_vgb_aft_vpre(data)       
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_agb_aft_apre(data)     HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_agb_aft_apre(data)       
#define  HD21_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_trailing_agb(data)     HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_get_no_trailing_agb(data)       
