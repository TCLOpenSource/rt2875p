/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file:        hdmi_reg.c
 *
 * author:  Falcon, Sam Wilson
 * date:
 * version:     3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include "hdmi_reg.h"


/*--------------------------------------------------
 * Phy register setting
 *--------------------------------------------------*/
const struct hdmi21_dfe_reg_st *hd21_dfe[4] = 
{
    (struct hdmi21_dfe_reg_st *) HDMI_DFE0_BASE,
    (struct hdmi21_dfe_reg_st *) HDMI_DFE1_BASE,

    (struct hdmi21_dfe_reg_st *) HDMI_DFE2_BASE,
    (struct hdmi21_dfe_reg_st *) HDMI_DFE3_BASE
};


const struct hdmi21_aphy_reg_st *hd21_aphy[4] = {
	(struct hdmi21_aphy_reg_st *) HDMI_APHY0_BASE,
	(struct hdmi21_aphy_reg_st *) HDMI_APHY1_BASE,
	(struct hdmi21_aphy_reg_st *) HDMI_APHY2_BASE,
	(struct hdmi21_aphy_reg_st *) HDMI_APHY3_BASE
};

const struct hdmi21_dphy_reg_st *hd21_dphy[4] = {
	(struct hdmi21_dphy_reg_st *) HDMI_DPHY0_BASE,
	(struct hdmi21_dphy_reg_st *) HDMI_DPHY1_BASE,
	(struct hdmi21_dphy_reg_st *) HDMI_DPHY2_BASE,
	(struct hdmi21_dphy_reg_st *) HDMI_DPHY3_BASE
};


const struct hdmi21_aphy_rstb_reg_st *hd21_aphy_rstb[4] = {
	(struct hdmi21_aphy_rstb_reg_st *) HDMI_APHY0_RSTB_BASE,
	(struct hdmi21_aphy_rstb_reg_st *) HDMI_APHY1_RSTB_BASE,
	(struct hdmi21_aphy_rstb_reg_st *) HDMI_APHY2_RSTB_BASE,
	(struct hdmi21_aphy_rstb_reg_st *) HDMI_APHY3_RSTB_BASE
};


/*--------------------------------------------------
 * Mac register setting
 *--------------------------------------------------*/
const struct hdmi_mac_reg_st mac[4] =
{
    // mac 0
    {
        .soft_0                       = HDMI_P0_SOFT_0_reg,
        .soft_1                       = HDMI_P0_SOFT_1_reg,
        // CRC Function
        .pp_tmds_crcc                 = HDMI_P0_PP_TMDS_CRCC_reg,
        .pp_tmds_crco0                = HDMI_P0_PP_TMDS_CRCO0_reg,
        .pp_tmds_crco1                = HDMI_P0_PP_TMDS_CRCO1_reg,

        // TMDS Channel Status
        .tmds_ctrl                    = HDMI_P0_TMDS_CTRL_reg,
        .tmds_outctl                  = HDMI_P0_TMDS_OUTCTL_reg,
        .tmds_pwdctl                  = HDMI_P0_TMDS_PWDCTL_reg,
        .tmds_z0cc                    = HDMI_P0_TMDS_Z0CC_reg,
        .tmds_cps                     = HDMI_P0_TMDS_CPS_reg,
        .tmds_udc                     = HDMI_P0_TMDS_UDC_reg,
        .tmds_errc                    = HDMI_P0_TMDS_ERRC_reg,
        .tmds_out_ctrl                = HDMI_P0_TMDS_OUT_CTRL_reg,
        .tmds_rout                    = HDMI_P0_TMDS_ROUT_reg,
        .tmds_gout                    = HDMI_P0_TMDS_GOUT_reg,
        .tmds_bout                    = HDMI_P0_TMDS_BOUT_reg,
        .tmds_dpc0                    = HDMI_P0_TMDS_DPC0_reg,
        .tmds_dpc1                    = HDMI_P0_TMDS_DPC1_reg,
        .tmds_dpc_set0                = HDMI_P0_TMDS_DPC_SET0_reg,
        .tmds_dpc_set1                = HDMI_P0_TMDS_DPC_SET1_reg,
        .tmds_dpc_set2                = HDMI_P0_TMDS_DPC_SET2_reg,
        .tmds_dpc_set3                = HDMI_P0_TMDS_DPC_SET3_reg,

        // Video Data Error Detect
        .video_bit_err_det            = HDMI_P0_VIDEO_BIT_ERR_DET_reg,
        .video_bit_err_status_b       = HDMI_P0_VIDEO_BIT_ERR_STATUS_B_reg,
        .video_bit_err_status_g       = HDMI_P0_VIDEO_BIT_ERR_STATUS_G_reg,
        .video_bit_err_status_r       = HDMI_P0_VIDEO_BIT_ERR_STATUS_R_reg,

        // TERC4 Error Detect
        .terc4_err_det                = HDMI_P0_TERC4_ERR_DET_reg,
        .terc4_err_status_b           = HDMI_P0_TERC4_ERR_STATUS_B_reg,
        .terc4_err_status_g           = HDMI_P0_TERC4_ERR_STATUS_G_reg,
        .terc4_err_status_r           = HDMI_P0_TERC4_ERR_STATUS_R_reg,

        // Flow and Packet
        .hdmi_sr                      = HDMI_P0_HDMI_SR_reg,
        .hdmi_gpvs                    = HDMI_P0_HDMI_GPVS_reg,
        .hdmi_gpvs1                   = HDMI_P0_HDMI_GPVS1_reg,
        .hdmi_psap                    = HDMI_P0_HDMI_PSAP_reg,
        .hdmi_psdp                    = HDMI_P0_HDMI_PSDP_reg,

        .hdmi_scr                     = HDMI_P0_HDMI_SCR_reg,
        .hdmi_bchcr                   = HDMI_P0_HDMI_BCHCR_reg,
        .hdmi_avmcr                   = HDMI_P0_HDMI_AVMCR_reg,
        .hdmi_wdcr                    = HDMI_P0_HDMI_WDCR_reg,

        .hdmi_pamicr                  = HDMI_P0_HDMI_PAMICR_reg,
        .hdmi_ptrsv1                  = HDMI_P0_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2                  = HDMI_P0_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3                  = HDMI_P0_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0                  = HDMI_P0_HDMI_PVGCR0_reg,
        .hdmi_pvsr0                   = HDMI_P0_HDMI_PVSR0_reg,
        .hdmi_vcr                     = HDMI_P0_HDMI_VCR_reg,
        .hdmi_acrcr                   = HDMI_P0_HDMI_ACRCR_reg,
        .hdmi_acrsr0                  = HDMI_P0_HDMI_ACRSR0_reg,
        .hdmi_acrsr1                  = HDMI_P0_HDMI_ACRSR1_reg,
        .hdmi_intcr                   = HDMI_P0_HDMI_INTCR_reg,

        // Packet Guard Band
        .hdmi_bcsr                    = HDMI_P0_HDMI_BCSR_reg,
        .hdmi_asr0                    = HDMI_P0_HDMI_ASR0_reg,
        .hdmi_asr1                    = HDMI_P0_HDMI_ASR1_reg,
        .hdmi_video_format            = HDMI_P0_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format               = HDMI_P0_HDMI_3D_FORMAT_reg,
        .hdmi_fvs                     = HDMI_P0_HDMI_FVS_reg,
        .hdmi_drm                     = HDMI_P0_HDMI_DRM_reg,
        .hdmi_pcmc                    = HDMI_P0_HDMI_PCMC_reg,

        // EMP
        .hdrap                        = HDMI_P0_hdrap_reg,
        .hdrdp                        = HDMI_P0_hdrdp_reg,
        .hdr_st                       = HDMI_P0_hdr_st_reg,
        .em_ct                        = HDMI_P0_em_ct_reg,
        .fapa_ct                      = HDMI_P0_fapa_ct_reg,
        .hdmi_emap                    = HDMI_P0_HDMI_EMAP_reg,
        .hdmi_emdp                    = HDMI_P0_HDMI_EMDP_reg,
        .hdmi_emap_pp                 = HDMI_P0_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp                 = HDMI_P0_HDMI_EMDP_PP_reg,
        .vrrem_st                     = HDMI_P0_VRREM_ST_reg,
        .hdmi_vrr_emc                 = HDMI_P0_HDMI_VRR_EMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI_P0_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet   = HDMI_P0_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr                   = HDMI_P0_HDMI_AUDCR_reg,
        .hdmi_audsr                   = HDMI_P0_HDMI_AUDSR_reg,

        // Clock Detection
        .clkdetcr                     = HDMI_P0_CLKDETCR_reg,
        .clkdetsr                     = HDMI_P0_CLKDETSR_reg,
        .clk_setting_00               = HDMI_P0_CLK_SETTING_00_reg,
        .clk_setting_01               = HDMI_P0_CLK_SETTING_01_reg,
        .clk_setting_02               = HDMI_P0_CLK_SETTING_02_reg,

        // RPBS
        .prbs_r_ctrl                  = HDMI_P0_PRBS_R_CTRL_reg,
        .prbs_g_ctrl                  = HDMI_P0_PRBS_G_CTRL_reg,
        .prbs_b_ctrl                  = HDMI_P0_PRBS_B_CTRL_reg,

        // MHL
        .hdmi_leading_gb_cmp_ctrl     = HDMI_P0_HDMI_leading_GB_cmp_ctrl_reg,
        .hdmi_leading_gb_cmp_times    = HDMI_P0_HDMI_leading_GB_cmp_times_reg,
        .hdmi_leading_gb_cmp_cnt      = HDMI_P0_HDMI_leading_GB_cmp_cnt_reg,
        .audio_cts_up_bound           = HDMI_P0_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound          = HDMI_P0_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound             = HDMI_P0_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound            = HDMI_P0_AUDIO_N_LOW_BOUND_reg,

        // BIST & FIFO
        .light_sleep                  = HDMI_P0_LIGHT_SLEEP_reg,
        .read_margin_enable           = HDMI_P0_READ_MARGIN_ENABLE_reg,
        .read_margin3                 = HDMI_P0_READ_MARGIN3_reg,
        .read_margin1                 = HDMI_P0_READ_MARGIN1_reg,
        .read_margin0                 = HDMI_P0_READ_MARGIN0_reg,
        .bist_mode                    = HDMI_P0_BIST_MODE_reg,
        .bist_done                    = HDMI_P0_BIST_DONE_reg,
        .bist_fail                    = HDMI_P0_BIST_FAIL_reg,
        .drf_mode                     = HDMI_P0_DRF_MODE_reg,
        .drf_resume                   = HDMI_P0_DRF_RESUME_reg,
        .drf_done                     = HDMI_P0_DRF_DONE_reg,
        .drf_pause                    = HDMI_P0_DRF_PAUSE_reg,
        .drf_fail                     = HDMI_P0_DRF_FAIL_reg,

        // PHY FIFO
        .phy_fifo_cr0                 = HDMI_P0_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1                 = HDMI_P0_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0                 = HDMI_P0_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1                 = HDMI_P0_PHY_FIFO_SR1_reg,
        .hdmi_cts_fifo_ctl            = HDMI_P0_HDMI_CTS_FIFO_CTL_reg,

        // CH FIFO
        .ch_cr                        = HDMI_P0_CH_CR_reg,
        .ch_sr                        = HDMI_P0_CH_SR_reg,

        //-------------------------------------------------------------
        // HDMI 2.0
        //-------------------------------------------------------------
        .hdmi_2p0_cr                  = HDMI_P0_HDMI_2p0_CR_reg,
        .hdmi_2p0_cr1                 = HDMI_P0_HDMI_2p0_CR1_reg,
        .scr_cr                       = HDMI_P0_SCR_CR_reg,

        // CED
        .cercr                        = HDMI_P0_CERCR_reg,
        .cersr0                       = HDMI_P0_CERSR0_reg,
        .cersr1                       = HDMI_P0_CERSR1_reg,
        .cersr2                       = HDMI_P0_CERSR2_reg,
        .cersr3                       = HDMI_P0_CERSR3_reg,
        .cersr4                       = HDMI_P0_CERSR4_reg,

        // YUV420 to 444
        .yuv420_cr                    = HDMI_P0_YUV420_CR_reg,
        .yuv420_cr1                   = HDMI_P0_YUV420_CR1_reg,

        // EMP
        .fapa_ct_tmp                  = HDMI_P0_fapa_ct_tmp_reg,
        .hdr_em_ct2                   = HDMI_P0_HDR_EM_CT2_reg,
        .hdmi_vsem_emc                = HDMI_P0_HDMI_VSEM_EMC_reg,
        .hdmi_vsem_emc2               = HDMI_P0_HDMI_VSEM_EMC2_reg,
        .vsem_st                      = HDMI_P0_VSEM_ST_reg,
        .hd20_ps_ct                   = HDMI_P0_HD20_ps_ct_reg,
        .hd20_ps_st                   = HDMI_P0_HD20_ps_st_reg,
        .irq_all_status               = HDMI_P0_irq_all_status_reg,
        .xtal_1ms                     = HDMI_P0_xtal_1ms_reg,
        .xtal_10ms                    = HDMI_P0_xtal_10ms_reg,
        .ps_measure_ctrl              = HDMI_P0_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk          = HDMI_P0_ps_measure_xtal_clk_reg,
        .ps_measure_tmds_clk          = HDMI_P0_ps_measure_tmds_clk_reg,
        .hdmi_pa_irq_en               = HDMI_P0_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta              = HDMI_P0_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt            = HDMI_P0_HDMI_PA_FRAME_CNT_reg,

        // HDMI scrmble control align
        .sc_align_b                   = HDMI_P0_SC_ALIGN_B_reg,
        .sc_align_r                   = HDMI_P0_SC_ALIGN_R_reg,
        .scr_ctr_per_lfsr             = HDMI_P0_SCR_CTR_PER_LFSR_reg,

        // HDCP 1.4
        .hdcp_keepout                 = HDMI_P0_HDCP_KEEPOUT_reg,
        .hdcp_cr                      = HDMI_P0_HDCP_CR_reg,
        .hdcp_cr2                     = HDMI_P0_HDCP_CR2_reg,
        .hdcp_pcr                     = HDMI_P0_HDCP_PCR_reg,
        .hdcp_flag1                   = HDMI_P0_HDCP_FLAG1_reg,
        .hdcp_flag2                   = HDMI_P0_HDCP_FLAG2_reg,

        // HDCP 2.2
        .hdcp_2p2_cr                  = HDMI_P0_HDCP_2p2_CR_reg,
        .hdcp_2p2_ks0                 = HDMI_P0_HDCP_2p2_ks0_reg,
        .hdcp_2p2_ks1                 = HDMI_P0_HDCP_2p2_ks1_reg,
        .hdcp_2p2_ks2                 = HDMI_P0_HDCP_2p2_ks2_reg,
        .hdcp_2p2_ks3                 = HDMI_P0_HDCP_2p2_ks3_reg,
        .hdcp_2p2_lc0                 = HDMI_P0_HDCP_2p2_lc0_reg,
        .hdcp_2p2_lc1                 = HDMI_P0_HDCP_2p2_lc1_reg,
        .hdcp_2p2_lc2                 = HDMI_P0_HDCP_2p2_lc2_reg,
        .hdcp_2p2_lc3                 = HDMI_P0_HDCP_2p2_lc3_reg,
        .hdcp_2p2_riv0                = HDMI_P0_HDCP_2p2_riv0_reg,
        .hdcp_2p2_riv1                = HDMI_P0_HDCP_2p2_riv1_reg,
        .hdcp_2p2_sr0                 = HDMI_P0_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1                 = HDMI_P0_HDCP_2p2_SR1_reg,
        .hdcp_hoa                     = HDMI_P0_HDCP_HOA_reg,
        .hdcp_power_saving_ctrl       = HDMI_P0_HDCP_POWER_SAVING_CTRL_reg,
        .win_opp_ctl                  = HDMI_P0_WIN_OPP_CTRL_reg,
        .hdmi_vpllcr0                 = HDMI_P0_HDMI_VPLLCR0_reg,
        .hdmi_vpllcr1                 = HDMI_P0_HDMI_VPLLCR1_reg,
        .hdmi_vpllcr2                 = HDMI_P0_HDMI_VPLLCR2_reg,
        .mn_sclkg_ctrl                = HDMI_P0_MN_SCLKG_CTRL_reg,
        .mn_sclkg_divs                = HDMI_P0_MN_SCLKG_DIVS_reg,
        .mdd_cr                       = HDMI_P0_MDD_CR_reg,
        .mdd_sr                       = HDMI_P0_MDD_SR_reg,
        .fw_func                      = HDMI_P0_FW_FUNC_reg,
        .port_switch                  = HDMI_P0_PORT_SWITCH_reg,
        .power_saving                 = HDMI_P0_POWER_SAVING_reg,
        .crc_ctrl                     = HDMI_P0_CRC_Ctrl_reg,
        .crc_result                   = HDMI_P0_CRC_Result_reg,
        .des_crc                      = HDMI_P0_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI_P0_CRC_ERR_CNT0_reg,
        .hdmi_20_21_ctrl              = HDMI_P0_hdmi_20_21_ctrl_reg, //524

        // YUV422_CD
        .yuv422_cd                    = HDMI_P0_YUV422_CD_reg, //528

        // HDR Extended Metadata Packet for common port  //(0xXXXXX530~0xXXXXX5C8)only for 2nd common port (p0 register), 1st common port0 still use "Per port" r/w
        .common_hdrap                 = HDMI_P0_common_hdrap_reg, // 530
        .common_hdrdp                 = HDMI_P0_common_hdrdp_reg, // 534
        .common_hdr_st                = HDMI_P0_common_hdr_st_reg, // 538
        .common_em_ct                 = HDMI_P0_common_em_ct_reg, // 53c
        .common_fapa_ct               = HDMI_P0_common_fapa_ct_reg, // 540
        .common_fapa_ct_tmp           = HDMI_P0_common_fapa_ct_tmp_reg,
        .common_hdr_em_ct2            = HDMI_P0_common_HDR_EM_CT2_reg,
        .common_hdmi_vsem_emc         = HDMI_P0_common_HDMI_VSEM_EMC_reg, // 54c
        .common_hdmi_vsem_emc2        = HDMI_P0_common_HDMI_VSEM_EMC2_reg,  // 550
        .common_vsem_st               = HDMI_P0_common_VSEM_ST_reg, //554
        .common_hdmi_emap             = HDMI_P0_common_HDMI_EMAP_reg, //558
        .common_hdmi_emdp             = HDMI_P0_common_HDMI_EMDP_reg,
        .common_hd21_hdmi_hdrap       = HDMI_P0_common_hd21_HDMI_HDRAP,
        .common_hd21_hdmi_hdrdp       = HDMI_P0_common_hd21_HDMI_HDRDP,
        .common_hd21_hdr_em_st        = HDMI_P0_common_HD21_HDR_EM_ST_reg,
        .common_hd21_hdr_em_ct        = HDMI_P0_common_HD21_HDR_EM_CT_reg,
        .common_hd21_hdr_em_ct2       = HDMI_P0_common_HD21_HDR_EM_CT2_reg, //578
        .common_hd21_hdmi_emap        = HDMI_P0_common_HD21_HDMI_EMAP_reg,
        .common_hd21_hdmi_emdp        = HDMI_P0_common_hd21_HDMI_EMDP_reg,
        .common_hd21_vsem_st          = HDMI_P0_common_HD21_VSEM_ST_reg, //584
        .common_hd21_cvtem_st         = HDMI_P0_common_HD21_CVTEM_ST_reg,
        .common_hd21_hdmi_vsem_emc    = HDMI_P0_common_HD21_HDMI_VSEM_EMC_reg,
        .common_hd21_hdmi_cvtem_emc   = HDMI_P0_common_HD21_HDMI_CVTEM_EMC_reg,
        .common_hd21_hdmi_vsem_emc2   = HDMI_P0_common_HD21_HDMI_VSEM_EMC2_reg,

        // HDMI DSCD DBG Ctrl for common port
        .common_hd21_dscd_dbg         = HDMI_P0_common_hd21_DSCD_DBG_reg,
        .common_hd21_dscd_dbg_ap      = HDMI_P0_common_HD21_DSCD_DBG_AP_reg,
        .common_hd21_dscd_dbg_dp      = HDMI_P0_common_hd21_DSCD_DBG_DP_reg,

        // Audio Control for common port
        .common_hdmi_audcr            = HDMI_P0_common_HDMI_AUDCR_reg,
        .common_hdmi_audsr            = HDMI_P0_common_HDMI_AUDSR_reg,
        .common_hd21_hdmi_audcr       = HDMI_P0_common_HD21_HDMI_AUDCR_reg,
        .common_hd21_hdmi_audsr       = HDMI_P0_common_HD21_HDMI_AUDSR_reg,

        // HDMI Packet Status Register for common port
        .common_hdmi_gpvs             = HDMI_P0_common_HDMI_GPVS_reg,
        .common_hd21_hdmi_gpvs        = HDMI_P0_common_HD21_HDMI_GPVS_reg,

        // RM Zero Pading for common port
        .common_hd21_rmzp             = HDMI_P0_common_HD21_rmzp_reg,

        // Video Timing Generator Status Register for common port
        .common_hd21_timing_gen_sr    = HDMI_P0_common_HD21_TIMING_GEN_SR_reg, // 5c8

        // One Time Register  (No Function for MAC P1,P2,P3)
        .ot                           = HDMI_P0_ot_reg, // 600
        .irq                          = HDMI_P0_IRQ_reg,
        .mp                           = HDMI_P0_mp_reg,
        .hdmi_v_bypass                = HDMI_P0_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P0_hdmi_mp_reg,
        .debug_port                   = HDMI_P0_debug_port_reg,
        .debug_port_ctrl              = HDMI_P0_debug_port_ctrl_reg, //618
        .aud_common_port_switch       = HDMI_P0_AUD_COMMON_PORT_SWITCH_reg, //61c

        .hdmi_rst0                    = HDMI_P0_hdmi_rst0_reg,
        .hdmi_rst3                    = HDMI_P0_hdmi_rst3_reg,
        .hdmi_clken0                  = HDMI_P0_hdmi_clken0_reg,
        .hdmi_clken3                  = HDMI_P0_hdmi_clken3_reg,
    },
    // mac 1
    {
        .soft_0                       = HDMI_P1_SOFT_0_reg,
        .soft_1                       = HDMI_P1_SOFT_1_reg,
        // CRC Function
        .pp_tmds_crcc                 = HDMI_P1_PP_TMDS_CRCC_reg,
        .pp_tmds_crco0                = HDMI_P1_PP_TMDS_CRCO0_reg,
        .pp_tmds_crco1                = HDMI_P1_PP_TMDS_CRCO1_reg,

        // TMDS Channel Status
        .tmds_ctrl                    = HDMI_P1_TMDS_CTRL_reg,
        .tmds_outctl                  = HDMI_P1_TMDS_OUTCTL_reg,
        .tmds_pwdctl                  = HDMI_P1_TMDS_PWDCTL_reg,
        .tmds_z0cc                    = HDMI_P1_TMDS_Z0CC_reg,
        .tmds_cps                     = HDMI_P1_TMDS_CPS_reg,
        .tmds_udc                     = HDMI_P1_TMDS_UDC_reg,
        .tmds_errc                    = HDMI_P1_TMDS_ERRC_reg,
        .tmds_out_ctrl                = HDMI_P1_TMDS_OUT_CTRL_reg,
        .tmds_rout                    = HDMI_P1_TMDS_ROUT_reg,
        .tmds_gout                    = HDMI_P1_TMDS_GOUT_reg,
        .tmds_bout                    = HDMI_P1_TMDS_BOUT_reg,
        .tmds_dpc0                    = HDMI_P1_TMDS_DPC0_reg,
        .tmds_dpc1                    = HDMI_P1_TMDS_DPC1_reg,
        .tmds_dpc_set0                = HDMI_P1_TMDS_DPC_SET0_reg,
        .tmds_dpc_set1                = HDMI_P1_TMDS_DPC_SET1_reg,
        .tmds_dpc_set2                = HDMI_P1_TMDS_DPC_SET2_reg,
        .tmds_dpc_set3                = HDMI_P1_TMDS_DPC_SET3_reg,

        // Video Data Error Detect
        .video_bit_err_det            = HDMI_P1_VIDEO_BIT_ERR_DET_reg,
        .video_bit_err_status_b       = HDMI_P1_VIDEO_BIT_ERR_STATUS_B_reg,
        .video_bit_err_status_g       = HDMI_P1_VIDEO_BIT_ERR_STATUS_G_reg,
        .video_bit_err_status_r       = HDMI_P1_VIDEO_BIT_ERR_STATUS_R_reg,

        // TERC4 Error Detect
        .terc4_err_det                = HDMI_P1_TERC4_ERR_DET_reg,
        .terc4_err_status_b           = HDMI_P1_TERC4_ERR_STATUS_B_reg,
        .terc4_err_status_g           = HDMI_P1_TERC4_ERR_STATUS_G_reg,
        .terc4_err_status_r           = HDMI_P1_TERC4_ERR_STATUS_R_reg,

        // Flow and Packet
        .hdmi_sr                      = HDMI_P1_HDMI_SR_reg,
        .hdmi_gpvs                    = HDMI_P1_HDMI_GPVS_reg,
        .hdmi_gpvs1                   = HDMI_P1_HDMI_GPVS1_reg,
        .hdmi_psap                    = HDMI_P1_HDMI_PSAP_reg,
        .hdmi_psdp                    = HDMI_P1_HDMI_PSDP_reg,

        .hdmi_scr                     = HDMI_P1_HDMI_SCR_reg,
        .hdmi_bchcr                   = HDMI_P1_HDMI_BCHCR_reg,
        .hdmi_avmcr                   = HDMI_P1_HDMI_AVMCR_reg,
        .hdmi_wdcr                    = HDMI_P1_HDMI_WDCR_reg,

        .hdmi_pamicr                  = HDMI_P1_HDMI_PAMICR_reg,
        .hdmi_ptrsv1                  = HDMI_P1_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2                  = HDMI_P1_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3                  = HDMI_P1_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0                  = HDMI_P1_HDMI_PVGCR0_reg,
        .hdmi_pvsr0                   = HDMI_P1_HDMI_PVSR0_reg,
        .hdmi_vcr                     = HDMI_P1_HDMI_VCR_reg,
        .hdmi_acrcr                   = HDMI_P1_HDMI_ACRCR_reg,
        .hdmi_acrsr0                  = HDMI_P1_HDMI_ACRSR0_reg,
        .hdmi_acrsr1                  = HDMI_P1_HDMI_ACRSR1_reg,
        .hdmi_intcr                   = HDMI_P1_HDMI_INTCR_reg,

        // Packet Guard Band
        .hdmi_bcsr                    = HDMI_P1_HDMI_BCSR_reg,
        .hdmi_asr0                    = HDMI_P1_HDMI_ASR0_reg,
        .hdmi_asr1                    = HDMI_P1_HDMI_ASR1_reg,
        .hdmi_video_format            = HDMI_P1_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format               = HDMI_P1_HDMI_3D_FORMAT_reg,
        .hdmi_fvs                     = HDMI_P1_HDMI_FVS_reg,
        .hdmi_drm                     = HDMI_P1_HDMI_DRM_reg,
        .hdmi_pcmc                    = HDMI_P1_HDMI_PCMC_reg,

        // EMP
        .hdrap                        = HDMI_P1_hdrap_reg,
        .hdrdp                        = HDMI_P1_hdrdp_reg,
        .hdr_st                       = HDMI_P1_hdr_st_reg,
        .em_ct                        = HDMI_P1_em_ct_reg,
        .fapa_ct                      = HDMI_P1_fapa_ct_reg,
        .hdmi_emap                    = HDMI_P1_HDMI_EMAP_reg,
        .hdmi_emdp                    = HDMI_P1_HDMI_EMDP_reg,
        .hdmi_emap_pp                 = HDMI_P1_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp                 = HDMI_P1_HDMI_EMDP_PP_reg,
        .vrrem_st                     = HDMI_P1_VRREM_ST_reg,
        .hdmi_vrr_emc                 = HDMI_P1_HDMI_VRR_EMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI_P1_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet   = HDMI_P1_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr                   = HDMI_P1_HDMI_AUDCR_reg,
        .hdmi_audsr                   = HDMI_P1_HDMI_AUDSR_reg,

        // Clock Detection
        .clkdetcr                     = HDMI_P1_CLKDETCR_reg,
        .clkdetsr                     = HDMI_P1_CLKDETSR_reg,
        .clk_setting_00               = HDMI_P1_CLK_SETTING_00_reg,
        .clk_setting_01               = HDMI_P1_CLK_SETTING_01_reg,
        .clk_setting_02               = HDMI_P1_CLK_SETTING_02_reg,

        // RPBS
        .prbs_r_ctrl                  = HDMI_P1_PRBS_R_CTRL_reg,
        .prbs_g_ctrl                  = HDMI_P1_PRBS_G_CTRL_reg,
        .prbs_b_ctrl                  = HDMI_P1_PRBS_B_CTRL_reg,

        // MHL
        .hdmi_leading_gb_cmp_ctrl     = HDMI_P1_HDMI_leading_GB_cmp_ctrl_reg,
        .hdmi_leading_gb_cmp_times    = HDMI_P1_HDMI_leading_GB_cmp_times_reg,
        .hdmi_leading_gb_cmp_cnt      = HDMI_P1_HDMI_leading_GB_cmp_cnt_reg,
        .audio_cts_up_bound           = HDMI_P1_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound          = HDMI_P1_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound             = HDMI_P1_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound            = HDMI_P1_AUDIO_N_LOW_BOUND_reg,

        // BIST & FIFO
        .light_sleep                  = HDMI_P1_LIGHT_SLEEP_reg,
        .read_margin_enable           = HDMI_P1_READ_MARGIN_ENABLE_reg,
        .read_margin3                 = HDMI_P1_READ_MARGIN3_reg,
        .read_margin1                 = HDMI_P1_READ_MARGIN1_reg,
        .read_margin0                 = HDMI_P1_READ_MARGIN0_reg,
        .bist_mode                    = HDMI_P1_BIST_MODE_reg,
        .bist_done                    = HDMI_P1_BIST_DONE_reg,
        .bist_fail                    = HDMI_P1_BIST_FAIL_reg,
        .drf_mode                     = HDMI_P1_DRF_MODE_reg,
        .drf_resume                   = HDMI_P1_DRF_RESUME_reg,
        .drf_done                     = HDMI_P1_DRF_DONE_reg,
        .drf_pause                    = HDMI_P1_DRF_PAUSE_reg,
        .drf_fail                     = HDMI_P1_DRF_FAIL_reg,

        // PHY FIFO
        .phy_fifo_cr0                 = HDMI_P1_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1                 = HDMI_P1_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0                 = HDMI_P1_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1                 = HDMI_P1_PHY_FIFO_SR1_reg,
        .hdmi_cts_fifo_ctl            = HDMI_P1_HDMI_CTS_FIFO_CTL_reg,

        // CH FIFO
        .ch_cr                        = HDMI_P1_CH_CR_reg,
        .ch_sr                        = HDMI_P1_CH_SR_reg,

        //-------------------------------------------------------------
        // HDMI 2.0
        //-------------------------------------------------------------
        .hdmi_2p0_cr                  = HDMI_P1_HDMI_2p0_CR_reg,
        .hdmi_2p0_cr1                 = HDMI_P1_HDMI_2p0_CR1_reg,
        .scr_cr                       = HDMI_P1_SCR_CR_reg,

        // CED
        .cercr                        = HDMI_P1_CERCR_reg,
        .cersr0                       = HDMI_P1_CERSR0_reg,
        .cersr1                       = HDMI_P1_CERSR1_reg,
        .cersr2                       = HDMI_P1_CERSR2_reg,
        .cersr3                       = HDMI_P1_CERSR3_reg,
        .cersr4                       = HDMI_P1_CERSR4_reg,

        // YUV420 to 444
        .yuv420_cr                    = HDMI_P1_YUV420_CR_reg,
        .yuv420_cr1                   = HDMI_P1_YUV420_CR1_reg,

        // EMP
        .fapa_ct_tmp                  = HDMI_P1_fapa_ct_tmp_reg,
        .hdr_em_ct2                   = HDMI_P1_HDR_EM_CT2_reg,
        .hdmi_vsem_emc                = HDMI_P1_HDMI_VSEM_EMC_reg,
        .hdmi_vsem_emc2               = HDMI_P1_HDMI_VSEM_EMC2_reg,
        .vsem_st                      = HDMI_P1_VSEM_ST_reg,
        .hd20_ps_ct                   = HDMI_P1_HD20_ps_ct_reg,
        .hd20_ps_st                   = HDMI_P1_HD20_ps_st_reg,
        .irq_all_status               = HDMI_P1_irq_all_status_reg,
        .xtal_1ms                     = HDMI_P1_xtal_1ms_reg,
        .xtal_10ms                    = HDMI_P1_xtal_10ms_reg,
        .ps_measure_ctrl              = HDMI_P1_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk          = HDMI_P1_ps_measure_xtal_clk_reg,
        .ps_measure_tmds_clk          = HDMI_P1_ps_measure_tmds_clk_reg,
        .hdmi_pa_irq_en               = HDMI_P1_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta              = HDMI_P1_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt            = HDMI_P1_HDMI_PA_FRAME_CNT_reg,

        // HDMI scrmble control align
        .sc_align_b                   = HDMI_P1_SC_ALIGN_B_reg,
        .sc_align_r                   = HDMI_P1_SC_ALIGN_R_reg,
        .scr_ctr_per_lfsr             = HDMI_P1_SCR_CTR_PER_LFSR_reg,

        // HDCP 1.4
        .hdcp_keepout                 = HDMI_P1_HDCP_KEEPOUT_reg,
        .hdcp_cr                      = HDMI_P1_HDCP_CR_reg,
        .hdcp_cr2                     = HDMI_P1_HDCP_CR2_reg,
        .hdcp_pcr                     = HDMI_P1_HDCP_PCR_reg,
        .hdcp_flag1                   = HDMI_P1_HDCP_FLAG1_reg,
        .hdcp_flag2                   = HDMI_P1_HDCP_FLAG2_reg,

        // HDCP 2.2
        .hdcp_2p2_cr                  = HDMI_P1_HDCP_2p2_CR_reg,
        .hdcp_2p2_ks0                 = HDMI_P1_HDCP_2p2_ks0_reg,
        .hdcp_2p2_ks1                 = HDMI_P1_HDCP_2p2_ks1_reg,
        .hdcp_2p2_ks2                 = HDMI_P1_HDCP_2p2_ks2_reg,
        .hdcp_2p2_ks3                 = HDMI_P1_HDCP_2p2_ks3_reg,
        .hdcp_2p2_lc0                 = HDMI_P1_HDCP_2p2_lc0_reg,
        .hdcp_2p2_lc1                 = HDMI_P1_HDCP_2p2_lc1_reg,
        .hdcp_2p2_lc2                 = HDMI_P1_HDCP_2p2_lc2_reg,
        .hdcp_2p2_lc3                 = HDMI_P1_HDCP_2p2_lc3_reg,
        .hdcp_2p2_riv0                = HDMI_P1_HDCP_2p2_riv0_reg,
        .hdcp_2p2_riv1                = HDMI_P1_HDCP_2p2_riv1_reg,
        .hdcp_2p2_sr0                 = HDMI_P1_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1                 = HDMI_P1_HDCP_2p2_SR1_reg,
        .hdcp_hoa                     = HDMI_P1_HDCP_HOA_reg,
        .hdcp_power_saving_ctrl       = HDMI_P1_HDCP_POWER_SAVING_CTRL_reg,
        .win_opp_ctl                  = HDMI_P1_WIN_OPP_CTRL_reg,
        .hdmi_vpllcr0                 = HDMI_P1_HDMI_VPLLCR0_reg,
        .hdmi_vpllcr1                 = HDMI_P1_HDMI_VPLLCR1_reg,
        .hdmi_vpllcr2                 = HDMI_P1_HDMI_VPLLCR2_reg,
        .mn_sclkg_ctrl                = HDMI_P1_MN_SCLKG_CTRL_reg,
        .mn_sclkg_divs                = HDMI_P1_MN_SCLKG_DIVS_reg,
        .mdd_cr                       = HDMI_P1_MDD_CR_reg,
        .mdd_sr                       = HDMI_P1_MDD_SR_reg,
        .fw_func                      = HDMI_P1_FW_FUNC_reg,
        .port_switch                  = HDMI_P1_PORT_SWITCH_reg,
        .power_saving                 = HDMI_P1_POWER_SAVING_reg,
        .crc_ctrl                     = HDMI_P1_CRC_Ctrl_reg,
        .crc_result                   = HDMI_P1_CRC_Result_reg,
        .des_crc                      = HDMI_P1_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI_P1_CRC_ERR_CNT0_reg,
        .hdmi_20_21_ctrl              = HDMI_P1_hdmi_20_21_ctrl_reg,

        // YUV422_CD
        .yuv422_cd                    = HDMI_P1_YUV422_CD_reg, //528

        // HDR Extended Metadata Packet for common port  //(0xXXXXX530~0xXXXXX5C8)only for 2nd common port (p0 register), 1st common port0 still use "Per port" r/w
        .common_hdrap                 = HDMI_P1_common_hdrap_reg, // 530
        .common_hdrdp                 = HDMI_P1_common_hdrdp_reg, // 534
        .common_hdr_st                = HDMI_P1_common_hdr_st_reg, // 538
        .common_em_ct                 = HDMI_P1_common_em_ct_reg, // 53c
        .common_fapa_ct               = HDMI_P1_common_fapa_ct_reg, // 540
        .common_fapa_ct_tmp           = HDMI_P1_common_fapa_ct_tmp_reg,
        .common_hdr_em_ct2            = HDMI_P1_common_HDR_EM_CT2_reg,
        .common_hdmi_vsem_emc         = HDMI_P1_common_HDMI_VSEM_EMC_reg, // 54c
        .common_hdmi_vsem_emc2        = HDMI_P1_common_HDMI_VSEM_EMC2_reg,  // 550
        .common_vsem_st               = HDMI_P1_common_VSEM_ST_reg, //554
        .common_hdmi_emap             = HDMI_P1_common_HDMI_EMAP_reg, //558
        .common_hdmi_emdp             = HDMI_P1_common_HDMI_EMDP_reg,
        .common_hd21_hdmi_hdrap       = HDMI_P1_common_hd21_HDMI_HDRAP,
        .common_hd21_hdmi_hdrdp       = HDMI_P1_common_hd21_HDMI_HDRDP,
        .common_hd21_hdr_em_st        = HDMI_P1_common_HD21_HDR_EM_ST_reg,
        .common_hd21_hdr_em_ct        = HDMI_P1_common_HD21_HDR_EM_CT_reg,
        .common_hd21_hdr_em_ct2       = HDMI_P1_common_HD21_HDR_EM_CT2_reg, //578
        .common_hd21_hdmi_emap        = HDMI_P1_common_HD21_HDMI_EMAP_reg,
        .common_hd21_hdmi_emdp        = HDMI_P1_common_hd21_HDMI_EMDP_reg,
        .common_hd21_vsem_st          = HDMI_P1_common_HD21_VSEM_ST_reg, //584
        .common_hd21_cvtem_st         = HDMI_P1_common_HD21_CVTEM_ST_reg,
        .common_hd21_hdmi_vsem_emc    = HDMI_P1_common_HD21_HDMI_VSEM_EMC_reg,
        .common_hd21_hdmi_cvtem_emc   = HDMI_P1_common_HD21_HDMI_CVTEM_EMC_reg,
        .common_hd21_hdmi_vsem_emc2   = HDMI_P1_common_HD21_HDMI_VSEM_EMC2_reg,

        // HDMI DSCD DBG Ctrl for common port
        .common_hd21_dscd_dbg         = HDMI_P1_common_hd21_DSCD_DBG_reg,
        .common_hd21_dscd_dbg_ap      = HDMI_P1_common_HD21_DSCD_DBG_AP_reg,
        .common_hd21_dscd_dbg_dp      = HDMI_P1_common_hd21_DSCD_DBG_DP_reg,

        // Audio Control for common port
        .common_hdmi_audcr            = HDMI_P1_common_HDMI_AUDCR_reg,
        .common_hdmi_audsr            = HDMI_P1_common_HDMI_AUDSR_reg,
        .common_hd21_hdmi_audcr       = HDMI_P1_common_HD21_HDMI_AUDCR_reg,
        .common_hd21_hdmi_audsr       = HDMI_P1_common_HD21_HDMI_AUDSR_reg,

        // HDMI Packet Status Register for common port
        .common_hdmi_gpvs             = HDMI_P1_common_HDMI_GPVS_reg,
        .common_hd21_hdmi_gpvs        = HDMI_P1_common_HD21_HDMI_GPVS_reg,

        // RM Zero Pading for common port
        .common_hd21_rmzp             = HDMI_P1_common_HD21_rmzp_reg,

        // Video Timing Generator Status Register for common port
        .common_hd21_timing_gen_sr    = HDMI_P1_common_HD21_TIMING_GEN_SR_reg, // 5c8

        // One Time Register  (No Function for MAC P1,P2,P3)
        .ot                           = HDMI_P1_ot_reg, // 600
        .irq                          = HDMI_P1_IRQ_reg,
        .mp                           = HDMI_P1_mp_reg,
        .hdmi_v_bypass                = HDMI_P1_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P1_hdmi_mp_reg,
        .debug_port                   = HDMI_P1_debug_port_reg,
        .debug_port_ctrl              = HDMI_P1_debug_port_ctrl_reg, //618
        .aud_common_port_switch       = HDMI_P1_AUD_COMMON_PORT_SWITCH_reg, //61c

         // CRT
        .hdmi_rst0                    = HDMI_P1_hdmi_rst0_reg,
        .hdmi_rst3                    = HDMI_P1_hdmi_rst3_reg,
        .hdmi_clken0                  = HDMI_P1_hdmi_clken0_reg,
        .hdmi_clken3                  = HDMI_P1_hdmi_clken3_reg,
    },
    // mac 2
    {
        .soft_0                       = HDMI_P2_SOFT_0_reg,
        .soft_1                       = HDMI_P2_SOFT_1_reg,
        // CRC Function
        .pp_tmds_crcc                 = HDMI_P2_PP_TMDS_CRCC_reg,
        .pp_tmds_crco0                = HDMI_P2_PP_TMDS_CRCO0_reg,
        .pp_tmds_crco1                = HDMI_P2_PP_TMDS_CRCO1_reg,

        // TMDS Channel Status
        .tmds_ctrl                    = HDMI_P2_TMDS_CTRL_reg,
        .tmds_outctl                  = HDMI_P2_TMDS_OUTCTL_reg,
        .tmds_pwdctl                  = HDMI_P2_TMDS_PWDCTL_reg,
        .tmds_z0cc                    = HDMI_P2_TMDS_Z0CC_reg,
        .tmds_cps                     = HDMI_P2_TMDS_CPS_reg,
        .tmds_udc                     = HDMI_P2_TMDS_UDC_reg,
        .tmds_errc                    = HDMI_P2_TMDS_ERRC_reg,
        .tmds_out_ctrl                = HDMI_P2_TMDS_OUT_CTRL_reg,
        .tmds_rout                    = HDMI_P2_TMDS_ROUT_reg,
        .tmds_gout                    = HDMI_P2_TMDS_GOUT_reg,
        .tmds_bout                    = HDMI_P2_TMDS_BOUT_reg,
        .tmds_dpc0                    = HDMI_P2_TMDS_DPC0_reg,
        .tmds_dpc1                    = HDMI_P2_TMDS_DPC1_reg,
        .tmds_dpc_set0                = HDMI_P2_TMDS_DPC_SET0_reg,
        .tmds_dpc_set1                = HDMI_P2_TMDS_DPC_SET1_reg,
        .tmds_dpc_set2                = HDMI_P2_TMDS_DPC_SET2_reg,
        .tmds_dpc_set3                = HDMI_P2_TMDS_DPC_SET3_reg,

        // Video Data Error Detect
        .video_bit_err_det            = HDMI_P2_VIDEO_BIT_ERR_DET_reg,
        .video_bit_err_status_b       = HDMI_P2_VIDEO_BIT_ERR_STATUS_B_reg,
        .video_bit_err_status_g       = HDMI_P2_VIDEO_BIT_ERR_STATUS_G_reg,
        .video_bit_err_status_r       = HDMI_P2_VIDEO_BIT_ERR_STATUS_R_reg,

        // TERC4 Error Detect
        .terc4_err_det                = HDMI_P2_TERC4_ERR_DET_reg,
        .terc4_err_status_b           = HDMI_P2_TERC4_ERR_STATUS_B_reg,
        .terc4_err_status_g           = HDMI_P2_TERC4_ERR_STATUS_G_reg,
        .terc4_err_status_r           = HDMI_P2_TERC4_ERR_STATUS_R_reg,

        // Flow and Packet
        .hdmi_sr                      = HDMI_P2_HDMI_SR_reg,
        .hdmi_gpvs                    = HDMI_P2_HDMI_GPVS_reg,
        .hdmi_gpvs1                   = HDMI_P2_HDMI_GPVS1_reg,
        .hdmi_psap                    = HDMI_P2_HDMI_PSAP_reg,
        .hdmi_psdp                    = HDMI_P2_HDMI_PSDP_reg,

        .hdmi_scr                     = HDMI_P2_HDMI_SCR_reg,
        .hdmi_bchcr                   = HDMI_P2_HDMI_BCHCR_reg,
        .hdmi_avmcr                   = HDMI_P2_HDMI_AVMCR_reg,
        .hdmi_wdcr                    = HDMI_P2_HDMI_WDCR_reg,

        .hdmi_pamicr                  = HDMI_P2_HDMI_PAMICR_reg,
        .hdmi_ptrsv1                  = HDMI_P2_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2                  = HDMI_P2_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3                  = HDMI_P2_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0                  = HDMI_P2_HDMI_PVGCR0_reg,
        .hdmi_pvsr0                   = HDMI_P2_HDMI_PVSR0_reg,
        .hdmi_vcr                     = HDMI_P2_HDMI_VCR_reg,
        .hdmi_acrcr                   = HDMI_P2_HDMI_ACRCR_reg,
        .hdmi_acrsr0                  = HDMI_P2_HDMI_ACRSR0_reg,
        .hdmi_acrsr1                  = HDMI_P2_HDMI_ACRSR1_reg,
        .hdmi_intcr                   = HDMI_P2_HDMI_INTCR_reg,

        // Packet Guard Band
        .hdmi_bcsr                    = HDMI_P2_HDMI_BCSR_reg,
        .hdmi_asr0                    = HDMI_P2_HDMI_ASR0_reg,
        .hdmi_asr1                    = HDMI_P2_HDMI_ASR1_reg,
        .hdmi_video_format            = HDMI_P2_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format               = HDMI_P2_HDMI_3D_FORMAT_reg,
        .hdmi_fvs                     = HDMI_P2_HDMI_FVS_reg,
        .hdmi_drm                     = HDMI_P2_HDMI_DRM_reg,
        .hdmi_pcmc                    = HDMI_P2_HDMI_PCMC_reg,

        // EMP
        .hdrap                        = HDMI_P2_hdrap_reg,
        .hdrdp                        = HDMI_P2_hdrdp_reg,
        .hdr_st                       = HDMI_P2_hdr_st_reg,
        .em_ct                        = HDMI_P2_em_ct_reg,
        .fapa_ct                      = HDMI_P2_fapa_ct_reg,
        .hdmi_emap                    = HDMI_P2_HDMI_EMAP_reg,
        .hdmi_emdp                    = HDMI_P2_HDMI_EMDP_reg,
        .hdmi_emap_pp                 = HDMI_P2_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp                 = HDMI_P2_HDMI_EMDP_PP_reg,
        .vrrem_st                     = HDMI_P2_VRREM_ST_reg,
        .hdmi_vrr_emc                 = HDMI_P2_HDMI_VRR_EMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI_P2_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet   = HDMI_P2_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr                   = HDMI_P2_HDMI_AUDCR_reg,
        .hdmi_audsr                   = HDMI_P2_HDMI_AUDSR_reg,

        // Clock Detection
        .clkdetcr                     = HDMI_P2_CLKDETCR_reg,
        .clkdetsr                     = HDMI_P2_CLKDETSR_reg,
        .clk_setting_00               = HDMI_P2_CLK_SETTING_00_reg,
        .clk_setting_01               = HDMI_P2_CLK_SETTING_01_reg,
        .clk_setting_02               = HDMI_P2_CLK_SETTING_02_reg,

        // RPBS
        .prbs_r_ctrl                  = HDMI_P2_PRBS_R_CTRL_reg,
        .prbs_g_ctrl                  = HDMI_P2_PRBS_G_CTRL_reg,
        .prbs_b_ctrl                  = HDMI_P2_PRBS_B_CTRL_reg,

        // MHL
        .hdmi_leading_gb_cmp_ctrl     = HDMI_P2_HDMI_leading_GB_cmp_ctrl_reg,
        .hdmi_leading_gb_cmp_times    = HDMI_P2_HDMI_leading_GB_cmp_times_reg,
        .hdmi_leading_gb_cmp_cnt      = HDMI_P2_HDMI_leading_GB_cmp_cnt_reg,
        .audio_cts_up_bound           = HDMI_P2_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound          = HDMI_P2_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound             = HDMI_P2_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound            = HDMI_P2_AUDIO_N_LOW_BOUND_reg,

        // BIST & FIFO
        .light_sleep                  = HDMI_P2_LIGHT_SLEEP_reg,
        .read_margin_enable           = HDMI_P2_READ_MARGIN_ENABLE_reg,
        .read_margin3                 = HDMI_P2_READ_MARGIN3_reg,
        .read_margin1                 = HDMI_P2_READ_MARGIN1_reg,
        .read_margin0                 = HDMI_P2_READ_MARGIN0_reg,
        .bist_mode                    = HDMI_P2_BIST_MODE_reg,
        .bist_done                    = HDMI_P2_BIST_DONE_reg,
        .bist_fail                    = HDMI_P2_BIST_FAIL_reg,
        .drf_mode                     = HDMI_P2_DRF_MODE_reg,
        .drf_resume                   = HDMI_P2_DRF_RESUME_reg,
        .drf_done                     = HDMI_P2_DRF_DONE_reg,
        .drf_pause                    = HDMI_P2_DRF_PAUSE_reg,
        .drf_fail                     = HDMI_P2_DRF_FAIL_reg,

        // PHY FIFO
        .phy_fifo_cr0                 = HDMI_P2_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1                 = HDMI_P2_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0                 = HDMI_P2_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1                 = HDMI_P2_PHY_FIFO_SR1_reg,
        .hdmi_cts_fifo_ctl            = HDMI_P2_HDMI_CTS_FIFO_CTL_reg,

        // CH FIFO
        .ch_cr                        = HDMI_P2_CH_CR_reg,
        .ch_sr                        = HDMI_P2_CH_SR_reg,

        //-------------------------------------------------------------
        // HDMI 2.0
        //-------------------------------------------------------------
        .hdmi_2p0_cr                  = HDMI_P2_HDMI_2p0_CR_reg,
        .hdmi_2p0_cr1                 = HDMI_P2_HDMI_2p0_CR1_reg,
        .scr_cr                       = HDMI_P2_SCR_CR_reg,

        // CED
        .cercr                        = HDMI_P2_CERCR_reg,
        .cersr0                       = HDMI_P2_CERSR0_reg,
        .cersr1                       = HDMI_P2_CERSR1_reg,
        .cersr2                       = HDMI_P2_CERSR2_reg,
        .cersr3                       = HDMI_P2_CERSR3_reg,
        .cersr4                       = HDMI_P2_CERSR4_reg,

        // YUV420 to 444
        .yuv420_cr                    = HDMI_P2_YUV420_CR_reg,
        .yuv420_cr1                   = HDMI_P2_YUV420_CR1_reg,

        // EMP
        .fapa_ct_tmp                  = HDMI_P2_fapa_ct_tmp_reg,
        .hdr_em_ct2                   = HDMI_P2_HDR_EM_CT2_reg,
        .hdmi_vsem_emc                = HDMI_P2_HDMI_VSEM_EMC_reg,
        .hdmi_vsem_emc2               = HDMI_P2_HDMI_VSEM_EMC2_reg,
        .vsem_st                      = HDMI_P2_VSEM_ST_reg,
        .hd20_ps_ct                   = HDMI_P2_HD20_ps_ct_reg,
        .hd20_ps_st                   = HDMI_P2_HD20_ps_st_reg,
        .irq_all_status               = HDMI_P2_irq_all_status_reg,
        .xtal_1ms                     = HDMI_P2_xtal_1ms_reg,
        .xtal_10ms                    = HDMI_P2_xtal_10ms_reg,
        .ps_measure_ctrl              = HDMI_P2_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk          = HDMI_P2_ps_measure_xtal_clk_reg,
        .ps_measure_tmds_clk          = HDMI_P2_ps_measure_tmds_clk_reg,
        .hdmi_pa_irq_en               = HDMI_P2_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta              = HDMI_P2_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt            = HDMI_P2_HDMI_PA_FRAME_CNT_reg,

        // HDMI scrmble control align
        .sc_align_b                   = HDMI_P2_SC_ALIGN_B_reg,
        .sc_align_r                   = HDMI_P2_SC_ALIGN_R_reg,
        .scr_ctr_per_lfsr             = HDMI_P2_SCR_CTR_PER_LFSR_reg,

        // HDCP 1.4
        .hdcp_keepout                 = HDMI_P2_HDCP_KEEPOUT_reg,
        .hdcp_cr                      = HDMI_P2_HDCP_CR_reg,
        .hdcp_cr2                     = HDMI_P2_HDCP_CR2_reg,
        .hdcp_pcr                     = HDMI_P2_HDCP_PCR_reg,
        .hdcp_flag1                   = HDMI_P2_HDCP_FLAG1_reg,
        .hdcp_flag2                   = HDMI_P2_HDCP_FLAG2_reg,

        // HDCP 2.2
        .hdcp_2p2_cr                  = HDMI_P2_HDCP_2p2_CR_reg,
        .hdcp_2p2_ks0                 = HDMI_P2_HDCP_2p2_ks0_reg,
        .hdcp_2p2_ks1                 = HDMI_P2_HDCP_2p2_ks1_reg,
        .hdcp_2p2_ks2                 = HDMI_P2_HDCP_2p2_ks2_reg,
        .hdcp_2p2_ks3                 = HDMI_P2_HDCP_2p2_ks3_reg,
        .hdcp_2p2_lc0                 = HDMI_P2_HDCP_2p2_lc0_reg,
        .hdcp_2p2_lc1                 = HDMI_P2_HDCP_2p2_lc1_reg,
        .hdcp_2p2_lc2                 = HDMI_P2_HDCP_2p2_lc2_reg,
        .hdcp_2p2_lc3                 = HDMI_P2_HDCP_2p2_lc3_reg,
        .hdcp_2p2_riv0                = HDMI_P2_HDCP_2p2_riv0_reg,
        .hdcp_2p2_riv1                = HDMI_P2_HDCP_2p2_riv1_reg,
        .hdcp_2p2_sr0                 = HDMI_P2_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1                 = HDMI_P2_HDCP_2p2_SR1_reg,
        .hdcp_hoa                     = HDMI_P2_HDCP_HOA_reg,
        .hdcp_power_saving_ctrl       = HDMI_P2_HDCP_POWER_SAVING_CTRL_reg,
        .win_opp_ctl                  = HDMI_P2_WIN_OPP_CTRL_reg,
        .hdmi_vpllcr0                 = HDMI_P2_HDMI_VPLLCR0_reg,
        .hdmi_vpllcr1                 = HDMI_P2_HDMI_VPLLCR1_reg,
        .hdmi_vpllcr2                 = HDMI_P2_HDMI_VPLLCR2_reg,
        .mn_sclkg_ctrl                = HDMI_P2_MN_SCLKG_CTRL_reg,
        .mn_sclkg_divs                = HDMI_P2_MN_SCLKG_DIVS_reg,
        .mdd_cr                       = HDMI_P2_MDD_CR_reg,
        .mdd_sr                       = HDMI_P2_MDD_SR_reg,
        .fw_func                      = HDMI_P2_FW_FUNC_reg,
        .port_switch                  = HDMI_P2_PORT_SWITCH_reg,
        .power_saving                 = HDMI_P2_POWER_SAVING_reg,
        .crc_ctrl                     = HDMI_P2_CRC_Ctrl_reg,
        .crc_result                   = HDMI_P2_CRC_Result_reg,
        .des_crc                      = HDMI_P2_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI_P2_CRC_ERR_CNT0_reg,
        .hdmi_20_21_ctrl              = HDMI_P2_hdmi_20_21_ctrl_reg,

        // YUV422_CD
        .yuv422_cd                    = HDMI_P2_YUV422_CD_reg, //528

        // HDR Extended Metadata Packet for common port  //(0xXXXXX530~0xXXXXX5C8)only for 2nd common port (p0 register), 1st common port0 still use "Per port" r/w
        .common_hdrap                 = HDMI_P2_common_hdrap_reg, // 530
        .common_hdrdp                 = HDMI_P2_common_hdrdp_reg, // 534
        .common_hdr_st                = HDMI_P2_common_hdr_st_reg, // 538
        .common_em_ct                 = HDMI_P2_common_em_ct_reg, // 53c
        .common_fapa_ct               = HDMI_P2_common_fapa_ct_reg, // 540
        .common_fapa_ct_tmp           = HDMI_P2_common_fapa_ct_tmp_reg,
        .common_hdr_em_ct2            = HDMI_P2_common_HDR_EM_CT2_reg,
        .common_hdmi_vsem_emc         = HDMI_P2_common_HDMI_VSEM_EMC_reg, // 54c
        .common_hdmi_vsem_emc2        = HDMI_P2_common_HDMI_VSEM_EMC2_reg,  // 550
        .common_vsem_st               = HDMI_P2_common_VSEM_ST_reg, //554
        .common_hdmi_emap             = HDMI_P2_common_HDMI_EMAP_reg, //558
        .common_hdmi_emdp             = HDMI_P2_common_HDMI_EMDP_reg,
        .common_hd21_hdmi_hdrap       = HDMI_P2_common_hd21_HDMI_HDRAP,
        .common_hd21_hdmi_hdrdp       = HDMI_P2_common_hd21_HDMI_HDRDP,
        .common_hd21_hdr_em_st        = HDMI_P2_common_HD21_HDR_EM_ST_reg,
        .common_hd21_hdr_em_ct        = HDMI_P2_common_HD21_HDR_EM_CT_reg,
        .common_hd21_hdr_em_ct2       = HDMI_P2_common_HD21_HDR_EM_CT2_reg, //578
        .common_hd21_hdmi_emap        = HDMI_P2_common_HD21_HDMI_EMAP_reg,
        .common_hd21_hdmi_emdp        = HDMI_P2_common_hd21_HDMI_EMDP_reg,
        .common_hd21_vsem_st          = HDMI_P2_common_HD21_VSEM_ST_reg, //584
        .common_hd21_cvtem_st         = HDMI_P2_common_HD21_CVTEM_ST_reg,
        .common_hd21_hdmi_vsem_emc    = HDMI_P2_common_HD21_HDMI_VSEM_EMC_reg,
        .common_hd21_hdmi_cvtem_emc   = HDMI_P2_common_HD21_HDMI_CVTEM_EMC_reg,
        .common_hd21_hdmi_vsem_emc2   = HDMI_P2_common_HD21_HDMI_VSEM_EMC2_reg,

        // HDMI DSCD DBG Ctrl for common port
        .common_hd21_dscd_dbg         = HDMI_P2_common_hd21_DSCD_DBG_reg,
        .common_hd21_dscd_dbg_ap      = HDMI_P2_common_HD21_DSCD_DBG_AP_reg,
        .common_hd21_dscd_dbg_dp      = HDMI_P2_common_hd21_DSCD_DBG_DP_reg,

        // Audio Control for common port
        .common_hdmi_audcr            = HDMI_P2_common_HDMI_AUDCR_reg,
        .common_hdmi_audsr            = HDMI_P2_common_HDMI_AUDSR_reg,
        .common_hd21_hdmi_audcr       = HDMI_P2_common_HD21_HDMI_AUDCR_reg,
        .common_hd21_hdmi_audsr       = HDMI_P2_common_HD21_HDMI_AUDSR_reg,

        // HDMI Packet Status Register for common port
        .common_hdmi_gpvs             = HDMI_P2_common_HDMI_GPVS_reg,
        .common_hd21_hdmi_gpvs        = HDMI_P2_common_HD21_HDMI_GPVS_reg,

        // RM Zero Pading for common port
        .common_hd21_rmzp             = HDMI_P2_common_HD21_rmzp_reg,

        // Video Timing Generator Status Register for common port
        .common_hd21_timing_gen_sr    = HDMI_P2_common_HD21_TIMING_GEN_SR_reg, // 5c8

        // One Time Register  (No Function for MAC P1,P2,P3)
        .ot                           = HDMI_P2_ot_reg, // 600
        .irq                          = HDMI_P2_IRQ_reg,
        .mp                           = HDMI_P2_mp_reg,
        .hdmi_v_bypass                = HDMI_P2_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P2_hdmi_mp_reg,
        .debug_port                   = HDMI_P2_debug_port_reg,
        .debug_port_ctrl              = HDMI_P2_debug_port_ctrl_reg, //618
        .aud_common_port_switch       = HDMI_P2_AUD_COMMON_PORT_SWITCH_reg, //61c

         // CRT
        .hdmi_rst0                    = HDMI_P2_hdmi_rst0_reg,
        .hdmi_rst3                    = HDMI_P2_hdmi_rst3_reg,
        .hdmi_clken0                  = HDMI_P2_hdmi_clken0_reg,
        .hdmi_clken3                  = HDMI_P2_hdmi_clken3_reg,
    },
    // mac 3
    {
        .soft_0                       = HDMI_P3_SOFT_0_reg,
        .soft_1                       = HDMI_P3_SOFT_1_reg,
        // CRC Function
        .pp_tmds_crcc                 = HDMI_P3_PP_TMDS_CRCC_reg,
        .pp_tmds_crco0                = HDMI_P3_PP_TMDS_CRCO0_reg,
        .pp_tmds_crco1                = HDMI_P3_PP_TMDS_CRCO1_reg,

        // TMDS Channel Status
        .tmds_ctrl                    = HDMI_P3_TMDS_CTRL_reg,
        .tmds_outctl                  = HDMI_P3_TMDS_OUTCTL_reg,
        .tmds_pwdctl                  = HDMI_P3_TMDS_PWDCTL_reg,
        .tmds_z0cc                    = HDMI_P3_TMDS_Z0CC_reg,
        .tmds_cps                     = HDMI_P3_TMDS_CPS_reg,
        .tmds_udc                     = HDMI_P3_TMDS_UDC_reg,
        .tmds_errc                    = HDMI_P3_TMDS_ERRC_reg,
        .tmds_out_ctrl                = HDMI_P3_TMDS_OUT_CTRL_reg,
        .tmds_rout                    = HDMI_P3_TMDS_ROUT_reg,
        .tmds_gout                    = HDMI_P3_TMDS_GOUT_reg,
        .tmds_bout                    = HDMI_P3_TMDS_BOUT_reg,
        .tmds_dpc0                    = HDMI_P3_TMDS_DPC0_reg,
        .tmds_dpc1                    = HDMI_P3_TMDS_DPC1_reg,
        .tmds_dpc_set0                = HDMI_P3_TMDS_DPC_SET0_reg,
        .tmds_dpc_set1                = HDMI_P3_TMDS_DPC_SET1_reg,
        .tmds_dpc_set2                = HDMI_P3_TMDS_DPC_SET2_reg,
        .tmds_dpc_set3                = HDMI_P3_TMDS_DPC_SET3_reg,

        // Video Data Error Detect
        .video_bit_err_det            = HDMI_P3_VIDEO_BIT_ERR_DET_reg,
        .video_bit_err_status_b       = HDMI_P3_VIDEO_BIT_ERR_STATUS_B_reg,
        .video_bit_err_status_g       = HDMI_P3_VIDEO_BIT_ERR_STATUS_G_reg,
        .video_bit_err_status_r       = HDMI_P3_VIDEO_BIT_ERR_STATUS_R_reg,

        // TERC4 Error Detect
        .terc4_err_det                = HDMI_P3_TERC4_ERR_DET_reg,
        .terc4_err_status_b           = HDMI_P3_TERC4_ERR_STATUS_B_reg,
        .terc4_err_status_g           = HDMI_P3_TERC4_ERR_STATUS_G_reg,
        .terc4_err_status_r           = HDMI_P3_TERC4_ERR_STATUS_R_reg,

        // Flow and Packet
        .hdmi_sr                      = HDMI_P3_HDMI_SR_reg,
        .hdmi_gpvs                    = HDMI_P3_HDMI_GPVS_reg,
        .hdmi_gpvs1                   = HDMI_P3_HDMI_GPVS1_reg,
        .hdmi_psap                    = HDMI_P3_HDMI_PSAP_reg,
        .hdmi_psdp                    = HDMI_P3_HDMI_PSDP_reg,

        .hdmi_scr                     = HDMI_P3_HDMI_SCR_reg,
        .hdmi_bchcr                   = HDMI_P3_HDMI_BCHCR_reg,
        .hdmi_avmcr                   = HDMI_P3_HDMI_AVMCR_reg,
        .hdmi_wdcr                    = HDMI_P3_HDMI_WDCR_reg,

        .hdmi_pamicr                  = HDMI_P3_HDMI_PAMICR_reg,
        .hdmi_ptrsv1                  = HDMI_P3_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2                  = HDMI_P3_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3                  = HDMI_P3_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0                  = HDMI_P3_HDMI_PVGCR0_reg,
        .hdmi_pvsr0                   = HDMI_P3_HDMI_PVSR0_reg,
        .hdmi_vcr                     = HDMI_P3_HDMI_VCR_reg,
        .hdmi_acrcr                   = HDMI_P3_HDMI_ACRCR_reg,
        .hdmi_acrsr0                  = HDMI_P3_HDMI_ACRSR0_reg,
        .hdmi_acrsr1                  = HDMI_P3_HDMI_ACRSR1_reg,
        .hdmi_intcr                   = HDMI_P3_HDMI_INTCR_reg,

        // Packet Guard Band
        .hdmi_bcsr                    = HDMI_P3_HDMI_BCSR_reg,
        .hdmi_asr0                    = HDMI_P3_HDMI_ASR0_reg,
        .hdmi_asr1                    = HDMI_P3_HDMI_ASR1_reg,
        .hdmi_video_format            = HDMI_P3_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format               = HDMI_P3_HDMI_3D_FORMAT_reg,
        .hdmi_fvs                     = HDMI_P3_HDMI_FVS_reg,
        .hdmi_drm                     = HDMI_P3_HDMI_DRM_reg,
        .hdmi_pcmc                    = HDMI_P3_HDMI_PCMC_reg,

        // EMP
        .hdrap                        = HDMI_P3_hdrap_reg,
        .hdrdp                        = HDMI_P3_hdrdp_reg,
        .hdr_st                       = HDMI_P3_hdr_st_reg,
        .em_ct                        = HDMI_P3_em_ct_reg,
        .fapa_ct                      = HDMI_P3_fapa_ct_reg,
        .hdmi_emap                    = HDMI_P3_HDMI_EMAP_reg,
        .hdmi_emdp                    = HDMI_P3_HDMI_EMDP_reg,
        .hdmi_emap_pp                 = HDMI_P3_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp                 = HDMI_P3_HDMI_EMDP_PP_reg,
        .vrrem_st                     = HDMI_P3_VRREM_ST_reg,
        .hdmi_vrr_emc                 = HDMI_P3_HDMI_VRR_EMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI_P3_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet   = HDMI_P3_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr                   = HDMI_P3_HDMI_AUDCR_reg,
        .hdmi_audsr                   = HDMI_P3_HDMI_AUDSR_reg,

        // Clock Detection
        .clkdetcr                     = HDMI_P3_CLKDETCR_reg,
        .clkdetsr                     = HDMI_P3_CLKDETSR_reg,
        .clk_setting_00               = HDMI_P3_CLK_SETTING_00_reg,
        .clk_setting_01               = HDMI_P3_CLK_SETTING_01_reg,
        .clk_setting_02               = HDMI_P3_CLK_SETTING_02_reg,

        // RPBS
        .prbs_r_ctrl                  = HDMI_P3_PRBS_R_CTRL_reg,
        .prbs_g_ctrl                  = HDMI_P3_PRBS_G_CTRL_reg,
        .prbs_b_ctrl                  = HDMI_P3_PRBS_B_CTRL_reg,

        // MHL
        .hdmi_leading_gb_cmp_ctrl     = HDMI_P3_HDMI_leading_GB_cmp_ctrl_reg,
        .hdmi_leading_gb_cmp_times    = HDMI_P3_HDMI_leading_GB_cmp_times_reg,
        .hdmi_leading_gb_cmp_cnt      = HDMI_P3_HDMI_leading_GB_cmp_cnt_reg,
        .audio_cts_up_bound           = HDMI_P3_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound          = HDMI_P3_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound             = HDMI_P3_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound            = HDMI_P3_AUDIO_N_LOW_BOUND_reg,

        // BIST & FIFO
        .light_sleep                  = HDMI_P3_LIGHT_SLEEP_reg,
        .read_margin_enable           = HDMI_P3_READ_MARGIN_ENABLE_reg,
        .read_margin3                 = HDMI_P3_READ_MARGIN3_reg,
        .read_margin1                 = HDMI_P3_READ_MARGIN1_reg,
        .read_margin0                 = HDMI_P3_READ_MARGIN0_reg,
        .bist_mode                    = HDMI_P3_BIST_MODE_reg,
        .bist_done                    = HDMI_P3_BIST_DONE_reg,
        .bist_fail                    = HDMI_P3_BIST_FAIL_reg,
        .drf_mode                     = HDMI_P3_DRF_MODE_reg,
        .drf_resume                   = HDMI_P3_DRF_RESUME_reg,
        .drf_done                     = HDMI_P3_DRF_DONE_reg,
        .drf_pause                    = HDMI_P3_DRF_PAUSE_reg,
        .drf_fail                     = HDMI_P3_DRF_FAIL_reg,

        // PHY FIFO
        .phy_fifo_cr0                 = HDMI_P3_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1                 = HDMI_P3_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0                 = HDMI_P3_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1                 = HDMI_P3_PHY_FIFO_SR1_reg,
        .hdmi_cts_fifo_ctl            = HDMI_P3_HDMI_CTS_FIFO_CTL_reg,

        // CH FIFO
        .ch_cr                        = HDMI_P3_CH_CR_reg,
        .ch_sr                        = HDMI_P3_CH_SR_reg,

        //-------------------------------------------------------------
        // HDMI 2.0
        //-------------------------------------------------------------
        .hdmi_2p0_cr                  = HDMI_P3_HDMI_2p0_CR_reg,
        .hdmi_2p0_cr1                 = HDMI_P3_HDMI_2p0_CR1_reg,
        .scr_cr                       = HDMI_P3_SCR_CR_reg,

        // CED
        .cercr                        = HDMI_P3_CERCR_reg,
        .cersr0                       = HDMI_P3_CERSR0_reg,
        .cersr1                       = HDMI_P3_CERSR1_reg,
        .cersr2                       = HDMI_P3_CERSR2_reg,
        .cersr3                       = HDMI_P3_CERSR3_reg,
        .cersr4                       = HDMI_P3_CERSR4_reg,

        // YUV420 to 444
        .yuv420_cr                    = HDMI_P3_YUV420_CR_reg,
        .yuv420_cr1                   = HDMI_P3_YUV420_CR1_reg,

        // EMP
        .fapa_ct_tmp                  = HDMI_P3_fapa_ct_tmp_reg,
        .hdr_em_ct2                   = HDMI_P3_HDR_EM_CT2_reg,
        .hdmi_vsem_emc                = HDMI_P3_HDMI_VSEM_EMC_reg,
        .hdmi_vsem_emc2               = HDMI_P3_HDMI_VSEM_EMC2_reg,
        .vsem_st                      = HDMI_P3_VSEM_ST_reg,
        .hd20_ps_ct                   = HDMI_P3_HD20_ps_ct_reg,
        .hd20_ps_st                   = HDMI_P3_HD20_ps_st_reg,
        .irq_all_status               = HDMI_P3_irq_all_status_reg,
        .xtal_1ms                     = HDMI_P3_xtal_1ms_reg,
        .xtal_10ms                    = HDMI_P3_xtal_10ms_reg,
        .ps_measure_ctrl              = HDMI_P3_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk          = HDMI_P3_ps_measure_xtal_clk_reg,
        .ps_measure_tmds_clk          = HDMI_P3_ps_measure_tmds_clk_reg,
        .hdmi_pa_irq_en               = HDMI_P3_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta              = HDMI_P3_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt            = HDMI_P3_HDMI_PA_FRAME_CNT_reg,

        // HDMI scrmble control align
        .sc_align_b                   = HDMI_P3_SC_ALIGN_B_reg,
        .sc_align_r                   = HDMI_P3_SC_ALIGN_R_reg,
        .scr_ctr_per_lfsr             = HDMI_P3_SCR_CTR_PER_LFSR_reg,

        // HDCP 1.4
        .hdcp_keepout                 = HDMI_P3_HDCP_KEEPOUT_reg,
        .hdcp_cr                      = HDMI_P3_HDCP_CR_reg,
        .hdcp_cr2                     = HDMI_P3_HDCP_CR2_reg,
        .hdcp_pcr                     = HDMI_P3_HDCP_PCR_reg,
        .hdcp_flag1                   = HDMI_P3_HDCP_FLAG1_reg,
        .hdcp_flag2                   = HDMI_P3_HDCP_FLAG2_reg,

        // HDCP 2.2
        .hdcp_2p2_cr                  = HDMI_P3_HDCP_2p2_CR_reg,
        .hdcp_2p2_ks0                 = HDMI_P3_HDCP_2p2_ks0_reg,
        .hdcp_2p2_ks1                 = HDMI_P3_HDCP_2p2_ks1_reg,
        .hdcp_2p2_ks2                 = HDMI_P3_HDCP_2p2_ks2_reg,
        .hdcp_2p2_ks3                 = HDMI_P3_HDCP_2p2_ks3_reg,
        .hdcp_2p2_lc0                 = HDMI_P3_HDCP_2p2_lc0_reg,
        .hdcp_2p2_lc1                 = HDMI_P3_HDCP_2p2_lc1_reg,
        .hdcp_2p2_lc2                 = HDMI_P3_HDCP_2p2_lc2_reg,
        .hdcp_2p2_lc3                 = HDMI_P3_HDCP_2p2_lc3_reg,
        .hdcp_2p2_riv0                = HDMI_P3_HDCP_2p2_riv0_reg,
        .hdcp_2p2_riv1                = HDMI_P3_HDCP_2p2_riv1_reg,
        .hdcp_2p2_sr0                 = HDMI_P3_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1                 = HDMI_P3_HDCP_2p2_SR1_reg,
        .hdcp_hoa                     = HDMI_P3_HDCP_HOA_reg,
        .hdcp_power_saving_ctrl       = HDMI_P3_HDCP_POWER_SAVING_CTRL_reg,
        .win_opp_ctl                  = HDMI_P3_WIN_OPP_CTRL_reg,
        .hdmi_vpllcr0                 = HDMI_P3_HDMI_VPLLCR0_reg,
        .hdmi_vpllcr1                 = HDMI_P3_HDMI_VPLLCR1_reg,
        .hdmi_vpllcr2                 = HDMI_P3_HDMI_VPLLCR2_reg,
        .mn_sclkg_ctrl                = HDMI_P3_MN_SCLKG_CTRL_reg,
        .mn_sclkg_divs                = HDMI_P3_MN_SCLKG_DIVS_reg,
        .mdd_cr                       = HDMI_P3_MDD_CR_reg,
        .mdd_sr                       = HDMI_P3_MDD_SR_reg,
        .fw_func                      = HDMI_P3_FW_FUNC_reg,
        .port_switch                  = HDMI_P3_PORT_SWITCH_reg,
        .power_saving                 = HDMI_P3_POWER_SAVING_reg,
        .crc_ctrl                     = HDMI_P3_CRC_Ctrl_reg,
        .crc_result                   = HDMI_P3_CRC_Result_reg,
        .des_crc                      = HDMI_P3_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI_P3_CRC_ERR_CNT0_reg,
        .hdmi_20_21_ctrl              = HDMI_P3_hdmi_20_21_ctrl_reg,

        // YUV422_CD
        .yuv422_cd                    = HDMI_P3_YUV422_CD_reg, //528

        // HDR Extended Metadata Packet for common port  //(0xXXXXX530~0xXXXXX5C8)only for 2nd common port (p0 register), 1st common port0 still use "Per port" r/w
        .common_hdrap                 = HDMI_P3_common_hdrap_reg, // 530
        .common_hdrdp                 = HDMI_P3_common_hdrdp_reg, // 534
        .common_hdr_st                = HDMI_P3_common_hdr_st_reg, // 538
        .common_em_ct                 = HDMI_P3_common_em_ct_reg, // 53c
        .common_fapa_ct               = HDMI_P3_common_fapa_ct_reg, // 540
        .common_fapa_ct_tmp           = HDMI_P3_common_fapa_ct_tmp_reg,
        .common_hdr_em_ct2            = HDMI_P3_common_HDR_EM_CT2_reg,
        .common_hdmi_vsem_emc         = HDMI_P3_common_HDMI_VSEM_EMC_reg, // 54c
        .common_hdmi_vsem_emc2        = HDMI_P3_common_HDMI_VSEM_EMC2_reg,  // 550
        .common_vsem_st               = HDMI_P3_common_VSEM_ST_reg, //554
        .common_hdmi_emap             = HDMI_P3_common_HDMI_EMAP_reg, //558
        .common_hdmi_emdp             = HDMI_P3_common_HDMI_EMDP_reg,
        .common_hd21_hdmi_hdrap       = HDMI_P3_common_hd21_HDMI_HDRAP,
        .common_hd21_hdmi_hdrdp       = HDMI_P3_common_hd21_HDMI_HDRDP,
        .common_hd21_hdr_em_st        = HDMI_P3_common_HD21_HDR_EM_ST_reg,
        .common_hd21_hdr_em_ct        = HDMI_P3_common_HD21_HDR_EM_CT_reg,
        .common_hd21_hdr_em_ct2       = HDMI_P3_common_HD21_HDR_EM_CT2_reg, //578
        .common_hd21_hdmi_emap        = HDMI_P3_common_HD21_HDMI_EMAP_reg,
        .common_hd21_hdmi_emdp        = HDMI_P3_common_hd21_HDMI_EMDP_reg,
        .common_hd21_vsem_st          = HDMI_P3_common_HD21_VSEM_ST_reg, //584
        .common_hd21_cvtem_st         = HDMI_P3_common_HD21_CVTEM_ST_reg,
        .common_hd21_hdmi_vsem_emc    = HDMI_P3_common_HD21_HDMI_VSEM_EMC_reg,
        .common_hd21_hdmi_cvtem_emc   = HDMI_P3_common_HD21_HDMI_CVTEM_EMC_reg,
        .common_hd21_hdmi_vsem_emc2   = HDMI_P3_common_HD21_HDMI_VSEM_EMC2_reg,

        // HDMI DSCD DBG Ctrl for common port
        .common_hd21_dscd_dbg         = HDMI_P3_common_hd21_DSCD_DBG_reg,
        .common_hd21_dscd_dbg_ap      = HDMI_P3_common_HD21_DSCD_DBG_AP_reg,
        .common_hd21_dscd_dbg_dp      = HDMI_P3_common_hd21_DSCD_DBG_DP_reg,

        // Audio Control for common port
        .common_hdmi_audcr            = HDMI_P3_common_HDMI_AUDCR_reg,
        .common_hdmi_audsr            = HDMI_P3_common_HDMI_AUDSR_reg,
        .common_hd21_hdmi_audcr       = HDMI_P3_common_HD21_HDMI_AUDCR_reg,
        .common_hd21_hdmi_audsr       = HDMI_P3_common_HD21_HDMI_AUDSR_reg,

        // HDMI Packet Status Register for common port
        .common_hdmi_gpvs             = HDMI_P3_common_HDMI_GPVS_reg,
        .common_hd21_hdmi_gpvs        = HDMI_P3_common_HD21_HDMI_GPVS_reg,

        // RM Zero Pading for common port
        .common_hd21_rmzp             = HDMI_P3_common_HD21_rmzp_reg,

        // Video Timing Generator Status Register for common port
        .common_hd21_timing_gen_sr    = HDMI_P3_common_HD21_TIMING_GEN_SR_reg, // 5c8

        // One Time Register  (No Function for MAC P1,P3,P3)
        .ot                           = HDMI_P3_ot_reg, // 600
        .irq                          = HDMI_P3_IRQ_reg,
        .mp                           = HDMI_P3_mp_reg,
        .hdmi_v_bypass                = HDMI_P3_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P3_hdmi_mp_reg,
        .debug_port                   = HDMI_P3_debug_port_reg,
        .debug_port_ctrl              = HDMI_P3_debug_port_ctrl_reg, //618
        .aud_common_port_switch       = HDMI_P3_AUD_COMMON_PORT_SWITCH_reg, //61c

         // CRT
        .hdmi_rst0                    = HDMI_P3_hdmi_rst0_reg,
        .hdmi_rst3                    = HDMI_P3_hdmi_rst3_reg,
        .hdmi_clken0                  = HDMI_P3_hdmi_clken0_reg,
        .hdmi_clken3                  = HDMI_P3_hdmi_clken3_reg,
    },
};

/*--------------------------------------------------
 * HDMI2.0 MEAS register setting
 *--------------------------------------------------*/
const struct hdmi20_meas_reg_st hd20_meas[HDMI20_MAC_NUM] =
{
    // HDMI2.0 MEAS0
    {
        .frl_measure_0 = HDMI20_P0_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI20_P0_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI20_P0_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI20_P0_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI20_P0_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI20_P0_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI20_P0_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI20_P0_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI20_P0_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI20_P0_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI20_P0_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI20_P0_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI20_P0_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI20_P0_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI20_P0_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI20_P0_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.0 MEAS1
    {
        .frl_measure_0 = HDMI20_P1_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI20_P1_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI20_P1_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI20_P1_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI20_P1_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI20_P1_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI20_P1_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI20_P1_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI20_P1_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI20_P1_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI20_P1_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI20_P1_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI20_P1_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI20_P1_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI20_P1_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI20_P1_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.0 MEAS2
    {
        .frl_measure_0 = HDMI20_P2_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI20_P2_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI20_P2_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI20_P2_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI20_P2_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI20_P2_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI20_P2_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI20_P2_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI20_P2_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI20_P2_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI20_P2_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI20_P2_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI20_P2_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI20_P2_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI20_P2_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI20_P2_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.0 MEAS3
    {
        .frl_measure_0 = HDMI20_P3_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI20_P3_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI20_P3_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI20_P3_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI20_P3_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI20_P3_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI20_P3_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI20_P3_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI20_P3_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI20_P3_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI20_P3_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI20_P3_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI20_P3_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI20_P3_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI20_P3_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI20_P3_MEAS_FRL_MEASURE_18_reg
    }
};

const struct hdmi_stb_mac_reg_st stb_mac[4] =
{
    // STB MAC0
    {
        .scdc_cr                  = HDMI_STB_P0_SCDC_CR_reg,
        .scdc_pcr                 = HDMI_STB_P0_SCDC_PCR_reg,
        .scdc_ap                  = HDMI_STB_P0_SCDC_AP_reg,
        .scdc_dp                  = HDMI_STB_P0_SCDC_DP_reg,
        .hdcp_cr                  = HDMI_STB_P0_HDCP_CR_reg,
        .hdcp_dkap                = HDMI_STB_P0_HDCP_DKAP_reg,
        .hdcp_pcr                 = HDMI_STB_P0_HDCP_PCR_reg,
        .hdcp_flag1               = HDMI_STB_P0_HDCP_FLAG1_reg,
        .hdcp_flag2               = HDMI_STB_P0_HDCP_FLAG2_reg,
        .hdcp_ap                  = HDMI_STB_P0_HDCP_AP_reg,
        .hdcp_dp                  = HDMI_STB_P0_HDCP_DP_reg,
        .hdcp_2p2_cr              = HDMI_STB_P0_HDCP_2p2_CR_reg,
        .hdcp_2p2_sr0             = HDMI_STB_P0_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1             = HDMI_STB_P0_HDCP_2p2_SR1_reg,
        .hdcp_msap                = HDMI_STB_P0_HDCP_MSAP_reg,
        .hdcp_msdp                = HDMI_STB_P0_HDCP_MSDP_reg,
        .hdcp_pend                = HDMI_STB_P0_HDCP_PEND_reg,
        .mbist_st0                = HDMI_STB_P0_MBIST_ST0_reg,
        .mbist_st1                = HDMI_STB_P0_MBIST_ST1_reg,
        .mbist_st2                = HDMI_STB_P0_MBIST_ST2_reg,
        .mbist_st3                = HDMI_STB_P0_MBIST_ST3_reg,
        .stb_xtal_1ms             = HDMI_STB_P0_stb_xtal_1ms_reg,
        .stb_xtal_4p7us           = HDMI_STB_P0_stb_xtal_4p7us_reg,
        .hdmi_stb_dbgbox_ctl      = HDMI_STB_P0_HDMI_STB_DBGBOX_CTL_reg,
        .clkdetsr                 = HDMI_STB_P0_CLKDETSR_reg,
        .gdi_tmdsclk_setting_00   = HDMI_STB_P0_GDI_TMDSCLK_SETTING_00_reg,
        .gdi_tmdsclk_setting_01   = HDMI_STB_P0_GDI_TMDSCLK_SETTING_01_reg,
        .gdi_tmdsclk_setting_02   = HDMI_STB_P0_GDI_TMDSCLK_SETTING_02_reg,
        .ot                       = HDMI_STB_P0_ot_reg,
        .hdmi_stb_dbgbox_port_sel = HDMI_STB_P0_HDMI_STB_DBGBOX_PORT_SEL_reg,
        .clken_hdmi_stb           = HDMI_STB_P0_clken_hdmi_stb_reg,
        .rst_n_hdmi_stb           = HDMI_STB_P0_rst_n_hdmi_stb_reg,
        .hdmi_mp                  = HDMI_STB_P0_hdmi_mp_reg,
    },
    // STB MAC1
    {
        .scdc_cr                  = HDMI_STB_P1_SCDC_CR_reg,
        .scdc_pcr                 = HDMI_STB_P1_SCDC_PCR_reg,
        .scdc_ap                  = HDMI_STB_P1_SCDC_AP_reg,
        .scdc_dp                  = HDMI_STB_P1_SCDC_DP_reg,
        .hdcp_cr                  = HDMI_STB_P1_HDCP_CR_reg,
        .hdcp_dkap                = HDMI_STB_P1_HDCP_DKAP_reg,
        .hdcp_pcr                 = HDMI_STB_P1_HDCP_PCR_reg,
        .hdcp_flag1               = HDMI_STB_P1_HDCP_FLAG1_reg,
        .hdcp_flag2               = HDMI_STB_P1_HDCP_FLAG2_reg,
        .hdcp_ap                  = HDMI_STB_P1_HDCP_AP_reg,
        .hdcp_dp                  = HDMI_STB_P1_HDCP_DP_reg,
        .hdcp_2p2_cr              = HDMI_STB_P1_HDCP_2p2_CR_reg,
        .hdcp_2p2_sr0             = HDMI_STB_P1_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1             = HDMI_STB_P1_HDCP_2p2_SR1_reg,
        .hdcp_msap                = HDMI_STB_P1_HDCP_MSAP_reg,
        .hdcp_msdp                = HDMI_STB_P1_HDCP_MSDP_reg,
        .hdcp_pend                = HDMI_STB_P1_HDCP_PEND_reg,
        .mbist_st0                = HDMI_STB_P1_MBIST_ST0_reg,
        .mbist_st1                = HDMI_STB_P1_MBIST_ST1_reg,
        .mbist_st2                = HDMI_STB_P1_MBIST_ST2_reg,
        .mbist_st3                = HDMI_STB_P1_MBIST_ST3_reg,
        .stb_xtal_1ms             = HDMI_STB_P1_stb_xtal_1ms_reg,
        .stb_xtal_4p7us           = HDMI_STB_P1_stb_xtal_4p7us_reg,
        .hdmi_stb_dbgbox_ctl      = HDMI_STB_P1_HDMI_STB_DBGBOX_CTL_reg,
        .clkdetsr                 = HDMI_STB_P1_CLKDETSR_reg,
        .gdi_tmdsclk_setting_00   = HDMI_STB_P1_GDI_TMDSCLK_SETTING_00_reg,
        .gdi_tmdsclk_setting_01   = HDMI_STB_P1_GDI_TMDSCLK_SETTING_01_reg,
        .gdi_tmdsclk_setting_02   = HDMI_STB_P1_GDI_TMDSCLK_SETTING_02_reg,
        .ot                       = HDMI_STB_P1_ot_reg,
        .hdmi_stb_dbgbox_port_sel = HDMI_STB_P1_HDMI_STB_DBGBOX_PORT_SEL_reg,
        .clken_hdmi_stb           = HDMI_STB_P1_clken_hdmi_stb_reg,
        .rst_n_hdmi_stb           = HDMI_STB_P1_rst_n_hdmi_stb_reg,
        .hdmi_mp                  = HDMI_STB_P1_hdmi_mp_reg,
    },
    // STB MAC2
    {
        .scdc_cr                  = HDMI_STB_P2_SCDC_CR_reg,
        .scdc_pcr                 = HDMI_STB_P2_SCDC_PCR_reg,
        .scdc_ap                  = HDMI_STB_P2_SCDC_AP_reg,
        .scdc_dp                  = HDMI_STB_P2_SCDC_DP_reg,
        .hdcp_cr                  = HDMI_STB_P2_HDCP_CR_reg,
        .hdcp_dkap                = HDMI_STB_P2_HDCP_DKAP_reg,
        .hdcp_pcr                 = HDMI_STB_P2_HDCP_PCR_reg,
        .hdcp_flag1               = HDMI_STB_P2_HDCP_FLAG1_reg,
        .hdcp_flag2               = HDMI_STB_P2_HDCP_FLAG2_reg,
        .hdcp_ap                  = HDMI_STB_P2_HDCP_AP_reg,
        .hdcp_dp                  = HDMI_STB_P2_HDCP_DP_reg,
        .hdcp_2p2_cr              = HDMI_STB_P2_HDCP_2p2_CR_reg,
        .hdcp_2p2_sr0             = HDMI_STB_P2_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1             = HDMI_STB_P2_HDCP_2p2_SR1_reg,
        .hdcp_msap                = HDMI_STB_P2_HDCP_MSAP_reg,
        .hdcp_msdp                = HDMI_STB_P2_HDCP_MSDP_reg,
        .hdcp_pend                = HDMI_STB_P2_HDCP_PEND_reg,
        .mbist_st0                = HDMI_STB_P2_MBIST_ST0_reg,
        .mbist_st1                = HDMI_STB_P2_MBIST_ST1_reg,
        .mbist_st2                = HDMI_STB_P2_MBIST_ST2_reg,
        .mbist_st3                = HDMI_STB_P2_MBIST_ST3_reg,
        .stb_xtal_1ms             = HDMI_STB_P2_stb_xtal_1ms_reg,
        .stb_xtal_4p7us           = HDMI_STB_P2_stb_xtal_4p7us_reg,
        .hdmi_stb_dbgbox_ctl      = HDMI_STB_P2_HDMI_STB_DBGBOX_CTL_reg,
        .clkdetsr                 = HDMI_STB_P2_CLKDETSR_reg,
        .gdi_tmdsclk_setting_00   = HDMI_STB_P2_GDI_TMDSCLK_SETTING_00_reg,
        .gdi_tmdsclk_setting_01   = HDMI_STB_P2_GDI_TMDSCLK_SETTING_01_reg,
        .gdi_tmdsclk_setting_02   = HDMI_STB_P2_GDI_TMDSCLK_SETTING_02_reg,
        .ot                       = HDMI_STB_P2_ot_reg,
        .hdmi_stb_dbgbox_port_sel = HDMI_STB_P2_HDMI_STB_DBGBOX_PORT_SEL_reg,
        .clken_hdmi_stb           = HDMI_STB_P2_clken_hdmi_stb_reg,
        .rst_n_hdmi_stb           = HDMI_STB_P2_rst_n_hdmi_stb_reg,
        .hdmi_mp                  = HDMI_STB_P2_hdmi_mp_reg,
    },
    // STB MAC3
    {
        .scdc_cr                  = HDMI_STB_P3_SCDC_CR_reg,
        .scdc_pcr                 = HDMI_STB_P3_SCDC_PCR_reg,
        .scdc_ap                  = HDMI_STB_P3_SCDC_AP_reg,
        .scdc_dp                  = HDMI_STB_P3_SCDC_DP_reg,
        .hdcp_cr                  = HDMI_STB_P3_HDCP_CR_reg,
        .hdcp_dkap                = HDMI_STB_P3_HDCP_DKAP_reg,
        .hdcp_pcr                 = HDMI_STB_P3_HDCP_PCR_reg,
        .hdcp_flag1               = HDMI_STB_P3_HDCP_FLAG1_reg,
        .hdcp_flag2               = HDMI_STB_P3_HDCP_FLAG2_reg,
        .hdcp_ap                  = HDMI_STB_P3_HDCP_AP_reg,
        .hdcp_dp                  = HDMI_STB_P3_HDCP_DP_reg,
        .hdcp_2p2_cr              = HDMI_STB_P3_HDCP_2p2_CR_reg,
        .hdcp_2p2_sr0             = HDMI_STB_P3_HDCP_2p2_SR0_reg,
        .hdcp_2p2_sr1             = HDMI_STB_P3_HDCP_2p2_SR1_reg,
        .hdcp_msap                = HDMI_STB_P3_HDCP_MSAP_reg,
        .hdcp_msdp                = HDMI_STB_P3_HDCP_MSDP_reg,
        .hdcp_pend                = HDMI_STB_P3_HDCP_PEND_reg,
        .mbist_st0                = HDMI_STB_P3_MBIST_ST0_reg,
        .mbist_st1                = HDMI_STB_P3_MBIST_ST1_reg,
        .mbist_st2                = HDMI_STB_P3_MBIST_ST2_reg,
        .mbist_st3                = HDMI_STB_P3_MBIST_ST3_reg,
        .stb_xtal_1ms             = HDMI_STB_P3_stb_xtal_1ms_reg,
        .stb_xtal_4p7us           = HDMI_STB_P3_stb_xtal_4p7us_reg,
        .hdmi_stb_dbgbox_ctl      = HDMI_STB_P3_HDMI_STB_DBGBOX_CTL_reg,
        .clkdetsr                 = HDMI_STB_P3_CLKDETSR_reg,
        .gdi_tmdsclk_setting_00   = HDMI_STB_P3_GDI_TMDSCLK_SETTING_00_reg,
        .gdi_tmdsclk_setting_01   = HDMI_STB_P3_GDI_TMDSCLK_SETTING_01_reg,
        .gdi_tmdsclk_setting_02   = HDMI_STB_P3_GDI_TMDSCLK_SETTING_02_reg,
        .ot                       = HDMI_STB_P3_ot_reg,
        .hdmi_stb_dbgbox_port_sel = HDMI_STB_P3_HDMI_STB_DBGBOX_PORT_SEL_reg,
        .clken_hdmi_stb           = HDMI_STB_P3_clken_hdmi_stb_reg,
        .rst_n_hdmi_stb           = HDMI_STB_P3_rst_n_hdmi_stb_reg,
        .hdmi_mp                  = HDMI_STB_P3_hdmi_mp_reg,
    },
};

/*--------------------------------------------------
 * HDMI2.1 Mac register setting
 *--------------------------------------------------*/
const struct hdmi21_mac_reg_st hd21_mac[HDMI21_MAC_NUM] =
{
    // HDMI2.1 mac0
    {
        // HDMI_2p1 exist or not
        .valid = 1,

        // HDMI_2p1_control_register
        .cr = HDMI21_P0_HD21_CR_reg,

        // Video Part
        .phy_fifo_cr0 = HDMI21_P0_HD21_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1 = HDMI21_P0_HD21_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0 = HDMI21_P0_HD21_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1 = HDMI21_P0_HD21_PHY_FIFO_SR1_reg,

        // Data Align
        .channel_align_c = HDMI21_P0_hd21_channel_align_c_reg,
        .channel_align_r = HDMI21_P0_hd21_channel_align_r_reg,
        .channel_align_g = HDMI21_P0_hd21_channel_align_g_reg,
        .channel_align_b = HDMI21_P0_hd21_channel_align_b_reg,

        // Channel Sync
        .ch_sync_ctrl = HDMI21_P0_hd21_ch_sync_ctrl_reg,
        .ch_sync_status = HDMI21_P0_hd21_ch_sync_status_reg,
        .ch_sync_r = HDMI21_P0_hd21_ch_sync_r_reg,
        .ch_sync_g = HDMI21_P0_hd21_ch_sync_g_reg,
        .ch_sync_b = HDMI21_P0_hd21_ch_sync_b_reg,
        .ch_sync_c = HDMI21_P0_hd21_ch_sync_c_reg,

        // Error Detect Function
        .errdet_r_ctrl = HDMI21_P0_hd21_errdet_r_ctrl_reg,
        .errdet_g_ctrl = HDMI21_P0_hd21_errdet_g_ctrl_reg,
        .errdet_b_ctrl = HDMI21_P0_hd21_errdet_b_ctrl_reg,
        .errdet_c_ctrl = HDMI21_P0_hd21_errdet_c_ctrl_reg,
        .errdet_r_disparity = HDMI21_P0_hd21_errdet_r_disparity_reg,
        .errdet_g_disparity = HDMI21_P0_hd21_errdet_g_disparity_reg,
        .errdet_b_disparity = HDMI21_P0_hd21_errdet_b_disparity_reg,
        .errdet_c_disparity = HDMI21_P0_hd21_errdet_c_disparity_reg,
        .errdet_r_char = HDMI21_P0_hd21_errdet_r_char_reg,
        .errdet_g_char = HDMI21_P0_hd21_errdet_g_char_reg,
        .errdet_b_char = HDMI21_P0_hd21_errdet_b_char_reg,
        .errdet_c_char = HDMI21_P0_hd21_errdet_c_char_reg,
        .disp_ctrl = HDMI21_P0_hd21_disp_ctrl_reg,

        // FRL Status
        .mm_cr = HDMI21_P0_HD21_MM_CR_reg,
        .mm_sr = HDMI21_P0_HD21_MM_SR_reg,

        // Extended Metadata Packet
        .hdmi_hdrap = HDMI21_P0_hd21_HDMI_HDRAP_reg,
        .hdmi_hdrdp = HDMI21_P0_hd21_HDMI_HDRDP_reg,
        .fapa_ct2 = HDMI21_P0_HD21_fapa_ct2_reg,
        .hdr_em_st = HDMI21_P0_HD21_HDR_EM_ST_reg, 
        .hdr_em_ct = HDMI21_P0_HD21_HDR_EM_CT_reg,
        .hdmi_emap = HDMI21_P0_HD21_HDMI_EMAP_reg,
        .hdmi_emdp = HDMI21_P0_hd21_HDMI_EMDP_reg,
        .hdmi_emap_pp = HDMI21_P0_HD21_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp = HDMI21_P0_hd21_HDMI_EMDP_PP_reg,
        .fapa_ct = HDMI21_P0_HD21_fapa_ct_reg,
        .hdr_em_ct2 = HDMI21_P0_HD21_HDR_EM_CT2_reg, 

        // Error DET
        .cercr = HDMI21_P0_hd21_cercr_reg,
        .cersr0 = HDMI21_P0_HD21_CERSR0_reg,
        .cersr1 = HDMI21_P0_HD21_CERSR1_reg,
        .cersr2 = HDMI21_P0_HD21_CERSR2_reg,

        .rmzp = HDMI21_P0_HD21_rmzp_reg,

        // Extended Metadata Packet 2
        .vsem_st = HDMI21_P0_HD21_VSEM_ST_reg,
        .vrrem_st = HDMI21_P0_HD21_VRREM_ST_reg,
        .cvtem_st = HDMI21_P0_HD21_CVTEM_ST_reg,
        .hdmi_vsem_emc = HDMI21_P0_HD21_HDMI_VSEM_EMC_reg,
        .hdmi_vrr_emc = HDMI21_P0_HD21_HDMI_VRR_EMC_reg,
        .hdmi_cvtem_emc = HDMI21_P0_HD21_HDMI_CVTEM_EMC_reg,
        .hdmi_vsem_emc2 = HDMI21_P0_HD21_HDMI_VSEM_EMC2_reg,

        // CRC Control Register
        .crc_ctrl                     = HDMI21_P0_HD21_CRC_Ctrl_reg,
        .crc_result                   = HDMI21_P0_HD21_CRC_Result_reg,
        .des_crc                      = HDMI21_P0_HD21_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI21_P0_HD21_CRC_ERR_CNT0_reg,

        // Video Timing Generator
        .timing_gen_cr = HDMI21_P0_HD21_TIMING_GEN_CR_reg,
        .timing_gen_sr = HDMI21_P0_HD21_TIMING_GEN_SR_reg,

        // Channel Status
        .ch_st = HDMI21_P0_HD21_CH_ST_reg,

        // PRBS
        .prbs_r_ctrl = HDMI21_P0_HD21_PRBS_R_CTRL_reg,
        .prbs_g_ctrl = HDMI21_P0_HD21_PRBS_G_CTRL_reg,
        .prbs_b_ctrl = HDMI21_P0_HD21_PRBS_B_CTRL_reg,
        .prbs_c_ctrl = HDMI21_P0_HD21_PRBS_C_CTRL_reg,
        .prbs_r_status = HDMI21_P0_HD21_PRBS_R_STATUS_reg,
        .prbs_g_status = HDMI21_P0_HD21_PRBS_G_STATUS_reg,
        .prbs_b_status = HDMI21_P0_HD21_PRBS_B_STATUS_reg,
        .prbs_c_status = HDMI21_P0_HD21_PRBS_C_STATUS_reg,

        // DSCD_DBG
        .dscd_dbg = HDMI21_P0_hd21_DSCD_DBG_reg,
        .dscd_dbg_ap = HDMI21_P0_HD21_DSCD_DBG_AP_reg,
        .dscd_dbg_dp = HDMI21_P0_hd21_DSCD_DBG_DP_reg,

        // LINK TRAINING
        .lt_r = HDMI21_P0_HD21_LT_R_reg,
        .lt_g = HDMI21_P0_HD21_LT_G_reg,
        .lt_b = HDMI21_P0_HD21_LT_B_reg,
        .lt_c = HDMI21_P0_HD21_LT_C_reg,
        .lt_all = HDMI21_P0_HD21_LT_ALL_reg,

        // Debug Selection
        .udc = HDMI21_P0_HD21_UDC_reg,

        //Deep Color
        .tmds_dpc0 = HDMI21_P0_HD21_TMDS_DPC0_reg,
        .tmds_dpc1 = HDMI21_P0_HD21_TMDS_DPC1_reg,
        .tmds_dpc_set0 = HDMI21_P0_HD21_TMDS_DPC_SET0_reg,

        // Packet Status
        .hdmi_sr = HDMI21_P0_HD21_HDMI_SR_reg,
        .hdmi_gpvs = HDMI21_P0_HD21_HDMI_GPVS_reg,
        .hdmi_gpvs1 = HDMI21_P0_HD21_HDMI_GPVS1_reg,
        .hdmi_psap = HDMI21_P0_HD21_HDMI_PSAP_reg,
        .hdmi_psdp = HDMI21_P0_HD21_HDMI_PSDP_reg,

        // Mode Control
        .hdmi_scr = HDMI21_P0_HD21_HDMI_SCR_reg,

        // BCH Control
        .hdmi_bchcr = HDMI21_P0_HD21_HDMI_BCHCR_reg,

        // AV Mute Control
        .hdmi_avmcr = HDMI21_P0_HD21_HDMI_AVMCR_reg,
        .hdmi_wdcr = HDMI21_P0_HD21_HDMI_WDCR_reg,

        // Packet Variation
        .hdmi_pamicr = HDMI21_P0_HD21_HDMI_PAMICR_reg,
        .hdmi_ptrsv1 = HDMI21_P0_HD21_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2 = HDMI21_P0_HD21_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3 = HDMI21_P0_HD21_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0 = HDMI21_P0_HD21_HDMI_PVGCR0_reg,
        .hdmi_pvsr0 = HDMI21_P0_HD21_HDMI_PVSR0_reg,

        // Video Flow control
        .hdmi_vcr = HDMI21_P0_HD21_HDMI_VCR_reg,

        // Audio Clock Regeneration Packet
        .hdmi_acrcr = HDMI21_P0_HD21_HDMI_ACRCR_reg,
        .hdmi_acrsr0 = HDMI21_P0_HD21_HDMI_ACRSR0_reg,
        .hdmi_acrsr1 = HDMI21_P0_HD21_HDMI_ACRSR1_reg,

        // IRQ
        .hdmi_intcr = HDMI21_P0_HD21_HDMI_INTCR_reg,

        //Packet/Guard Band Error
        .hdmi_bcsr = HDMI21_P0_HD21_HDMI_BCSR_reg,

        // Packet Content
        .hdmi_asr0 = HDMI21_P0_HD21_HDMI_ASR0_reg,
        .hdmi_asr1 = HDMI21_P0_HD21_HDMI_ASR1_reg,
        .hdmi_video_format = HDMI21_P0_HD21_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format = HDMI21_P0_HD21_HDMI_3D_FORMAT_reg,
        .hdmi_fvs = HDMI21_P0_HD21_HDMI_FVS_reg,
        .hdmi_drm = HDMI21_P0_HD21_HDMI_DRM_reg,
        .hdmi_pcmc = HDMI21_P0_HD21_HDMI_PCMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI21_P0_HD21_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet = HDMI21_P0_HD21_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr = HDMI21_P0_HD21_HDMI_AUDCR_reg,
        .hdmi_audsr = HDMI21_P0_HD21_HDMI_AUDSR_reg,
        .audio_cts_up_bound = HDMI21_P0_HD21_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound = HDMI21_P0_HD21_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound = HDMI21_P0_HD21_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound = HDMI21_P0_HD21_AUDIO_N_LOW_BOUND_reg,

        // CLKDET part
        .clkdetcr = HDMI21_P0_HD21_CLKDETCR_reg,
        .clkdetsr = HDMI21_P0_HD21_CLKDETSR_reg,
        .clk_setting_00 = HDMI21_P0_HD21_CLK_SETTING_00_reg,
        .clk_setting_01 = HDMI21_P0_HD21_CLK_SETTING_01_reg,
        .clk_setting_02 = HDMI21_P0_HD21_CLK_SETTING_02_reg,
        .clkdet2cr = HDMI21_P0_HD21_CLKDET2CR_reg,
        .clkdet2sr = HDMI21_P0_HD21_CLKDET2SR_reg,
        .clk2_setting_00 = HDMI21_P0_HD21_CLK2_SETTING_00_reg,
        .clk2_setting_01 = HDMI21_P0_HD21_CLK2_SETTING_01_reg,
        .clk2_setting_02 = HDMI21_P0_HD21_CLK2_SETTING_02_reg,

        // CLK High Width Measure
        .clk_hwm_setting_00 = HDMI21_P0_HD21_CLK_HWM_SETTING_00_reg,
        .clk_hwm_setting_01 = HDMI21_P0_HD21_CLK_HWM_SETTING_01_reg,
        .clk_hwm_setting_02 = HDMI21_P0_HD21_CLK_HWM_SETTING_02_reg,
        .clk_hwn_one_st_00 = HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_reg,
        .clk_hwn_cont_st_01 = HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_reg,

        // HDMI TMP part
        .fapa_ct_tmp = HDMI21_P0_HD21_fapa_ct_tmp_reg,
        .ps_ct = HDMI21_P0_HD21_ps_ct_reg,
        .ps_st = HDMI21_P0_HD21_ps_st_reg,
        .irq_all_status = HDMI21_P0_hd21_irq_all_status_reg,

        // XTAL CNT
        .xtal_1s = HDMI21_P0_hd21_xtal_1s_reg,
        .xtal_280ms = HDMI21_P0_hd21_xtal_280ms_reg,
        .xtal_10ms = HDMI21_P0_hd21_xtal_10ms_reg,

        // Power Saving Measure
        .ps_measure_ctrl = HDMI21_P0_hd21_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk0 = HDMI21_P0_hd21_ps_measure_xtal_clk0_reg,
        .ps_measure_xtal_clk1 = HDMI21_P0_hd21_ps_measure_xtal_clk1_reg,
        .ps_measure_xtal_clk2 = HDMI21_P0_hd21_ps_measure_xtal_clk2_reg,

        // Packet Absence
        .hdmi_pa_irq_en = HDMI21_P0_HD21_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta = HDMI21_P0_HD21_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt = HDMI21_P0_HD21_HDMI_PA_FRAME_CNT_reg
    },
    // HDMI2.1 mac 1
    {
        // HDMI_2p1 exist or not
        .valid = 1,

        // HDMI_2p1_control_register
        .cr = HDMI21_P1_HD21_CR_reg,

        // Video Part
        .phy_fifo_cr0 = HDMI21_P1_HD21_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1 = HDMI21_P1_HD21_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0 = HDMI21_P1_HD21_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1 = HDMI21_P1_HD21_PHY_FIFO_SR1_reg,

        // Data Align
        .channel_align_c = HDMI21_P1_hd21_channel_align_c_reg,
        .channel_align_r = HDMI21_P1_hd21_channel_align_r_reg,
        .channel_align_g = HDMI21_P1_hd21_channel_align_g_reg,
        .channel_align_b = HDMI21_P1_hd21_channel_align_b_reg,

        // Channel Sync
        .ch_sync_ctrl = HDMI21_P1_hd21_ch_sync_ctrl_reg,
        .ch_sync_status = HDMI21_P1_hd21_ch_sync_status_reg,
        .ch_sync_r = HDMI21_P1_hd21_ch_sync_r_reg,
        .ch_sync_g = HDMI21_P1_hd21_ch_sync_g_reg,
        .ch_sync_b = HDMI21_P1_hd21_ch_sync_b_reg,
        .ch_sync_c = HDMI21_P1_hd21_ch_sync_c_reg,

        // Error Detect Function
        .errdet_r_ctrl = HDMI21_P1_hd21_errdet_r_ctrl_reg,
        .errdet_g_ctrl = HDMI21_P1_hd21_errdet_g_ctrl_reg,
        .errdet_b_ctrl = HDMI21_P1_hd21_errdet_b_ctrl_reg,
        .errdet_c_ctrl = HDMI21_P1_hd21_errdet_c_ctrl_reg,
        .errdet_r_disparity = HDMI21_P1_hd21_errdet_r_disparity_reg,
        .errdet_g_disparity = HDMI21_P1_hd21_errdet_g_disparity_reg,
        .errdet_b_disparity = HDMI21_P1_hd21_errdet_b_disparity_reg,
        .errdet_c_disparity = HDMI21_P1_hd21_errdet_c_disparity_reg,
        .errdet_r_char = HDMI21_P1_hd21_errdet_r_char_reg,
        .errdet_g_char = HDMI21_P1_hd21_errdet_g_char_reg,
        .errdet_b_char = HDMI21_P1_hd21_errdet_b_char_reg,
        .errdet_c_char = HDMI21_P1_hd21_errdet_c_char_reg,
        .disp_ctrl = HDMI21_P1_hd21_disp_ctrl_reg,

        // FRL Status
        .mm_cr = HDMI21_P1_HD21_MM_CR_reg,
        .mm_sr = HDMI21_P1_HD21_MM_SR_reg,

        // Extended Metadata Packet
        .hdmi_hdrap = HDMI21_P1_hd21_HDMI_HDRAP_reg,
        .hdmi_hdrdp = HDMI21_P1_hd21_HDMI_HDRDP_reg,
        .fapa_ct2 = 0, // Only avaliable on P0
        .hdr_em_st = HDMI21_P1_HD21_HDR_EM_ST_reg, 
        .hdr_em_ct = HDMI21_P1_HD21_HDR_EM_CT_reg,
        .hdmi_emap = HDMI21_P1_HD21_HDMI_EMAP_reg,
        .hdmi_emdp = HDMI21_P1_hd21_HDMI_EMDP_reg,
        .hdmi_emap_pp = HDMI21_P1_HD21_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp = HDMI21_P1_hd21_HDMI_EMDP_PP_reg,
        .fapa_ct = HDMI21_P1_HD21_fapa_ct_reg,
        .hdr_em_ct2 = HDMI21_P1_HD21_HDR_EM_CT2_reg, 

        // Error DET
        .cercr = HDMI21_P1_hd21_cercr_reg,
        .cersr0 = HDMI21_P1_HD21_CERSR0_reg,
        .cersr1 = HDMI21_P1_HD21_CERSR1_reg,
        .cersr2 = HDMI21_P1_HD21_CERSR2_reg,

        .rmzp = HDMI21_P1_HD21_rmzp_reg,

        // Extended Metadata Packet 2
        .vsem_st = HDMI21_P1_HD21_VSEM_ST_reg,
        .vrrem_st = HDMI21_P1_HD21_VRREM_ST_reg,
        .cvtem_st = HDMI21_P1_HD21_CVTEM_ST_reg,
        .hdmi_vsem_emc = HDMI21_P1_HD21_HDMI_VSEM_EMC_reg,
        .hdmi_vrr_emc = HDMI21_P1_HD21_HDMI_VRR_EMC_reg,
        .hdmi_cvtem_emc = HDMI21_P1_HD21_HDMI_CVTEM_EMC_reg,
        .hdmi_vsem_emc2 = HDMI21_P1_HD21_HDMI_VSEM_EMC2_reg,

        // CRC Control Register
        .crc_ctrl                     = HDMI21_P1_HD21_CRC_Ctrl_reg,
        .crc_result                   = HDMI21_P1_HD21_CRC_Result_reg,
        .des_crc                      = HDMI21_P1_HD21_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI21_P1_HD21_CRC_ERR_CNT0_reg,

        // Video Timing Generator
        .timing_gen_cr = HDMI21_P1_HD21_TIMING_GEN_CR_reg,
        .timing_gen_sr = HDMI21_P1_HD21_TIMING_GEN_SR_reg,

        // Channel Status
        .ch_st = HDMI21_P1_HD21_CH_ST_reg,

        // PRBS
        .prbs_r_ctrl = HDMI21_P1_HD21_PRBS_R_CTRL_reg,
        .prbs_g_ctrl = HDMI21_P1_HD21_PRBS_G_CTRL_reg,
        .prbs_b_ctrl = HDMI21_P1_HD21_PRBS_B_CTRL_reg,
        .prbs_c_ctrl = HDMI21_P1_HD21_PRBS_C_CTRL_reg,
        .prbs_r_status = HDMI21_P1_HD21_PRBS_R_STATUS_reg,
        .prbs_g_status = HDMI21_P1_HD21_PRBS_G_STATUS_reg,
        .prbs_b_status = HDMI21_P1_HD21_PRBS_B_STATUS_reg,
        .prbs_c_status = HDMI21_P1_HD21_PRBS_C_STATUS_reg,

        // DSCD_DBG
        .dscd_dbg = HDMI21_P1_hd21_DSCD_DBG_reg,
        .dscd_dbg_ap = HDMI21_P1_HD21_DSCD_DBG_AP_reg,
        .dscd_dbg_dp = HDMI21_P1_hd21_DSCD_DBG_DP_reg,

        // LINK TRAINING
        .lt_r = HDMI21_P1_HD21_LT_R_reg,
        .lt_g = HDMI21_P1_HD21_LT_G_reg,
        .lt_b = HDMI21_P1_HD21_LT_B_reg,
        .lt_c = HDMI21_P1_HD21_LT_C_reg,
        .lt_all = HDMI21_P1_HD21_LT_ALL_reg,

        // Debug Selection
        .udc = HDMI21_P1_HD21_UDC_reg,

        //Deep Color
        .tmds_dpc0 = HDMI21_P1_HD21_TMDS_DPC0_reg,
        .tmds_dpc1 = HDMI21_P1_HD21_TMDS_DPC1_reg,
        .tmds_dpc_set0 = HDMI21_P1_HD21_TMDS_DPC_SET0_reg,

        // Packet Status
        .hdmi_sr = HDMI21_P1_HD21_HDMI_SR_reg,
        .hdmi_gpvs = HDMI21_P1_HD21_HDMI_GPVS_reg,
        .hdmi_gpvs1 = HDMI21_P1_HD21_HDMI_GPVS1_reg,
        .hdmi_psap = HDMI21_P1_HD21_HDMI_PSAP_reg,
        .hdmi_psdp = HDMI21_P1_HD21_HDMI_PSDP_reg,

        // Mode Control
        .hdmi_scr = HDMI21_P1_HD21_HDMI_SCR_reg,

        // BCH Control
        .hdmi_bchcr = HDMI21_P1_HD21_HDMI_BCHCR_reg,

        // AV Mute Control
        .hdmi_avmcr = HDMI21_P1_HD21_HDMI_AVMCR_reg,
        .hdmi_wdcr = HDMI21_P1_HD21_HDMI_WDCR_reg,

        // Packet Variation
        .hdmi_pamicr = HDMI21_P1_HD21_HDMI_PAMICR_reg,
        .hdmi_ptrsv1 = HDMI21_P1_HD21_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2 = HDMI21_P1_HD21_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3 = HDMI21_P1_HD21_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0 = HDMI21_P1_HD21_HDMI_PVGCR0_reg,
        .hdmi_pvsr0 = HDMI21_P1_HD21_HDMI_PVSR0_reg,

        // Video Flow control
        .hdmi_vcr = HDMI21_P1_HD21_HDMI_VCR_reg,

        // Audio Clock Regeneration Packet
        .hdmi_acrcr = HDMI21_P1_HD21_HDMI_ACRCR_reg,
        .hdmi_acrsr0 = HDMI21_P1_HD21_HDMI_ACRSR0_reg,
        .hdmi_acrsr1 = HDMI21_P1_HD21_HDMI_ACRSR1_reg,

        // IRQ
        .hdmi_intcr = HDMI21_P1_HD21_HDMI_INTCR_reg,

        //Packet/Guard Band Error
        .hdmi_bcsr = HDMI21_P1_HD21_HDMI_BCSR_reg,

        // Packet Content
        .hdmi_asr0 = HDMI21_P1_HD21_HDMI_ASR0_reg,
        .hdmi_asr1 = HDMI21_P1_HD21_HDMI_ASR1_reg,
        .hdmi_video_format = HDMI21_P1_HD21_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format = HDMI21_P1_HD21_HDMI_3D_FORMAT_reg,
        .hdmi_fvs = HDMI21_P1_HD21_HDMI_FVS_reg,
        .hdmi_drm = HDMI21_P1_HD21_HDMI_DRM_reg,
        .hdmi_pcmc = HDMI21_P1_HD21_HDMI_PCMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI21_P1_HD21_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet = HDMI21_P1_HD21_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr = HDMI21_P1_HD21_HDMI_AUDCR_reg,
        .hdmi_audsr = HDMI21_P1_HD21_HDMI_AUDSR_reg,
        .audio_cts_up_bound = HDMI21_P1_HD21_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound = HDMI21_P1_HD21_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound = HDMI21_P1_HD21_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound = HDMI21_P1_HD21_AUDIO_N_LOW_BOUND_reg,

        // CLKDET part
        .clkdetcr = HDMI21_P1_HD21_CLKDETCR_reg,
        .clkdetsr = HDMI21_P1_HD21_CLKDETSR_reg,
        .clk_setting_00 = HDMI21_P1_HD21_CLK_SETTING_00_reg,
        .clk_setting_01 = HDMI21_P1_HD21_CLK_SETTING_01_reg,
        .clk_setting_02 = HDMI21_P1_HD21_CLK_SETTING_02_reg,
        .clkdet2cr = HDMI21_P1_HD21_CLKDET2CR_reg,
        .clkdet2sr = HDMI21_P1_HD21_CLKDET2SR_reg,
        .clk2_setting_00 = HDMI21_P1_HD21_CLK2_SETTING_00_reg,
        .clk2_setting_01 = HDMI21_P1_HD21_CLK2_SETTING_01_reg,
        .clk2_setting_02 = HDMI21_P1_HD21_CLK2_SETTING_02_reg,

        // CLK High Width Measure
        .clk_hwm_setting_00 = HDMI21_P1_HD21_CLK_HWM_SETTING_00_reg,
        .clk_hwm_setting_01 = HDMI21_P1_HD21_CLK_HWM_SETTING_01_reg,
        .clk_hwm_setting_02 = HDMI21_P1_HD21_CLK_HWM_SETTING_02_reg,
        .clk_hwn_one_st_00 = HDMI21_P1_HD21_CLK_HWN_ONE_ST_00_reg,
        .clk_hwn_cont_st_01 = HDMI21_P1_HD21_CLK_HWN_CONT_ST_01_reg,

        // HDMI TMP part
        .fapa_ct_tmp = HDMI21_P1_HD21_fapa_ct_tmp_reg,
        .ps_ct = HDMI21_P1_HD21_ps_ct_reg,
        .ps_st = HDMI21_P1_HD21_ps_st_reg,
        .irq_all_status = HDMI21_P1_hd21_irq_all_status_reg,

        // XTAL CNT
        .xtal_1s = HDMI21_P1_hd21_xtal_1s_reg,  // Only avaliable on P0
        .xtal_280ms = HDMI21_P1_hd21_xtal_280ms_reg,  // Only avaliable on P0
        .xtal_10ms = HDMI21_P1_hd21_xtal_10ms_reg,  // Only avaliable on P0

        // Power Saving Measure
        .ps_measure_ctrl = HDMI21_P1_hd21_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk0 = HDMI21_P1_hd21_ps_measure_xtal_clk0_reg,
        .ps_measure_xtal_clk1 = HDMI21_P1_hd21_ps_measure_xtal_clk1_reg,
        .ps_measure_xtal_clk2 = HDMI21_P1_hd21_ps_measure_xtal_clk2_reg,

        // Packet Absence
        .hdmi_pa_irq_en = HDMI21_P1_HD21_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta = HDMI21_P1_HD21_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt = HDMI21_P1_HD21_HDMI_PA_FRAME_CNT_reg
    },
    // HDMI2.1 mac 2
    {
        // HDMI_2p1 exist or not
        .valid = 1,

        // HDMI_2p1_control_register
        .cr = HDMI21_P2_HD21_CR_reg,

        // Video Part
        .phy_fifo_cr0 = HDMI21_P2_HD21_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1 = HDMI21_P2_HD21_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0 = HDMI21_P2_HD21_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1 = HDMI21_P2_HD21_PHY_FIFO_SR1_reg,

        // Data Align
        .channel_align_c = HDMI21_P2_hd21_channel_align_c_reg,
        .channel_align_r = HDMI21_P2_hd21_channel_align_r_reg,
        .channel_align_g = HDMI21_P2_hd21_channel_align_g_reg,
        .channel_align_b = HDMI21_P2_hd21_channel_align_b_reg,

        // Channel Sync
        .ch_sync_ctrl = HDMI21_P2_hd21_ch_sync_ctrl_reg,
        .ch_sync_status = HDMI21_P2_hd21_ch_sync_status_reg,
        .ch_sync_r = HDMI21_P2_hd21_ch_sync_r_reg,
        .ch_sync_g = HDMI21_P2_hd21_ch_sync_g_reg,
        .ch_sync_b = HDMI21_P2_hd21_ch_sync_b_reg,
        .ch_sync_c = HDMI21_P2_hd21_ch_sync_c_reg,

        // Error Detect Function
        .errdet_r_ctrl = HDMI21_P2_hd21_errdet_r_ctrl_reg,
        .errdet_g_ctrl = HDMI21_P2_hd21_errdet_g_ctrl_reg,
        .errdet_b_ctrl = HDMI21_P2_hd21_errdet_b_ctrl_reg,
        .errdet_c_ctrl = HDMI21_P2_hd21_errdet_c_ctrl_reg,
        .errdet_r_disparity = HDMI21_P2_hd21_errdet_r_disparity_reg,
        .errdet_g_disparity = HDMI21_P2_hd21_errdet_g_disparity_reg,
        .errdet_b_disparity = HDMI21_P2_hd21_errdet_b_disparity_reg,
        .errdet_c_disparity = HDMI21_P2_hd21_errdet_c_disparity_reg,
        .errdet_r_char = HDMI21_P2_hd21_errdet_r_char_reg,
        .errdet_g_char = HDMI21_P2_hd21_errdet_g_char_reg,
        .errdet_b_char = HDMI21_P2_hd21_errdet_b_char_reg,
        .errdet_c_char = HDMI21_P2_hd21_errdet_c_char_reg,
        .disp_ctrl = HDMI21_P2_hd21_disp_ctrl_reg,

        // FRL Status
        .mm_cr = HDMI21_P2_HD21_MM_CR_reg,
        .mm_sr = HDMI21_P2_HD21_MM_SR_reg,

        // Extended Metadata Packet
        .hdmi_hdrap = HDMI21_P2_hd21_HDMI_HDRAP_reg,
        .hdmi_hdrdp = HDMI21_P2_hd21_HDMI_HDRDP_reg,
        .fapa_ct2 = 0, // Only avaliable on P0
        .hdr_em_st = HDMI21_P2_HD21_HDR_EM_ST_reg, 
        .hdr_em_ct = HDMI21_P2_HD21_HDR_EM_CT_reg,
        .hdmi_emap = HDMI21_P2_HD21_HDMI_EMAP_reg,
        .hdmi_emdp = HDMI21_P2_hd21_HDMI_EMDP_reg,
        .hdmi_emap_pp = HDMI21_P2_HD21_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp = HDMI21_P2_hd21_HDMI_EMDP_PP_reg,
        .fapa_ct = HDMI21_P2_HD21_fapa_ct_reg,
        .hdr_em_ct2 = HDMI21_P2_HD21_HDR_EM_CT2_reg, 

        // Error DET
        .cercr = HDMI21_P2_hd21_cercr_reg,
        .cersr0 = HDMI21_P2_HD21_CERSR0_reg,
        .cersr1 = HDMI21_P2_HD21_CERSR1_reg,
        .cersr2 = HDMI21_P2_HD21_CERSR2_reg,

        .rmzp = HDMI21_P2_HD21_rmzp_reg,

        // Extended Metadata Packet 2
        .vsem_st = HDMI21_P2_HD21_VSEM_ST_reg,
        .vrrem_st = HDMI21_P2_HD21_VRREM_ST_reg,
        .cvtem_st = HDMI21_P2_HD21_CVTEM_ST_reg,
        .hdmi_vsem_emc = HDMI21_P2_HD21_HDMI_VSEM_EMC_reg,
        .hdmi_vrr_emc = HDMI21_P2_HD21_HDMI_VRR_EMC_reg,
        .hdmi_cvtem_emc = HDMI21_P2_HD21_HDMI_CVTEM_EMC_reg,
        .hdmi_vsem_emc2 = HDMI21_P2_HD21_HDMI_VSEM_EMC2_reg,

        // CRC Control Register
        .crc_ctrl                     = HDMI21_P2_HD21_CRC_Ctrl_reg,
        .crc_result                   = HDMI21_P2_HD21_CRC_Result_reg,
        .des_crc                      = HDMI21_P2_HD21_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI21_P2_HD21_CRC_ERR_CNT0_reg,

        // Video Timing Generator
        .timing_gen_cr = HDMI21_P2_HD21_TIMING_GEN_CR_reg,
        .timing_gen_sr = HDMI21_P2_HD21_TIMING_GEN_SR_reg,

        // Channel Status
        .ch_st = HDMI21_P2_HD21_CH_ST_reg,

        // PRBS
        .prbs_r_ctrl = HDMI21_P2_HD21_PRBS_R_CTRL_reg,
        .prbs_g_ctrl = HDMI21_P2_HD21_PRBS_G_CTRL_reg,
        .prbs_b_ctrl = HDMI21_P2_HD21_PRBS_B_CTRL_reg,
        .prbs_c_ctrl = HDMI21_P2_HD21_PRBS_C_CTRL_reg,
        .prbs_r_status = HDMI21_P2_HD21_PRBS_R_STATUS_reg,
        .prbs_g_status = HDMI21_P2_HD21_PRBS_G_STATUS_reg,
        .prbs_b_status = HDMI21_P2_HD21_PRBS_B_STATUS_reg,
        .prbs_c_status = HDMI21_P2_HD21_PRBS_C_STATUS_reg,

        // DSCD_DBG
        .dscd_dbg = HDMI21_P2_hd21_DSCD_DBG_reg,
        .dscd_dbg_ap = HDMI21_P2_HD21_DSCD_DBG_AP_reg,
        .dscd_dbg_dp = HDMI21_P2_hd21_DSCD_DBG_DP_reg,

        // LINK TRAINING
        .lt_r = HDMI21_P2_HD21_LT_R_reg,
        .lt_g = HDMI21_P2_HD21_LT_G_reg,
        .lt_b = HDMI21_P2_HD21_LT_B_reg,
        .lt_c = HDMI21_P2_HD21_LT_C_reg,
        .lt_all = HDMI21_P2_HD21_LT_ALL_reg,

        // Debug Selection
        .udc = HDMI21_P2_HD21_UDC_reg,

        //Deep Color
        .tmds_dpc0 = HDMI21_P2_HD21_TMDS_DPC0_reg,
        .tmds_dpc1 = HDMI21_P2_HD21_TMDS_DPC1_reg,
        .tmds_dpc_set0 = HDMI21_P2_HD21_TMDS_DPC_SET0_reg,

        // Packet Status
        .hdmi_sr = HDMI21_P2_HD21_HDMI_SR_reg,
        .hdmi_gpvs = HDMI21_P2_HD21_HDMI_GPVS_reg,
        .hdmi_gpvs1 = HDMI21_P2_HD21_HDMI_GPVS1_reg,
        .hdmi_psap = HDMI21_P2_HD21_HDMI_PSAP_reg,
        .hdmi_psdp = HDMI21_P2_HD21_HDMI_PSDP_reg,

        // Mode Control
        .hdmi_scr = HDMI21_P2_HD21_HDMI_SCR_reg,

        // BCH Control
        .hdmi_bchcr = HDMI21_P2_HD21_HDMI_BCHCR_reg,

        // AV Mute Control
        .hdmi_avmcr = HDMI21_P2_HD21_HDMI_AVMCR_reg,
        .hdmi_wdcr = HDMI21_P2_HD21_HDMI_WDCR_reg,

        // Packet Variation
        .hdmi_pamicr = HDMI21_P2_HD21_HDMI_PAMICR_reg,
        .hdmi_ptrsv1 = HDMI21_P2_HD21_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2 = HDMI21_P2_HD21_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3 = HDMI21_P2_HD21_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0 = HDMI21_P2_HD21_HDMI_PVGCR0_reg,
        .hdmi_pvsr0 = HDMI21_P2_HD21_HDMI_PVSR0_reg,

        // Video Flow control
        .hdmi_vcr = HDMI21_P2_HD21_HDMI_VCR_reg,

        // Audio Clock Regeneration Packet
        .hdmi_acrcr = HDMI21_P2_HD21_HDMI_ACRCR_reg,
        .hdmi_acrsr0 = HDMI21_P2_HD21_HDMI_ACRSR0_reg,
        .hdmi_acrsr1 = HDMI21_P2_HD21_HDMI_ACRSR1_reg,

        // IRQ
        .hdmi_intcr = HDMI21_P2_HD21_HDMI_INTCR_reg,

        //Packet/Guard Band Error
        .hdmi_bcsr = HDMI21_P2_HD21_HDMI_BCSR_reg,

        // Packet Content
        .hdmi_asr0 = HDMI21_P2_HD21_HDMI_ASR0_reg,
        .hdmi_asr1 = HDMI21_P2_HD21_HDMI_ASR1_reg,
        .hdmi_video_format = HDMI21_P2_HD21_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format = HDMI21_P2_HD21_HDMI_3D_FORMAT_reg,
        .hdmi_fvs = HDMI21_P2_HD21_HDMI_FVS_reg,
        .hdmi_drm = HDMI21_P2_HD21_HDMI_DRM_reg,
        .hdmi_pcmc = HDMI21_P2_HD21_HDMI_PCMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI21_P2_HD21_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet = HDMI21_P2_HD21_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr = HDMI21_P2_HD21_HDMI_AUDCR_reg,
        .hdmi_audsr = HDMI21_P2_HD21_HDMI_AUDSR_reg,
        .audio_cts_up_bound = HDMI21_P2_HD21_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound = HDMI21_P2_HD21_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound = HDMI21_P2_HD21_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound = HDMI21_P2_HD21_AUDIO_N_LOW_BOUND_reg,

        // CLKDET part
        .clkdetcr = HDMI21_P2_HD21_CLKDETCR_reg,
        .clkdetsr = HDMI21_P2_HD21_CLKDETSR_reg,
        .clk_setting_00 = HDMI21_P2_HD21_CLK_SETTING_00_reg,
        .clk_setting_01 = HDMI21_P2_HD21_CLK_SETTING_01_reg,
        .clk_setting_02 = HDMI21_P2_HD21_CLK_SETTING_02_reg,
        .clkdet2cr = HDMI21_P2_HD21_CLKDET2CR_reg,
        .clkdet2sr = HDMI21_P2_HD21_CLKDET2SR_reg,
        .clk2_setting_00 = HDMI21_P2_HD21_CLK2_SETTING_00_reg,
        .clk2_setting_01 = HDMI21_P2_HD21_CLK2_SETTING_01_reg,
        .clk2_setting_02 = HDMI21_P2_HD21_CLK2_SETTING_02_reg,

        // CLK High Width Measure
        .clk_hwm_setting_00 = HDMI21_P2_HD21_CLK_HWM_SETTING_00_reg,
        .clk_hwm_setting_01 = HDMI21_P2_HD21_CLK_HWM_SETTING_01_reg,
        .clk_hwm_setting_02 = HDMI21_P2_HD21_CLK_HWM_SETTING_02_reg,
        .clk_hwn_one_st_00 = HDMI21_P2_HD21_CLK_HWN_ONE_ST_00_reg,
        .clk_hwn_cont_st_01 = HDMI21_P2_HD21_CLK_HWN_CONT_ST_01_reg,

        // HDMI TMP part
        .fapa_ct_tmp = HDMI21_P2_HD21_fapa_ct_tmp_reg,
        .ps_ct = HDMI21_P2_HD21_ps_ct_reg,
        .ps_st = HDMI21_P2_HD21_ps_st_reg,
        .irq_all_status = HDMI21_P2_hd21_irq_all_status_reg,

        // XTAL CNT
        .xtal_1s = HDMI21_P2_hd21_xtal_1s_reg,  // Only avaliable on P0
        .xtal_280ms = HDMI21_P2_hd21_xtal_280ms_reg,  // Only avaliable on P0
        .xtal_10ms = HDMI21_P2_hd21_xtal_10ms_reg,  // Only avaliable on P0

        // Power Saving Measure
        .ps_measure_ctrl = HDMI21_P2_hd21_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk0 = HDMI21_P2_hd21_ps_measure_xtal_clk0_reg,
        .ps_measure_xtal_clk1 = HDMI21_P2_hd21_ps_measure_xtal_clk1_reg,
        .ps_measure_xtal_clk2 = HDMI21_P2_hd21_ps_measure_xtal_clk2_reg,

        // Packet Absence
        .hdmi_pa_irq_en = HDMI21_P2_HD21_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta = HDMI21_P2_HD21_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt = HDMI21_P2_HD21_HDMI_PA_FRAME_CNT_reg
    },
    // HDMI2.1 mac 3
    {
        // HDMI_2p1 exist or not
        .valid = 1,

        // HDMI_2p1_control_register
        .cr = HDMI21_P3_HD21_CR_reg,

        // Video Part
        .phy_fifo_cr0 = HDMI21_P3_HD21_PHY_FIFO_CR0_reg,
        .phy_fifo_cr1 = HDMI21_P3_HD21_PHY_FIFO_CR1_reg,
        .phy_fifo_sr0 = HDMI21_P3_HD21_PHY_FIFO_SR0_reg,
        .phy_fifo_sr1 = HDMI21_P3_HD21_PHY_FIFO_SR1_reg,

        // Data Align
        .channel_align_c = HDMI21_P3_hd21_channel_align_c_reg,
        .channel_align_r = HDMI21_P3_hd21_channel_align_r_reg,
        .channel_align_g = HDMI21_P3_hd21_channel_align_g_reg,
        .channel_align_b = HDMI21_P3_hd21_channel_align_b_reg,

        // Channel Sync
        .ch_sync_ctrl = HDMI21_P3_hd21_ch_sync_ctrl_reg,
        .ch_sync_status = HDMI21_P3_hd21_ch_sync_status_reg,
        .ch_sync_r = HDMI21_P3_hd21_ch_sync_r_reg,
        .ch_sync_g = HDMI21_P3_hd21_ch_sync_g_reg,
        .ch_sync_b = HDMI21_P3_hd21_ch_sync_b_reg,
        .ch_sync_c = HDMI21_P3_hd21_ch_sync_c_reg,

        // Error Detect Function
        .errdet_r_ctrl = HDMI21_P3_hd21_errdet_r_ctrl_reg,
        .errdet_g_ctrl = HDMI21_P3_hd21_errdet_g_ctrl_reg,
        .errdet_b_ctrl = HDMI21_P3_hd21_errdet_b_ctrl_reg,
        .errdet_c_ctrl = HDMI21_P3_hd21_errdet_c_ctrl_reg,
        .errdet_r_disparity = HDMI21_P3_hd21_errdet_r_disparity_reg,
        .errdet_g_disparity = HDMI21_P3_hd21_errdet_g_disparity_reg,
        .errdet_b_disparity = HDMI21_P3_hd21_errdet_b_disparity_reg,
        .errdet_c_disparity = HDMI21_P3_hd21_errdet_c_disparity_reg,
        .errdet_r_char = HDMI21_P3_hd21_errdet_r_char_reg,
        .errdet_g_char = HDMI21_P3_hd21_errdet_g_char_reg,
        .errdet_b_char = HDMI21_P3_hd21_errdet_b_char_reg,
        .errdet_c_char = HDMI21_P3_hd21_errdet_c_char_reg,
        .disp_ctrl = HDMI21_P3_hd21_disp_ctrl_reg,

        // FRL Status
        .mm_cr = HDMI21_P3_HD21_MM_CR_reg,
        .mm_sr = HDMI21_P3_HD21_MM_SR_reg,

        // Extended Metadata Packet
        .hdmi_hdrap = HDMI21_P3_hd21_HDMI_HDRAP_reg,
        .hdmi_hdrdp = HDMI21_P3_hd21_HDMI_HDRDP_reg,
        .fapa_ct2 = 0, // Only avaliable on P0
        .hdr_em_st = HDMI21_P3_HD21_HDR_EM_ST_reg, 
        .hdr_em_ct = HDMI21_P3_HD21_HDR_EM_CT_reg,
        .hdmi_emap = HDMI21_P3_HD21_HDMI_EMAP_reg,
        .hdmi_emdp = HDMI21_P3_hd21_HDMI_EMDP_reg,
        .hdmi_emap_pp = HDMI21_P3_HD21_HDMI_EMAP_PP_reg,
        .hdmi_emdp_pp = HDMI21_P3_hd21_HDMI_EMDP_PP_reg,
        .fapa_ct = HDMI21_P3_HD21_fapa_ct_reg,
        .hdr_em_ct2 = HDMI21_P3_HD21_HDR_EM_CT2_reg, 

        // Error DET
        .cercr = HDMI21_P3_hd21_cercr_reg,
        .cersr0 = HDMI21_P3_HD21_CERSR0_reg,
        .cersr1 = HDMI21_P3_HD21_CERSR1_reg,
        .cersr2 = HDMI21_P3_HD21_CERSR2_reg,

        .rmzp = HDMI21_P3_HD21_rmzp_reg,

        // Extended Metadata Packet 2
        .vsem_st = HDMI21_P3_HD21_VSEM_ST_reg,
        .vrrem_st = HDMI21_P3_HD21_VRREM_ST_reg,
        .cvtem_st = HDMI21_P3_HD21_CVTEM_ST_reg,
        .hdmi_vsem_emc = HDMI21_P3_HD21_HDMI_VSEM_EMC_reg,
        .hdmi_vrr_emc = HDMI21_P3_HD21_HDMI_VRR_EMC_reg,
        .hdmi_cvtem_emc = HDMI21_P3_HD21_HDMI_CVTEM_EMC_reg,
        .hdmi_vsem_emc2 = HDMI21_P3_HD21_HDMI_VSEM_EMC2_reg,

        // CRC Control Register
        .crc_ctrl                     = HDMI21_P3_HD21_CRC_Ctrl_reg,
        .crc_result                   = HDMI21_P3_HD21_CRC_Result_reg,
        .des_crc                      = HDMI21_P3_HD21_DES_CRC_reg,
        .crc_err_cnt0                 = HDMI21_P3_HD21_CRC_ERR_CNT0_reg,

        // Video Timing Generator
        .timing_gen_cr = HDMI21_P3_HD21_TIMING_GEN_CR_reg,
        .timing_gen_sr = HDMI21_P3_HD21_TIMING_GEN_SR_reg,

        // Channel Status
        .ch_st = HDMI21_P3_HD21_CH_ST_reg,

        // PRBS
        .prbs_r_ctrl = HDMI21_P3_HD21_PRBS_R_CTRL_reg,
        .prbs_g_ctrl = HDMI21_P3_HD21_PRBS_G_CTRL_reg,
        .prbs_b_ctrl = HDMI21_P3_HD21_PRBS_B_CTRL_reg,
        .prbs_c_ctrl = HDMI21_P3_HD21_PRBS_C_CTRL_reg,
        .prbs_r_status = HDMI21_P3_HD21_PRBS_R_STATUS_reg,
        .prbs_g_status = HDMI21_P3_HD21_PRBS_G_STATUS_reg,
        .prbs_b_status = HDMI21_P3_HD21_PRBS_B_STATUS_reg,
        .prbs_c_status = HDMI21_P3_HD21_PRBS_C_STATUS_reg,

        // DSCD_DBG
        .dscd_dbg = HDMI21_P3_hd21_DSCD_DBG_reg,
        .dscd_dbg_ap = HDMI21_P3_HD21_DSCD_DBG_AP_reg,
        .dscd_dbg_dp = HDMI21_P3_hd21_DSCD_DBG_DP_reg,

        // LINK TRAINING
        .lt_r = HDMI21_P3_HD21_LT_R_reg,
        .lt_g = HDMI21_P3_HD21_LT_G_reg,
        .lt_b = HDMI21_P3_HD21_LT_B_reg,
        .lt_c = HDMI21_P3_HD21_LT_C_reg,
        .lt_all = HDMI21_P3_HD21_LT_ALL_reg,

        // Debug Selection
        .udc = HDMI21_P3_HD21_UDC_reg,

        //Deep Color
        .tmds_dpc0 = HDMI21_P3_HD21_TMDS_DPC0_reg,
        .tmds_dpc1 = HDMI21_P3_HD21_TMDS_DPC1_reg,
        .tmds_dpc_set0 = HDMI21_P3_HD21_TMDS_DPC_SET0_reg,

        // Packet Status
        .hdmi_sr = HDMI21_P3_HD21_HDMI_SR_reg,
        .hdmi_gpvs = HDMI21_P3_HD21_HDMI_GPVS_reg,
        .hdmi_gpvs1 = HDMI21_P3_HD21_HDMI_GPVS1_reg,
        .hdmi_psap = HDMI21_P3_HD21_HDMI_PSAP_reg,
        .hdmi_psdp = HDMI21_P3_HD21_HDMI_PSDP_reg,

        // Mode Control
        .hdmi_scr = HDMI21_P3_HD21_HDMI_SCR_reg,

        // BCH Control
        .hdmi_bchcr = HDMI21_P3_HD21_HDMI_BCHCR_reg,

        // AV Mute Control
        .hdmi_avmcr = HDMI21_P3_HD21_HDMI_AVMCR_reg,
        .hdmi_wdcr = HDMI21_P3_HD21_HDMI_WDCR_reg,

        // Packet Variation
        .hdmi_pamicr = HDMI21_P3_HD21_HDMI_PAMICR_reg,
        .hdmi_ptrsv1 = HDMI21_P3_HD21_HDMI_PTRSV1_reg,
        .hdmi_ptrsv2 = HDMI21_P3_HD21_HDMI_PTRSV2_reg,
        .hdmi_ptrsv3 = HDMI21_P3_HD21_HDMI_PTRSV3_reg,
        .hdmi_pvgcr0 = HDMI21_P3_HD21_HDMI_PVGCR0_reg,
        .hdmi_pvsr0 = HDMI21_P3_HD21_HDMI_PVSR0_reg,

        // Video Flow control
        .hdmi_vcr = HDMI21_P3_HD21_HDMI_VCR_reg,

        // Audio Clock Regeneration Packet
        .hdmi_acrcr = HDMI21_P3_HD21_HDMI_ACRCR_reg,
        .hdmi_acrsr0 = HDMI21_P3_HD21_HDMI_ACRSR0_reg,
        .hdmi_acrsr1 = HDMI21_P3_HD21_HDMI_ACRSR1_reg,

        // IRQ
        .hdmi_intcr = HDMI21_P3_HD21_HDMI_INTCR_reg,

        //Packet/Guard Band Error
        .hdmi_bcsr = HDMI21_P3_HD21_HDMI_BCSR_reg,

        // Packet Content
        .hdmi_asr0 = HDMI21_P3_HD21_HDMI_ASR0_reg,
        .hdmi_asr1 = HDMI21_P3_HD21_HDMI_ASR1_reg,
        .hdmi_video_format = HDMI21_P3_HD21_HDMI_VIDEO_FORMAT_reg,
        .hdmi_3d_format = HDMI21_P3_HD21_HDMI_3D_FORMAT_reg,
        .hdmi_fvs = HDMI21_P3_HD21_HDMI_FVS_reg,
        .hdmi_drm = HDMI21_P3_HD21_HDMI_DRM_reg,
        .hdmi_pcmc = HDMI21_P3_HD21_HDMI_PCMC_reg,

        // Audio Control
        .audio_sample_rate_change_irq = HDMI21_P3_HD21_Audio_Sample_Rate_Change_IRQ_reg,
        .high_bit_rate_audio_packet = HDMI21_P3_HD21_High_Bit_Rate_Audio_Packet_reg,
        .hdmi_audcr = HDMI21_P3_HD21_HDMI_AUDCR_reg,
        .hdmi_audsr = HDMI21_P3_HD21_HDMI_AUDSR_reg,
        .audio_cts_up_bound = HDMI21_P3_HD21_AUDIO_CTS_UP_BOUND_reg,
        .audio_cts_low_bound = HDMI21_P3_HD21_AUDIO_CTS_LOW_BOUND_reg,
        .audio_n_up_bound = HDMI21_P3_HD21_AUDIO_N_UP_BOUND_reg,
        .audio_n_low_bound = HDMI21_P3_HD21_AUDIO_N_LOW_BOUND_reg,

        // CLKDET part
        .clkdetcr = HDMI21_P3_HD21_CLKDETCR_reg,
        .clkdetsr = HDMI21_P3_HD21_CLKDETSR_reg,
        .clk_setting_00 = HDMI21_P3_HD21_CLK_SETTING_00_reg,
        .clk_setting_01 = HDMI21_P3_HD21_CLK_SETTING_01_reg,
        .clk_setting_02 = HDMI21_P3_HD21_CLK_SETTING_02_reg,
        .clkdet2cr = HDMI21_P3_HD21_CLKDET2CR_reg,
        .clkdet2sr = HDMI21_P3_HD21_CLKDET2SR_reg,
        .clk2_setting_00 = HDMI21_P3_HD21_CLK2_SETTING_00_reg,
        .clk2_setting_01 = HDMI21_P3_HD21_CLK2_SETTING_01_reg,
        .clk2_setting_02 = HDMI21_P3_HD21_CLK2_SETTING_02_reg,

        // CLK High Width Measure
        .clk_hwm_setting_00 = HDMI21_P3_HD21_CLK_HWM_SETTING_00_reg,
        .clk_hwm_setting_01 = HDMI21_P3_HD21_CLK_HWM_SETTING_01_reg,
        .clk_hwm_setting_02 = HDMI21_P3_HD21_CLK_HWM_SETTING_02_reg,
        .clk_hwn_one_st_00 = HDMI21_P3_HD21_CLK_HWN_ONE_ST_00_reg,
        .clk_hwn_cont_st_01 = HDMI21_P3_HD21_CLK_HWN_CONT_ST_01_reg,

        // HDMI TMP part
        .fapa_ct_tmp = HDMI21_P3_HD21_fapa_ct_tmp_reg,
        .ps_ct = HDMI21_P3_HD21_ps_ct_reg,
        .ps_st = HDMI21_P3_HD21_ps_st_reg,
        .irq_all_status = HDMI21_P3_hd21_irq_all_status_reg,

        // XTAL CNT
        .xtal_1s = HDMI21_P3_hd21_xtal_1s_reg,  // Only avaliable on P0
        .xtal_280ms = HDMI21_P3_hd21_xtal_280ms_reg,  // Only avaliable on P0
        .xtal_10ms = HDMI21_P3_hd21_xtal_10ms_reg,  // Only avaliable on P0

        // Power Saving Measure
        .ps_measure_ctrl = HDMI21_P3_hd21_ps_measure_ctrl_reg,
        .ps_measure_xtal_clk0 = HDMI21_P3_hd21_ps_measure_xtal_clk0_reg,
        .ps_measure_xtal_clk1 = HDMI21_P3_hd21_ps_measure_xtal_clk1_reg,
        .ps_measure_xtal_clk2 = HDMI21_P3_hd21_ps_measure_xtal_clk2_reg,

        // Packet Absence
        .hdmi_pa_irq_en = HDMI21_P3_HD21_HDMI_PA_IRQ_EN_reg,
        .hdmi_pa_irq_sta = HDMI21_P3_HD21_HDMI_PA_IRQ_STA_reg,
        .hdmi_pa_frame_cnt = HDMI21_P3_HD21_HDMI_PA_FRAME_CNT_reg
    }
};

/*--------------------------------------------------
 * HDMI2.1 MEAS register setting
 *--------------------------------------------------*/
const struct hdmi21_meas_reg_st hd21_meas[HDMI21_MAC_NUM] =
{
    // HDMI2.1 MEAS0
    {
        .frl_measure_0 = HDMI21_P0_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI21_P0_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI21_P0_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI21_P0_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI21_P0_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI21_P0_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI21_P0_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI21_P0_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI21_P0_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI21_P0_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI21_P0_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI21_P0_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI21_P0_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI21_P0_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI21_P0_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI21_P0_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.1 MEAS1
    {
        .frl_measure_0 = HDMI21_P1_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI21_P1_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI21_P1_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI21_P1_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI21_P1_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI21_P1_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI21_P1_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI21_P1_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI21_P1_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI21_P1_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI21_P1_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI21_P1_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI21_P1_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI21_P1_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI21_P1_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI21_P1_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.1 MEAS2
    {
        .frl_measure_0 = HDMI21_P2_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI21_P2_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI21_P2_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI21_P2_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI21_P2_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI21_P2_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI21_P2_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI21_P2_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI21_P2_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI21_P2_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI21_P2_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI21_P2_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI21_P2_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI21_P2_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI21_P2_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI21_P2_MEAS_FRL_MEASURE_18_reg
    },
    // HDMI2.1 MEAS3
    {
        .frl_measure_0 = HDMI21_P3_MEAS_FRL_MEASURE_0_reg,
        .frl_measure_1 = HDMI21_P3_MEAS_FRL_MEASURE_1_reg,
        .frl_measure_3 = HDMI21_P3_MEAS_FRL_MEASURE_3_reg,
        .frl_measure_4 = HDMI21_P3_MEAS_FRL_MEASURE_4_reg,
        .frl_measure_5 = HDMI21_P3_MEAS_FRL_MEASURE_5_reg,
        .frl_measure_7 = HDMI21_P3_MEAS_FRL_MEASURE_7_reg,
        .frl_measure_9 = HDMI21_P3_MEAS_FRL_MEASURE_9_reg,
        .frl_measure_a = HDMI21_P3_MEAS_FRL_MEASURE_A_reg,
        .frl_measure_b = HDMI21_P3_MEAS_FRL_MEASURE_B_reg,
        .frl_measure_d = HDMI21_P3_MEAS_FRL_MEASURE_D_reg,
        .frl_measure_f = HDMI21_P3_MEAS_FRL_MEASURE_F_reg,
        .frl_measure_11 = HDMI21_P3_MEAS_FRL_MEASURE_11_reg,
        .frl_measure_13 = HDMI21_P3_MEAS_FRL_MEASURE_13_reg,
        .frl_measure_14 = HDMI21_P3_MEAS_FRL_MEASURE_14_reg,
        .frl_measure_16 = HDMI21_P3_MEAS_FRL_MEASURE_16_reg,
        .frl_measure_18 = HDMI21_P3_MEAS_FRL_MEASURE_18_reg
    }
};


/*--------------------------------------------------
 * HDMI2.1 FEC register setting
 *--------------------------------------------------*/
const struct hdmi21_fec_reg_st hd21_fec[HDMI21_MAC_NUM] =
{
    // HDMI2.1 FEC0
    {
        .hdmi21_rs_0 = HDMI21_P0_FEC_FEC_HDMI21_RS_0_reg,
        .hdmi21_rs_1 = HDMI21_P0_FEC_FEC_HDMI21_RS_1_reg,
        .hdmi21_rs_2 = HDMI21_P0_FEC_FEC_HDMI21_RS_2_reg,
        .hdmi21_rs_3 = HDMI21_P0_FEC_FEC_HDMI21_RS_3_reg,

        .hdmi21_fw_rs_0 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_0_reg,
        .hdmi21_fw_rs_1 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_1_reg,
        .hdmi21_fw_rs_2 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_2_reg,
        .hdmi21_fw_rs_3 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_3_reg,
        .hdmi21_fw_rs_4 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_4_reg,
        .hdmi21_fw_rs_5 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_5_reg,
        .hdmi21_fw_rs_6 = HDMI21_P0_FEC_FEC_HDMI21_FW_RS_6_reg,

        .hdmi21_lfsr_1 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_1_reg,
        .hdmi21_lfsr_2 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_2_reg,
        .hdmi21_lfsr_3 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_3_reg,
        .hdmi21_lfsr_4 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_4_reg,
        .hdmi21_lfsr_5 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_5_reg,
        .hdmi21_lfsr_6 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_6_reg,
        .hdmi21_lfsr_7 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_7_reg,
        .hdmi21_lfsr_8 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_8_reg,
        .hdmi21_lfsr_9 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_9_reg,
        .hdmi21_lfsr_10 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_10_reg,
        .hdmi21_lfsr_11 = HDMI21_P0_FEC_FEC_HDMI21_LFSR_11_reg,

        .hdmi21_di_0 = HDMI21_P0_FEC_FEC_HDMI21_DI_0_reg,

        .hdmi21_rs_sram_0 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_0_reg,
        .hdmi21_rs_sram_1 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_1_reg,
        .hdmi21_rs_sram_2 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_2_reg,
        .hdmi21_rs_sram_bist_0 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_0_reg,
        .hdmi21_rs_sram_bist_1 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_1_reg,
        .hdmi21_rs_sram_bist_2 = HDMI21_P0_FEC_FEC_HDMI21_RS_SRAM_BIST_2_reg,

        .hdmi21_rs_read_margin = HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_reg,
        .hdmi21_rs_read_margin_enable = HDMI21_P0_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_reg,

        .hdmi21_frl_0 = HDMI21_P0_FEC_FEC_HDMI21_FRL_0_reg,
        .hdmi21_frl_1 = HDMI21_P0_FEC_FEC_HDMI21_FRL_1_reg,
        .hdmi21_frl_D1 = HDMI21_P0_FEC_FEC_HDMI21_FRL_D1_reg,
        .hdmi21_frl_D2 = HDMI21_P0_FEC_FEC_HDMI21_FRL_D2_reg,

        .hdmi21_test_sel = HDMI21_P0_FEC_FEC_HDMI21_TEST_SEL_reg,

        .hdmi21_lt_9 = HDMI21_P0_FEC_FEC_HDMI21_LT_9_reg,

        .port_page73_hdmi_scr = HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg,
        .hdmi_sr = HDMI21_P0_FEC_FEC_HDMI_SR_reg,
        .port_page73_hdmi_bcsr = HDMI21_P0_FEC_FEC_PORT_PAGE73_HDMI_BCSR_reg
    },
    // HDMI2.1 FEC 1
    {
        .hdmi21_rs_0 = HDMI21_P1_FEC_FEC_HDMI21_RS_0_reg,
        .hdmi21_rs_1 = HDMI21_P1_FEC_FEC_HDMI21_RS_1_reg,
        .hdmi21_rs_2 = HDMI21_P1_FEC_FEC_HDMI21_RS_2_reg,
        .hdmi21_rs_3 = HDMI21_P1_FEC_FEC_HDMI21_RS_3_reg,

        .hdmi21_fw_rs_0 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_0_reg,
        .hdmi21_fw_rs_1 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_1_reg,
        .hdmi21_fw_rs_2 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_2_reg,
        .hdmi21_fw_rs_3 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_3_reg,
        .hdmi21_fw_rs_4 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_4_reg,
        .hdmi21_fw_rs_5 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_5_reg,
        .hdmi21_fw_rs_6 = HDMI21_P1_FEC_FEC_HDMI21_FW_RS_6_reg,

        .hdmi21_lfsr_1 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_1_reg,
        .hdmi21_lfsr_2 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_2_reg,
        .hdmi21_lfsr_3 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_3_reg,
        .hdmi21_lfsr_4 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_4_reg,
        .hdmi21_lfsr_5 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_5_reg,
        .hdmi21_lfsr_6 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_6_reg,
        .hdmi21_lfsr_7 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_7_reg,
        .hdmi21_lfsr_8 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_8_reg,
        .hdmi21_lfsr_9 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_9_reg,
        .hdmi21_lfsr_10 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_10_reg,
        .hdmi21_lfsr_11 = HDMI21_P1_FEC_FEC_HDMI21_LFSR_11_reg,

        .hdmi21_di_0 = HDMI21_P1_FEC_FEC_HDMI21_DI_0_reg,

        .hdmi21_rs_sram_0 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_0_reg,
        .hdmi21_rs_sram_1 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_1_reg,
        .hdmi21_rs_sram_2 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_2_reg,
        .hdmi21_rs_sram_bist_0 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_BIST_0_reg,
        .hdmi21_rs_sram_bist_1 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_BIST_1_reg,
        .hdmi21_rs_sram_bist_2 = HDMI21_P1_FEC_FEC_HDMI21_RS_SRAM_BIST_2_reg,

        .hdmi21_rs_read_margin = HDMI21_P1_FEC_FEC_HDMI21_RS_READ_MARGIN_reg,
        .hdmi21_rs_read_margin_enable = HDMI21_P1_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_reg,

        .hdmi21_frl_0 = HDMI21_P1_FEC_FEC_HDMI21_FRL_0_reg,
        .hdmi21_frl_1 = HDMI21_P1_FEC_FEC_HDMI21_FRL_1_reg,
        .hdmi21_frl_D1 = HDMI21_P1_FEC_FEC_HDMI21_FRL_D1_reg,
        .hdmi21_frl_D2 = HDMI21_P1_FEC_FEC_HDMI21_FRL_D2_reg,

        .hdmi21_test_sel = HDMI21_P1_FEC_FEC_HDMI21_TEST_SEL_reg,

        .hdmi21_lt_9 = HDMI21_P1_FEC_FEC_HDMI21_LT_9_reg,

        .port_page73_hdmi_scr = HDMI21_P1_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg,
        .hdmi_sr = HDMI21_P1_FEC_FEC_HDMI_SR_reg,
        .port_page73_hdmi_bcsr = HDMI21_P1_FEC_FEC_PORT_PAGE73_HDMI_BCSR_reg
    },
    // HDMI2.1 FEC 1
    {
        .hdmi21_rs_0 = HDMI21_P2_FEC_FEC_HDMI21_RS_0_reg,
        .hdmi21_rs_1 = HDMI21_P2_FEC_FEC_HDMI21_RS_1_reg,
        .hdmi21_rs_2 = HDMI21_P2_FEC_FEC_HDMI21_RS_2_reg,
        .hdmi21_rs_3 = HDMI21_P2_FEC_FEC_HDMI21_RS_3_reg,

        .hdmi21_fw_rs_0 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_0_reg,
        .hdmi21_fw_rs_1 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_1_reg,
        .hdmi21_fw_rs_2 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_2_reg,
        .hdmi21_fw_rs_3 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_3_reg,
        .hdmi21_fw_rs_4 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_4_reg,
        .hdmi21_fw_rs_5 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_5_reg,
        .hdmi21_fw_rs_6 = HDMI21_P2_FEC_FEC_HDMI21_FW_RS_6_reg,

        .hdmi21_lfsr_1 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_1_reg,
        .hdmi21_lfsr_2 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_2_reg,
        .hdmi21_lfsr_3 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_3_reg,
        .hdmi21_lfsr_4 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_4_reg,
        .hdmi21_lfsr_5 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_5_reg,
        .hdmi21_lfsr_6 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_6_reg,
        .hdmi21_lfsr_7 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_7_reg,
        .hdmi21_lfsr_8 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_8_reg,
        .hdmi21_lfsr_9 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_9_reg,
        .hdmi21_lfsr_10 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_10_reg,
        .hdmi21_lfsr_11 = HDMI21_P2_FEC_FEC_HDMI21_LFSR_11_reg,

        .hdmi21_di_0 = HDMI21_P2_FEC_FEC_HDMI21_DI_0_reg,

        .hdmi21_rs_sram_0 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_0_reg,
        .hdmi21_rs_sram_1 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_1_reg,
        .hdmi21_rs_sram_2 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_2_reg,
        .hdmi21_rs_sram_bist_0 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_BIST_0_reg,
        .hdmi21_rs_sram_bist_1 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_BIST_1_reg,
        .hdmi21_rs_sram_bist_2 = HDMI21_P2_FEC_FEC_HDMI21_RS_SRAM_BIST_2_reg,

        .hdmi21_rs_read_margin = HDMI21_P2_FEC_FEC_HDMI21_RS_READ_MARGIN_reg,
        .hdmi21_rs_read_margin_enable = HDMI21_P2_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_reg,

        .hdmi21_frl_0 = HDMI21_P2_FEC_FEC_HDMI21_FRL_0_reg,
        .hdmi21_frl_1 = HDMI21_P2_FEC_FEC_HDMI21_FRL_1_reg,
        .hdmi21_frl_D1 = HDMI21_P2_FEC_FEC_HDMI21_FRL_D1_reg,
        .hdmi21_frl_D2 = HDMI21_P2_FEC_FEC_HDMI21_FRL_D2_reg,

        .hdmi21_test_sel = HDMI21_P2_FEC_FEC_HDMI21_TEST_SEL_reg,

        .hdmi21_lt_9 = HDMI21_P2_FEC_FEC_HDMI21_LT_9_reg,

        .port_page73_hdmi_scr = HDMI21_P2_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg,
        .hdmi_sr = HDMI21_P2_FEC_FEC_HDMI_SR_reg,
        .port_page73_hdmi_bcsr = HDMI21_P2_FEC_FEC_PORT_PAGE73_HDMI_BCSR_reg
    },
    // HDMI2.1 FEC 3
    {
        .hdmi21_rs_0    = HDMI21_P3_FEC_FEC_HDMI21_RS_0_reg,
        .hdmi21_rs_1    = HDMI21_P3_FEC_FEC_HDMI21_RS_1_reg,
        .hdmi21_rs_2    = HDMI21_P3_FEC_FEC_HDMI21_RS_2_reg,
        .hdmi21_rs_3    = HDMI21_P3_FEC_FEC_HDMI21_RS_3_reg,

        .hdmi21_fw_rs_0 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_0_reg,
        .hdmi21_fw_rs_1 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_1_reg,
        .hdmi21_fw_rs_2 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_2_reg,
        .hdmi21_fw_rs_3 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_3_reg,
        .hdmi21_fw_rs_4 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_4_reg,
        .hdmi21_fw_rs_5 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_5_reg,
        .hdmi21_fw_rs_6 = HDMI21_P3_FEC_FEC_HDMI21_FW_RS_6_reg,

        .hdmi21_lfsr_1  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_1_reg,
        .hdmi21_lfsr_2  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_2_reg,
        .hdmi21_lfsr_3  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_3_reg,
        .hdmi21_lfsr_4  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_4_reg,
        .hdmi21_lfsr_5  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_5_reg,
        .hdmi21_lfsr_6  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_6_reg,
        .hdmi21_lfsr_7  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_7_reg,
        .hdmi21_lfsr_8  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_8_reg,
        .hdmi21_lfsr_9  = HDMI21_P3_FEC_FEC_HDMI21_LFSR_9_reg,
        .hdmi21_lfsr_10 = HDMI21_P3_FEC_FEC_HDMI21_LFSR_10_reg,
        .hdmi21_lfsr_11 = HDMI21_P3_FEC_FEC_HDMI21_LFSR_11_reg,

        .hdmi21_di_0    = HDMI21_P3_FEC_FEC_HDMI21_DI_0_reg,

        .hdmi21_rs_sram_0 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_0_reg,
        .hdmi21_rs_sram_1 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_1_reg,
        .hdmi21_rs_sram_2 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_2_reg,
        .hdmi21_rs_sram_bist_0 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_BIST_0_reg,
        .hdmi21_rs_sram_bist_1 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_BIST_1_reg,
        .hdmi21_rs_sram_bist_2 = HDMI21_P3_FEC_FEC_HDMI21_RS_SRAM_BIST_2_reg,

        .hdmi21_rs_read_margin = HDMI21_P3_FEC_FEC_HDMI21_RS_READ_MARGIN_reg,
        .hdmi21_rs_read_margin_enable = HDMI21_P3_FEC_FEC_HDMI21_RS_READ_MARGIN_ENABLE_reg,

        .hdmi21_frl_0 = HDMI21_P3_FEC_FEC_HDMI21_FRL_0_reg,
        .hdmi21_frl_1 = HDMI21_P3_FEC_FEC_HDMI21_FRL_1_reg,
        .hdmi21_frl_D1 = HDMI21_P3_FEC_FEC_HDMI21_FRL_D1_reg,
        .hdmi21_frl_D2 = HDMI21_P3_FEC_FEC_HDMI21_FRL_D2_reg,

        .hdmi21_test_sel = HDMI21_P3_FEC_FEC_HDMI21_TEST_SEL_reg,

        .hdmi21_lt_9 = HDMI21_P3_FEC_FEC_HDMI21_LT_9_reg,

        .port_page73_hdmi_scr = HDMI21_P3_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg,
        .hdmi_sr = HDMI21_P3_FEC_FEC_HDMI_SR_reg,
        .port_page73_hdmi_bcsr = HDMI21_P3_FEC_FEC_PORT_PAGE73_HDMI_BCSR_reg
    }
};

const struct dscd_reg_st dscd_hw[DSCD_NUM] =
{
    {
        .pps_dw00                             = DSCD_P0_PPS_DW00_reg,
        .pps_dw01                             = DSCD_P0_PPS_DW01_reg,
        .pps_dw02                             = DSCD_P0_PPS_DW02_reg,
        .pps_dw03                             = DSCD_P0_PPS_DW03_reg,
        .pps_dw04                             = DSCD_P0_PPS_DW04_reg,
        .pps_dw05                             = DSCD_P0_PPS_DW05_reg,
        .pps_dw06                             = DSCD_P0_PPS_DW06_reg,
        .pps_dw07                             = DSCD_P0_PPS_DW07_reg,
        .pps_dw08                             = DSCD_P0_PPS_DW08_reg,
        .pps_dw09                             = DSCD_P0_PPS_DW09_reg,
        .pps_dw10                             = DSCD_P0_PPS_DW10_reg,
        .pps_dw11                             = DSCD_P0_PPS_DW11_reg,
        .pps_dw12                             = DSCD_P0_PPS_DW12_reg,
        .pps_dw13                             = DSCD_P0_PPS_DW13_reg,
        .pps_dw14                             = DSCD_P0_PPS_DW14_reg,
        .pps_dw15                             = DSCD_P0_PPS_DW15_reg,
        .pps_dw16                             = DSCD_P0_PPS_DW16_reg,
        .pps_dw17                             = DSCD_P0_PPS_DW17_reg,
        .pps_dw18                             = DSCD_P0_PPS_DW18_reg,
        .pps_dw19                             = DSCD_P0_PPS_DW19_reg,
        .pps_dw20                             = DSCD_P0_PPS_DW20_reg,
        .pps_dw21                             = DSCD_P0_PPS_DW21_reg,
        .pps_dw22                             = DSCD_P0_PPS_DW22_reg,
        .pps_dw23                             = DSCD_P0_PPS_DW23_reg,
        .pps_dw24                             = DSCD_P0_PPS_DW24_reg,
        .pps_dw25                             = DSCD_P0_PPS_DW25_reg,
        .pps_dw26                             = DSCD_P0_PPS_DW26_reg,
        .pps_dw27                             = DSCD_P0_PPS_DW27_reg,
        .pps_dw28                             = DSCD_P0_PPS_DW28_reg,
        .pps_dw29                             = DSCD_P0_PPS_DW29_reg,
        .pps_dw30                             = DSCD_P0_PPS_DW30_reg,
        .pps_dw31                             = DSCD_P0_PPS_DW31_reg,
        .pps_dw32                             = DSCD_P0_PPS_DW32_reg,
        .pps_dw33                             = DSCD_P0_PPS_DW33_reg,
        .pps_dw00_cur                         = DSCD_P0_PPS_DW00_CUR_reg,
        .pps_dw01_cur                         = DSCD_P0_PPS_DW01_CUR_reg,
        .pps_dw02_cur                         = DSCD_P0_PPS_DW02_CUR_reg,
        .pps_dw03_cur                         = DSCD_P0_PPS_DW03_CUR_reg,
        .pps_dw04_cur                         = DSCD_P0_PPS_DW04_CUR_reg,
        .pps_dw05_cur                         = DSCD_P0_PPS_DW05_CUR_reg,
        .pps_dw06_cur                         = DSCD_P0_PPS_DW06_CUR_reg,
        .pps_dw07_cur                         = DSCD_P0_PPS_DW07_CUR_reg,
        .pps_dw08_cur                         = DSCD_P0_PPS_DW08_CUR_reg,
        .pps_dw09_cur                         = DSCD_P0_PPS_DW09_CUR_reg,
        .pps_dw10_cur                         = DSCD_P0_PPS_DW10_CUR_reg,
        .pps_dw11_cur                         = DSCD_P0_PPS_DW11_CUR_reg,
        .pps_dw12_cur                         = DSCD_P0_PPS_DW12_CUR_reg,
        .pps_dw13_cur                         = DSCD_P0_PPS_DW13_CUR_reg,
        .pps_dw14_cur                         = DSCD_P0_PPS_DW14_CUR_reg,
        .pps_dw15_cur                         = DSCD_P0_PPS_DW15_CUR_reg,
        .pps_dw16_cur                         = DSCD_P0_PPS_DW16_CUR_reg,
        .pps_dw17_cur                         = DSCD_P0_PPS_DW17_CUR_reg,
        .pps_dw18_cur                         = DSCD_P0_PPS_DW18_CUR_reg,
        .pps_dw19_cur                         = DSCD_P0_PPS_DW19_CUR_reg,
        .pps_dw20_cur                         = DSCD_P0_PPS_DW20_CUR_reg,
        .pps_dw21_cur                         = DSCD_P0_PPS_DW21_CUR_reg,
        .pps_dw22_cur                         = DSCD_P0_PPS_DW22_CUR_reg,
        .pps_dw23_cur                         = DSCD_P0_PPS_DW23_CUR_reg,
        .pps_dw24_cur                         = DSCD_P0_PPS_DW24_CUR_reg,
        .pps_dw25_cur                         = DSCD_P0_PPS_DW25_CUR_reg,
        .pps_dw26_cur                         = DSCD_P0_PPS_DW26_CUR_reg,
        .pps_dw27_cur                         = DSCD_P0_PPS_DW27_CUR_reg,
        .pps_dw28_cur                         = DSCD_P0_PPS_DW28_CUR_reg,
        .pps_dw29_cur                         = DSCD_P0_PPS_DW29_CUR_reg,
        .pps_dw30_cur                         = DSCD_P0_PPS_DW30_CUR_reg,
        .pps_dw31_cur                         = DSCD_P0_PPS_DW31_CUR_reg,
        .pps_dw32_cur                         = DSCD_P0_PPS_DW32_CUR_reg,
        .pps_dw33_cur                         = DSCD_P0_PPS_DW33_CUR_reg,
        .db_ctrl                              = DSCD_P0_DB_CTRL_reg,
        .ctrl                                 = DSCD_P0_CTRL_reg,
        .status                               = DSCD_P0_STATUS_reg,
        .wde_dscd                             = DSCD_P0_WDE_DSCD_reg,
        .inte_dscd                            = DSCD_P0_INTE_DSCD_reg,
        .ints_dscd                            = DSCD_P0_INTS_DSCD_reg,
        .inte_dscd_2                          = DSCD_P0_INTE_DSCD_2_reg,
        .ints_dscd_2                          = DSCD_P0_INTS_DSCD_2_reg,
        .dummy                                = DSCD_P0_dummy_reg,
        .debug                                = DSCD_P0_debug_reg,
        .performance                          = DSCD_P0_performance_reg,
        .out_iv_delay                         = DSCD_P0_OUT_IV_DELAY_reg,
        .out_vs_width                         = DSCD_P0_OUT_VS_WIDTH_reg,
        .out_den_delay                        = DSCD_P0_OUT_DEN_DELAY_reg,
        .out_swap                             = DSCD_P0_OUT_SWAP_reg,
        .gt_cycle_measure_ctrl                = DSCD_P0_GT_CYCLE_MEASURE_CTRL_reg,
        .gt_cycle_measure_result_0            = DSCD_P0_GT_CYCLE_MEASURE_RESULT_0_reg,
        .gt_cycle_measure_result_1            = DSCD_P0_GT_CYCLE_MEASURE_RESULT_1_reg,
        .gt_cycle_measure_result_2            = DSCD_P0_GT_CYCLE_MEASURE_RESULT_2_reg,
        .out_measure                          = DSCD_P0_OUT_MEASURE_reg,
        .out_measure_outbuf1                  = DSCD_P0_OUT_MEASURE_OUTBUF1_reg,
        .dscd_timing_monitor_ctrl             = DSCD_P0_DSCD_Timing_monitor_ctrl_reg,
        .dscd_timing_monitor_multi            = DSCD_P0_DSCD_Timing_monitor_multi_reg,
        .dscd_timing_monitor_compare_th       = DSCD_P0_DSCD_Timing_monitor_compare_th_reg,
        .dscd_timing_monitor_golden           = DSCD_P0_DSCD_Timing_monitor_golden_reg,
        .dscd_timing_monitor_result1          = DSCD_P0_DSCD_Timing_monitor_result1_reg,
        .dscd_timing_monitor_result2          = DSCD_P0_DSCD_Timing_monitor_result2_reg,
        .dscd_timing_monitor_result3          = DSCD_P0_DSCD_Timing_monitor_result3_reg,
        .dscd_timing_monitor_result4          = DSCD_P0_DSCD_Timing_monitor_result4_reg,
        .dscd_timing_monitor_max              = DSCD_P0_DSCD_Timing_monitor_max_reg,
        .dscd_timing_monitor_min              = DSCD_P0_DSCD_Timing_monitor_min_reg,
        .dscd_timing_monitor_st               = DSCD_P0_DSCD_Timing_monitor_st_reg,
        .dscd_out_crc_en                      = DSCD_P0_DSCD_out_crc_en_reg,
        .dscd_out_crc_result                  = DSCD_P0_DSCD_out_crc_result_reg,
        .dscd_in_crc_en                       = DSCD_P0_Dscd_in_crc_en_reg,
        .dscd_in_crc_result                   = DSCD_P0_Dscd_in_crc_result_reg,
        .dscd_pps_crc_en                      = DSCD_P0_DSCD_pps_crc_en_reg,
        .dscd_pps_crc_result                  = DSCD_P0_DSCD_pps_crc_result_reg,
        .dscd_dbg_en                          = DSCD_P0_DSCD_dbg_en_reg,
        .dscd_dbg1                            = DSCD_P0_DSCD_dbg1_reg,
        .dscd_dbg1_size                       = DSCD_P0_DSCD_dbg1_size_reg,
        .dscd_dbg_sram                        = DSCD_P0_DSCD_dbg_sram_reg,
        .dscd_sram_d0                         = DSCD_P0_DSCD_sram_d0_reg,
        .dscd_sram_d1                         = DSCD_P0_DSCD_sram_d1_reg,
        .dscd_sram_d2                         = DSCD_P0_DSCD_sram_d2_reg,
        .dscd_sram_d3                         = DSCD_P0_DSCD_sram_d3_reg,
        .dscd_sram_d4                         = DSCD_P0_DSCD_sram_d4_reg,
        .dscd_sram_d5                         = DSCD_P0_DSCD_sram_d5_reg,
        .dscd_sram_d6                         = DSCD_P0_DSCD_sram_d6_reg,
        .dscd_sram_d7                         = DSCD_P0_DSCD_sram_d7_reg,
        .dscd_dbg_sram_rw                     = DSCD_P0_DSCD_dbg_sram_rw_reg,
        .dscd_sram_q0                         = DSCD_P0_DSCD_sram_q0_reg,
        .dscd_sram_q1                         = DSCD_P0_DSCD_sram_q1_reg,
        .dscd_sram_q2                         = DSCD_P0_DSCD_sram_q2_reg,
        .dscd_sram_q3                         = DSCD_P0_DSCD_sram_q3_reg,
        .dscd_sram_q4                         = DSCD_P0_DSCD_sram_q4_reg,
        .dscd_sram_q5                         = DSCD_P0_DSCD_sram_q5_reg,
        .dscd_sram_q6                         = DSCD_P0_DSCD_sram_q6_reg,
        .dscd_sram_q7                         = DSCD_P0_DSCD_sram_q7_reg,
        .dscd_timing_gen_ctrl                 = DSCD_P0_DSCD_TIMING_GEN_CTRL_reg,
        .dscd_timing_gen_sta                  = DSCD_P0_DSCD_TIMING_GEN_STA_reg,
        .dscd_timing_gen_size                 = DSCD_P0_DSCD_TIMING_GEN_SIZE_reg,
        .dscd_timing_gen_end                  = DSCD_P0_DSCD_TIMING_GEN_END_reg,
        .dscd_timing_gen_frame_rate           = DSCD_P0_DSCD_TIMING_GEN_FRAME_RATE_reg,
        .dscd_pattern_gen_ctrl                = DSCD_P0_DSCD_PATTERN_GEN_CTRL_reg,
        .dscd_pattern_gen_c0_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C0_RGB_reg,
        .dscd_pattern_gen_c1_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C1_RGB_reg,
        .dscd_pattern_gen_c2_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C2_RGB_reg,
        .dscd_pattern_gen_c3_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C3_RGB_reg,
        .dscd_pattern_gen_c4_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C4_RGB_reg,
        .dscd_pattern_gen_c5_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C5_RGB_reg,
        .dscd_pattern_gen_c6_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C6_RGB_reg,
        .dscd_pattern_gen_c7_rgb              = DSCD_P0_DSCD_PATTERN_GEN_C7_RGB_reg,
        .dscd_pattern_gen_b_msb               = DSCD_P0_DSCD_PATTERN_GEN_B_MSB_reg,
        .dscd_mbist_read_margin_0             = DSCD_P0_DSCD_MBIST_READ_MARGIN_0_reg,
        .dscd_mbist_read_margin_1             = DSCD_P0_DSCD_MBIST_READ_MARGIN_1_reg,
        .dscd_bist_mode                       = DSCD_P0_DSCD_BIST_MODE_reg,
        .dscd_bist_done                       = DSCD_P0_DSCD_BIST_DONE_reg,
        .dscd_bist_fail_group                 = DSCD_P0_DSCD_BIST_FAIL_GROUP_reg,
        .dscd_bist_fail                       = DSCD_P0_DSCD_BIST_FAIL_reg,
        .dscd_drf_bist_mode                   = DSCD_P0_DSCD_DRF_BIST_MODE_reg,
        .dscd_drf_bist_resume                 = DSCD_P0_DSCD_DRF_BIST_RESUME_reg,
        .dscd_drf_bist_done                   = DSCD_P0_DSCD_DRF_BIST_DONE_reg,
        .dscd_drf_bist_fail_group             = DSCD_P0_DSCD_DRF_BIST_FAIL_GROUP_reg,
        .dscd_drf_bist_fail                   = DSCD_P0_DSCD_DRF_BIST_FAIL_reg,
        .dscd_drf_bist_pause                  = DSCD_P0_DSCD_DRF_BIST_PAUSE_reg,
        .dscd_bist_t12_test                   = DSCD_P0_DSCD_BIST_T12_TEST_reg,
    },
    {
        .pps_dw00                             = DSCD_P1_PPS_DW00_reg,
        .pps_dw01                             = DSCD_P1_PPS_DW01_reg,
        .pps_dw02                             = DSCD_P1_PPS_DW02_reg,
        .pps_dw03                             = DSCD_P1_PPS_DW03_reg,
        .pps_dw04                             = DSCD_P1_PPS_DW04_reg,
        .pps_dw05                             = DSCD_P1_PPS_DW05_reg,
        .pps_dw06                             = DSCD_P1_PPS_DW06_reg,
        .pps_dw07                             = DSCD_P1_PPS_DW07_reg,
        .pps_dw08                             = DSCD_P1_PPS_DW08_reg,
        .pps_dw09                             = DSCD_P1_PPS_DW09_reg,
        .pps_dw10                             = DSCD_P1_PPS_DW10_reg,
        .pps_dw11                             = DSCD_P1_PPS_DW11_reg,
        .pps_dw12                             = DSCD_P1_PPS_DW12_reg,
        .pps_dw13                             = DSCD_P1_PPS_DW13_reg,
        .pps_dw14                             = DSCD_P1_PPS_DW14_reg,
        .pps_dw15                             = DSCD_P1_PPS_DW15_reg,
        .pps_dw16                             = DSCD_P1_PPS_DW16_reg,
        .pps_dw17                             = DSCD_P1_PPS_DW17_reg,
        .pps_dw18                             = DSCD_P1_PPS_DW18_reg,
        .pps_dw19                             = DSCD_P1_PPS_DW19_reg,
        .pps_dw20                             = DSCD_P1_PPS_DW20_reg,
        .pps_dw21                             = DSCD_P1_PPS_DW21_reg,
        .pps_dw22                             = DSCD_P1_PPS_DW22_reg,
        .pps_dw23                             = DSCD_P1_PPS_DW23_reg,
        .pps_dw24                             = DSCD_P1_PPS_DW24_reg,
        .pps_dw25                             = DSCD_P1_PPS_DW25_reg,
        .pps_dw26                             = DSCD_P1_PPS_DW26_reg,
        .pps_dw27                             = DSCD_P1_PPS_DW27_reg,
        .pps_dw28                             = DSCD_P1_PPS_DW28_reg,
        .pps_dw29                             = DSCD_P1_PPS_DW29_reg,
        .pps_dw30                             = DSCD_P1_PPS_DW30_reg,
        .pps_dw31                             = DSCD_P1_PPS_DW31_reg,
        .pps_dw32                             = DSCD_P1_PPS_DW32_reg,
        .pps_dw33                             = DSCD_P1_PPS_DW33_reg,
        .pps_dw00_cur                         = DSCD_P1_PPS_DW00_CUR_reg,
        .pps_dw01_cur                         = DSCD_P1_PPS_DW01_CUR_reg,
        .pps_dw02_cur                         = DSCD_P1_PPS_DW02_CUR_reg,
        .pps_dw03_cur                         = DSCD_P1_PPS_DW03_CUR_reg,
        .pps_dw04_cur                         = DSCD_P1_PPS_DW04_CUR_reg,
        .pps_dw05_cur                         = DSCD_P1_PPS_DW05_CUR_reg,
        .pps_dw06_cur                         = DSCD_P1_PPS_DW06_CUR_reg,
        .pps_dw07_cur                         = DSCD_P1_PPS_DW07_CUR_reg,
        .pps_dw08_cur                         = DSCD_P1_PPS_DW08_CUR_reg,
        .pps_dw09_cur                         = DSCD_P1_PPS_DW09_CUR_reg,
        .pps_dw10_cur                         = DSCD_P1_PPS_DW10_CUR_reg,
        .pps_dw11_cur                         = DSCD_P1_PPS_DW11_CUR_reg,
        .pps_dw12_cur                         = DSCD_P1_PPS_DW12_CUR_reg,
        .pps_dw13_cur                         = DSCD_P1_PPS_DW13_CUR_reg,
        .pps_dw14_cur                         = DSCD_P1_PPS_DW14_CUR_reg,
        .pps_dw15_cur                         = DSCD_P1_PPS_DW15_CUR_reg,
        .pps_dw16_cur                         = DSCD_P1_PPS_DW16_CUR_reg,
        .pps_dw17_cur                         = DSCD_P1_PPS_DW17_CUR_reg,
        .pps_dw18_cur                         = DSCD_P1_PPS_DW18_CUR_reg,
        .pps_dw19_cur                         = DSCD_P1_PPS_DW19_CUR_reg,
        .pps_dw20_cur                         = DSCD_P1_PPS_DW20_CUR_reg,
        .pps_dw21_cur                         = DSCD_P1_PPS_DW21_CUR_reg,
        .pps_dw22_cur                         = DSCD_P1_PPS_DW22_CUR_reg,
        .pps_dw23_cur                         = DSCD_P1_PPS_DW23_CUR_reg,
        .pps_dw24_cur                         = DSCD_P1_PPS_DW24_CUR_reg,
        .pps_dw25_cur                         = DSCD_P1_PPS_DW25_CUR_reg,
        .pps_dw26_cur                         = DSCD_P1_PPS_DW26_CUR_reg,
        .pps_dw27_cur                         = DSCD_P1_PPS_DW27_CUR_reg,
        .pps_dw28_cur                         = DSCD_P1_PPS_DW28_CUR_reg,
        .pps_dw29_cur                         = DSCD_P1_PPS_DW29_CUR_reg,
        .pps_dw30_cur                         = DSCD_P1_PPS_DW30_CUR_reg,
        .pps_dw31_cur                         = DSCD_P1_PPS_DW31_CUR_reg,
        .pps_dw32_cur                         = DSCD_P1_PPS_DW32_CUR_reg,
        .pps_dw33_cur                         = DSCD_P1_PPS_DW33_CUR_reg,
        .db_ctrl                              = DSCD_P1_DB_CTRL_reg,
        .ctrl                                 = DSCD_P1_CTRL_reg,
        .status                               = DSCD_P1_STATUS_reg,
        .wde_dscd                             = DSCD_P1_WDE_DSCD_reg,
        .inte_dscd                            = DSCD_P1_INTE_DSCD_reg,
        .ints_dscd                            = DSCD_P1_INTS_DSCD_reg,
        .inte_dscd_2                          = DSCD_P1_INTE_DSCD_2_reg,
        .ints_dscd_2                          = DSCD_P1_INTS_DSCD_2_reg,
        .dummy                                = DSCD_P1_dummy_reg,
        .debug                                = DSCD_P1_debug_reg,
        .performance                          = DSCD_P1_performance_reg,
        .out_iv_delay                         = DSCD_P1_OUT_IV_DELAY_reg,
        .out_vs_width                         = DSCD_P1_OUT_VS_WIDTH_reg,
        .out_den_delay                        = DSCD_P1_OUT_DEN_DELAY_reg,
        .out_swap                             = DSCD_P1_OUT_SWAP_reg,
        .gt_cycle_measure_ctrl                = DSCD_P1_GT_CYCLE_MEASURE_CTRL_reg,
        .gt_cycle_measure_result_0            = DSCD_P1_GT_CYCLE_MEASURE_RESULT_0_reg,
        .gt_cycle_measure_result_1            = DSCD_P1_GT_CYCLE_MEASURE_RESULT_1_reg,
        .gt_cycle_measure_result_2            = DSCD_P1_GT_CYCLE_MEASURE_RESULT_2_reg,
        .out_measure                          = DSCD_P1_OUT_MEASURE_reg,
        .out_measure_outbuf1                  = DSCD_P1_OUT_MEASURE_OUTBUF1_reg,
        .dscd_timing_monitor_ctrl             = DSCD_P1_DSCD_Timing_monitor_ctrl_reg,
        .dscd_timing_monitor_multi            = DSCD_P1_DSCD_Timing_monitor_multi_reg,
        .dscd_timing_monitor_compare_th       = DSCD_P1_DSCD_Timing_monitor_compare_th_reg,
        .dscd_timing_monitor_golden           = DSCD_P1_DSCD_Timing_monitor_golden_reg,
        .dscd_timing_monitor_result1          = DSCD_P1_DSCD_Timing_monitor_result1_reg,
        .dscd_timing_monitor_result2          = DSCD_P1_DSCD_Timing_monitor_result2_reg,
        .dscd_timing_monitor_result3          = DSCD_P1_DSCD_Timing_monitor_result3_reg,
        .dscd_timing_monitor_result4          = DSCD_P1_DSCD_Timing_monitor_result4_reg,
        .dscd_timing_monitor_max              = DSCD_P1_DSCD_Timing_monitor_max_reg,
        .dscd_timing_monitor_min              = DSCD_P1_DSCD_Timing_monitor_min_reg,
        .dscd_timing_monitor_st               = DSCD_P1_DSCD_Timing_monitor_st_reg,
        .dscd_out_crc_en                      = DSCD_P1_DSCD_out_crc_en_reg,
        .dscd_out_crc_result                  = DSCD_P1_DSCD_out_crc_result_reg,
        .dscd_in_crc_en                       = DSCD_P1_Dscd_in_crc_en_reg,
        .dscd_in_crc_result                   = DSCD_P1_Dscd_in_crc_result_reg,
        .dscd_pps_crc_en                      = DSCD_P1_DSCD_pps_crc_en_reg,
        .dscd_pps_crc_result                  = DSCD_P1_DSCD_pps_crc_result_reg,
        .dscd_dbg_en                          = DSCD_P1_DSCD_dbg_en_reg,
        .dscd_dbg1                            = DSCD_P1_DSCD_dbg1_reg,
        .dscd_dbg1_size                       = DSCD_P1_DSCD_dbg1_size_reg,
        .dscd_dbg_sram                        = DSCD_P1_DSCD_dbg_sram_reg,
        .dscd_sram_d0                         = DSCD_P1_DSCD_sram_d0_reg,
        .dscd_sram_d1                         = DSCD_P1_DSCD_sram_d1_reg,
        .dscd_sram_d2                         = DSCD_P1_DSCD_sram_d2_reg,
        .dscd_sram_d3                         = DSCD_P1_DSCD_sram_d3_reg,
        .dscd_sram_d4                         = DSCD_P1_DSCD_sram_d4_reg,
        .dscd_sram_d5                         = DSCD_P1_DSCD_sram_d5_reg,
        .dscd_sram_d6                         = DSCD_P1_DSCD_sram_d6_reg,
        .dscd_sram_d7                         = DSCD_P1_DSCD_sram_d7_reg,
        .dscd_dbg_sram_rw                     = DSCD_P1_DSCD_dbg_sram_rw_reg,
        .dscd_sram_q0                         = DSCD_P1_DSCD_sram_q0_reg,
        .dscd_sram_q1                         = DSCD_P1_DSCD_sram_q1_reg,
        .dscd_sram_q2                         = DSCD_P1_DSCD_sram_q2_reg,
        .dscd_sram_q3                         = DSCD_P1_DSCD_sram_q3_reg,
        .dscd_sram_q4                         = DSCD_P1_DSCD_sram_q4_reg,
        .dscd_sram_q5                         = DSCD_P1_DSCD_sram_q5_reg,
        .dscd_sram_q6                         = DSCD_P1_DSCD_sram_q6_reg,
        .dscd_sram_q7                         = DSCD_P1_DSCD_sram_q7_reg,
        .dscd_timing_gen_ctrl                 = DSCD_P1_DSCD_TIMING_GEN_CTRL_reg,
        .dscd_timing_gen_sta                  = DSCD_P1_DSCD_TIMING_GEN_STA_reg,
        .dscd_timing_gen_size                 = DSCD_P1_DSCD_TIMING_GEN_SIZE_reg,
        .dscd_timing_gen_end                  = DSCD_P1_DSCD_TIMING_GEN_END_reg,
        .dscd_timing_gen_frame_rate           = DSCD_P1_DSCD_TIMING_GEN_FRAME_RATE_reg,
        .dscd_pattern_gen_ctrl                = DSCD_P1_DSCD_PATTERN_GEN_CTRL_reg,
        .dscd_pattern_gen_c0_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C0_RGB_reg,
        .dscd_pattern_gen_c1_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C1_RGB_reg,
        .dscd_pattern_gen_c2_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C2_RGB_reg,
        .dscd_pattern_gen_c3_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C3_RGB_reg,
        .dscd_pattern_gen_c4_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C4_RGB_reg,
        .dscd_pattern_gen_c5_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C5_RGB_reg,
        .dscd_pattern_gen_c6_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C6_RGB_reg,
        .dscd_pattern_gen_c7_rgb              = DSCD_P1_DSCD_PATTERN_GEN_C7_RGB_reg,
        .dscd_pattern_gen_b_msb               = DSCD_P1_DSCD_PATTERN_GEN_B_MSB_reg,
        .dscd_mbist_read_margin_0             = DSCD_P1_DSCD_MBIST_READ_MARGIN_0_reg,
        .dscd_mbist_read_margin_1             = DSCD_P1_DSCD_MBIST_READ_MARGIN_1_reg,
        .dscd_bist_mode                       = DSCD_P1_DSCD_BIST_MODE_reg,
        .dscd_bist_done                       = DSCD_P1_DSCD_BIST_DONE_reg,
        .dscd_bist_fail_group                 = DSCD_P1_DSCD_BIST_FAIL_GROUP_reg,
        .dscd_bist_fail                       = DSCD_P1_DSCD_BIST_FAIL_reg,
        .dscd_drf_bist_mode                   = DSCD_P1_DSCD_DRF_BIST_MODE_reg,
        .dscd_drf_bist_resume                 = DSCD_P1_DSCD_DRF_BIST_RESUME_reg,
        .dscd_drf_bist_done                   = DSCD_P1_DSCD_DRF_BIST_DONE_reg,
        .dscd_drf_bist_fail_group             = DSCD_P1_DSCD_DRF_BIST_FAIL_GROUP_reg,
        .dscd_drf_bist_fail                   = DSCD_P1_DSCD_DRF_BIST_FAIL_reg,
        .dscd_drf_bist_pause                  = DSCD_P1_DSCD_DRF_BIST_PAUSE_reg,
        .dscd_bist_t12_test                   = DSCD_P1_DSCD_BIST_T12_TEST_reg,
    },
};
