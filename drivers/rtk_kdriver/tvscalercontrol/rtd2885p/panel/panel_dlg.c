#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <rtk_kdriver/io.h>
#include <generated/autoconf.h>
#else
#include <timer.h>
#include <string.h>
#endif
#include <tvscalercontrol/panel/panel_dlg.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/sfg_reg.h>
#include <rbus/osdovl_reg.h>
#include <panelConfigParameter.h>
#include <rtd_log/rtd_module_log.h>
#include <mach/platform.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);

#ifdef CONFIG_DYNAMIC_PANEL_SELECT

extern platform_info_t platform_info;
static unsigned char s_vbeDlgEnable = 0;
static PANEL_DLG_MODE s_curDlgMode = PANEL_DLG_DEFAULT;
static DLG_CALLBACK dlgCallbackFunc = NULL;
static PANEL_DLG_MODE s_panelDlgMode = PANEL_DLG_DEFAULT;
static unsigned int print_dlg_message_cnt = 0;

/**
** if vrr off, but device also send vrr info, hsr off, then we can not run vrr flow
*/
unsigned char vbe_get_panel_vrr_freesync_timing_is_valid(void)
{
    MEASURE_TIMING_T  vfehdmitiminginfo = {0};
    unsigned int panelSupportMaxFrameRate = 0;
    static int printCnt = 0;

    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
    {
        drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);

        panelSupportMaxFrameRate = Get_DISPLAY_REFRESH_RATE() * 1000; //0.001hz

        //if input timing over range of panel panelSupportMaxFrameRate, not run vrr
        if(vfehdmitiminginfo.run_vrr && (vfehdmitiminginfo.v_freq > panelSupportMaxFrameRate) && ((vfehdmitiminginfo.v_freq - panelSupportMaxFrameRate) > 100))
        {
            if(printCnt == 0)
            {
                printCnt = 200;
                rtd_pr_vbe_err("[Error]hsr panel, vrr timing overrange panel can support max frameRate, so go non vrr flow input timing(%dHz), panel support max framerate(%dHz)\n", vfehdmitiminginfo.v_freq / 1000, panelSupportMaxFrameRate / 1000);
            }

            printCnt--;

            return 0;
        }
    }

    return 1;
}

unsigned char vbe_get_dlg_support_all_timing(void)
{
    if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K60_4K1K120)
    {
        return 1;
    }

    return 0;
}

void vbe_set_cur_dlg_mode(PANEL_DLG_MODE dlgMode)
{
    s_curDlgMode = dlgMode;
}

PANEL_DLG_MODE vbe_get_cur_dlg_mode(void)
{
    return s_curDlgMode;
}

void HAL_VBE_Panel_Set_DlgEnable(unsigned char dlgEnable)
{
    s_vbeDlgEnable = dlgEnable;
}

unsigned char HAL_VBE_Panel_Get_DlgEnable(void)
{
    return s_vbeDlgEnable;
}

void vbe_dynamic_set_dlgmode(PANEL_DLG_MODE dlgMode)
{
    s_panelDlgMode = dlgMode;
}

unsigned char vbe_check_dlg_mode_change(void)
{
    PANEL_DLG_MODE newDlgMode = PANEL_DLG_DEFAULT;

    newDlgMode = vbe_get_panel_dlg_mode();

    if(vbe_get_cur_dlg_mode() == newDlgMode)
    {
        return 0;
    }

    //rtd_pr_vbe_info("[%s %d]change dlgMode %d -> %d\n", __FUNCTION__, __LINE__, vbe_get_cur_dlg_mode(), newDlgMode);

    return 1;
}

PANEL_DLG_MODE vbe_get_panel_dlg_mode(void)
{
    return vbe_get_timing_support_dlg(Get_DISPLAY_HSR_MODE(), Get_DISPLAY_PANEL_CUSTOM_INDEX());
}

void vbe_set_panel_dlg_mode(PANEL_DLG_MODE panelDlgMode)
{
    vbe_set_cur_dlg_mode(panelDlgMode);
    panel_dynamic_select_setting(panelDlgMode);
}

#if 0
static void vbe_set_dpll_clk(DPLL_TYPE dpllType)
{
    if(dpllType == DPLL_594M)
    {
        // DCLK = 594MHz
        rtd_outl(0xb80006A0, 0x00000100);
        rtd_outl(0xb80006A4, 0x00000000);
        rtd_outl(0xb80006C0, 0x03f01550);
        rtd_outl(0xb80006C4, 0x00000000);
        rtd_maskl(0xb80006C8, 0xFFFFCFFF, 0x00000000);
        rtd_outl(0xb80006A0, 0x00000101);
        usleep_range(1000, 2000);
        rtd_outl(0xb80006A0, 0x00000100);
    }
    else if(dpllType == DPLL_686_88M)
    {
        // DCLK = 686.88MHz
        rtd_outl(0xb80006A0, 0x028F2000);
        rtd_outl(0xb80006A4, 0x00000000);
        rtd_outl(0xb80006C0, 0x0490155A);
        rtd_outl(0xb80006C4, 0x00000000);
        rtd_maskl(0xb80006C8, 0xFFFFCFFF, 0x00000000);
        rtd_outl(0xb80006A0, 0x028F2001);
        usleep_range(1000, 2000);
        rtd_outl(0xb80006A0, 0x028F2000);
    }
    else
    {
        // DCLK = 594MHz
        rtd_outl(0xb80006A0, 0x00000100);
        rtd_outl(0xb80006A4, 0x00000000);
        rtd_outl(0xb80006C0, 0x03f01550);
        rtd_outl(0xb80006C4, 0x00000000);
        rtd_maskl(0xb80006C8, 0xFFFFCFFF, 0x00000000);
        rtd_outl(0xb80006A0, 0x00000101);
        usleep_range(1000, 2000);
        rtd_outl(0xb80006A0, 0x00000100);
    }
}
#endif

void vbe_set_hsr_pin(unsigned char enable)
{
    if(enable)
    {
        rtd_pr_vbe_info("[%s %d]enable:%d\n", __FUNCTION__, __LINE__, enable);
        //HSR_EN Pin Control,GPIO_23,E7 enable hsr
        rtd_maskl(0xb8000814, 0xFFFFFF0F, 0x000000f0);
        rtd_maskl(0xb801bd00, 0xFEFFFFFE, 0x01000001);
        rtd_maskl(0xb801bd18, 0xFEFFFFFE, 0x01000001);
    }
    else
    {
        rtd_pr_vbe_info("[%s %d]enable:%d\n", __FUNCTION__, __LINE__, enable);
        //HSR_EN Pin Control,GPIO_23,E7
        rtd_maskl(0xb8000814, 0xFFFFFF0F, 0x000000f0);
        rtd_maskl(0xb801bd00, 0xFEFFFFFE, 0x01000001);
        rtd_maskl(0xb801bd18, 0xFEFFFFFE, 0x01000001);
        rtd_maskl(0xb801bd18, 0xFEFFFFFE, 0x01000000);
    }
}

void panel_vby1_4k1k240_tx(void)
{
    rtd_pr_vbe_info("[VBE]panel_vby1_4k1k240_tx\n");

    vbe_set_hsr_pin(1);

    //---------------------------------------------------------------------------------------------------------------------------------
    //SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb802D950, 0x044C0014);	// 4K1K 240Hz change 0x089d002d 0x044C0014 DV_Den_end,DV_Daen_sta, (follow DTG timing)
    rtd_outl(0xb802D954, 0x07E40064);	// 4K1K 240Hz change 0x080c008c 0x07E40064 DH_Den_end,DH_Den_sta, (follow DTG timing)
    rtd_outl(0xb802D958, 0x053C0847);	// 4K1K 240Hz change 0x053f0897 0x053C0847 DV_width,Dh_width,Dh_total (follow DTG timing
    rtd_outl(0xb802D9A4, 0x006407E4);	// hs_den_io_dist, hs_io_dist,
    rtd_outl(0xb802DBF0, 0x0EF08700);	// 4K1K 240Hz change SFG Pattern Gen. Disable, RGB Mode, Width = 480, Hight = 270, pattern reset by hs,

    //---------------------------------------------------------------------------------------------------------------------------------
    //PIF_APHY
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
    //
    rtd_outl(0xb8000D00, 0x66002838);	// Lane0 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D10, 0x66002838);	// Lane1 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D20, 0x66002838);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D30, 0x66002838);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D40, 0x66002838);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D50, 0x66002838);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D60, 0x66002838);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D70, 0x66002838);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D80, 0x66002838);	// Lane8 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D90, 0x66002838);	// Lane9 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DA0, 0x66002838);	// Lane10: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DB0, 0x66002838);	// Lane11: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DC0, 0x66002838);	// Lane12: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DD0, 0x66002838);	// Lane13: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DE0, 0x66002838);	// Lane14: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DF0, 0x66002838);	// Lane15: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    //
    rtd_outl(0xb8000D04, 0x44003006);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D14, 0x44003006);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D24, 0x44003006);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D34, 0x44003006);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D44, 0x44003006);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D54, 0x44003006);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D64, 0x44003006);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D74, 0x44003006);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D84, 0x44003006);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D94, 0x44003006);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DA4, 0x44003006);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DB4, 0x44003006);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DC4, 0x44003006);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DD4, 0x44003006);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DE4, 0x44003006);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DF4, 0x44003006);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    //
    rtd_outl(0xb8000DA8, 0x00000079);	// Lane10: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
    rtd_outl(0xb8000DB8, 0x000001a9);	// Lane11: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),

    msleep(10);

#if 0
    if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
    {
        //4k2k144 dpll 686M change to 4k2k120, need update dpll 594M
        vbe_set_dpll_clk(DPLL_594M);
    }
#endif
    // ------------------------ OSD DTG 2K2K ------------------------
    rtd_outl(0xb8028700, 0x80000001);	// osd-pif, frc, enable
    rtd_outl(0xb8028704, 0x00000464);	// osd_dtg dv_total
    rtd_outl(0xb8028708, 0x112f112f);	// osd_dtg dh_total
    rtd_outl(0xb802871C, 0x01181018);	// osd_dtg dh_den_sta_end
    rtd_outl(0xb8028718, 0x0014044C);	// osd_dtg dv_den_sta_end

    rtd_outl(0xb8028000, 0x01000000);	// FRC Mode

    //-----------------------------DTG_timming--------------------------------
    //rtd_outl(0xb8000208, 0x00000074);		// D-Domain, SFG Clock Enable
    //-----------------------------4K120--------------------------------
    rtd_outl(0xb8000230, 0x00210000);		// [22:20]:osd_f2p/4, [18:16]:osd_f1p/2, [14:12]:s2_f2p=/1, [10:8]:s2_f1p/1, [6:4]:s1_f2p=/1, [2:0]:s1_f1p/1
    rtd_outl(0xb8000234, 0x00100111);		// [22:20]:s2_f2p_div2(mPlus)/2, [18:16]:mc/1, [14:12]:mc_ippre2x/1, [10:8]:mc_ippre/2, [6:4]:memc_out/2, [2:0]:memc_in/2
    rtd_outl(0xb802811C, 0x00000001);	 // gating apply

    //DTG
    // ----------------------- 4K120 ----------------------------
    rtd_outl(0xb802850C, 0xC0000000);	// 4K120 mode, two_step_uzu

    // ----------------------- Master DTG -----------------------
    rtd_outl(0xb8028004, 0x00060008);	// D-Domain 1-Pixel Mode
    rtd_outl(0xb8028008, 0x00000078);	// Hsync width
    rtd_outl(0xb802800C, 0x112f112f);	// DH_total
    rtd_outl(0xb8028010, 0x00000005);	// DV_sync width
    rtd_outl(0xb8028014, 0x00000464);	// DV_total
    rtd_outl(0xb8028020, 0x000000FF);	// Display Background Color (Green/Blue)
    rtd_outl(0xb8028024, 0x00000000);	// Display Background Color (Red)

    // ------------------------ UZU DTG -------------------------
    rtd_outl(0xb8028504, 0x00000464);	// uzudtg dv_total
    rtd_outl(0xb8028508, 0x112f112f);	// uzudtg dh_total
    rtd_outl(0xb8028518, 0x01181018);	// uzudtg dh_den_sta_end
    rtd_outl(0xb802851c, 0x0014044C);	// uzudtg dv_den_sta_end

    // -------------------------- MTG ---------------------------
    rtd_outl(0xb8028308, 0x11100000);	// Main Border Color, m_bd_b, m_hbd_r
    rtd_outl(0xb802830C, 0x33303774);	// Main Border Color, m_bd_r, m_bd_g
    rtd_outl(0xb8028310, 0x00000000);	// Main Background Color, m_bg_g, m_bg_b
    rtd_outl(0xb8028314, 0x00000000);	// Main Background Color, m_bg_r
    rtd_outl(0xb8028540, 0x00000F00);	// Main DEN_H_str,Main DEN_H_end
    rtd_outl(0xb8028544, 0x00000438);	// Main DEN_V_str,Main DEN_V_end
    rtd_outl(0xb8028548, 0x00000F00);	// Main Background_H_str,Main Background_H_end
    rtd_outl(0xb802854C, 0x00000438);	// Main Background_V_str,Main Background_V_end
    rtd_outl(0xb8028550, 0x00000F00);	// Main Active_H_str,Main Active_H_end
    rtd_outl(0xb8028554, 0x00000438);	// Main Active_V_str,Main Active_V_end

    // ------------------------ MEMC DTG ------------------------
    rtd_outl(0xb8028604, 0x00000464);	// memc_dtg dv_total
    rtd_outl(0xb8028608, 0x112f112f);	// memc_dtg dh_total
    rtd_outl(0xb802861C, 0x01181018);	// memc_dtg dh_den_sta_end
    rtd_outl(0xb8028618, 0x0014044C);	// memc_dtg dv_den_sta_end


    //UZU
    //-----------------------------4K120--------------------------------
    // 2step UZU
    rtd_outl(0xb8029B2C, 0x0F000438);		// Input Size (3840x2160)
    rtd_outl(0xb8029B20, 0x802a8000);		// Input Size (3840x2160)

    rtd_part_outl(0xb802CA00, 10,10, 0x1);	 // OD_4K120 mode
}

void panel_vby1_4k2k120_tx(void)
{
    rtd_pr_vbe_info("=== panel_vby1_4k2k120_tx ===\n");

    vbe_set_hsr_pin(0);

    //---------------------------------------------------------------------------------------------------------------------------------
    //SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb802D950, 0x089D002D);	// DV_Den_end,DV_Den_sta, (follow DTG timing)
    rtd_outl(0xb802D954, 0x080C008C);	// DH_Den_end,DH_Den_sta, (follow DTG timing)
    rtd_outl(0xb802D958, 0x053F0897);	// DV_width,Dh_width,Dh_total (follow DTG timing)

    rtd_outl(0xb802D9A4, 0x008C0894);	// hs_den_io_dist, hs_io_dist,

    rtd_outl(0xb802DBF0, 0x0F010E00);	// SFG Pattern Gen. Disable, RGB Mode, Width = 240, Hight = 270, pattern reset by hs,
    //---------------------------------------------------------------------------------------------------------------------------------
    //PIF_APHY
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
    //
    rtd_outl(0xb8000D00, 0x6600233f);	// Lane0 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D10, 0x6600233f);	// Lane1 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D20, 0x6600233f);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D30, 0x6600233f);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D40, 0x6600233f);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D50, 0x6600233f);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D60, 0x6600233f);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D70, 0x6600233f);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D80, 0x6600233f);	// Lane8 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D90, 0x6600233f);	// Lane9 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DA0, 0x6600233f);	// Lane10: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DB0, 0x6600233f);	// Lane11: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DC0, 0x6600233f);	// Lane12: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DD0, 0x6600233f);	// Lane13: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DE0, 0x6600233f);	// Lane14: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DF0, 0x6600233f);	// Lane15: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    //
    rtd_outl(0xb8000D04, 0x4400300f);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D14, 0x4400300f);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D24, 0x4400300f);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D34, 0x4400300f);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D44, 0x4400300f);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D54, 0x4400300f);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D64, 0x4400300f);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D74, 0x4400300f);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D84, 0x4400300f);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D94, 0x4400300f);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DA4, 0x4400300f);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DB4, 0x4400300f);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DC4, 0x4400300f);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DD4, 0x4400300f);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DE4, 0x4400300f);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DF4, 0x4400300f);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    //
    rtd_outl(0xb8000DA8, 0x00000079);	// Lane10: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE), Sync TXI, VCM, Down_Shift_0.3V from 0x18000C50
    rtd_outl(0xb8000DB8, 0x000001A9);	// Lane11: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE), Sync TXI, VCM, Down_Shift_0.3V from 0x18000C50

#if 0
    if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
    {
        //4k2k144 dpll 686M change to 4k2k120, need update dpll 594M
        vbe_set_dpll_clk(DPLL_594M);
    }
#endif
    // ------------------------ OSD DTG 2K2K ------------------------
    rtd_outl(0xb8028700, 0x90000050);	// osd-pif, frc, enable
    rtd_outl(0xb8028704, 0x00000000);	// osd_dtg dv_total
    rtd_outl(0xb8028708, 0x00000000);	// osd_dtg dh_total
    rtd_outl(0xb802871C, 0x00000000);	// osd_dtg dh_den_sta_end
    rtd_outl(0xb8028718, 0x00000000);	// osd_dtg dv_den_sta_end
    rtd_outl(0xb8028000, 0x01000000);	// FRC Mode

    //-----------------------------DTG_timming--------------------------------
    //rtd_outl(0xb8000208, 0x00000074);		// D-Domain, SFG Clock Enable
    //-----------------------------4K120--------------------------------
    rtd_outl(0xb8000230, 0x00210000);		// [22:20]:osd_f2p/4, [18:16]:osd_f1p/2, [14:12]:s2_f2p=/1, [10:8]:s2_f1p/1, [6:4]:s1_f2p=/1, [2:0]:s1_f1p/1
    rtd_outl(0xb8000234, 0x00100111);		// [22:20]:s2_f2p_div2(mPlus)/2, [18:16]:mc/1, [14:12]:mc_ippre2x/1, [10:8]:mc_ippre/2, [6:4]:memc_out/2, [2:0]:memc_in/2
    rtd_outl(0xb802811C, 0x00000001);	 // gating apply

    //DTG
    // ----------------------- 4K120 ----------------------------
    rtd_outl(0xb802850C, 0xc0770042);	// 4K120 mode, two_step_uzu

    // ----------------------- Master DTG -----------------------
    rtd_outl(0xb8028004, 0x00060008);	// D-Domain 1-Pixel Mode
    rtd_outl(0xb8028008, 0x0000000f);	// Hsync width
    rtd_outl(0xb802800C, 0x112f112f);	// DH_total
    rtd_outl(0xb8028010, 0x00000005);	// DV_sync width
    rtd_outl(0xb8028014, 0x000008c9);	// DV_total
    rtd_outl(0xb8028020, 0x00000000);	// Display Background Color (Green/Blue)
    rtd_outl(0xb8028024, 0x00000000);	// Display Background Color (Red)

    // ------------------------ UZU DTG -------------------------
    rtd_outl(0xb8028504, 0x000008c9);	// uzudtg dv_total
    rtd_outl(0xb8028508, 0x112f112f);	// uzudtg dh_total
    rtd_outl(0xb8028518, 0x01181018);	// uzudtg dh_den_sta_end
    rtd_outl(0xb802851c, 0x002d089d);	// uzudtg dv_den_sta_end

    // -------------------------- MTG ---------------------------
    rtd_outl(0xb8028308, 0x00000000);	// Main Border Color, m_bd_b, m_hbd_r
    rtd_outl(0xb802830C, 0x00000000);	// Main Border Color, m_bd_r, m_bd_g
    rtd_outl(0xb8028310, 0x00000000);	// Main Background Color, m_bg_g, m_bg_b
    rtd_outl(0xb8028314, 0x00000000);	// Main Background Color, m_bg_r
    rtd_outl(0xb8028540, 0x00000F00);	// Main DEN_H_str,Main DEN_H_end
    rtd_outl(0xb8028544, 0x00000870);	// Main DEN_V_str,Main DEN_V_end
    rtd_outl(0xb8028548, 0x00000F00);	// Main Background_H_str,Main Background_H_end
    rtd_outl(0xb802854C, 0x00000870);	// Main Background_V_str,Main Background_V_end
    rtd_outl(0xb8028550, 0x00000F00);	// Main Active_H_str,Main Active_H_end
    rtd_outl(0xb8028554, 0x00000870);	// Main Active_V_str,Main Active_V_end

    // ------------------------ MEMC DTG ------------------------
    rtd_outl(0xb8028604, 0x000008c9);	// memc_dtg dv_total
    rtd_outl(0xb8028608, 0x112f112f);	// memc_dtg dh_total
    rtd_outl(0xb802861C, 0x01181018);	// memc_dtg dh_den_sta_end
    rtd_outl(0xb8028618, 0x002d089d);	// memc_dtg dv_den_sta_end

    //UZU
    //-----------------------------4K120--------------------------------
    // 2step UZU
    rtd_outl(0xb8029B2C, 0x00000000);		// Input Size (3840x2160)
    rtd_outl(0xb8029B20, 0x002a8030);		// Input Size (3840x2160)

    rtd_part_outl(0xb802CA00, 10,10, 0x1);	 // OD_4K120 mode
}

void panel_vby1_4k1k288_tx(void)
{
    rtd_pr_vbe_info("=== panel_vby1_4k1k288_tx ===\n");

    vbe_set_hsr_pin(1);

    //---------------------------------------------------------------------------------------------------------------------------------
    //SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb802D950, 0x044C0014);	// 4K1K 288Hz change 0x089d002d 0x044C0014 DV_Den_end,DV_Daen_sta, (follow DTG timing)
    rtd_outl(0xb802D954, 0x07E40064);	// 4K1K 288Hz change 0x080c008c 0x07E40064 DH_Den_end,DH_Den_sta, (follow DTG timing)
    rtd_outl(0xb802D958, 0x053C0847);	// 4K1K 288Hz change 0x053f0897 0x053C0847 DV_width,Dh_width,Dh_total (follow DTG timing)

    rtd_outl(0xb802D9A4, 0x006407E4);	// hs_den_io_dist, hs_io_dist,

    rtd_outl(0xb802DBF0, 0x0EF08700);	// 4K1K 288Hz change SFG Pattern Gen. Disable, RGB Mode, Width = 480, Hight = 270, pattern reset by hs,
    //---------------------------------------------------------------------------------------------------------------------------------
    //PIF_APHY
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
    //
    rtd_outl(0xb8000D00, 0x6600233f);	// Lane0 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D10, 0x6600233f);	// Lane1 : Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D20, 0x6600233f);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D30, 0x6600233f);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D40, 0x6600233f);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D50, 0x6600233f);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D60, 0x6600233f);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D70, 0x6600233f);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D80, 0x6600233f);	// Lane8 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D90, 0x6600233f);	// Lane9 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DA0, 0x6600233f);	// Lane10: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DB0, 0x6600233f);	// Lane11: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DC0, 0x6600233f);	// Lane12: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DD0, 0x6600233f);	// Lane13: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DE0, 0x6600233f);	// Lane14: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DF0, 0x6600233f);	// Lane15: Hi-Z Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    //
    rtd_outl(0xb8000D04, 0x4400300f);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D14, 0x4400300f);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D24, 0x4400300f);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D34, 0x4400300f);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D44, 0x4400300f);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D54, 0x4400300f);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D64, 0x4400300f);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D74, 0x4400300f);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D84, 0x4400300f);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D94, 0x4400300f);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DA4, 0x4400300f);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DB4, 0x4400300f);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DC4, 0x4400300f);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DD4, 0x4400300f);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DE4, 0x4400300f);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DF4, 0x4400300f);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    //
    rtd_outl(0xb8000DA8, 0x00000079);	// Lane10: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),
    rtd_outl(0xb8000DB8, 0x000001a9);	// Lane11: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE),

    msleep(10);

    // ------------------------ OSD DTG 2K2K ------------------------
    rtd_outl(0xb8028700, 0x80000001);	// osd-pif, frc, enable
    rtd_outl(0xb8028704, 0x00000464);	// osd_dtg dv_total
    rtd_outl(0xb8028708, 0x108F108F);	// osd_dtg dh_total
    rtd_outl(0xb802871C, 0x00C80FC8);	// osd_dtg dh_den_sta_end
    rtd_outl(0xb8028718, 0x0014044C);	// osd_dtg dv_den_sta_end
    rtd_outl(0xb8028000, 0x01000000);	// FRC Mode

    // DCLK = 686.88MHz
    //vbe_set_dpll_clk(DPLL_686_88M);

    //-----------------------------DTG_timming--------------------------------
    //-----------------------------4K120--------------------------------
    rtd_outl(0xb8000230, 0x00210000);		// [22:20]:osd_f2p/4, [18:16]:osd_f1p/2, [14:12]:s2_f2p=/1, [10:8]:s2_f1p/1, [6:4]:s1_f2p=/1, [2:0]:s1_f1p/1
    rtd_outl(0xb8000234, 0x00100111);		// [22:20]:s2_f2p_div2(mPlus)/2, [18:16]:mc/1, [14:12]:mc_ippre2x/1, [10:8]:mc_ippre/2, [6:4]:memc_out/2, [2:0]:memc_in/2
    rtd_outl(0xb802811C, 0x00000001);	 // gating apply

    //DTG
    // ----------------------- 4K120 ----------------------------
    rtd_outl(0xb802850C, 0xC0000000);	// 4K120 mode, two_step_uzu

    // ----------------------- Master DTG -----------------------
    rtd_outl(0xb8028004, 0x00060008);	// D-Domain 1-Pixel Mode
    rtd_outl(0xb8028008, 0x0000007f);	// Hsync width
    rtd_outl(0xb802800C, 0x108F108F);	// DH_total
    rtd_outl(0xb8028010, 0x00000005);	// DV_sync width
    rtd_outl(0xb8028014, 0x00000464);	// DV_total
    rtd_outl(0xb8028020, 0x000000FF);	// Display Background Color (Green/Blue)
    rtd_outl(0xb8028024, 0x00000000);	// Display Background Color (Red)

    // ------------------------ UZU DTG -------------------------
    rtd_outl(0xb8028504, 0x00000464);	// uzudtg dv_total
    rtd_outl(0xb8028508, 0x108F108F);	// uzudtg dh_total
    rtd_outl(0xb8028518, 0x00C80FC8);	// uzudtg dh_den_sta_end
    rtd_outl(0xb802851c, 0x0014044C);	// uzudtg dv_den_sta_end

    // -------------------------- MTG ---------------------------
    rtd_outl(0xb8028308, 0x11100000);	// Main Border Color, m_bd_b, m_hbd_r
    rtd_outl(0xb802830C, 0x33303774);	// Main Border Color, m_bd_r, m_bd_g
    rtd_outl(0xb8028310, 0x00000000);	// Main Background Color, m_bg_g, m_bg_b
    rtd_outl(0xb8028314, 0x00000000);	// Main Background Color, m_bg_r
    rtd_outl(0xb8028540, 0x00000F00);	// Main DEN_H_str,Main DEN_H_end
    rtd_outl(0xb8028544, 0x00000438);	// Main DEN_V_str,Main DEN_V_end
    rtd_outl(0xb8028548, 0x00000F00);	// Main Background_H_str,Main Background_H_end
    rtd_outl(0xb802854C, 0x00000438);	// Main Background_V_str,Main Background_V_end
    rtd_outl(0xb8028550, 0x00000F00);	// Main Active_H_str,Main Active_H_end
    rtd_outl(0xb8028554, 0x00000438);	// Main Active_V_str,Main Active_V_end

    // ------------------------ MEMC DTG ------------------------
    rtd_outl(0xb8028604, 0x00000464);	// memc_dtg dv_total
    rtd_outl(0xb8028608, 0x108F108F);	// memc_dtg dh_total
    rtd_outl(0xb802861C, 0x00C80FC8);	// memc_dtg dh_den_sta_end
    rtd_outl(0xb8028618, 0x0014044C);	// memc_dtg dv_den_sta_end

    //UZU
    //-----------------------------4K120--------------------------------
    // 2step UZU
    rtd_outl(0xb8029B2C, 0x0F000438);		// Input Size (3840x2160)
    rtd_outl(0xb8029B20, 0x802a8000);		// Input Size (3840x2160)

    rtd_part_outl(0xb802CA00, 10,10, 0x1);	 // OD_4K120 mode
		//-----------------------------timmingEnd--------------------------------
}

void panel_vby1_4k2k144_tx(void)
{
    rtd_pr_vbe_info("=== panel_vby1_4k2k144_tx ===\n");

    vbe_set_hsr_pin(0);

    //---------------------------------------------------------------------------------------------------------------------------------
    //SFG: DH_ST_END = 0x01181018, DTG_HS_Width = 32, DTG_VS_Width = 12
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb802D950, 0x089d002d);	// DV_Den_end,DV_Daen_sta, (follow DTG timing)
    rtd_outl(0xb802D954, 0x07E40064);	// DH_Den_end,DH_Den_sta, (follow DTG timing)
    rtd_outl(0xb802D958, 0x053f0847);	// DV_width,Dh_width,Dh_total (follow DTG timing)

    rtd_outl(0xb802D9A4, 0x006407E4);	//0x008C0890 hs_den_io_dist, hs_io_dist,

    rtd_outl(0xb802DBF0, 0x0EF10E00);	// SFG Pattern Gen. Disable, RGB Mode, Width = 480, Hight = 270, pattern reset by hs,		
    //---------------------------------------------------------------------------------------------------------------------------------
    //PIF_APHY
    //---------------------------------------------------------------------------------------------------------------------------------
    rtd_outl(0xb8000C50, 0x01F48000);	// IBN Current, VCM Level, LDO1V_EN(1), VCM_SHIFT_DOWN(0),
    //
    rtd_outl(0xb8000D00, 0x6600233f);	// Lane0 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D10, 0x6600233f);	// Lane1 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D20, 0x6600233f);	// Lane2 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D30, 0x6600233f);	// Lane3 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D40, 0x6600233f);	// Lane4 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D50, 0x6600233f);	// Lane5 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D60, 0x6600233f);	// Lane6 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D70, 0x6600233f);	// Lane7 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D80, 0x6600233f);	// Lane8 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000D90, 0x6600233f);	// Lane9 : HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DA0, 0x6600233f);	// Lane10: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DB0, 0x6600233f);	// Lane11: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DC0, 0x6600233f);	// Lane12: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DD0, 0x6600233f);	// Lane13: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DE0, 0x6600233f);	// Lane14: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    rtd_outl(0xb8000DF0, 0x6600233f);	// Lane15: HS Mode, PI_SEL(PI_DM), SR_SEL(0), TERM_SEL, IDRV_SEL(15),
    //
    rtd_outl(0xb8000D04, 0x4400300f);	// Lane0 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D14, 0x4400300f);	// Lane1 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D24, 0x4400300f);	// Lane2 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D34, 0x4400300f);	// Lane3 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D44, 0x4400300f);	// Lane4 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D54, 0x4400300f);	// Lane5 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D64, 0x4400300f);	// Lane6 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D74, 0x4400300f);	// Lane7 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D84, 0x4400300f);	// Lane8 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000D94, 0x4400300f);	// Lane9 : CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DA4, 0x4400300f);	// Lane10: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DB4, 0x4400300f);	// Lane11: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DC4, 0x4400300f);	// Lane12: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DD4, 0x4400300f);	// Lane13: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DE4, 0x4400300f);	// Lane14: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    rtd_outl(0xb8000DF4, 0x4400300f);	// Lane15: CKMX_INV(0), RT_EN(1), RP_EN(1), CML1.2V, IEM_SEL(6),
    //
    rtd_outl(0xb8000DA8, 0x00000079);	// Lane10: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE), Sync TXI, VCM, Down_Shift_0.3V from 0x18000C50
    rtd_outl(0xb8000DB8, 0x000001A9);	// Lane11: PI_SEL(PI_DM), DIFF_PULL_LOW(0), CMFB_EN(0), DRV_MODE_SEL(CML_MODE), Sync TXI, VCM, Down_Shift_0.3V from 0x18000C50

    msleep(10);

    // ------------------------ OSD DTG 2K2K ------------------------
    rtd_outl(0xb8028700, 0x80000001);	// osd-pif, frc, enable
    rtd_outl(0xb8028704, 0x000008C9);	// osd_dtg dv_total
    rtd_outl(0xb8028708, 0x108F108F);	// osd_dtg dh_total
    rtd_outl(0xb802871C, 0x00C80FC8);	// osd_dtg dh_den_sta_end
    rtd_outl(0xb8028718, 0x002D089D);	// osd_dtg dv_den_sta_end

    rtd_outl(0xb8028000, 0x01000000);	// FRC Mode

    // DCLK = 686.88MHz
    //vbe_set_dpll_clk(DPLL_686_88M);

    //-----------------------------DTG_timming--------------------------------
    //-----------------------------4K144--------------------------------
    rtd_outl(0xb8000230, 0x00210000);		// [22:20]:osd_f2p/4, [18:16]:osd_f1p/2, [14:12]:s2_f2p=/1, [10:8]:s2_f1p/1, [6:4]:s1_f2p=/1, [2:0]:s1_f1p/1
    rtd_outl(0xb8000234, 0x00100111);		// [22:20]:s2_f2p_div2(mPlus)/2, [18:16]:mc/1, [14:12]:mc_ippre2x/1, [10:8]:mc_ippre/2, [6:4]:memc_out/2, [2:0]:memc_in/2
    rtd_outl(0xb802811C, 0x00000001);	 // gating apply

    //DTG
    // ----------------------- 4K120 ----------------------------
    rtd_outl(0xb802850C, 0xC0000000);	// 4K120 mode, two_step_uzu

    // ----------------------- Master DTG -----------------------
    rtd_outl(0xb8028004, 0x00060008);	// D-Domain 1-Pixel Mode
    rtd_outl(0xb8028008, 0x0000007F);	// Hsync width
    rtd_outl(0xb802800C, 0x108F108F);	// DH_total
    rtd_outl(0xb8028010, 0x00000005);	// DV_sync width
    rtd_outl(0xb8028014, 0x000008C9);	// DV_total
    rtd_outl(0xb8028020, 0x000000FF);	// Display Background Color (Green/Blue)
    rtd_outl(0xb8028024, 0x00000000);	// Display Background Color (Red)

    // ------------------------ UZU DTG -------------------------
    rtd_outl(0xb8028504, 0x000008C9);	// uzudtg dv_total
    rtd_outl(0xb8028508, 0x108F108F);	// uzudtg dh_total
    rtd_outl(0xb8028518, 0x00C80FC8);	// uzudtg dh_den_sta_end
    rtd_outl(0xb802851c, 0x002D089D);	// uzudtg dv_den_sta_end

    // -------------------------- MTG ---------------------------
    rtd_outl(0xb8028308, 0x11100000);	// Main Border Color, m_bd_b, m_hbd_r
    rtd_outl(0xb802830C, 0x33303774);	// Main Border Color, m_bd_r, m_bd_g
    rtd_outl(0xb8028310, 0x00000000);	// Main Background Color, m_bg_g, m_bg_b
    rtd_outl(0xb8028314, 0x00000000);	// Main Background Color, m_bg_r
    rtd_outl(0xb8028540, 0x00000F00);	// Main DEN_H_str,Main DEN_H_end
    rtd_outl(0xb8028544, 0x00000870);	// Main DEN_V_str,Main DEN_V_end
    rtd_outl(0xb8028548, 0x00000F00);	// Main Background_H_str,Main Background_H_end
    rtd_outl(0xb802854C, 0x00000870);	// Main Background_V_str,Main Background_V_end
    rtd_outl(0xb8028550, 0x00000F00);	// Main Active_H_str,Main Active_H_end
    rtd_outl(0xb8028554, 0x00000870);	// Main Active_V_str,Main Active_V_end

    // ------------------------ MEMC DTG ------------------------
    rtd_outl(0xb8028604, 0x000008C9);	// memc_dtg dv_total
    rtd_outl(0xb8028608, 0x108F108F);	// memc_dtg dh_total
    rtd_outl(0xb802861C, 0x00C80FC8);	// memc_dtg dh_den_sta_end
    rtd_outl(0xb8028618, 0x002D089D);	// memc_dtg dv_den_sta_end

    //UZU
    //-----------------------------4K120--------------------------------
    // 2step UZU
    rtd_outl(0xb8029B2C, 0x00000000);		// Input Size (3840x2160)
    rtd_outl(0xb8029B20, 0x002a8030);		// Input Size (3840x2160)

    rtd_part_outl(0xb802CA00, 10,10, 0x1);	 // OD_4K120 mode
}

void panel_vby1_dlg_4k2K120_4k1k240(PANEL_DLG_MODE dlgMode)
{
    PANEL_CONFIG_PARAMETER panelConfigParameter = {0};

    memcpy(&panelConfigParameter, (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter, sizeof(PANEL_CONFIG_PARAMETER));

    if(dlgMode == PANEL_DLG_4k1k240)
    {
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 240;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL = 1125;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_VPOS = 20;
        panelConfigParameter.iCONFIG_DISP_DEN_END_VPOS = 1100;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_VPOS = 1080;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = 1090;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = 1350;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = 1122;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = 1350;

        panel_vby1_4k1k240_tx();
    }
    else
    {
        panel_vby1_4k2k120_tx();
    }

    Panel_InitParameter(&panelConfigParameter); 
}

//sfg_down_sample
void panel_vby1_dlg_4k2K60_4k2k120(PANEL_DLG_MODE dlgMode)
{
    PANEL_CONFIG_PARAMETER panelConfigParameter = {0};

    rtd_pr_vbe_info("[%s %d]dlgMode:%d\n", __FUNCTION__, __LINE__, dlgMode);

    memcpy(&panelConfigParameter, (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter, sizeof(PANEL_CONFIG_PARAMETER));
    
    if(dlgMode == PANEL_DLG_4k1k120)
    {
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 120;

        //HSR_EN Pin Control,GPIO_23,E7 enable hsr
        vbe_set_hsr_pin(1);

        //4k2k120 clk
        rtd_outl(0xb8000230, 0x00210010);
        rtd_outl(0xb8000234, 0x00200111);
        rtd_outl(0xb802811C, 0x00000001);

        //sfg enable dowm smaple
        rtd_outl(0xb802D954, 0x080c008c);
        rtd_outl(0xb802D958, 0x053f0897);
        rtd_outl(0xb802D964, 0x00010007);
        rtd_outl(0xb802D988, 0x40000001);
        rtd_outl(0xb802D9A4, 0x008c0894);

        rtd_outl(0xb8029B2C, 0x00000000);
        rtd_outl(0xb8029B20, 0x002a8030);
        rtd_part_outl(0xb802CA00, 10,10, 0x1);
    }
    else
    {
        //update panel parameter
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 60;

        //HSR_EN Pin Control,GPIO_23,E7 disable hsr
        vbe_set_hsr_pin(0);

        //4k2k60 clk
        rtd_outl(0xb8000230, 0x00211010);
        rtd_outl(0xb8000234, 0x00210121);
        rtd_outl(0xb802811C, 0x00000001);

        //sfg disable down sample
        rtd_outl(0xb802D954, 0x10180118);
        rtd_outl(0xb802D958, 0x057f112f);
        rtd_outl(0xb802D964, 0x00000007);
        rtd_outl(0xb802D988, 0x40000000);
        rtd_outl(0xb802D9A4, 0x01181128);

        rtd_outl(0xb8029B2C, 0x00000000);
        rtd_outl(0xb8029B20, 0x002a8030);
        rtd_part_outl(0xb802CA00, 10,10, 0x0);
    }

    Panel_InitParameter(&panelConfigParameter);
}

void panel_vby1_dlg_4k2K144_4k1k288(PANEL_DLG_MODE dlgMode)
{
    PANEL_CONFIG_PARAMETER panelConfigParameter = {0};

    memcpy(&panelConfigParameter, (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter, sizeof(PANEL_CONFIG_PARAMETER));

    if(dlgMode == PANEL_DLG_4k1k288)
    {
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 288;
        panelConfigParameter.iCONFIG_DISPLAY_CLOCK_TYPICAL = 686880000;    
        panelConfigParameter.iCONFIG_DISP_HORIZONTAL_TOTAL = 4240;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL = 1125;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_HPOS = 200;
        panelConfigParameter.iCONFIG_DISP_DEN_END_HPOS = 4040;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_VPOS = 20;
        panelConfigParameter.iCONFIG_DISP_DEN_END_VPOS = 1100;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_HPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_HPOS = 3840;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_VPOS = 1080;;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = 1122;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = 1350;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = 1122;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = 1350;

        panel_vby1_4k1k288_tx();
    }
#if 0
    else if(dlgMode == PANEL_DLG_4k1k240)
    {
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 240;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL = 1125;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_VPOS = 20;
        panelConfigParameter.iCONFIG_DISP_DEN_END_VPOS = 1100;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_VPOS = 1080;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = 1090;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = 1350;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = 1122;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = 1350;

        panel_vby1_4k1k240_tx();
    }
    else if(dlgMode == PANEL_DLG_4k2k144)
    {
        panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 144;
        panelConfigParameter.iCONFIG_DISPLAY_CLOCK_TYPICAL = 686880000;    
        panelConfigParameter.iCONFIG_DISP_HORIZONTAL_TOTAL = 4240;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL = 2250;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_HPOS = 200;
        panelConfigParameter.iCONFIG_DISP_DEN_END_HPOS = 4040;
        panelConfigParameter.iCONFIG_DISP_DEN_STA_VPOS = 45;
        panelConfigParameter.iCONFIG_DISP_DEN_END_VPOS = 2205;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_HPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_HPOS = 3840;
        panelConfigParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;
        panelConfigParameter.iCONFIG_DISP_ACT_END_VPOS = 2160;;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = 2210;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = 8000;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = 2210;
        panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = 8000;

        panel_vby1_4k2k144_tx();
    }
#endif
    else
    {
        panel_vby1_4k2k144_tx();
    }

    Panel_InitParameter(&panelConfigParameter);
}
void handle_customer_dlg_setting(PANEL_DLG_MODE dlgMode, unsigned int customerIndex)
{
    switch(dlgMode)
    {
        case PANEL_DLG_DEFAULT:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2k120_4K1K240)
            {
                panel_vby1_dlg_4k2K120_4k1k240(dlgMode);
            }
            else if(customerIndex == VBY_ONE_PANEL_HSR_4K2K60_4K1K120)
            {
                panel_vby1_dlg_4k2K60_4k2k120(dlgMode);
            }
            else if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
            break;

        case PANEL_DLG_4k1k240:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2k120_4K1K240)
            {
                panel_vby1_dlg_4k2K120_4k1k240(dlgMode);
            }
#if 0
            else if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
#endif
            break;

        case PANEL_DLG_4k1k120:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2K60_4K1K120)
            {
                panel_vby1_dlg_4k2K60_4k2k120(dlgMode);
            }
            break;
#if 0
        case PANEL_DLG_4k2k144:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                 panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
            break;
#endif
        case PANEL_DLG_4k1k288:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                 panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
            break;

        default:
            break;
    }
}

PANEL_DLG_MODE get_customer_dlg_mode(PANEL_HSR_MODE hsrMode, unsigned int customerIndex)
{
    PANEL_DLG_MODE panelDlgMode = PANEL_DLG_DEFAULT;

    if(hsrMode == PANEL_HSR_4k1k240)
    {
        //hdmi 2k1k240 input support support 4k1k240 output
        if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
        {
            MEASURE_TIMING_T vfehdmitiminginfo = {0};
            drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
            if((vfehdmitiminginfo.v_freq > 200000)
                && (vfehdmitiminginfo.v_total < 1600))
            {
                panelDlgMode  = PANEL_DLG_4k1k240;
            }
        }
    }
    else if(hsrMode == PANEL_HSR_4k1k120)
    {
        panelDlgMode = PANEL_DLG_4k1k120;
    }
    else if(hsrMode == PANEL_HSR_4k1k288)
    {
        if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
        {
            MEASURE_TIMING_T vfehdmitiminginfo = {0};
            drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
            if((vfehdmitiminginfo.v_freq > 200000)
                && (vfehdmitiminginfo.v_total < 1600))
            {
                //hdmi 2k1k288 or 2k1k240 input support 4k1k288 or 4k1k240 output
                panelDlgMode  = PANEL_DLG_4k1k288;
            }
#if 0
            else if((vfehdmitiminginfo.v_freq > 200000)
                && (vfehdmitiminginfo.v_total < 1600))
            {
                panelDlgMode  = PANEL_DLG_4k1k240;
            }
            else if((vfehdmitiminginfo.v_freq > 143500)
                && (vfehdmitiminginfo.v_freq < 144500))
            {
                //hdmi 144 input support support 144 output
                panelDlgMode = PANEL_DLG_4k2k144;
            }
#endif
        }
    }
    else
    {
        panelDlgMode = PANEL_DLG_DEFAULT;
    }

    return panelDlgMode;
}

PANEL_DLG_MODE vbe_get_timing_support_dlg(PANEL_HSR_MODE hsrMode, unsigned int customerIndex)
{
    PANEL_DLG_MODE panelDlgMode = PANEL_DLG_DEFAULT;

    if(HAL_VBE_Panel_Get_DlgEnable())
    {
        return get_customer_dlg_mode(hsrMode, customerIndex);
    }
#if 0
    else
    {
        if(hsrMode == PANEL_HSR_4k1k288)
        {
            if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
            {
                MEASURE_TIMING_T vfehdmitiminginfo = {0};
                drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
                if((vfehdmitiminginfo.v_freq > 250000)
                    && (vfehdmitiminginfo.v_total < 1600))
                {
                    //hsr disable hdmi 2k1k288 input support support 4k2k144 output
                    panelDlgMode  = PANEL_DLG_4k2k144;
                }
                else if((vfehdmitiminginfo.v_freq > 143500)
                    && (vfehdmitiminginfo.v_freq < 144500))
                {
                    //hdmi 144 input support support 144 output
                    panelDlgMode = PANEL_DLG_4k2k144;
                }
            }
            else
            {
                panelDlgMode = PANEL_DLG_DEFAULT;
            }
        }
        else
        {
            panelDlgMode = PANEL_DLG_DEFAULT;
        }
    }
#endif
    return panelDlgMode;
}



void dlg_sfg_inner_ptg(unsigned char bOnOff)
{
    sfg_sfg_force_bg_at_dif_RBUS sfg_sfg_force_bg_at_dif_reg;

    sfg_sfg_force_bg_at_dif_reg.regValue = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);

    sfg_sfg_force_bg_at_dif_reg.bg_dif_red = 0;
    sfg_sfg_force_bg_at_dif_reg.bg_dif_grn = 0;
    sfg_sfg_force_bg_at_dif_reg.bg_dif_blu = 0;

    if(bOnOff)
    {
        sfg_sfg_force_bg_at_dif_reg.en_force_bg_dif = 1;
    }
    else
    {
        sfg_sfg_force_bg_at_dif_reg.en_force_bg_dif = 0;
    }

    IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_sfg_force_bg_at_dif_reg.regValue);
}

void panel_dynamic_select_setting(PANEL_DLG_MODE dlgMode)
{
    if(s_panelDlgMode == dlgMode)
    {
        rtd_pr_vbe_info("[%s %d]dlg mode not change dlgMode:%d\n", __FUNCTION__, __LINE__, dlgMode);
        if(dlgCallbackFunc != NULL)
        {
            dlgCallbackFunc(E_DLG_DISABLE_DLG_MASK);
        }

        return;
    }

    s_panelDlgMode = dlgMode;

    dlg_sfg_inner_ptg(1);

    if(dlgCallbackFunc != NULL)
    {
        dlgCallbackFunc(E_DLG_FREE_RUN);
        dlgCallbackFunc(E_DLG_DISABLE_OSD);
    }

    handle_customer_dlg_setting(dlgMode, Get_DISPLAY_PANEL_CUSTOM_INDEX());

    if(dlgCallbackFunc != NULL)
    {
        dlgCallbackFunc(E_DLG_NOTIFY_VCPU_UPGRAD_PANEL_PARAMETER);
        //4k2k60  4k2k120 sfg down sample
        if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K60_4K1K120)
        {
            dlgCallbackFunc(E_DLG_DTG_FRACTION);
            dlgCallbackFunc(E_DLG_MEMC);
            dlgCallbackFunc(E_DLG_ENABLE_OSD);
            dlgCallbackFunc(E_DLG_SET_PRINT_DLG_MESSAGE_CNT);
        }

        dlgCallbackFunc(E_DLG_DISABLE_DLG_MASK);
    }

    dlg_sfg_inner_ptg(0);
}

unsigned int HAL_VBE_DISP_GetOutputFrameRateByMasterDtg(void)
{
    unsigned int framerate = 0;

    ppoverlay_dvs_cnt_RBUS ppoverlay_dvs_cnt_Reg;

    ppoverlay_dvs_cnt_Reg.regValue = IoReg_Read32(PPOVERLAY_DVS_cnt_reg);

    if(ppoverlay_dvs_cnt_Reg.dvs_cnt != 0)
    {
        framerate = (27000000 / ppoverlay_dvs_cnt_Reg.dvs_cnt);
    }

    return framerate;
}
static char *get_dlg_mode_str(void)
{
    char *dlg_table[] = {"PANEL_DLG_DEFAULT", "PANEL_DLG_4k1k120",
                        /*"PANEL_DLG_4k2k120", "PANEL_DLG_4k2k144",*/
                        "PANEL_DLG_4k1k240", "PANEL_DLG_4k1k288"};

    PANEL_DLG_MODE index = vbe_get_cur_dlg_mode();

    if(index < (sizeof(dlg_table) / sizeof(char *)))
    {
        return dlg_table[index];
    }
    else
    {
        return "Error DLG MODE";
    }
}

void printf_dlg_msg(void)
{
    unsigned int outPutFrameRate = HAL_VBE_DISP_GetOutputFrameRateByMasterDtg();

    if(print_dlg_message_cnt > 0)
    {
        rtd_pr_vbe_info("DLG MSG==>\n");
        rtd_pr_vbe_info("DLG Enable: %d\n", HAL_VBE_Panel_Get_DlgEnable());
        rtd_pr_vbe_info("DLG OutputFrame: %dHz\n", outPutFrameRate);
        rtd_pr_vbe_info("DLG MODE: %s\n", get_dlg_mode_str());
        print_dlg_message_cnt--;
    }
}

void vbe_set_print_dlg_message_cnt(unsigned int cnt)
{
    print_dlg_message_cnt = cnt;
}

void vbe_dlg_init(DLG_CALLBACK callback)
{
    dlgCallbackFunc = callback;
    dlgCallbackFunc(E_DLG_INIT);
}

#endif
