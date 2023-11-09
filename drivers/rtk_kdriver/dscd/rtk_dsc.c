#include <rtk_kdriver/dscd/rtk_dsc.h>
#include "rtk_dscd-priv.h"

DSCD_CVTEM_INFO dscd_cvtem_info[6];// 4 hdmi port + 2 dp port

spinlock_t dscd_spin_lock;

void rtk_dscd_dump_dsc_pps(RTK_DSC_PPS *p_pps)
{
    unsigned char *rc_p = p_pps->rc_parameter_set;

    RTK_DSC_INFO("RTK_DSC_PPS: dsc_version=%d.%d, pps_id=%d, bits_per_comp=%d, line_buffer_depth=%d, block_pred_enable=%d\n",
                 p_pps->dsc_version_major,
                 p_pps->dsc_version_minor,
                 p_pps->pps_identifier,
                 p_pps->bits_per_component,
                 p_pps->line_buffer_depth,
                 p_pps->block_pred_enable);

    RTK_DSC_INFO("RTK_DSC_PPS: convert_rgb=%d, simple_422=%d, vbr_enable=%d, bits_per_pixel=%d\n",
                 p_pps->convert_rgb,
                 p_pps->simple_422,
                 p_pps->vbr_enable,
                 p_pps->bits_per_pixel);


    RTK_DSC_INFO("RTK_DSC_PPS: pic_height=%d, pic_width=%d, slice_height=%d, slice_width=%d, chunk_size=%d\n",
                 p_pps->pic_height,
                 p_pps->pic_width,
                 p_pps->slice_height,
                 p_pps->slice_width,
                 p_pps->chunk_size);

    RTK_DSC_INFO("RTK_DSC_PPS: initial_xmit_delay=%d, initial_dec_delay=%d, initial_scale_value=%d, scale_increment_interval=%d, scale_decrement_interval=%d\n",
                 p_pps->initial_xmit_delay,
                 p_pps->initial_dec_delay,
                 p_pps->initial_scale_value,
                 p_pps->scale_increment_interval,
                 p_pps->scale_decrement_interval);

    RTK_DSC_INFO("RTK_DSC_PPS: first_line_bpg_offset=%d, nfl_bpg_offset=%d, slice_bpg_offset=%d, initial_offset=%d, final_offset=%d\n",
                 p_pps->first_line_bpg_offset,
                 p_pps->nfl_bpg_offset,
                 p_pps->slice_bpg_offset,
                 p_pps->initial_offset,
                 p_pps->final_offset);

    RTK_DSC_INFO("RTK_DSC_PPS: flatness_min_qp=%d, flatness_max_qp=%d\n",
                 p_pps->flatness_min_qp,
                 p_pps->flatness_max_qp);


    RTK_DSC_INFO("RTK_DSC_PPS[36:37]: native_422=%d, flatness_max_qp=%d\n",
                 p_pps->flatness_min_qp,
                 p_pps->flatness_max_qp);


    // PPS38 - 87
    RTK_DSC_INFO("RTK_DSC_PPS[38:53]:RC[0] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 rc_p[0], rc_p[1], rc_p[2], rc_p[3],
                 rc_p[4], rc_p[5], rc_p[6], rc_p[7],
                 rc_p[8], rc_p[9], rc_p[10], rc_p[11],
                 rc_p[12], rc_p[13], rc_p[14], rc_p[15]);

    RTK_DSC_INFO("RTK_DSC_PPS[54:69]:RC[16] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 rc_p[16], rc_p[17], rc_p[18], rc_p[19],
                 rc_p[20], rc_p[21], rc_p[22], rc_p[23],
                 rc_p[24], rc_p[25], rc_p[26], rc_p[27],
                 rc_p[28], rc_p[29], rc_p[30], rc_p[31]);

    RTK_DSC_INFO("RTK_DSC_PPS[70:85]:RC[32] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 rc_p[32], rc_p[33], rc_p[34], rc_p[35],
                 rc_p[36], rc_p[37], rc_p[38], rc_p[39],
                 rc_p[40], rc_p[41], rc_p[42], rc_p[43],
                 rc_p[44], rc_p[45], rc_p[46], rc_p[47]);

    RTK_DSC_INFO("RTK_DSC_PPS[86:87]:RC[48] = %02x %02x\n", rc_p[48], rc_p[49]);

    // PPS88-91
    RTK_DSC_INFO("RTK_DSC_PPS[88:91]: native_422=%d, native_420=%d, second_line_bpg_offset=%d, nsl_bpg_offset=%d, second_line_offset_adj=%d\n",
                 p_pps->native_422,
                 p_pps->native_420,
                 p_pps->second_line_bpg_offset,
                 p_pps->nsl_bpg_offset,
                 p_pps->second_line_offset_adj);
}

EXPORT_SYMBOL(rtk_dscd_dump_dsc_pps);

void rtk_dscd_parse_dsc_pps(unsigned char pps[RTK_DSC_PPS_LEN], RTK_DSC_PPS *p_pps)
{
    // PPS0
    p_pps->dsc_version_major = ((pps[0] >> 4) & 0xF);
    p_pps->dsc_version_minor = (pps[0] & 0xF);

    // PPS1
    p_pps->pps_identifier = pps[1];

    // PPS2
    // PPS3
    p_pps->bits_per_component = ((pps[3] >> 4) & 0xF); // [7:4]
    p_pps->line_buffer_depth  = ((pps[3]) & 0xF);     // [3:0]


    // PPS4~5
    p_pps->block_pred_enable = ((pps[4] >> 5) & 0x1); // [5]
    p_pps->convert_rgb       = ((pps[4] >> 4) & 0x1); // [4]
    p_pps->simple_422        = ((pps[4] >> 3) & 0x1); // [3]
    p_pps->vbr_enable        = ((pps[4] >> 2) & 0x1); // [2]
    p_pps->bits_per_pixel    = ((pps[4] & 0x3) << 8) + pps[5];

    // PPS6~7
    p_pps->pic_height        = (pps[6] << 8) + pps[7];

    // PPS8~9
    p_pps->pic_width         = (pps[8] << 8) + pps[9];

    // PPS10-11
    p_pps->slice_height      = (pps[10] << 8) + pps[11];

    // PPS12-13
    p_pps->slice_width       = (pps[12] << 8) + pps[13];

    // PPS14-15
    p_pps->chunk_size        = (pps[14] << 8) + pps[15];

    // PPS16-17
    p_pps->initial_xmit_delay = ((pps[16] & 0x3) << 8) + pps[17];

    // PPS18-19
    p_pps->initial_dec_delay = (pps[18] << 8) + pps[19];

    // PPS20-21
    p_pps->initial_scale_value = pps[21] & 0x3F;

    // PPS22-23
    p_pps->scale_increment_interval = (pps[22] << 8) + pps[23];

    // PPS24-25
    p_pps->scale_decrement_interval = ((pps[24] & 0xF) << 8) + pps[25];

    // PPS26-27
    p_pps->first_line_bpg_offset = pps[27] & 0x1F;

    // PPS28-29
    p_pps->nfl_bpg_offset = (pps[28] << 8) + pps[29];

    // PPS30-31
    p_pps->slice_bpg_offset = (pps[30] << 8) + pps[31];

    // PPS32-33
    p_pps->initial_offset = (pps[32] << 8) + pps[33];

    // PPS34-35
    p_pps->final_offset = (pps[34] << 8) + pps[35];

    // PPS36
    p_pps->flatness_min_qp = pps[36] & 0x1F;

    // PPS37
    p_pps->flatness_max_qp = pps[37] & 0x1F;

    // PPS38 - 87
    memcpy(p_pps->rc_parameter_set, &pps[38], 50);

    // PPS88
    p_pps->native_420 = (pps[88] >> 1) & 0x1;
    p_pps->native_422 = (pps[88] >> 0) & 0x1;

    // PPS89
    p_pps->second_line_bpg_offset = (pps[89] & 0x1F);

    // PPS90~91
    p_pps->nsl_bpg_offset = (pps[90] << 8) + pps[91];

    // PPS92~93
    p_pps->second_line_offset_adj = (pps[92] << 8) + pps[93];
}

EXPORT_SYMBOL(rtk_dscd_parse_dsc_pps);

void dsc_crt_reset(void)
{
    unsigned long flags;

    RTK_DSC_INFO("[CRT][DSCD]dsc_crt_reset!\n");
    spin_lock_irqsave(&dscd_spin_lock, flags);  // add spinlock to prevent racing

    // reset DSC
    // DO CRT reset (I):  RST = L
    rtd_outl(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_dscd_mask);
    udelay(5);

    // DO CRT reset (I):  CLK = OFF
    rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dscd_mask);
    udelay(5);

    // DO CRT reset (I):  CLK = ON
    rtd_outl(SYS_REG_SYS_CLKEN0_reg, (SYS_REG_SYS_CLKEN0_clken_dscd_mask |
                                      SYS_REG_SYS_CLKEN0_write_data_mask));

    udelay(5);
    // DO CRT reset (I):  CLK = OFF
    rtd_outl(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dscd_mask);

    udelay(5);
    // DO CRT reset (I):  RST = H
    rtd_outl(SYS_REG_SYS_SRST0_reg, (SYS_REG_SYS_SRST0_rstn_dscd_mask |
                                     SYS_REG_SYS_SRST0_write_data_mask));
    udelay(5);

    // DO CRT reset (I):  CLK = ON
    rtd_outl(SYS_REG_SYS_CLKEN0_reg, (SYS_REG_SYS_CLKEN0_clken_dscd_mask |
                                      SYS_REG_SYS_CLKEN0_write_data_mask));
    spin_unlock_irqrestore(&dscd_spin_lock, flags);

}

unsigned char rtk_dsc_get_crt_on(void)
{
    return ((rtd_inl(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_dscd_mask) >> SYS_REG_SYS_CLKEN0_clken_dscd_shift);
}
EXPORT_SYMBOL(rtk_dsc_get_crt_on);

void rtk_dsc_set_fw_pps(unsigned char pps[RTK_DSC_PPS_LEN])
{
    unsigned char *p_pps = pps;
    int i;

    for (i = 0; i < RTK_DSC_PPS_LEN; i += 4) {
        rtd_outl(DSCD_PPS_DW00_reg + i, ((p_pps[3] << 24) | (p_pps[2] << 16) | (p_pps[1] << 8) | p_pps[0]));
        p_pps += 4;
    }
}

void rtk_dsc_get_cur_pps(unsigned char pps[RTK_DSC_PPS_LEN])
{
    unsigned char *p_pps = pps;
    int i;
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_cur_pps] DSC CRT not ready!!\n");
        return;
    }

    for (i = 0; i < RTK_DSC_PPS_LEN; i += 4) {
        unsigned int val = rtd_inl(DSCD_PPS_DW00_CUR_reg + i);
        p_pps[0] = val & 0xFF;
        p_pps[1] = (val >> 8) & 0xFF;
        p_pps[2] = (val >> 16) & 0xFF;
        p_pps[3] = (val >> 24) & 0xFF;
        p_pps += 4;
    }
}

unsigned char rtk_dsc_get_dscd_p_n_s_not_finish(void)
{
    return DSCD_INTS_DSCD_get_p_n_s_not_finish_even_irq(rtd_inl(DSCD_INTS_DSCD_reg));
}

void rtk_dsc_clr_dscd_p_n_s_not_finish(void)
{
    //write 1 clear
    rtd_maskl(DSCD_INTS_DSCD_reg, ~DSCD_INTS_DSCD_p_n_s_not_finish_even_irq_mask, DSCD_INTS_DSCD_p_n_s_not_finish_even_irq_mask);
}

unsigned char rtk_dsc_get_dscd_out_pic_finish(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_dscd_out_pic_finish] DSC CRT not ready!!\n");
        return 0;
    }

    return DSCD_INTS_DSCD_2_get_dscd_out_pic_finish_irq(rtd_inl(DSCD_INTS_DSCD_2_reg));
}

void rtk_dsc_clr_dscd_out_pic_finish(void)
{
    //write 1 clear
    rtd_maskl(DSCD_INTS_DSCD_2_reg, ~DSCD_INTS_DSCD_2_dscd_out_pic_finish_irq_mask, DSCD_INTS_DSCD_2_dscd_out_pic_finish_irq_mask);
}


void rtk_dsc_double_buffer_enable(unsigned char enable)
{
    return rtd_maskl(DSCD_DB_CTRL_reg, ~DSCD_DB_CTRL_db_en_mask, (enable) ? DSCD_DB_CTRL_db_en_mask : 0);
}

void rtk_dsc_enable(unsigned char enable)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_enable] DSC CRT not ready!! enable=%d\n", enable);
    }

    return rtd_maskl(DSCD_CTRL_reg, ~DSCD_CTRL_dscd_enable_mask, (enable) ? DSCD_CTRL_dscd_enable_mask : 0);
}
EXPORT_SYMBOL(rtk_dsc_enable);

unsigned char dscd_get_dsc_enable(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[dscd_get_dsc_enable] DSC CRT not ready!!\n");
        return 0;
    }
    return DSCD_CTRL_get_dscd_enable(rtd_inl(DSCD_CTRL_reg));
}

void rtk_dsc_fw_mode_enable(unsigned char enable)
{
    return rtd_maskl(DSCD_CTRL_reg, ~DSCD_CTRL_pps_fw_mode_mask, (enable) ? DSCD_CTRL_pps_fw_mode_mask : 0);
}

void rtk_dsc_set_crc_en(unsigned char enable)
{
    rtd_maskl(DSCD_DSCD_out_crc_en_reg, ~DSCD_DSCD_out_crc_en_dscd_out_crc_en_mask,
              (enable) ? DSCD_DSCD_out_crc_en_dscd_out_crc_en_mask : 0);
    rtd_maskl(DSCD_Dscd_in_crc_en_reg, ~DSCD_Dscd_in_crc_en_dscd_in_crc_en_mask,
              (enable) ? DSCD_Dscd_in_crc_en_dscd_in_crc_en_mask : 0);
    rtd_maskl(DSCD_DSCD_pps_crc_en_reg, ~DSCD_DSCD_pps_crc_en_dscd_pps_crc_en_mask,
              (enable) ? DSCD_DSCD_pps_crc_en_dscd_pps_crc_en_mask : 0);
}

unsigned int rtk_dsc_get_dscd_out_crc(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_dscd_out_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if (!(DSCD_DSCD_out_crc_en_get_dscd_out_crc_en(rtd_inl(DSCD_DSCD_out_crc_en_reg)))) {
        return 0;
    }

    return rtd_inl(DSCD_DSCD_out_crc_result_reg);
}

unsigned int rtk_dsc_get_dscd_in_crc(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_dscd_in_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if (!(DSCD_Dscd_in_crc_en_get_dscd_in_crc_en(rtd_inl(DSCD_Dscd_in_crc_en_reg)))) {
        return 0;
    }

    return rtd_inl(DSCD_Dscd_in_crc_result_reg);
}

unsigned int rtk_dsc_get_dscd_pps_crc(void)
{
    unsigned char retry = 0;
    unsigned int pps_crc = 0;
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_dscd_pps_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if (!(DSCD_DSCD_pps_crc_en_get_dscd_pps_crc_en(rtd_inl(DSCD_DSCD_pps_crc_en_reg)))) {
        return 0;
    }

    for (retry = 0; retry < 10; ++retry) {
        pps_crc = rtd_inl(DSCD_DSCD_pps_crc_result_reg);
        if (pps_crc != 0x1d1d1d1d) {
            break;
        }

        msleep(5);
    }

    if (retry > 1) {
        RTK_DSC_EMG("[rtk_dsc_get_dscd_pps_crc] Get pps_crc 0x%x with retry for %d times!!\n", pps_crc, retry);
    }

    return pps_crc;
}

void rtk_dsc_set_output_hporch(unsigned int value)
{
    if (value < 20) {
        RTK_DSC_EMG("[rtk_dsc_set_output_hporch] must >=20, value =%d\n", value);
        return;
    }

    rtd_maskl(DSCD_OUT_DEN_DELAY_reg, ~DSCD_OUT_DEN_DELAY_dscd_hporch_mask, DSCD_OUT_DEN_DELAY_dscd_hporch(value));
}

unsigned char rtk_dsc_get_output_error_status(void)
{
    unsigned int is_dec_not_finish = 0;
    unsigned int is_in_async_overflow = 0;
    unsigned int is_bs_demux_overflow = 0;
    unsigned int is_rc_fifo_overflow = 0;

    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_output_error_status] DSCD CRT not enable!! DSCD can not work!\n");
        return TRUE;
    }
    is_dec_not_finish = DSCD_INTS_DSCD_get_dec_not_finish_even_irq(rtd_inl(DSCD_INTS_DSCD_reg));
    is_in_async_overflow = DSCD_INTS_DSCD_get_in_async_overflow_even_irq(rtd_inl(DSCD_INTS_DSCD_reg));
    is_bs_demux_overflow = DSCD_INTS_DSCD_get_bs_demux_overflow_even_irq(rtd_inl(DSCD_INTS_DSCD_reg));
    is_rc_fifo_overflow = DSCD_INTS_DSCD_get_rc_fifo_overflow_even_irq(rtd_inl(DSCD_INTS_DSCD_reg));

    if ((is_dec_not_finish | is_in_async_overflow | is_bs_demux_overflow | is_rc_fifo_overflow) > 0) {
        RTK_DSC_WARN("[rtk_dsc_get_output_error_status] is_dec_not_finish=%d, is_in_async_overflow=%d, is_bs_demux_overflow=%d, is_rc_fifo_overflow=%d, INTS=%x, INTS2=%x\n ",
                     is_dec_not_finish, is_in_async_overflow, is_bs_demux_overflow, is_rc_fifo_overflow, rtd_inl(DSCD_INTS_DSCD_reg), rtd_inl(DSCD_INTS_DSCD_2_reg));
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void rtk_dsc_clr_output_error_status(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_clr_output_error_status] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }

    rtd_maskl(DSCD_INTS_DSCD_reg,
              ~(DSCD_INTS_DSCD_dec_not_finish_even_irq_mask | DSCD_INTS_DSCD_in_async_overflow_even_irq_mask |
                DSCD_INTS_DSCD_bs_demux_overflow_even_irq_mask | DSCD_INTS_DSCD_rc_fifo_overflow_even_irq_mask),
              (DSCD_INTS_DSCD_dec_not_finish_even_irq_mask | DSCD_INTS_DSCD_in_async_overflow_even_irq_mask |
               DSCD_INTS_DSCD_bs_demux_overflow_even_irq_mask | DSCD_INTS_DSCD_rc_fifo_overflow_even_irq_mask)); //write 1 clear
}

void rtk_dscd_timing_monitor_en(unsigned char enable)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dscd_timing_monitor_en] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_en_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_en(enable));
}

unsigned char rtk_dscd_is_timing_monitor_en(void)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dscd_is_timing_monitor_completed] DSCD CRT not enable!! DSCD can not work!\n");
        return 0;
    }
    return DSCD_DSCD_Timing_monitor_ctrl_get_timing_monitor_en(rtd_inl(DSCD_DSCD_Timing_monitor_ctrl_reg));
}

void rtk_dsc_set_timing_monitor_sta0(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_sta0] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta0_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta0(sta_end_type));
}

void rtk_dsc_set_timing_monitor_end0(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_end0] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end0_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end0(sta_end_type));
}

void rtk_dsc_set_timing_monitor_sta1(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_sta1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta1_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta1(sta_end_type));
}

void rtk_dsc_set_timing_monitor_end1(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_end1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end1_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end1(sta_end_type));
}

void rtk_dsc_set_timing_monitor_sta2(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_sta1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_multi_reg, ~DSCD_DSCD_Timing_monitor_multi_timing_monitor_sta2_mask, DSCD_DSCD_Timing_monitor_multi_timing_monitor_sta2(sta_end_type));
}

void rtk_dsc_set_timing_monitor_end2(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_set_timing_monitor_end1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    rtd_maskl(DSCD_DSCD_Timing_monitor_multi_reg, ~DSCD_DSCD_Timing_monitor_multi_timing_monitor_end2_mask, DSCD_DSCD_Timing_monitor_multi_timing_monitor_end2(sta_end_type));
}
unsigned int rtk_dsc_get_timing_monitor_result(RTK_DSCD_TIMING_MONITOR_RESULT_INDEX result_index)
{
    //Single mode, all index use sta0 and end0; Muilti mode, each index use different sta0~sta5 and end0~end5
    unsigned int target_reg = 0;
    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[rtk_dsc_get_timing_monitor_result] DSCD CRT not enable!! DSCD can not work!\n");
        return 0;
    }
    if (result_index >= RTK_DSCD_TM_RESULT_NUM) {
        RTK_DSC_EMG("[rtk_dsc_get_timing_monitor_result] Invalid reg address, use wrong result_index = %d\n", result_index);
        return 0;
    }
    target_reg = DSCD_DSCD_Timing_monitor_result1_reg + (result_index << 2);
    return DSCD_DSCD_Timing_monitor_result1_get_t_m_result_previous1(rtd_inl(target_reg)); // bit0~bit27;
}

void rtk_dsc_clr_irq_status(void)
{
    rtd_outl(DSCD_INTS_DSCD_reg, 0xFFFFFFFF);
    rtd_outl(DSCD_INTS_DSCD_2_reg, 0xFFFFFFFF);
}

unsigned int rtk_dsc_get_dscd_irq_status(void)
{
    return rtd_inl(DSCD_INTS_DSCD_reg);
}

unsigned int rtk_dsc_get_dscd_2_irq_status(void)
{
    return rtd_inl(DSCD_INTS_DSCD_2_reg);
}

RTK_DSCD_FSM_STATUS rtk_dscd_fsm_st = {
    .current_dscd_fsm = RTK_DSCD_FSM_INIT,
    .pre_dscd_fsm = RTK_DSCD_FSM_INIT,
    .run_dscd_port = 0xf,
    .last_pps_crc = 0,
    .wait_pps_cnt = 0,
};

EXPORT_SYMBOL(rtk_dscd_fsm_st);

const char *rtk_dsc_fsm_str(RTK_DSCD_FSM_T fsm)
{
    switch (fsm) {
        case RTK_DSCD_FSM_INIT:       return "RTK_DSCD_FSM_INIT";
        case RTK_DSCD_FSM_SET_FW_PPS: return "RTK_DSCD_FSM_SET_FW_PPS";
        case RTK_DSCD_FSM_WAIT_PPS_READY:         return "RTK_DSCD_FSM_WAIT_PPS_READY";
        case RTK_DSCD_FSM_PPS_MEASURE:      return "RTK_DSCD_FSM_PPS_MEASURE";
        case RTK_DSCD_FSM_RUN:     return "RTK_DSCD_FSM_RUN";
        default:
            return "RTK_DSCD FSM UNKNOW";
    }
}
EXPORT_SYMBOL(rtk_dsc_fsm_str);

MEASURE_TIMING_T *dscd_get_current_pps_timing(void)
{
    return &rtk_dscd_fsm_st.pps_timing;
}

HDMI_COLOR_DEPTH_T newbase_dsc_get_colordepth(void)
{
    return rtk_dscd_fsm_st.pps_timing.colordepth;
}


EXPORT_SYMBOL(newbase_dsc_get_colordepth);

/*------------------------------------------------------------------
 * Func : newbase_hdmi_check_dscd_vaild_crc_check
 *
 * Desc : check DSCD output CRC. When DSCD output invalid, crc has special regular symptom. Ex: 0x0c0c0c0c, it is invalid output.
 *           Maybe HDCP2.2 auth is not success. So, DSCD can not decode.
 * Para : [IN] crc  : CRC value for checking
 *
 * Retn :  TRUE: check pass, FALSE: check fail
 *------------------------------------------------------------------*/
unsigned char newbase_check_dscd_vaild_crc_check(unsigned int crc)
{
    //Check CRC for debugging
    unsigned int crc_bit0_3 = crc & 0xF;
    unsigned int crc_bit4_7 = (crc & 0xF0) >> 4;
    unsigned int crc_bit8_11 = (crc & 0xF00) >> 8;
    unsigned int crc_bit12_15 = (crc & 0xF000) >> 12;
    unsigned int crc_bit16_19 = (crc & 0xF0000) >> 16;
    unsigned int crc_bit20_23 = (crc & 0xF00000) >> 20;
    unsigned int crc_bit24_27 = (crc & 0xF000000) >> 24;
    unsigned int crc_bit28_31 = (crc & 0xF0000000) >> 28;
#if 0
    if (GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_CRC_VAILD_EN) == FALSE) {
        return TRUE;    //If CHECK_CRC_VAILD_EN == FALSE, Bypase check, force return TRUE.
    }
#endif
    if ((crc > 0)
            && ((crc_bit0_3 == crc_bit8_11) && (crc_bit8_11 == crc_bit16_19) && (crc_bit16_19 == crc_bit24_27))
            && ((crc_bit4_7 == crc_bit12_15) && (crc_bit12_15 == crc_bit20_23) && (crc_bit20_23 == crc_bit28_31))) {
        //RTK_DSC_WARN("DSCD INVALID CRC=0x%x, crc_bit0_3=%x, crc_bit4_7=%x, \n", crc, crc_bit0_3, crc_bit4_7); //for detail debugging
        return FALSE;
    }
    else {
        return TRUE;
    }
}


unsigned char rtk_check_dscd_output_valid(void)
{
    unsigned char result = FALSE;
    unsigned char wait_count = 10; //GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_CNT_THD);

    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD CRT not enable!! DSCD can not work!\n");
        return FALSE;
    }
#if 0
    if (GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_EN) == FALSE) {
        return TRUE;    //If DSCD_CHECK_OUTPUT_EN == FALSE, Bypase check, force return TRUE.
    }
#endif
    if (GET_DSCD_FSM() < RTK_DSCD_FSM_PPS_MEASURE) {
        RTK_DSC_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD FSM:%s!! DSCD output not ready \n", rtk_dsc_fsm_str(GET_DSCD_FSM()));
        return FALSE;
    }

    rtk_dsc_clr_output_error_status();
    rtk_dsc_clr_dscd_out_pic_finish();    //clear dscd output status

    do {
        unsigned int out_pic_finish = rtk_dsc_get_dscd_out_pic_finish();    // only support for merlin6
        unsigned int out_crc = rtk_dsc_get_dscd_out_crc();
        unsigned int in_crc = rtk_dsc_get_dscd_in_crc();
        unsigned int pps_crc = rtk_dsc_get_dscd_pps_crc();
        unsigned char buf[128];
        unsigned char check_output_crc_vaild = newbase_check_dscd_vaild_crc_check(pps_crc);
        unsigned char is_dscd_no_error = !rtk_dsc_get_output_error_status();
        rtk_dsc_get_cur_pps(buf);

        if (!rtk_dsc_get_crt_on()) {
            RTK_DSC_EMG("[rtk_check_dscd_output_valid] DSCD CRT not enable!! DSCD can not work! round=%d\n", wait_count);
            return FALSE;
        }

        RTK_DSC_WARN("DSCD FSM:%s, dscd_port=%d, count=%d, dscd_en=%d, Check DSCD (gen_finish, crc_vaild, is_no_err)=(%d,%d,%d),(out, in,pps)=(%08x,%08x,%08x),INTS=%x, INTS_2=%x\n",
                     rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port,
                     wait_count,  dscd_get_dsc_enable(), out_pic_finish, check_output_crc_vaild, is_dscd_no_error,
                     out_crc, in_crc,  pps_crc, rtd_inl(DSCD_INTS_DSCD_reg),
                     rtd_inl(DSCD_INTS_DSCD_2_reg));

        if ((out_pic_finish == 1) && (check_output_crc_vaild == TRUE) && (is_dscd_no_error == TRUE)) {
            result = TRUE;
            break;
        }
        else if ((out_pic_finish == 1) && (is_dscd_no_error == FALSE)) {
            // DSCD error, check next frame.
            RTK_DSC_EMG("[newbase_hdmi_check_dscd_output_valid] round:%d, DSCD output decode not ready, check next round!!!\n", wait_count);
            rtk_dsc_clr_output_error_status(); //clear error status and check again
        }

        if (wait_count == 1) {
            result = FALSE;
            RTK_DSC_EMG("[rtk_check_dscd_output_valid] DSCD output vaild check timeout!!\n");
        }

        msleep(40); //wait next v sync

        wait_count --;
    } while (wait_count > 0 && rtk_dsc_get_crt_on());

    return result;
}
EXPORT_SYMBOL(rtk_check_dscd_output_valid);


unsigned char dscd_pps_measure(unsigned char port, MEASURE_TIMING_T *tm, DSCD_CVTEM_INFO *info)
{
    unsigned char hw_pps_buf[RTK_DSC_PPS_LEN];
    RTK_DSC_PPS dscd_hw_pps;

    if (tm == NULL) {
        RTK_DSC_EMG("[newbase_hdmi_hd21_dsc_pps_measure] NULL pointer tm!!\n");
        return FALSE;

    }

    if (!rtk_dsc_get_crt_on()) {
        RTK_DSC_EMG("[newbase_hdmi_hd21_dsc_pps_measure] DSC CRT not ready!!\n");
        return FALSE;
    }

    rtk_dsc_get_cur_pps(hw_pps_buf);

    rtk_dscd_parse_dsc_pps(hw_pps_buf, &dscd_hw_pps);
#if 0 //for debugging, if pps measure fail

    dscd_dump_dsc_pps(&dscd_hw_pps);
#endif
    tm->h_act_len = dscd_hw_pps.pic_width;
    tm->v_act_len = dscd_hw_pps.pic_height;
    tm->h_total = dscd_hw_pps.pic_width + info->hback + info->hfront + info->hsync;
    tm->h_act_sta = info->hsync + info->hback;
    tm->polarity = 3; //force set H/V positive
    tm->IHSyncPulseCount = info->hsync;

    switch (dscd_hw_pps.bits_per_component) {
        case 12:
            tm->colordepth = HDMI_COLOR_DEPTH_12B;
            break;
        case 10:
            tm->colordepth = HDMI_COLOR_DEPTH_10B;
            break;
        case 8:
        default:
            tm->colordepth = HDMI_COLOR_DEPTH_8B;
            break;
    }

    RTK_DSC_WARN("[DSCD] IHTotal: %d\n", tm->h_total);
    RTK_DSC_WARN("[DSCD] IHAct: %d\n", tm->h_act_len);
    RTK_DSC_WARN("[DSCD] IVAct: %d\n", tm->v_act_len);
    RTK_DSC_WARN("[DSCD] IHStr: %d\n", tm->h_act_sta);
    RTK_DSC_WARN("[DSCD] Polarity: %d\n", tm->polarity);
    RTK_DSC_WARN("[DSCD] IHsyncWidth %d\n", tm->IHSyncPulseCount);
    RTK_DSC_WARN("[DSCD] Colordepth %dB (%d)\n", dscd_hw_pps.bits_per_component, tm->colordepth);

    return TRUE;
}


unsigned int rtk_dscd_get_current_output_vbackporch(unsigned int is_clk_div_en)
{
    unsigned int tm_result_clk_cycle = 0;
    unsigned int dscd_output_v_back_porch = 0;
    unsigned int dscd_in_htotal = rtk_dscd_fsm_st.pps_timing.h_total;
    if (dscd_in_htotal == 0) {
        RTK_DSC_WARN("[newbase_dscd_get_current_output_vbackporch] DSCD PPS not ready, can not read vbackporch\n");
        return 0;
    }

    tm_result_clk_cycle = rtk_dsc_get_timing_monitor_result(RTK_DSCD_TM_RESULT_1);

    //if(lib_hdmi_hd21_is_clk_div_en(rtk_dscd_fsm_st.run_dscd_port))
    if (is_clk_div_en) {
        dscd_output_v_back_porch = (tm_result_clk_cycle / (dscd_in_htotal / 4)); // htotal/4
    }
    else {
        dscd_output_v_back_porch = (tm_result_clk_cycle / (dscd_in_htotal / 2)); // htotal/4*2
    }
    RTK_DSC_INFO("[newbase_dscd_get_current_output_vbackporch ] DSCD dscd_output_v_back_porch=%d, dscd_in_htotal=%d, raw_cycle=%d\n", dscd_output_v_back_porch, dscd_in_htotal, tm_result_clk_cycle);

    return dscd_output_v_back_porch;
}
EXPORT_SYMBOL(rtk_dscd_get_current_output_vbackporch);

DSCD_CVTEM_INFO rtk_dsc_get_cvtem_info(unsigned char port, unsigned char is_hdmi)
{
    unsigned char dscd_port = is_hdmi ? port : (port + 4);
    return dscd_cvtem_info[dscd_port];
}

void rtk_dsc_set_cvtem_info(unsigned char port, unsigned char is_hdmi, DSCD_CVTEM_INFO *info)
{
    unsigned char dscd_port = is_hdmi ? port : (port + 4);
    memcpy(&dscd_cvtem_info[dscd_port], info, sizeof(DSCD_CVTEM_INFO));
}

static short rtk_dscd_print_cycle_cnt = 0;
static short rtk_dscd_output_error_cnt = 0;
#define RTK_DSCD_OUTPUT_ERROR_THRESHOLD    5
unsigned short g_rtk_dscd_print_cycle_cnt_thd = 1000;

void dscd_fsm_init_proccess(void)
{
    rtk_dscd_fsm_st.last_pps_crc = 0;
    rtk_dscd_fsm_st.wait_pps_cnt = 0;
    memset(&rtk_dscd_fsm_st.pps_timing, 0, sizeof(MEASURE_TIMING_T));
    rtk_dsc_set_crc_en(1);
    rtk_dsc_fw_mode_enable(0); //disable pps fw mode
    rtk_dscd_timing_monitor_en(FALSE);
    rtk_dsc_enable(0);                    // disable DSC
    SET_DSCD_FSM(RTK_DSCD_FSM_WAIT_PPS_READY);
}

void dscd_fsm_wait_pps_ready_proccess(void)
{
    RTK_DSC_PPS dscd_hw_pps;
    unsigned int current_pps_crc = rtk_dsc_get_dscd_pps_crc();
    //Set timing monitor
    rtk_dsc_set_timing_monitor_sta0(RTK_DSCD_TM_VS_FALLING);
    rtk_dsc_set_timing_monitor_end0(RTK_DSCD_TM_VACT_RISING);
    rtk_dscd_timing_monitor_en(TRUE);
    rtk_dsc_enable(1);                    // enable DSC
    rtk_dsc_get_cur_pps((unsigned char *)&dscd_hw_pps);
    if ((dscd_hw_pps.dsc_version_major > 0) && current_pps_crc > 0) {
        rtk_dscd_fsm_st.last_pps_crc = current_pps_crc;
        RTK_DSC_WARN("[newbase_hdmi_hd21_dsc_handler] GET DSCD PPS, current_pps_crc=%8x\n", current_pps_crc);
        SET_DSCD_FSM(RTK_DSCD_FSM_PPS_MEASURE);
    }
    else {
        msleep(20); //wait at least 1 frame
        if (rtk_dscd_fsm_st.wait_pps_cnt < 3) {
            rtk_dscd_fsm_st.wait_pps_cnt ++;
            SET_DSCD_FSM(RTK_DSCD_FSM_WAIT_PPS_READY);
        }
        else {
            RTK_DSC_WARN("[newbase_hdmi_hd21_dsc_handler] Wait DSCD HW PPS timeout!!current_pps_crc=%8x, dsc_version_major=%d\n", current_pps_crc, dscd_hw_pps.dsc_version_major);
            SET_DSCD_FSM(RTK_DSCD_FSM_INIT);
        }
    }
}

void dscd_fsm_pps_measure_proccess(unsigned char port, HDMI_COLOR_SPACE_T color_space)
{
    unsigned int current_pps_crc = 0;
    msleep(20); //wait at least 1 frame

    current_pps_crc =  rtk_dsc_get_dscd_pps_crc();
    if (current_pps_crc != rtk_dscd_fsm_st.last_pps_crc) {
        //check measure pass and crc no change
        RTK_DSC_INFO("DSCD FSM:%s,  dscd_port=%d,  Check PPS_CRC  fail!!current_pps_crc =%8x,  original_crc=%8x\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port, current_pps_crc, rtk_dscd_fsm_st.last_pps_crc);
        rtk_dscd_fsm_st.wait_pps_cnt = 0;
        SET_DSCD_FSM(RTK_DSCD_FSM_WAIT_PPS_READY);
    }
    else if (!dscd_pps_measure(rtk_dscd_fsm_st.run_dscd_port, &rtk_dscd_fsm_st.pps_timing, &dscd_cvtem_info[port])) {
        RTK_DSC_INFO("DSCD FSM:%s, dscd_port=%d, Check PPS Measure fail!!\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port);
        SET_DSCD_FSM(RTK_DSCD_FSM_WAIT_PPS_READY);
    }
    else {
        int dsc_output_hporch = rtk_dscd_fsm_st.pps_timing.h_total - rtk_dscd_fsm_st.pps_timing.h_act_len;
        if (color_space == COLOR_YUV420) {
            dsc_output_hporch >>= 1;
            RTK_DSC_INFO("DSCD YUV420, Hporch need divide 2, dsc_output_hporch= %d;PPS: htotal=%d, h_act_sta=%d, h_act_len=%d, v_act_len=%d\n",
                         dsc_output_hporch, rtk_dscd_fsm_st.pps_timing.h_total, rtk_dscd_fsm_st.pps_timing.h_act_sta, rtk_dscd_fsm_st.pps_timing.h_act_len, rtk_dscd_fsm_st.pps_timing.v_act_len);

            if (dsc_output_hporch <= RTK_DSCD_OUTPUT_HPORCH_MIN) {
                dsc_output_hporch = RTK_DSCD_OUTPUT_HPORCH_MIN;
            }
        }
        else {
            RTK_DSC_INFO("DSCD NOT YUV420,  dsc_output_hporch= %d;PPS: h_act_sta=%d, h_act_len=%d, v_act_len=%d\n",
                         dsc_output_hporch, rtk_dscd_fsm_st.pps_timing.h_act_sta, rtk_dscd_fsm_st.pps_timing.h_act_len, rtk_dscd_fsm_st.pps_timing.v_act_len);
        }

        if ((dsc_output_hporch >= RTK_DSCD_OUTPUT_HPORCH_MIN) && (dsc_output_hporch < rtk_dscd_fsm_st.pps_timing.h_act_len) && (rtk_dscd_fsm_st.pps_timing.h_act_len > 0) && (rtk_dscd_fsm_st.pps_timing.v_act_len > 0)) {
            rtk_dsc_clr_irq_status();//clear all status before dscd run

            rtk_dsc_set_output_hporch(dsc_output_hporch >> 2); // 4pixel mode, need to div4
            RTK_DSC_INFO("DSCD FSM:%s, dscd_port=%d, Check PPS Pass!!start to run dsc!!\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port);
            SET_DSCD_FSM(RTK_DSCD_FSM_RUN);

        }
        else {
            RTK_DSC_INFO("DSCD FSM:%s, Invalid dsc_output_hporch, h_total=%d, h_act_len=%d, pps_crc=0x%x\n",  rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.pps_timing.h_total, rtk_dscd_fsm_st.pps_timing.h_act_len, rtk_dsc_get_dscd_pps_crc());
            rtk_dscd_fsm_st.wait_pps_cnt = 0;
            SET_DSCD_FSM(RTK_DSCD_FSM_WAIT_PPS_READY);
        }
    }
}

void dscd_fsm_pps_run_proccess(unsigned char port)
{
    unsigned int current_pps_crc = 0;
    current_pps_crc =  rtk_dsc_get_dscd_pps_crc();

    //condition checking
    if (rtk_dscd_fsm_st.run_dscd_port != port) {
        RTK_DSC_INFO("DSCD FSM:%s, HDMI Port Change!! from %d to %d\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port, port);
        SET_DSCD_FSM(RTK_DSCD_FSM_INIT);
    }
    else if (current_pps_crc != rtk_dscd_fsm_st.last_pps_crc) {
        RTK_DSC_INFO("DSCD FSM:%s,  dscd_port=%d, PPS Change!! current_pps_crc =%8x,  original_crc=%8x\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port, current_pps_crc, rtk_dscd_fsm_st.last_pps_crc);
        rtk_dscd_fsm_st.last_pps_crc = current_pps_crc;
        SET_DSCD_FSM(RTK_DSCD_FSM_INIT);
    }
}

unsigned char dscd_is_cvtem_ready(unsigned char port)
{
    if ((dscd_cvtem_info[port].hfront == 0) || (dscd_cvtem_info[port].hsync == 0) || (dscd_cvtem_info[port].hback == 0)) {
        return FALSE;
    }
    return TRUE;
}

void rtk_dsc_handler(unsigned char port, unsigned char is_hdmi, HDMI_COLOR_SPACE_T color_space)
{
    // when cvtem info packet received,link training pass ,then run dsc flow, otherwise return
    unsigned char dscd_port = is_hdmi ? port : (port + 4);
    if (dscd_is_cvtem_ready(dscd_port) == FALSE) {
        return;
    }

    if (!rtk_dsc_get_crt_on()) {
        dsc_crt_reset();
        RTK_DSC_INFO("[rtk_dsc_handler port:%d][DSC] Detected CVTEM!!, Start DSC CRT and Handler\n", port);
        rtk_dscd_output_error_cnt = 0;
        return;
    }

    if (rtk_dscd_fsm_st.run_dscd_port != dscd_port) {
        RTK_DSC_INFO("DSCD FSM:%s, HDMI Port change from %d to %d, reset FSM\n", rtk_dsc_fsm_str(GET_DSCD_FSM()), rtk_dscd_fsm_st.run_dscd_port, port);
        rtk_dscd_fsm_st.run_dscd_port = dscd_port;
        SET_DSCD_FSM(RTK_DSCD_FSM_INIT);
    }

    if (GET_DSCD_FSM() >= RTK_DSCD_FSM_PPS_MEASURE) {
        if (rtk_dsc_get_output_error_status()) {
            rtk_dscd_output_error_cnt++;
            RTK_DSC_ERR("DSCD output error detected (%d/%d)!!\n", rtk_dscd_output_error_cnt, RTK_DSCD_OUTPUT_ERROR_THRESHOLD);
            rtk_dsc_clr_output_error_status(); //clear error status and check again

            if (rtk_dscd_output_error_cnt > RTK_DSCD_OUTPUT_ERROR_THRESHOLD) {
                RTK_DSC_ERR("DSCD output error timeout, force reset DSCD FSM!!\n");
                SET_DSCD_FSM(RTK_DSCD_FSM_INIT);
                rtk_dscd_output_error_cnt = 0;
            }
        }
        else {
            if (rtk_dscd_output_error_cnt) {
                RTK_DSC_INFO("DSCD output error has been recovered (cnt=%d -> 0)!!\n", rtk_dscd_output_error_cnt);
                rtk_dscd_output_error_cnt = 0;
            }
        }
    }

    switch (GET_DSCD_FSM()) {
        case RTK_DSCD_FSM_INIT:
            dscd_fsm_init_proccess();
            break;
        case RTK_DSCD_FSM_WAIT_PPS_READY:
            dscd_fsm_wait_pps_ready_proccess();
            break;
        case RTK_DSCD_FSM_SET_FW_PPS:
            break;
        case RTK_DSCD_FSM_PPS_MEASURE:
            dscd_fsm_pps_measure_proccess(dscd_port, color_space);
            break;

        case RTK_DSCD_FSM_RUN:
            dscd_fsm_pps_run_proccess(dscd_port);
            break;

        default:
            break;
    }

    if (rtk_dscd_fsm_st.pre_dscd_fsm != rtk_dscd_fsm_st.current_dscd_fsm) {
        // force print when fsm change
        rtk_dscd_fsm_st.pre_dscd_fsm = rtk_dscd_fsm_st.current_dscd_fsm;
        rtk_dscd_print_cycle_cnt = g_rtk_dscd_print_cycle_cnt_thd;
    }

    if ((GET_DSCD_FSM() > RTK_DSCD_FSM_INIT) && (rtk_dscd_print_cycle_cnt++ >= g_rtk_dscd_print_cycle_cnt_thd)) {
        unsigned out_crc = rtk_dsc_get_dscd_out_crc();
        unsigned in_crc = rtk_dsc_get_dscd_in_crc();
        unsigned pps_crc = rtk_dsc_get_dscd_pps_crc();

        RTK_DSC_INFO("DSCD FSM:%s,(Current,DSC) port=(%d,%d), dscd_en=%d, (out, in,pps)=(%08x,%08x,%08x), PPS hactive:%d, vactive:%d, INTS=%x, INTS_2=%x\n",
                     rtk_dsc_fsm_str(GET_DSCD_FSM()),
                     port, rtk_dscd_fsm_st.run_dscd_port,
                     dscd_get_dsc_enable(),
                     out_crc, in_crc, pps_crc,
                     rtk_dscd_fsm_st.pps_timing.h_act_len,
                     rtk_dscd_fsm_st.pps_timing.v_act_len,
                     rtk_dsc_get_dscd_irq_status(),
                     rtk_dsc_get_dscd_2_irq_status());

        rtk_dscd_print_cycle_cnt = 0;
    }
}
EXPORT_SYMBOL(rtk_dsc_handler);

MODULE_LICENSE("GPL");

