#include "rbus/dscd_reg.h"
#include "rbus/sys_reg_reg.h"

#include "hdmi_common.h"
#include "hdmi_debug.h"
#include "hdmi_reg.h"
#include "hdmi_dsc.h"
#include "hdmi_vfe_config.h"
#include "hdmiPlatform.h"
extern spinlock_t hdmi_spin_lock;


unsigned char fw_pps[128] =
{
#if 1
// QD980, VIC 97, 4k60
    0x12 ,0x00 ,0x00 ,0x8d ,0x30 ,0xc0 ,0x08 ,0x70 ,0x0f ,0x00 ,0x08 ,0x70 ,0x07 ,0x80 ,0x0b ,0x40,
    0x01 ,0x55 ,0x05 ,0x20 ,0x00 ,0x0a ,0xad ,0x99 ,0x01 ,0x40 ,0x00 ,0x0f ,0x00 ,0x0f ,0x00 ,0x0a,
    0x08 ,0x00 ,0x10 ,0xf4 ,0x03 ,0x0c ,0x20 ,0x00 ,0x06 ,0x0b ,0x0b ,0x33 ,0x0e ,0x1c ,0x2a ,0x38,
    0x46 ,0x54 ,0x62 ,0x69 ,0x70 ,0x77 ,0x79 ,0x7b ,0x7d ,0x7e ,0x00 ,0x82 ,0x00 ,0xc0 ,0x09 ,0x00,
    0x09 ,0x7e ,0x19 ,0xbc ,0x19 ,0xba ,0x19 ,0xf8 ,0x1a ,0x38 ,0x1a ,0x38 ,0x1a ,0x76 ,0x2a ,0x76,
    0x2a ,0x76 ,0x2a ,0x74 ,0x3a ,0xb4 ,0x52 ,0xf4 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
#else
// VIC 118, 4k120
    0x12 ,0x00 ,0x00 ,0x8d ,0x30 ,0xc0 ,0x08 ,0x70 ,0x0f ,0x00 ,0x08 ,0x70 ,0x03 ,0xc0 ,0x05 ,0xa0,
    0x01 ,0x55 ,0x03 ,0x90 ,0x00 ,0x0a ,0x7f ,0xa5 ,0x00 ,0xa0 ,0x00 ,0x0f ,0x00 ,0x0f ,0x00 ,0x13,
    0x08 ,0x00 ,0x10 ,0xf4 ,0x03 ,0x0c ,0x20 ,0x00 ,0x06 ,0x0b ,0x0b ,0x33 ,0x0e ,0x1c ,0x2a ,0x38,
    0x46 ,0x54 ,0x62 ,0x69 ,0x70 ,0x77 ,0x79 ,0x7b ,0x7d ,0x7e ,0x00 ,0x82 ,0x00 ,0xc0 ,0x09 ,0x00,
    0x09 ,0x7e ,0x19 ,0xbc ,0x19 ,0xba ,0x19 ,0xf8 ,0x1a ,0x38 ,0x1a ,0x38 ,0x1a ,0x76 ,0x2a ,0x76,
    0x2a ,0x76 ,0x2a ,0x74 ,0x3a ,0xb4 ,0x52 ,0xf4 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
    0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
#endif
};

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];

static short hdmi_dscd_print_cycle_cnt = 0;
static short hdmi_dscd_output_error_cnt = 0;
#define DSCD_OUTPUT_ERROR_RECOVER_EN    GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_OUTPUT_ERROR_RECOVERY_EN)
#define DSCD_OUTPUT_ERROR_THRESHOLD     GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_OUTPUT_ERROR_RECOVERY_THD)

void _dump_dsc_pps(DSC_PPS* p_pps)
{
    unsigned char* rc_p = p_pps->rc_parameter_set;

    HDMI_DSC_INFO("DSC_PPS: dsc_version=%d.%d, pps_id=%d, bits_per_comp=%d, line_buffer_depth=%d, block_pred_enable=%d\n",
                    p_pps->dsc_version_major,
                    p_pps->dsc_version_minor,
                    p_pps->pps_identifier,
                    p_pps->bits_per_component,
                    p_pps->line_buffer_depth,
                    p_pps->block_pred_enable);

    HDMI_DSC_INFO("DSC_PPS: convert_rgb=%d, simple_422=%d, vbr_enable=%d, bits_per_pixel=%d\n",
                    p_pps->convert_rgb,
                    p_pps->simple_422,
                    p_pps->vbr_enable,
                    p_pps->bits_per_pixel);


    HDMI_DSC_INFO("DSC_PPS: pic_height=%d, pic_width=%d, slice_height=%d, slice_width=%d, chunk_size=%d\n",
                p_pps->pic_height,
                p_pps->pic_width,
                p_pps->slice_height,
                p_pps->slice_width,
                p_pps->chunk_size);

    HDMI_DSC_INFO("DSC_PPS: initial_xmit_delay=%d, initial_dec_delay=%d, initial_scale_value=%d, scale_increment_interval=%d, scale_decrement_interval=%d\n",
                p_pps->initial_xmit_delay,
                p_pps->initial_dec_delay,
                p_pps->initial_scale_value,
                p_pps->scale_increment_interval,
                p_pps->scale_decrement_interval);

    HDMI_DSC_INFO("DSC_PPS: first_line_bpg_offset=%d, nfl_bpg_offset=%d, slice_bpg_offset=%d, initial_offset=%d, final_offset=%d\n",
                p_pps->first_line_bpg_offset,
                p_pps->nfl_bpg_offset,
                p_pps->slice_bpg_offset,
                p_pps->initial_offset,
                p_pps->final_offset);

    HDMI_DSC_INFO("DSC_PPS: flatness_min_qp=%d, flatness_max_qp=%d\n",
                p_pps->flatness_min_qp,
                p_pps->flatness_max_qp);


    HDMI_DSC_INFO("DSC_PPS[36:37]: native_422=%d, flatness_max_qp=%d\n",
                p_pps->flatness_min_qp,
                p_pps->flatness_max_qp);


    // PPS38 - 87
    HDMI_DSC_INFO("DSC_PPS[38:53]:RC[0] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                rc_p[0], rc_p[1], rc_p[2], rc_p[3],
                rc_p[4], rc_p[5], rc_p[6], rc_p[7],
                rc_p[8], rc_p[9], rc_p[10], rc_p[11],
                rc_p[12], rc_p[13], rc_p[14], rc_p[15]);

    HDMI_DSC_INFO("DSC_PPS[54:69]:RC[16] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                rc_p[16], rc_p[17], rc_p[18], rc_p[19],
                rc_p[20], rc_p[21], rc_p[22], rc_p[23],
                rc_p[24], rc_p[25], rc_p[26], rc_p[27],
                rc_p[28], rc_p[29], rc_p[30], rc_p[31]);

    HDMI_DSC_INFO("DSC_PPS[70:85]:RC[32] = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                rc_p[32], rc_p[33], rc_p[34], rc_p[35],
                rc_p[36], rc_p[37], rc_p[38], rc_p[39],
                rc_p[40], rc_p[41], rc_p[42], rc_p[43],
                rc_p[44], rc_p[45], rc_p[46], rc_p[47]);

    HDMI_DSC_INFO("DSC_PPS[86:87]:RC[48] = %02x %02x\n", rc_p[48], rc_p[49]);

    // PPS88-91
    HDMI_DSC_INFO("DSC_PPS[88:91]: native_422=%d, native_420=%d, second_line_bpg_offset=%d, nsl_bpg_offset=%d, second_line_offset_adj=%d\n",
                p_pps->native_422,
                p_pps->native_420,
                p_pps->second_line_bpg_offset,
                p_pps->nsl_bpg_offset,
                p_pps->second_line_offset_adj);
}


void _parse_dsc_pps(unsigned char pps[DSC_PPS_LEN], DSC_PPS* p_pps)
{
    // PPS0
    p_pps->dsc_version_major = ((pps[0]>>4) & 0xF);
    p_pps->dsc_version_minor = (pps[0] & 0xF);

    // PPS1
    p_pps->pps_identifier = pps[1];

    // PPS2
    // PPS3
    p_pps->bits_per_component = ((pps[3]>>4) & 0xF);  // [7:4]
    p_pps->line_buffer_depth  = ((pps[3]) & 0xF);     // [3:0]


    // PPS4~5
    p_pps->block_pred_enable = ((pps[4]>>5) & 0x1);  // [5]
    p_pps->convert_rgb       = ((pps[4]>>4) & 0x1);  // [4]
    p_pps->simple_422        = ((pps[4]>>3) & 0x1);  // [3]
    p_pps->vbr_enable        = ((pps[4]>>2) & 0x1);  // [2]
    p_pps->bits_per_pixel    = ((pps[4] & 0x3)<<8) + pps[5];

    // PPS6~7
    p_pps->pic_height        = (pps[6]<<8) + pps[7];

    // PPS8~9
    p_pps->pic_width         = (pps[8]<<8) + pps[9];

    // PPS10-11
    p_pps->slice_height      = (pps[10]<<8) + pps[11];

    // PPS12-13
    p_pps->slice_width       = (pps[12] <<8) + pps[13];

    // PPS14-15
    p_pps->chunk_size        = (pps[14] <<8) + pps[15];

    // PPS16-17
    p_pps->initial_xmit_delay= ((pps[16] & 0x3) <<8) + pps[17];

    // PPS18-19
    p_pps->initial_dec_delay = (pps[18] <<8) + pps[19];

    // PPS20-21
    p_pps->initial_scale_value = pps[21] & 0x3F;

    // PPS22-23
    p_pps->scale_increment_interval = (pps[22] <<8) + pps[23];

    // PPS24-25
    p_pps->scale_decrement_interval = ((pps[24] & 0xF) <<8) + pps[25];

    // PPS26-27
    p_pps->first_line_bpg_offset = pps[27] & 0x1F;

    // PPS28-29
    p_pps->nfl_bpg_offset = (pps[28] <<8) + pps[29];

    // PPS30-31
    p_pps->slice_bpg_offset = (pps[30] <<8) + pps[31];

    // PPS32-33
    p_pps->initial_offset = (pps[32] <<8) + pps[33];

    // PPS34-35
    p_pps->final_offset = (pps[34] <<8) + pps[35];

    // PPS36
    p_pps->flatness_min_qp = pps[36] & 0x1F;

    // PPS37
    p_pps->flatness_max_qp = pps[37] & 0x1F;

    // PPS38 - 87
    memcpy(p_pps->rc_parameter_set, &pps[38], 50);

    // PPS88
    p_pps->native_420 = (pps[88]>>1) & 0x1;
    p_pps->native_422 = (pps[88]>>0) & 0x1;

    // PPS89
    p_pps->second_line_bpg_offset = (pps[89]& 0x1F);

    // PPS90~91
    p_pps->nsl_bpg_offset =  (pps[90] <<8) + pps[91];

    // PPS92~93
    p_pps->second_line_offset_adj =  (pps[92] <<8) + pps[93];
}


void lib_hdmi_dsc_crt_reset(void)
{
    unsigned long flags;

    HDMI_INFO("[CRT][DSCD]lib_hdmi_dsc_crt_reset!\n");
    spin_lock_irqsave(&hdmi_spin_lock, flags);  // add spinlock to prevent racing

    // reset DSC
    // DO CRT reset (I):  RST = L
    hdmi_out(SYS_REG_SYS_SRST0_reg, SYS_REG_SYS_SRST0_rstn_dscd_mask);
    udelay(5);

    // DO CRT reset (I):  CLK = OFF
    hdmi_out(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dscd_mask);
    udelay(5);

    // DO CRT reset (I):  CLK = ON
    hdmi_out(SYS_REG_SYS_CLKEN0_reg, (SYS_REG_SYS_CLKEN0_clken_dscd_mask|
                                      SYS_REG_SYS_CLKEN0_write_data_mask));

    udelay(5);
    // DO CRT reset (I):  CLK = OFF
    hdmi_out(SYS_REG_SYS_CLKEN0_reg, SYS_REG_SYS_CLKEN0_clken_dscd_mask);

    udelay(5);
    // DO CRT reset (I):  RST = H
    hdmi_out(SYS_REG_SYS_SRST0_reg, (SYS_REG_SYS_SRST0_rstn_dscd_mask|                                     
                                     SYS_REG_SYS_SRST0_write_data_mask));
    udelay(5);

    // DO CRT reset (I):  CLK = ON
    hdmi_out(SYS_REG_SYS_CLKEN0_reg, (SYS_REG_SYS_CLKEN0_clken_dscd_mask|                                      
                                      SYS_REG_SYS_CLKEN0_write_data_mask));
    spin_unlock_irqrestore(&hdmi_spin_lock, flags);

}

unsigned char lib_hdmi_dsc_get_crt_on(void)
{
    return ((hdmi_in(SYS_REG_SYS_CLKEN0_reg) & SYS_REG_SYS_CLKEN0_clken_dscd_mask) >> SYS_REG_SYS_CLKEN0_clken_dscd_shift);
}

void lib_hdmi_dsc_set_fw_pps(unsigned char pps[DSC_PPS_LEN])
{
    unsigned char* p_pps = pps;
    int i;

    for (i=0; i<DSC_PPS_LEN; i+=4)
    {
        hdmi_out(DSCD_PPS_DW00_reg +i, ((p_pps[3]<<24)|(p_pps[2]<<16)|(p_pps[1]<<8)|p_pps[0]));
        p_pps += 4;
    }
}

void lib_hdmi_dsc_get_cur_pps(unsigned char pps[DSC_PPS_LEN])
{
    unsigned char* p_pps = pps;
    int i;
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_cur_pps] DSC CRT not ready!!\n");
        return;
    }

    for (i=0; i<DSC_PPS_LEN; i+=4)
    {
        unsigned int val = hdmi_in(DSCD_PPS_DW00_CUR_reg +i);
        p_pps[0] = val & 0xFF;
        p_pps[1] = (val>>8) & 0xFF;
        p_pps[2] = (val>>16) & 0xFF;
        p_pps[3] = (val>>24) & 0xFF;
        p_pps += 4;
    }
}

unsigned char lib_hdmi_dsc_get_dscd_p_n_s_not_finish(void)
{
    return DSCD_INTS_DSCD_get_p_n_s_not_finish_even_irq(hdmi_in(DSCD_INTS_DSCD_reg));
}

void lib_hdmi_dsc_clr_dscd_p_n_s_not_finish(void)
{//write 1 clear
    hdmi_mask(DSCD_INTS_DSCD_reg, ~DSCD_INTS_DSCD_p_n_s_not_finish_even_irq_mask, DSCD_INTS_DSCD_p_n_s_not_finish_even_irq_mask);
}

unsigned char lib_hdmi_dsc_get_dscd_out_pic_finish(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_dscd_out_pic_finish] DSC CRT not ready!!\n");
        return 0;
    }

    return DSCD_INTS_DSCD_2_get_dscd_out_pic_finish_irq(hdmi_in(DSCD_INTS_DSCD_2_reg));
}

void lib_hdmi_dsc_clr_dscd_out_pic_finish(void)
{//write 1 clear
    hdmi_mask(DSCD_INTS_DSCD_2_reg, ~DSCD_INTS_DSCD_2_dscd_out_pic_finish_irq_mask, DSCD_INTS_DSCD_2_dscd_out_pic_finish_irq_mask);
}


void lib_hdmi_dsc_double_buffer_enable(unsigned char enable)
{
    return hdmi_mask(DSCD_DB_CTRL_reg, ~DSCD_DB_CTRL_db_en_mask, (enable) ? DSCD_DB_CTRL_db_en_mask : 0);
}

void lib_hdmi_dsc_enable(unsigned char enable)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_enable] DSC CRT not ready!! enable=%d\n", enable);
    }

    return hdmi_mask(DSCD_CTRL_reg, ~DSCD_CTRL_dscd_enable_mask, (enable) ? DSCD_CTRL_dscd_enable_mask : 0);
}

unsigned char lib_hdmi_get_dsc_enable(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_get_dsc_enable] DSC CRT not ready!!\n");
        return 0;
    }
    return DSCD_CTRL_get_dscd_enable(hdmi_in(DSCD_CTRL_reg));
}

void lib_hdmi_dsc_fw_mode_enable(unsigned char enable)
{
    return hdmi_mask(DSCD_CTRL_reg, ~DSCD_CTRL_pps_fw_mode_mask, (enable) ? DSCD_CTRL_pps_fw_mode_mask : 0);
}

void lib_hdmi_dsc_set_crc_en(unsigned char enable)
{
    hdmi_mask(DSCD_DSCD_out_crc_en_reg, ~DSCD_DSCD_out_crc_en_dscd_out_crc_en_mask,
                 (enable) ? DSCD_DSCD_out_crc_en_dscd_out_crc_en_mask : 0);
    hdmi_mask(DSCD_Dscd_in_crc_en_reg, ~DSCD_Dscd_in_crc_en_dscd_in_crc_en_mask,
                 (enable) ? DSCD_Dscd_in_crc_en_dscd_in_crc_en_mask : 0);
    hdmi_mask(DSCD_DSCD_pps_crc_en_reg, ~DSCD_DSCD_pps_crc_en_dscd_pps_crc_en_mask,
                 (enable) ? DSCD_DSCD_pps_crc_en_dscd_pps_crc_en_mask : 0);
}

unsigned int lib_hdmi_dsc_get_dscd_out_crc(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_dscd_out_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if(!(DSCD_DSCD_out_crc_en_get_dscd_out_crc_en(hdmi_in(DSCD_DSCD_out_crc_en_reg))))
        return 0;

    return hdmi_in(DSCD_DSCD_out_crc_result_reg);
}

unsigned int lib_hdmi_dsc_get_dscd_in_crc(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_dscd_in_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if(!(DSCD_Dscd_in_crc_en_get_dscd_in_crc_en(hdmi_in(DSCD_Dscd_in_crc_en_reg))))
        return 0;

    return hdmi_in(DSCD_Dscd_in_crc_result_reg);
}

unsigned int lib_hdmi_dsc_get_dscd_pps_crc(void)
{
    unsigned char retry=0;
    unsigned int pps_crc=0;
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_dscd_pps_crc] DSC CRT not ready!!\n");
        return 0;
    }

    if(!(DSCD_DSCD_pps_crc_en_get_dscd_pps_crc_en(hdmi_in(DSCD_DSCD_pps_crc_en_reg))))
        return 0;

    for(retry=0;retry<10;++retry)
    {
        pps_crc = hdmi_in(DSCD_DSCD_pps_crc_result_reg);
        if(pps_crc!=0x1d1d1d1d)
            break;

        msleep(5);
    }

    if(retry>1)
        HDMI_EMG("[lib_hdmi_dsc_get_dscd_pps_crc] Get pps_crc 0x%x with retry for %d times!!\n",pps_crc,retry);

    return pps_crc;
}

void lib_hdmi_dsc_set_output_hporch(unsigned int value)
{
    if(value <20)
    {
        HDMI_EMG("[lib_hdmi_dsc_set_output_hporch] must >=20, value =%d\n", value);
        return;
    }

    hdmi_mask(DSCD_OUT_DEN_DELAY_reg, ~DSCD_OUT_DEN_DELAY_dscd_hporch_mask, DSCD_OUT_DEN_DELAY_dscd_hporch(value));
}

unsigned char lib_hdmi_dsc_get_output_error_status(void)
{
    unsigned int is_dec_not_finish = 0;
    unsigned int is_in_async_overflow = 0;
    unsigned int is_bs_demux_overflow = 0;
    unsigned int is_rc_fifo_overflow = 0;

    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_output_error_status] DSCD CRT not enable!! DSCD can not work!\n");
        return TRUE;
    }
    is_dec_not_finish = DSCD_INTS_DSCD_get_dec_not_finish_even_irq(hdmi_in(DSCD_INTS_DSCD_reg));
    is_in_async_overflow = DSCD_INTS_DSCD_get_in_async_overflow_even_irq(hdmi_in(DSCD_INTS_DSCD_reg));
    is_bs_demux_overflow = DSCD_INTS_DSCD_get_bs_demux_overflow_even_irq(hdmi_in(DSCD_INTS_DSCD_reg));
    is_rc_fifo_overflow = DSCD_INTS_DSCD_get_rc_fifo_overflow_even_irq(hdmi_in(DSCD_INTS_DSCD_reg));

    if((is_dec_not_finish|is_in_async_overflow|is_bs_demux_overflow|is_rc_fifo_overflow) >0)
    {
        HDMI_WARN("[lib_hdmi_dsc_get_output_error_status] is_dec_not_finish=%d, is_in_async_overflow=%d, is_bs_demux_overflow=%d, is_rc_fifo_overflow=%d, INTS=%x, INTS2=%x\n ",
            is_dec_not_finish, is_in_async_overflow, is_bs_demux_overflow, is_rc_fifo_overflow, hdmi_in(DSCD_INTS_DSCD_reg), hdmi_in(DSCD_INTS_DSCD_2_reg));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void lib_hdmi_dsc_clr_output_error_status(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_clr_output_error_status] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }

    hdmi_mask(DSCD_INTS_DSCD_reg,
        ~(DSCD_INTS_DSCD_dec_not_finish_even_irq_mask|DSCD_INTS_DSCD_in_async_overflow_even_irq_mask|
        DSCD_INTS_DSCD_bs_demux_overflow_even_irq_mask|DSCD_INTS_DSCD_rc_fifo_overflow_even_irq_mask),
        (DSCD_INTS_DSCD_dec_not_finish_even_irq_mask|DSCD_INTS_DSCD_in_async_overflow_even_irq_mask|
        DSCD_INTS_DSCD_bs_demux_overflow_even_irq_mask|DSCD_INTS_DSCD_rc_fifo_overflow_even_irq_mask));//write 1 clear
}

void lib_hdmi_dscd_timing_monitor_en(unsigned char enable)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dscd_timing_monitor_en] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_en_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_en(enable));
}

unsigned char lib_hdmi_dscd_is_timing_monitor_en(void)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dscd_is_timing_monitor_completed] DSCD CRT not enable!! DSCD can not work!\n");
        return 0;
    }
    return DSCD_DSCD_Timing_monitor_ctrl_get_timing_monitor_en(hdmi_in(DSCD_DSCD_Timing_monitor_ctrl_reg));
}

void lib_hdmi_dsc_set_timing_monitor_sta0(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_sta0] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta0_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta0(sta_end_type));
}

void lib_hdmi_dsc_set_timing_monitor_end0(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_end0] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end0_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end0(sta_end_type));
}

void lib_hdmi_dsc_set_timing_monitor_sta1(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_sta1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta1_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_sta1(sta_end_type));
}

void lib_hdmi_dsc_set_timing_monitor_end1(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_end1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_ctrl_reg, ~DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end1_mask, DSCD_DSCD_Timing_monitor_ctrl_timing_monitor_end1(sta_end_type));
}

void lib_hdmi_dsc_set_timing_monitor_sta2(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_sta1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_multi_reg, ~DSCD_DSCD_Timing_monitor_multi_timing_monitor_sta2_mask, DSCD_DSCD_Timing_monitor_multi_timing_monitor_sta2(sta_end_type));
}

void lib_hdmi_dsc_set_timing_monitor_end2(DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type)
{
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_set_timing_monitor_end1] DSCD CRT not enable!! DSCD can not work!\n");
        return ;
    }
    hdmi_mask(DSCD_DSCD_Timing_monitor_multi_reg, ~DSCD_DSCD_Timing_monitor_multi_timing_monitor_end2_mask, DSCD_DSCD_Timing_monitor_multi_timing_monitor_end2(sta_end_type));
}
unsigned int lib_hdmi_dsc_get_timing_monitor_result(DSCD_TIMING_MONITOR_RESULT_INDEX result_index)
{//Single mode, all index use sta0 and end0; Muilti mode, each index use different sta0~sta5 and end0~end5
    unsigned int target_reg = 0;
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[lib_hdmi_dsc_get_timing_monitor_result] DSCD CRT not enable!! DSCD can not work!\n");
        return 0;
    }
    if(result_index >= DSCD_TM_RESULT_NUM)
    {
        HDMI_EMG("[lib_hdmi_dsc_get_timing_monitor_result] Invalid reg address, use wrong result_index = %d\n", result_index);
        return 0;
    }
    target_reg = DSCD_DSCD_Timing_monitor_result1_reg + (result_index<<2);
    return DSCD_DSCD_Timing_monitor_result1_get_t_m_result_previous1(hdmi_in(target_reg)); // bit0~bit27;
}

DSCD_FSM_STATUS dscd_fsm_st =
{
    .current_dscd_fsm = DSCD_FSM_WAIT_LT_PASS,
    .pre_dscd_fsm = DSCD_FSM_WAIT_LT_PASS,
    .run_dscd_port = 0xf,
    .last_pps_crc = 0,
    .wait_pps_cnt = 0,
};


const char* _hdmi_hd21_dsc_fsm_str(HDMI_DSCD_FSM_T fsm)
{
    switch (fsm) {
    case DSCD_FSM_WAIT_LT_PASS:       return "DSCD_FSM_WAIT_LT_PASS";
    case DSCD_FSM_INIT:       return "DSCD_FSM_INIT";
    case DSCD_FSM_WAIT_CVTEM: return "DSCD_FSM_WAIT_CVTEM";
    case DSCD_FSM_SET_FW_PPS: return "DSCD_FSM_SET_FW_PPS";
    case DSCD_FSM_WAIT_PPS_READY:         return "DSCD_FSM_WAIT_PPS_READY";
    case DSCD_FSM_PPS_MEASURE:      return "DSCD_FSM_PPS_MEASURE";
    case DSCD_FSM_RUN:     return "DSCD_FSM_RUN";
    default:
    return "DSCD FSM UNKNOW";
    }
}

unsigned char newbase_hdmi_hd21_dsc_pps_measure(unsigned char port, MEASURE_TIMING_T *tm)
{
    unsigned char hw_pps_buf[DSC_PPS_LEN];
    CVTEM_INFO info;
    DSC_PPS dscd_hw_pps;

    if(tm == NULL)
    {
        HDMI_EMG("[newbase_hdmi_hd21_dsc_pps_measure] NULL pointer tm!!\n");
        return FALSE;

    }

    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[newbase_hdmi_hd21_dsc_pps_measure] DSC CRT not ready!!\n");
        return FALSE;
    }

    lib_hdmi_dsc_get_cur_pps(hw_pps_buf);

    if(_parse_cvtem_info(hdmi_rx[port].cvtem_emp, &info) == FALSE)
    {
        HDMI_WARN("[newbase_hdmi_hd21_dsc_pps_measure] Invalid DSC PPS\n");
        _dump_cvtem_info(&info);

        return FALSE;
    }
    _parse_dsc_pps(hw_pps_buf, &dscd_hw_pps);
    #if 0 //for debugging, if pps measure fail

    _dump_dsc_pps(&dscd_hw_pps);
    #endif
    tm->h_act_len = dscd_hw_pps.pic_width;
    tm->v_act_len = dscd_hw_pps.pic_height;
    tm->h_total = dscd_hw_pps.pic_width + info.hback + info.hfront + info.hsync;
    tm->h_act_sta = info.hsync + info.hback;
    tm->polarity = 3; //force set H/V positive
    tm->IHSyncPulseCount = info.hsync;

    switch(dscd_hw_pps.bits_per_component)
    {
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

    HDMI_WARN("[DSCD] IHTotal: %d\n", tm->h_total);
    HDMI_WARN("[DSCD] IHAct: %d\n", tm->h_act_len);
    HDMI_WARN("[DSCD] IVAct: %d\n", tm->v_act_len);
    HDMI_WARN("[DSCD] IHStr: %d\n", tm->h_act_sta);
    HDMI_WARN("[DSCD] Polarity: %d\n", tm->polarity);
    HDMI_WARN("[DSCD] IHsyncWidth %d\n", tm->IHSyncPulseCount);
    HDMI_WARN("[DSCD] Colordepth %dB (%d)\n", dscd_hw_pps.bits_per_component, tm->colordepth);

    return TRUE;

}

MEASURE_TIMING_T* newbase_hdmi_hd21_dsc_get_current_pps_timing(void)
{
    return &dscd_fsm_st.pps_timing;
}

HDMI_COLOR_DEPTH_T newbase_hdmi_hd21_dsc_get_colordepth(void)
{
    return dscd_fsm_st.pps_timing.colordepth;
}


void newbase_hdmi_hd21_dsc_handler(unsigned char port)
{

    if(port >= HDMI_PORT_TOTAL_NUM || newbase_hdmi_get_current_display_port() != port)
    {
        return;
    }

    if(GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_SUPPORT_DSC) &&(hdmi_rx[port].cvtem_emp_cnt>0) &&  !lib_hdmi_dsc_get_crt_on())
    {//  Start DSC handler when detected CVTEM
        HDMI_DSC_INFO("[newbase_hdmi_hd21_dsc_handler port:%d][DSC] Detected CVTEM!!, Start DSC CRT and Handler\n", port);
        lib_hdmi_dsc_crt_reset();
    }

    if(!lib_hdmi_dsc_get_crt_on())
    {
        hdmi_dscd_output_error_cnt = 0;
        return;
    }

    if(dscd_fsm_st.run_dscd_port != port)
    {
        HDMI_DSC_INFO("DSCD FSM:%s, HDMI Port change from %d to %d, reset FSM\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, port);
        dscd_fsm_st.run_dscd_port = port;
        SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
    }
    else if(!lib_hdmi_is_hdmi_21_available(port))//only work for HDMI2.1 port
    {
        SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
    }
    else if(GET_FRL_LT_FSM(port) < LT_FSM_LTSP_PASS)
    {
        SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
    }
    else if (newbase_hdmi_get_hpd(port)==0 || newbase_hdmi_get_5v_state(port)==0)
    {
        SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
    }

    if (DSCD_OUTPUT_ERROR_RECOVER_EN && GET_DSCD_FSM() >= DSCD_FSM_PPS_MEASURE)
    {
        if (lib_hdmi_dsc_get_output_error_status())
        {
            hdmi_dscd_output_error_cnt++;
            HDMI_DSC_ERR("DSCD output error detected (%d/%d)!!\n", hdmi_dscd_output_error_cnt, DSCD_OUTPUT_ERROR_THRESHOLD);            
            lib_hdmi_dsc_clr_output_error_status(); //clear error status and check again   

            if (hdmi_dscd_output_error_cnt > DSCD_OUTPUT_ERROR_THRESHOLD)        
            {
                HDMI_DSC_ERR("DSCD output error timeout, force reset DSCD FSM!!\n");
                SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
                hdmi_dscd_output_error_cnt = 0;
            }
        }
        else
        {
            if (hdmi_dscd_output_error_cnt)
            {
                HDMI_DSC_INFO("DSCD output error has been recovered (cnt=%d -> 0)!!\n", hdmi_dscd_output_error_cnt);
                hdmi_dscd_output_error_cnt = 0;
            }
        }
    }

    switch(GET_DSCD_FSM())
    {
    case DSCD_FSM_WAIT_LT_PASS:
        if(GET_FRL_LT_FSM(port) >= LT_FSM_LTSP_PASS)
        {
            HDMI_DSC_INFO("DSCD FSM:%s, LT PASS, start to run DSC FSM!!\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()));
            lib_hdmi_dsc_crt_reset();
            SET_DSCD_FSM(DSCD_FSM_INIT);
        }
        break;
    case DSCD_FSM_INIT:
        dscd_fsm_st.last_pps_crc = 0;
        dscd_fsm_st.wait_pps_cnt = 0;
        memset(&dscd_fsm_st.pps_timing, 0, sizeof(MEASURE_TIMING_T));
        lib_hdmi_dsc_set_crc_en(1);
        lib_hdmi_dsc_fw_mode_enable(0); //disable pps fw mode
        lib_hdmi_dscd_timing_monitor_en(FALSE);
        newbase_hdmi_hd21_dsc_enanble(dscd_fsm_st.run_dscd_port, 0);     // disable DSC
        hdmi_rx[dscd_fsm_st.run_dscd_port].cvtem_emp_cnt = 0;
#ifdef CONFIG_ARCH_RTK2885P
        hdmi_mask(DSCD_CTRL_reg, ~(DSCD_CTRL_dscd_core_clk_div2_mask), 0);
#endif
        GET_H_RUN_DSC(dscd_fsm_st.run_dscd_port) = FALSE;
        lib_hdmi_hd21_rmzp_clr_less_empacket_err(dscd_fsm_st.run_dscd_port);
        lib_hdmi_hd21_rmzp_clr_more_empacket_err(dscd_fsm_st.run_dscd_port);

        if(g_hdmi_dsc_fw_mode_en >0)
        {
            SET_DSCD_FSM(DSCD_FSM_SET_FW_PPS);
        }
        else
        {
            SET_DSCD_FSM(DSCD_FSM_WAIT_CVTEM);
        }
        break;

    case DSCD_FSM_WAIT_CVTEM:
    {
        if(hdmi_rx[port].cvtem_emp_cnt == 0) 
        {
            SET_DSCD_FSM(DSCD_FSM_WAIT_CVTEM);
        }
        else if (lib_hdmi_hd21_rmzp_get_less_empacket_err(port) || lib_hdmi_hd21_rmzp_get_more_empacket_err(port))
        {
            HDMI_DSC_INFO("DSCD FSM:%s,  dscd_port=%d, CVTEM Packet error! cvtem_emp_cnt=%d, less=%d, more=%d \n", 
                _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, hdmi_rx[port].cvtem_emp_cnt, lib_hdmi_hd21_rmzp_get_less_empacket_err(port), lib_hdmi_hd21_rmzp_get_more_empacket_err(port));
            lib_hdmi_hd21_rmzp_clr_less_empacket_err(dscd_fsm_st.run_dscd_port);
            lib_hdmi_hd21_rmzp_clr_more_empacket_err(dscd_fsm_st.run_dscd_port);

            SET_DSCD_FSM(DSCD_FSM_WAIT_CVTEM);
        }
        else
        {
            newbase_hdmi_hd21_dsc_enanble(port, 1);     // enable DSC, hw will copy cvtem to dscd_pps sram
            //Set timing monitor
            lib_hdmi_dsc_set_timing_monitor_sta0(DSCD_TM_VS_FALLING);
            lib_hdmi_dsc_set_timing_monitor_end0(DSCD_TM_VACT_RISING);
            lib_hdmi_dscd_timing_monitor_en(TRUE);

            SET_DSCD_FSM(DSCD_FSM_WAIT_PPS_READY);
        }
        break;
    }
    case DSCD_FSM_SET_FW_PPS:
    {
        unsigned char cvtem_pps_buf[DSC_PPS_LEN];
        CVTEM_INFO info;

        if(_parse_cvtem_info(hdmi_rx[port].cvtem_emp, &info) == FALSE)
        {
            HDMI_WARN("[newbase_hdmi_hd21_dsc_pps_measure] Invalid DSC PPS beforw set FW PPS\n");
            _dump_cvtem_info(&info);
            SET_DSCD_FSM(DSCD_FSM_WAIT_CVTEM);
            break;
        }
        memcpy(cvtem_pps_buf, info.pps, sizeof(cvtem_pps_buf));
        newbase_hdmi_hd21_dsc_enanble(port, 1);
        lib_hdmi_dsc_fw_mode_enable(1);
        lib_hdmi_dsc_set_fw_pps(cvtem_pps_buf);
        SET_DSCD_FSM(DSCD_FSM_WAIT_PPS_READY);
        break;
    }
    case DSCD_FSM_WAIT_PPS_READY:
    {
        DSC_PPS dscd_hw_pps;
        unsigned int current_pps_crc = lib_hdmi_dsc_get_dscd_pps_crc();

        lib_hdmi_dsc_get_cur_pps((unsigned char*)&dscd_hw_pps);
        if((dscd_hw_pps.dsc_version_major > 0) && current_pps_crc > 0)
        {
            dscd_fsm_st.last_pps_crc = current_pps_crc;
            HDMI_WARN("[newbase_hdmi_hd21_dsc_handler] GET DSCD PPS, current_pps_crc=%8x\n", current_pps_crc);
            SET_DSCD_FSM(DSCD_FSM_PPS_MEASURE);
        }
        else
        {
            msleep(20); //wait at least 1 frame
            if(dscd_fsm_st.wait_pps_cnt<3)
            {
                dscd_fsm_st.wait_pps_cnt ++;
                SET_DSCD_FSM(DSCD_FSM_WAIT_PPS_READY);
            }
            else
            {
                HDMI_WARN("[newbase_hdmi_hd21_dsc_handler] Wait DSCD HW PPS timeout!!current_pps_crc=%8x, dsc_version_major=%d\n", current_pps_crc, dscd_hw_pps.dsc_version_major);
                SET_DSCD_FSM(DSCD_FSM_INIT);
            }
        }
    }
        break;

    case DSCD_FSM_PPS_MEASURE:
    {
        unsigned int current_pps_crc = 0;
        msleep(20); //wait at least 1 frame

        current_pps_crc =  lib_hdmi_dsc_get_dscd_pps_crc();
        if(current_pps_crc != dscd_fsm_st.last_pps_crc)
        {//check measure pass and crc no change
            HDMI_DSC_INFO("DSCD FSM:%s,  dscd_port=%d,  Check PPS_CRC  fail!!current_pps_crc =%8x,  original_crc=%8x\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, current_pps_crc, dscd_fsm_st.last_pps_crc);
            dscd_fsm_st.wait_pps_cnt = 0;
            SET_DSCD_FSM(DSCD_FSM_WAIT_PPS_READY);
        }
        else if(!newbase_hdmi_hd21_dsc_pps_measure(dscd_fsm_st.run_dscd_port, &dscd_fsm_st.pps_timing))
        {
            HDMI_DSC_INFO("DSCD FSM:%s, dscd_port=%d, Check PPS Measure fail!!\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port);
            SET_DSCD_FSM(DSCD_FSM_WAIT_CVTEM);
        }
        else
        {
            int dsc_output_hporch = dscd_fsm_st.pps_timing.h_total - dscd_fsm_st.pps_timing.h_act_len;
            if(GET_H_COLOR_SPACE(dscd_fsm_st.run_dscd_port) == COLOR_YUV420)
            {
                dsc_output_hporch >>=1;
                HDMI_DSC_INFO("DSCD YUV420, Hporch need divide 2, dsc_output_hporch= %d;PPS: htotal=%d, h_act_sta=%d, h_act_len=%d, v_act_len=%d\n",
                    dsc_output_hporch, dscd_fsm_st.pps_timing.h_total , dscd_fsm_st.pps_timing.h_act_sta, dscd_fsm_st.pps_timing.h_act_len, dscd_fsm_st.pps_timing.v_act_len);

                if(dsc_output_hporch<=DSCD_OUTPUT_HPORCH_MIN)
                    dsc_output_hporch = DSCD_OUTPUT_HPORCH_MIN;
            }
            else
            {
                HDMI_DSC_INFO("DSCD NOT YUV420,  dsc_output_hporch= %d;PPS: h_act_sta=%d, h_act_len=%d, v_act_len=%d\n",
                    dsc_output_hporch, dscd_fsm_st.pps_timing.h_act_sta, dscd_fsm_st.pps_timing.h_act_len, dscd_fsm_st.pps_timing.v_act_len);
            }

            if((dsc_output_hporch>=DSCD_OUTPUT_HPORCH_MIN) && (dsc_output_hporch< dscd_fsm_st.pps_timing.h_act_len) && (dscd_fsm_st.pps_timing.h_act_len >0) && (dscd_fsm_st.pps_timing.v_act_len >0))
            {
                hdmi_out(DSCD_INTS_DSCD_reg, 0xFFFFFFFF);    //clear all status before dscd run
                hdmi_out(DSCD_INTS_DSCD_2_reg, 0xFFFFFFFF);     // only support for merlin6

                lib_hdmi_dsc_set_output_hporch(dsc_output_hporch>>2); // 4pixel mode, need to div4
                HDMI_DSC_INFO("DSCD FSM:%s, dscd_port=%d, Check PPS Pass!!start to run dsc!!\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port);
                SET_DSCD_FSM(DSCD_FSM_RUN);

            }
            else
            {
                HDMI_DSC_INFO("DSCD FSM:%s, Invalid dsc_output_hporch, h_total=%d, h_act_len=%d, pps_crc=0x%x\n",  _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.pps_timing.h_total, dscd_fsm_st.pps_timing.h_act_len, lib_hdmi_dsc_get_dscd_pps_crc());
                dscd_fsm_st.wait_pps_cnt = 0;
                SET_DSCD_FSM(DSCD_FSM_WAIT_PPS_READY);
            }
        }
    }
        break;

    case DSCD_FSM_RUN:
    {
        unsigned int current_pps_crc = 0;
        current_pps_crc =  lib_hdmi_dsc_get_dscd_pps_crc();

        //condition checking
        if(dscd_fsm_st.run_dscd_port != port)
        {
            HDMI_DSC_INFO("DSCD FSM:%s, HDMI Port Change!! from %d to %d\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, port);
            SET_DSCD_FSM(DSCD_FSM_INIT);
        }
        else if (newbase_hdmi_get_hpd(port)==0 || newbase_hdmi_get_5v_state(port)==0)
        {
            HDMI_DSC_INFO("DSCD FSM:%s, 5V/HPD Change!! HPD:%d, 5V:%d\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), newbase_hdmi_get_hpd(port), newbase_hdmi_get_5v_state(port));
            SET_DSCD_FSM(DSCD_FSM_WAIT_LT_PASS);
        }
        else if(current_pps_crc != dscd_fsm_st.last_pps_crc)
        {
            HDMI_DSC_INFO("DSCD FSM:%s,  dscd_port=%d, PPS Change!! current_pps_crc =%8x,  original_crc=%8x\n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, current_pps_crc, dscd_fsm_st.last_pps_crc);
            dscd_fsm_st.last_pps_crc = current_pps_crc;
            SET_DSCD_FSM(DSCD_FSM_INIT);
        }
    }
        break;

    default:
        break;
    }

    if(dscd_fsm_st.pre_dscd_fsm != dscd_fsm_st.current_dscd_fsm)
    {// force print when fsm change
        dscd_fsm_st.pre_dscd_fsm = dscd_fsm_st.current_dscd_fsm;
        hdmi_dscd_print_cycle_cnt = g_dscd_print_cycle_cnt_thd;
    }

    if ((GET_DSCD_FSM() >DSCD_FSM_INIT) && (hdmi_dscd_print_cycle_cnt++ >= g_dscd_print_cycle_cnt_thd))
    {
        unsigned out_crc = lib_hdmi_dsc_get_dscd_out_crc();
        unsigned in_crc = lib_hdmi_dsc_get_dscd_in_crc();
        unsigned pps_crc = lib_hdmi_dsc_get_dscd_pps_crc();

        DSCD_FLOW_PRINTF("DSCD FSM:%s,(Current,DSC) port=(%d,%d), dscd_en=%d, (out, in,pps)=(%08x,%08x,%08x), PPS hactive:%d, vactive:%d, output_vback:%d, INTS=%x, INTS_2=%x\n",
            _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()),
            port, dscd_fsm_st.run_dscd_port,
            lib_hdmi_get_dsc_enable(),
            out_crc, in_crc, pps_crc,
            dscd_fsm_st.pps_timing.h_act_len,
            dscd_fsm_st.pps_timing.v_act_len,
            newbase_hdmi_dscd_current_output_vbackporch(),
            hdmi_in(DSCD_INTS_DSCD_reg), 
            hdmi_in(DSCD_INTS_DSCD_2_reg));

        hdmi_dscd_print_cycle_cnt = 0;
    }
}

void newbase_hdmi_hd21_dsc_enanble(unsigned char port, unsigned char enable)
{
    unsigned char nport = port;
    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[newbase_hdmi_hd21_dsc_enanble] DSC CRT not ready!!\n");
        return;
    }

    if (enable)
    {
        hdmi_mask(HD21_CR_reg, ~HD21_CR_dsc_mode_fw_mask, HD21_CR_dsc_mode_fw_mask);           // enable DSC output
        hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~HDMI_HDMI_VPLLCR1_dpll_freeze_dsc_mask, 0);         // defreeze dsc
        lib_hdmi_dsc_enable(1);                    // enable DSC
        udelay(100);
    }
    else
    {
        lib_hdmi_dsc_enable(0);                     // disable DSC
        hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~HDMI_HDMI_VPLLCR1_dpll_freeze_dsc_mask, HDMI_HDMI_VPLLCR1_dpll_freeze_dsc_mask);   // freeze dsc
        hdmi_mask(HD21_CR_reg, ~HD21_CR_dsc_mode_fw_mask, 0x0);           // disable DSC output
    }

    HDMI_WARN("[newbase_hdmi_hd21_dsc_enanble] HDMI2.1 port =%d, enable=%d\n", port, enable);
}

unsigned int newbase_hdmi_dscd_current_output_vbackporch(void)
{
    unsigned int tm_result_clk_cycle = 0;
    unsigned int dscd_output_v_back_porch = 0;
    unsigned int dscd_in_htotal = dscd_fsm_st.pps_timing.h_total;
    if(dscd_in_htotal ==0)
    {
        HDMI_WARN("[newbase_hdmi_dscd_tm_vbackporch] DSCD PPS not ready, can not read vbackporch\n");
        return 0;
    }
    if(!lib_hdmi_is_hdmi_21_available(dscd_fsm_st.run_dscd_port))
    {
        HDMI_WARN("[newbase_hdmi_dscd_tm_vbackporch] DSCD port is invalid, dscd port=%d\n", dscd_fsm_st.run_dscd_port);
        return 0;
    }

    tm_result_clk_cycle = lib_hdmi_dsc_get_timing_monitor_result(DSCD_TM_RESULT_1);

    if(lib_hdmi_hd21_is_clk_div_en(dscd_fsm_st.run_dscd_port))
    {
        dscd_output_v_back_porch = (tm_result_clk_cycle/(dscd_in_htotal/4)); // htotal/4
    }
    else
    {
        dscd_output_v_back_porch = (tm_result_clk_cycle/(dscd_in_htotal/2)); // htotal/4*2
    }
    HDMI_INFO("[newbase_hdmi_dscd_tm_vbackporch ] DSCD dscd_output_v_back_porch=%d, dscd_in_htotal=%d, raw_cycle=%d\n", dscd_output_v_back_porch, dscd_in_htotal, tm_result_clk_cycle);

    return dscd_output_v_back_porch;
}


/*------------------------------------------------------------------
 * Func : newbase_hdmi_check_dscd_vaild_crc_check
 *
 * Desc : check DSCD output CRC. When DSCD output invalid, crc has special regular symptom. Ex: 0x0c0c0c0c, it is invalid output.  
 *           Maybe HDCP2.2 auth is not success. So, DSCD can not decode.
 * Para : [IN] crc  : CRC value for checking 
 *
 * Retn :  TRUE: check pass, FALSE: check fail
 *------------------------------------------------------------------*/
unsigned char newbase_hdmi_check_dscd_vaild_crc_check(unsigned int crc)
{//Check CRC for debugging
    unsigned int crc_bit0_3 = crc&0xF;
    unsigned int crc_bit4_7 = (crc&0xF0) >>4;
    unsigned int crc_bit8_11 = (crc&0xF00) >>8;
    unsigned int crc_bit12_15 = (crc&0xF000) >>12;
    unsigned int crc_bit16_19 = (crc&0xF0000) >>16;
    unsigned int crc_bit20_23 = (crc&0xF00000) >>20;
    unsigned int crc_bit24_27 = (crc&0xF000000) >>24;
    unsigned int crc_bit28_31 = (crc&0xF0000000) >>28;

    if(GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_CRC_VAILD_EN) ==FALSE)
        return TRUE; //If CHECK_CRC_VAILD_EN == FALSE, Bypase check, force return TRUE.

    if((crc >0)
        && ((crc_bit0_3==crc_bit8_11) && (crc_bit8_11==crc_bit16_19)&&(crc_bit16_19==crc_bit24_27))
        && ((crc_bit4_7==crc_bit12_15) && (crc_bit12_15==crc_bit20_23)&&(crc_bit20_23==crc_bit28_31)))
    {
        //HDMI_WARN("DSCD INVALID CRC=0x%x, crc_bit0_3=%x, crc_bit4_7=%x, \n", crc, crc_bit0_3, crc_bit4_7); //for detail debugging
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


unsigned char newbase_hdmi_check_dscd_output_valid(void)
{
    unsigned char result = FALSE;
    unsigned char wait_count = GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_CNT_THD);

    if(!lib_hdmi_dsc_get_crt_on())
    {
        HDMI_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD CRT not enable!! DSCD can not work!\n");
        return FALSE;
    }
    if(GET_FLOW_CFG(HDMI_FLOW_CFG_DSC, HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_EN) ==FALSE)
        return TRUE; //If DSCD_CHECK_OUTPUT_EN == FALSE, Bypase check, force return TRUE.

    if(GET_DSCD_FSM() <DSCD_FSM_PPS_MEASURE)
    {
        HDMI_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD FSM:%s!! DSCD output not ready \n", _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()));
        return FALSE;
    }

    lib_hdmi_dsc_clr_output_error_status();
    lib_hdmi_dsc_clr_dscd_out_pic_finish();    //clear dscd output status

    do
    {
        unsigned int out_pic_finish = lib_hdmi_dsc_get_dscd_out_pic_finish();    // only support for merlin6
        unsigned int out_crc = lib_hdmi_dsc_get_dscd_out_crc();
        unsigned int in_crc = lib_hdmi_dsc_get_dscd_in_crc();
        unsigned int pps_crc = lib_hdmi_dsc_get_dscd_pps_crc();
        unsigned char buf[128];
        unsigned char check_output_crc_vaild = newbase_hdmi_check_dscd_vaild_crc_check(pps_crc);
        unsigned char is_dscd_no_error = !lib_hdmi_dsc_get_output_error_status();
        lib_hdmi_dsc_get_cur_pps(buf);

        if(!lib_hdmi_dsc_get_crt_on())
        {
            HDMI_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD CRT not enable!! DSCD can not work! round=%d\n", wait_count);
            return FALSE;
        }
        HDMI_WARN("DSCD FSM:%s, dscd_port=%d, count=%d, dscd_en=%d, Check DSCD (gen_finish, crc_vaild, is_no_err)=(%d,%d,%d),(out, in,pps)=(%08x,%08x,%08x),INTS=%x, INTS_2=%x\n",
            _hdmi_hd21_dsc_fsm_str(GET_DSCD_FSM()), dscd_fsm_st.run_dscd_port, 
           wait_count,  lib_hdmi_get_dsc_enable(), out_pic_finish, check_output_crc_vaild, is_dscd_no_error,
            out_crc, in_crc,  pps_crc, hdmi_in(DSCD_INTS_DSCD_reg), 
            hdmi_in(DSCD_INTS_DSCD_2_reg));

        if((out_pic_finish ==1) && ( check_output_crc_vaild==TRUE)&&(is_dscd_no_error == TRUE))
        {
            result = TRUE;
            break;
        }
        else if((out_pic_finish ==1) && (is_dscd_no_error == FALSE))
        {// DSCD error, check next frame.
            HDMI_EMG("[newbase_hdmi_check_dscd_output_valid] round:%d, DSCD output decode not ready, check next round!!!\n", wait_count);
            lib_hdmi_dsc_clr_output_error_status(); //clear error status and check again
        }

        if(wait_count==1)
        {
            result = FALSE;
            HDMI_EMG("[newbase_hdmi_check_dscd_output_valid] DSCD output vaild check timeout!!\n");
        }

        msleep(40); //wait next v sync

        wait_count --;
    }
    while(wait_count>0 && lib_hdmi_dsc_get_crt_on());

    return result;
}

