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
 * Mac register setting
 *--------------------------------------------------*/
const struct hdmi_mac_reg_st mac[3] =
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
        .hdmi_20_21_ctrl              = HDMI_P0_hdmi_20_21_ctrl_reg,
        .irq                          = HDMI_P0_IRQ_reg,
        .mp                           = HDMI_P0_mp_reg,
        .hdmi_v_bypass                = HDMI_P0_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P0_hdmi_mp_reg,
        .debug_port                   = HDMI_P0_debug_port_reg,
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
        .irq                          = HDMI_P1_IRQ_reg,
        .mp                           = HDMI_P1_mp_reg,
        .hdmi_v_bypass                = HDMI_P1_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P1_hdmi_mp_reg,
        .debug_port                   = HDMI_P1_debug_port_reg,
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
        .irq                          = HDMI_P2_IRQ_reg,
        .mp                           = HDMI_P2_mp_reg,
        .hdmi_v_bypass                = HDMI_P2_hdmi_v_bypass_reg,
        .hdmi_mp                      = HDMI_P2_hdmi_mp_reg,
        .debug_port                   = HDMI_P2_debug_port_reg,
        .hdmi_rst0                    = HDMI_P2_hdmi_rst0_reg,
        .hdmi_rst3                    = HDMI_P2_hdmi_rst3_reg,
        .hdmi_clken0                  = HDMI_P2_hdmi_clken0_reg,
        .hdmi_clken3                  = HDMI_P2_hdmi_clken3_reg,
    },
};



const struct hdmi_stb_mac_reg_st stb_mac[3] =
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
};
