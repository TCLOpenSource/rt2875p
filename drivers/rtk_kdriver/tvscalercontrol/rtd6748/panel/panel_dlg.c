#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <rtk_kdriver/io.h>
#include <generated/autoconf.h>
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
#include <rtk_kdriver/i2c-rtk-api.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/sfg_reg.h>
#include <rbus/osdovl_reg.h>
#include <rbus/od_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/two_step_uzu_reg.h>
#include <rbus/pinmux_reg.h>
#ifdef CONFIG_DYNAMIC_PANEL_SELECT

extern platform_info_t platform_info;
static unsigned char s_vbeDlgEnable = 1;
static PANEL_DLG_MODE s_curDlgMode = PANEL_DLG_DEFAULT;
static DLG_CALLBACK dlgCallbackFunc = NULL;
/*static*/ PANEL_DLG_MODE s_panelDlgMode = PANEL_DLG_DEFAULT;
static unsigned int print_dlg_message_cnt = 0;

#define PANEL_HSR_4k1k240_MIN_VFREQ  200000
#define PANEL_HSR_4k1k240_MAX_V_TOTAL  1600

#define PANEL_HSR_4k1k120_MIN_VFREQ  95000
#define PANEL_HSR_4k1k120_MIN_V_TOTAL  1400
#define PANEL_HSR_4k1k120_MAX_V_TOTAL  1600
#define PANEL_HSR_4k1k120_MIN_H_TOTAL  2600
#define PANEL_HSR_4k1k120_MAX_H_TOTAL  3100

#define PANEL_HSR_4k1k288_MIN_VFREQ  250000
#define PANEL_HSR_4k1k288_MAX_V_TOTAL  1600

#define PANEL_DLG_4k2k144_MIN_VFREQ  143500
#define PANEL_DLG_4k2k144_MAX_VFREQ  144500

extern void fwif_color_dlg_trigger(void);
extern void drvif_color_od_enable_set(unsigned char od_enable);
extern unsigned int frameChangeStep;


// GMT IC TC901
#define TC901_DEV_ADDR              0x20    // ic slave address
#define TC901_CTRL_ADDR             0x02    // control register address
#define TC901_DATA_NUM              74
#define TC901_CTRL_EEPROM_ADDR      0xff
#define TC901_CTRL_EEPROM_VALUE     0X80

unsigned char TC901_Data_PT850GT01_4_normal[74] =
{
    0x00, 0x00, 0x00, 0x00, 0xD9, 0x26, 0x1A, 0x02, 0x24, 0x1A, 0x14, 0x2D, 0x0A, 0x2D, 0x49, 0x28,
    0x2E, 0x3B, 0x67, 0x3D, 0x13, 0xC5, 0x34, 0x52, 0xD2, 0x29, 0x32, 0x42, 0x21, 0xE1, 0xB2, 0x17,
    0x61, 0x45, 0x10, 0x20, 0xA9, 0x01, 0xA0, 0x12, 0x1E, 0x2D, 0x3F, 0x8D, 0x0F, 0x0C, 0x0A, 0x00,
    0x00, 0x5F, 0xD9, 0x75, 0x93, 0x49, 0x80, 0x49, 0x80, 0x19, 0x10, 0x00, 0x00, 0x60, 0x87, 0x20,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};
unsigned char TC901_Data_PT850GT01_4_HSR[74] =
{
    0x00, 0x00, 0x00, 0x00, 0xD9, 0x26, 0x1A, 0x02, 0x24, 0x1A, 0x14, 0x37, 0x0A, 0x37, 0x49, 0x28,
    0x2E, 0x3B, 0x67, 0x3D, 0x13, 0xC5, 0x34, 0x52, 0xD2, 0x29, 0x32, 0x42, 0x21, 0xE1, 0xB2, 0x17,
    0x61, 0x45, 0x10, 0x20, 0xA9, 0x01, 0xA0, 0x12, 0x20, 0x2D, 0x3F, 0x78, 0x0F, 0x0C, 0x0A, 0x00,
    0x00, 0x5F, 0xD9, 0x75, 0x93, 0x49, 0x80, 0x49, 0x80, 0x19, 0x10, 0x00, 0x00, 0x60, 0x87, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

unsigned char TC901_Data_ST55D12_4_normal[74] =
{
    0x00, 0x00, 0x00, 0x00, 0xD9, 0x26, 0x1A, 0x01, 0x2C, 0x11, 0x10, 0x19, 0x0F, 0x19, 0x3A, 0x2B,
    0x39, 0x30, 0x64, 0x3C, 0x03, 0xA4, 0x30, 0x52, 0xB1, 0x28, 0x42, 0x1E, 0x20, 0x41, 0xAC, 0x19,
    0x11, 0x28, 0x0F, 0x50, 0x9D, 0x01, 0xC0, 0x0D, 0x1E, 0x2D, 0x3F, 0x8D, 0x0D, 0x06, 0x06, 0x00,
    0x00, 0x5F, 0xC9, 0x74, 0x93, 0x47, 0x80, 0x47, 0x80, 0x15, 0x10, 0x00, 0x00, 0x60, 0xC7, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};
unsigned char TC901_Data_ST55D12_4_HSR[74] =
{
    0x00, 0x00, 0x00, 0x00, 0xD9, 0x26, 0x1A, 0x01, 0x2C, 0x11, 0x1A, 0x19, 0x0F, 0x19, 0x3A, 0x2B,
    0x39, 0x30, 0x64, 0x3C, 0x03, 0xA4, 0x30, 0x52, 0xB1, 0x28, 0x42, 0x1E, 0x20, 0x41, 0xAC, 0x19,
    0x11, 0x28, 0x0F, 0x50, 0x9D, 0x01, 0xC0, 0x0D, 0x1E, 0x2D, 0x3F, 0x8D, 0x0D, 0x06, 0x06, 0x00,
    0x00, 0x5F, 0xC9, 0x74, 0x93, 0x47, 0x80, 0x47, 0x80, 0x15, 0x10, 0x00, 0x00, 0x60, 0xC7, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

void vbe_set_TC901_hsr_mode(unsigned char enable)
{
    int ret;
    unsigned char TC901_normal_set[2]={0x43,0x00};
    unsigned char TC901_DLG_set[2]={0x43,0x20};
    if(enable){
        ret = i2c_master_send_ex(1,TC901_DEV_ADDR,&TC901_DLG_set[0],2);
    }
    else{
        ret = i2c_master_send_ex(1,TC901_DEV_ADDR,&TC901_normal_set[0],2);
    }

    if(ret < 0){
        rtd_pr_vbe_emerg("[%s] i2c HSR_TC901 output_mode write fail\n",__FUNCTION__);
    }
    else{
        rtd_pr_vbe_emerg("[%s] i2c HSR_TC901 output_mode write ok\n",__FUNCTION__);
    }
}

static void TC901_i2c_write(unsigned char *pDes)
{
    int i,ret;
    unsigned char t_buff[TC901_DATA_NUM-1];

    t_buff[0] = 0x02;
    for(i=0; i< TC901_DATA_NUM-2; i++)
    {
        t_buff[i+1] = pDes[i+2];
    }

    ret = i2c_master_send_ex(1, TC901_DEV_ADDR, t_buff, TC901_DATA_NUM-1);
    if(ret < 0){
        rtd_pr_vbe_emerg("[%s] i2c TC901 write fail\n",__FUNCTION__);
        //rtd_pr_vbe_info("i2c TC901 write fail\n");
    }
    else{
        rtd_pr_vbe_emerg("[%s] i2c TC901 write OK\n",__FUNCTION__);
        //rtd_pr_vbe_info("i2c TC901 write OK\n");
    }
}


/**
** if vrr off, but device also send vrr info, hsr off, then we can not run vrr flow
*/
unsigned char vbe_get_panel_vrr_freesync_timing_is_valid(void)
{
    HDMI_TIMING_T vfehdmitiminginfo = {0};
    unsigned int panelSupportMaxFrameRate = 0;
    static int printCnt = 0;

    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
    {
        drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);

        panelSupportMaxFrameRate = Get_DISPLAY_REFRESH_RATE() * 1000;

        //if input timing over range of panel panelSupportMaxFrameRate, not run vrr
        if(vfehdmitiminginfo.run_vrr && (vfehdmitiminginfo.v_freq > panelSupportMaxFrameRate) && ((vfehdmitiminginfo.v_freq - panelSupportMaxFrameRate) > 500))
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

void panel_isp_ST55D12_4_tcon_setting(unsigned char DLG_mode)
{
    rtd_pr_vbe_emerg("[%s] ST55D12_4 DLG_mode = %d\n",__FUNCTION__,DLG_mode);

    if(DLG_mode == 1){
        TC901_i2c_write(TC901_Data_ST55D12_4_HSR);
    }else{
        TC901_i2c_write(TC901_Data_ST55D12_4_normal);
    }

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    rtd_part_outl(0xb800080c, 15, 12, 0x5);     //STV L6 Tcon0
    rtd_part_outl(0xb8000800, 31, 28, 0x5);     //CK1 E14 Tcon3,2
    rtd_part_outl(0xb8000804, 31, 28, 0x6);     //CK2 F14 Tcon11,10
    rtd_part_outl(0xb8000800, 23, 20, 0x5);     //LC D14 Tcon5

    rtd_outl(0xb802D208, 0x00000000);       // Global Tcon disable,

    if(DLG_mode == 1){
        //STV Tcon0
        rtd_outl(0xB802D210, 0x0e9e0096);       // H_start/H_end, //CHANGE 
        rtd_outl(0xb802D214, 0x00140017);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 Tcon11,10 //rising
        //TCON10
        rtd_outl(0xB802D350, 0x0d0d0f0d);
        rtd_outl(0xB802D354, 0x00170452);
        rtd_outl(0xB802D358, 0x00000080);
        //Tcon11
        rtd_outl(0xB802D370, 0x04750675);
        rtd_outl(0xB802D374, 0x00170452);
        rtd_outl(0xB802D378, 0x00000082);
        //CPV1 TCON3,2 //rising
        //Tcon2
        rtd_outl(0xB802D250, 0x00160216);
        rtd_outl(0xB802D254, 0x00160451);
        rtd_outl(0xB802D258, 0x00000080);
        //Tcon3
        rtd_outl(0xB802D270, 0x08ae0aae);
        rtd_outl(0xB802D274, 0x00150450);
        rtd_outl(0xB802D278, 0x00000082);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00100010);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00C700C7);       // Across Frame Period,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00100010);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,
        // Tcon21 (POL),
        rtd_outl(0xb802D4b0, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D4b4, 0x002b08A5);       // V_start/V_end,
        rtd_outl(0xb802D4b8, 0x00000383);       // Control,
        rtd_outl(0xb802D4bc, 0x00040002);       // Across Frame Period,

    }
    else{
        //STV Tcon0
        rtd_outl(0xB802D210, 0x0E4A03B5);       // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x0029002E);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0AED0BED);//rising
        rtd_outl(0xB802D374, 0x002e08A1);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x01E002E0);//rising
        rtd_outl(0xB802D274, 0x002b089E);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00190019);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00630063);       // Across Frame Period,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00200020);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,
        // Tcon21 (POL),
        rtd_outl(0xb802D4b0, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D4b4, 0x002b08A5);       // V_start/V_end,
        rtd_outl(0xb802D4b8, 0x00000383);       // Control,
        rtd_outl(0xb802D4bc, 0x00040002);       // Across Frame Period,
    }
    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);       // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);       // Global Tcon enable,
}

void panel_cmpi_V850DJ2_Q01_tcon_setting(unsigned char DLG_mode)
{
    rtd_pr_vbe_emerg("[%s] V850DJ2_Q01 DLG_mode = %d\n",__FUNCTION__,DLG_mode);

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    rtd_part_outl(0xb800080c, 15, 12, 0x5); 	//STV L6 Tcon0
    rtd_part_outl(0xb8000800, 31, 28, 0x5); 	//CK1 E14 Tcon3,2,1
    rtd_part_outl(0xb8000804, 31, 28, 0x6); 	//CK2 F14 Tcon11,10,9
    rtd_part_outl(0xb8000800,  7,  4, 0x3); 	//RESET D15 Tcon7
    //rtd_part_outl(0xb8000800, 23, 20, 0x5);	//LC D14 Tcon5

    rtd_outl(0xb802D208, 0x00000000);       // Global Tcon disable,

    if(DLG_mode == 1){
        ////////////////////////////////////////////////////////////////////////////
        //STV Tcon0
        rtd_outl(0xB802D210, 0x096d0178);		// H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x00130017);		// change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);		// Control,

        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11,10,9 //rising
        //TCON9 CK1,3,5,7
        rtd_outl(0xB802D330, 0x08ee0aee);
        rtd_outl(0xB802D334, 0x00000465);
        rtd_outl(0xB802D338, 0x00000080);
        //TCON10 CK2,4,6,8
        rtd_outl(0xB802D350, 0x00560256);
        rtd_outl(0xB802D354, 0x00000465);
        rtd_outl(0xB802D358, 0x00000082);
        //TCON11
        rtd_outl(0xB802D370, 0x025608ee);
        rtd_outl(0xB802D374, 0x000f0017);
        rtd_outl(0xB802D378, 0x000001c1);
        //CPV1 TCON3,2,1 //rising
        //Tcon1 CK1,3,5,7
        rtd_outl(0xB802D230, 0x09740b74);
        rtd_outl(0xB802D234, 0x00000465);
        rtd_outl(0xB802D238, 0x00000080);
        //Tcon2 CK2,4,6,8
        rtd_outl(0xB802D250, 0x00cf02cf);
        rtd_outl(0xB802D254, 0x00000465);
        rtd_outl(0xB802D258, 0x00000082);
        //TCON3
        rtd_outl(0xB802D270, 0x04ab0974);
        rtd_outl(0xB802D274, 0x000d0015);
        rtd_outl(0xB802D278, 0x000001c1);

        ////////////////////////////////////////////////////////////////////////////
        //RESET, Tcon7
        rtd_outl(0xb802D2f0, 0x00cf0056);	  // H_start/H_end, //CHANGE
        rtd_outl(0xb802D2f4, 0x044f0451);	  // V_start/V_end,  //CHANGE
        rtd_outl(0xb802D2f8, 0x00000180);	  // Control,

        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21,22
        // Tcon20 (POL2),
        rtd_outl(0xb802D490, 0x00000000);		// H_start/H_end,
        rtd_outl(0xb802D494, 0x00000000);		// V_start/V_end,
        rtd_outl(0xb802D498, 0x00000000);		// Control,
        rtd_outl(0xb802D4A0, 0x00000000);		// Across Frame Period,
        // Tcon21 (POL),
        rtd_outl(0xb802D4b0, 0x00000000);		// H_start/H_end,
        rtd_outl(0xb802D4b4, 0x00000000);		// V_start/V_end,
        rtd_outl(0xb802D4b8, 0x00000000);		// Control,
        rtd_outl(0xb802D4bc, 0x00000000);		// Across Frame Period,
        //POLC Tcon22
        rtd_outl(0xb802d4d0, 0x00640064);
        rtd_outl(0xb802d4d4, 0x00100010);
        rtd_outl(0xb802d4d8, 0x00000480);
        rtd_outl(0xb802d4e0, 0x00000000);
    }
    else{
        ////////////////////////////////////////////////////////////////////////////
        //STV Tcon0
        rtd_outl(0xB802D210, 0x09b10102);		// H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x0026002e);		// change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);		// Control,

        ////////////////////////////////////////////////////////////////////////////
        rtd_outl(0xB802D350, 0x09750b75);//rising
        rtd_outl(0xB802D354, 0x0022002d);
        rtd_outl(0xB802D358, 0x000000a0);
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x09750b75);//rising
        rtd_outl(0xB802D374, 0x000008ca);
        rtd_outl(0xB802D378, 0x00000081);
        //CPV1 TCON2
        rtd_outl(0xB802D250, 0x0a1e0c1e);//rising
        rtd_outl(0xB802D254, 0x001e0029);
        rtd_outl(0xB802D258, 0x000000a0);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x0a1e0c1e);//rising
        rtd_outl(0xB802D274, 0x000008c9);
        rtd_outl(0xB802D278, 0x00000081);

        ////////////////////////////////////////////////////////////////////////////
        //RESET, Tcon7
        rtd_outl(0xb802D2f0, 0x0a1e0975);	  // H_start/H_end, //CHANGE
        rtd_outl(0xb802D2f4, 0x08a108a5);	  // V_start/V_end,  //CHANGE
        rtd_outl(0xb802D2f8, 0x00000180);	  // Control,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21,22
        // Tcon20 (POL2),
        rtd_outl(0xb802D490, 0x00000000);		// H_start/H_end,
        rtd_outl(0xb802D494, 0x00000000);		// V_start/V_end,
        rtd_outl(0xb802D498, 0x00000000);		// Control,
        rtd_outl(0xb802D4A0, 0x00000000);		// Across Frame Period,
        // Tcon21 (POL),
        rtd_outl(0xb802D4b0, 0x00000000);		// H_start/H_end,
        rtd_outl(0xb802D4b4, 0x00000000);		// V_start/V_end,
        rtd_outl(0xb802D4b8, 0x00000000);		// Control,
        rtd_outl(0xb802D4bc, 0x00000000);		// Across Frame Period,
        //POLC Tcon22
        rtd_outl(0xb802d4d0, 0x00640064);
        rtd_outl(0xb802d4d4, 0x00100010);
        rtd_outl(0xb802d4d8, 0x00000480);
        rtd_outl(0xb802d4e0, 0x00000000);
    }
    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);       // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);       // Global Tcon enable,
}

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

    if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
    {
        //4k2k144 dpll 686M change to 4k2k120, need update dpll 594M
        vbe_set_dpll_clk(DPLL_594M);
    }

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

    if(Get_DISPLAY_PANEL_CUSTOM_INDEX() == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
    {
        //4k2k144 dpll 686M change to 4k2k120, need update dpll 594M
        vbe_set_dpll_clk(DPLL_594M);
    }

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
    vbe_set_dpll_clk(DPLL_686_88M);

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
    vbe_set_dpll_clk(DPLL_686_88M);

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
void panel_tcon_dlg_4k2K60_4k1k120(PANEL_DLG_MODE dlgMode,unsigned int customerIndex)
{
    PANEL_CONFIG_PARAMETER panelConfigParameter = {0};

    memcpy(&panelConfigParameter, (PANEL_CONFIG_PARAMETER *)&platform_info.panel_parameter, sizeof(PANEL_CONFIG_PARAMETER));

    if(dlgMode == PANEL_DLG_4k1k120)
    {
       panelConfigParameter.iCONFIG_DISPLAY_REFRESH_RATE = 120;
       panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL = 1125;
       panelConfigParameter.iCONFIG_DISP_DEN_STA_VPOS = 20;
       panelConfigParameter.iCONFIG_DISP_DEN_END_VPOS = 1100;
       panelConfigParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;
       panelConfigParameter.iCONFIG_DISP_ACT_END_VPOS = 1080;
       panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = 1090;
       panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = 1350;
       panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = 1122;
       panelConfigParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = 1350;
    }

    if(customerIndex == 228){
        panel_isp_ST55D18_2_tcon_setting(dlgMode);
    }
    else if(customerIndex == 233){
        panel_isp_ST65D06_3_tcon_setting(dlgMode);
    }
    else if(customerIndex == 235){
        panel_isp_ST75D03_2_tcon_setting(dlgMode);
    }
    else if(customerIndex == 214){
        panel_isp_PT85GT01_4_tcon_setting(dlgMode);
    }
    else if(customerIndex == 230){
        panel_isp_ST55D12_4_tcon_setting(dlgMode);
    }
    else if(customerIndex == 244){
        panel_cmpi_V850DJ2_Q01_tcon_setting(dlgMode);
    }

    Panel_InitParameter(&panelConfigParameter);
	panel_dlg_customer_dtg_setting();
}

void panel_isp_ST55D18_2_tcon_setting(unsigned char DLG_mode)
{
    vbe_set_TC901_hsr_mode(DLG_mode);

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);   // Dv_den_end[29:16] = 1102, Dv_den_sta[13:0] = 22
        rtd_outl(0xb802D954, 0x10180118);   // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);   // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);   // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);   // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);   // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    //STV L6 Tcon0
    rtd_part_outl(0xb800080c, 15, 12, 0x5);
    //CK1 E14 Tcon3
    rtd_part_outl(0xb8000800, 31, 28, 0x5);
    //CK2 F14 Tcon11
    rtd_part_outl(0xb8000804, 31, 28, 0x6);
    //RESET D15 Tcon7
    //rtd_part_outl(0xb8000800,  7,  4, 0x3);
    //LC D14 Tcon5
    rtd_part_outl(0xb8000800, 23, 20, 0x5);
    rtd_outl(0xb802D208, 0x00000000);       // Global Tcon disable,

    if(DLG_mode == 1)
    {
        //STV Tcon0
        rtd_outl(0xB802D210, 0x0a870a87);   // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x00140017);   // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);   // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0c700d70);   // rising
        rtd_outl(0xB802D374, 0x00170453);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x022a032a);   // rising
        rtd_outl(0xB802D274, 0x00160452);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);   // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00100010);   // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);   // Control,
        rtd_outl(0xb802D2c0, 0x00c700c7);   // Across Frame Period,
        ////////////////////////////////////////////////////////////////////////////
        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1); // H_start/H_end, //CHANGE
        //rtd_outl(0xb802D2f4, 0x08A608ad); // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180); // Control,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);   // H_start/H_end,
        rtd_outl(0xb802D494, 0x00100010);   // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);   // Control,
        rtd_outl(0xb802D4A0, 0x00000000);   // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);   // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);   // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);   // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);   // Across Frame Period,
    }
    else{
        //STV Tcon0
        rtd_outl(0xB802D210, 0x08d808d8);   // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x0028002E);   // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);   // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0C6A0D6A);   //rising
        rtd_outl(0xB802D374, 0x002e08A5);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x09080a08);   //rising
        rtd_outl(0xB802D274, 0x002b08A2);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);   // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00190019);   // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);   // Control,
        rtd_outl(0xb802D2c0, 0x00630063);   // Across Frame Period,
        ////////////////////////////////////////////////////////////////////////////
        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1); // H_start/H_end, //CHANGE
        //rtd_outl(0xb802D2f4, 0x08A608ad); // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180); // Control,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);   // H_start/H_end,
        rtd_outl(0xb802D494, 0x00200020);   // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);   // Control,
        rtd_outl(0xb802D4A0, 0x00000000);   // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);   // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);   // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);   // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);   // Across Frame Period,
        }

    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);       // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);       // Global Tcon enable,
}

void panel_isp_ST65D06_3_tcon_setting(unsigned char DLG_mode)
{
    rtd_pr_vbe_emerg("[%s] ST65D06_3 DLG_mode = %d\n",__FUNCTION__,DLG_mode);
    vbe_set_TC901_hsr_mode(DLG_mode);

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    //STV L6 Tcon0
    rtd_part_outl(0xb800080c, 15, 12, 0x5);
    //CK1 E14 Tcon3
    rtd_part_outl(0xb8000800, 31, 28, 0x5);
    //CK2 F14 Tcon11
    rtd_part_outl(0xb8000804, 31, 28, 0x6);
    //RESET D15 Tcon7
    //rtd_part_outl(0xb8000800,  7,  4, 0x3);
    //LC D14 Tcon5
    rtd_part_outl(0xb8000800, 23, 20, 0x5);

    rtd_outl(0xb802D208, 0x00000000);        // Global Tcon disable,
    if(DLG_mode == 1){
        //STV Tcon0
        rtd_outl(0xB802D210, 0x098c098c);        // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x00140017);        // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);        // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0b690d69);//rising
        rtd_outl(0xB802D374, 0x00170452);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x012f025d);//rising
        rtd_outl(0xB802D274, 0x00160451);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);        // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00100010);        // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);        // Control,
        rtd_outl(0xb802D2c0, 0x00c700c7);        // Across Frame Period,

        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1);        // H_start/H_end, //CHANGE 
        //rtd_outl(0xb802D2f4, 0x08A608ad);        // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180);        // Control,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);        // H_start/H_end,
        rtd_outl(0xb802D494, 0x00100010);        // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);        // Control,
        rtd_outl(0xb802D4A0, 0x00000000);        // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);        // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);        // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);        // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);        // Across Frame Period,
    }
    else{
        //STV Tcon0
        rtd_outl(0xB802D210, 0x08000800);        // H_start/H_end, //CHANGE 
        rtd_outl(0xb802D214, 0x0029002E);        // change1: V_start/V_end,    
        rtd_outl(0xb802D218, 0x00000180);        // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0b690d69);//rising
        rtd_outl(0xB802D374, 0x002e08A5);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x08000a00);//rising
        rtd_outl(0xB802D274, 0x002b08a2);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);        // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00190019);        // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);        // Control,
        rtd_outl(0xb802D2c0, 0x00630063);        // Across Frame Period,

        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1);        // H_start/H_end, //CHANGE 
        //rtd_outl(0xb802D2f4, 0x08A608ad);        // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180);        // Control,    
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);        // H_start/H_end,
        rtd_outl(0xb802D494, 0x00200020);        // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);        // Control,
        rtd_outl(0xb802D4A0, 0x00000000);        // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);        // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);        // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);        // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);        // Across Frame Period,
    }

    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);        // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);        // Global Tcon enable,
}

void panel_isp_ST75D03_2_tcon_setting(unsigned char DLG_mode)
{
    rtd_pr_vbe_emerg("[%s] ST75D03_2 DLG_mode = %d\n",__FUNCTION__,DLG_mode);
    vbe_set_TC901_hsr_mode(DLG_mode);

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    //STV L6 Tcon0
    rtd_part_outl(0xb800080c, 15, 12, 0x5);
    //CK1 E14 Tcon3
    rtd_part_outl(0xb8000800, 31, 28, 0x5);
    //CK2 F14 Tcon11
    rtd_part_outl(0xb8000804, 31, 28, 0x6);
    //RESET D15 Tcon7
    //rtd_part_outl(0xb8000800,  7,  4, 0x3);
    //LC D14 Tcon5
    rtd_part_outl(0xb8000800, 23, 20, 0x5);
    
    rtd_outl(0xb802D208, 0x00000000);       // Global Tcon disable,

    if(DLG_mode == 1){
        //STV Tcon0
        rtd_outl(0xB802D210, 0x09c709c7);       // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x00140017);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0ba10ca1);//rising
        rtd_outl(0xB802D374, 0x00170454);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x01470247);//rising
        rtd_outl(0xB802D274, 0x00160453);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00100010);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00c700c7);       // Across Frame Period,

        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1);     // H_start/H_end, //CHANGE
        //rtd_outl(0xb802D2f4, 0x08A608ad);     // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180);     // Control, 
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00100010);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);     // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);     // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);     // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);     // Across Frame Period,
    }
    else{
        //STV Tcon0
        rtd_outl(0xB802D210, 0x08410841);       // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x0028002E);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,
        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0A9D0B9D);//rising
        rtd_outl(0xB802D374, 0x002e08A1);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x08250925);//rising
        rtd_outl(0xB802D274, 0x002b089E);
        rtd_outl(0xB802D278, 0x00000080);
        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x01A801A8);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x00190019);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00630063);       // Across Frame Period,

        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1);     // H_start/H_end, //CHANGE
        //rtd_outl(0xb802D2f4, 0x08A608ad);     // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180);     // Control,
        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00200020);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,
        // Tcon21 (POL),
        //rtd_outl(0xb802D4b0, 0x00640064);     // H_start/H_end,
        //rtd_outl(0xb802D4b4, 0x002b08A5);     // V_start/V_end,
        //rtd_outl(0xb802D4b8, 0x00000383);     // Control,
        //rtd_outl(0xb802D4bc, 0x00040002);     // Across Frame Period,
    }

    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);       // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);       // Global Tcon enable,
}

void panel_isp_PT85GT01_4_tcon_setting(unsigned char DLG_mode)
{
    rtd_pr_vbe_emerg("[%s] PT85GT01_4 DLG_mode = %d\n",__FUNCTION__,DLG_mode);
    //rtd_pr_vbe_info("PT85GT01_4 DLG_mode = %d\n", DLG_mode);

    if(DLG_mode == 1){
        TC901_i2c_write(TC901_Data_PT850GT01_4_HSR);
    }else{
        TC901_i2c_write(TC901_Data_PT850GT01_4_normal);
    }

    if(DLG_mode == 1){
        rtd_outl(0xb802D950, 0x044e0016);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }else{
        rtd_outl(0xb802D950, 0x089D002D);    // Dv_den_end[29:16] = 2205, Dv_den_sta[13:0] = 45
        rtd_outl(0xb802D954, 0x10180118);    // Dh_den_end[29:16] = 4120, Dh_den_sta[13:0] = 280
        rtd_outl(0xb802D958, 0x057F112F);    // Dv_width[31:24] = 5, Dh_width[23:16] = 127, Dh_total[12:0] = 4399
    }

    rtd_part_outl(0xb800080c, 15, 12, 0x5);		//STV L6 Tcon0
    rtd_part_outl(0xb8000800, 31, 28, 0x5);		//CK1 E14 Tcon3,Tcon2
    rtd_part_outl(0xb8000804, 31, 28, 0x6);		//CK2 F14 Tcon11,Tcon10
    rtd_part_outl(0xb8000800, 23, 20, 0x5);		//LC D14 Tcon5

    rtd_outl(0xb802D208, 0x00000000);       // Global Tcon disable,

    if(DLG_mode == 1){
        ////////////////////////////////////////////////////////////////////////////
        //STV Tcon0
        rtd_outl(0xB802D210, 0x064c0b59);       // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x00140016);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,

        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11,Tcon10 //rising
        rtd_outl(0xB802D350, 0x02220422);
        rtd_outl(0xB802D354, 0x00180451);
        rtd_outl(0xB802D358, 0x00000080);

        rtd_outl(0xB802D370, 0x0bc90dc9);
        rtd_outl(0xB802D374, 0x00170450);
        rtd_outl(0xB802D378, 0x00000082);

        //CPV1 TCON3,Tcon2 //rising
        rtd_outl(0xB802D250, 0x07200920);
        rtd_outl(0xB802D254, 0x0016044f);
        rtd_outl(0xB802D258, 0x00000080);

        rtd_outl(0xB802D270, 0x0f7c1093);
        rtd_outl(0xB802D274, 0x0015044e);
        rtd_outl(0xB802D278, 0x00000082);

        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x03560356);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x04570457);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00c700c7);       // Across Frame Period,

        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00100010);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,

    }
    else{
        ////////////////////////////////////////////////////////////////////////////
        //STV Tcon0
        rtd_outl(0xB802D210, 0x02eb0b84);       // H_start/H_end, //CHANGE
        rtd_outl(0xb802D214, 0x0028002c);       // change1: V_start/V_end,
        rtd_outl(0xb802D218, 0x00000180);       // Control,

        //RESET, Tcon7
        //rtd_outl(0xb802D2f0, 0x00670cD1);     // H_start/H_end, //CHANGE
        //rtd_outl(0xb802D2f4, 0x08A608ad);     // V_start/V_end,  //CHANGE
        //rtd_outl(0xb802D2f8, 0x00000180);     // Control,

        ////////////////////////////////////////////////////////////////////////////
        //CPV2 TCON11
        rtd_outl(0xB802D370, 0x0b830d83);//rising
        rtd_outl(0xB802D374, 0x002e08A1);
        rtd_outl(0xB802D378, 0x00000080);
        //CPV1 TCON3
        rtd_outl(0xB802D270, 0x02eb04eb);//rising
        rtd_outl(0xB802D274, 0x002b089e);
        rtd_outl(0xB802D278, 0x00000080);

        ////////////////////////////////////////////////////////////////////////////
        //LC TCON5,
        rtd_outl(0xb802D2b0, 0x0b830b83);       // H_start/H_end,
        rtd_outl(0xb802D2b4, 0x08b708b7);       // V_start/V_end,
        rtd_outl(0xb802D2b8, 0x00000480);       // Control,
        rtd_outl(0xb802D2c0, 0x00630063);       // Across Frame Period,

        ////////////////////////////////////////////////////////////////////////////
        // POL, Tcon20,21
        // Tcon20 (POL),
        rtd_outl(0xb802D490, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D494, 0x00200020);       // V_start/V_end,
        rtd_outl(0xb802D498, 0x00000480);       // Control,
        rtd_outl(0xb802D4A0, 0x00000000);       // Across Frame Period,
        // Tcon21 (POL),
        rtd_outl(0xb802D4b0, 0x00640064);       // H_start/H_end,
        rtd_outl(0xb802D4b4, 0x002b08A5);       // V_start/V_end,
        rtd_outl(0xb802D4b8, 0x00000383);       // Control,
        rtd_outl(0xb802D4bc, 0x00040002);       // Across Frame Period,

    }

    ////////////////////////////////////////////////////////////////////////////
    rtd_part_outl(0xb802D208, 1, 1, 0x1);       // Across frame and Across line 3 enable,
    rtd_part_outl(0xb802D208, 0, 0, 0x1);       // Global Tcon enable,
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
    else
    {
        panel_vby1_4k2k120_tx();
    }

    Panel_InitParameter(&panelConfigParameter);
}
void panel_dlg_customer_dtg_setting(void)
{
    sfg_sfg_ctrl_24_RBUS sfg_sfg_ctrl_24_reg;
    sfg_sfg_ctrl_25_RBUS sfg_sfg_ctrl_25_reg;
    sys_reg_sys_dclk_gate_sel0_RBUS sys_reg_sys_dclk_gate_sel0_reg;
    sys_reg_sys_dclk_gate_sel1_RBUS sys_reg_sys_dclk_gate_sel1_reg;
    ppoverlay_dispd_smooth_toggle1_RBUS ppoverlay_dispd_smooth_toggle1_reg;
    ppoverlay_dh_total_last_line_length_RBUS dh_total_last_line_length_reg;
    ppoverlay_dv_length_RBUS dv_length_reg;
    ppoverlay_dv_total_RBUS dv_total_reg;
    ppoverlay_uzudtg_dv_total_RBUS uzudtg_dv_total_reg;
    ppoverlay_uzudtg_dh_total_RBUS uzudtg_dh_total_reg;
    ppoverlay_uzudtg_control3_RBUS ppoverlay_uzudtg_control3_reg;
    ppoverlay_dh_den_start_end_RBUS dh_den_start_end_reg;
    ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
    ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
    scaleup_dm_uzu_input_size_RBUS scaleup_dm_uzu_input_size_reg;
    scaleup_ds_uzu_input_size_RBUS scaleup_ds_uzu_input_size_reg;
    two_step_uzu_dm_two_step_sr_input_size_RBUS two_step_uzu_dm_two_step_sr_input_size_reg;
    two_step_uzu_dm_two_step_sr_ctrl_RBUS two_step_uzu_dm_two_step_sr_ctrl_reg;
    ppoverlay_memc_mux_ctrl_RBUS memc_mux_ctrl_reg;
    ppoverlay_memcdtg_dv_total_RBUS memcdtg_dv_total_reg;
    ppoverlay_memcdtg_dh_total_RBUS memcdtg_dh_total_reg;
    ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end_reg;
    ppoverlay_memcdtg_dh_den_start_end_RBUS memcdtg_dh_den_start_end_reg;
    ppoverlay_main_den_v_start_end_RBUS main_den_v_start_end_reg;
    ppoverlay_main_background_h_start_end_RBUS main_background_h_start_end_reg;
    ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;
    ppoverlay_main_background_v_start_end_RBUS main_background_v_start_end_reg;
    ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;
    ppoverlay_main_den_h_start_end_RBUS main_den_h_start_end_reg;
    od_od_ctrl_RBUS od_od_ctrl_reg;
    ppoverlay_double_buffer_ctrl_RBUS double_buffer_ctrl_reg;
    ppoverlay_double_buffer_ctrl2_RBUS double_buffer_ctrl2_reg;
    sfg_sfg_ctrl_26_RBUS sfg_sfg_ctrl_26_reg;
    sfg_sfg_ctrl_c_RBUS sfg_sfg_ctrl_c_reg;
    sfg_sfg_ctrl_6_RBUS sfg_ctrl_6_reg;
    sfg_sfg_ctrl_0_RBUS sfg_ctrl_0_reg;
    ppoverlay_dispd_stage1_sm_ctrl_RBUS dispd_stage1_sm_ctrl_reg;
    unsigned int count = 0xfffff;



    pr_info("[%s %d]panel refresh:%d\n", __FUNCTION__, __LINE__, Get_DISPLAY_REFRESH_RATE());



    sys_reg_sys_dclk_gate_sel0_reg.regValue = rtd_inl(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
    sys_reg_sys_dclk_gate_sel1_reg.regValue = rtd_inl(SYS_REG_SYS_DCLK_GATE_SEL1_reg);



    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 2;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre2x_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_gate_sel = 0;
    }
    else
    {
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 2;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre_gate_sel = 1;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_ippre2x_gate_sel = 0;
        sys_reg_sys_dclk_gate_sel1_reg.dclk_mc_gate_sel = 0;
    }



    rtd_outl(SYS_REG_SYS_DCLK_GATE_SEL0_reg, sys_reg_sys_dclk_gate_sel0_reg.regValue);
    rtd_outl(SYS_REG_SYS_DCLK_GATE_SEL1_reg, sys_reg_sys_dclk_gate_sel1_reg.regValue);



    dispd_stage1_sm_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_stage1_sm_ctrl_reg);
    //stage1 clk mode revised default set to 1
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1;
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 1;
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
    IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle1_reg.regValue);



    ppoverlay_dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);



    while((ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply)  && --count)
    {
        ppoverlay_dispd_smooth_toggle1_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
    }



    if(count == 0)
    {
        pr_err("dispd_smooth_toggle1 timeout !!!\n");
    }



    //Master DTG
    dh_total_last_line_length_reg.regValue = rtd_inl(PPOVERLAY_DH_Total_Last_Line_Length_reg);
    dh_total_last_line_length_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
    dh_total_last_line_length_reg.dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_DH_Total_Last_Line_Length_reg, dh_total_last_line_length_reg.regValue);



    dv_length_reg.regValue = rtd_inl(PPOVERLAY_DV_Length_reg);
    dv_length_reg.dv_length = Get_DISP_VSYNC_LENGTH();
    rtd_outl(PPOVERLAY_DV_Length_reg, dv_length_reg.regValue);



    dv_total_reg.regValue = rtd_inl(PPOVERLAY_DV_total_reg);
    dv_total_reg.dv_total = Get_DISP_VERTICAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_DV_total_reg, dv_total_reg.regValue);



    //UZU DTG
    uzudtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    uzudtg_dv_total_reg.uzudtg_dv_total = Get_DISP_VERTICAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_uzudtg_DV_TOTAL_reg, uzudtg_dv_total_reg.regValue);



    uzudtg_dh_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DH_TOTAL_reg);
    uzudtg_dh_total_reg.uzudtg_dh_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
    uzudtg_dh_total_reg.uzudtg_dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_uzudtg_DH_TOTAL_reg, uzudtg_dh_total_reg.regValue);



    dh_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_DH_DEN_Start_End_reg);
    dh_den_start_end_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS();
    dh_den_start_end_reg.dh_den_end = Get_DISP_DEN_END_HPOS();
    rtd_outl(PPOVERLAY_DH_DEN_Start_End_reg, dh_den_start_end_reg.regValue);



    dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_DV_DEN_Start_End_reg);
    dv_den_start_end_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
    dv_den_start_end_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
    rtd_outl(PPOVERLAY_DV_DEN_Start_End_reg, dv_den_start_end_reg.regValue);

    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        ppoverlay_uzudtg_control1_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_control1_reg);
        ppoverlay_uzudtg_control1_reg.uzu_4k_120_mode = 1;
        rtd_outl(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);



        scaleup_dm_uzu_input_size_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Input_Size_reg);
        scaleup_dm_uzu_input_size_reg.hor_input_size = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();
        scaleup_dm_uzu_input_size_reg.ver_input_size = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();
        rtd_outl(SCALEUP_DM_UZU_Input_Size_reg, scaleup_dm_uzu_input_size_reg.regValue);



        scaleup_ds_uzu_input_size_reg.regValue = rtd_inl(SCALEUP_DS_UZU_Input_Size_reg);
        scaleup_ds_uzu_input_size_reg.hor_input_size = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();
        scaleup_ds_uzu_input_size_reg.ver_input_size = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();
        rtd_outl(SCALEUP_DS_UZU_Input_Size_reg, scaleup_ds_uzu_input_size_reg.regValue);



        two_step_uzu_dm_two_step_sr_input_size_reg.regValue = rtd_inl(TWO_STEP_UZU_DM_TWO_STEP_SR_Input_Size_reg);
        two_step_uzu_dm_two_step_sr_input_size_reg.hor_input_size = Get_DISP_ACT_END_HPOS() - Get_DISP_ACT_STA_HPOS();
        two_step_uzu_dm_two_step_sr_input_size_reg.ver_input_size = Get_DISP_ACT_END_VPOS() - Get_DISP_ACT_STA_VPOS();
        rtd_outl(TWO_STEP_UZU_DM_TWO_STEP_SR_Input_Size_reg, two_step_uzu_dm_two_step_sr_input_size_reg.regValue);

        two_step_uzu_dm_two_step_sr_ctrl_reg.regValue = rtd_inl(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg);
        two_step_uzu_dm_two_step_sr_ctrl_reg.sr_mdom_sub_en = 1;
        rtd_outl(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg, two_step_uzu_dm_two_step_sr_ctrl_reg.regValue);
    }
    else
    {
        ppoverlay_uzudtg_control1_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_control1_reg);
        ppoverlay_uzudtg_control1_reg.uzu_4k_120_mode = 0;
        rtd_outl(PPOVERLAY_uzudtg_control1_reg, ppoverlay_uzudtg_control1_reg.regValue);



        two_step_uzu_dm_two_step_sr_ctrl_reg.regValue = rtd_inl(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg);
        two_step_uzu_dm_two_step_sr_ctrl_reg.sr_mdom_sub_en = 0;
        rtd_outl(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg, two_step_uzu_dm_two_step_sr_ctrl_reg.regValue);
    }



    //MEMC DTG setting
    memcdtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    memcdtg_dv_total_reg.memcdtg_dv_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_memcdtg_DV_TOTAL_reg, memcdtg_dv_total_reg.memcdtg_dv_total);



    memcdtg_dh_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_TOTAL_reg);
    memcdtg_dh_total_reg.memcdtg_dh_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
    memcdtg_dh_total_reg.memcdtg_dh_total_last_line = Get_DISP_HORIZONTAL_TOTAL() - 1;
    rtd_outl(PPOVERLAY_memcdtg_DH_TOTAL_reg, memcdtg_dh_total_reg.regValue);



    memcdtg_dh_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
    memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = Get_DISP_DEN_STA_HPOS();
    memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = Get_DISP_DEN_END_HPOS();
    rtd_outl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, memcdtg_dh_den_start_end_reg.regValue);



    memcdtg_dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = Get_DISP_DEN_STA_VPOS();
    memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = Get_DISP_DEN_END_VPOS();
    rtd_outl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, memcdtg_dv_den_start_end_reg.regValue);



    memc_mux_ctrl_reg.regValue = rtd_inl(PPOVERLAY_MEMC_MUX_CTRL_reg);
    memc_mux_ctrl_reg.memc_outmux_sel = 0;
    memc_mux_ctrl_reg.memc_out_bg_en = 0;
    rtd_outl(PPOVERLAY_MEMC_MUX_CTRL_reg, memc_mux_ctrl_reg.regValue);



    ppoverlay_uzudtg_control3_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_control3_reg);
    ppoverlay_uzudtg_control3_reg.align_output_timing = 1;
    rtd_outl(PPOVERLAY_uzudtg_control3_reg, ppoverlay_uzudtg_control3_reg.regValue);



    //MTG
    main_den_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
    main_den_h_start_end_reg.mh_den_sta = Get_DISP_ACT_STA_HPOS();
    main_den_h_start_end_reg.mh_den_end = Get_DISP_ACT_END_HPOS();
    rtd_outl(PPOVERLAY_MAIN_DEN_H_Start_End_reg, main_den_h_start_end_reg.regValue);



    main_den_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
    main_den_v_start_end_reg.mv_den_sta = Get_DISP_ACT_STA_VPOS();
    main_den_v_start_end_reg.mv_den_end = Get_DISP_ACT_END_VPOS();
    rtd_outl(PPOVERLAY_MAIN_DEN_V_Start_End_reg, main_den_v_start_end_reg.regValue);



    main_background_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Background_H_Start_End_reg);
    main_background_h_start_end_reg.mh_bg_sta = Get_DISP_ACT_STA_HPOS();
    main_background_h_start_end_reg.mh_bg_end = Get_DISP_ACT_END_HPOS();
    rtd_outl(PPOVERLAY_MAIN_Background_H_Start_End_reg, main_background_h_start_end_reg.regValue);



    main_background_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Background_V_Start_End_reg);
    main_background_v_start_end_reg.mv_bg_sta = Get_DISP_ACT_STA_VPOS();
    main_background_v_start_end_reg.mv_bg_end = Get_DISP_ACT_END_VPOS();
    rtd_outl(PPOVERLAY_MAIN_Background_V_Start_End_reg, main_background_v_start_end_reg.regValue);



    main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
    main_active_h_start_end_reg.mh_act_sta = Get_DISP_ACT_STA_HPOS();
    main_active_h_start_end_reg.mh_act_end = Get_DISP_ACT_END_HPOS();
    rtd_outl(PPOVERLAY_MAIN_Active_H_Start_End_reg, main_active_h_start_end_reg.regValue);



    main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);
    main_active_v_start_end_reg.mv_act_sta = Get_DISP_ACT_STA_VPOS();
    main_active_v_start_end_reg.mv_act_end = Get_DISP_ACT_END_VPOS();
    rtd_outl(PPOVERLAY_MAIN_Active_V_Start_End_reg, main_active_v_start_end_reg.regValue);



    //od
    od_od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        od_od_ctrl_reg.od_4k120_mode = 1;
    }
    else
    {
        od_od_ctrl_reg.od_4k120_mode = 0;
    }



    rtd_outl(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);



    double_buffer_ctrl_reg.regValue =rtd_inl(PPOVERLAY_Double_Buffer_CTRL_reg);
    double_buffer_ctrl_reg.dreg_dbuf_en = 0;
    double_buffer_ctrl_reg.dmainreg_dbuf_en = 0;
    double_buffer_ctrl_reg.drm_multireg_dbuf_en = 0;
    rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg,double_buffer_ctrl_reg.regValue);



    double_buffer_ctrl2_reg.regValue = rtd_inl(PPOVERLAY_Double_Buffer_CTRL2_reg);
    double_buffer_ctrl2_reg.uzudtgreg_dbuf_en = 0;
    double_buffer_ctrl2_reg.memcdtgreg_dbuf_en = 0;
    rtd_outl(PPOVERLAY_Double_Buffer_CTRL2_reg, double_buffer_ctrl2_reg.regValue);
#if 0
    if(((Get_DISPLAY_REFRESH_RATE() == 144) || (Get_DISPLAY_REFRESH_RATE() == 288))
&& (get_panel_pixel_mode() > PANEL_1_PIXEL))
    {
        panel_dlg_set_dpll_clk(DPLL_4k2k144);
    }
    else
    {
        panel_dlg_set_dpll_clk(DPLL_4k2k120);
    }
#endif
    sfg_sfg_ctrl_24_reg.regValue = rtd_inl(SFG_SFG_CTRL_24_reg);
    sfg_sfg_ctrl_24_reg.dv_den_sta = Get_DISP_DEN_STA_VPOS();
    sfg_sfg_ctrl_24_reg.dv_den_end = Get_DISP_DEN_END_VPOS();
    rtd_outl(SFG_SFG_CTRL_24_reg, sfg_sfg_ctrl_24_reg.regValue);



    sfg_sfg_ctrl_25_reg.regValue = rtd_inl(SFG_SFG_CTRL_25_reg);
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        sfg_sfg_ctrl_25_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS() / 2;
        sfg_sfg_ctrl_25_reg.dh_den_end = Get_DISP_DEN_END_HPOS() / 2;
    }
    else
    {
        sfg_sfg_ctrl_25_reg.dh_den_sta = Get_DISP_DEN_STA_HPOS();
        sfg_sfg_ctrl_25_reg.dh_den_end = Get_DISP_DEN_END_HPOS();
    }
    rtd_outl(SFG_SFG_CTRL_25_reg, sfg_sfg_ctrl_25_reg.regValue);



    sfg_sfg_ctrl_26_reg.regValue = rtd_inl(SFG_SFG_CTRL_26_reg);
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        sfg_sfg_ctrl_26_reg.dv_width = Get_DISP_VSYNC_LENGTH();
        sfg_sfg_ctrl_26_reg.dh_width = Get_DISP_HSYNC_WIDTH() / 2 - 1;
        sfg_sfg_ctrl_26_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL() / 2 - 1;
    }
    else
    {
        sfg_sfg_ctrl_26_reg.dv_width = Get_DISP_VSYNC_LENGTH();
        sfg_sfg_ctrl_26_reg.dh_width = Get_DISP_HSYNC_WIDTH() - 1;
        sfg_sfg_ctrl_26_reg.dh_total = Get_DISP_HORIZONTAL_TOTAL() - 1;
    }



    rtd_outl(SFG_SFG_CTRL_26_reg, sfg_sfg_ctrl_26_reg.regValue);



    sfg_sfg_ctrl_c_reg.regValue = rtd_inl(SFG_SFG_CTRL_C_reg);
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        sfg_sfg_ctrl_c_reg.hs_io_dist = Get_DISP_DEN_END_HPOS() / 2;
        sfg_sfg_ctrl_c_reg.hs_den_io_dist = Get_DISP_DEN_STA_HPOS() / 2;
    }
    else
    {
        sfg_sfg_ctrl_c_reg.hs_io_dist = Get_DISP_DEN_END_HPOS();
        sfg_sfg_ctrl_c_reg.hs_den_io_dist = Get_DISP_DEN_STA_HPOS();
    }
    rtd_outl(SFG_SFG_CTRL_C_reg, sfg_sfg_ctrl_c_reg.regValue);



    sfg_ctrl_6_reg.regValue = rtd_inl(SFG_SFG_CTRL_6_reg);
    sfg_ctrl_6_reg.downsample_v = 0;
    rtd_outl(SFG_SFG_CTRL_6_reg, sfg_ctrl_6_reg.regValue);



    sfg_ctrl_0_reg.regValue = rtd_inl(SFG_SFG_CTRL_0_reg);
    if(get_panel_pixel_mode() > PANEL_1_PIXEL)
    {
        sfg_ctrl_0_reg.pif_pixel_mode = 1;
    }
    else
    {
        sfg_ctrl_0_reg.pif_pixel_mode = 0;
    }



    rtd_outl(SFG_SFG_CTRL_0_reg, sfg_ctrl_0_reg.regValue);



    //wait for timing stable
    msleep(60);
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
            else if(customerIndex == 228 || customerIndex == 233 || customerIndex == 235
                 || customerIndex == 214 || customerIndex == 230 || customerIndex == 244)
            {
                panel_tcon_dlg_4k2K60_4k1k120(dlgMode, customerIndex);
            }
            break;

        case PANEL_DLG_4k1k240:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2k120_4K1K240)
            {
                panel_vby1_dlg_4k2K120_4k1k240(dlgMode);
            }
            else if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
            break;

        case PANEL_DLG_4k1k120://tcl dlg mode use this case
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2K60_4K1K120)
            {
                panel_vby1_dlg_4k2K60_4k2k120(dlgMode);
            }
            else if(customerIndex == 228 || customerIndex == 233 || customerIndex == 235
                 || customerIndex == 214 || customerIndex == 230 || customerIndex == 244)
            {
                panel_tcon_dlg_4k2K60_4k1k120(dlgMode,customerIndex);
            }
            break;

        case PANEL_DLG_4k2k144:
            if(customerIndex == VBY_ONE_PANEL_HSR_4K2K144_4K1K288)
            {
                 panel_vby1_dlg_4k2K144_4k1k288(dlgMode);
            }
            break;

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
            HDMI_TIMING_T vfehdmitiminginfo = {0};
            drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
            if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k240_MIN_VFREQ)
                && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k240_MAX_V_TOTAL))
            {
                panelDlgMode  = PANEL_DLG_4k1k240;
            }
        }
    }
    else if(hsrMode == PANEL_HSR_4k1k120)//tcl dlg mode use this hsr mode
    {
        if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
        {
            HDMI_TIMING_T vfehdmitiminginfo = {0};
            drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
            if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k120_MIN_VFREQ)
            && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k120_MAX_V_TOTAL))
            {
                panelDlgMode  = PANEL_DLG_4k1k120;
            }
            else if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k120_MIN_VFREQ)
                && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k120_MAX_V_TOTAL)
                && (vfehdmitiminginfo.v_total > PANEL_HSR_4k1k120_MIN_V_TOTAL)
                && (vfehdmitiminginfo.h_total < PANEL_HSR_4k1k120_MAX_H_TOTAL)
                && (vfehdmitiminginfo.h_total > PANEL_HSR_4k1k120_MIN_H_TOTAL))
            {
               panelDlgMode  = PANEL_DLG_4k1k120;
            }
        }
    }
    else if(hsrMode == PANEL_HSR_4k1k288)
    {
        if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
        {
            HDMI_TIMING_T vfehdmitiminginfo = {0};
            drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
            if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k288_MIN_VFREQ)
                && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k288_MAX_V_TOTAL))
            {
                //hdmi 2k1k288 input support support 4k1k288 output
                panelDlgMode  = PANEL_DLG_4k1k288;
            }
            else if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k240_MIN_VFREQ)
                && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k240_MAX_V_TOTAL))
            {
                panelDlgMode  = PANEL_DLG_4k1k240;
            }
            else if((vfehdmitiminginfo.v_freq > PANEL_DLG_4k2k144_MIN_VFREQ)
                && (vfehdmitiminginfo.v_freq < PANEL_DLG_4k2k144_MAX_VFREQ))
            {
                //hdmi 144 input support support 144 output
                panelDlgMode = PANEL_DLG_4k2k144;
            }
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
    else
    {
        if(hsrMode == PANEL_HSR_4k1k288)
        {
            if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
            {
                HDMI_TIMING_T vfehdmitiminginfo = {0};
                drvif_Hdmi_GetRawTimingInfo(&vfehdmitiminginfo);
                if((vfehdmitiminginfo.v_freq > PANEL_HSR_4k1k288_MIN_VFREQ)
                    && (vfehdmitiminginfo.v_total < PANEL_HSR_4k1k288_MAX_V_TOTAL))
                {
                    //hsr disable hdmi 2k1k288 input support support 4k2k144 output
                    panelDlgMode  = PANEL_DLG_4k2k144;
                }
                else if((vfehdmitiminginfo.v_freq > PANEL_DLG_4k2k144_MIN_VFREQ)
                    && (vfehdmitiminginfo.v_freq < PANEL_DLG_4k2k144_MAX_VFREQ))
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

extern unsigned char update_dlg_Setting;

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

	update_dlg_Setting = 1;
    s_panelDlgMode = dlgMode;
    dlgCallbackFunc(E_DLG_WAIT_MEMC_DTG_PORCH);
    dlg_sfg_inner_ptg(1);
	frameChangeStep = 0;
	drvif_color_od_enable_set(0);

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
        dlgCallbackFunc(E_DLG_DTG_FRACTION);
        //dlgCallbackFunc(E_DLG_MEMC);
        dlgCallbackFunc(E_DLG_ENABLE_OSD);
        dlgCallbackFunc(E_DLG_SET_PRINT_DLG_MESSAGE_CNT);
        dlgCallbackFunc(E_DLG_DISABLE_DLG_MASK);
    }
	fwif_color_dlg_trigger();

    dlg_sfg_inner_ptg(0);
	update_dlg_Setting = 0;
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
                        "PANEL_DLG_4k2k120", "PANEL_DLG_4k2k144",
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
        //rtd_pr_vbe_info("PANEL_CUSTOM_INDEX: %d\n", Get_DISPLAY_PANEL_CUSTOM_INDEX());
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
