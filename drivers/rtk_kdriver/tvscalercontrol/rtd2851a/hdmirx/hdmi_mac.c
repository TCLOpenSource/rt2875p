/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file:    hdmi_mac.c
 *
 * author:  kevin_wang@realtek.com
 * date:
 * version:     3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include <rbus/stb_reg.h>
#include <rbus/dma_vgip_reg.h>
#include <rbus/vgip_reg.h>
#include "hdmi_hdcp.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include <mach/rtk_platform.h>
#include <mach/platform.h>
#include <rbus/sys_reg_reg.h>


#define TMDS_WAIT_INTERLACE_V_SYNC_CNT    2    //Interlace flag need to wait V sync


#define HDMI_HDMI_debug_port_get_vs(x)      ((x >>3) & 0x1) // only work on hdmi debug mode 5
#define HDMI_HDMI_debug_port_get_bch_err_pkt(x)      ((x >>1) & 0xF) // only work on hdmi debug mode 4


extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
extern spinlock_t hdmi_spin_lock;

const VIDEO_DPLL_RATIO_T dpll_ratio[] =
{
    {   15, 15,     1, 1    },  // 24 bits
    {   12, 15,     4, 5    },  // 30 bits
    {   10, 15,     2, 3    },  // 36 bits
    {   15, 30,     1, 2    },  // 48 bits
};

HDMI_CONST VIDEO_DPLL_FIX_T video_pll_main_param[]=
{
    {30 ,0,0b010,0b011},
    {62,3,0b011,0b101},
    {30,1,0b010,0b011},
    {6,0,0b000,0b100},
    {14,3,0b001,0b011},
    {6,1,0b000,0b100},
    {6,2,0b000,0b100},
    {6,3,0b000,0b100},
    {6,4,0b000,0b100},
    {6,6,0b000,0b100},
    {2,3,0b000,0b000},
    {2,4,0b000,0b000},

};

HDMI_CONST VIDEO_DPLL_RANGE_T video_pll_others_param[]=
{
    {6 ,10,0b000,0b100},
    {11,18,0b001,0b011},
    {19,30,0b010,0b011},
    {31,48,0b011,0b100},
    {49,75,0b101,0b100}
};

HDMI_CONST VIDEO_DPLL_FIX_T video_pll_frl_param[] =
{
    {65 ,1,0b100,0b110},
    {66 ,1,0b100,0b110},
    {32 ,1,0b100,0b110},
};

//enum PLAFTORM_TYPE g_platform = PLATFORM_KXLP;


//------------------------------------------------------------------
// CRT
//------------------------------------------------------------------

void lib_hdmi_mac_crt_perport_off(unsigned char nport)
{
    unsigned int clken_pre_mask = HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask << nport;
    unsigned int clken_hdcp_pre_mask = HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0_mask << nport;
    unsigned int rstn_pre_mask = HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask << nport;
    unsigned int rstn_hdcp_pre_addr = HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask << nport;

    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~(clken_pre_mask|clken_hdcp_pre_mask), 0x0);
    udelay(5);
    hdmi_mask(HDMI_P0_hdmi_rst0_reg,~(rstn_pre_mask|rstn_hdcp_pre_addr), 0x0);
}

void lib_hdmi_mac_crt_perport_on(unsigned char nport)
{
    // reset HDMI preport HW : including HDCP/SCDC.....
    // this function should be called to reset HDMI mac, every time cable off/on

    unsigned int clken_pre_mask = HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask << nport;
    unsigned int clken_hdcp_pre_mask = HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0_mask << nport;
    unsigned int rstn_pre_mask = HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask << nport;
    unsigned int rstn_hdcp_pre_addr = HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask << nport;

    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~(clken_pre_mask|clken_hdcp_pre_mask), 0);    // Clock off
    udelay(5);

    hdmi_mask(HDMI_P0_hdmi_rst0_reg,~(rstn_pre_mask|rstn_hdcp_pre_addr), 0);    // Rst Low
    udelay(5);

    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~(clken_pre_mask|clken_hdcp_pre_mask), (clken_pre_mask|clken_hdcp_pre_mask));  // Clock on
    udelay(5);

    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~(clken_pre_mask|clken_hdcp_pre_mask), 0);    // Clock off
    udelay(5);

    hdmi_mask(HDMI_P0_hdmi_rst0_reg,~(rstn_pre_mask|rstn_hdcp_pre_addr), (rstn_pre_mask|rstn_hdcp_pre_addr)); // Rst = 1 (release)
    udelay(5);

    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~(clken_pre_mask|clken_hdcp_pre_mask), (clken_pre_mask|clken_hdcp_pre_mask));  // Clock on
    udelay(5);
    
    lib_hdmi_hdcp22_reset_aes_engine(nport);   // Mac7p/8p/...series have to run this .Reset AES engine (After reset HDCP, we need to reset AES engine as well)
}

void lib_hdmi_mac_crt_perport_reset(unsigned char nport)
{
    HDMI_INFO("lib_hdmi_mac_crt_perport_reset(%d)\n", nport);
    lib_hdmi_mac_crt_perport_off(nport);
    udelay(5);
    lib_hdmi_mac_crt_perport_on(nport);
}

void lib_hdmi_sys_crt_on(void)
{
    HDMI_INFO("[CRT] lib_hdmi_sys_crt_on \n");

    if (!((hdmi_in(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_hdmi_mask) &&
          (hdmi_in(SYS_REG_SYS_SRST1_reg) & SYS_REG_SYS_SRST1_rstn_hdmi_mask)))
    {
        hdmi_out(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_hdmi_mask);
        udelay(5);
        hdmi_out(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_hdmi_mask);

        udelay(5);

        hdmi_out(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_hdmi_mask|SYS_REG_SYS_CLKEN1_write_data_mask));
        udelay(5);

        hdmi_out(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_hdmi_mask);
        udelay(5);

        hdmi_out(SYS_REG_SYS_SRST1_reg, (SYS_REG_SYS_SRST1_rstn_hdmi_mask|SYS_REG_SYS_SRST1_write_data_mask));
        udelay(5);
        hdmi_out(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_hdmi_mask|SYS_REG_SYS_CLKEN1_write_data_mask));
    }

    // reset DDC
    //hdmi_out(SYS_REG_SYS_SRST3_reg, SYS_REG_SYS_SRST3_write_data_mask|SYS_REG_SYS_SRST3_rstn_ddc_mask);
    //hdmi_out(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_write_data_mask|SYS_REG_SYS_CLKEN3_clken_ddc_mask);

    // reset for DDC
    hdmi_out(STB_ST_SRST1_reg, (STB_ST_SRST1_write_data_mask|STB_ST_SRST1_rstn_ddc_mask|STB_ST_SRST1_rstn_osc_mask));
    hdmi_out(STB_ST_CLKEN1_reg, (STB_ST_CLKEN1_write_data_mask|STB_ST_CLKEN1_clken_ddc_mask|STB_ST_CLKEN1_clken_osc_mask));

    // reset for OFMS
    hdmi_out(SYS_REG_SYS_SRST1_reg, (SYS_REG_SYS_SRST1_write_data_mask|SYS_REG_SYS_SRST1_rstn_offms_mask));
    hdmi_out(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_write_data_mask|SYS_REG_SYS_CLKEN1_clken_offms_mask));

    hdmi_out(SYS_REG_SYS_SRST2_reg, (SYS_REG_SYS_SRST2_rstn_dispm_cap_mask|SYS_REG_SYS_SRST2_rstn_dispm_disp_mask|SYS_REG_SYS_SRST2_write_data_mask));
    hdmi_out(SYS_REG_SYS_CLKEN2_reg, (SYS_REG_SYS_CLKEN2_clken_dispm_cap_mask|SYS_REG_SYS_CLKEN2_clken_dispm_disp_mask|SYS_REG_SYS_CLKEN2_write_data_mask));

    // Enable HDMI SCPU IRQ
    hdmi_out(SYS_REG_INT_CTRL_MEMC_reg, (SYS_REG_INT_CTRL_MEMC_hdmi_irq_scpu_routing_en_mask|SYS_REG_INT_CTRL_MEMC_write_data_mask));
}

void lib_hdmi_crt_off(unsigned char nport)
{
    unsigned int val = 0;

    switch (nport)
    {
        case 0:
            val = HDMI_STB_clken_hdmi_stb_port0_mask;
            break;
        case 1:
            val = HDMI_STB_clken_hdmi_stb_port1_mask;
            break;
        case 2:
            val = HDMI_STB_clken_hdmi_stb_port2_mask;
            break;
        case 3:
            val = HDMI_STB_clken_hdmi_stb_port3_mask;
            break;
        default:
            return;
    }
    // Disable for HDMI STB MAC
    hdmi_mask(HDMI_STB_clken_hdmi_stb_reg, ~val, 0x0);
    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_STB_rst_n_hdmi_stb_port0_mask;
            break;
        case 1:
            val = HDMI_STB_rst_n_hdmi_stb_port1_mask;
            break;
        case 2:
            val = HDMI_STB_rst_n_hdmi_stb_port2_mask;
            break;
        case 3:
            val = HDMI_STB_rst_n_hdmi_stb_port3_mask;
            break;
        default:
            return;
    }
    hdmi_mask(HDMI_STB_rst_n_hdmi_stb_reg, ~val, 0x0);
    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0_mask;
            break;
        case 1:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre1_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre1_mask;
            break;
        case 2:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre2_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre2_mask;
            break;
        case 3:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre3_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre3_mask;
            break;
        default:
            return;
    }
    // Disable MAC crt (always control on P0)
    hdmi_mask(HDMI_P0_hdmi_clken0_reg, ~val, 0x0);
    if (!((hdmi_in(HDMI_P0_hdmi_clken0_reg) & \
        (HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask | HDMI_P0_hdmi_clken0_clken_hdmi_pre1_mask \
        | HDMI_P0_hdmi_clken0_clken_hdmi_pre2_mask | HDMI_P0_hdmi_clken0_clken_hdmi_pre3_mask))))
        hdmi_mask(HDMI_P0_hdmi_clken0_reg,~HDMI_hdmi_clken0_clken_hdmi_common_mask,0x0);

    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask;
            break;
        case 1:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre1_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre1_mask;
            break;
        case 2:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre2_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre2_mask;
            break;
        case 3:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre3_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre3_mask;
            break;
        default:
            return;
    }
    hdmi_mask(HDMI_P0_hdmi_rst0_reg, ~val, 0x0);

    if (!((hdmi_in(HDMI_P0_hdmi_rst0_reg) & \
        (HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask | HDMI_P0_hdmi_rst0_rstn_hdmi_pre1_mask \
        | HDMI_P0_hdmi_rst0_rstn_hdmi_pre2_mask | HDMI_P0_hdmi_rst0_rstn_hdmi_pre3_mask))))
        hdmi_mask(HDMI_P0_hdmi_rst0_reg,~HDMI_hdmi_rst0_rstn_hdmi_common_mask,0x0);

    udelay(5);
}

void lib_hdmi_crt_on(unsigned char nport)
{
    unsigned int val = 0;

    switch (nport)
    {
        case 0:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre0_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre0_mask;
            break;
        case 1:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre1_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre1_mask;
            break;
        case 2:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre2_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre2_mask;
            break;
        case 3:
            val = HDMI_P0_hdmi_rst0_rstn_hdmi_pre3_mask|HDMI_P0_hdmi_rst0_rstn_hdmi_hdcp_pre3_mask;
            break;
        default:
            return;
    }
    val |= HDMI_hdmi_rst0_rstn_hdmi_common_mask;
    hdmi_mask(HDMI_P0_hdmi_rst0_reg, ~val, val);
    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre0_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre0_mask;
            break;
        case 1:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre1_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre1_mask;
            break;
        case 2:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre2_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre2_mask;
            break;
        case 3:
            val = HDMI_P0_hdmi_clken0_clken_hdmi_pre3_mask|HDMI_P0_hdmi_clken0_clken_hdmi_hdcp_pre3_mask;
            break;
        default:
            return;
    }
    val |= HDMI_hdmi_clken0_clken_hdmi_common_mask;
    hdmi_mask(HDMI_P0_hdmi_clken0_reg, ~val,val);

    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_STB_rst_n_hdmi_stb_port0_mask;
            break;
        case 1:
            val = HDMI_STB_rst_n_hdmi_stb_port1_mask;
            break;
        case 2:
            val = HDMI_STB_rst_n_hdmi_stb_port2_mask;
            break;
        case 3:
            val = HDMI_STB_rst_n_hdmi_stb_port3_mask;
            break;
        default:
            return;
    }

    // reset for HDMI STB MAC
    hdmi_mask(HDMI_STB_rst_n_hdmi_stb_reg, ~val, val);
    udelay(5);

    switch (nport)
    {
        case 0:
            val = HDMI_STB_clken_hdmi_stb_port0_mask;
            break;
        case 1:
            val = HDMI_STB_clken_hdmi_stb_port1_mask;
            break;
        case 2:
            val = HDMI_STB_clken_hdmi_stb_port2_mask;
            break;
        case 3:
            val = HDMI_STB_clken_hdmi_stb_port3_mask;
            break;
        default:
            return;
    }

    hdmi_mask(HDMI_STB_clken_hdmi_stb_reg, ~val, val);
}

void lib_hdmi_crt_reset(unsigned char nport)
{//crt reset all block about HDMI
    HDMI_EMG("[CRT] lib_hdmi_crt_reset\n");
    lib_hdmi_crt_off(nport);
    lib_hdmi_crt_on(nport);
}    


void lib_hdmi_crt_reset_hdmi_common(void)
{
    // reset HDMI2.0 MAC common : including DEEP Color, YUV420, CRC, MD_Detect...
    HDMI_INFO("[CRT] lib_hdmi_crt_reset_hdmi_common\n");

    // Clock = 0
    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~HDMI_hdmi_clken0_clken_hdmi_common_mask, 0x0);
    udelay(5);

    // RST = 0
    hdmi_mask(HDMI_P0_hdmi_rst0_reg,~HDMI_hdmi_rst0_rstn_hdmi_common_mask, 0x0);
    udelay(5);

    // Clock = 1 : start clock to make sure HW can detect reset=Low
    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~HDMI_hdmi_clken0_clken_hdmi_common_mask, HDMI_hdmi_clken0_clken_hdmi_common_mask);
    udelay(5);

    // Clock = 0 : stop clock (need to stop clock before release reset)
    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~HDMI_hdmi_clken0_clken_hdmi_common_mask, 0x0);
    udelay(5);

    // RST = 1   : release reset
    hdmi_mask(HDMI_P0_hdmi_rst0_reg,~HDMI_hdmi_rst0_rstn_hdmi_common_mask, HDMI_hdmi_rst0_rstn_hdmi_common_mask);
    udelay(5);

    // Clock = 1 : enable clock
    hdmi_mask(HDMI_P0_hdmi_clken0_reg,~HDMI_hdmi_clken0_clken_hdmi_common_mask, HDMI_hdmi_clken0_clken_hdmi_common_mask);
    udelay(5);
}


//------------------------------------------------------------------
// Per Port CRC
//------------------------------------------------------------------

void lib_hdmi_crc_check(unsigned char nport)
{
    hdmi_out(HDMI_PP_TMDS_CRCC_reg,
        HDMI_PP_TMDS_CRCC_crc_r_en_mask|
        HDMI_PP_TMDS_CRCC_crc_g_en_mask|
        HDMI_PP_TMDS_CRCC_crc_b_en_mask|
        HDMI_PP_TMDS_CRCC_crc_nonstable_mask|
        HDMI_PP_TMDS_CRCC_crcc_mask);
}

unsigned int lib_hdmi_read_crc(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return 0;  // Do Nothing : HDMI 2.1 MAC has no CRC function

    return hdmi_in(HDMI_PP_TMDS_CRCO0_reg);
}

void lib_hdmi_read_crc_long(unsigned char nport, unsigned int*  mac_crc0, unsigned int* mac_crc1)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        return;  // Do Nothing : HDMI 2.1 MAC has no CRC function
    }
    else
    {// 24/24 bit
        *mac_crc0= hdmi_in(HDMI_PP_TMDS_CRCO0_reg);
        *mac_crc1= hdmi_in(HDMI_PP_TMDS_CRCO1_reg);
    }
}
//------------------------------------------------------------------
// TMDS
//------------------------------------------------------------------

void lib_hdmi_tmds_init(unsigned char nport)
{
    lib_hdmi_mac_select_port(nport);

    lib_hdmi_mac_hd20_tmds_input_eanbale(nport, 1);

#ifdef HDMI_FIX_HDMI_POWER_SAVING
    hdmi_mask(HDMI_TMDS_PWDCTL_reg, ~HDMI_TMDS_PWDCTL_iccaf_mask, 0);  // Input Channel control by auto function
#endif
}

void lib_hdmi_tmds_cps_pll_div2_en(unsigned char nport, unsigned char en)
{
    HDMI_INFO("[lib_hdmi_tmds_cps_pll_div2_en port:%d] en=%d\n", nport, en);
    hdmi_mask(HDMI_TMDS_CPS_reg, ~HDMI_TMDS_CPS_pll_div2_en_mask, HDMI_TMDS_CPS_pll_div2_en(en?1:0));
}

unsigned char lib_hdmi_is_rgb_status_ready(unsigned char nport)
{
    return ((hdmi_in(HDMI_TMDS_CTRL_reg) & (HDMI_TMDS_CTRL_bcd_mask|HDMI_TMDS_CTRL_gcd_mask|HDMI_TMDS_CTRL_rcd_mask))
        == (HDMI_TMDS_CTRL_bcd_mask|HDMI_TMDS_CTRL_gcd_mask|HDMI_TMDS_CTRL_rcd_mask));
}

void lib_hdmi_clear_rgb_hv_status(unsigned char nport)
{
    hdmi_out(HDMI_TMDS_CTRL_reg, (HDMI_TMDS_CTRL_bcd_mask|HDMI_TMDS_CTRL_gcd_mask|HDMI_TMDS_CTRL_rcd_mask|HDMI_TMDS_CTRL_ho_mask|HDMI_TMDS_CTRL_yo_mask));
}

unsigned int lib_hdmi_read_rgb_hv_status(unsigned char nport)
{
	return hdmi_in(HDMI_TMDS_CTRL_reg);
}

unsigned char lib_hdmi_is_vsync_detected(unsigned char nport)
{
    return HDMI_TMDS_CTRL_get_yo(hdmi_in(HDMI_TMDS_CTRL_reg));
}

void lib_hdmi_clear_vsync_detect(unsigned char nport)
{
    hdmi_mask(HDMI_TMDS_CTRL_reg, ~HDMI_TMDS_CTRL_yo_mask, HDMI_TMDS_CTRL_yo_mask); //write 1 clear
}

unsigned char lib_hdmi_vsync_checked(unsigned char nport)
{
    unsigned char i;

    lib_hdmi_clear_vsync_detect(nport);
    for (i=0; i<10; i++) {
        if (lib_hdmi_is_vsync_detected(nport)) {
            return TRUE;
        }
        msleep(10);
    }

    //hdmi_out(HDMI_TMDS_CTRL_reg, HDMI_TMDS_CTRL_yo_mask);
    lib_hdmi_clear_vsync_detect(nport);
    HDMI_PRINTF("[lib_hdmi_vsync_checked port:%d][WARNING] Vsync cannot seen?\n", nport);
    return FALSE;
}

void lib_hdmi_tmds_out_ctrl(unsigned char nport, unsigned char bypass, unsigned int r, unsigned int g, unsigned int b)
{
    hdmi_mask(HDMI_TMDS_OUT_CTRL_reg, ~(HDMI_TMDS_OUT_CTRL_tmds_bypass_mask), HDMI_TMDS_OUT_CTRL_tmds_bypass(bypass));
    hdmi_out(HDMI_TMDS_ROUT_reg, r);
    hdmi_out(HDMI_TMDS_GOUT_reg, g);
    hdmi_out(HDMI_TMDS_BOUT_reg, b);
}

unsigned char lib_hdmi_tmds_get_br_swap(unsigned char nport)
{
    return HDMI_TMDS_PWDCTL_get_brcw(hdmi_in(HDMI_TMDS_PWDCTL_reg));
}

void lib_hdmi_power(unsigned char nport, unsigned char enable)
{
    HDMI_WARN("[lib_hdmi_power nport:%d] enable=%d\n", nport, enable);
    if (enable) {
        hdmi_mask(HDMI_TMDS_OUTCTL_reg, ~(HDMI_TMDS_OUTCTL_tbcoe_mask|HDMI_TMDS_OUTCTL_tgcoe_mask|HDMI_TMDS_OUTCTL_trcoe_mask|HDMI_TMDS_OUTCTL_ocke_mask),
            (HDMI_TMDS_OUTCTL_tbcoe(1)|HDMI_TMDS_OUTCTL_tgcoe(1)|HDMI_TMDS_OUTCTL_trcoe(1)|HDMI_TMDS_OUTCTL_ocke(1)));      //enable PLL TMDS, RGB clock output

#ifdef HDMI_FIX_HDMI_POWER_SAVING
        hdmi_mask(HDMI_TMDS_OUTCTL_reg, ~HDMI_TMDS_OUTCTL_aoe_mask, HDMI_TMDS_OUTCTL_aoe_mask);  // enable auto power saving
#endif
        lib_hdmi_z0_set(nport, LN_ALL, 1);
    } else {

        hdmi_mask(HDMI_TMDS_OUTCTL_reg,~(HDMI_TMDS_OUTCTL_tbcoe_mask | HDMI_TMDS_OUTCTL_tgcoe_mask | HDMI_TMDS_OUTCTL_trcoe_mask | HDMI_TMDS_OUTCTL_ocke_mask)
                      ,(HDMI_TMDS_OUTCTL_tbcoe(0) | HDMI_TMDS_OUTCTL_tgcoe(0) | HDMI_TMDS_OUTCTL_trcoe(0) | HDMI_TMDS_OUTCTL_ocke(0)));     //disable PLL TMDS, RGB clock output

#ifdef HDMI_FIX_HDMI_POWER_SAVING
        hdmi_mask(HDMI_TMDS_OUTCTL_reg, ~HDMI_TMDS_OUTCTL_aoe_mask, 0);  // disable auto output enable
#endif

        lib_hdmi_z0_set(nport, LN_ALL, 0);
    }
}

void lib_hdmi_valid_format_condition(unsigned char nport)
{
    hdmi_mask(HDMI_TMDS_CPS_reg,
        ~(HDMI_TMDS_CPS_pk_gb_num_mask | HDMI_TMDS_CPS_vd_gb_num_mask
        | HDMI_TMDS_CPS_pk_pre_num_mask | HDMI_TMDS_CPS_vd_pre_num_mask)
        , (HDMI_TMDS_CPS_pk_gb_num(HDMI_PK_GB_NUM) | HDMI_TMDS_CPS_vd_gb_num(HDMI_VD_GB_NUM)
        | HDMI_TMDS_CPS_pk_pre_num(HDMI_PK_PRE_NUM) | HDMI_TMDS_CPS_vd_pre_num(HDMI_VD_PRE_NUM)));
}


/*---------------------------------------------------
 * Func : lib_hdmi_clear_all_infoframe
 *
 * Desc : clear info frame SRAM. all data in the info
 *        frame SRAM buffer will be cleared.
 *
 * Para : nport : HDMI port number
 *
 * Retn : 0 : not reveived, 1 : received
 *--------------------------------------------------*/
void lib_hdmi_clear_all_infoframe(unsigned char nport)
{
    hdmi_mask(HDMI_TMDS_CPS_reg,~HDMI_TMDS_CPS_clr_infoframe_dvi_mask,HDMI_TMDS_CPS_clr_infoframe_dvi_mask);

    hdmi_mask(HDMI_TMDS_CPS_reg,~HDMI_TMDS_CPS_clr_infoframe_dvi_mask,0x0);
}


//------------------------------------------------------------------
// Video Data Error Detection
//------------------------------------------------------------------
void lib_hdmi_bit_err_rst(unsigned char nport, unsigned char rst)
{
    if (rst)
    {
        hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_en_mask, 0); // disable
        hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_reset_mask, HDMI_VIDEO_BIT_ERR_DET_reset_mask); //reset cnt
    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg, ~(HDMI_VIDEO_BIT_ERR_DET_reset_mask), 0);
    }
}

void lib_hdmi_bit_err_start(unsigned char nport, unsigned char conti_mode, unsigned char period)
{
    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_en_mask, 0); // disable
    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_reset_mask, HDMI_VIDEO_BIT_ERR_DET_reset_mask); //reset cnt

    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_mode_mask, HDMI_VIDEO_BIT_ERR_DET_mode(conti_mode));
    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_period_mask, HDMI_VIDEO_BIT_ERR_DET_period(period));
    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_thd_mask, HDMI_VIDEO_BIT_ERR_DET_thd(1)); //thd

    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_reset_mask, 0);

    hdmi_mask(HDMI_VIDEO_BIT_ERR_STATUS_B_reg, 	//wclr_out
        ~(HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask),
        (HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask));

    hdmi_mask(HDMI_VIDEO_BIT_ERR_STATUS_G_reg, 	//wclr_out
        ~(HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask),
        (HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask));

    hdmi_mask(HDMI_VIDEO_BIT_ERR_STATUS_R_reg, 	//wclr_out
        ~(HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask),
        (HDMI_VIDEO_BIT_ERR_STATUS_B_no_dis_reset_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_thd_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_b_mask|HDMI_VIDEO_BIT_ERR_STATUS_B_bit_err_cnt_of_b_mask));

    hdmi_mask(HDMI_VIDEO_BIT_ERR_DET_reg, ~HDMI_VIDEO_BIT_ERR_DET_en_mask, HDMI_VIDEO_BIT_ERR_DET_en_mask);
}

unsigned char lib_hdmi_bit_err_det_done(unsigned char nport)
{
    return (!HDMI_VIDEO_BIT_ERR_DET_get_en(hdmi_in(HDMI_VIDEO_BIT_ERR_DET_reg)));
}

unsigned char lib_hdmi_bit_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{
    if (HDMI_VIDEO_BIT_ERR_DET_get_mode(hdmi_in(HDMI_VIDEO_BIT_ERR_DET_reg)))
    {  //continue mode
		// do nothing
    }
    else
    {
        if (HDMI_VIDEO_BIT_ERR_DET_get_en(hdmi_in(HDMI_VIDEO_BIT_ERR_DET_reg)))
        {
            return 0;	//not yet finish counting
        }
    }

    *r_err = HDMI_VIDEO_BIT_ERR_STATUS_R_get_bit_err_cnt_r(hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_R_reg));
    *g_err = HDMI_VIDEO_BIT_ERR_STATUS_G_get_bit_err_cnt_g(hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_G_reg));
    *b_err = HDMI_VIDEO_BIT_ERR_STATUS_B_get_bit_err_cnt_b(hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_B_reg));

    return 1;
}


//------------------------------------------------------------------
// BCH Error control
//------------------------------------------------------------------

void lib_hdmi_set_bch_eanble(unsigned char nport, unsigned char on)
{
    hdmi_mask(HDMI_HDMI_BCHCR_reg , ~(HDMI_HDMI_BCHCR_pe_mask|HDMI_HDMI_BCHCR_bche_mask), (on) ? (HDMI_HDMI_BCHCR_pe_mask|HDMI_HDMI_BCHCR_bche_mask) : 0);
}

unsigned char lib_hdmi_get_bch_1bit_error(unsigned char nport)
{
    return HDMI_HDMI_BCHCR_get_bches(hdmi_in(HDMI_HDMI_BCHCR_reg));
}

unsigned char lib_hdmi_get_bch_2bit_error(unsigned char nport)
{
    return HDMI_HDMI_BCHCR_get_bches2(hdmi_in(HDMI_HDMI_BCHCR_reg));
}

void lib_hdmi_bch_1bit_error_clr(unsigned char nport)
{
    hdmi_mask(HDMI_HDMI_BCHCR_reg , ~(HDMI_HDMI_BCHCR_bches_mask), (HDMI_HDMI_BCHCR_bches_mask));
}

void lib_hdmi_bch_2bit_error_clr(unsigned char nport)
{
    hdmi_mask(HDMI_HDMI_BCHCR_reg , ~(HDMI_HDMI_BCHCR_bches2_mask), HDMI_HDMI_BCHCR_bches2_mask);
}


void lib_hdmi_bch_error_irq_en(unsigned char nport, unsigned char enable)
{
    HDMI_INFO("lib_hdmi_bch_error_irq_en nport=%d, enable=%d, hdmi_rx[nport].hdmi_2p1_en=%d\n", nport, enable, hdmi_rx[nport].hdmi_2p1_en);
    hdmi_mask(HDMI_HDMI_BCHCR_reg , ~(HDMI_HDMI_BCHCR_bches2_irq_en_mask), HDMI_HDMI_BCHCR_bches2_irq_en((enable) ? 1:0));

}

unsigned char lib_hdmi_is_bch_error_irq_en(unsigned char nport)
{
    return HDMI_HDMI_BCHCR_get_bches2_irq_en(hdmi_in(HDMI_HDMI_BCHCR_reg));
}

//------------------------------------------------------------------
// AVMute Control
//------------------------------------------------------------------

void lib_hdmi_video_output(unsigned char nport, unsigned char on)
{
    hdmi_mask(HDMI_HDMI_AVMCR_reg, ~HDMI_HDMI_AVMCR_ve_mask, (on) ? HDMI_HDMI_AVMCR_ve_mask :0);
}

unsigned char lib_hdmi_is_video_output(unsigned char nport)
{
    return HDMI_HDMI_AVMCR_get_ve(hdmi_in(HDMI_HDMI_AVMCR_reg));
}

void lib_hdmi_set_avmute_ignore(unsigned char nport, unsigned char on)
{
    hdmi_mask(HDMI_HDMI_AVMCR_reg, ~(HDMI_HDMI_AVMCR_avmute_flag_mask), (on) ? HDMI_HDMI_AVMCR_avmute_flag_mask : 0);
}

void lib_hdmi_clear_avmute(unsigned char nport)
{
    lib_hdmi_set_avmute_ignore(nport, 1);
    lib_hdmi_set_avmute_ignore(nport, 0);
}

unsigned char lib_hdmi_get_avmute(unsigned char nport)
{
    if(g_hdmi_debug_force_avmute[nport] !=0)
    {//debug use
        return 1; //force avmute
    }

    return HDMI_HDMI_SR_get_avmute_fw(hdmi_in(HDMI_HDMI_SR_reg));
}

unsigned char lib_hdmi_set_avmute_wd_en(unsigned char nport, unsigned char en)
{
    hdmi_mask(HDMI_HDMI_WDCR_reg, ~(HDMI_HDMI_WDCR_avmute_wd_en_mask), (en) ? HDMI_HDMI_WDCR_avmute_wd_en_mask : 0);

    return HDMI_HDMI_WDCR_get_avmute_wd_en(hdmi_in(HDMI_HDMI_WDCR_reg));
}

unsigned char lib_hdmi_is_hdmi_mode(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return MODE_HDMI;   // HDMI 2.1 is always HDMI mode

    if (HDMI_HDMI_SCR_get_mode(hdmi_in(HDMI_HDMI_SCR_reg))) {   // if FW mode
        return HDMI_HDMI_SCR_get_msmode(hdmi_in(HDMI_HDMI_SCR_reg));
    }

    if (HDMI_HDMI_SR_get_mode(hdmi_in(HDMI_HDMI_SR_reg))) {   // if HW mode
        return MODE_HDMI;
    } else {
        return MODE_DVI;
    }
}

unsigned char lib_hdmi_get_hdmi_mode_reg(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
        return MODE_HDMI;   // HDMI 2.1 is always HDMI mode
    
    if (HDMI_HDMI_SR_get_mode(hdmi_in(HDMI_HDMI_SR_reg))) {
        return MODE_HDMI;
    } else {
        return MODE_DVI;
    }
}

//------------------------------------------------------------------
// Packet Variation
//------------------------------------------------------------------

void lib_hdmi_set_video_packet_variation_irq(unsigned char nport, unsigned int val)
{
    hdmi_mask(HDMI_HDMI_PAMICR_reg, ~HDMI_HDMI_PAMICR_icpvsb_mask, val);
}

//------------------------------------------------------------------
// Video Flow Control
//------------------------------------------------------------------

void lib_hdmi_clear_interlace_reg(unsigned char nport)
{
    unsigned int val;

    // reset VCR status bits
    val  = hdmi_in(HDMI_HDMI_VCR_reg) & (~(HDMI_HDMI_VCR_int_pro_chg_flag_mask|HDMI_HDMI_VCR_eot_mask|HDMI_HDMI_VCR_se_mask));
    val |= HDMI_HDMI_VCR_eot_mask;     // clear Even/odd toggle error
    val |= HDMI_HDMI_VCR_se_mask;      // clear even/odd signal error

    hdmi_out(HDMI_HDMI_VCR_reg, val);
}

unsigned char lib_hdmi_get_is_interlace_reg(unsigned char nport)
{

    if (HDMI_HDMI_VCR_get_se(hdmi_in(HDMI_HDMI_VCR_reg)))
    {
        lib_hdmi_clear_interlace_reg(nport);
        return 0;
    }
    else
    {
        return HDMI_HDMI_VCR_get_eot(hdmi_in(HDMI_HDMI_VCR_reg));
    }

}


void lib_hdmi_clr_int_pro_chg_flag(unsigned char nport)
{
    hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_int_pro_chg_flag_mask, HDMI_HDMI_VCR_int_pro_chg_flag_mask);
}

unsigned char lib_hdmi_get_int_pro_chg_flag(unsigned char nport)
{
    if (HDMI_HDMI_VCR_get_int_pro_chg_flag(hdmi_in(HDMI_HDMI_VCR_reg)))
    {
        HDMI_INFO("[lib_hdmi_get_int_pro_chg_flag port:%d] TMDS interlace change!!\n", nport);
        lib_hdmi_clr_int_pro_chg_flag(nport);
        return TRUE;
    }

    return FALSE;
}

void lib_hdmi_set_repeat_manual(unsigned char nport, unsigned char rep_value)
{
    unsigned int val;

    val = hdmi_in(HDMI_HDMI_VCR_reg) & (~(HDMI_HDMI_VCR_int_pro_chg_flag_mask|HDMI_HDMI_VCR_eot_mask|HDMI_HDMI_VCR_se_mask|HDMI_HDMI_VCR_prdsam_mask));
    hdmi_out(HDMI_HDMI_VCR_reg, val);

    val &= ~HDMI_HDMI_VCR_dsc_mask;
    hdmi_out(HDMI_HDMI_VCR_reg, val|HDMI_HDMI_VCR_dsc(rep_value));
}

void lib_hdmi_set_repeat_auto(unsigned char nport)
{
    unsigned int val;
    val = hdmi_in(HDMI_HDMI_VCR_reg) & (~(HDMI_HDMI_VCR_int_pro_chg_flag_mask|HDMI_HDMI_VCR_eot_mask|HDMI_HDMI_VCR_se_mask));
    hdmi_out(HDMI_HDMI_VCR_reg, val|HDMI_HDMI_VCR_prdsam_mask);
}

unsigned char lib_hdmi_get_pixelrepeat(unsigned char nport)
{
    if (HDMI_HDMI_VCR_get_prdsam(hdmi_in(HDMI_HDMI_VCR_reg))) {
        return HDMI_HDMI_VCR_get_dsc_r(hdmi_in(HDMI_HDMI_VCR_reg));
    } else {
        return HDMI_HDMI_VCR_get_dsc(hdmi_in(HDMI_HDMI_VCR_reg));
    }
}

void lib_hdmi_set_colorspace_manual(unsigned char nport, unsigned char color_space)
{
    unsigned int val;

    val = hdmi_in(HDMI_HDMI_VCR_reg) & (~(HDMI_HDMI_VCR_int_pro_chg_flag_mask|HDMI_HDMI_VCR_eot_mask|HDMI_HDMI_VCR_se_mask|HDMI_HDMI_VCR_csam_mask));
    hdmi_out(HDMI_HDMI_VCR_reg, val);

    msleep(20);

    val &= ~HDMI_HDMI_VCR_csc_mask;
    hdmi_out(HDMI_HDMI_VCR_reg, val|HDMI_HDMI_VCR_csc(color_space));
}

void lib_hdmi_set_colorspace_auto(unsigned char nport)
{
    unsigned int val;
    val = hdmi_in(HDMI_HDMI_VCR_reg) & (~(HDMI_HDMI_VCR_int_pro_chg_flag_mask|HDMI_HDMI_VCR_eot_mask|HDMI_HDMI_VCR_se_mask));
    hdmi_out(HDMI_HDMI_VCR_reg, val|HDMI_HDMI_VCR_csam_mask);
}

unsigned char lib_hdmi_get_colorspace(unsigned char nport)
{
    if (HDMI_HDMI_VCR_get_csam(hdmi_in(HDMI_HDMI_VCR_reg))) { //auto
        return HDMI_HDMI_VCR_get_csc_r(hdmi_in(HDMI_HDMI_VCR_reg));
    } else {
        return HDMI_HDMI_VCR_get_csc(hdmi_in(HDMI_HDMI_VCR_reg));
    }
}


//------------------------------------------------------------------
// Deep Color
//------------------------------------------------------------------

unsigned char lib_hdmi_get_color_depth(unsigned char nport)
{
    unsigned char cd;

    cd = HDMI_TMDS_DPC0_dpc_cd(hdmi_in(HDMI_TMDS_DPC0_reg));

    if (cd > 3) {
        return (cd-4);
    } else {
        return 0;
    }
}

void lib_hdmi_set_dpc_enable(unsigned char nport, unsigned char on)
{
    HDMI_PRINTF("lib_hdmi_set_dpc_enable[%d]=%d\n", nport, on);
    hdmi_mask(HDMI_TMDS_DPC_SET0_reg, ~(HDMI_TMDS_DPC_SET0_dpc_en_mask), (on) ? HDMI_TMDS_DPC_SET0_dpc_en_mask : 0);
    HDMI_INFO("lib_hdmi_set_dpc_enable, port=%d, on=%d, color depth=%d\n", nport, on, lib_hdmi_get_color_depth(nport));
}


void lib_hdmi_set_dpc_bypass_dis(unsigned char nport, unsigned char on)
{
    hdmi_mask(HDMI_TMDS_DPC_SET0_reg, ~HDMI_TMDS_DPC_SET0_dpc_bypass_dis_mask, HDMI_TMDS_DPC_SET0_dpc_bypass_dis((on) ? 1 : 0));
}

unsigned char lib_hdmi_is_dpc_default_phase(unsigned char nport)
{// GCP default phase
    return HDMI_TMDS_DPC0_get_dpc_default_ph(hdmi_in(HDMI_TMDS_DPC0_reg));

}
//------------------------------------------------------------------
// Mode Control
//------------------------------------------------------------------


void lib_hdmi2p0_scramble_enable(unsigned char nport, unsigned char on_off)
{
    if (on_off) {
        //HDMI_PRINTF("HDMI2.0 Scramble Enable\n");
        //For RGB bug
        hdmi_mask(HDMI_HDMI_SCR_reg,~(HDMI_HDMI_SCR_dvi_reset_ds_cm_en_mask),0);
                            //force scramble mode 20151014 LG QD 804 no scramble bug
        hdmi_mask(HDMI_SCR_CR_reg, ~(HDMI_SCR_CR_scr_en_fw_mask|HDMI_SCR_CR_scr_auto_mask), HDMI_SCR_CR_scr_en_fw_mask);
        //for 6G audio bug setting
        //20150819
        hdmi_mask(HDMI_HDMI_SCR_reg,~(HDMI_HDMI_SCR_mode_mask|HDMI_HDMI_SCR_msmode_mask),(HDMI_HDMI_SCR_mode_mask|HDMI_HDMI_SCR_msmode_mask));
    } else {
        //HDMI_PRINTF("HDMI2.0 Scramble disable\n");
        hdmi_mask(HDMI_HDMI_SCR_reg,~(HDMI_HDMI_SCR_dvi_reset_ds_cm_en_mask),HDMI_HDMI_SCR_dvi_reset_ds_cm_en_mask);
        //for 6G audio bug setting
          //force scramble mode 20151014 LG QD 804 no scramble bug
        hdmi_mask(HDMI_SCR_CR_reg, ~(HDMI_SCR_CR_scr_en_fw_mask|HDMI_SCR_CR_scr_auto_mask), 0);
        //for 6G audio bug setting
        //20150819
        hdmi_mask(HDMI_HDMI_SCR_reg,~(HDMI_HDMI_SCR_mode_mask|HDMI_HDMI_SCR_msmode_mask),0);
        //For 340M>TMDS
    }
}

void lib_hdmi_gcp_ackg_header_parsing_mode(unsigned char nport, unsigned char hd21)
{
    hdmi_mask(HDMI_HDMI_SCR_reg, ~HDMI_HDMI_SCR_packet_header_parsing_mode_mask,
        HDMI_HDMI_SCR_packet_header_parsing_mode(HDMI_GCP_ACKG_PACKET_HEADER_PARSER_MODE));
}

//------------------------------------------------------------------
// IRQ
//------------------------------------------------------------------

void lib_hdmi_set_interrupt_enable(unsigned char nport, unsigned int val)
{
    hdmi_out(HDMI_HDMI_INTCR_reg, val);
}

unsigned int lib_hdmi_get_interrupt_enable(unsigned char nport)
{
    return hdmi_in(HDMI_HDMI_INTCR_reg);
}

void lib_hdmi_avmute_irq_enable(unsigned char nport)
{//  bit 7 write 1.
    HDMI_INFO("lib_hdmi_avmute_irq_enable nport=%d, hdmi_rx[nport].hdmi_2p1_en=%d\n", nport, hdmi_rx[nport].hdmi_2p1_en);
    hdmi_mask(HDMI_HDMI_INTCR_reg,~(HDMI_HDMI_INTCR_avmute_mask | HDMI_HDMI_INTCR_pending_mask),HDMI_HDMI_INTCR_avmute_mask);
}

void lib_hdmi_avmute_irq_disable(unsigned char nport)
{
    //bit 9 write  1 clear and bit 7 write 0.
    HDMI_INFO("lib_hdmi_avmute_irq_disable nport=%d, hdmi_rx[nport].hdmi_2p1_en=%d\n", nport, hdmi_rx[nport].hdmi_2p1_en);
    hdmi_mask(HDMI_HDMI_INTCR_reg,~(HDMI_HDMI_INTCR_avmute_mask | HDMI_HDMI_INTCR_pending_mask), 0);
}

unsigned char lib_hdmi_is_avmute_irq_enable(unsigned char nport)
{
    return (hdmi_in(HDMI_HDMI_INTCR_reg) & HDMI_HDMI_INTCR_avmute_mask) ? 1 : 0;
}

unsigned char lib_hdmi_is_irq_pending(unsigned char nport)
{
    return HDMI_HDMI_INTCR_get_pending(hdmi_in(HDMI_HDMI_INTCR_reg)) ? 1 : 0;
}

void lib_hdmi_clr_irq_pending(unsigned char nport)
{
    hdmi_mask(HDMI_HDMI_INTCR_reg,~(HDMI_HDMI_INTCR_pending_mask),(HDMI_HDMI_INTCR_pending_mask));//write 1 clear
}

//------------------------------------------------------------------
// Video Format
//------------------------------------------------------------------

unsigned char lib_hdmi_get_video_format(unsigned char nport)
{
    return HDMI_HDMI_VIDEO_FORMAT_get_hvf(hdmi_in(HDMI_HDMI_VIDEO_FORMAT_reg));
}

unsigned char lib_hdmi_get_vsi_vic(unsigned char nport)
{
    /*
    HDMI_VIC
    0x01:4Kx2K@30Hz
    0x02:4Kx2K@25Hz
    0x03:4Kx2K@24Hz
    0x04:4Kx2K@24Hz(SMPTE)
    Others:reserved
    */
    return HDMI_HDMI_VIDEO_FORMAT_get_hdmi_vic(hdmi_in(HDMI_HDMI_VIDEO_FORMAT_reg));
}


//------------------------------------------------------------------
// 3D Format
//------------------------------------------------------------------
unsigned char lib_hdmi_get_3d_structure(unsigned char nport)
{
    return HDMI_HDMI_3D_FORMAT_get_hdmi_3d_structure(hdmi_in(HDMI_HDMI_3D_FORMAT_reg));
}

unsigned char lib_hdmi_get_3d_extdata(unsigned char nport)
{
    /*

    3D Ext Data
    Horizontal sub-sampling
    0000:Odd/Left picture, Odd/Right picture
    0001:Odd/Left picture, Even/Right picture
    0010:Even/Left picture, Odd/Right picture
    0011:Even/Left picture, Even/Right picture
    Quincunx matrix
    0100:Odd/Left picture, Odd/Right picture
    0101:Odd/Left picture, Even/Right picture
    0110:Even/Left picture, Odd/Right picture
    0111:Even/Left picture, Even/Right picture
    1000~1111:Reserved

    */
    return HDMI_HDMI_3D_FORMAT_get_hdmi_3d_ext_data(hdmi_in(HDMI_HDMI_3D_FORMAT_reg));
}


//------------------------------------------------------------------
// Async Phy FIFO control
//------------------------------------------------------------------

void lib_hdmi_mac_afifo_enable(unsigned char nport,unsigned char lane_mode)
{
    hdmi_mask(HDMI_PHY_FIFO_CR0_reg,
        ~((HDMI_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_b_flush_mask|
        HDMI_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_g_flush_mask|
        HDMI_PHY_FIFO_CR0_port0_r_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_r_flush_mask)<<(nport*7)),
        ((HDMI_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_b_flush_mask|
        HDMI_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_g_flush_mask|
        HDMI_PHY_FIFO_CR0_port0_r_afifo_en_mask|HDMI_PHY_FIFO_CR0_port0_r_flush_mask)<<(nport*7)));
}

//------------------------------------------------------------------
// CTS FIFO control
//------------------------------------------------------------------

void lib_hdmi_ofms_clock_sel(unsigned char nport, unsigned char src)
{
    if (nport >= HDMI_PORT_TOTAL_NUM)
        return;

    hdmi_mask(HDMI_HDMI_CTS_FIFO_CTL_reg, ~HDMI_HDMI_CTS_FIFO_CTL_offms_clock_sel_mask, (src) ? HDMI_HDMI_CTS_FIFO_CTL_offms_clock_sel_mask : 0);
}

void lib_hdmi_mac_out_sel(unsigned char nport, unsigned char hd21)
{
    if (nport >= HDMI_PORT_TOTAL_NUM)
        return;

    hdmi_mask(HDMI_HDMI_CTS_FIFO_CTL_reg, ~HDMI_HDMI_CTS_FIFO_CTL_mac_out_sel_mask,
                                           HDMI_HDMI_CTS_FIFO_CTL_mac_out_sel(hd21));
}

//------------------------------------------------------------------
// Async CH FIFO control
//------------------------------------------------------------------

void lib_hdmi_afifo_enable(unsigned char nport, unsigned char enable)
{
    if (enable) {
        hdmi_mask(HDMI_CH_CR_reg,~(HDMI_CH_CR_r_ch_afifo_en_mask|HDMI_CH_CR_g_ch_afifo_en_mask|HDMI_CH_CR_b_ch_afifo_en_mask),(HDMI_CH_CR_r_ch_afifo_en_mask|HDMI_CH_CR_g_ch_afifo_en_mask|HDMI_CH_CR_b_ch_afifo_en_mask));
    } else {
        hdmi_mask(HDMI_CH_CR_reg,~(HDMI_CH_CR_r_ch_afifo_en_mask|HDMI_CH_CR_g_ch_afifo_en_mask|HDMI_CH_CR_b_ch_afifo_en_mask),0);
    }
}


//------------------------------------------------------------------
// CED
//------------------------------------------------------------------


void lib_hdmi_fw_char_err_rst(unsigned char nport, unsigned char rst)
{
    if(rst)
    {
        hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_en_mask), 0);
        hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_reset_mask),HDMI_CERCR_reset_mask);

    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_reset_mask),0);
    }
}

void lib_hdmi_scdc_char_err_rst(unsigned char nport, unsigned char rst)
{
    if (rst)
    {
        hdmi_mask(HDMI_CERCR_reg,
            ~(HDMI_CERCR_valid_reset_mask|HDMI_CERCR_reset_err_det_mask|HDMI_CERCR_ch_locked_reset_mask),
            (HDMI_CERCR_valid_reset_mask|HDMI_CERCR_reset_err_det_mask|HDMI_CERCR_ch_locked_reset_mask));

    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg,
            ~(HDMI_CERCR_valid_reset_mask|HDMI_CERCR_reset_err_det_mask|HDMI_CERCR_ch_locked_reset_mask),
            0);
    }
}

void lib_hdmi_char_err_start(unsigned char nport, unsigned char conti_mode, unsigned char period)
{
    hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_en_mask), 0);

    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_reset_mask), (HDMI_CERCR_reset_mask));
    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_keep_err_det_mask),(HDMI_CERCR_keep_err_det_mask));

    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_period_mask|HDMI_CERCR_mode_mask), HDMI_CERCR_period(period)|HDMI_CERCR_mode(conti_mode));

    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_reset_mask),0);
    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_keep_err_det_mask),0);

    hdmi_mask(HDMI_CERCR_reg,~(HDMI_CERCR_refer_implem_mask), HDMI_CERCR_refer_implem_mask);

    hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_en_mask), HDMI_CERCR_en_mask);
}

unsigned char lib_hdmi_char_err_det_done(unsigned char nport)
{
    return (!HDMI_CERCR_get_en(hdmi_in(HDMI_CERCR_reg)));
}

unsigned char lib_hdmi_char_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{
    unsigned long flags;

    if (HDMI_CERCR_get_mode(hdmi_in(HDMI_CERCR_reg)))
    {  //continue mode
        // do nothing
    }
    else
    {
        if (HDMI_CERCR_get_en(hdmi_in(HDMI_CERCR_reg)) /*&&
            (!HDMI_CERCR_get_ch0_over_max_err_num(hdmi_in(HDMI_CERCR_reg)) &&
            !HDMI_CERCR_get_ch1_over_max_err_num(hdmi_in(HDMI_CERCR_reg)) &&
            !HDMI_CERCR_get_ch2_over_max_err_num(hdmi_in(HDMI_CERCR_reg)))*/)
        {
            return 0;	//not yet finish counting
        }
    }

    spin_lock_irqsave(&hdmi_spin_lock, flags);
    hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_err_cnt_sel_mask, 0); // switch to FW CED
    *b_err = HDMI_CERSR0_get_err_cnt0_video(hdmi_in(HDMI_CERSR0_reg));
    *g_err = HDMI_CERSR0_get_err_cnt1_video(hdmi_in(HDMI_CERSR0_reg));
    *r_err = HDMI_CERSR1_get_err_cnt2_video(hdmi_in(HDMI_CERSR1_reg));
    spin_unlock_irqrestore(&hdmi_spin_lock, flags);

    return 1;
}

unsigned char lib_hdmi_char_err_get_scdc_ced(unsigned char nport, unsigned char frl_mode, unsigned int err_cnt[4])
{
    unsigned long flags;

    spin_lock_irqsave(&hdmi_spin_lock, flags);

    hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_err_cnt_sel_mask, HDMI_CERCR_err_cnt_sel_mask); // switch to SCDC CED
    err_cnt[0] = HDMI_CERSR0_get_err_cnt0_video(hdmi_in(HDMI_CERSR0_reg));
    err_cnt[1] = HDMI_CERSR0_get_err_cnt1_video(hdmi_in(HDMI_CERSR0_reg));
    err_cnt[2] = HDMI_CERSR1_get_err_cnt2_video(hdmi_in(HDMI_CERSR1_reg));
    err_cnt[3] = 0;
    hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_err_cnt_sel_mask, 0); // switch back to FW mode

    spin_unlock_irqrestore(&hdmi_spin_lock, flags);

    return 1;
}

//------------------------------------------------------------------
// YUV 420 ti 444
//------------------------------------------------------------------

void lib_hdmi_420_en(unsigned char nport, unsigned char en)
{
    HDMI_INFO("[lib_hdmi_420_en nport:%d], en=%d\n", nport, en);
    hdmi_mask(HDMI_YUV420_CR_reg, ~HDMI_YUV420_CR_en_mask, HDMI_YUV420_CR_en(en?1:0));
}

void lib_hdmi_set_420_vactive(unsigned char nport, unsigned int vactive)
{
    hdmi_mask(HDMI_YUV420_CR1_reg, ~HDMI_YUV420_CR1_mode_mask, HDMI_YUV420_CR1_mode_mask);
    hdmi_mask(HDMI_YUV420_CR1_reg, ~HDMI_YUV420_CR1_vactive_mask, HDMI_YUV420_CR1_vactive(vactive));
    HDMI_INFO("lib_hdmi_set_420_vactive, nport =%d, vactive=%d\n", nport, vactive);
}

void lib_hdmi_420_fva_ds_mode(unsigned char nport, unsigned char en)
{
// MAC7P not support
    return;
}

void lib_hdmi_420_clkgen_en(unsigned char nport, unsigned char en)
{
// MAC7P not support
    return;
}

//------------------------------------------------------------------
// YUV420 4K 120 Hz mode
//------------------------------------------------------------------

void lib_hdmi_4k120_yu420_en(unsigned char nport, unsigned char en)
{
// MAC7P not support
    return;
}

void lib_hdmi_force_bypass_yuv420_engine_en(unsigned char nport, unsigned char en)
{
    if(en)
    {//en=1, bypass yuv420 engine, force send yuv 420 data;
        lib_hdmi_set_dpc_bypass_dis(nport, 0);    //[8] = 0, dpc_bypass_dis
        lib_hdmi_tmds_cps_pll_div2_en(nport, FALSE);
        lib_hdmi_420_clkgen_en(nport,  FALSE);
        lib_hdmi_420_en(nport,  0);    //[0] = 0, yuv420_en
    }
    else
    {// en=0,reset to normal
        unsigned char enable_2x = 0;
        unsigned char cd = lib_hdmi_get_color_depth(nport);

        enable_2x |= lib_hdmi_get_is_interlace_reg(nport);    //interlace must 2 x
        enable_2x |= (lib_hdmi_get_colorspace(nport) == COLOR_YUV420);    //YUV420 must 2 x
        if(enable_2x)
        {
            lib_hdmi_tmds_cps_pll_div2_en(nport, TRUE);
        }
        else
        {
            lib_hdmi_tmds_cps_pll_div2_en(nport, FALSE);
        }

        if(cd || enable_2x)
            lib_hdmi_set_dpc_bypass_dis(nport, 1);
        else
            lib_hdmi_set_dpc_bypass_dis(nport, 0);

        if(enable_2x)
        {
            lib_hdmi_420_clkgen_en(nport,  TRUE);
            lib_hdmi_420_en(nport,  1);
        }
        else
        {
            lib_hdmi_420_clkgen_en(nport,  FALSE);
            lib_hdmi_420_en(nport,  0);
        }
    }
}

//------------------------------------------------------------------
// Power Saving
//------------------------------------------------------------------

#define HDMI_PS_MODE_AUTO  0
#define HDMI_PS_MODE_CONTROLLED_BY_MAC  1
#define HDMI_PS_MODE_CONTROLLED_BY_FW   2

unsigned int hdmi_ps_mode = HDMI_PS_MODE_CONTROLLED_BY_FW;  // set default PS mode of HDMI1.4 to FW mode to prevent PS glitch
unsigned int hdmi_ps_phy_stable_time = 1;

void lib_hdmi_enter_power_saving_mode(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode, unsigned int last_pixel2vs_tmds_cnt, unsigned int vs2vs_cnt)
{
    unsigned int timer_cnt_end = vs2vs_cnt * 9 / 10;  // about 8 ms for 60Hz
    unsigned int hdmi_mode = lib_hdmi_is_hdmi_mode(nport);  // hw hdmi/dvi mode

    PWR_EMG("HDMI[p%d] enter power saveing (hdmi_mode(Current/SR_reg)=%d/%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d, last_pixel2vs_tmds_cnt=%d, vs2vs_cnt=%d, timer_cnt_end=%d)\n",
            nport, hdmi_mode, lib_hdmi_get_hdmi_mode_reg(nport), frl_mode, scrambled, hdcp_mode, last_pixel2vs_tmds_cnt, vs2vs_cnt, timer_cnt_end);
    if(get_ic_version()!=0)
    {
        //fix issue : RL6641-1297
        hdmi_ps_mode = HDMI_PS_MODE_AUTO;
    }

    if (scrambled==0 && hdmi_ps_mode==2)
    {
        scrambled = 1;
        PWR_EMG("hdmi_ps_mode==2, force running with PS mode 2\n");
    }

    // Gating Data output first to prevent incorrect data to be sent to the data island
    if (frl_mode == MODE_TMDS)
        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, 0); 
    else {
        // Not support HDMI 2.1, do nothing...
    }
        
    lib_hdmi_hdcp22_enable_reauth(nport, 0);
    hdmi_mask(HDMI_HDMI_SCR_reg, ~(HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode_mask),    //enable manual HDMI/DVI mode : need to use manual mode to prevent unstable data during PS
                                   HDMI_HDMI_SCR_mode_mask | HDMI_HDMI_SCR_msmode((hdmi_mode==MODE_HDMI) ? 1 : 0));

    lib_hdmi_set_colorspace_manual(nport, lib_hdmi_get_colorspace(nport));

    // set hdcp mode
    if (frl_mode == MODE_TMDS)
    {
        if (scrambled==0)
        {
            lib_hdmi_phy_enter_power_saving_mode(nport, HDMI_14, lib_hdmi_tmds_get_br_swap(nport));
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_timer_cnt_end_mask, HDMI_HD20_ps_ct_timer_cnt_end(timer_cnt_end));
            hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(1) | HDMI_HDCP_CR2_enc_window_de2vs(last_pixel2vs_tmds_cnt));
            hdmi_mask(HDMI_HDCP_HOA_reg, ~HDMI_HDCP_HOA_enc_method_mask, HDMI_HDCP_HOA_enc_method(1));
        }
        else
        {
            hdmi_out(HDMI_HDCP_CR2_reg, HDMI_HDCP_CR2_win_opp_mode(1) | HDMI_HDCP_CR2_enc_window_de2vs(last_pixel2vs_tmds_cnt));
        }
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    lib_hdmi_bch_error_irq_en(nport, 0); // disable bch error irq

    // Set Power Saving mode by HDCP mode
    if (frl_mode == MODE_TMDS)
    {
        switch(hdcp_mode)
        {
        case NO_HDCP:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(0));
            break;
        case HDCP14:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(1));
            break;
        case HDCP22:
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_hdcp_mode_mask, HDMI_HD20_ps_ct_hdcp_mode(2));
            break;
        }
    }
    else
    {
	// Not sipport HDMI 2.1... ignore it
    }

    // Start Power Saving mode
    if (frl_mode == MODE_TMDS)
    {
        if (scrambled==0)
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(1));
        else
        {
            hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, HDMI_HDMI_VCR_bg_gck(1));
            lib_hdmi_phy_enter_power_saving_mode(nport, HDMI_20, lib_hdmi_tmds_get_br_swap(nport));   // for HD20 signal, phy should be setting after mac ready
        }
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }
}

unsigned char lib_hdmi_phy_power_saving_status(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // not support HDMI 2.1, do nothing
        return 0;
    }
    else
    {
        return HDMI_HD20_ps_st_get_phy_power_on_flag(hdmi_in(HDMI_HD20_ps_st_reg));
    }
}

unsigned int hdmi_exit_power_saving_debug = 0;

void lib_hdmi_exit_power_saving_mode(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode)
{
    int i;
    int bch_err_cnt = 0;

    PWR_EMG("HDMI[p%d] exit power saveing (hdmi_mode=%d, frl_mode=%d, scrambled=%d, hdcp_mode=%d)\n",
            nport, lib_hdmi_is_hdmi_mode(nport), frl_mode, scrambled, hdcp_mode);

    if (scrambled==0 && hdmi_ps_mode==2)
    {
        scrambled = 1;
        PWR_EMG("hdmi_ps_mode==2, force running with PS mode 2\n");
    }

    if (frl_mode  == MODE_TMDS)
    {
        if (scrambled==0)
        {
            hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_ps_mode_mask, HDMI_HD20_ps_ct_ps_mode(0));
            lib_hdmi_phy_exit_power_saving_mode(nport, HDMI_14, lib_hdmi_tmds_get_br_swap(nport));
        }
        else
        {
            lib_hdmi_phy_exit_power_saving_mode(nport, HDMI_20, lib_hdmi_tmds_get_br_swap(nport));
            msleep(hdmi_ps_phy_stable_time);   // wait for phy stable. time? sync to aic
            hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, 0);
        }
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    if ((frl_mode == MODE_TMDS) && scrambled)
    {
        unsigned int timeout = 100;

        hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

        while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
            udelay(100);

        PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 1111 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));

        hdmi_mask(HDMI_CH_SR_reg, ~HDMI_CH_SR_rgb_de_align_flag_mask, HDMI_CH_SR_rgb_de_align_flag_mask);

        timeout = 100;

        while(HDMI_CH_SR_get_rgb_de_align_flag(hdmi_in(HDMI_CH_SR_reg))==0 && timeout--)
            udelay(100);

        PWR_FLOW_INFO("HDMI[p%d] rgb_de_align detected 2222 (%08x=%08x)\n", nport, HDMI_CH_SR_reg, hdmi_in(HDMI_CH_SR_reg));
    }

    if (hdmi_exit_power_saving_debug)
    {
        for(i=0; i<10; i++)
        {
            unsigned char RxStatus[2];

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] before mac out, RxStatus(%02x, %02x), bch error=%d/%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }

    if (frl_mode == MODE_TMDS)
    {
        unsigned int timeout = 100;
        hdmi_mask(HDMI_HD20_ps_st_reg, ~HDMI_HD20_ps_st_win_opp_detect_mask, HDMI_HD20_ps_st_win_opp_detect_mask);

        while(HDMI_HD20_ps_st_get_win_opp_detect(hdmi_in(HDMI_HD20_ps_st_reg))==0 && timeout--)
            udelay(100);

        hdmi_mask(HDMI_HD20_ps_ct_reg, ~HDMI_HD20_ps_ct_mac_out_en_mask, HDMI_HD20_ps_ct_mac_out_en_mask);        // enable macout
    }
    else
    {
        // not support HDMI 2.1, do nothing
    }

    lib_hdmi_hdcp_set_win_opp_mode(nport, 0);   // change win opp mode to VS mode
    msleep(50); // wait at least one frame
    lib_hdmi_hdcp22_enable_reauth(nport, 1);
    lib_hdmi_set_colorspace_auto(nport);        // change color space detection back to HW auto mode

    if (hdmi_exit_power_saving_debug || (frl_mode != MODE_TMDS))
    {
        for(i=0; i<30; i++)
        {
            unsigned char RxStatus[2];

            if (lib_hdmi_get_bch_2bit_error(nport))
                bch_err_cnt++;

            lib_hdmi_hdcp22_get_rx_status(nport, RxStatus);

            PWR_FLOW_INFO("HDMI[p%d] after mac out, RxStatus(%02x, %02x), bch error(1bit/2bit)=%d/%d, bch_err_cnt=%d, enc_toggle=%d\n",
                nport, RxStatus[0], RxStatus[1],
                lib_hdmi_get_bch_1bit_error(nport),
                lib_hdmi_get_bch_2bit_error(nport),
                bch_err_cnt,
                lib_hdmi_hdcp_get_enc_toggle(nport));

            if (lib_hdmi_get_bch_1bit_error(nport))
                lib_hdmi_bch_1bit_error_clr(nport);   // clear bch 1bit error

            if (lib_hdmi_get_bch_2bit_error(nport))
                lib_hdmi_bch_2bit_error_clr(nport);   // clear bch 2bit error

            if (lib_hdmi_hdcp_get_enc_toggle(nport))
                lib_hdmi_hdcp_clear_enc_toggle(nport);

            msleep(10);
        }
    }
}

void lib_hdmi_reset_power_saving_mode(unsigned char nport)
{
    // reset HD20 mac ps state
    hdmi_mask(HDMI_HDMI_VCR_reg, ~HDMI_HDMI_VCR_bg_gck_mask, 0); // disable bg clock gating (HD20)
    hdmi_out(HDMI_HD20_ps_ct_reg, HDMI_HD20_ps_ct_mac_out_en_mask);  // enable mac output

    // HDCP related
    hdmi_out(HDMI_HDCP_CR2_reg, 0);
    hdmi_out(HDMI_HDCP_HOA_reg, 0);
}

void lib_hdmi_dump_mac_power_saving_reg(unsigned char nport)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        // Do nothing ... not support HDMI 2.1
    }
    else
    {
        PWR_EMG("HDMI[p%d] mac setting : HD20_ps_ct=%08x (ps_mode=%d, hdcp_mode=%d, imer_cnt_end=%d, mac_out=%d), HD21_ps_st=%08x (phy_pwr_on=%d)\n",
            nport,
            hdmi_in(HDMI_HD20_ps_ct_reg), 
            HDMI_HD20_ps_ct_get_ps_mode(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_hdcp_mode(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_timer_cnt_end(hdmi_in(HDMI_HD20_ps_ct_reg)),
            HDMI_HD20_ps_ct_get_mac_out_en(hdmi_in(HDMI_HD20_ps_ct_reg)),
            hdmi_in(HDMI_HD20_ps_st_reg), 
            HDMI_HD20_ps_st_get_phy_power_on_flag(hdmi_in(HDMI_HD20_ps_st_reg)));
    }

    PWR_EMG("HDMI[p%d] mac setting : TMDS_PWDCTL=%08x, iccaf=%d, TMDS_OUTCTL=%08x, aoe=%d\n",
        nport,
        hdmi_in(HDMI_TMDS_PWDCTL_reg),
        HDMI_TMDS_PWDCTL_get_iccaf(hdmi_in(HDMI_TMDS_PWDCTL_reg)),
        hdmi_in(HDMI_TMDS_OUTCTL_reg),
        HDMI_TMDS_OUTCTL_get_aoe(hdmi_in(HDMI_TMDS_OUTCTL_reg)));

    PWR_EMG("HDMI[p%d] mac setting : HDMI_HDCP_CR=%08x, namfe=%d / HDMI_HDCP_CR2=%08x, enc_window_de2vs=%08x\n",
        nport,
        hdmi_in(HDMI_HDCP_CR_reg),
        HDMI_HDCP_CR_get_namfe(hdmi_in(HDMI_HDCP_CR_reg)),
        hdmi_in(HDMI_HDCP_CR2_reg),
        HDMI_HDCP_CR2_get_enc_window_de2vs(hdmi_in(HDMI_HDCP_CR2_reg)));
}

//------------------------------------------------------------------
// Power Saving Measure
//------------------------------------------------------------------

void lib_hdmi_ps_measure_enable(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_ps_measure_ctrl_reg, ~HDMI_ps_measure_ctrl_en_mask, (enable) ? HDMI_ps_measure_ctrl_en(1) : 0);
    hdmi_out(HDMI_ps_measure_xtal_clk_reg, 1);
    hdmi_out(HDMI_ps_measure_tmds_clk_reg, 1);
}

unsigned char lib_hdmi_ps_measure_complete(unsigned char nport)
{
    return (HDMI_ps_measure_ctrl_get_en(hdmi_in(HDMI_ps_measure_ctrl_reg))==0) ? 1 : 0;
}

void lib_hdmi_ps_measure_get_value(unsigned char nport, unsigned int* p_vs2vs_cycle, unsigned int* p_last_pixel2vs)
{
    if (!lib_hdmi_ps_measure_complete(nport))
    {
        *p_vs2vs_cycle = 0;
        *p_last_pixel2vs = 0;
        return;
    }

    *p_vs2vs_cycle = HDMI_ps_measure_xtal_clk_get_vs2vs_cycle_count(hdmi_in(HDMI_ps_measure_xtal_clk_reg));
    *p_last_pixel2vs = HDMI_ps_measure_tmds_clk_get_last_pixel2vs_count(hdmi_in(HDMI_ps_measure_tmds_clk_reg));
}

//------------------------------------------------------------------
// Video PLL Register (shared by all ports)
//------------------------------------------------------------------

unsigned char lib_hdmi_set_video_pll(
    unsigned char           nport,
    unsigned int            b_clk,
    unsigned char           frl_mode,
    unsigned char           cd,
    unsigned char           is_interlace,
    unsigned char           is_420
    )
{
    // Fin = Fxtal * b / 256
    // Target vco = ( Fin * m / n )     , TagretVco_HB = 500 ,  TagretVco_LB=250
    // Fin * m / n / 2^o / 2 * s = pixel clock = Fin * [24/30, 24/36, 24/48] ,  [10bits, 12bits,16bits]
    // FVCO = Fin * m / n
    // Fin = TMDS clock = pixel clock * SN/SM   --> pixel clock = Fin * SM / SN
    // pixel clock = FVCO / 2^o / 2*s
    // Fin * SM / SN = FVCO / 2^o / 2*s
    // FVCO = Fin *  2^o * 2*s * SM/SN
    // FVCO = Fin * 2^o * s * [24/30 * 2, 24/36 * 2, 24/48 * 2] --> Fin * 2^o * s * [8/5,4/3,1]
    //  250 <  ( Fin * m / n )  < 500  -->  250 <   Fin * 2^o * s * [ 8/5 , 4/3, 1 ]   < 500
    unsigned char enable_2x = 0;
    unsigned int large_ratio, Smean,Stest, m, n, o , fvco;
    unsigned long pixel_clockx1024;
    unsigned int bpsin, fpfd1024;
    unsigned int i,j,tabsize;
    unsigned char dpll_rs= 0, dpll_ip= 0;

    HDMI_INFO("lib_hdmi_set_video_pll, port=%d, b_clk=%d, cd=%d, interlace=%d, is_420=%d, ver=%d\n",
        nport, b_clk, cd, is_interlace, is_420, get_ic_version());

    lib_hdmi_420_en(nport, 0); // disable YUV420 function before Video PLL setting
    udelay(1);

    lib_hdmi_set_dpc_enable(nport, 0);  // disable DPC before change Video PLL, to prevent un-stable video clock input

    lib_hdmi_video_output(nport, 0);  // pll clock need to DISABLE

    udelay(1);

    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_pow_mask), 0);// Disable PLL
    udelay(1);
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_rstb_mask), 0);
    udelay(1);
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_freeze_mask), HDMI_HDMI_VPLLCR1_dpll_freeze_mask);

#define FVCO_MIN    250
#define FVCO_MAX    500

    //tmds clock = b * 27 / 256
    //tmds clock = pixel clock * SN/SM --> b* 27 / 256 = pixel clock * SN/SM
    //pixel clock = b * 27 * M / (SN * 256)
    //pixel clock * 1024 = b * 27 * M * 1024 / (N * 256)

    if (frl_mode == MODE_TMDS)
    {
        pixel_clockx1024 =((unsigned long)b_clk * 27 * dpll_ratio[cd].SM * 1024) / (dpll_ratio[cd].SN * 256);
        HDMI_INFO("[OFMS] Real Pixel clock(Base TMDS clock) =%ld (pixel_clock_1024 = %ld) \n", pixel_clockx1024, (pixel_clockx1024*1024L/1000L));

        if (pixel_clockx1024 == 0)
        {
            HDMI_EMG("[ERR] pixel_clockx1024 is zero=%d,%d\n", b_clk, cd);
            return FALSE;
        }
    }
    else
    {
        HDMI_EMG("[ERR] lib_hdmi_set_video_pll failed, not support FRL mode\n");
        return FALSE;
    }

    enable_2x = is_interlace;

    if ((pixel_clockx1024 < (160 * 1024) && enable_2x == 0) || (is_420))
        enable_2x = 1;      // if pixel_clock is under 160MHz then enable 2X clock maybe for DI

    // interlace must 2 x
    if (enable_2x)
    {
        large_ratio = 2;
        hdmi_mask(HDMI_TMDS_CPS_reg, ~HDMI_TMDS_CPS_pll_div2_en_mask, HDMI_TMDS_CPS_pll_div2_en(1));
    }
    else
    {
        large_ratio = 1;
        hdmi_mask(HDMI_TMDS_CPS_reg, ~HDMI_TMDS_CPS_pll_div2_en_mask, HDMI_TMDS_CPS_pll_div2_en(0));
    }

    if  (cd || enable_2x)
        lib_hdmi_set_dpc_bypass_dis(nport, 1);   // disable deep color bypass
    else
        lib_hdmi_set_dpc_bypass_dis(nport, 0);   // disable deep color bypass

    if (large_ratio != 1)
        pixel_clockx1024 <<= 1;

    // s = smean
    // FVCO_MIN * 1024 / 2^o / 2s <= pixel_clock1024
    // (pixel_clock1024 * 2^o) * 2s >= FVCO_MIN * 1024
    // (pixel_clock1024 * 2) * 2s >= FVCO_MIN * 1024
    //ex: s = 0 , 2s = Stest = 1 (no div)
    //ex: s = 1 , 2s = Stest = 2 (div 2)

    o = 1;
    Smean = 0;
    Stest = 1;
    while(pixel_clockx1024 <= (200*1024))
    {
        if (Smean == 0)
            Stest = 1;
        else
            Stest = Smean * 2;

        // 2---> 2^o
        if (((pixel_clockx1024*2)*Stest)>= (FVCO_MIN*1024))
            break;
        Smean ++;
    };

    if (pixel_clockx1024 > (200*1024)) { // >200MHz
        o = 0;
        Smean = 0;
    }
    /*else if (pixel_clockx1024 > (100*1024)){//100MHz ~200MHz
        o = 1;
        Smean = 0;
    }
    else if (pixel_clockx1024 > (50*1024)){//50MHz ~100MHz
        o = 1;
        Smean = 1;
    }
    else if (pixel_clockx1024 > (25*1024)){//25MHz ~50MHz
        o = 1;
        Smean = 2;
    }
    else {//13.5MHz~25MHz
        o = 1;
        Smean = 4;
    }*/

    //HDMI_PRINTF( "Smean =  %d\n", Smean);

    //if (Smean == 0) Stest = 1;
    //else Stest = Smean * 2;

    ////FVCO = Fin * m / n
    //Fin = TMDS clock = pixel clock * SN/SM   --> pixel clock = Fin * SM / SN
    //pixel clock = FVCO / 2^o / 2*s *  (1/large_ratio)
    //Fin * SM / SN = FVCO / 2^o / 2*s * (1/large_ratio)
    //FVCO = Fin *  2^o * 2*s * SM/SN * large_ratio
    //Fin * m / n = Fin * 2^o * 2*s * SM/SN * large_ratio
    //m/n = 2^o * 2*s * SM/SN * large_ratio
    //when n determined, m = SM * (2*s) * n * large_ratio * 2^o / SN
    //m = (SM * (2*s) * n * large_ratio ) << o / SN

    if (frl_mode == MODE_TMDS)
    {
        n = 0;
        do
        {
            n += dpll_ratio[cd].RatioN;
            m =((dpll_ratio[cd].RatioM * Stest * n * large_ratio)<<o) / dpll_ratio[cd].RatioN;
            //HDMI_PRINTF( "step1%d %d\n", m, n);
        } while(n < 2);

        //M code must >= 6
        while(m < 8)
        {
            n += dpll_ratio[cd].RatioN;
            m =((dpll_ratio[cd].RatioM * Stest * n * large_ratio)<<o) / dpll_ratio[cd].RatioN;
            //HDMI_PRINTF( "step2:%d %d\n", m, n);
        }


        if (n >8)
        {
            n = 0;
            do
            {
                n += dpll_ratio[cd].RatioN;
                m =((dpll_ratio[cd].RatioM * Stest * n * large_ratio)<<o) / dpll_ratio[cd].RatioN;
                //HDMI_PRINTF( "step3:%d %d\n", m, n);
            } while(n < 2);
        }

        fvco = (b_clk * 27 * m) / (256 * n);

        //mag2 PLL bug  m min must > 1 , or PLL will crash

        while (m<3)
        {
            n <<= 1;
            m <<= 1;
        }

        if (n > 9)
        {
            HDMI_EMG("[ERR] m=%d, n=%d > 9\n", m, n);
        }
    }
    else
    {
        n = 3;  //fix n = 2 for 297M or 594M
        fvco = pixel_clockx1024 >> 10 ;
        m = fvco * n / 27;
    }

    if (frl_mode == MODE_TMDS)
    {
        tabsize = sizeof(video_pll_main_param)/sizeof(VIDEO_DPLL_FIX_T);

        //search main table
        for (i = 0; i < tabsize ; ++i)
        {
            if (((m - 2) == video_pll_main_param[i].dpll_M) && ((n - 2) == video_pll_main_param[i].dpll_N))
            {
                dpll_ip = video_pll_main_param[i].dpll_IP;
                dpll_rs = video_pll_main_param[i].dpll_RS;
                break;
            }
        }

        // search other table,  if not fiind in main table
        if (i >= tabsize)
        {
            //fpfd = fin/(DPLL_BPSIN + (1-DPLL_BPSIN) * (DPLL_N+2))
            bpsin = HDMI_HDMI_VPLLCR0_get_dpll_bpsin(hdmi_in(HDMI_HDMI_VPLLCR0_reg));

            if (frl_mode == MODE_TMDS)
                fpfd1024 = (b_clk * 27 * 1024 / 256) / (bpsin + ((1-bpsin) * n));
            else  //frl mode
                fpfd1024 = (27 * 1024) / (bpsin + ((1-bpsin) * n));

            //(13.5 - 0.3) * 1024 = 13516  ,  (40+0.3) * 1024 = 41267  (margin +- 0.3M)
            if ((fpfd1024 < 13516) || (fpfd1024 > 41267))
            {  //margin +- 0.3M
                HDMI_EMG("[WARNING] fpfd is not correct range %d \n", fpfd1024);
            }

            for (j = 0 ; j < 4 ; ++j)
            {
                if (((m - 2) >= video_pll_others_param[j].min_M ) && ((m - 2) <= video_pll_others_param[j].max_M))
               {
                    dpll_ip = video_pll_others_param[j].dpll_IP;
                    dpll_rs = video_pll_others_param[j].dpll_RS;
                    break;
                }
            }

            if (j == 4)
            {
                dpll_ip =  2;
                dpll_rs = 3;
                HDMI_EMG("[ERROR] video pll table not find\n");
            }
        }
    }
    else
    {
        tabsize = sizeof(video_pll_frl_param)/sizeof(VIDEO_DPLL_FIX_T);
        for (i = 0; i < tabsize ; ++i)
        {
            if (((m - 2) == video_pll_frl_param[i].dpll_M) && ((n - 2) == video_pll_frl_param[i].dpll_N))
            {
                dpll_ip = video_pll_frl_param[i].dpll_IP;
                dpll_rs = video_pll_frl_param[i].dpll_RS;
                break;
            }

            if (i >= tabsize)
            {
                HDMI_EMG("[WARNING] can not find video table for frl\n");
            }
        }
    }
    #if 0    //debugging use
    HDMI_INFO("***************TMDS=%d\n", b_clk);
    HDMI_INFO("***************cd=%d\n",cd);
    HDMI_INFO("***************m=%d\n",m);
    HDMI_INFO("***************n=%d\n",n);
    HDMI_INFO("***************o=%d\n",o);
    HDMI_INFO("***************s=%d\n",Smean);
    //HDMI_PRINTF("***************fraction1=%d\n",fraction1);
    //HDMI_PRINTF("***************fraction2=%d\n",fraction2);
    HDMI_INFO("***************pixel_clockx1024=%ld\n",pixel_clockx1024);
    HDMI_INFO("***************fvco=%d MHz\n",fvco);
    HDMI_INFO("***************larget=%d\n",large_ratio);
    HDMI_INFO("***************ip=%d\n",dpll_ip);
    HDMI_INFO("***************rs=%d\n",dpll_rs);
    #endif
    HDMI_INFO("fvco=%d MHz\n",fvco);

    hdmi_out(HDMI_HDMI_VPLLCR0_reg, HDMI_HDMI_VPLLCR0_dpll_m(m-2)|HDMI_HDMI_VPLLCR0_dpll_o(o)|HDMI_HDMI_VPLLCR0_dpll_n(n-2)|HDMI_HDMI_VPLLCR0_dpll_rs(dpll_rs)|HDMI_HDMI_VPLLCR0_dpll_ip(dpll_ip));    //set video PLL parameter
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_cp_mask), HDMI_HDMI_VPLLCR1_dpll_cp(1));
    hdmi_out(HDMI_MN_SCLKG_DIVS_reg,Smean);     //set video PLL output divider

    if (frl_mode == MODE_TMDS)
        hdmi_mask(HDMI_MN_SCLKG_CTRL_reg,~HDMI_MN_SCLKG_CTRL_sclkg_pll_in_sel_mask,HDMI_MN_SCLKG_CTRL_sclkg_pll_in_sel_mask);
    else
        hdmi_mask(HDMI_MN_SCLKG_CTRL_reg,~HDMI_MN_SCLKG_CTRL_sclkg_pll_in_sel_mask,0x0);

    hdmi_mask(HDMI_MN_SCLKG_CTRL_reg, ~HDMI_MN_SCLKG_CTRL_sclkg_dbuf_mask, HDMI_MN_SCLKG_CTRL_sclkg_dbuf_mask);       //video PLL double buffer load
    udelay(1);

    //video PLL power enable
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_ldo_pow_mask), HDMI_HDMI_VPLLCR1_dpll_ldo_pow_mask);
    udelay(1);
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_pow_mask), HDMI_HDMI_VPLLCR1_dpll_pow_mask); //20140922
    udelay(1);
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_rstb_mask), HDMI_HDMI_VPLLCR1_dpll_rstb_mask);
    udelay(40);
    hdmi_mask(HDMI_HDMI_VPLLCR1_reg, ~(HDMI_HDMI_VPLLCR1_dpll_freeze_mask), 0);

    lib_hdmi_video_output(nport, 1);  // pll clock need to enable ouput before pll_pow and 420_en on. //follow 20170125 huang kuan-chia's mail

    if(get_ic_version() < VERSION_C)
    {
        // RL6583-277 : work around of YUV420 issue
        // need special flow for YUV420
        if (is_420)
        {
            if(frl_mode !=MODE_TMDS)
            {
                HDMI_EMG("[ERR] lib_hdmi_set_video_pll failed, not support FRL mode\n");
                return FALSE;
            }
            else
            {   // TMDS mode
                lib_hdmi_420_en(nport, 1);  // enable yuv 420
                // wait vs to start dpc...
                lib_hdmi_wait_hdcp_vs(nport, 3, 50); // need to wait vs to start dpc within 50ms
                lib_hdmi_set_dpc_enable(nport, 1);   // enable deep color support
            }

            return TRUE;
        }
    }

    udelay(1);

    if(frl_mode !=MODE_TMDS)
    {
        HDMI_EMG("[ERR] lib_hdmi_set_video_pll failed, not support FRL mode\n");
        return FALSE;
    }
    else
    {
        lib_hdmi_set_dpc_enable(nport, 1);  // enable deep color support
        udelay(1);
        lib_hdmi_420_en(nport, (is_420) ? 1 : 0); // enable YUV420 function
    }

    return TRUE;
}

//------------------------------------------------------------------
// Video Common CRC
//------------------------------------------------------------------

void lib_hdmi_crc_continue_mode_en(unsigned char enable)
{
    if(enable)
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~(HDMI_CRC_Ctrl_crc_conti_mask), HDMI_CRC_Ctrl_crc_conti(1) ); //enable CRC_conti
    else
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~(HDMI_CRC_Ctrl_crc_conti_mask), HDMI_CRC_Ctrl_crc_conti(0) ); //clr CRC_conti
}
int lib_hdmi_get_crc_continue_mode_status(void)
{
    return HDMI_CRC_Ctrl_get_crc_conti(hdmi_in(HDMI_CRC_Ctrl_reg));
}
void lib_hdmi_set_des_crc(unsigned int common_crc)
{
    hdmi_out(HDMI_DES_CRC_reg, common_crc);
}
int lib_hdmi_get_des_crc(void)
{
    return HDMI_P0_DES_CRC_get_crc_des(hdmi_in(HDMI_DES_CRC_reg));
}
int lib_hdmi_get_crc_err_cnt(void)
{
    return HDMI_P0_CRC_ERR_CNT0_get_crc_err_cnt(hdmi_in(HDMI_CRC_ERR_CNT0_reg));
}
void lib_hdmi_crc_auto_cmp_en(unsigned char enable)
{
    if(enable){
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~HDMI_CRC_Ctrl_crc_auto_cmp_en_mask, 0);
        udelay(100);
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~HDMI_CRC_Ctrl_crc_auto_cmp_en_mask, HDMI_CRC_Ctrl_crc_auto_cmp_en_mask);
        udelay(100);
    }else {
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~HDMI_CRC_Ctrl_crc_auto_cmp_en_mask, 0);
        udelay(100);
    }
}
int lib_hdmi_get_crc_auto_cmp_status(void)
{
    return HDMI_P0_CRC_Ctrl_get_crc_auto_cmp_en(hdmi_in(HDMI_CRC_Ctrl_reg));
}
int lib_hdmi_read_video_common_crc(unsigned char res_sel, unsigned int* p_crc, unsigned char is_continue_mode)
{// Common crc
    if(is_continue_mode)
    {
        *p_crc= hdmi_in(HDMI_CRC_Result_reg);
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~(HDMI_CRC_Ctrl_crc_conti_mask | HDMI_CRC_Ctrl_crc_res_sel_mask | HDMI_CRC_Ctrl_crc_start_mask),
                        HDMI_CRC_Ctrl_crc_conti(1) | HDMI_CRC_Ctrl_get_crc_res_sel(res_sel) | HDMI_CRC_Ctrl_crc_start(1));
        if(*p_crc == 0){
            return -1;
        }
        return 0;
    }
    else
    {
        hdmi_mask(HDMI_CRC_Ctrl_reg, ~(HDMI_CRC_Ctrl_crc_conti_mask), 0); //clr CRC_conti
        udelay(100);
    if (HDMI_CRC_Ctrl_get_crc_start(hdmi_in(HDMI_CRC_Ctrl_reg))==0)
    {
        // restart CRC
        *p_crc= hdmi_in(HDMI_CRC_Result_reg);
            hdmi_mask(HDMI_CRC_Ctrl_reg, ~(HDMI_CRC_Ctrl_crc_conti_mask | HDMI_CRC_Ctrl_crc_res_sel_mask | HDMI_CRC_Ctrl_crc_start_mask),
                        HDMI_CRC_Ctrl_crc_conti(0) |HDMI_CRC_Ctrl_get_crc_res_sel(res_sel) | HDMI_CRC_Ctrl_crc_start(1)); // When crc_start=1, it will clear CRC result to 0.
        return 0;
    }
    return -1;
    }
}

//------------------------------------------------------------------
// One Time Register (shared by all ports)
//------------------------------------------------------------------

void lib_hdmi_ddc_sel(unsigned char nport, unsigned char ddc_ch)
{
    //TODO : bind DDC channel to HDMI MAC
    //Do nothing, to simpliy the design, the do not support random ddc mac routing...

    switch(ddc_ch)
    {
    case 0:
        hdmi_mask(HDMI_STB_ot_reg, ~HDMI_STB_ot_ddc_ch0_sel_mask, HDMI_STB_ot_ddc_ch0_sel(nport));
        break;
    case 1:
        hdmi_mask(HDMI_STB_ot_reg, ~HDMI_STB_ot_ddc_ch1_sel_mask, HDMI_STB_ot_ddc_ch1_sel(nport));
        break;
    case 2:
        hdmi_mask(HDMI_STB_ot_reg, ~HDMI_STB_ot_ddc_ch2_sel_mask, HDMI_STB_ot_ddc_ch2_sel(nport));
        break;
    case 3:
        hdmi_mask(HDMI_STB_ot_reg, ~HDMI_STB_ot_ddc_ch3_sel_mask, HDMI_STB_ot_ddc_ch3_sel(nport));
        break;
    default:
        // do nothing...
        HDMI_INFO("lib_hdmi_ddc_sel failed, invalid ddc_ch=%d\n", ddc_ch);
        return;
    }

    HDMI_INFO("lib_hdmi_ddc_sel (port=%d, ddc_ch=%d, HDMI_STB_ot_reg=%08x )\n", nport, ddc_ch, hdmi_in(HDMI_STB_ot_reg));
}

//------------------------------------------------------------------
// FW Function (shared by all register)
//------------------------------------------------------------------

void lib_hdmi_mac_select_port(unsigned char nport)
{   // reset all mac
    HDMI_INFO("[lib_hdmi_mac_select_port port:%d]\n", nport);
    hdmi_mask(HDMI_PORT_SWITCH_reg,
        ~(HDMI_PORT_SWITCH_port_sel_mask),
        (HDMI_PORT_SWITCH_port_sel(nport)));
}

void lib_hdmi_meas_select_port(unsigned char nport)
{
    unsigned char offms_hd21 = (hdmi_rx[nport].hdmi_2p1_en) ? 1 : 0;

    hdmi_mask(HDMI_PORT_SWITCH_reg,
        ~(HDMI_PORT_SWITCH_offms_port_sel_mask   | HDMI_PORT_SWITCH_offms_hd21_sel_mask),
          HDMI_PORT_SWITCH_offms_port_sel(nport) | HDMI_PORT_SWITCH_offms_hd21_sel(offms_hd21));
}

//------------------------------------------------------------------
// HD20/21 CTRL
//------------------------------------------------------------------
void lib_hdmi_set_hd20_21_ctrl_pixel_mode(unsigned char nport, unsigned char pixel_mode)
{//at video common, p0=p1=p2=p3 
    switch(pixel_mode)
    {
    case 1: // 1 pixel mode
        hdmi_mask(HDMI_hdmi_20_21_ctrl_reg, ~HDMI_hdmi_20_21_ctrl_pixel_mode_mask, HDMI_hdmi_20_21_ctrl_pixel_mode(0));
        break;

    case 2: // 2 pixel mode
        hdmi_mask(HDMI_hdmi_20_21_ctrl_reg, ~HDMI_hdmi_20_21_ctrl_pixel_mode_mask, HDMI_hdmi_20_21_ctrl_pixel_mode(1));
        break;
    case 4: // 4 pixel mode
        HDMI_ERR("[lib_hdmi_set_hd20_21_ctrl_pixel_mode port:%d] Not support 4 pixel mode on this Model\n", nport);
        break;
    default:
        HDMI_EMG("[lib_hdmi_set_hd20_21_ctrl_pixel_mode port:%d] invalid pixel mode =%d\n", nport, pixel_mode);
        break;
    }
    HDMI_INFO("[lib_hdmi_set_hd20_21_ctrl_pixel_mode nport=%d] pixel_mode=%d \n", nport, pixel_mode);
}

//-------------------------------------------------------------------------
// Dummy registers that are reserved for FW debug usage.
//
// To easy to debug, HDMI driver provides serval reserved register bits
// to allow user to change behavior of the HDMI driver.
//
// Please note that these registers might vary on different SOC.
//
// On K5LP, the FW debug registers are show as below
//-------------------------------------------------------------------------

/*---------------------------------------------------
 * Func : lib_hdmi_get_fw_debug_bit
 *
 * Desc : get FW debug bits status.
 *
 * Para : debug_bit : debug bit mask
 *        val       : 0 : disable, others  enabled
 *
 * Retn : 0 : disabled, 1 : eanbled
 *--------------------------------------------------*/
unsigned char lib_hdmi_get_fw_debug_bit(unsigned int debug_bit)
{
    return ((hdmi_in(HDMI_FW_FUNC_reg) & debug_bit)) ? 1 : 0;
}

/*---------------------------------------------------
 * Func : lib_hdmi_set_fw_debug_bit
 *
 * Desc : set FW debug bits.
 *
 * Para : debug_bit : debug bit mask
 *        val       : 0 : disable, others  enabled
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_set_fw_debug_bit(unsigned int debug_bit, unsigned char on)
{
    hdmi_mask(HDMI_FW_FUNC_reg, ~debug_bit, (on) ? debug_bit : 0);
}

//------------------------------------------------------------------
// High level API
//------------------------------------------------------------------


void lib_hdmi_mac_init(unsigned char nport, unsigned char br_swap)
{
    HDMI_WARN("lib_hdmi_mac_init(p%d, br_swap=%d)\n", nport, br_swap);

    lib_hdmi_mac_select_port(nport);
    hdmi_out(HDMI_POWER_SAVING_reg, 0x00); // disable pattern gen clock

    // ----------------------------------------- EX PHY FIFO -------------------------------------------------
#if 0 // Need to Fix
    // 1. ex phy fifo ck en
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_r_afifo_en_mask));    // Port0
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_r_afifo_en_mask));    // Port1
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_r_afifo_en_mask));    // Port2
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_b_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_g_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_r_afifo_en_mask));    // Port2

    // 2. ex phy afifo en
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR2_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR2_port0_c_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR2_port1_c_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR2_port0_c_afifo_en_mask|HDMI_EX_P0_EX_PHY_FIFO_CR2_port1_c_afifo_en_mask));    // Port0_c_afifo_en & Port1_c_afifo_en = 1

    // 3. ex mac  clk invert
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR1_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_rclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_gclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_bclk_inv_mask),
       (HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_rclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_gclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port0_bclk_inv_mask));    // port0
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR1_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_rclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_gclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_bclk_inv_mask),
       (HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_rclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_gclk_inv_mask|HDMI_EX_P0_EX_PHY_FIFO_CR1_port1_bclk_inv_mask));    // port1

    // 4. ex phy fifo c clk inv
    hdmi_mask(HDMI_EX_P0_EX_PHY_FIFO_CR3_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR3_port0_cclk_inv_mask| HDMI_EX_P0_EX_PHY_FIFO_CR3_port1_cclk_inv_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR3_port0_cclk_inv_mask| HDMI_EX_P0_EX_PHY_FIFO_CR3_port1_cclk_inv_mask));    //Port0_cclk_inv & Port1_cclk_inv = 1

    // 5. PHY FIFO R/G/B ch async fifo enable
    hdmi_mask(HDMI_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port0_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port0_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port0_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port0_r_afifo_en_mask));    // Port0
    hdmi_mask(HDMI_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port1_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port1_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port1_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port1_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port1_r_afifo_en_mask));    // Port1
    hdmi_mask(HDMI_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port2_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port2_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port2_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port2_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port2_r_afifo_en_mask));    // Port2
    hdmi_mask(HDMI_PHY_FIFO_CR0_reg,
        ~(HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port3_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port3_r_afifo_en_mask),
        (HDMI_EX_P0_EX_PHY_FIFO_CR0_port3_b_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port3_g_afifo_en_mask|HDMI_P0_PHY_FIFO_CR0_port3_r_afifo_en_mask));    // Port3
#endif

    // 6. PHY FIFO R/G/B PHY input clk invert
    hdmi_mask(HDMI_PHY_FIFO_CR1_reg,
        ~(HDMI_P0_PHY_FIFO_CR1_port3_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port3_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port3_rclk_inv_mask),
        (HDMI_P0_PHY_FIFO_CR1_port3_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port3_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port3_rclk_inv_mask));    // Port3
    hdmi_mask(HDMI_PHY_FIFO_CR1_reg,
        ~(HDMI_P0_PHY_FIFO_CR1_port2_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port2_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port2_rclk_inv_mask),
        (HDMI_P0_PHY_FIFO_CR1_port2_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port2_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port2_rclk_inv_mask));    // Port2
    hdmi_mask(HDMI_PHY_FIFO_CR1_reg,
        ~(HDMI_P0_PHY_FIFO_CR1_port1_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port1_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port1_rclk_inv_mask),
        (HDMI_P0_PHY_FIFO_CR1_port1_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port1_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port1_rclk_inv_mask));    // Port1
    hdmi_mask(HDMI_PHY_FIFO_CR1_reg,
        ~(HDMI_P0_PHY_FIFO_CR1_port0_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port0_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port0_rclk_inv_mask),
        (HDMI_P0_PHY_FIFO_CR1_port0_bclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port0_gclk_inv_mask|HDMI_P0_PHY_FIFO_CR1_port0_rclk_inv_mask));    // Port0


    // 20161018: SL8800 HDCP CTS1.4, contiune test without disconnect tmds_clock.
    hdmi_out(HDMI_HDMI_SCR_reg , 0x163);//DVI/HDMI condition(A,B) select  /* hdcp cts1.4 : Bstatus (MSB): 0x00*/

    lib_hdmi_audio_init(nport);
    lib_hdmi_video_output(nport, 1);

    newbase_hdmi_scdc_init(nport);

    lib_hdmi_avmute_irq_disable(nport);

    //hdmi_mask(HDMI_SCDC_PCR_reg, (~HDMI_SCDC_PCR_i2c_free_num_mask), HDMI_SCDC_PCR_i2c_free_num(0x0a));
    hdmi_mask(HDMI_HDMI_2p0_CR_reg,~(HDMI_HDMI_2p0_CR_hdmi_2p0_en_mask),HDMI_HDMI_2p0_CR_hdmi_2p0_en_mask);

    hdmi_mask(HDMI_HDCP_PCR_reg, ~HDMI_HDCP_PCR_hdcp_vs_sel_mask, HDMI_HDCP_PCR_hdcp_vs_sel_mask);

    hdmi_mask(HDMI_HDMI_VCR_reg, ~(HDMI_HDMI_VCR_prdsam_mask|HDMI_HDMI_VCR_csam_mask),  HDMI_HDMI_VCR_prdsam(1)|HDMI_HDMI_VCR_csam(1));

    if (br_swap)        // swap BR lane if necessary
        hdmi_mask(HDMI_TMDS_PWDCTL_reg, ~(HDMI_TMDS_PWDCTL_brcw_mask), HDMI_TMDS_PWDCTL_brcw_mask);
    else
        hdmi_mask(HDMI_TMDS_PWDCTL_reg, ~(HDMI_TMDS_PWDCTL_brcw_mask), 0);

#ifdef HDMI_FIX_HDMI_POWER_SAVING
    // when PS mode is enabled, only R lane still alive in ECO mode
    if (lib_hdmi_tmds_get_br_swap(nport)==0)
        hdmi_mask(HDMI_HDMI_VCR_reg,~HDMI_HDMI_VCR_iclk_sel_mask,HDMI_HDMI_VCR_iclk_sel(2));    // by default reference R lane
    else
        hdmi_mask(HDMI_HDMI_VCR_reg,~HDMI_HDMI_VCR_iclk_sel_mask,HDMI_HDMI_VCR_iclk_sel(0));    // by default reference B(R) lane
#else
    hdmi_mask(HDMI_HDMI_VCR_reg,~HDMI_HDMI_VCR_iclk_sel_mask,HDMI_HDMI_VCR_iclk_sel(0));    // by default reference B lane
#endif


    lib_hdmi_set_interrupt_enable(nport, 0);  // disable irq to A/V CPU
    hdmi_mask(HDMI_HDMI_VCR_reg, ~(HDMI_HDMI_VCR_eoi_mask) , 0);  // not inverse EVEN/ODD

    lib_hdmi_set_video_packet_variation_irq(nport, 0x0);  //Disable packet variation Watch Dog

    //hdmi_mask(HDMI_HDMI_AOCR_reg,(~0x0ff),0xFF);  //Enable SPDIF/I2S Output

    hdmi_mask(HDMI_TMDS_ERRC_reg, ~HDMI_TMDS_ERRC_nl_mask, HDMI_TMDS_ERRC_nl(3));

    lib_hdmi_set_bch_eanble(nport, 1);  //willychou enable bch error function
    lib_hdmi_fw_char_err_rst(nport, 1); //reset fw CED until measure done
    lib_hdmi_scdc_char_err_rst(nport, 1);  //reset scdc CED until measure done
    lib_hdmi_bit_err_rst(nport, 1);  //reset bit err det
    lib_hdmi_set_dpc_enable(nport, 0); // disable dpc
    lib_hdmi_ofms_clock_sel(nport, 0);  // tmds clock
    newbase_hdmi_hdcp_enable(nport, 1); // enable hdcp

    lib_hdmi_mac_release(nport, MODE_TMDS); // QC: when ac on, need this for hdcp auth, otherwise no clock in MSHG-800
}


/*---------------------------------------------------
 * Func : lib_hdmi_mac_set_hde_enable
 *
 * Desc : enable/disable hde
 *
 * Para : nport : HDMI port number
 *        enable : 0 : Disable, gated clock and cut off
 *                     TMDS pull up resistor for saving power.
 *                 other : Enable
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_mac_set_hde_enable(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_TMDS_Z0CC_reg, ~HDMI_TMDS_Z0CC_hde_mask, (enable) ?  HDMI_TMDS_Z0CC_hde_mask : 0);
}

/*---------------------------------------------------
 * Func : lib_hdmi_mac_hd20_tmds_input_eanbale
 *
 * Desc : enable/disable TMDS input for HD20 Mac
 *
 * Para : nport : HDMI port number
 *        enable : 0 : Disable
 *                 other : Enable
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_mac_hd20_tmds_input_eanbale(unsigned char nport, unsigned char enable)
{
    HDMI_WARN("lib_hdmi_mac_hd20_tmds_input_eanbale(%d,%d)\n", nport, enable);

    if (enable)
    {
        hdmi_mask(HDMI_TMDS_PWDCTL_reg,
            ~(HDMI_TMDS_PWDCTL_ebip_mask|
              HDMI_TMDS_PWDCTL_egip_mask|
              HDMI_TMDS_PWDCTL_erip_mask|
              HDMI_TMDS_PWDCTL_ecc_mask),
             (HDMI_TMDS_PWDCTL_ebip(1)|
              HDMI_TMDS_PWDCTL_egip(1)|
              HDMI_TMDS_PWDCTL_erip(1)|
              HDMI_TMDS_PWDCTL_ecc(1)));
    }
    else
    {
        hdmi_mask(HDMI_TMDS_PWDCTL_reg,
            ~(HDMI_TMDS_PWDCTL_ebip_mask|
              HDMI_TMDS_PWDCTL_egip_mask|
              HDMI_TMDS_PWDCTL_erip_mask|
              HDMI_TMDS_PWDCTL_ecc_mask), 0);
    }
}

void lib_hdmi_mac_reset(unsigned char nport)
{
    lib_hdmi_mac_crt_perport_reset(nport);

#ifdef HDMI_FIX_HDMI_POWER_SAVING
    lib_hdmi_reset_power_saving_mode(nport);  // reset power saving registers (just in case...)
#endif

    hdmi_mask(HDMI_TMDS_DPC_SET0_reg, ~(HDMI_TMDS_DPC_SET0_dpc_en_mask), 0);
    lib_hdmi_afifo_enable(nport, 0);
    lib_hdmi_mac_hd20_tmds_input_eanbale(nport, 0);
    lib_hdmi_mac_set_hde_enable(nport, 0);   // Disable clock and cut off TMDS pull up resistor for power saving
}

void lib_hdmi_mac_release(unsigned char nport, unsigned char frl_mode)
{
    lib_hdmi_mac_set_hde_enable(nport, 1);  // Enable clock and connect TMDS pull up resistor

#ifdef HDMI_DISABLE_TMDS_INPUT_ON_FRL_MODE
    lib_hdmi_mac_hd20_tmds_input_eanbale(nport, (frl_mode!=MODE_TMDS) ? 0 : 1);  // only enable TMDS input on HD20 mode
#else
    lib_hdmi_mac_hd20_tmds_input_eanbale(nport, 1);
#endif
    lib_hdmi_afifo_enable(nport, 1);
    lib_hdmi_set_dpc_enable(nport, 0);
}

//------------------------------------------------------------------
// DMA VGIP
//------------------------------------------------------------------

void lib_hdmi_dma_vgip_capture_pixel_enable(unsigned short x, unsigned short y)
{
#ifdef CONFIG_FORCE_RUN_I3DDMA
    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en(0)); //enable capture

    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_vcaptst_num_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_vcaptst_num(y));
    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_hcaptst_num_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_hcaptst_num(x));

    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_mode_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_mode(0)); //capture mode select

    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en(1)); //enable capture
#else
    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en(0)); //enable capture

    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_vcaptst_num_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_vcaptst_num(y));
    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_hcaptst_num_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_hcaptst_num(x));

    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_mode_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_mode(0)); //capture mode select

    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en(1)); //enable capture
#endif
}

void lib_hdmi_dma_vgip_capture_pixel_disable(void)
{
#ifdef CONFIG_FORCE_RUN_I3DDMA
    hdmi_mask(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg, ~DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en_mask,
                                               DMA_VGIP_DMA_VGIP_PTN_H_VI_dma_captst_en(0)); //enable capture
#else
    hdmi_mask(VGIP_VGIP_CHN1_PTN_H_VI_reg,    ~VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en_mask,
                                               VGIP_VGIP_CHN1_PTN_H_VI_ch1_captst_en(0)); //enable capture
#endif
}

unsigned int lib_hdmi_dma_vgip_is_video_capture_en(void)
{
#ifdef CONFIG_FORCE_RUN_I3DDMA
    return DMA_VGIP_DMA_VGIP_PTN_H_VI_get_dma_captst_en(hdmi_in(DMA_VGIP_DMA_VGIP_PTN_H_VI_reg));
#else
    return VGIP_VGIP_CHN1_PTN_H_VI_get_ch1_captst_en(hdmi_in(VGIP_VGIP_CHN1_PTN_H_VI_reg));
#endif
}

void lib_hdmi_dma_vgip_get_pixel_value(unsigned short val[3])
{
#ifdef CONFIG_FORCE_RUN_I3DDMA
    val[0] = DMA_VGIP_DMA_VGIP_PTN0_get_dma_captst_r_data(hdmi_in(DMA_VGIP_DMA_VGIP_PTN0_reg)); // Pixel 1 , R data 12bit
    val[1] = DMA_VGIP_DMA_VGIP_PTN0_get_dma_captst_g_data(hdmi_in(DMA_VGIP_DMA_VGIP_PTN0_reg)); // Pixel 1 , G data 12bit
    val[2] = DMA_VGIP_DMA_VGIP_PTN1_get_dma_captst_b_data(hdmi_in(DMA_VGIP_DMA_VGIP_PTN1_reg)); // Pixel 1 , B data 12bit
#else
    val[0] = VGIP_VGIP_CHN1_PTN0_get_ch1_captst_r_data(hdmi_in(VGIP_VGIP_CHN1_PTN0_reg));  // Pixel 1 , R data 12bit
    val[1] = VGIP_VGIP_CHN1_PTN0_get_ch1_captst_g_data(hdmi_in(VGIP_VGIP_CHN1_PTN0_reg));  // Pixel 1 , G data 12bit
    val[2] = VGIP_VGIP_CHN1_PTN1_get_ch1_captst_b_data(hdmi_in(VGIP_VGIP_CHN1_PTN1_reg));  // Pixel 1 , B data 12bit
#endif
}

//------------------------------------------------------------------
// Debug Functios
//------------------------------------------------------------------

unsigned char lib_hdmi_get_mac_h_sync(unsigned char nport)
{
    unsigned char timeout_count = 5;
    unsigned char is_hsync = HDMI_TMDS_CTRL_get_ho(hdmi_in(HDMI_TMDS_CTRL_reg));


    if (newbase_hdmi_get_fva_factor(nport))
    {//might take longer to get hSync in FVA mode with 980B.
        timeout_count += GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MEASURE_HSYNC_EXTRA_TIMEOUT_CNT);
    }
    hdmi_mask(HDMI_TMDS_CTRL_reg, ~HDMI_TMDS_CTRL_ho_mask, HDMI_TMDS_CTRL_ho_mask); //write 1 clear
    return is_hsync;
}

void lib_hdmi_wait_hdcp_vs(unsigned char nport, unsigned char cnt, unsigned int timeout)
{
    int vs_rise = 0;
    int vs_pre = 1;
    int vsc = 0;
    int vs_inv = HDMI_HDCP_CR_get_ivsp(hdmi_in(HDMI_HDCP_CR_reg));

    HDMI_INFO("lib_hdmi_wait_hdcp_vs !!!!, port=%d, cnt=%d, timeout=%d ms, vs_inv=%d\n", nport, cnt, timeout, vs_inv);

    timeout *= 100;

    // setup debug port
    hdmi_mask(HDMI_TMDS_UDC_reg, ~HDMI_TMDS_UDC_debug_sel_mask, HDMI_TMDS_UDC_debug_sel(5));
    hdmi_mask(HDMI_HDMI_CTS_FIFO_CTL_reg,
              ~HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel_mask,
              HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel(1));

    // check vsync rising
    while(vsc < cnt && timeout--)
    {
        int vs = HDMI_HDMI_debug_port_get_vs(hdmi_in(HDMI_debug_port_reg));

        vs ^= vs_inv;  // check with vs polarity

        if (vs==0)
        {
            vsc = 0;     // reset vs counter
            vs_rise = 0; // reset rising edge
        }
        else
        {
            if (vs_pre==0)
                vs_rise = 1;  // rising edge detected

            if (vs_rise)
                vsc++;
        }

        vs_pre = vs;
        udelay(10);
    }
}

unsigned int lib_hdmi_get_debug_bch_err_pkt(unsigned char nport)
{
    unsigned int packet_err_status = 0;
    hdmi_mask(HDMI_TMDS_UDC_reg, ~HDMI_TMDS_UDC_debug_sel_mask, HDMI_TMDS_UDC_debug_sel(0x4));
    hdmi_mask(HDMI_HDMI_CTS_FIFO_CTL_reg, ~HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel_mask, HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_test_sel(0x1));
    packet_err_status = HDMI_HDMI_debug_port_get_bch_err_pkt(hdmi_in(HDMI_debug_port_reg));

    return packet_err_status;
}

void lib_hdmi_dbg_reg_dump(unsigned char nport)
{

    #define AUDIO_REG_DBG 1
    #define VIDEO_REG_DBG 1
    #define HDCP_REG_DBG 1

    HDMI_EMG("===================Port%d Dump Reg Start===================\n",nport);

#if AUDIO_REG_DBG
    lib_hdmi_audio_dbg_reg_dump(nport);
#endif

#if VIDEO_REG_DBG

    HDMI_EMG("###################TMDS Channel Status Begin###################\n");
    HDMI_EMG("TMDS_CTRL=%x TMDS_OUTCTL=%x \n",hdmi_in(HDMI_TMDS_CTRL_reg),hdmi_in(HDMI_TMDS_OUTCTL_reg));
    HDMI_EMG("###################TMDS Channel Status End###################\n");


    HDMI_EMG("###################Deep Color Status Begin###################\n");
    HDMI_EMG("DPC_SET2=%x DPC_SET3=%x \n",hdmi_in(HDMI_TMDS_DPC_SET2_reg),hdmi_in(HDMI_TMDS_DPC_SET3_reg));
    HDMI_EMG("###################Deep Color Status End###################\n");


    HDMI_EMG("###################Video Data Error Detect Begin###################\n");
    HDMI_EMG("BIT_ERR_B=%x BIT_ERR_G=%x BIT_ERR_R=%x \n",hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_B_reg),hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_G_reg), hdmi_in(HDMI_VIDEO_BIT_ERR_STATUS_R_reg));
    HDMI_EMG("TREC4_ERR_B=%x TREC4_ERR_G=%x TREC4_ERR_R=%x \n",hdmi_in(HDMI_TERC4_ERR_STATUS_B_reg),hdmi_in(HDMI_TERC4_ERR_STATUS_G_reg), hdmi_in(HDMI_TERC4_ERR_STATUS_R_reg));
    HDMI_EMG("BCH_ERR=%x \n", hdmi_in(HDMI_HDMI_BCHCR_reg));
    HDMI_EMG("###################Video Data Error Detect End###################\n");


    HDMI_EMG("################### Video Control Register Begin###################\n");
    HDMI_EMG("VCR=%x \n",hdmi_in(HDMI_HDMI_VCR_reg));
    HDMI_EMG("###################Video Control Register End###################\n");

    HDMI_EMG("###################Guard Band Error Status Begin###################\n");
    HDMI_EMG("BCSR=%x \n", hdmi_in(HDMI_HDMI_BCSR_reg));
    HDMI_EMG("###################Guard Band Error Status End###################\n");


    HDMI_EMG("###################PRBS Status Begin###################\n");
    HDMI_EMG("PRBS_R=%x PRBS_G=%x PRBS_B=%x\n",hdmi_in(HDMI_PRBS_R_CTRL_reg),hdmi_in(HDMI_PRBS_G_CTRL_reg),hdmi_in(HDMI_PRBS_B_CTRL_reg));
    HDMI_EMG("###################PRBS Status End###################\n");


    HDMI_EMG("###################Asynchronous Fifo Status Begin###################\n");
    HDMI_EMG("FIFO_SR0=%x FIFO_SR1=%x\n",hdmi_in(HDMI_PHY_FIFO_SR0_reg),hdmi_in(HDMI_PHY_FIFO_SR1_reg));
    HDMI_EMG("CHSR=%x\n",hdmi_in(HDMI_CH_SR_reg));
    HDMI_EMG("###################PHY Asynchronous Fifo Status End###################\n");


    HDMI_EMG("###################Asynchronous Fifo Status Begin###################\n");
    HDMI_EMG("FIFO_SR0=%x FIFO_SR1=%x\n",hdmi_in(HDMI_PHY_FIFO_SR0_reg),hdmi_in(HDMI_PHY_FIFO_SR1_reg));
    HDMI_EMG("CHSR=%x\n",hdmi_in(HDMI_CH_SR_reg));
    HDMI_EMG("###################PHY Asynchronous Fifo Status End###################\n");

    HDMI_EMG("###################Scramble Status Begin###################\n");
    HDMI_EMG("SCR_CR=%x\n",hdmi_in(HDMI_SCR_CR_reg));
    HDMI_EMG("###################Scramble Status End###################\n");


    HDMI_EMG("###################CED Status Begin###################\n");
    HDMI_EMG("CER_SR0=%x CER_SR1=%x CER_SR2=%x\n",hdmi_in(HDMI_CERSR0_reg), hdmi_in(HDMI_CERSR1_reg), hdmi_in(HDMI_CERSR2_reg));
    HDMI_EMG("CER_SR3=%x CER_SR4=%x\n",hdmi_in(HDMI_CERSR3_reg), hdmi_in(HDMI_CERSR4_reg));
    HDMI_EMG("###################CED Statuss End###################\n");


    HDMI_EMG("###################YUV420 FIFO Status Begin###################\n");
    HDMI_EMG("YUV420_CR=%x\n",hdmi_in(HDMI_YUV420_CR_reg));
    HDMI_EMG("###################YUV420 FIFO Status End###################\n");

#endif


#if HDCP_REG_DBG
    HDMI_EMG("###################HDCP Status Begin###################\n");
    HDMI_EMG("PCR=%x\n",hdmi_in(HDMI_HDCP_PCR_reg));  //check ENC
    HDMI_EMG("HDCP_FLAG1=%x\n",hdmi_in(HDMI_HDCP_FLAG1_reg));
    HDMI_EMG("HDCP2P2_SR0=%x\n",hdmi_in(HDMI_HDCP_2p2_SR0_reg));
    HDMI_EMG("###################HDCP Status End###################\n");
#endif


    HDMI_EMG("===================Port%d Dump Reg End===================\n",nport);

}


