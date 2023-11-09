#include <linux/init.h>
#include <io.h>

#include <rbus/hdmitx_phy_reg.h>
#include <rbus/hdmitx20_mac1_reg.h>
#include <rbus/dsce_misc_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/scaleup3_reg.h>
#include <rbus/two_step_uzu_reg.h>
#include <rbus/dispd_ds444_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/pll27x_reg_reg.h>

#include <rbus/dsce_reg.h>
#include <rbus/hdmitx_misc_reg.h>
#include <rbus/hdmitx20_mac0_reg.h>
#include <rbus/hdmitx20_on_region_reg.h>
#include <rbus/hdmitx21_mac_reg.h>

#include <rtk_kdriver/pcbMgr.h>
#include <tvscalercontrol/hdmitx/hdmitx_lib.h>
#include <tvscalercontrol/hdmitx/hdmitx.h>
#include <tvscalercontrol/hdmitx/hdmitx_common.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <scaler_vbedev.h>
#include <scaler/scalerCommon.h>

#include <mach/platform.h>
#include <mach/rtk_platform.h>

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#include <rtk_kdriver/RPCDriver.h>
#include <VideoRPC_System.h>
#endif

// [MARK2] module init
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/kthread.h>

// panel parameter init
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/panel/panel.h>

PANEL_CONFIG_PARAMETER hdmitxPanelParameter;

static bool hdmitx_tsk_running_flag = FALSE;//Record vsc_scaler_tsk status. True: Task is running
static struct task_struct *p_hdmitx_tsk = NULL;

static unsigned char bHdmiTxRun_Flag=0;

static unsigned char bHdmiTxAudio_ch=0;
static unsigned char bHdmiTxAudio_type=0;
static unsigned char bHdmiTxAudio_mute=0;
static unsigned int HdmiTxAudio_freq=0;


#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
extern void Scaler_MEMC_outMux(unsigned char dbuf_en,unsigned char outmux_sel);
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr; // debug scaler flag tracker
extern unsigned char vsc_force_rerun_main_scaler;

#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
typedef enum{
    HDMITX_CRC_UZU,
    HDMITX_CRC_DSCE,
    HDMITX_CRC_MISC,
    HDMITX_CRC_HDMI20TX,
    HDMITX_CRC_HDMI21TX,
    HDMITX_CRC_LIST_NUM,
} ENUM_HDMITX_CRC_LIST;

#define HDMITX_CRC_CH_NUM 3
typedef struct {
    unsigned int crc_ch[HDMITX_CRC_CH_NUM]; // CRC in 3 channel
} DS_HDMITX_CRC_TYPE;
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP

extern void fw_scaler_dtg_double_buffer_apply(void);
extern EnumHdmi21FrlType ScalerHdmiTxGetTargetFrlRate(void);
extern void ScalerHdmiTxSetTimingStandardType(EnumHdmitxTimingStdType stdType);
extern EnumHdmitxTimingStdType ScalerHdmiTxGetTimingStandardType(void);
extern void ScalerHdmiMacTx0DataIslandPacking(void);
extern void ScalerHdmiTxSetMute(unsigned char enable);
extern void ScalerHdmiTxPhy_SetFllTracking(unsigned char enable);
extern void ScalerHdmiTx0_H5xDsc_inputSrcConfig(EnumHDMITXDscSrc dsc_src);
extern void ScalerHdmiTxSetVTEMpacektEn(unsigned char enable);
extern void ScalerHdmiTxSetVRRFlagEn(unsigned char enable);

void ScalerHdmiTxSetTxRunFlag(unsigned char enable)
{
    if(bHdmiTxRun_Flag != enable){
        rtd_pr_hdmi_err("[HDMITX] bHdmiTxRun_Flag=%d\n", enable);
        bHdmiTxRun_Flag = enable;
    }
    return;
}

void ScalerHdmiTx_reset_FRL(void)
{
    // frl_en=0, frl_reset=1
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
    rtd_maskl(HDMITX21_MAC_HDMI21_FRL_0_reg,~(HDMITX21_MAC_HDMI21_FRL_0_frl_en_mask|HDMITX21_MAC_HDMI21_FRL_0_frl_reset_mask), HDMITX21_MAC_HDMI21_FRL_0_frl_reset_mask);//frl_en[7]=0, frl_reset[6]=1 (active mode)

    // frl_reset=0
    HDMITX_DTG_Wait_Den_Start((unsigned char*)__FUNCTION__, __LINE__);
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
    rtd_maskl(HDMITX21_MAC_HDMI21_FRL_0_reg, ~HDMITX21_MAC_HDMI21_FRL_0_frl_reset_mask, 0);//frl_reset[6]=0 (active mode)

    // frl_en=1
    HDMITX_DTG_Wait_Den_Start((unsigned char*)__FUNCTION__, __LINE__);
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
    rtd_maskl(HDMITX21_MAC_HDMI21_FRL_0_reg, ~HDMITX21_MAC_HDMI21_FRL_0_frl_en_mask, HDMITX21_MAC_HDMI21_FRL_0_frl_en_mask);//frl_en[7]=1
    return;
}

void ScalerHdmiTxSetTxPkt(unsigned int pktType)
{
    rtd_pr_hdmi_notice("[HDMITX] Set Packet Type[%d]\n", pktType);
    ScalerHdmiMacTx0DataIslandPacking();
    return;
}


void ScalerHdmiTxSetTxDump(unsigned int index)
{
    unsigned char frl_rate = ScalerHdmiTxGetTargetFrlRate();

    rtd_pr_hdmi_notice("[HDMITX] dump index[%d]@HDMI2.%c mode(FRL=%d)\n", index, (frl_rate != 0? '1': '0'), frl_rate);

    // dump TX setting
    rtd_pr_hdmi_notice("w 18000100 %8x\n", rtd_inl(0xb8000100));
    rtd_pr_hdmi_notice("w 18000120 %8x\n", rtd_inl(0xb8000120));
    rtd_pr_hdmi_notice("w 18000110 %8x\n", rtd_inl(0xb8000110));
    rtd_pr_hdmi_notice("w 18000104 %8x\n", rtd_inl(0xb8000104));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 1800010c %8x\n", rtd_inl(0xb800010c));
    rtd_pr_hdmi_notice("w 1800011c %8x\n", rtd_inl(0xb800011c));
    rtd_pr_hdmi_notice("w 180004e4 %8x\n", rtd_inl(0xb80004e4));
    rtd_pr_hdmi_notice("w 180004e4 %8x\n", rtd_inl(0xb80004e4));
    rtd_pr_hdmi_notice("w 18000124 %8x\n", rtd_inl(0xb8000124));
    rtd_pr_hdmi_notice("w 18000134 %8x\n", rtd_inl(0xb8000134));
    rtd_pr_hdmi_notice("w 18000414 %8x\n", rtd_inl(0xb8000414));
    rtd_pr_hdmi_notice("w 1800069c %8x\n", rtd_inl(0xb800069c));
    rtd_pr_hdmi_notice("w 18000c74 %8x\n", rtd_inl(0xb8000c74));
    rtd_pr_hdmi_notice("w 18000424 %8x\n", rtd_inl(0xb8000424));
    rtd_pr_hdmi_notice("w 180003b0 %8x\n", rtd_inl(0xb80003b0));
    rtd_pr_hdmi_notice("w 18000408 %8x\n", rtd_inl(0xb8000408));
    rtd_pr_hdmi_notice("w 18000410 %8x\n", rtd_inl(0xb8000410));
    rtd_pr_hdmi_notice("w 18000434 %8x\n", rtd_inl(0xb8000434));
    rtd_pr_hdmi_notice("w 180006c8 %8x\n", rtd_inl(0xb80006c8));
    rtd_pr_hdmi_notice("w 180006c0 %8x\n", rtd_inl(0xb80006c0));
    rtd_pr_hdmi_notice("w 180006a0 %8x\n", rtd_inl(0xb80006a0));
    rtd_pr_hdmi_notice("w 18000418 %8x\n", rtd_inl(0xb8000418));
    rtd_pr_hdmi_notice("w 18000474 %8x\n", rtd_inl(0xb8000474));
    rtd_pr_hdmi_notice("w 18000460 %8x\n", rtd_inl(0xb8000460));
    rtd_pr_hdmi_notice("w 180006a0 %8x\n", rtd_inl(0xb80006a0));
    rtd_pr_hdmi_notice("w 18000438 %8x\n", rtd_inl(0xb8000438));
    rtd_pr_hdmi_notice("w 18000480 %8x\n", rtd_inl(0xb8000480));
    rtd_pr_hdmi_notice("w 18000360 %8x\n", rtd_inl(0xb8000360));
    rtd_pr_hdmi_notice("w 18000428 %8x\n", rtd_inl(0xb8000428));
    rtd_pr_hdmi_notice("w 18000438 %8x\n", rtd_inl(0xb8000438));
    rtd_pr_hdmi_notice("w 18000480 %8x\n", rtd_inl(0xb8000480));
    rtd_pr_hdmi_notice("w 18000360 %8x\n", rtd_inl(0xb8000360));
    rtd_pr_hdmi_notice("w 18000420 %8x\n", rtd_inl(0xb8000420));
    rtd_pr_hdmi_notice("w 18000430 %8x\n", rtd_inl(0xb8000430));
    rtd_pr_hdmi_notice("w 18000424 %8x\n", rtd_inl(0xb8000424));
    rtd_pr_hdmi_notice("w 18000434 %8x\n", rtd_inl(0xb8000434));
    rtd_pr_hdmi_notice("w 18000474 %8x\n", rtd_inl(0xb8000474));
    rtd_pr_hdmi_notice("w 18000354 %8x\n", rtd_inl(0xb8000354));
    rtd_pr_hdmi_notice("w 18000424 %8x\n", rtd_inl(0xb8000424));
    rtd_pr_hdmi_notice("w 18000434 %8x\n", rtd_inl(0xb8000434));
    rtd_pr_hdmi_notice("w 18000474 %8x\n", rtd_inl(0xb8000474));
    rtd_pr_hdmi_notice("w 18000354 %8x\n", rtd_inl(0xb8000354));
    rtd_pr_hdmi_notice("w 18000424 %8x\n", rtd_inl(0xb8000424));
    rtd_pr_hdmi_notice("w 18000434 %8x\n", rtd_inl(0xb8000434));
    rtd_pr_hdmi_notice("w 18000474 %8x\n", rtd_inl(0xb8000474));
    rtd_pr_hdmi_notice("w 18000354 %8x\n", rtd_inl(0xb8000354));
    rtd_pr_hdmi_notice("w 180003b4 %8x\n", rtd_inl(0xb80003b4));
    rtd_pr_hdmi_notice("w 1800040c %8x\n", rtd_inl(0xb800040c));
    rtd_pr_hdmi_notice("w 18000414 %8x\n", rtd_inl(0xb8000414));
    rtd_pr_hdmi_notice("w 1800041c %8x\n", rtd_inl(0xb800041c));
    rtd_pr_hdmi_notice("w 18000464 %8x\n", rtd_inl(0xb8000464));
    rtd_pr_hdmi_notice("w 180003b4 %8x\n", rtd_inl(0xb80003b4));
    rtd_pr_hdmi_notice("w 1800040c %8x\n", rtd_inl(0xb800040c));
    rtd_pr_hdmi_notice("w 18000414 %8x\n", rtd_inl(0xb8000414));
    rtd_pr_hdmi_notice("w 1800041c %8x\n", rtd_inl(0xb800041c));
    rtd_pr_hdmi_notice("w 18000464 %8x\n", rtd_inl(0xb8000464));
    rtd_pr_hdmi_notice("w 180003b4 %8x\n", rtd_inl(0xb80003b4));
    rtd_pr_hdmi_notice("w 1800040c %8x\n", rtd_inl(0xb800040c));
    rtd_pr_hdmi_notice("w 18000414 %8x\n", rtd_inl(0xb8000414));
    rtd_pr_hdmi_notice("w 1800041c %8x\n", rtd_inl(0xb800041c));
    rtd_pr_hdmi_notice("w 18000464 %8x\n", rtd_inl(0xb8000464));
    rtd_pr_hdmi_notice("w 180004b8 %8x\n", rtd_inl(0xb80004b8));
    rtd_pr_hdmi_notice("w 180004d8 %8x\n", rtd_inl(0xb80004d8));
    rtd_pr_hdmi_notice("w 180004dc %8x\n", rtd_inl(0xb80004dc));
    rtd_pr_hdmi_notice("w 180004dc %8x\n", rtd_inl(0xb80004dc));
    rtd_pr_hdmi_notice("w 180004dc %8x\n", rtd_inl(0xb80004dc));
    rtd_pr_hdmi_notice("w 180004b4 %8x\n", rtd_inl(0xb80004b4));
    rtd_pr_hdmi_notice("w 180004c8 %8x\n", rtd_inl(0xb80004c8));
    rtd_pr_hdmi_notice("w 180004b4 %8x\n", rtd_inl(0xb80004b4));
    rtd_pr_hdmi_notice("w 180004c8 %8x\n", rtd_inl(0xb80004c8));
    rtd_pr_hdmi_notice("w 180004ec %8x\n", rtd_inl(0xb80004ec));
    rtd_pr_hdmi_notice("w 180006a0 %8x\n", rtd_inl(0xb80006a0));
    rtd_pr_hdmi_notice("w 1800069c %8x\n", rtd_inl(0xb800069c));
    rtd_pr_hdmi_notice("w 18000200 %8x\n", rtd_inl(0xb8000200));
    rtd_pr_hdmi_notice("w 18000110 %8x\n", rtd_inl(0xb8000110));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 18000118 %8x\n", rtd_inl(0xb8000118));
    rtd_pr_hdmi_notice("w 1800011c %8x\n", rtd_inl(0xb800011c));
    rtd_pr_hdmi_notice("w 18000130 %8x\n", rtd_inl(0xb8000130));
    rtd_pr_hdmi_notice("w 18000110 %8x\n", rtd_inl(0xb8000110));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 18000118 %8x\n", rtd_inl(0xb8000118));
    rtd_pr_hdmi_notice("w 1800011c %8x\n", rtd_inl(0xb800011c));
    rtd_pr_hdmi_notice("w 18000130 %8x\n", rtd_inl(0xb8000130));
    rtd_pr_hdmi_notice("w 18000100 %8x\n", rtd_inl(0xb8000100));
    rtd_pr_hdmi_notice("w 18000120 %8x\n", rtd_inl(0xb8000120));
    rtd_pr_hdmi_notice("w 18000104 %8x\n", rtd_inl(0xb8000104));
    rtd_pr_hdmi_notice("w 18000108 %8x\n", rtd_inl(0xb8000108));
    rtd_pr_hdmi_notice("w 1800010c %8x\n", rtd_inl(0xb800010c));
    rtd_pr_hdmi_notice("w 18000110 %8x\n", rtd_inl(0xb8000110));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 18000118 %8x\n", rtd_inl(0xb8000118));
    rtd_pr_hdmi_notice("w 1800011c %8x\n", rtd_inl(0xb800011c));
    rtd_pr_hdmi_notice("w 18000130 %8x\n", rtd_inl(0xb8000130));
    rtd_pr_hdmi_notice("w 18000c74 %8x\n", rtd_inl(0xb8000c74));
    rtd_pr_hdmi_notice("w 18000124 %8x\n", rtd_inl(0xb8000124));
    rtd_pr_hdmi_notice("w 18000134 %8x\n", rtd_inl(0xb8000134));
    rtd_pr_hdmi_notice("w 18000208 %8x\n", rtd_inl(0xb8000208));
    rtd_pr_hdmi_notice("w 18000110 %8x\n", rtd_inl(0xb8000110));
    rtd_pr_hdmi_notice("w 18000130 %8x\n", rtd_inl(0xb8000130));
    rtd_pr_hdmi_notice("w 18000130 %8x\n", rtd_inl(0xb8000130));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 18000208 %8x\n", rtd_inl(0xb8000208));
    rtd_pr_hdmi_notice("w 18000234 %8x\n", rtd_inl(0xb8000234));
    rtd_pr_hdmi_notice("w 18000208 %8x\n", rtd_inl(0xb8000208));
    rtd_pr_hdmi_notice("w 18000230 %8x\n", rtd_inl(0xb8000230));
    rtd_pr_hdmi_notice("w 18000234 %8x\n", rtd_inl(0xb8000234));
    rtd_pr_hdmi_notice("w 1804a8f8 %8x\n", rtd_inl(0xb804a8f8));
    rtd_pr_hdmi_notice("w 1802811c %8x\n", rtd_inl(0xb802811c));
    rtd_pr_hdmi_notice("w 1802ac00 %8x\n", rtd_inl(0xb802ac00));
    rtd_pr_hdmi_notice("w 1802a800 %8x\n", rtd_inl(0xb802a800));
    rtd_pr_hdmi_notice("w 1802ac00 %8x\n", rtd_inl(0xb802ac00));
    rtd_pr_hdmi_notice("w 1802d988 %8x\n", rtd_inl(0xb802d988));
    rtd_pr_hdmi_notice("w 1802d900 %8x\n", rtd_inl(0xb802d900));
    rtd_pr_hdmi_notice("w 180004b4 %8x\n", rtd_inl(0xb80004b4));
    rtd_pr_hdmi_notice("w 180004bc %8x\n", rtd_inl(0xb80004bc));
    rtd_pr_hdmi_notice("w 180004e4 %8x\n", rtd_inl(0xb80004e4));
    rtd_pr_hdmi_notice("w 18000114 %8x\n", rtd_inl(0xb8000114));
    rtd_pr_hdmi_notice("w 18000118 %8x\n", rtd_inl(0xb8000118));
    rtd_pr_hdmi_notice("w 1800011c %8x\n", rtd_inl(0xb800011c));
    rtd_pr_hdmi_notice("w 18006000 %8x\n", rtd_inl(0xb8006000));
    rtd_pr_hdmi_notice("w 18006030 %8x\n", rtd_inl(0xb8006030));
    rtd_pr_hdmi_notice("w 18006034 %8x\n", rtd_inl(0xb8006034));
    rtd_pr_hdmi_notice("w 18006000 %8x\n", rtd_inl(0xb8006000));
    rtd_pr_hdmi_notice("w 1804c044 %8x\n", rtd_inl(0xb804c044));
    rtd_pr_hdmi_notice("w 1804a800 %8x\n", rtd_inl(0xb804a800));
    rtd_pr_hdmi_notice("w 1804a808 %8x\n", rtd_inl(0xb804a808));
    rtd_pr_hdmi_notice("w 1804a80c %8x\n", rtd_inl(0xb804a80c));
    rtd_pr_hdmi_notice("w 1804a814 %8x\n", rtd_inl(0xb804a814));
    rtd_pr_hdmi_notice("w 1804a818 %8x\n", rtd_inl(0xb804a818));
    rtd_pr_hdmi_notice("w 1804a81c %8x\n", rtd_inl(0xb804a81c));
    rtd_pr_hdmi_notice("w 1804a820 %8x\n", rtd_inl(0xb804a820));
    rtd_pr_hdmi_notice("w 1804a828 %8x\n", rtd_inl(0xb804a828));
    rtd_pr_hdmi_notice("w 1804a830 %8x\n", rtd_inl(0xb804a830));
    rtd_pr_hdmi_notice("w 1804a844 %8x\n", rtd_inl(0xb804a844));
    rtd_pr_hdmi_notice("w 1804a85c %8x\n", rtd_inl(0xb804a85c));
    rtd_pr_hdmi_notice("w 1804a8b0 %8x\n", rtd_inl(0xb804a8b0));
    rtd_pr_hdmi_notice("w 1804a8b4 %8x\n", rtd_inl(0xb804a8b4));
    rtd_pr_hdmi_notice("w 1804a8ac %8x\n", rtd_inl(0xb804a8ac));
    rtd_pr_hdmi_notice("w 1804a804 %8x\n", rtd_inl(0xb804a804));
    rtd_pr_hdmi_notice("w 1804a824 %8x\n", rtd_inl(0xb804a824));
    rtd_pr_hdmi_notice("w 1804a844 %8x\n", rtd_inl(0xb804a844));
    rtd_pr_hdmi_notice("w 1804a824 %8x\n", rtd_inl(0xb804a824));
    rtd_pr_hdmi_notice("w 1804a818 %8x\n", rtd_inl(0xb804a818));
    rtd_pr_hdmi_notice("w 1804a828 %8x\n", rtd_inl(0xb804a828));
    rtd_pr_hdmi_notice("w 1804a81c %8x\n", rtd_inl(0xb804a81c));
    rtd_pr_hdmi_notice("w 1804a8f8 %8x\n", rtd_inl(0xb804a8f8));
    rtd_pr_hdmi_notice("w 1804a8f8 %8x\n", rtd_inl(0xb804a8f8));
    rtd_pr_hdmi_notice("w 1804a8ac %8x\n", rtd_inl(0xb804a8ac));
    rtd_pr_hdmi_notice("w 1804a8ac %8x\n", rtd_inl(0xb804a8ac));
    rtd_pr_hdmi_notice("w 18029000 %8x\n", rtd_inl(0xb8029000));
    rtd_pr_hdmi_notice("w 18029000 %8x\n", rtd_inl(0xb8029000));
    rtd_pr_hdmi_notice("w 18029074 %8x\n", rtd_inl(0xb8029074));
    rtd_pr_hdmi_notice("w 18029028 %8x\n", rtd_inl(0xb8029028));
    rtd_pr_hdmi_notice("w 18029b2c %8x\n", rtd_inl(0xb8029b2c));
    rtd_pr_hdmi_notice("w 18029074 %8x\n", rtd_inl(0xb8029074));
    rtd_pr_hdmi_notice("w 1804b48c %8x\n", rtd_inl(0xb804b48c));
    rtd_pr_hdmi_notice("w 18028540 %8x\n", rtd_inl(0xb8028540));
    rtd_pr_hdmi_notice("w 18028544 %8x\n", rtd_inl(0xb8028544));
    rtd_pr_hdmi_notice("w 18028548 %8x\n", rtd_inl(0xb8028548));
    rtd_pr_hdmi_notice("w 1802854c %8x\n", rtd_inl(0xb802854c));
    rtd_pr_hdmi_notice("w 18028550 %8x\n", rtd_inl(0xb8028550));
    rtd_pr_hdmi_notice("w 18028554 %8x\n", rtd_inl(0xb8028554));
    rtd_pr_hdmi_notice("w 18028300 %8x\n", rtd_inl(0xb8028300));
    rtd_pr_hdmi_notice("w 18028008 %8x\n", rtd_inl(0xb8028008));
    rtd_pr_hdmi_notice("w 18028518 %8x\n", rtd_inl(0xb8028518));
    rtd_pr_hdmi_notice("w 1802800c %8x\n", rtd_inl(0xb802800c));
    rtd_pr_hdmi_notice("w 18028010 %8x\n", rtd_inl(0xb8028010));
    rtd_pr_hdmi_notice("w 1802851c %8x\n", rtd_inl(0xb802851c));
    rtd_pr_hdmi_notice("w 18028014 %8x\n", rtd_inl(0xb8028014));
    rtd_pr_hdmi_notice("w 18028504 %8x\n", rtd_inl(0xb8028504));
    rtd_pr_hdmi_notice("w 18028508 %8x\n", rtd_inl(0xb8028508));
    rtd_pr_hdmi_notice("w 18028500 %8x\n", rtd_inl(0xb8028500));
    rtd_pr_hdmi_notice("w 18028604 %8x\n", rtd_inl(0xb8028604));
    rtd_pr_hdmi_notice("w 18028608 %8x\n", rtd_inl(0xb8028608));
    rtd_pr_hdmi_notice("w 18028618 %8x\n", rtd_inl(0xb8028618));
    rtd_pr_hdmi_notice("w 1802861c %8x\n", rtd_inl(0xb802861c));
    rtd_pr_hdmi_notice("w 18028600 %8x\n", rtd_inl(0xb8028600));
    rtd_pr_hdmi_notice("w 18028704 %8x\n", rtd_inl(0xb8028704));
    rtd_pr_hdmi_notice("w 18028708 %8x\n", rtd_inl(0xb8028708));
    rtd_pr_hdmi_notice("w 18028718 %8x\n", rtd_inl(0xb8028718));
    rtd_pr_hdmi_notice("w 1802871c %8x\n", rtd_inl(0xb802871c));
    rtd_pr_hdmi_notice("w 18028700 %8x\n", rtd_inl(0xb8028700));
    rtd_pr_hdmi_notice("w 18028000 %8x\n", rtd_inl(0xb8028000));
    rtd_pr_hdmi_notice("w 18028008 %8x\n", rtd_inl(0xb8028008));
    rtd_pr_hdmi_notice("w 18028010 %8x\n", rtd_inl(0xb8028010));
    rtd_pr_hdmi_notice("w 18028028 %8x\n", rtd_inl(0xb8028028));
    rtd_pr_hdmi_notice("w 18028000 %8x\n", rtd_inl(0xb8028000));
    rtd_pr_hdmi_notice("w 18028000 %8x\n", rtd_inl(0xb8028000));
    rtd_pr_hdmi_notice("w 1804b40c %8x\n", rtd_inl(0xb804b40c));

    if(ScalerHdmiTxGetTargetFrlRate() != 0){ // HDMI2.1
        rtd_pr_hdmi_notice("w 1804cb04 %8x\n", rtd_inl(0xb804cb04));
        rtd_pr_hdmi_notice("w 1804cb0c %8x\n", rtd_inl(0xb804cb0c));
        rtd_pr_hdmi_notice("w 1804cb1c %8x\n", rtd_inl(0xb804cb1c));
        rtd_pr_hdmi_notice("w 1804cb20 %8x\n", rtd_inl(0xb804cb20));
        rtd_pr_hdmi_notice("w 1804cb24 %8x\n", rtd_inl(0xb804cb24));
        rtd_pr_hdmi_notice("w 1804cb3c %8x\n", rtd_inl(0xb804cb3c));
        rtd_pr_hdmi_notice("w 1804cb4c %8x\n", rtd_inl(0xb804cb4c));
        rtd_pr_hdmi_notice("w 1804cb50 %8x\n", rtd_inl(0xb804cb50));
        rtd_pr_hdmi_notice("w 1804cb54 %8x\n", rtd_inl(0xb804cb54));
        rtd_pr_hdmi_notice("w 1804cb5c %8x\n", rtd_inl(0xb804cb5c));
        rtd_pr_hdmi_notice("w 1804cb88 %8x\n", rtd_inl(0xb804cb88));
        rtd_pr_hdmi_notice("w 1804cb00 %8x\n", rtd_inl(0xb804cb00));
        rtd_pr_hdmi_notice("w 1804cb10 %8x\n", rtd_inl(0xb804cb10));
        rtd_pr_hdmi_notice("w 1804cb84 %8x\n", rtd_inl(0xb804cb84));
        rtd_pr_hdmi_notice("w 1804c410 %8x\n", rtd_inl(0xb804c410));
        rtd_pr_hdmi_notice("w 1804c7c0 %8x\n", rtd_inl(0xb804c7c0));
        rtd_pr_hdmi_notice("w 1804c044 %8x\n", rtd_inl(0xb804c044));
        rtd_pr_hdmi_notice("w 1804c418 %8x\n", rtd_inl(0xb804c418));
        rtd_pr_hdmi_notice("w 1804c41c %8x\n", rtd_inl(0xb804c41c));
        rtd_pr_hdmi_notice("w 1804c420 %8x\n", rtd_inl(0xb804c420));
        rtd_pr_hdmi_notice("w 1804c424 %8x\n", rtd_inl(0xb804c424));
        rtd_pr_hdmi_notice("w 1804c428 %8x\n", rtd_inl(0xb804c428));
        rtd_pr_hdmi_notice("w 1804c42c %8x\n", rtd_inl(0xb804c42c));
        rtd_pr_hdmi_notice("w 1804c430 %8x\n", rtd_inl(0xb804c430));
        rtd_pr_hdmi_notice("w 1804c434 %8x\n", rtd_inl(0xb804c434));
        rtd_pr_hdmi_notice("w 1804c580 %8x\n", rtd_inl(0xb804c580));
        rtd_pr_hdmi_notice("w 1804c58c %8x\n", rtd_inl(0xb804c58c));
        rtd_pr_hdmi_notice("w 1804c590 %8x\n", rtd_inl(0xb804c590));
        rtd_pr_hdmi_notice("w 1804c594 %8x\n", rtd_inl(0xb804c594));
        rtd_pr_hdmi_notice("w 1804c598 %8x\n", rtd_inl(0xb804c598));
        rtd_pr_hdmi_notice("w 1804c5ac %8x\n", rtd_inl(0xb804c5ac));
        rtd_pr_hdmi_notice("w 1804c5b4 %8x\n", rtd_inl(0xb804c5b4));
        rtd_pr_hdmi_notice("w 1804c59c %8x\n", rtd_inl(0xb804c59c));
        rtd_pr_hdmi_notice("w 1804c490 %8x\n", rtd_inl(0xb804c490));
        rtd_pr_hdmi_notice("w 1804c4c4 %8x\n", rtd_inl(0xb804c4c4));
        rtd_pr_hdmi_notice("w 1804c4cc %8x\n", rtd_inl(0xb804c4cc));
        rtd_pr_hdmi_notice("w 1804c4d0 %8x\n", rtd_inl(0xb804c4d0));
        rtd_pr_hdmi_notice("w 1804c4e4 %8x\n", rtd_inl(0xb804c4e4));
        rtd_pr_hdmi_notice("w 1804c588 %8x\n", rtd_inl(0xb804c588));
        rtd_pr_hdmi_notice("w 1804c580 %8x\n", rtd_inl(0xb804c580));
        rtd_pr_hdmi_notice("w 1804c6a8 %8x\n", rtd_inl(0xb804c6a8));
        rtd_pr_hdmi_notice("w 1804c6c0 %8x\n", rtd_inl(0xb804c6c0));
        rtd_pr_hdmi_notice("w 1804c6cc %8x\n", rtd_inl(0xb804c6cc));
        rtd_pr_hdmi_notice("w 1804c780 %8x\n", rtd_inl(0xb804c780));
        rtd_pr_hdmi_notice("w 1804c790 %8x\n", rtd_inl(0xb804c790));
        rtd_pr_hdmi_notice("w 1804c7c0 %8x\n", rtd_inl(0xb804c7c0));
        rtd_pr_hdmi_notice("w 180bb2d0 %8x\n", rtd_inl(0xb80bb2d0));
        rtd_pr_hdmi_notice("w 1804c894 %8x\n", rtd_inl(0xb804c894));
        rtd_pr_hdmi_notice("w 1804c898 %8x\n", rtd_inl(0xb804c898));
        rtd_pr_hdmi_notice("w 1804c89c %8x\n", rtd_inl(0xb804c89c));
        rtd_pr_hdmi_notice("w 1804c8a0 %8x\n", rtd_inl(0xb804c8a0));
        rtd_pr_hdmi_notice("w 1804c8a4 %8x\n", rtd_inl(0xb804c8a4));
        rtd_pr_hdmi_notice("w 1804c8a8 %8x\n", rtd_inl(0xb804c8a8));
        rtd_pr_hdmi_notice("w 1804c8ac %8x\n", rtd_inl(0xb804c8ac));
        rtd_pr_hdmi_notice("w 1804c8b4 %8x\n", rtd_inl(0xb804c8b4));
        rtd_pr_hdmi_notice("w 1804c8b8 %8x\n", rtd_inl(0xb804c8b8));
        rtd_pr_hdmi_notice("w 1804c8bc %8x\n", rtd_inl(0xb804c8bc));
        rtd_pr_hdmi_notice("w 1804c8c0 %8x\n", rtd_inl(0xb804c8c0));
        rtd_pr_hdmi_notice("w 1804c8c4 %8x\n", rtd_inl(0xb804c8c4));
        rtd_pr_hdmi_notice("w 1804c8c8 %8x\n", rtd_inl(0xb804c8c8));
        rtd_pr_hdmi_notice("w 1804c8cc %8x\n", rtd_inl(0xb804c8cc));
        rtd_pr_hdmi_notice("w 1804c8d4 %8x\n", rtd_inl(0xb804c8d4));
        rtd_pr_hdmi_notice("w 1804c8d8 %8x\n", rtd_inl(0xb804c8d8));
        rtd_pr_hdmi_notice("w 1804c8dc %8x\n", rtd_inl(0xb804c8dc));
        rtd_pr_hdmi_notice("w 1804c8e0 %8x\n", rtd_inl(0xb804c8e0));
        rtd_pr_hdmi_notice("w 1804c8e4 %8x\n", rtd_inl(0xb804c8e4));
        rtd_pr_hdmi_notice("w 1804c8e8 %8x\n", rtd_inl(0xb804c8e8));
        rtd_pr_hdmi_notice("w 1804c8ec %8x\n", rtd_inl(0xb804c8ec));
        rtd_pr_hdmi_notice("w 1804c8f4 %8x\n", rtd_inl(0xb804c8f4));
        rtd_pr_hdmi_notice("w 1804c8f8 %8x\n", rtd_inl(0xb804c8f8));
        rtd_pr_hdmi_notice("w 1804c8fc %8x\n", rtd_inl(0xb804c8fc));
        rtd_pr_hdmi_notice("w 1804c900 %8x\n", rtd_inl(0xb804c900));
        rtd_pr_hdmi_notice("w 1804c904 %8x\n", rtd_inl(0xb804c904));
        rtd_pr_hdmi_notice("w 1804c908 %8x\n", rtd_inl(0xb804c908));
        rtd_pr_hdmi_notice("w 1804c90c %8x\n", rtd_inl(0xb804c90c));
        rtd_pr_hdmi_notice("w 1804c914 %8x\n", rtd_inl(0xb804c914));
        rtd_pr_hdmi_notice("w 1804c918 %8x\n", rtd_inl(0xb804c918));
        rtd_pr_hdmi_notice("w 1804c91c %8x\n", rtd_inl(0xb804c91c));
        rtd_pr_hdmi_notice("w 1804c944 %8x\n", rtd_inl(0xb804c944));
        rtd_pr_hdmi_notice("w 1804c948 %8x\n", rtd_inl(0xb804c948));
        rtd_pr_hdmi_notice("w 1804c940 %8x\n", rtd_inl(0xb804c940));
        rtd_pr_hdmi_notice("w 1804c940 %8x\n", rtd_inl(0xb804c940));
        rtd_pr_hdmi_notice("w 1804c938 %8x\n", rtd_inl(0xb804c938));
    }else{ // HDMI2.0
        rtd_pr_hdmi_notice("w 1804cc00 %8x\n", rtd_inl(0xb804cc00));
        rtd_pr_hdmi_notice("w 1804cc14 %8x\n", rtd_inl(0xb804cc14));
        rtd_pr_hdmi_notice("w 1804cc18 %8x\n", rtd_inl(0xb804cc18));
        rtd_pr_hdmi_notice("w 1804cc1c %8x\n", rtd_inl(0xb804cc1c));
        rtd_pr_hdmi_notice("w 1804cc10 %8x\n", rtd_inl(0xb804cc10));
        rtd_pr_hdmi_notice("w 1804cc24 %8x\n", rtd_inl(0xb804cc24));
        rtd_pr_hdmi_notice("w 1804cc2c %8x\n", rtd_inl(0xb804cc2c));
        rtd_pr_hdmi_notice("w 1804cc30 %8x\n", rtd_inl(0xb804cc30));
        rtd_pr_hdmi_notice("w 1804cc6c %8x\n", rtd_inl(0xb804cc6c));
        rtd_pr_hdmi_notice("w 1804ccb8 %8x\n", rtd_inl(0xb804ccb8));
        rtd_pr_hdmi_notice("w 1804cfc4 %8x\n", rtd_inl(0xb804cfc4));
        rtd_pr_hdmi_notice("w 1804ccc4 %8x\n", rtd_inl(0xb804ccc4));
        rtd_pr_hdmi_notice("w 1804cf6c %8x\n", rtd_inl(0xb804cf6c));
        rtd_pr_hdmi_notice("w 1804d1c0 %8x\n", rtd_inl(0xb804d1c0));
        rtd_pr_hdmi_notice("w 1804d140 %8x\n", rtd_inl(0xb804d140));
        rtd_pr_hdmi_notice("w 1804d144 %8x\n", rtd_inl(0xb804d144));
        rtd_pr_hdmi_notice("w 1804d148 %8x\n", rtd_inl(0xb804d148));
        rtd_pr_hdmi_notice("w 1804d168 %8x\n", rtd_inl(0xb804d168));
        rtd_pr_hdmi_notice("w 1804cc20 %8x\n", rtd_inl(0xb804cc20));
        rtd_pr_hdmi_notice("w 1804ccd8 %8x\n", rtd_inl(0xb804ccd8));
        rtd_pr_hdmi_notice("w 1804ccdc %8x\n", rtd_inl(0xb804ccdc));
        rtd_pr_hdmi_notice("w 1804cdb0 %8x\n", rtd_inl(0xb804cdb0));
        rtd_pr_hdmi_notice("w 1804d1d0 %8x\n", rtd_inl(0xb804d1d0));
        rtd_pr_hdmi_notice("w 1804d1cc %8x\n", rtd_inl(0xb804d1cc));
        rtd_pr_hdmi_notice("w 1804d1f8 %8x\n", rtd_inl(0xb804d1f8));
        rtd_pr_hdmi_notice("w 1804cf88 %8x\n", rtd_inl(0xb804cf88));
        rtd_pr_hdmi_notice("w 1804d1cc %8x\n", rtd_inl(0xb804d1cc));
    }

    rtd_pr_hdmi_notice("w 18028000 %8x\n", rtd_inl(0xb8028000));
    rtd_pr_hdmi_notice("w 1804c6cc %8x\n", rtd_inl(0xb804c6cc));
    rtd_pr_hdmi_notice("w 1804c4c4 %8x\n", rtd_inl(0xb804c4c4));
    rtd_pr_hdmi_notice("w 1804c40c %8x\n", rtd_inl(0xb804c40c));
    rtd_pr_hdmi_notice("w 1804c718 %8x\n", rtd_inl(0xb804c718));
    rtd_pr_hdmi_notice("w 1804c718 %8x\n", rtd_inl(0xb804c718));
    rtd_pr_hdmi_notice("w 180280f4 %8x\n", rtd_inl(0xb80280f4));
    rtd_pr_hdmi_notice("w 18028228 %8x\n", rtd_inl(0xb8028228));

    return;
}


unsigned char ScalerHdmiTxGetTxRunFlag(void)
{
    return bHdmiTxRun_Flag;
}

// [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
unsigned char HDMITX_wait_for_tgLineCount(unsigned int start, unsigned int end, unsigned char wait_zero)
{
	unsigned int timeoutcnt = 0x065000;
	unsigned int lineCnt=0, stc=IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	do {
		lineCnt = PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg));
		if(((lineCnt >= start) && (lineCnt < end))|| (wait_zero && (lineCnt == 0))) {
			return TRUE;
		}
	} while(timeoutcnt-- != 0);

	rtd_pr_hdmi_err("[HDMITX] Wait TG LC timeout, %d->%d(z=%d)@LC/diff=%d/%d\n", start, end, wait_zero, lineCnt, (IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)-stc)/90);
	return FALSE;
}


unsigned char HDMITX_DTG_Wait_vsync_start(unsigned char *func, unsigned int line) {
    unsigned char ret=0;
    unsigned int dv_len = PPOVERLAY_DV_Length_get_dv_length(rtd_inl(PPOVERLAY_DV_Length_reg));
    if(dv_len && (dv_len > 1))
        ret = HDMITX_wait_for_tgLineCount(1, dv_len, (PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg)) ? 1 : 0));
    else
        ret = HDMITX_wait_for_tgLineCount(0, 1, (PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg)) ? 1 : 0));

    if(ret == 0)
        rtd_pr_hdmi_err("[HDMITX] Wait v-sync start Fail From %s.%d\n", func, line);

    return ret;
}



unsigned char HDMITX_DTG_Wait_Den_Stop_Done(unsigned char *func, unsigned int line) {
    unsigned char ret=0;
    unsigned int vEnd = PPOVERLAY_DV_DEN_Start_End_get_dv_den_end(IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg));
    if(vEnd)
        ret = HDMITX_wait_for_tgLineCount(vEnd, 0x1fff, (PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg)) ? 1 : 0));
    else
        rtd_pr_hdmi_notice("[HDMITX] Wait vEnd=0\n");

    if(ret == 0)
        rtd_pr_hdmi_debug("[HDMITX] Wait Den Stop Fail From %s.%d\n", func, line);

    return ret;
}

unsigned char HDMITX_DTG_Wait_Den_Start(unsigned char *func, unsigned int line) {
    unsigned char ret=0;
    unsigned int vStart = PPOVERLAY_DV_DEN_Start_End_get_dv_den_sta(IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg));
    if(vStart)
        ret = HDMITX_wait_for_tgLineCount(vStart, 0x300, (PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg)) ? 1 : 0));
    else
        rtd_pr_hdmi_notice("[HDMITX] Wait vStart=0\n");

    if(ret == 0)
        rtd_pr_hdmi_debug("[HDMITX] Wait Den Start Fail From %s.%d\n", func, line);

    return ret;
}


void HDMITX_set_IvSrcSelChange(void) {
    unsigned int *ivSrcSelChange = NULL;
    rtd_pr_hdmi_debug("[txsoc] set iv_src_sel, 90k: %d", IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90);

    ivSrcSelChange = (unsigned int*)Scaler_GetShareMemVirAddr(IV_SRC_SEL_READY);


    *ivSrcSelChange = htonl(TRUE);
}

bool HDMITX_get_IvSrcSelChange(void) {
    unsigned int *ivSrcSelChange = NULL;

    ivSrcSelChange = (unsigned int*)Scaler_GetShareMemVirAddr(IV_SRC_SEL_READY);

    *ivSrcSelChange = htonl(*ivSrcSelChange);

    return *ivSrcSelChange;

}

void rpcVoIvSrcSel(int source) {
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
    long ret = 0;
    int timeout = 5;

    HDMITX_set_IvSrcSelChange();

    rtd_pr_hdmi_debug("[HDMITX]line: %d, start send rpc to vcpu, command: %d, source: %d\n", __LINE__, VIDEO_RPC_VOUT_FILTER_ToAgent_VODMA_ConfigVsyncGenIvSrc, source);
    //set_vo_nosignal_flag((port_local & 0xf), 1);
    if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_FILTER_ToAgent_VODMA_ConfigVsyncGenIvSrc, 0, source, &ret))
        rtd_pr_hdmi_err("RPC fail!!\n");

    if( ret == RPC_FAIL)
        rtd_pr_hdmi_err("[HDMITX]RPC fail!!\n");

    while(HDMITX_get_IvSrcSelChange() && timeout--) {
        msleep(10);
        rtd_pr_hdmi_info("[HDMITX] check iv_src_sel, 90k: %d", IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90);
    }
#endif

    return;
}



extern void ScalerHdmiTxSetFastTimingSwitchEn(unsigned char enable);
extern unsigned char ScalerHdmiTxGetFastTimingSwitchEn(void);
extern unsigned char ScalerHdmiTxGetStreamOffWithoutLinkTrainingFlag(void);
//extern HOST_PLAFTORM_TYPE ScalerHdmiTxGetH5DPlatformType(void);
extern void ScalerHdmiTxSetPixelFormatChange(unsigned int status);
extern unsigned int ScalerHdmiTxGetPixelFormatChange(void);
extern void ScalerHdmiTxSetChipVersion(unsigned char value);
extern unsigned char ScalerHdmiTxGetChipVersion(void);
extern unsigned char ScalerHdmiTxGetFrlNewModeEnable(void);
extern void ScalerHdmiTxSetFrlNewModeEnable(unsigned char enable);

void Scaler_FastTimingSwitch_VRR_4k2k_setting_by_framerate(int frame_rate);

unsigned int VO_ACT_H_START ;
unsigned int VO_ACT_V_START ;
unsigned int VO_H_BACKPORCH ;
unsigned int VO_V_BACKPORCH ;

unsigned int DTG_H_DEN_STA ;
unsigned int DTG_V_DEN_STA ;
unsigned int DTG_H_PORCH ;
unsigned int DTG_V_PORCH ;
unsigned int DTG_IV2DV_HSYNC_WIDTH = 0x320;
unsigned int DTG_DH_WIDTH ;
unsigned int DTG_DV_LENGTH ;

unsigned long long HDMI_TX_I2C_BUSID;

unsigned char HDMI_TIMING_FAST_SWITCH_EN=0;
unsigned int HDMI_TIMING_PIXEL_FMT_CHANGE=0x7;
static int last_frlType=-1;
static int last_pixelClk=0;
static int last_colorFmt=0, last_colorDepth=0;
static int last_frameRate=0;

static unsigned char vrrLoopMode=0;
static unsigned char vrrLoopDelay=0;
static unsigned char vrr24Hz=0;

//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
static unsigned char DBG_EN_TOGGLE_PTG=0;
static unsigned char DBG_EN_PTG=0;
//#endif

static int HDMITX_TIMING_TYPE = -1;

static unsigned char _FORCE_DSC_ENABLE=FALSE;

typedef enum
{
    PIXEL_COLOR = _BIT0,
    PIXEL_DEPTH = _BIT1,
    PIXEL_CLK = _BIT2,
}EnumPixelType;


typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int frame_rate;
    I3DDMA_COLOR_SPACE_T color_type;
    I3DDMA_COLOR_DEPTH_T color_depth;
    unsigned int frl_type;
    EnumHdmitxTimingStdType std_type;
    char timingName[40];
    bool    enable;
} hdmi_tx_output_timing_type;



/*
 * Discription : video timing info. table when selecting hdmi timing gen.
 * Table format : {x, {set1}, {set2}, {set3}} every row (video format)
 *  		x   : vic
 *  	{set1}  : {blank, active, sync, front, back} for h
 *  	{set2}  : {space, blank, act_video, active, sync, front, back} for v
 *         y    : pixelFreq (0.1 MHz)
 * Table index  :
 *      HDMI_TX_VIDEO_TIMING_NUM : see hdmi_tx_video_format_index
 */
hdmi_tx_timing_gen_st tx_cea_timing_table[TX_TIMING_NUM] = {
    /* *********** HDMI21 ******************** */
    /* HDMI2.1 4K2Kp60 444 */
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          //TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L (HDMI21_TX_3840_2160P_444_60HZ_6G4L) (HDMI2.1)

    /* HDMI2.1 8K4Kp60 420 */
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G (HDMI21_TX_7680_4320P_420_60HZ_8G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_60HZ_10G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_60HZ_10G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP60_YUV420_12BIT_12G (HDMI21_TX_7680_4320P_420_60HZ_12G) (HDMI2.1)

    /* HDMI2.1 8K4Kp59 420 */
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_8G (HDMI21_TX_7680_4320P_420_60HZ_8G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_60HZ_10G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_60HZ_10G) (HDMI2.1)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP59_YUV420_12BIT_12G (HDMI21_TX_7680_4320P_420_60HZ_12G) (HDMI2.1)

    /* HDMI2.1 8K4Kp23 420 */
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP23_YUV420_12BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_24HZ_10G)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_24HZ_10G)

    /* HDMI2.1 8K4Kp24 420 */
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP24_YUV420_12BIT_6G4L (HDMI21_TX_7680_4320P_420_24HZ_6G4L)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_24HZ_10G)
    {194, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 11880}, // TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_24HZ_10G)

    /* HDMI2.1 8K4Kp25 420 */
    {195, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_6G4L (HDMI21_TX_7680_4320P_420_25HZ_6G4L) (HDMI2.1)
    {195, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_6G4L (HDMI21_TX_7680_4320P_420_25HZ_6G4L) (HDMI2.1)
    {195, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP25_YUV420_12BIT_6G4L (HDMI21_TX_7680_4320P_420_25HZ_6G4L) (HDMI2.1)
    {195, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_25HZ_10G) (HDMI2.1)
    {195, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_25HZ_10G) (HDMI2.1)

    /* HDMI2.1 8K4Kp30 420 */
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV420_12BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_30HZ_10G) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_30HZ_10G) (HDMI2.1)

    /* HDMI2.1 8K4Kp29 420 */
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_30HZ_10G) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_30HZ_10G) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP29_YUV420_12BIT_6G4L (HDMI21_TX_7680_4320P_420_30HZ_6G4L) (HDMI2.1)

    /* HDMI2.1 8K4Kp30 444 */
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_10G (HDMI21_TX_7680_4320P_444_30HZ_10G) (HDMI2.1)
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},    //TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_12G (HDMI21_TX_7680_4320P_444_30HZ_12G) (HDMI2.1)

    /* HDMI2.1 8K4Kp47 420 */
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_8G (HDMI21_TX_7680_4320P_420_48HZ_8G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_48HZ_10G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP47_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_48HZ_10G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP47_YUV420_12BIT_12G (HDMI21_TX_7680_4320P_420_48HZ_12G)

    /* HDMI2.1 8K4Kp48 420 */
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_8G (HDMI21_TX_7680_4320P_420_48HZ_8G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_48HZ_10G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP48_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_48HZ_10G)
    {197, {3320, 7680, 176, 2552, 592}, {1, 180, 4320, 4320, 20, 16, 144}, 23760}, // TX_TIMING_HDMI21_8K4KP48_YUV420_12BIT_12G (HDMI21_TX_7680_4320P_420_48HZ_12G)

    /* HDMI2.1 8K4Kp50 420 */
    {198, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_8G (HDMI21_TX_7680_4320P_420_50HZ_8G) (HDMI2.1)
    {198, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_10G (HDMI21_TX_7680_4320P_420_50HZ_10G) (HDMI2.1)
    {198, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP50_YUV420_10BIT_10G (HDMI21_TX_7680_4320P_420_50HZ_10G) (HDMI2.1)
    {198, {3120, 7680, 176, 2352, 592}, {1,  80, 4320, 4320, 20, 16, 44}, 23760},    //TX_TIMING_HDMI21_8K4KP50_YUV420_12BIT_12G (HDMI21_TX_7680_4320P_420_50HZ_12G) (HDMI2.1)

    /* HDMI2.1 4K2Kp120 444 */
    {118, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 11880},	        //TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G (HDMI21_TX_3840_2160P_444_120HZ_8G) (HDMI2.1)

    /* HDMI2.1 special 4k 444 input timing for bridge application */
    {0, {560, 3840, 88, 176, 296}, {1, 180, 4320, 4320, 10, 98, 72}, 11880},	        //TX_TIMING_HDMI21_3840x4320P60_YUV444_8BIT_8G (HDMI21_TX_3840_4320P_444_60HZ_8G ) (HDMI2.1)
    {0, {560, 3840, 88, 176, 296}, {1, 360, 8640, 8640, 10, 278, 72}, 11880},	        //TX_TIMING_HDMI21_3840x8640P30_YUV444_8BIT_8G (HDMI21_TX_3840_8640P_444_30HZ_8G ) (HDMI2.1)
    {0, {208, 4192, 88,  40,   80}, {1, 180, 4320, 4320, 10, 98, 72}, 11880},	        //TX_TIMING_HDMI21_4192x4320P60_YUV444_8BIT_8G (HDMI21_TX_4192x4320P_444_60HZ_8G ) (HDMI2.1)


    /* HDMI2.1 4K2Kp47 444 */
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP47_YUV444_8BIT_10G (HDMI21_TX_3840_2160P_444_47HZ_10G) (HDMI2.1)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP47_YUV444_10BIT_10G (HDMI21_TX_3840_2160P_444_47HZ_10G) (HDMI2.1)

    /* HDMI2.1 4K2Kp48 444 */
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP48_YUV444_8BIT_10G (HDMI21_TX_3840_2160P_444_48HZ_10G) (HDMI2.1)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP48_YUV444_10BIT_10G (HDMI21_TX_3840_2160P_444_48HZ_10G) (HDMI2.1)

    /* HDMI2.1 4K2Kp50 444 */
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP50_YUV444_8BIT_10G (HDMI21_TX_3840_2160P_444_50HZ_10G) (HDMI2.1)
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP50_YUV444_10BIT_10G (HDMI21_TX_3840_2160P_444_50HZ_10G) (HDMI2.1)

    /* HDMI2.1 4K2Kp60 444 */
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_RGB_8BIT_10G (HDMI21_TX_3840_2160P_444_60HZ_10G) (HDMI2.1)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_RGB_10BIT_10G (HDMI21_TX_3840_2160P_444_60HZ_10G) (HDMI2.1)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G (HDMI21_TX_3840_2160P_444_60HZ_10G) (HDMI2.1)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_YUV444_10BIT_10G (HDMI21_TX_3840_2160P_444_60HZ_10G) (HDMI2.1)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_YUV444_12BIT_10G (HDMI21_TX_3840_2160P_444_60HZ_10G) (HDMI2.1)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},	        //TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_12G (HDMI21_TX_3840_2160P_444_60HZ_12G) (HDMI2.1)

    /* HDMI2.1 4K2Kp47 420 */
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_6G4L (HDMI21_TX_3840_2160P_420_47HZ_6G4L)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_6G4L (HDMI21_TX_3840_2160P_420_47HZ_6G4L)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_10G (HDMI21_TX_3840_2160P_420_47HZ_10G)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_10G (HDMI21_TX_3840_2160P_420_47HZ_10G)

    /* HDMI2.1 4K2Kp48 420 */
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_6G4L (HDMI21_TX_3840_2160P_420_48HZ_6G4L)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_6G4L (HDMI21_TX_3840_2160P_420_48HZ_6G4L)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_10G (HDMI21_TX_3840_2160P_420_48HZ_10G)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_10G (HDMI21_TX_3840_2160P_420_48HZ_10G)

    /* HDMI2.1 4K2Kp50 420 */
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_6G4L (HDMI21_TX_3840_2160P_420_50HZ_6G4L)
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_6G4L (HDMI21_TX_3840_2160P_420_50HZ_6G4L)
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_10G (HDMI21_TX_3840_2160P_420_50HZ_10G)
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_10G (HDMI21_TX_3840_2160P_420_50HZ_10G)

    /* HDMI2.1 4K2Kp60 420 */
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_6G4L (HDMI21_TX_3840_2160P_420_60HZ_6G4L)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_6G4L (HDMI21_TX_3840_2160P_420_60HZ_6G4L)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_10G (HDMI21_TX_3840_2160P_420_60HZ_10G)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G (HDMI21_TX_3840_2160P_420_60HZ_10G)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},              // TX_TIMING_HDMI21_4K2KP60_YUV420_12BIT_6G4L (HDMI21_TX_3840_2160P_420_60HZ_6G4L)

    /* HDMI2.1 2k1k@3G (3Lane) */
    {1, {160, 640, 96, 16, 48}, {1, 45, 480, 480, 2, 10, 33}, 251},                              // TX_TIMING_HDMI21_640x480P59_RGB_8BIT_3G (HDMI21_TX_640_480P_60HZ)
    {2, {138, 720, 64, 16, 60}, {1, 45, 480, 480, 6, 9, 30}, 270},                              // TX_TIMING_HDMI21_480P59_YUV444_8BIT_3G (HDMI21_TX_720_480P_60HZ)
    {4, {370, 1280, 40, 110, 220}, {1, 30, 720, 720, 5, 5, 20}, 742},                         //TX_TIMING_HDMI21_720P60_YUV444_8BIT_3G (HDMI21_TX_1280_720P_60HZ)
    {16, {280, 1920, 44, 88, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},                    //TX_TIMING_HDMI21_1080P60_YUV444_8BIT_3G (HDMI21_TX_1920_1080P_60HZ)

    /* HDMI2.1 2k1k@6G4L (4Lane) */
    {1, {160, 640, 96, 16, 48}, {1, 45, 480, 480, 2, 10, 33}, 251},                              // TX_TIMING_HDMI21_640x480P59_RGB_8BIT_6G4L (HDMI21_TX_640_480P_60HZ)
    {2, {138, 720, 64, 16, 60}, {1, 45, 480, 480, 6, 9, 30}, 270},                              // TX_TIMING_HDMI21_480P59_YUV444_8BIT_6G4L (HDMI21_TX_720_480P_60HZ)
    {4, {370, 1280, 40, 110, 220}, {1, 30, 720, 720, 5, 5, 20}, 742},                         //TX_TIMING_HDMI21_720P60_YUV444_8BIT_6G4L (HDMI21_TX_1280_720P_60HZ)
    {16, {280, 1920, 44, 88, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},                    //TX_TIMING_HDMI21_1080P60_YUV444_8BIT_6G4L (HDMI21_TX_1920_1080P_60HZ)
    {16, {280, 1920, 44, 88, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},                    //TX_TIMING_HDMI21_1080P60_RGB_8BIT_6G4L (HDMI21_TX_1920_1080P_60HZ)

    /* *********** HDMI20 ******************** */
    /* HDMI2.0 640x480p59 RGB */
    {1, {160, 640, 96, 16, 48}, {1, 45, 480, 480, 2, 10, 33}, 251},	              //TX_TIMING_HDMI20_640X480P59_RGB_8BIT (HDMI_TX_640_480P_60HZ) (HDMI2.0)

    /* HDMI2.0 720x480p59 YUV */
    {2, {140, 720, 64, 16, 60}, {1, 45, 480, 480, 6, 9, 30}, 270},                  //TX_TIMING_HDMI20_720X480P59_YUV444_8BIT (HDMI21_TX_720_480P_60HZ)

    /* HDMI2.0 720x576p50 YUV */
    {17, {144, 720, 64, 12, 68}, {1, 49, 576, 576, 5, 5, 39}, 270},	                //TX_TIMING_HDMI20_720X576P50_YUV444_8BIT (HDMI_TX_720_576P_50HZ)

    /* HDMI2.0 720p50 444 */
    {19, {700, 1280, 40, 440, 220}, {1, 30, 720, 720, 5, 5, 20}, 742},	        //TX_TIMING_HDMI20_720P50_YUV444_8BIT (HDMI_TX_1280_720P_50HZ)

    /* HDMI2.0 720p60 444 */
    {4, {370, 1280, 40, 110, 220}, {1, 30, 720, 720, 5, 5, 20}, 742},		    // TX_TIMING_HDMI20_720P60_YUV444_8BIT (HDMI_TX_1280_720P_60HZ)

    /* HDMI2.0 1080p24 444 */
    {32, {830, 1920, 44, 638, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 742},	// TX_TIMING_HDMI20_1080P24_YUV444_8BIT (HDMI_TX_1920_1080P_24HZ)

    /* HDMI2.0 1080p60/50/48/47 444 */
    {111, {830, 1920, 44, 638, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},             //TX_TIMING_HDMI20_1080P47_YUV444_8BIT (HDMI_TX_1920_1080P_48HZ) (HDMI2.0)
    {111, {830, 1920, 44, 638, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},             //TX_TIMING_HDMI20_1080P48_YUV444_8BIT (HDMI_TX_1920_1080P_48HZ) (HDMI2.0)
    {31, {720, 1920, 44, 528, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},             //TX_TIMING_HDMI20_1080P50_YUV444_8BIT (HDMI_TX_1920_1080P_50HZ) (HDMI2.0)
    {16, {280, 1920, 44, 88, 148}, {1, 45, 1080, 1080, 5, 4, 36}, 1485},             //TX_TIMING_HDMI20_1080P60_YUV444_8BIT (HDMI_TX_1920_1080P_60HZ) (HDMI2.0)

    /* HDMI2.0 4k2kp30 444 */
    {95, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 2970},	        // TX_TIMING_HDMI20_4K2KP30_YUV444_8BIT (HDMI_TX_3840_2160P_444_30HZ)

    /* HDMI2.0 4k2kp60 420 */
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI20_4K2KP60_YUV420_8BIT (HDMI_TX_3840_2160P_420_60HZ)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI20_4K2KP60_YUV420_10BIT (HDMI_TX_3840_2160P_420_60HZ)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI20_4K2KP60_YUV420_12BIT (HDMI_TX_3840_2160P_420_60HZ)

    /* HDMI2.0 4k2kp60/50/48/47 444 */
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},		 //TX_TIMING_HDMI20_4K2KP47_YUV444_8BIT (HDMI_TX_3840_2160P_444_48HZ) (HDMI2.0)
    {114, {1660, 3840, 88, 1276, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},		 //TX_TIMING_HDMI20_4K2KP48_YUV444_8BIT (HDMI_TX_3840_2160P_444_48HZ) (HDMI2.0)
    {96, {1440, 3840, 88, 1056, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},		 //TX_TIMING_HDMI20_4K2KP50_YUV444_8BIT (HDMI_TX_3840_2160P_444_50HZ) (HDMI2.0)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},		 //TX_TIMING_HDMI20_4K2KP60_YUV444_8BIT (HDMI_TX_3840_2160P_444_60HZ) (HDMI2.0)

    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},		 //TX_TIMING_HDMI20_4K2KP60_RGB_8BIT (HDMI_TX_3840_2160P_444_60HZ) (HDMI2.0)


    /* HDMI20 STB6500 timing */
    // STB6500 4k2kp60/1080p60/720p60/4k2kp50/1080p50/720p50/720x576p/4k2kp30
    {17, {144, 720, 32, 108, 4}, {1, 48, 576, 576, 6, 24, 18}, 270},	          //TX_TIMING_HDMI20_STB6500_720X576P50_YUV444_8BIT (HDMI_TX_STB6500_720_576P_50HZ)
    {19, {700, 1280, 20, 525, 155}, {1, 30, 720, 720, 6, 15, 9}, 742},	          //TX_TIMING_HDMI20_STB6500_720P50_YUV444_8BIT (HDMI_TX_STB6500_1280_720P_50HZ)
    {4, {370, 1280, 20, 278, 72}, {1, 30, 720, 720, 6, 15, 9}, 742},		    // TX_TIMING_HDMI20_STB6500_720P60_YUV444_8BIT (HDMI_TX_STB6500_1280_720P_60HZ)
    {31, {720, 1920, 44, 360, 316}, {1, 44, 1080, 1080, 6, 21, 17}, 1485},      //TX_TIMING_HDMI20_STB6500_1080P50_YUV444_8BIT (HDMI_TX_STB6500_1920_1080P_50HZ) (HDMI2.0)
    {16, {280, 1920, 44, 140, 96}, {1, 44, 1080, 1080, 6, 22, 16}, 1485},        //TX_TIMING_HDMI20_STB6500_1080P60_YUV444_8BIT (HDMI_TX_STB6500_1920_1080P_60HZ) (HDMI2.0)
    {95, {560, 3840, 88, 280, 192}, {1, 90, 2160, 2160, 10, 45, 35}, 2970},	    // TX_TIMING_HDMI20_STB6500_4K2KP30_YUV444_8BIT (HDMI_TX_STB6500_3840_2160P_444_30HZ)
    {96, {1440, 3840, 88, 720, 632}, {1, 90, 2160, 2160, 10, 45, 35}, 5940},   //TX_TIMING_HDMI20_STB6500_4K2KP50_YUV444_8BIT (HDMI_TX_STB6500_3840_2160P_444_50HZ) (HDMI2.0)
    {97, {560, 3840, 88, 280, 192}, {1, 90, 2160, 2160, 10, 45, 35}, 5940},      //TX_TIMING_HDMI20_STB6500_4K2KP60_YUV444_8BIT (HDMI_TX_STB6500_3840_2160P_444_60HZ) (HDMI2.0)


    /* *********** HDMI2.1 DSC timing ******************** */
    /* HDMI2.1 DSC 4k2kp60 444 FRL=6G4L */
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_8BIT_6G4L (HDMI21_TX_DSCE_3840_2160P_444_60Hz_6G4L)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_10BIT_6G4L (HDMI21_TX_DSCE_3840_2160P_444_60Hz_6G4L)
    {97, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 5940},          // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_12BIT_6G4L (HDMI21_TX_DSCE_3840_2160P_444_60Hz_6G4L)

    /* HDMI2.1 DSC 4k2kp120 444 FRL=6G4L */
    {118, {560, 3840, 88, 176, 296}, {1, 90, 2160, 2160, 10, 8, 72}, 11880},       // TX_TIMING_HDMI21_DSCE_3840_2160P120Hz_YUV444_8BIT_6G4L (HDMI21_TX_DSCE_3840_2160P_444_120Hz_6G4L)

    /* HDMI2.1 DSC 8k4kp30 444 FRL=6G4L */
    {196, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 11880},   // TX_TIMING_HDMI21_DSCE_7680_4320P30Hz_YUV444_8BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_30Hz_6G4L)

    /* HDMI2.1 DSC 8k4kp60 444 FRL=6G4L */
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)

    /* HDMI2.1 DSC 8k4kp60 444 FRL=12G4L */
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_12G (HDMI21_TX_DSCE_7680_4320P_444_60Hz_12G)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_12G (HDMI21_TX_DSCE_7680_4320P_444_60Hz_12G)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_12G (HDMI21_TX_DSCE_7680_4320P_444_60Hz_12G)

    /* HDMI2.1 DSC 8k4kp59 444 FRL=6G4L */
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_8BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_10BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)
    {199, {1320, 7680, 176, 552, 592}, {1, 80, 4320, 4320, 20, 16, 44}, 23760},   // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_12BIT_6G4L (HDMI21_TX_DSCE_7680_4320P_444_60Hz_6G4L)

};


/*
 * Description: HDMITX output timing info
 * Tab le format: {set1}
 * {set1} : {width, height, frame_rate, color_type, color_depth, frl_type}
 * Table index  :
 *      HDMI_TX_VIDEO_TIMING_NUM : see hdmi_tx_video_format_index
 */
hdmi_tx_output_timing_type tx_output_timing_table[TX_TIMING_NUM] = {
    /* *********** HDMI21 ******************** */
    /* HDMI2.1 4K2Kp60 444 */
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV444 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L

    /* HDMI2.1 8K4Kp60 420 */
    {7680,  4320,   600,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 8K4K 60p YUV420 8bit@8G",     TRUE},  // TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G
    {7680,  4320,   600,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 60p YUV420 8bit@10G",    TRUE},  // TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G
    {7680,  4320,   600,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 60p YUV420 10bit@10G",    TRUE},  // TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G
    {7680,  4320,   600,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_12G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 60p YUV420 12bit@12G",    TRUE},  // TX_TIMING_HDMI21_8K4KP60_YUV420_12BIT_12G

    /* HDMI2.1 8K4Kp59 420 */
    {7680,  4320,   599,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 8K4K 59p YUV420 8bit@8G",     TRUE},  // TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_8G
    {7680,  4320,   599,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 8K4K 59p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_10G
    {7680,  4320,   599,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 59p YUV420 10bit@10G",    TRUE},  // TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G
    {7680,  4320,   599,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_12G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 59p YUV420 12bit@12G",    TRUE},  // TX_TIMING_HDMI21_8K4KP59_YUV420_12BIT_12G

    /* HDMI2.1 8K4Kp23 420 */
    {7680,  4320,   239,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 23p YUV420 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_6G4L
    {7680,  4320,   239,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 23p YUV420 10bit@6G4L",    TRUE },  // TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_6G4L
    {7680,  4320,   239,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 23p YUV420 12bit@6G4L",    TRUE},  // TX_TIMING_HDMI21_8K4KP23_YUV420_12BIT_6G4L
    {7680,  4320,   239,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 23p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_10G
    {7680,  4320,   239,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 23p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_10G

    /* HDMI2.1 8K4Kp24 420 */
    {7680,  4320,   240,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 24p YUV420 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_6G4L
    {7680,  4320,   240,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 24p YUV420 10bit@6G4L",    TRUE },  // TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_6G4L
    {7680,  4320,   240,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 24p YUV420 12bit@6G4L",    TRUE},  // TX_TIMING_HDMI21_8K4KP24_YUV420_12BIT_6G4L
    {7680,  4320,   240,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 24p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_10G
    {7680,  4320,   240,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 24p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_10G

    /* HDMI2.1 8K4Kp25 420 */
    {7680,  4320,   250,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 25p YUV420 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_6G4L
    {7680,  4320,   250,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 25p YUV420 10bit@6G4L",    TRUE },  // TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_6G4L
    {7680,  4320,   250,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 25p YUV420 12bit@6G4L",    TRUE},  // TX_TIMING_HDMI21_8K4KP25_YUV420_12BIT_6G4L
    {7680,  4320,   250,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 25p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_10G
    {7680,  4320,   250,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 25p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_10G

    /* HDMI2.1 8K4Kp30 420 */
    {7680,  4320,   300,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 30p YUV420 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_6G4L
    {7680,  4320,   300,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 30p YUV420 10bit@6G4L",    TRUE },  // TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_6G4L
    {7680,  4320,   300,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 30p YUV420 12bit@6G4L",    TRUE},  // TX_TIMING_HDMI21_8K4KP30_YUV420_12BIT_6G4L
    {7680,  4320,   300,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 30p YUV420 8bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_10G
    {7680,  4320,   300,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 30p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_10G

    /* HDMI2.1 8K4Kp29 420 */
    {7680,  4320,   299,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 29p YUV420 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_6G4L
    {7680,  4320,   299,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 29p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_10G
    {7680,  4320,   299,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 29p YUV420 10bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_6G4L
    {7680,  4320,   299,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 29p YUV420 10bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_10G
    {7680,  4320,   299,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 29p YUV420 12bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_8K4KP29_YUV420_12BIT_6G4L

    /* HDMI2.1 8K4Kp30 444 */
    {7680,  4320,   300,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 30p YUV444 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_10G
    {7680,  4320,   300,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_12G, _HDMITX_STD_HDMI,        "HDMI2.1 8K4K 30p YUV444 8bit@12G",     TRUE},  // TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_12G

    /* HDMI2.1 8K4Kp47 420 */
    {7680,  4320,   479,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 47p YUV420 8bit@8G",     TRUE},  // TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_8G
    {7680,  4320,   479,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 47p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_10G
    {7680,  4320,   479,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 47p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP47_YUV420_10BIT_10G
    {7680,  4320,   479,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_12G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 47p YUV420 12bit@12G",    TRUE},  // TX_TIMING_HDMI21_8K4KP47_YUV420_12BIT_12G

    /* HDMI2.1 8K4Kp48 420 */
    {7680,  4320,   480,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 48p YUV420 8bit@8G",     TRUE},  // TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_8G
    {7680,  4320,   480,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 48p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_10G
    {7680,  4320,   480,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 48p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP48_YUV420_10BIT_10G
    {7680,  4320,   480,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_12G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 48p YUV420 12bit@12G",    TRUE},  // TX_TIMING_HDMI21_8K4KP48_YUV420_12BIT_12G

    /* HDMI2.1 8K4Kp50 420 */
    {7680,  4320,   500,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 50p YUV420 8bit@8G",     TRUE},  // TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_8G
    {7680,  4320,   500,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 50p YUV420 8bit@10G",     TRUE},  // TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_10G
    {7680,  4320,   500,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_10G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 50p YUV420 10bit@10G",    TRUE },  // TX_TIMING_HDMI21_8K4KP50_YUV420_10BIT_10G
    {7680,  4320,   500,    I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_12G, _HDMITX_STD_HDMI,  "HDMI2.1 8K4K 50p YUV420 12bit@12G",    TRUE},  // TX_TIMING_HDMI21_8K4KP50_YUV420_12BIT_12G

    /* HDMI2.1 4K2Kp120 444 */
    {3840,  2160,   1200,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 120p YUV444 8bit@8G",    TRUE}, // TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G

    /* HDMI2.1 special 4k 444 input timing for bridge application */
    {3840,  4320,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 3840x4320 60p YUV444 8bit@8G",    TRUE}, // TX_TIMING_HDMI21_3840x4320P60_YUV444_8BIT_8G
    {3840,  8640,   300,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 3840x8640 30p YUV444 8bit@8G",    TRUE}, // TX_TIMING_HDMI21_3840x8640P30_YUV444_8BIT_8G
    {4192,  4320,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_8G, _HDMITX_STD_HDMI,         "HDMI2.1 4192x4320 60p YUV444 8bit@8G",    TRUE}, // TX_TIMING_HDMI21_4192x4320P60_YUV444_8BIT_8G

    /* HDMI2.1 4K2Kp47 444 */
    {3840,  2160,   479,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 47p YUV444 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV444_8BIT_10G
    {3840,  2160,   479,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 47p YUV444 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV444_10BIT_10G

    /* HDMI2.1 4K2Kp48 444 */
    {3840,  2160,   480,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 48p YUV444 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV444_8BIT_10G
    {3840,  2160,   480,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 48p YUV444 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV444_10BIT_10G

    /* HDMI2.1 4K2Kp50 444 */
    {3840,  2160,   500,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 50p YUV444 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV444_8BIT_10G
    {3840,  2160,   500,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 50p YUV444 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV444_10BIT_10G

    /* HDMI2.1 4K2Kp60 444 */
    {3840,  2160,   600,   I3DDMA_COLOR_RGB,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p RGB 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_RGB_8BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_RGB,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p RGB 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_RGB_10BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p YUV444 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p YUV444 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV444_10BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_12B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p YUV444 12bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV444_12BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_12G, _HDMITX_STD_HDMI,         "HDMI2.1 4K2K 60p YUV444 8bit@12G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_12G

    /* HDMI2.1 4K2Kp47 420 */
    {3840,  2160,   479,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 47p YUV420 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_6G4L
    {3840,  2160,   479,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 47p YUV420 10bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_6G4L
    {3840,  2160,   479,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 47p YUV420 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_10G
    {3840,  2160,   479,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 47p YUV420 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_10G

    /* HDMI2.1 4K2Kp48 420 */
    {3840,  2160,   480,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 48p YUV420 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_6G4L
    {3840,  2160,   480,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 48p YUV420 10bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_6G4L
    {3840,  2160,   480,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 48p YUV420 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_10G
    {3840,  2160,   480,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 48p YUV420 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_10G

    /* HDMI2.1 4K2Kp50 420 */
    {3840,  2160,   500,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 50p YUV420 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_6G4L
    {3840,  2160,   500,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 50p YUV420 10bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_6G4L
    {3840,  2160,   500,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 50p YUV420 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_10G
    {3840,  2160,   500,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 50p YUV420 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_10G

    /* HDMI2.1 4K2Kp60 420 */
    {3840,  2160,   600,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV420 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_6G4L
    {3840,  2160,   600,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV420 10bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_6G4L
    {3840,  2160,   600,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV420 8bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_10G, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV420 10bit@10G",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G
    {3840,  2160,   600,   I3DDMA_COLOR_YUV411,    I3DDMA_COLOR_DEPTH_12B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI, "HDMI2.1 4K2K 60p YUV420 12bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_4K2KP60_YUV420_12BIT_6G4L

    /* HDMI2.1 2k1k@3G (3Lane) */
    {640,   480,    599,    I3DDMA_COLOR_RGB,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_3G, _HDMITX_STD_HDMI,       "HDMI2.1 640x480 59p RGB 8bit@3G",     TRUE}, // TX_TIMING_HDMI21_640x480P59_RGB_8BIT_3G
    {720,   480,    599,    I3DDMA_COLOR_YUV444,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_3G, _HDMITX_STD_HDMI,       "HDMI2.1 480 59p YUV444 8bit@3G",     TRUE}, // TX_TIMING_HDMI21_480P59_YUV444_8BIT_3G
    {1280,  720,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_3G, _HDMITX_STD_HDMI,       "HDMI2.1 720P 60p YUV444 8bit@3G",    TRUE}, // TX_TIMING_HDMI21_720P60_YUV444_8BIT_3G
    {1920,  1080,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_3G, _HDMITX_STD_HDMI,       "HDMI2.1 1080P 60p YUV444 8bit@3G",    TRUE}, // TX_TIMING_HDMI21_1080P60_YUV444_8BIT_3G

    /* HDMI2.1 2k1k@6G4L */
    {640,   480,    599,    I3DDMA_COLOR_RGB,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,       "HDMI2.1 640x480 59p RGB 8bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_640x480P59_RGB_8BIT_6G4L
    {720,   480,    599,    I3DDMA_COLOR_YUV444,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,       "HDMI2.1 480 59p YUV444 8bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_480P59_YUV444_8BIT_6G4L
    {1280,  720,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,       "HDMI2.1 720P 60p YUV444 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_720P60_YUV444_8BIT_6G4L
    {1920,  1080,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,       "HDMI2.1 1080P 60p YUV444 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_1080P60_YUV444_8BIT_6G4L
    {1920,  1080,   600,    I3DDMA_COLOR_RGB   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,       "HDMI2.1 1080P 60p RGB 8bit@6G4L",    TRUE}, // TX_TIMING_HDMI21_1080P60_RGB_8BIT_6G4L

    /* *********** HDMI20 ******************** */
    /* HDMI2.0 640x480p60/59 RGB */
    {640,   480,    599,    I3DDMA_COLOR_RGB,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 640X480 59p RGB 8bit",     TRUE}, // TX_TIMING_HDMI20_640X480P59_RGB_8BIT

    /* HDMI2.0 720x480p60/59 YUV444 */
    {720,   480,    599,    I3DDMA_COLOR_YUV444,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 720X480 59p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_720X480P59_YUV444_8BIT

    /* HDMI2.0 720x576p50 YUV444 */
    {720,   576,    500,    I3DDMA_COLOR_YUV444,       I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 720X576 50p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_720X576P50_YUV444_8BIT

    /* HDMI2.0 720p50 444 */
    {1280,  720,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 720P 50p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_720P50_YUV444_8BIT

    /* HDMI2.0 720p60 444 */
    {1280,  720,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 720P 60p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_720P60_YUV444_8BIT

    /* HDMI2.0 1080p24 444 */
    {1920,  1080,   240,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 1080P 24p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_1080P24_YUV444_8BIT

    /* HDMI2.0 1080p60/50/48/47 444 */
    {1920,  1080,   479,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 1080P 47p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_1080P47_YUV444_8BIT
    {1920,  1080,   480,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 1080P 48p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_1080P48_YUV444_8BIT
    {1920,  1080,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 1080P 50p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_1080P50_YUV444_8BIT
    {1920,  1080,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,       "HDMI2.0 1080P 60p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_1080P60_YUV444_8BIT

    /* HDMI2.0 4k2kp30 444 */
    {3840,  2160,   300,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 30p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP30_YUV444_8BIT

    /* HDMI2.0 4k2kp60 420 */
    {3840,  2160,   600,    I3DDMA_COLOR_YUV411   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 60p YUV420 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP60_YUV420_8BIT
    {3840,  2160,   600,    I3DDMA_COLOR_YUV411   , I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 60p YUV420 10bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP60_YUV420_10BIT
    {3840,  2160,   600,    I3DDMA_COLOR_YUV411   , I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 60p YUV420 12bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP60_YUV420_12BIT

    /* HDMI2.0 4k2kp60/50/48/47 444 */
    {3840,  2160,   479,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 47p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP47_YUV444_8BIT
    {3840,  2160,   480,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 48p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP48_YUV444_8BIT
    {3840,  2160,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 50p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP50_YUV444_8BIT
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,        "HDMI2.0 4K2K 60p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP60_YUV444_8BIT

    {3840,  2160,   600,    I3DDMA_COLOR_RGB   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_HDMI,               "HDMI2.0 4K2K 60p RGB 8bit",     TRUE}, // TX_TIMING_HDMI20_4K2KP60_RGB_8BIT

    /* HDMI20 STB6500 timing */
    {720,   576,    500,    I3DDMA_COLOR_YUV444,  I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,         "HDMI2.0 STB6500 720X576 50p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_STB6500_720X576P50_YUV444_8BIT
    {1280,  720,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,       "HDMI2.0 STB6500 720P 50p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_STB6500_720P50_YUV444_8BIT
    {1280,  720,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,       "HDMI2.0 STB6500 720P 60p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_STB6500_720P60_YUV444_8BIT
    {1920,  1080,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,       "HDMI2.0 STB6500 1080P 50p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_STB6500_1080P50_YUV444_8BIT
    {1920,  1080,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,       "HDMI2.0 STB6500 1080P 60p YUV444 8bit",    TRUE}, // TX_TIMING_HDMI20_STB6500_1080P60_YUV444_8BIT
    {3840,  2160,   300,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,        "HDMI2.0 STB6500 4K2K 30p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_STB6500_4K2KP30_YUV444_8BIT
    {3840,  2160,   500,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,        "HDMI2.0 STB6500 4K2K 50p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_STB6500_4K2KP50_YUV444_8BIT
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_NONE, _HDMITX_STD_STB6500,        "HDMI2.0 STB6500 4K2K 60p YUV444 8bit",     TRUE}, // TX_TIMING_HDMI20_STB6500_4K2KP60_YUV444_8BIT

    /* *********** HDMI2.1 DSC timing ******************** */
    /* HDMI2.1 DSC 4k2kp60 444 FRL=6G4L */
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,        "HDMI2.1 DSC 4K2K 60p YUV444 8bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_8BIT_6G4L
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_10B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,        "HDMI2.1 DSC 4K2K 60p YUV444 10bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_10BIT_6G4L
    {3840,  2160,   600,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_12B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,        "HDMI2.1 DSC 4K2K 60p YUV444 12bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_12BIT_6G4L

    /* HDMI2.1 DSC 4k2kp120 444 FRL=6G4L */
    {3840,  2160,   1200,    I3DDMA_COLOR_YUV444   , I3DDMA_COLOR_DEPTH_8B, _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,        "HDMI2.1 DSC 4K2K 120p YUV444 8bit@6G4L",     TRUE}, // TX_TIMING_HDMI21_DSCE_3840_2160P120Hz_YUV444_8BIT_6G4L

    /* HDMI2.1 DSC 8k4kp30 444 FRL=6G4L */
    {7680,  4320,   300,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,        "HDMI2.1 DSC 8K4K 30p YUV444 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P30Hz_YUV444_8BIT_6G4L

    /* HDMI2.1 DSC 8k4kp60 444 FRL=6G4L */
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 60p YUV444 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_6G4L
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 60p YUV444 10bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_6G4L
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_12B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 60p YUV444 12bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_6G4L

    /* HDMI2.1 DSC 8k4kp60 444 FRL=12G4L */
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_12G, _HDMITX_STD_HDMI,                   "HDMI2.1 DSC 8K4K 60p YUV444 8bit@12G",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_12G
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_12G, _HDMITX_STD_HDMI,                   "HDMI2.1 DSC 8K4K 60p YUV444 10bit@12G",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_12G
    {7680,  4320,   600,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_12B,  _HDMI21_FRL_12G, _HDMITX_STD_HDMI,                   "HDMI2.1 DSC 8K4K 60p YUV444 12bit@12G",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_12G

    /* HDMI2.1 DSC 8k4kp59 444 FRL=6G4L */
    {7680,  4320,   599,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_8B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 59p YUV444 8bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_8BIT_6G4L
    {7680,  4320,   599,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_10B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 59p YUV444 10bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_10BIT_6G4L
    {7680,  4320,   599,    I3DDMA_COLOR_YUV444,    I3DDMA_COLOR_DEPTH_12B,  _HDMI21_FRL_6G_4LANES, _HDMITX_STD_HDMI,         "HDMI2.1 DSC 8K4K 59p YUV444 12bit@6G4L",     TRUE},  // TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_12BIT_6G4L

};

extern void ScalerHdmiMacTx0PowerProc(unsigned char enumPowerAction);
extern void Scaler_TestCase_config(unsigned int width, unsigned int height, unsigned int frame_rate, I3DDMA_COLOR_DEPTH_T colorDepth, I3DDMA_COLOR_SPACE_T colorFmt, unsigned char timeIdx);
extern void ScalerHdmiMacTx0EdidSetFeature(EnumEdidFeatureType enumEdidFeature, BYTE ucValue);
extern void Scaler_TxStateHandler(void);
extern void ScalerHdmiTxSetTargetFrlRate(EnumHdmi21FrlType frlType);
extern void ScalerHdmiTxSetDSCEn(unsigned char enable);
extern void ScalerHdmiTxSetDscMode(unsigned char dscMode);
extern unsigned char ScalerHdmiTxGetDscMode(void);

extern void drvif_color_hdmitx_rgb2yuv_en(unsigned char enable);
extern void Scaler_color_set_HDMITX_RGB2YUV(void);

extern void ScalerHdmiTxSetHdmiRxPortLimit(unsigned char portNum);
extern void ScalerHdmiTxCheckEdidForTxOut(unsigned char enable);
extern unsigned short Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_INFO infoList);
extern void Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_INFO infoList, unsigned short value);
extern unsigned short Scaler_HDMITX_DispGetStatus(SCALER_DISP_CHANNEL channel, SLR_DISP_STATUS index);
extern void Scaler_HDMITX_DispSetStatus(SCALER_DISP_CHANNEL channel, SLR_DISP_STATUS index, unsigned short status);
extern unsigned char fwif_scaler_wait_for_event_done(unsigned int addr, unsigned int event);

void ScalerHdmiTxSetBypassLinkTrainingEn(unsigned char enable);
extern void ScalerHdmiTxSetHDCP22Enable(unsigned char enable);

void dvScript_txsoc_config(unsigned int width, unsigned height, unsigned short frame_rate)
{
#if 0//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    tx_soc_vgip_tx_soc_vgip_ctrl_RBUS tx_soc_vgip_tx_soc_vgip_ctrl_reg;
    tx_soc_vgip_tx_soc_vgip_act_hsta_width_RBUS tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg;
    tx_soc_vgip_tx_soc_vgip_act_vsta_length_RBUS tx_soc_vgip_tx_soc_vgip_act_vsta_length_reg;
    scaledown_txsoc_ich4_uzd_channel_swap_RBUS scaledown_txsoc_ich4_uzd_channel_swap_reg;
    scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_RBUS scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_reg;
    scaledown_txsoc_ich4_uzd_ctrl0_RBUS scaledown_txsoc_ich4_uzd_ctrl0_reg;
    scaledown_txsoc_ich4_uzd_initial_value_RBUS scaledown_txsoc_ich4_uzd_initial_value_reg;
    fsyncbuf_fsync_fifo_status_RBUS fsyncbuf_fsync_fifo_status_reg;
    fsyncbuf_pxl2bus_ctrl1_RBUS fsyncbuf_pxl2bus_ctrl1_reg;
    fsyncbuf_pxl2bus_ctrl2_RBUS fsyncbuf_pxl2bus_ctrl2_reg;
    txsoc_scaleup_txsoc_uzu_input_size_RBUS txsoc_scaleup_txsoc_uzu_input_size_reg;
    txsoc_scaleup_txsoc_uzu_ctrl_RBUS txsoc_scaleup_txsoc_uzu_ctrl_reg;
    txsoctg_tx_soc_tg_double_buffer_ctrl_RBUS txsoctg_tx_soc_tg_double_buffer_ctrl_reg;
    txsoctg_txsoc_dh_width_RBUS txsoctg_txsoc_dh_width_reg;
    txsoctg_txsoc_dh_total_last_line_length_RBUS txsoctg_txsoc_dh_total_last_line_length_reg;
    txsoctg_txosc_dv_length_RBUS txsoctg_txosc_dv_length_reg;
    txsoctg_txsoc_dv_total_RBUS txsoctg_txsoc_dv_total_reg;
    txsoctg_txsoc_dh_den_start_end_RBUS txsoctg_txsoc_dh_den_start_end_reg;
    txsoctg_txosc_dv_den_start_end_RBUS txsoctg_txosc_dv_den_start_end_reg;
    txsoctg_txsoc_active_h_start_end_RBUS txsoctg_txsoc_active_h_start_end_reg;
    txsoctg_txsoc_active_v_start_end_RBUS txsoctg_txsoc_active_v_start_end_reg;
    txsoctg_txsoc_fs_iv_dv_fine_tuning5_RBUS txsoctg_txsoc_fs_iv_dv_fine_tuning5_reg;
    txsoctg_txsoc_display_timing_ctrl1_RBUS txsoctg_txsoc_display_timing_ctrl1_reg;
    txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_RBUS txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_reg;
    txsoctg_txsoc_pcr_cnt_RBUS txsoctg_txsoc_pcr_cnt_reg;
    txsoc_ds444_txsoc_ds444_ctrl0_RBUS txsoc_ds444_txsoc_ds444_ctrl0_reg;
    unsigned char ucColorSpace;

    DebugMessageHDMITx("[HDMI_TX] %s...\n", __FUNCTION__);

    //rtd_outl(0xb8021510,0x40000001); // TXSOC in select VO1
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.regValue = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_ivrun = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_in_sel = 4; // 4: VODMA1
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_random_en = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_vact_end_ie = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_vact_start_ie = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_field_inv = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_field_sync_edge = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_force_tog = 0;
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_vs_inv = 0;
    rtd_outl(TX_SOC_VGIP_TX_SOC_VGIP_CTRL_reg, tx_soc_vgip_tx_soc_vgip_ctrl_reg.regValue);

    //rtd_outl(0xb8021518,0x00000258);
    tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg.regValue = 0;
    tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg.tx_soc_ih_act_sta = 0;
    tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg.tx_soc_bypass_den = 0;
    tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg.tx_soc_ih_act_wid = width;
    rtd_outl(TX_SOC_VGIP_TX_SOC_VGIP_ACT_HSTA_Width_reg, tx_soc_vgip_tx_soc_vgip_act_hsta_width_reg.regValue);

    //rtd_outl(0xb802151c,0x000001e0);
    tx_soc_vgip_tx_soc_vgip_act_vsta_length_reg.regValue = 0;
    tx_soc_vgip_tx_soc_vgip_act_vsta_length_reg.tx_soc_iv_act_sta = 0;
    tx_soc_vgip_tx_soc_vgip_act_vsta_length_reg.tx_soc_iv_act_len = height;
    rtd_outl(TX_SOC_VGIP_TX_SOC_VGIP_ACT_VSTA_Length_reg, tx_soc_vgip_tx_soc_vgip_act_vsta_length_reg.regValue);

    //rtd_outl(0xb8021510,0xc0000001); // TXSOC in select VO1
    tx_soc_vgip_tx_soc_vgip_ctrl_reg.tx_soc_ivrun = 1;
    rtd_outl(TX_SOC_VGIP_TX_SOC_VGIP_CTRL_reg, tx_soc_vgip_tx_soc_vgip_ctrl_reg.regValue);

    //rtd_outl(0xb8021944,0x02580000);
    scaledown_txsoc_ich4_uzd_channel_swap_reg.regValue = 0;
    scaledown_txsoc_ich4_uzd_channel_swap_reg.hsd_input_size = width;
    scaledown_txsoc_ich4_uzd_channel_swap_reg.rgb_ch_swap = 0;
    rtd_outl(SCALEDOWN_TXSOC_ICH4_UZD_channel_swap_reg, scaledown_txsoc_ich4_uzd_channel_swap_reg.regValue);

    //rtd_outl(0xb8021940,0x01e00258);
    scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_reg.regValue = 0;
    scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_reg.ibuf_v_size = height;
    scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_reg.ibuf_h_size = width;
    rtd_outl(SCALEDOWN_TXSOC_ICH4_IBUFF_CTRLI_BUFF_CTRL_reg, scaledown_txsoc_ich4_ibuff_ctrli_buff_ctrl_reg.regValue);

    //rtd_outl(0xb8021904,0x00000300); // UZD output 10bit 444
    scaledown_txsoc_ich4_uzd_ctrl0_reg.regValue = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.en_gate = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.cutout_en = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.sel_3d_en = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.truncationctrl = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.dummy18021904_23_12 = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.video_comp_en = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.odd_inv = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.out_bit = 1; // output 10bit
    scaledown_txsoc_ich4_uzd_ctrl0_reg.sort_fmt = 1; // output 444
    scaledown_txsoc_ich4_uzd_ctrl0_reg.v_y_table_sel =0 ;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.v_c_table_sel = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.h_y_table_sel = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.h_c_table_sel = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.v_zoom_en = 0;
    scaledown_txsoc_ich4_uzd_ctrl0_reg.h_zoom_en = 0;
    rtd_outl(SCALEDOWN_TXSOC_ICH4_UZD_Ctrl0_reg, scaledown_txsoc_ich4_uzd_ctrl0_reg.regValue);

    //rtd_outl(0xb8021914,0x00000000); // H/V init=0
    scaledown_txsoc_ich4_uzd_initial_value_reg.regValue = 0;
    rtd_outl(SCALEDOWN_TXSOC_ICH4_UZD_Initial_Value_reg, scaledown_txsoc_ich4_uzd_initial_value_reg.regValue);

    //rtd_outl(0xb8021b00,0x003f2013); // fifoof_th = 0x3f2;
    fsyncbuf_fsync_fifo_status_reg.regValue = 0;
    fsyncbuf_fsync_fifo_status_reg.fsbuf_fifoerr_wde = 0;
    fsyncbuf_fsync_fifo_status_reg.fsbuf_wd_debug_mode = 0;
    fsyncbuf_fsync_fifo_status_reg.fsbuf_wd_debug_target = 0;
    fsyncbuf_fsync_fifo_status_reg.fifoof_th = 0x3f2;
    fsyncbuf_fsync_fifo_status_reg.wclk_gated = 0;
    fsyncbuf_fsync_fifo_status_reg.fsyncbuf_fifoof = 0;
    fsyncbuf_fsync_fifo_status_reg.fsyncbuf_fifouf = 0;
    fsyncbuf_fsync_fifo_status_reg.fsyncbuf_fifoempty = 1;
    fsyncbuf_fsync_fifo_status_reg.wclk_gated_en = 1;
    fsyncbuf_fsync_fifo_status_reg.fsyncbuf_en = 1;
    rtd_outl(FSYNCBUF_fsync_fifo_status_reg, fsyncbuf_fsync_fifo_status_reg.regValue);

    // rtd_outl(0xb8021b04,0x00002573); // den_width=600, source mode=444 10bit
    fsyncbuf_pxl2bus_ctrl1_reg.regValue = 0;
    fsyncbuf_pxl2bus_ctrl1_reg.den_width = width - 1;
    fsyncbuf_pxl2bus_ctrl1_reg.source_mode_sel = 3; // 3:30bit (444 10bit mode)  &  4pixel_mode:120
    rtd_outl(FSYNCBUF_pxl2bus_ctrl1_reg, fsyncbuf_pxl2bus_ctrl1_reg.regValue);

    //rtd_outl(0xb8021b08,0x000001e0); // den_height=480
    fsyncbuf_pxl2bus_ctrl2_reg.regValue = 0;
    fsyncbuf_pxl2bus_ctrl2_reg.den_height = height;
    rtd_outl(FSYNCBUF_pxl2bus_ctrl2_reg, fsyncbuf_pxl2bus_ctrl2_reg.regValue);

    //rtd_outl(0xb8021c28,0x025801e0); // uzu input = 600x480
    txsoc_scaleup_txsoc_uzu_input_size_reg.regValue = 0;
    txsoc_scaleup_txsoc_uzu_input_size_reg.hor_input_size = width;
    txsoc_scaleup_txsoc_uzu_input_size_reg.ver_input_size = height;
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_Input_Size_reg, txsoc_scaleup_txsoc_uzu_input_size_reg.regValue);

    //rtd_outl(0xb8021c04,0x002a8000); // UZU disable
    txsoc_scaleup_txsoc_uzu_ctrl_reg.regValue = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.hor_mode_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.wd_main_buf_udf_en = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.irq_main_buf_udf_en = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.v_y_consttab_sel = 1;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.v_c_consttab_sel = 1;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.h_y_consttab_sel = 1;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.h_c_consttab_sel = 1;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.hor12_table_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.video_comp_en = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.odd_inv = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.in_fmt_conv = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.conv_type = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.v_y_table_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.v_c_table_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.h_y_table_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.h_c_table_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.in_fmt_dup = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.uzu_uzd_linebuf_sel = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.v_zoom_en = 0;
    txsoc_scaleup_txsoc_uzu_ctrl_reg.h_zoom_en = 0;
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_Ctrl_reg, txsoc_scaleup_txsoc_uzu_ctrl_reg.regValue);

    //rtd_outl(0xb8021804,0x0000001f); disp hsync width = 0x1f
    txsoctg_txsoc_dh_width_reg.regValue = 0;
    txsoctg_txsoc_dh_width_reg.dh_width = TXSOC_DH_WIDTH -1;
    rtd_outl(TXSOCTG_TXSOC_DH_Width_reg, txsoctg_txsoc_dh_width_reg.regValue);

    txsoctg_tx_soc_tg_double_buffer_ctrl_reg.regValue = rtd_inl(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg);
    txsoctg_tx_soc_tg_double_buffer_ctrl_reg.tx_soc_tg_db_en = 1;
    rtd_outl(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg, txsoctg_tx_soc_tg_double_buffer_ctrl_reg.regValue);
    //rtd_outl(0xb8021808,0x031f031f); // H-total: 4N-1
    txsoctg_txsoc_dh_total_last_line_length_reg.regValue = 0;
    txsoctg_txsoc_dh_total_last_line_length_reg.dh_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;//(width-1) + TXSOC_H_PORCH; // sync to vo h-total
    txsoctg_txsoc_dh_total_last_line_length_reg.dh_total_last_line = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;//(width-1) + TXSOC_H_PORCH; // sync to vo h-total
    rtd_outl(TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg, txsoctg_txsoc_dh_total_last_line_length_reg.regValue);

    txsoctg_tx_soc_tg_double_buffer_ctrl_reg.regValue = rtd_inl(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg);
    txsoctg_tx_soc_tg_double_buffer_ctrl_reg.tx_soc_tg_db_rdy = 1;
    rtd_outl(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg, txsoctg_tx_soc_tg_double_buffer_ctrl_reg.regValue);

    Wait_TXSOC_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
    Wait_TXSOC_Den_Start((unsigned char*)__FUNCTION__, __LINE__);

    //rtd_outl(0xb802180c,0x00000002); // vsync width =1N-1
    txsoctg_txosc_dv_length_reg.regValue = 0;
    txsoctg_txosc_dv_length_reg.dv_length = TXSOC_DV_LENGTH -1;
    rtd_outl(TXSOCTG_TXOSC_DV_Length_reg, txsoctg_txosc_dv_length_reg.regValue);

    //rtd_outl(0xb8021810,0x00000225); // DV total = 1N-1
    txsoctg_txsoc_dv_total_reg.regValue = 0;
    txsoctg_txsoc_dv_total_reg.dv_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_V_LEN) -1;//0x3fff;//v-sync by v-sync source: vodma
    rtd_outl(TXSOCTG_TXSOC_DV_total_reg, txsoctg_txsoc_dv_total_reg.regValue);

    //rtd_outl(0xb8021814,0x00400298); // h den start = 0x40, den end = 0x298
    txsoctg_txsoc_dh_den_start_end_reg.regValue = 0;
    txsoctg_txsoc_dh_den_start_end_reg.dh_den_sta = TXSOC_TG_H_DEN_STA;
    txsoctg_txsoc_dh_den_start_end_reg.dh_den_end = TXSOC_TG_H_DEN_STA + width;
    rtd_outl(TXSOCTG_TXSOC_DH_DEN_Start_End_reg, txsoctg_txsoc_dh_den_start_end_reg.regValue);

    //rtd_outl(0xb8021818,0x001901f9); // v_den start=0x19, den end = 0x1f9
    txsoctg_txosc_dv_den_start_end_reg.regValue = 0;
    txsoctg_txosc_dv_den_start_end_reg.dv_den_sta = TXSOC_TG_V_DEN_STA;
    txsoctg_txosc_dv_den_start_end_reg.dv_den_end = TXSOC_TG_V_DEN_STA + height;
    rtd_outl(TXSOCTG_TXOSC_DV_DEN_Start_End_reg, txsoctg_txosc_dv_den_start_end_reg.regValue);

    //rtd_outl(0xb8021824,0x00400298); // act h start = 0x40, h end = 0x298
    txsoctg_txsoc_active_h_start_end_reg.regValue = 0;
    txsoctg_txsoc_active_h_start_end_reg.dh_act_sta = TXSOC_TG_H_DEN_STA;
    txsoctg_txsoc_active_h_start_end_reg.dh_act_end = TXSOC_TG_H_DEN_STA + width;
    rtd_outl(TXSOCTG_TXSOC_Active_H_Start_End_reg, txsoctg_txsoc_active_h_start_end_reg.regValue);

    //rtd_outl(0xb8021828,0x001901f9); // act v star=0x19, v-end=0x1f9
    txsoctg_txsoc_active_v_start_end_reg.regValue = 0;
    txsoctg_txsoc_active_v_start_end_reg.dv_act_sta = TXSOC_TG_V_DEN_STA;
    txsoctg_txsoc_active_v_start_end_reg.dv_act_end = TXSOC_TG_V_DEN_STA + height;
    rtd_outl(TXSOCTG_TXSOC_Active_V_Start_End_reg, txsoctg_txsoc_active_v_start_end_reg.regValue);

    //rtd_outl(0xb8021854,0x00000000); // IV2DV line
    txsoctg_txsoc_fs_iv_dv_fine_tuning5_reg.regValue = 0;
    txsoctg_txsoc_fs_iv_dv_fine_tuning5_reg.iv2dv_line = 0;
    txsoctg_txsoc_fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 0;
    rtd_outl(TXSOCTG_TXSOC_FS_IV_DV_Fine_Tuning5_reg, txsoctg_txsoc_fs_iv_dv_fine_tuning5_reg.regValue);

    // vsync tracking setting - enable when Fsync_ivs_sel/Fix_last_line_sync_sel = 0:pcr_tracking vsync(A)(B) counter_vs (tg master)
    txsoctg_txsoc_pcr_cnt_reg.regValue = 0;
    txsoctg_txsoc_pcr_cnt_reg.pcr_count_en = 0;
    txsoctg_txsoc_pcr_cnt_reg.pcr_count_vs_clk_sel = 0; // 00: PCR A clock (PLLDDS 27MHz), 11: crystal clock
    txsoctg_txsoc_pcr_cnt_reg.count_vs_period_update = 1;
    txsoctg_txsoc_pcr_cnt_reg.count_vs_period = ((27000000*10)/frame_rate); //VSYNC_PERIOD;//VSYNC_PERIOD_60HZ; // 60hz
    rtd_outl(TXSOCTG_TXSOC_PCR_cnt_reg, txsoctg_txsoc_pcr_cnt_reg.regValue);

    txsoctg_txsoc_display_timing_ctrl1_reg.regValue = rtd_inl(TXSOCTG_TXSOC_Display_Timing_CTRL1_reg);
    //rtd_outl(0xb8021800,0x04000380); // disp_fsync_en=1, fsync_ivs_sel=txsoc_uzd_vs (vodma master)
    txsoctg_txsoc_display_timing_ctrl1_reg.disp_frc_fsync = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.disp_frc_on_fsync = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.disp_fix_last_line = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.disp_fsync_en = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.wde_to_free_run = 0;
 //   txsoctg_txsoc_display_timing_ctrl1_reg.disp_en = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.wde_to_bg = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.fsync_ivs_sel = 3; // 0:pcr_tracking vsync(A)(B) counter_vs (tg master), 3.txsoc_uzd_vs (vodma master)
    txsoctg_txsoc_display_timing_ctrl1_reg.frc2fsync_protection = 1;
    txsoctg_txsoc_display_timing_ctrl1_reg.frc2fsync_speedup_period = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.frc2fsync_speedup_en = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.frc2fsync_en = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.force_bg = 0;
    txsoctg_txsoc_display_timing_ctrl1_reg.fix_last_line_sync_sel = 3; // 0:pcr_tracking vsync(A)(B) counter_vs, 3: txsoc_uzd_vs
    txsoctg_txsoc_display_timing_ctrl1_reg.dv_rst_sscg_alignok_sel = 0;
    rtd_outl(TXSOCTG_TXSOC_Display_Timing_CTRL1_reg, txsoctg_txsoc_display_timing_ctrl1_reg.regValue);

    ucColorSpace = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
    // enable TXSOC TG forceBG
    if((DBG_EN_PTG == 0) && (DBG_EN_TOGGLE_PTG == 0)){
        txsoctg_txsoc_display_background_color_RBUS txsoctg_txsoc_display_background_color_reg;
        txsoctg_txsoc_display_background_color_2_RBUS txsoctg_txsoc_display_background_color_2_reg;
        txsoctg_txsoc_display_background_color_reg.regValue = rtd_inl(TXSOCTG_TXSOC_Display_Background_Color_reg);
        txsoctg_txsoc_display_background_color_2_reg.regValue = rtd_inl(TXSOCTG_TXSOC_Display_Background_Color_2_reg);
        txsoctg_txsoc_display_background_color_reg.d_bg_g = 0;
        txsoctg_txsoc_display_background_color_reg.d_bg_b = (ucColorSpace == I3DDMA_COLOR_RGB? 0: 0x800);
        txsoctg_txsoc_display_background_color_2_reg.d_bg_r = (ucColorSpace == I3DDMA_COLOR_RGB? 0: 0x800);
        rtd_outl(TXSOCTG_TXSOC_Display_Background_Color_reg, txsoctg_txsoc_display_background_color_reg.regValue);
        rtd_outl(TXSOCTG_TXSOC_Display_Background_Color_2_reg, txsoctg_txsoc_display_background_color_2_reg.regValue);

        txsoctg_txsoc_display_timing_ctrl1_reg.force_bg = 1;
        rtd_outl(TXSOCTG_TXSOC_Display_Timing_CTRL1_reg, txsoctg_txsoc_display_timing_ctrl1_reg.regValue);
        IoReg_Mask32(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg, ~TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_tx_soc_tg_db_rdy_mask, TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_tx_soc_tg_db_rdy(1));
    }

    //rtd_outl(0xb8021858,0x80000320); // IV to DV Hsync Width=0x320, IV to DV Hsync Self Generator Enable (always 1)
    txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_reg.regValue = 0;
    txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_gen_en = 1;
    txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = TXSOC_IV2DV_HSYNC_WIDTH;
    rtd_outl(TXSOCTG_TXSOC_dctl_iv2dv_ihs_ctrl_reg, txsoctg_txsoc_dctl_iv2dv_ihs_ctrl_reg.regValue);

    // TXSOC DS444 control, ds444_mode_sel = 00: disable, 1 : 444 to 420, 2: 444 to 422 (drop mode), 3: 444 to 422 (average mode)
    txsoc_ds444_txsoc_ds444_ctrl0_reg.regValue = rtd_inl(TXSOC_DS444_txsoc_ds444_ctrl0_reg);
    txsoc_ds444_txsoc_ds444_ctrl0_reg.ds444_mode_sel = (ucColorSpace == I3DDMA_COLOR_YUV411? 1: (ucColorSpace == I3DDMA_COLOR_YUV422? 3: 0));
    rtd_outl(TXSOC_DS444_txsoc_ds444_ctrl0_reg, txsoc_ds444_txsoc_ds444_ctrl0_reg.regValue);

    //rtd_outl(0xb8021800,0x05000380); // disp_en=1
    txsoctg_txsoc_display_timing_ctrl1_reg.disp_en = 1;
    rtd_outl(TXSOCTG_TXSOC_Display_Timing_CTRL1_reg, txsoctg_txsoc_display_timing_ctrl1_reg.regValue);

    DebugMessageHDMITx("[HDMI_TX] %s Done!\n", __FUNCTION__);
#endif // [MARK2] FIX-ME -- replace TXSOC setting
    return;
}


void HDMITX_verifier_init_txsoc_ptg(I3DDMA_COLOR_SPACE_T colorFmt)
{
#if 0//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    unsigned int width_src;

    DebugMessageHDMITx("[HDMI_TX] %s...\n", __FUNCTION__);

    // TXSOC VGIP PTG
    rtd_outl(TX_SOC_VGIP_TX_SOC_VGIP_DOUBLE_BUFFER_CTRL_reg, 0); // disable TXSOC VGIP db_en
    rtd_outl(TX_SOC_VGIP_PTG_TX_SOC_PTG_CTRL_reg, _BIT22|_BIT4|_BIT2); // enable TXSOC VGIP colorbar PTG (vertical 4 color)

    // TXSOC UZU PTG
    width_src = TX_SOC_VGIP_TX_SOC_VGIP_ACT_HSTA_Width_get_tx_soc_ih_act_wid(rtd_inl(TX_SOC_VGIP_TX_SOC_VGIP_ACT_HSTA_Width_reg));

    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_COLOR0_reg, 0x00ffffff);
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_COLOR1_reg, 0x00ff0000);
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_COLOR2_reg, 0x0000ff00);
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_COLOR3_reg, 0x000000ff);

    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE_reg, _BIT29|_BIT28|_BIT27|(DBG_EN_TOGGLE_PTG << 25)|((width_src/4)-1)); // PTG width B[21:0], PTG toggle B[26:25]: 0: no toggle, 1: 0/1 toggle, 2: 0/1/2 toggle, 3: 0/1/2/3 toggle
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE1_reg, (width_src/8)-1);
    rtd_outl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE2_reg, (width_src/12)-1);

    // YUV2RGB enable control
    if(colorFmt == I3DDMA_COLOR_RGB){ // RGB
        drvif_color_txsoc_rgb2yuv_en(false);
        drvif_color_txsoc_yuv2rgb_en(true);
    }else{ // YUV
        drvif_color_txsoc_rgb2yuv_en(false);
        drvif_color_txsoc_yuv2rgb_en(false);
    }

    // enable PTG
    rtd_maskl(TXSOC_SCALEUP_TXSOC_UZU_Globle_Ctrl_reg, ~_BIT0, _BIT5|_BIT0); // enable main PTG, // PTG B[6:4]: 1: randon, 2: build-in 4 color, 3: build-in 8 color, 4: self 4 color, 5: self 8 color
#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting
    return;
}



// [MARK2] Panel init for HDMITX ooutput flow
void ScalerHdmiTxSetPanelParameter_init(void)
{
    memset(&hdmitxPanelParameter,0,sizeof(PANEL_CONFIG_PARAMETER));
    hdmitxPanelParameter.iCONFIG_BACKLIGHT_PWM_DUTY=CONFIG_BACKLIGHT_PWM_DUTY;
    hdmitxPanelParameter.iCONFIG_BACKLIGHT_PWM_FREQ=CONFIG_BACKLIGHT_PWM_FREQ;
    hdmitxPanelParameter.iCONFIG_DEFAULT_DPLL_M_DIVIDER=CONFIG_DEFAULT_DPLL_M_DIVIDER;
    hdmitxPanelParameter.iCONFIG_DEFAULT_DPLL_N_DIVIDER=CONFIG_DEFAULT_DPLL_N_DIVIDER;
    hdmitxPanelParameter.iCONFIG_DISPLAY_BITMAPPING_TABLE=CONFIG_DISPLAY_BITMAPPING_TABLE;
    hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_MAX=CONFIG_DISPLAY_CLOCK_MAX;
    hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_TYPICAL=CONFIG_DISPLAY_CLOCK_TYPICAL;
    hdmitxPanelParameter.iCONFIG_DISPLAY_COLOR_BITS=CONFIG_DISPLAY_COLOR_BITS;
    hdmitxPanelParameter.iCONFIG_DISPLAY_EVEN_RSV1_BIT=CONFIG_DISPLAY_EVEN_RSV1_BIT;
#ifdef CONFIG_DISPLAY_HORIZONTAL_SYNC_NORMAL
    hdmitxPanelParameter.iCONFIG_DISPLAY_HORIZONTAL_SYNC_NORMAL=CONFIG_DISPLAY_HORIZONTAL_SYNC_NORMAL;
#endif
    hdmitxPanelParameter.iCONFIG_DISPLAY_MSB_LSB_SWAP=CONFIG_DISPLAY_MSB_LSB_SWAP;
    hdmitxPanelParameter.iCONFIG_DISPLAY_ODD_RSV1_BIT=CONFIG_DISPLAY_ODD_RSV1_BIT;
    hdmitxPanelParameter.iCONFIG_DISPLAY_OUTPUT_INVERSE=CONFIG_DISPLAY_OUTPUT_INVERSE;
    hdmitxPanelParameter.iCONFIG_DISPLAY_PORT=CONFIG_DISPLAY_PORT;
    hdmitxPanelParameter.iCONFIG_DISPLAY_PORTAB_SWAP=CONFIG_DISPLAY_PORTAB_SWAP;
    hdmitxPanelParameter.iCONFIG_DISPLAY_RATIO_4X3=CONFIG_DISPLAY_RATIO_4X3;
    //hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_INVERSE=CONFIG_DISPLAY_CLOCK_INVERSE;
    hdmitxPanelParameter.iCONFIG_DISPLAY_RED_BLUE_SWAP=CONFIG_DISPLAY_RED_BLUE_SWAP;
    hdmitxPanelParameter.iCONFIG_DISPLAY_REFRESH_RATE=CONFIG_DISPLAY_REFRESH_RATE;
    hdmitxPanelParameter.iCONFIG_DISPLAY_SKEW_DATA_OUTPUT=CONFIG_DISPLAY_SKEW_DATA_OUTPUT;
#ifdef CONFIG_DISPLAY_VERTICAL_SYNC_NORMAL
    hdmitxPanelParameter.iCONFIG_DISPLAY_VERTICAL_SYNC_NORMAL=CONFIG_DISPLAY_VERTICAL_SYNC_NORMAL;
#endif
    hdmitxPanelParameter.iCONFIG_DISP_ACT_END_HPOS=CONFIG_DISP_ACT_END_HPOS;
    hdmitxPanelParameter.iCONFIG_DISP_ACT_END_VPOS=CONFIG_DISP_ACT_END_VPOS;
    hdmitxPanelParameter.iCONFIG_DISP_ACT_STA_HPOS=CONFIG_DISP_ACT_STA_HPOS;
    hdmitxPanelParameter.iCONFIG_DISP_ACT_STA_VPOS=CONFIG_DISP_ACT_STA_VPOS;
    hdmitxPanelParameter.iCONFIG_DISP_DCLK_DELAY=CONFIG_DISP_DCLK_DELAY;
    hdmitxPanelParameter.iCONFIG_DISP_DEN_END_HPOS=CONFIG_DISP_DEN_END_HPOS;
    hdmitxPanelParameter.iCONFIG_DISP_DEN_END_VPOS=CONFIG_DISP_DEN_END_VPOS;
    hdmitxPanelParameter.iCONFIG_DISP_DEN_STA_HPOS=CONFIG_DISP_DEN_STA_HPOS;
    hdmitxPanelParameter.iCONFIG_DISP_DEN_STA_VPOS=CONFIG_DISP_DEN_STA_VPOS;
    hdmitxPanelParameter.iCONFIG_DISP_HORIZONTAL_TOTAL=(CONFIG_DISP_HORIZONTAL_TOTAL & (~_BIT0)) ; //Dh total need as even
    hdmitxPanelParameter.iCONFIG_DISP_HSYNC_LASTLINE=CONFIG_DISP_HSYNC_LASTLINE;
    hdmitxPanelParameter.iCONFIG_DISP_HSYNC_WIDTH=CONFIG_DISP_HSYNC_WIDTH;
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL=CONFIG_DISP_VERTICAL_TOTAL;
    hdmitxPanelParameter.iCONFIG_DISP_VSYNC_LENGTH=CONFIG_DISP_VSYNC_LENGTH;
    hdmitxPanelParameter.iLIGHT_TO_LDVS_OFF_ms=LIGHT_TO_LDVS_OFF_ms;
    hdmitxPanelParameter.iLVDS_TO_LIGHT_ON_ms=LVDS_TO_LIGHT_ON_ms;
    hdmitxPanelParameter.iLVDS_TO_PANEL_OFF_ms=LVDS_TO_PANEL_OFF_ms;
    hdmitxPanelParameter.iPANEL_OFF_TO_ON_ms=PANEL_OFF_TO_ON_ms;
    hdmitxPanelParameter.iPANEL_TO_LVDS_ON_ms=PANEL_TO_LVDS_ON_ms;
    hdmitxPanelParameter._iCONFIG_DISP_ACT_STA_BIOS=_CONFIG_DISP_ACT_STA_BIOS;
    hdmitxPanelParameter.iFIX_LAST_LINE_ENABLE = FIX_LAST_LINE_ENABLE;
    hdmitxPanelParameter.iFIX_LAST_LINE_4X_ENABLE = FIX_LAST_LINE_4X_ENABLE;
    hdmitxPanelParameter.iVFLIP = CONFIG_VFLIP_ON;
    hdmitxPanelParameter.iPICASSO_CONTROL_ON = CONFIG_PICASSO_CONTROL_ON;

    hdmitxPanelParameter.iCONFIG_DISPLAY_PORT_CONFIG1 = CONFIG_DISPLAY_PORT_CONFIG1;
    hdmitxPanelParameter.iCONFIG_DISPLAY_PORT_CONFIG2 = CONFIG_DISPLAY_PORT_CONFIG2;
    //#ifdef VBY_ONE_PANEL
    //#if defined(ENABLE_AOCUI)
    hdmitxPanelParameter.iCONFIG_SR_MODE = CONFIG_SR_MODE;
    hdmitxPanelParameter.iCONFIG_SR_PIXEL_MODE = CONFIG_SR_PIXEL_MODE;
    //#endif
    hdmitxPanelParameter.iCONFIG_SFG_SEG_NUM = CONFIG_SFG_SEG_NUM;
    hdmitxPanelParameter.iCONFIG_SFG_PORT_NUM = CONFIG_SFG_PORT_NUM;
    //#endif

    hdmitxPanelParameter.iCONFIG_PANEL_TYPE = CONFIG_PANEL_TYPE;
    hdmitxPanelParameter.iCONFIG_PANEL_CUSTOM_INDEX = CONFIG_PANEL_CUSTOM_INDEX;

    //Panel_InitParameter(&hdmitxPanelParameter);
}


void ScalerHdmiTxSetPanelParameter_config(int timing_type)
{
    #define PANEL_NAME_LEN 32
    hdmi_tx_output_timing_type *tx_output_timing;
    hdmi_tx_timing_gen_st *timing_gen;
    PANEL_TYPE hdmitx_panelType;

    if((timing_type < 0)|| (timing_type >= TX_TIMING_NUM)){
        ErrorMessageHDMITx("[HDMITX][ERROR] set input timing_type[%d] OVER RANGE@%s\n", timing_type, __FUNCTION__);
        return;
    }

    tx_output_timing = &tx_output_timing_table[timing_type];
    timing_gen = &tx_cea_timing_table[timing_type];

    //[Mark2] FIX-ME, D-domain Not Support 12bit output yet
    hdmitxPanelParameter.iCONFIG_DISPLAY_COLOR_BITS = (tx_output_timing->color_depth == I3DDMA_COLOR_DEPTH_8B? 1: 0);//  <timing_type.color_depth>, 0: 30bits, 1:24 bits, 2: 30bits
    hdmitxPanelParameter.iCONFIG_DISPLAY_PORT = 1;
    hdmitxPanelParameter.iCONFIG_DISPLAY_REFRESH_RATE = tx_output_timing->frame_rate/10;//<timing_type.frame_rate/10>
    hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_TYPICAL = timing_gen->pixelFreq *100000;//(pixelFreq*100000)
    hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_MAX = timing_gen->pixelFreq *110000;// CLOCK_TYPICAL+10%, for frame sync clock checking
    hdmitxPanelParameter.iCONFIG_DISPLAY_CLOCK_MIN = timing_gen->pixelFreq *90000;//CLOCK_TYPICAL-10%, for frame sync clock checking
    hdmitxPanelParameter.iCONFIG_DISP_HORIZONTAL_TOTAL = (timing_gen->h.blank + timing_gen->h.active);//(h.blank+h.active)
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL = (timing_gen->v.blank + timing_gen->v.active);//(v.blank+v.act)
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MIN = (timing_gen->v.blank + timing_gen->v.active - (timing_gen->v.blank/2));//VERTICAL_TOTAL-(porch/2), for framesync_lastlinefinetune
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL_50Hz_MAX = (timing_gen->v.blank + timing_gen->v.active + 500);//50Hz VERTICAL_TOTAL+500, for framesync_lastlinefinetune
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MIN = (timing_gen->v.blank + timing_gen->v.active - (timing_gen->v.blank/2));;//60Hz VERTICAL_TOTAL-(porch/2), for framesync_lastlinefinetune
    hdmitxPanelParameter.iCONFIG_DISP_VERTICAL_TOTAL_60Hz_MAX = (timing_gen->v.blank + timing_gen->v.active + 500);//60Hz VERTICAL_TOTAL+500, for framesync_lastlinefinetune
    hdmitxPanelParameter.iCONFIG_DISP_HSYNC_WIDTH = timing_gen->h.sync;//(h.sync)
    hdmitxPanelParameter.iCONFIG_DISP_VSYNC_LENGTH = timing_gen->v.sync;//(v.sync)
    hdmitxPanelParameter.iCONFIG_DISP_DEN_STA_HPOS = (timing_gen->h.sync + timing_gen->h.back);//(h.sync+h.back)
    hdmitxPanelParameter.iCONFIG_DISP_DEN_END_HPOS = (timing_gen->h.sync + timing_gen->h.back + timing_gen->h.active);//(h.sync+h.back+h.active)
    hdmitxPanelParameter.iCONFIG_DISP_DEN_STA_VPOS = (timing_gen->v.sync + timing_gen->v.back);//(v.sync+v.back)
    hdmitxPanelParameter.iCONFIG_DISP_DEN_END_VPOS = (timing_gen->v.sync + timing_gen->v.back + timing_gen->v.active);//(v.sync+v.back+v.active)
    hdmitxPanelParameter.iCONFIG_DISP_ACT_STA_HPOS = 0;// (same)
    hdmitxPanelParameter.iCONFIG_DISP_ACT_END_HPOS = timing_gen->h.active;//(h.active)
    hdmitxPanelParameter.iCONFIG_DISP_ACT_STA_VPOS = 0;// (same)
    hdmitxPanelParameter.iCONFIG_DISP_ACT_END_VPOS = timing_gen->v.active;//(v.active)
    hdmitxPanelParameter.iCONFIG_DISP_HSYNC_LASTLINE = (timing_gen->h.blank + timing_gen->h.active);//(h.blank+h.active)
    hdmitxPanelParameter.iCONFIG_SR_PIXEL_MODE = 0;// used for SLR_INPUT_PIXEL_MODE
    hdmitx_panelType = (tx_output_timing->width > 4096? P_VBY1_8K4K: (tx_output_timing->width > 2048? P_VBY1_4K2K: P_LVDS_2K1K));
    hdmitxPanelParameter.iCONFIG_PANEL_TYPE = hdmitx_panelType;// used by Get_DISPLAY_PANEL_TYPE()
    hdmitxPanelParameter.iCONFIG_PANEL_CUSTOM_INDEX = 0;// used by Get_DISPLAY_PANEL_CUSTOM_INDEX()
    snprintf((char *)&hdmitxPanelParameter.sPanelName, PANEL_NAME_LEN-1, "HDMITX_%d_%dx%dp%d_%db",
            timing_type, tx_output_timing->width, tx_output_timing->height, tx_output_timing->frame_rate/10, (hdmitxPanelParameter.iCONFIG_DISPLAY_COLOR_BITS == 1? 8: 10));

    NoteMessageHDMITx("[HDMITX] Video=%dx%d, panelType=%d\n", tx_output_timing->width, tx_output_timing->height, hdmitx_panelType);
    Panel_InitParameter(&hdmitxPanelParameter);
}


void setHdmitxPatternGen(unsigned int width)
{

   rtd_pr_hdmi_notice("[iRealOneTX] %s@Width[%d]\n", __FUNCTION__, width);

    //==================================== Pattern Generator ====================================
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_CTRL_0_reg,20,0, (width/8) - 1); // [dsce    ]<-[mcu] // [31]: patgen_en = d'0 ,[30:29] : PATGEN_RAND_MODE, [21:0]: PATGEN_WIDTH = 479+1 (=width/8 -1)

    //1st color -- white
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C0_RG_reg,23,12,3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C0_RG_reg,11,0,3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C0_B_reg,11,0,3840);
    //2nd color -- red
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C1_RG_reg,23,12, 0);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C1_RG_reg,11,0, 3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C1_B_reg,11,0, 0);
    //3rd color -- green
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C2_RG_reg,23,12, 3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C2_RG_reg,11,0, 0);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C2_B_reg,11,0, 0);
    //4th color -- blue
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C3_RG_reg,23,12, 0);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C3_RG_reg,11,0, 0);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C3_B_reg,11,0, 3840);
    //5th color -- grey
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C4_RG_reg,23,12,2048);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C4_RG_reg,11,0,2048);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C4_B_reg,11,0,2048);
    //6th color -- orange
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C5_RG_reg,23,12, 1536);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C5_RG_reg,11,0, 3968);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C5_B_reg,11,0, 0);
    //7th color -- yellow
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C6_RG_reg,23,12, 3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C6_RG_reg,11,0, 3840);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C6_B_reg,11,0, 0);
    //8th color -- brown
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C7_RG_reg,23,12, 768);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C7_RG_reg,11,0, 1792);
    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_C7_B_reg,11,0, 0);

    rtd_part_outl( DSCE_MISC_DSC_PATTERN_GEN_CTRL_0_reg, 31,31, 1);
    //==================================== Pattern Generator done ====================================

    return;
}



void dvScript_mainDtg_config(unsigned int width, unsigned height, unsigned short frame_rate)
{
//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
    ppoverlay_dispd_smooth_toggle1_RBUS ppoverlay_dispd_smooth_toggle1_reg;
    //ppoverlay_uzudtg_4k8k_ctrl0_RBUS ppoverlay_uzudtg_4k8k_ctrl0_reg;
    dispd_ds444_dispd_ds444_crc_ctrl_RBUS dispd_ds444_dispd_ds444_crc_ctrl_reg;
    scaleup_dm_uzu_input_size_RBUS scaleup_dm_uzu_input_size_reg;
    scaleup3_dm_uzu3_input_size_RBUS reg_scaleup3_dm_uzu3_input_size_reg;
    scaleup_d_uzu_patgen_frame_toggle_RBUS scaleup_d_uzu_patgen_frame_toggle_reg;
    scaleup_d_uzu_globle_ctrl_RBUS scaleup_d_uzu_globle_ctrl_reg;
    scaleup_dm_uzu_ctrl_RBUS scaleup_dm_uzu_ctrl_reg;
    ppoverlay_main_den_h_start_end_RBUS ppoverlay_main_den_h_start_end_reg;
    ppoverlay_main_den_v_start_end_RBUS ppoverlay_main_den_v_start_end_reg;
    ppoverlay_main_background_h_start_end_RBUS ppoverlay_main_background_h_start_end_reg;
    ppoverlay_main_background_v_start_end_RBUS ppoverlay_main_background_v_start_end_reg;
    ppoverlay_main_active_h_start_end_RBUS ppoverlay_main_active_h_start_end_reg;
    ppoverlay_main_active_v_start_end_RBUS ppoverlay_main_active_v_start_end_reg;
    ppoverlay_main_active_h_start_end_new_RBUS ppoverlay_main_active_h_start_end_new_reg;
    ppoverlay_main_display_control_rsv_RBUS ppoverlay_main_display_control_rsv_reg;
    ppoverlay_dh_width_RBUS ppoverlay_dh_width_reg;
    ppoverlay_dh_den_start_end_RBUS ppoverlay_dh_den_start_end_reg;
    ppoverlay_dh_total_last_line_length_RBUS ppoverlay_dh_total_last_line_length_reg;
    ppoverlay_dv_length_RBUS ppoverlay_dv_length_reg;
    ppoverlay_dv_den_start_end_RBUS ppoverlay_dv_den_start_end_reg;
    ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
    ppoverlay_uzudtg_dv_total_RBUS ppoverlay_uzudtg_dv_total_reg;
    ppoverlay_uzudtg_dh_total_RBUS ppoverlay_uzudtg_dh_total_reg;
    ppoverlay_uzudtg_countrol_RBUS ppoverlay_uzudtg_countrol_reg;
    ppoverlay_memcdtg_dv_total_RBUS ppoverlay_memcdtg_dv_total_reg;
    ppoverlay_memcdtg_dh_total_RBUS ppoverlay_memcdtg_dh_total_reg;
    ppoverlay_memcdtg_dv_den_start_end_RBUS ppoverlay_memcdtg_dv_den_start_end_reg;
    ppoverlay_memcdtg_dh_den_start_end_RBUS ppoverlay_memcdtg_dh_den_start_end_reg;
    ppoverlay_memcdtg_control_RBUS ppoverlay_memcdtg_control_reg;
    ppoverlay_display_timing_ctrl1_RBUS ppoverlay_display_timing_ctrl1_reg;
    ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
    ppoverlay_fs_iv_dv_fine_tuning2_RBUS ppoverlay_fs_iv_dv_fine_tuning2_reg;
    ppoverlay_fs_iv_dv_fine_tuning5_RBUS ppoverlay_fs_iv_dv_fine_tuning5_reg;
    ppoverlay_dctl_iv2dv_ihs_ctrl_RBUS ppoverlay_dctl_iv2dv_ihs_ctrl_reg;
    ppoverlay_fix_last_line_pcr_ctrl_RBUS ppoverlay_fix_last_line_pcr_ctrl_reg;
    pll27x_reg_sys_dclkss_RBUS pll27x_reg_sys_dclkss_reg;
    unsigned char ucColorSpace;
    two_step_uzu_dm_two_step_sr_input_size_RBUS two_step_uzu_dm_two_step_sr_input_size_reg;
    ppoverlay_osddtg_dv_total_RBUS ppoverlay_osddtg_dv_total_reg;
    ppoverlay_osddtg_dh_total_RBUS ppoverlay_osddtg_dh_total_reg;
    ppoverlay_osddtg_dv_den_start_end_RBUS ppoverlay_osddtg_dv_den_start_end_reg;
    ppoverlay_osddtg_dh_den_start_end_RBUS ppoverlay_osddtg_dh_den_start_end_reg;
    ppoverlay_osddtg_control_RBUS ppoverlay_osddtg_control_reg;

    NoteMessageHDMITx("[HDMITX] DTG Setting: %dx%dp%d\n", width, height, frame_rate);

#if 1 //#ifndef _MARK2_ZEBU_BRING_UP_RUN // [MARK2] CHECK-ME -- only for Zebu Verify (wait HDMI XTOR ready)
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
#endif

    // disable DTG clock
    rtd_clearbits(PPOVERLAY_Display_Timing_CTRL1_reg, PPOVERLAY_Display_Timing_CTRL1_disp_en_mask);

    // disable UZU
    rtd_clearbits(SCALEUP_DM_UZU_Ctrl_reg, SCALEUP_DM_UZU_Ctrl_v_zoom_en_mask|SCALEUP_DM_UZU_Ctrl_h_zoom_en_mask);
    rtd_clearbits(TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_reg, TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_v_zoom_en_mask|TWO_STEP_UZU_DM_TWO_STEP_SR_Ctrl_h_zoom_en_mask);
    rtd_clearbits(PPOVERLAY_uzudtg_control1_reg, PPOVERLAY_uzudtg_control1_third_uzu_mode_mask);

    ppoverlay_display_timing_ctrl2_reg.regValue = rtd_inl(PPOVERLAY_Display_Timing_CTRL2_reg);
    //rtd_outl(0xB8028004, 0x80060008);
    // [MARK2] CHECK
    //ppoverlay_display_timing_ctrl2_reg.dispd2hdmitx_clk_en = 1;
    ppoverlay_display_timing_ctrl2_reg.shpnr_line_mode_sel = 0;
    ppoverlay_display_timing_ctrl2_reg.d2i3ddma_src_sel = 6;
    ppoverlay_display_timing_ctrl2_reg.orbit_timing_en = 0;
    ppoverlay_display_timing_ctrl2_reg.frc2fsync_protection = 1;
    ppoverlay_display_timing_ctrl2_reg.frc2fsync_speedup_period = 0;
    ppoverlay_display_timing_ctrl2_reg.frc2fsync_speedup_en = 0;
    ppoverlay_display_timing_ctrl2_reg.frc2fsync_en = 0;

    // TXSOC DS444 control, ds444_mode_sel = 0: 444to444 (disable, willbypass from input to output),
    // 1: 444 to 422 (drop mode), 2: 444 to 422 (average_h mode), 3: 444 to 420 (drop mode), 4: 444 to 420 (average_h_mode), 5: 444 to 420 (average_v mode)
    ucColorSpace = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
    ppoverlay_display_timing_ctrl2_reg.ds444_mode_sel = (ucColorSpace == I3DDMA_COLOR_YUV411? 4: (ucColorSpace == I3DDMA_COLOR_YUV422? 2: 0));
    rtd_outl(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

    udelay(150);

    // disable DCLK Spread Spectrum
    pll27x_reg_sys_dclkss_reg.regValue = rtd_inl(PLL27X_REG_SYS_DCLKSS_reg);
    pll27x_reg_sys_dclkss_reg.dclk_new_en = 0;
    pll27x_reg_sys_dclkss_reg.dclk_ss_load = 0;
    pll27x_reg_sys_dclkss_reg.dclk_ss_en = 0;
    rtd_outl(PLL27X_REG_SYS_DCLKSS_reg, pll27x_reg_sys_dclkss_reg.regValue);

    udelay(150);

    //rtd_outl(0xb802811c, 0x00000005);
    ppoverlay_dispd_smooth_toggle1_reg.regValue = rtd_inl(PPOVERLAY_dispd_smooth_toggle1_reg);
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_clk_mode_revised = 1; // stage1 frac_a/b modify reg will latch by uzudtg delay done vsync.: 0:by vsync latch, 1:use old mode
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_src_sel = 0;
    ppoverlay_dispd_smooth_toggle1_reg.dispd_stage1_smooth_toggle_apply = 1;
    rtd_outl(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle1_reg.regValue);

    #if 0 // [MARK2] removed from mark2
    //rtd_outl(0xb8028134, 0x00000000);
    ppoverlay_uzudtg_4k8k_ctrl0_reg.regValue = rtd_inl(PPOVERLAY_UZUDTG_4K8K_CTRL0_reg);
    ppoverlay_uzudtg_4k8k_ctrl0_reg.uzudtg_sr_timing_sel = 0;
    ppoverlay_uzudtg_4k8k_ctrl0_reg.ip_uzu_sel = (width > 4096? 1: 0);
    ppoverlay_uzudtg_4k8k_ctrl0_reg.uzudtg_4k8k_timing_sel = 0;
    rtd_outl(PPOVERLAY_UZUDTG_4K8K_CTRL0_reg, ppoverlay_uzudtg_4k8k_ctrl0_reg.regValue);
    #endif

    //rtd_outl(0xb8029028, 0x0f00006c);
    scaleup_dm_uzu_input_size_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Input_Size_reg);
    scaleup_dm_uzu_input_size_reg.hor_input_size = width;
    scaleup_dm_uzu_input_size_reg.ver_input_size = height;
    rtd_outl(SCALEUP_DM_UZU_Input_Size_reg, scaleup_dm_uzu_input_size_reg.regValue);

    two_step_uzu_dm_two_step_sr_input_size_reg.regValue = rtd_inl(TWO_STEP_UZU_DM_TWO_STEP_SR_Input_Size_reg);
    two_step_uzu_dm_two_step_sr_input_size_reg.hor_input_size = width;
    two_step_uzu_dm_two_step_sr_input_size_reg.ver_input_size = height;
    rtd_outl(TWO_STEP_UZU_DM_TWO_STEP_SR_Input_Size_reg, two_step_uzu_dm_two_step_sr_input_size_reg.regValue);

    // [Mark2][IC] New
    reg_scaleup3_dm_uzu3_input_size_reg.regValue = rtd_inl(SCALEUP3_DM_UZU3_Input_Size_reg);
    reg_scaleup3_dm_uzu3_input_size_reg.hor_input_size = width;
    reg_scaleup3_dm_uzu3_input_size_reg.ver_input_size = height;
    rtd_outl(SCALEUP3_DM_UZU3_Input_Size_reg, reg_scaleup3_dm_uzu3_input_size_reg.regValue);

    scaleup_dm_uzu_ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
    scaleup_dm_uzu_ctrl_reg.h_zoom_en = 0;
    scaleup_dm_uzu_ctrl_reg.v_zoom_en = 0;
    rtd_outl(SCALEUP_DM_UZU_Ctrl_reg, scaleup_dm_uzu_ctrl_reg.regValue);

    //rtd_outl(0xb8029074, 0x00000005);
    scaleup_d_uzu_patgen_frame_toggle_reg.regValue = rtd_inl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg);
    scaleup_d_uzu_patgen_frame_toggle_reg.patgen_tog_num = 0;
    scaleup_d_uzu_patgen_frame_toggle_reg.patgen_tog_sel = 0; // 0: no toggle
    scaleup_d_uzu_patgen_frame_toggle_reg.patgen_width = (width > 1920? 0x1df: 0x27);//(width/16)-1;
    rtd_outl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg, scaleup_d_uzu_patgen_frame_toggle_reg.regValue);

    //rtd_outl(0xb8029000, 0x00000020);
    scaleup_d_uzu_globle_ctrl_reg.regValue = rtd_inl(SCALEUP_D_UZU_Globle_Ctrl_reg);
    scaleup_d_uzu_globle_ctrl_reg.uzu_bypass_no_pwrsave = 0;
    scaleup_d_uzu_globle_ctrl_reg.uzu_bypass_force = 0;
    scaleup_d_uzu_globle_ctrl_reg.in_black_en = 0;
    scaleup_d_uzu_globle_ctrl_reg.in_even_black = 0;
    scaleup_d_uzu_globle_ctrl_reg.out_black_en = 0;
    scaleup_d_uzu_globle_ctrl_reg.out_even_black = 0;
    scaleup_d_uzu_globle_ctrl_reg.patgen_sync = 0;
    scaleup_d_uzu_globle_ctrl_reg.patgen_mode = 3; // 2: build-in 4-color, 3: build-in 8-color
    scaleup_d_uzu_globle_ctrl_reg.patgen_field = 0;
    scaleup_d_uzu_globle_ctrl_reg.patgen_sel = 0; // 1: Pattern generator connect to MAIN
    rtd_outl(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);

    //rtd_outl(0xb8028540, 0x00000f00);
    ppoverlay_main_den_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg);
    ppoverlay_main_den_h_start_end_reg.mh_den_sta = 0;
    ppoverlay_main_den_h_start_end_reg.mh_den_end = width;
    rtd_outl(PPOVERLAY_MAIN_DEN_H_Start_End_reg, ppoverlay_main_den_h_start_end_reg.regValue);

    //rtd_outl(0xb8028544, 0x0000006c);
    ppoverlay_main_den_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg);
    ppoverlay_main_den_v_start_end_reg.mv_den_sta = 0;
    ppoverlay_main_den_v_start_end_reg.mv_den_end = height;
    rtd_outl(PPOVERLAY_MAIN_DEN_V_Start_End_reg, ppoverlay_main_den_v_start_end_reg.regValue);

    //rtd_outl(0xb8028548, 0x00000f00);
    ppoverlay_main_background_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Background_H_Start_End_reg);
    ppoverlay_main_background_h_start_end_reg.mh_bg_sta = 0;
    ppoverlay_main_background_h_start_end_reg.mh_bg_end = width;
    rtd_outl(PPOVERLAY_MAIN_Background_H_Start_End_reg, ppoverlay_main_background_h_start_end_reg.regValue);

    //rtd_outl(0xb802854c, 0x0000006c);
    ppoverlay_main_background_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Background_V_Start_End_reg);
    ppoverlay_main_background_v_start_end_reg.mv_bg_sta = 0;
    ppoverlay_main_background_v_start_end_reg.mv_bg_end = height;
    rtd_outl(PPOVERLAY_MAIN_Background_V_Start_End_reg, ppoverlay_main_background_v_start_end_reg.regValue);

    //rtd_outl(0xb8028550, 0x00000f00);
    ppoverlay_main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
    ppoverlay_main_active_h_start_end_reg.mh_act_sta = 0;
    ppoverlay_main_active_h_start_end_reg.mh_act_end = width;
    rtd_outl(PPOVERLAY_MAIN_Active_H_Start_End_reg, ppoverlay_main_active_h_start_end_reg.regValue);

    //rtd_outl(0xb8028554, 0x0000006c);
    ppoverlay_main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);
    ppoverlay_main_active_v_start_end_reg.mv_act_sta = 0;
    ppoverlay_main_active_v_start_end_reg.mv_act_end = height;
    rtd_outl(PPOVERLAY_MAIN_Active_V_Start_End_reg, ppoverlay_main_active_v_start_end_reg.regValue);

    // [Mark2][IC] New
    //rtd_outl(0xb8028558, 0x00000f00);
    ppoverlay_main_active_h_start_end_new_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_New_reg);
    ppoverlay_main_active_h_start_end_new_reg.mh_act_sta_new = 0;
    ppoverlay_main_active_h_start_end_new_reg.mh_act_end_new = width;
    rtd_outl(PPOVERLAY_MAIN_Active_H_Start_End_New_reg, ppoverlay_main_active_h_start_end_new_reg.regValue);

    //rtd_outl(0xb8028300, 0x00000001);
    ppoverlay_main_display_control_rsv_reg.regValue = rtd_inl(PPOVERLAY_Main_Display_Control_RSV_reg);
    ppoverlay_main_display_control_rsv_reg.main_wd_to_background = 0;
    ppoverlay_main_display_control_rsv_reg.main_wd_to_free_run = 0;
    ppoverlay_main_display_control_rsv_reg.m_hbd_en = 0;
    ppoverlay_main_display_control_rsv_reg.m_bd_trans = 0;
    ppoverlay_main_display_control_rsv_reg.m_force_bg = 0;
    ppoverlay_main_display_control_rsv_reg.m_disp_en = 1;
    rtd_outl(PPOVERLAY_Main_Display_Control_RSV_reg, ppoverlay_main_display_control_rsv_reg.regValue);

    //rtd_outl(0xb8028008, 0x00000057);
    ppoverlay_dh_width_reg.regValue = rtd_inl(PPOVERLAY_DH_Width_reg);
    ppoverlay_dh_width_reg.dh_width = DTG_DH_WIDTH -1;
    rtd_outl(PPOVERLAY_DH_Width_reg, ppoverlay_dh_width_reg.regValue);

    //rtd_outl(0xb8028518, 0x01801080);
    ppoverlay_dh_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_DH_DEN_Start_End_reg);
    ppoverlay_dh_den_start_end_reg.dh_den_sta = DTG_H_DEN_STA - 24;
    ppoverlay_dh_den_start_end_reg.dh_den_end = DTG_H_DEN_STA + width - 24 ;
    rtd_outl(PPOVERLAY_DH_DEN_Start_End_reg, ppoverlay_dh_den_start_end_reg.regValue);

    //rtd_outl(0xb802800c, 0x112f112f);
    ppoverlay_dh_total_last_line_length_reg.regValue = rtd_inl(PPOVERLAY_DH_Total_Last_Line_Length_reg);
    ppoverlay_dh_total_last_line_length_reg.dh_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    ppoverlay_dh_total_last_line_length_reg.dh_total_last_line = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    rtd_outl(PPOVERLAY_DH_Total_Last_Line_Length_reg, ppoverlay_dh_total_last_line_length_reg.regValue);

    //rtd_outl(0xb8028010, 0x00000001);
    ppoverlay_dv_length_reg.regValue = rtd_inl(PPOVERLAY_DV_Length_reg);
    ppoverlay_dv_length_reg.dv_length = DTG_DV_LENGTH -1;
    rtd_outl(PPOVERLAY_DV_Length_reg, ppoverlay_dv_length_reg.regValue);

    //rtd_outl(0xb802851c, 0x00080074);
    ppoverlay_dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_DV_DEN_Start_End_reg);
    ppoverlay_dv_den_start_end_reg.dv_den_sta = DTG_V_DEN_STA;
    ppoverlay_dv_den_start_end_reg.dv_den_end = DTG_V_DEN_STA + height;
    rtd_outl(PPOVERLAY_DV_DEN_Start_End_reg, ppoverlay_dv_den_start_end_reg.regValue);

    //rtd_outl(0xb8028014, 0x00000077);
    ppoverlay_dv_total_reg.regValue = rtd_inl(PPOVERLAY_DV_total_reg);
    ppoverlay_dv_total_reg.dv_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_V_LEN) -1;
    rtd_outl(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);

    //rtd_outl(0xb8028504, 0x00000077);
    ppoverlay_uzudtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    ppoverlay_uzudtg_dv_total_reg.uzudtg_dv_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_V_LEN) -1;
    rtd_outl(PPOVERLAY_uzudtg_DV_TOTAL_reg, ppoverlay_uzudtg_dv_total_reg.regValue);

    // rtd_outl(0xb8028508, 0x112f112f);
    ppoverlay_uzudtg_dh_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DH_TOTAL_reg);
    ppoverlay_uzudtg_dh_total_reg.uzudtg_dh_total_last_line = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    ppoverlay_uzudtg_dh_total_reg.uzudtg_dh_total =Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    rtd_outl(PPOVERLAY_uzudtg_DH_TOTAL_reg, ppoverlay_uzudtg_dh_total_reg.regValue);

    //rtd_outl(0xb8028500, 0x00000051);
    ppoverlay_uzudtg_countrol_reg.regValue = rtd_inl(PPOVERLAY_UZUDTG_COUNTROL_reg);
    ppoverlay_uzudtg_countrol_reg.uzudtg_mode_revised = 1;
    ppoverlay_uzudtg_countrol_reg.uzudtg_line_cnt_sync = 1;
    ppoverlay_uzudtg_countrol_reg.uzudtg_frc_fsync_status = 1; // 0 : free run, 1 : frame sync
    ppoverlay_uzudtg_countrol_reg.uzudtg_frc2fsync_by_hw = 0;
    ppoverlay_uzudtg_countrol_reg.uzudtg_fsync_en = 1; // 0: FRC mode, 1: Frame sync mode.
    ppoverlay_uzudtg_countrol_reg.uzudtg_en = 1;
    rtd_outl(PPOVERLAY_UZUDTG_COUNTROL_reg, ppoverlay_uzudtg_countrol_reg.regValue);

    //rtd_outl(0xb8028604, 0x00000077);
    ppoverlay_memcdtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_TOTAL_reg);
    ppoverlay_memcdtg_dv_total_reg.memcdtg_dv_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_V_LEN) -1;
    rtd_outl(PPOVERLAY_memcdtg_DV_TOTAL_reg, ppoverlay_memcdtg_dv_total_reg.regValue);

    //rtd_outl(0xb8028608, 0x112f112f);
    ppoverlay_memcdtg_dh_total_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_TOTAL_reg);
    ppoverlay_memcdtg_dh_total_reg.memcdtg_dh_total_last_line = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    ppoverlay_memcdtg_dh_total_reg.memcdtg_dh_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    rtd_outl(PPOVERLAY_memcdtg_DH_TOTAL_reg, ppoverlay_memcdtg_dh_total_reg.regValue);

    //rtd_outl(0xb8028618, 0x00080074);
    ppoverlay_memcdtg_dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
    ppoverlay_memcdtg_dv_den_start_end_reg.memcdtg_dv_den_sta = DTG_V_DEN_STA;
    ppoverlay_memcdtg_dv_den_start_end_reg.memcdtg_dv_den_end = DTG_V_DEN_STA + height;
    rtd_outl(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg, ppoverlay_memcdtg_dv_den_start_end_reg.regValue);

    //rtd_outl(0xb802861c, 0x01801080);
    ppoverlay_memcdtg_dh_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg);
    ppoverlay_memcdtg_dh_den_start_end_reg.memcdtg_dh_den_sta = DTG_H_DEN_STA;
    ppoverlay_memcdtg_dh_den_start_end_reg.memcdtg_dh_den_end = DTG_H_DEN_STA + width;
    rtd_outl(PPOVERLAY_memcdtg_DH_DEN_Start_End_reg, ppoverlay_memcdtg_dh_den_start_end_reg.regValue);

    ppoverlay_fix_last_line_pcr_ctrl_reg.regValue = rtd_inl(PPOVERLAY_fix_last_line_pcr_ctrl_reg);
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_count_en = 1;
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_clk_sel = 0; // 0: pcr_a_clk, 1: pcr_b_clk
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period_update = 1;
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period = ((27000000*10)/frame_rate); ;
    rtd_outl(PPOVERLAY_fix_last_line_pcr_ctrl_reg, ppoverlay_fix_last_line_pcr_ctrl_reg.regValue);

    //rtd_outl(0xb8028600, 0x00000051);
    ppoverlay_memcdtg_control_reg.regValue = rtd_inl(PPOVERLAY_MEMCDTG_CONTROL_reg);
    ppoverlay_memcdtg_control_reg.memcdtg_mode_revised = 1;
    ppoverlay_memcdtg_control_reg.memcdtg_line_cnt_sync = 1;
    ppoverlay_memcdtg_control_reg.memcdtg_frc_fsync_status = 1; // 0 : free run, 1 : frame sync
    ppoverlay_memcdtg_control_reg.memcdtg_frc2fsync_by_hw = 0;
    ppoverlay_memcdtg_control_reg.memcdtg_fsync_en = 1; // 0: FRC mode, 1: Frame sync mode.
    ppoverlay_memcdtg_control_reg.memcdtg_en = 1;
    rtd_outl(PPOVERLAY_MEMCDTG_CONTROL_reg, ppoverlay_memcdtg_control_reg.regValue);

    ppoverlay_osddtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_osddtg_DV_TOTAL_reg);
    ppoverlay_osddtg_dv_total_reg.osddtg_dv_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_V_LEN)-1;
    rtd_outl(PPOVERLAY_osddtg_DV_TOTAL_reg, ppoverlay_osddtg_dv_total_reg.regValue);

    ppoverlay_osddtg_dh_total_reg.regValue = rtd_inl(PPOVERLAY_osddtg_DH_TOTAL_reg);
    ppoverlay_osddtg_dh_total_reg.osddtg_dh_total_last_line = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    ppoverlay_osddtg_dh_total_reg.osddtg_dh_total = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_H_LEN)-1;
    rtd_outl(PPOVERLAY_osddtg_DH_TOTAL_reg, ppoverlay_osddtg_dh_total_reg.regValue);

    ppoverlay_osddtg_dv_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_osddtg_DV_DEN_Start_End_reg);
    ppoverlay_osddtg_dv_den_start_end_reg.osddtg_dv_den_sta = DTG_V_DEN_STA;
    ppoverlay_osddtg_dv_den_start_end_reg.osddtg_dv_den_end = DTG_V_DEN_STA + height;
    rtd_outl(PPOVERLAY_osddtg_DV_DEN_Start_End_reg, ppoverlay_osddtg_dv_den_start_end_reg.regValue);

    ppoverlay_osddtg_dh_den_start_end_reg.regValue = rtd_inl(PPOVERLAY_osddtg_DH_DEN_Start_End_reg);
    ppoverlay_osddtg_dh_den_start_end_reg.osddtg_dh_den_sta = DTG_H_DEN_STA;
    ppoverlay_osddtg_dh_den_start_end_reg.osddtg_dh_den_end = DTG_H_DEN_STA + width;
    rtd_outl(PPOVERLAY_osddtg_DH_DEN_Start_End_reg, ppoverlay_osddtg_dh_den_start_end_reg.regValue);

    ppoverlay_osddtg_control_reg.regValue = rtd_inl(PPOVERLAY_OSDDTG_CONTROL_reg);
    ppoverlay_osddtg_control_reg.osddtg_mode_revised = 1;
    ppoverlay_osddtg_control_reg.osddtg_line_cnt_sync = 1;
    ppoverlay_osddtg_control_reg.osddtg_frc_fsync_status = 1;
    ppoverlay_osddtg_control_reg.osddtg_fsync_en = 1;
    ppoverlay_osddtg_control_reg.osddtg_en = 1;
    rtd_outl(PPOVERLAY_OSDDTG_CONTROL_reg, ppoverlay_osddtg_control_reg.regValue);

    //rtd_outl(0xb8028000, 0x20000008);
    ppoverlay_display_timing_ctrl1_reg.regValue = rtd_inl(PPOVERLAY_Display_Timing_CTRL1_reg);
    ppoverlay_display_timing_ctrl1_reg.disp_frc_fsync = 0;
    ppoverlay_display_timing_ctrl1_reg.disp_frc_on_fsync = 0;
    ppoverlay_display_timing_ctrl1_reg.disp_line_4x = 1;
    ppoverlay_display_timing_ctrl1_reg.disp_fix_last_line_new = 0;
    ppoverlay_display_timing_ctrl1_reg.disp_fsync_en = 0;
    ppoverlay_display_timing_ctrl1_reg.mdtg_line_cnt_sync = 1;
    ppoverlay_display_timing_ctrl1_reg.disp_en = 0; // 1: Display timing generator runs normally
    ppoverlay_display_timing_ctrl1_reg.wde_to_free_run = 0;
    ppoverlay_display_timing_ctrl1_reg.dout_force_bg = 0;
    ppoverlay_display_timing_ctrl1_reg.fsync_fll_mode = 0;
    //ppoverlay_display_timing_ctrl1_reg.stage1_dly_auto_en = 1;
    ppoverlay_display_timing_ctrl1_reg.fix_last_line_vsync_sel = 0;
    ppoverlay_display_timing_ctrl1_reg.dv_rst_sscg_alignok_sel = 0;
    rtd_outl(PPOVERLAY_Display_Timing_CTRL1_reg, ppoverlay_display_timing_ctrl1_reg.regValue);

    ppoverlay_fs_iv_dv_fine_tuning2_reg.regValue = rtd_inl(PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg);
    ppoverlay_fs_iv_dv_fine_tuning2_reg.iv2dv_pixel = 0;
    ppoverlay_fs_iv_dv_fine_tuning2_reg.iv2dv_pixel2 = 0;
    rtd_outl(PPOVERLAY_FS_IV_DV_Fine_Tuning2_reg, ppoverlay_fs_iv_dv_fine_tuning2_reg.regValue);

    ppoverlay_fs_iv_dv_fine_tuning5_reg.regValue = rtd_inl(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
    ppoverlay_fs_iv_dv_fine_tuning5_reg.iv2dv_line = 0;
    ppoverlay_fs_iv_dv_fine_tuning5_reg.iv2dv_line_2 = 0;
    rtd_outl(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg, ppoverlay_fs_iv_dv_fine_tuning5_reg.regValue);

    ppoverlay_dctl_iv2dv_ihs_ctrl_reg.regValue = rtd_inl(PPOVERLAY_dctl_iv2dv_ihs_ctrl_reg);
    ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_gen_en = 1;
    ppoverlay_dctl_iv2dv_ihs_ctrl_reg.iv2dv_ih_slf_width = DTG_IV2DV_HSYNC_WIDTH;
    rtd_outl(PPOVERLAY_dctl_iv2dv_ihs_ctrl_reg, ppoverlay_dctl_iv2dv_ihs_ctrl_reg.regValue);

    //rtd_outl(0xb8028028, 0x00002222);
    ppoverlay_double_buffer_ctrl_reg.regValue = rtd_inl(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl_reg.orbit_dbuf_en = 0;
    ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en = 0;
    ppoverlay_double_buffer_ctrl_reg.dsubreg_dbuf_en = 0;
    ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_en = 0;
    ppoverlay_double_buffer_ctrl_reg.drm_multireg_dbuf_en = 0;
    rtd_outl(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_reg.regValue);

    //rtd_outl(0xb802e730, 0x00000003);
    dispd_ds444_dispd_ds444_crc_ctrl_reg.regValue = rtd_inl(DISPD_DS444_dispd_ds444_CRC_Ctrl_reg);
    dispd_ds444_dispd_ds444_crc_ctrl_reg.crc_conti = 1;
    dispd_ds444_dispd_ds444_crc_ctrl_reg.crc_start = 1;
    rtd_outl(DISPD_DS444_dispd_ds444_CRC_Ctrl_reg, dispd_ds444_dispd_ds444_crc_ctrl_reg.regValue);

    //rtd_outl(0xb8028000, 0x21000008);
    ppoverlay_display_timing_ctrl1_reg.disp_en = 1; // 1: Display timing generator runs normally
    rtd_outl(PPOVERLAY_Display_Timing_CTRL1_reg, ppoverlay_display_timing_ctrl1_reg.regValue);

#if 1 //#ifndef _MARK2_ZEBU_BRING_UP_RUN // [MARK2] CHECK-ME -- only for Zebu Verify (wait HDMI XTOR ready)
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
    NoteMessageHDMITx("[HDMITX] DTG %dx%d Setting Done\n",
            PPOVERLAY_MAIN_DEN_H_Start_End_get_mh_den_end(rtd_inl(PPOVERLAY_MAIN_DEN_H_Start_End_reg)),
            PPOVERLAY_MAIN_DEN_V_Start_End_get_mv_den_end(rtd_inl(PPOVERLAY_MAIN_DEN_V_Start_End_reg)));
#endif
//#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting
    return;
}


void HDMITX_verifier_init_mainDtg_ptg(unsigned int width, I3DDMA_COLOR_SPACE_T colorFmt)
{
    scaleup_d_uzu_globle_ctrl_RBUS scaleup_d_uzu_globle_ctrl_reg;
    unsigned char vgip_source_ready=0;

    InfoMessageHDMITx("[HDMI_TX] D-domain Ptg width=%d@%s...\n", width, __FUNCTION__);

    // TXSOC UZU PTG
    rtd_outl(SCALEUP_D_UZU_PATGEN_COLOR0_reg, 0x00ffffff);
    rtd_outl(SCALEUP_D_UZU_PATGEN_COLOR1_reg, 0x00ff0000);
    rtd_outl(SCALEUP_D_UZU_PATGEN_COLOR2_reg, 0x0000ff00);
    rtd_outl(SCALEUP_D_UZU_PATGEN_COLOR3_reg, 0x000000ff);

    rtd_outl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg, _BIT29|_BIT28|_BIT27|(DBG_EN_TOGGLE_PTG << 25)|(width > 1920? 0x1df: 0x27)/*((width/4)-1)*/); // PTG width B[21:0], PTG toggle B[26:25]: 0: no toggle, 1: 0/1 toggle, 2: 0/1/2 toggle, 3: 0/1/2/3 toggle
    rtd_outl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE1_reg, (width/8)-1);
    rtd_outl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE2_reg, (width/12)-1);

    // RGB2YUV enable control
    if(colorFmt == I3DDMA_COLOR_RGB){ // TX output format is RGB
        drvif_color_hdmitx_rgb2yuv_en(false);
    }else{ // YUV
        drvif_color_hdmitx_rgb2yuv_en(true);
    }

    // enable PTG
#if 1 // short pattern width for ZEBU XTOR windows view
    rtd_maskl(SCALEUP_D_UZU_Globle_Ctrl_reg, ~(_BIT6|_BIT5|_BIT4|_BIT0), _BIT5|_BIT4|_BIT0); // enable main PTG, // PTG B[6:4]: 1: randon, 2: build-in 4 color, 3: build-in 8 color, 4: self 4 color, 5: self 8 color
#else
    rtd_maskl(SCALEUP_D_UZU_Globle_Ctrl_reg, ~(_BIT6|_BIT5|_BIT4|_BIT0), _BIT5|_BIT0); // enable main PTG, // PTG B[6:4]: 1: randon, 2: build-in 4 color, 3: build-in 8 color, 4: self 4 color, 5: self 8 color
#endif

    vgip_source_ready = VGIP_VGIP_CHN1_CTRL_get_ch1_ivrun(rtd_inl(VGIP_VGIP_CHN1_CTRL_reg));

    //rtd_outl(0xb8029000, 0x00000021);
    scaleup_d_uzu_globle_ctrl_reg.regValue = rtd_inl(SCALEUP_D_UZU_Globle_Ctrl_reg);
    scaleup_d_uzu_globle_ctrl_reg.patgen_sel = (vgip_source_ready? 0: 1);//1; // 1: Pattern generator connect to MAIN
    rtd_outl(SCALEUP_D_UZU_Globle_Ctrl_reg, scaleup_d_uzu_globle_ctrl_reg.regValue);
    return;
}


unsigned int ScalerHdmiTx_get_bush(void)
{
	uint freq, tmp, value_f, value_n, pdiv, div,n_state;
	n_state = rtd_inl(PLL_REG_SYS_PLL_CPU_reg)&0x1; // = 0
	tmp = rtd_inl(PLL_REG_SYS_PLL_BUSH2_reg);
	value_f = ((tmp & PLL_REG_SYS_PLL_BUSH2_pllbush_fcode_mask) >> 16); // = 0x556
	value_n = ((tmp & PLL_REG_SYS_PLL_BUSH2_pllbush_ncode_mask) >> 8);  // = 0x3e
	tmp = rtd_inl(PLL_REG_SYS_PLL_BUSH1_reg);
	div = ((tmp & PLL_REG_SYS_PLL_BUSH1_pllbush_o_mask) >> 28); // = 0
	pdiv = ((tmp & PLL_REG_SYS_PLL_BUSH1_pllbush_prediv_mask)>>4);  // = 1
	if(n_state)
	{
		freq = ( 27 * (value_n + 3) + ((27 * value_f) >> 11)) /(pdiv + 1) / (1 << div);
	}
	else
	{
		freq = ( 27 * (value_n + 4) + ((27 * value_f) >> 11)) /(pdiv + 1) / (1 << div); // (27*(66) + (27*1366)/2048) / (1+1) /1 = (1782 + 18) / 2 = 900
	}
	return freq;
}



void ScalerHdmiTx_Set_DtgClkSource(int index, DCLK_SRC_TYPE type)
{
    unsigned int clk_tx, clk_bush;
    sys_reg_sys_dispclksel_RBUS sys_reg_sys_dispclksel_reg;

    if((index < 0)||(index >= TX_TIMING_NUM)){
        ErrorMessageHDMITx("[HDMITX] INDEX[%d/%d] OVER RANGE@%s\n", index, TX_TIMING_NUM, __FUNCTION__);
        return;
    }

    sys_reg_sys_dispclksel_reg.regValue = rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg);
    sys_reg_sys_dispclksel_reg.dclk_src_sel = type; // 0: from dpll, 1: from hdmitxpll

    // gdma_clk_sel = max(dispd/txpll-clock, sysh_clk)
    clk_tx = tx_cea_timing_table[index].pixelFreq / 10;
    clk_bush = ScalerHdmiTx_get_bush();
    sys_reg_sys_dispclksel_reg.dispd_gdma_clk_sel = (clk_tx > clk_bush? 1: 0); // 1: gdma_clk = dispd-clock div 1, 0: gdma_clk = sysh_clk
    rtd_outl(SYS_REG_SYS_DISPCLKSEL_reg, sys_reg_sys_dispclksel_reg.regValue);

    InfoMessageHDMITx("[HDMITX] TX[%d]/BusH clock: %d/%d Mhz, clk_sel=%d\n", index, clk_tx, clk_bush, sys_reg_sys_dispclksel_reg.dispd_gdma_clk_sel);

    return;
}


void ScalerHdmiTxSetDtgClkSource(unsigned int timing_type)
{
//#ifndef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
//#ifdef _MARK2_ZEBU_BRING_UP_NOT_RUN_FOR_DTG_FREE_RUN // [MARK2] FIX-ME -- let DTG free run for verify
    unsigned int width = tx_output_timing_table[timing_type].width;
    unsigned int frame_rate = tx_output_timing_table[timing_type].frame_rate;

    // switch Dclk source selection
    ScalerHdmiTx_Set_DtgClkSource(timing_type, DCLK_SRC_HDMITX_DPLL); // 0: from dpll, 1: from hdmitxpll
    udelay(150);

    InfoMessageHDMITx("[HDMITX] DTG width/FR=%d/%d\n", width, frame_rate);
    InfoMessageHDMITx("0230/0234/0208=%x/%x/%x\n", rtd_inl(0xb8000230), rtd_inl(0xb8000234), rtd_inl(0xb8000208));

//#endif // #ifdef _MARK2_ZEBU_BRING_UP_NOT_RUN_FOR_DTG_FREE_RUN // [MARK2] FIX-ME -- let DTG free run for verify
//#endif // #ifndef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
}


void hdmi_set_timingporch(unsigned int width, unsigned int height, unsigned int frame_rate, I3DDMA_COLOR_SPACE_T colorFmt, unsigned char timeIdx)
{
    hdmi_tx_timing_gen_st *timing_gen;
    timing_gen = &tx_cea_timing_table[(timeIdx < TX_TIMING_NUM? timeIdx: TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L)];
    VO_ACT_H_START = timing_gen->h.sync + timing_gen->h.front;
    VO_ACT_V_START = timing_gen->v.sync + timing_gen->v.front ;
    VO_H_BACKPORCH = timing_gen->h.back ;
    VO_V_BACKPORCH = timing_gen->v.back ;

    DTG_H_DEN_STA = timing_gen->h.back + timing_gen->h.sync;
    DTG_V_DEN_STA = timing_gen->v.back + timing_gen->v.sync;
    DTG_H_PORCH = (VO_ACT_H_START + VO_H_BACKPORCH);
    DTG_V_PORCH = (VO_ACT_V_START + VO_V_BACKPORCH);
    DTG_DH_WIDTH = timing_gen->h.sync;
    DTG_DV_LENGTH = timing_gen->v.sync;

    return;
}


void Scaler_TestCase_config(unsigned int width, unsigned int height, unsigned int frame_rate, I3DDMA_COLOR_DEPTH_T colorDepth, I3DDMA_COLOR_SPACE_T colorFmt, unsigned char timeIdx)
{
#define IH_START DTG_H_DEN_STA
#define IV_START DTG_V_DEN_STA
#define IHS_WID DTG_DH_WIDTH
#define IVS_WID DTG_DV_LENGTH
//#ifdef _MARK2_FIXME_H5X_SCALER_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
    unsigned short pixel_clock = tx_cea_timing_table[timeIdx].pixelFreq;
    unsigned int v_total;
    unsigned int h_freq;

    // scaler data format
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_COLOR_SPACE, colorFmt);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_COLOR_DEPTH, colorDepth);
    ScalerHdmiMacTx0EdidSetFeature(_HDMI_MAX_TMDS_CLK, 120); // TMDS clock = 600MHz

    NoteMessageHDMITx("[HDMITX] Input(%d) Video=%dx%dp%d, bit/color=%d/%d@clk=%d\n", HDMITX_Get_DscSrc(),
        width, height, frame_rate, Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_COLOR_DEPTH), Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_COLOR_SPACE), pixel_clock);

    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_PIXEL_CLOCK, pixel_clock);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_CLK_4PIXEL_MODE, 1); // VO will enable 4 pixel mode in default
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_HDMITX_DSC_SRC, HDMITX_Get_DscSrc()); // VO will enable 4 pixel mode in default
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_V_FREQ, frame_rate);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_COLOR_IMETRY, _COLORIMETRY_RGB_SRGB);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_EXT_COLORIMETRY, _COLORIMETRY_EXT_RGB_SRGB);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_RGB_QUANTIZATION_RANGE, _RGB_QUANTIZATION_FULL_RANGE);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_YCC_QUANTIZATION_RANGE, _YCC_QUANTIZATION_FULL_RANGE);
    Scaler_HDMITX_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE, 0);

    // scaler output timing
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_H_LEN, width + DTG_H_PORCH); // H-total
#if 1 // [MARK2] -- avoid overflow, kernel not support div operation of unsigned long long
    h_freq = ((width * height /100) * frame_rate /10) /(width + DTG_H_PORCH)/100;
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_H_FREQ, h_freq); // h_freq = plane->vsyncgen_pixel_freq / htotal / 100 (0.1KHz)
#endif
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_DISP_WID, width);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE, IH_START);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_HSYNC, IHS_WID);
    v_total = height + DTG_V_PORCH;
    // VIC 1, 2 is original 59Hz don't need change v-total
    if((timeIdx != TX_TIMING_HDMI20_640X480P59_RGB_8BIT) && (timeIdx != TX_TIMING_HDMI20_720X480P59_YUV444_8BIT)){
        if((frame_rate == 599)|| (frame_rate == 479)|| (frame_rate == 299)|| (frame_rate == 239))
            v_total = (v_total < 1000? v_total+1: v_total * 1001 / 1000);
    }
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_V_LEN, v_total); // v-total
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_DISP_LEN, height);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE, IV_START);
    Scaler_HDMITX_DispSetInputInfo(SLR_INPUT_VSYNC, IVS_WID);

    DebugMessageHDMITx("[HDMITX] %s Done!!\n", __FUNCTION__);
//#endif// #if 0 // [MARK2] Disable for Bring up

    return;
}

void HDMITX_Input_timing_config(unsigned int width, unsigned int height, unsigned short frame_rate,
                    I3DDMA_COLOR_DEPTH_T colorDepth, I3DDMA_COLOR_SPACE_T colorFmt, unsigned char timeIdx, EnumHdmi21FrlType frlType, EnumHdmitxTimingStdType stdType, unsigned char dscEn)
{
    // set DSC Enable type
    ScalerHdmiTxSetDSCEn(dscEn);

    // set HDMITX timing standard type
    ScalerHdmiTxSetTimingStandardType(stdType);

    // set target FRL type
    ScalerHdmiTxSetTargetFrlRate(frlType);

    // 2. config HDMITX input timing table
    hdmi_set_timingporch(width, height, frame_rate, colorFmt, timeIdx);

    // 3. HDMITX HW init: TX PHY power on, TX Mac reset, HPD enable, TX interrupt enable(on SOC)
    ScalerHdmiMacTx0PowerProc(_POWER_ACTION_AC_ON_TO_NORMAL);

    // 4. HDMITX input timing config
    Scaler_TestCase_config(width, height, frame_rate, colorDepth, colorFmt, timeIdx);

    // HDMITX Input path from Main DTG
    //if(HDMITX_Get_DscSrc() == DSC_SRC_DISPD){
#if 1 //#ifdef _MARK2_ZEBU_BRING_UP_NOT_RUN_FOR_DTG_FREE_RUN // [MARK2] FIX-ME -- let DTG free run for verify
        // 5-1. Main DTG setting
        dvScript_mainDtg_config(width, height, frame_rate);

        // 6-1. Main PTG
        HDMITX_verifier_init_mainDtg_ptg(width, colorFmt);
#else // #ifdef _MARK2_ZEBU_BRING_UP_NOT_RUN_FOR_DTG_FREE_RUN // [MARK2] FIX-ME -- let DTG free run for verify
//    } // HDMITX Input path from TXSOC
    else {
        // 5-2. TXSOC setting
        dvScript_txsoc_config(width, height, frame_rate);

        // 6-2. TXSOC PTG
        HDMITX_verifier_init_txsoc_ptg(colorFmt);
    }
#endif

    return;
}



TX_STATUS ScalerHdmiTx_Get_Timing_Status(void) {

    unsigned int *timingReady = NULL;

    timingReady = (unsigned int*)Scaler_GetShareMemVirAddr(HDMITX_TIMING_READY);

    return htonl(*timingReady);
}



void setHDMITX_Timing_Ready(TX_STATUS tx_status) {
//#ifdef _MARK2_FIXME_H5X_SCALER_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
    unsigned int *timingReady = NULL;

    timingReady = (unsigned int*)Scaler_GetShareMemVirAddr(HDMITX_TIMING_READY);

    *timingReady = htonl(tx_status);
//#endif
}

#define COLOR_DEPTH_10_BIT 10
bool isSettingType = false;
void setHdmitxTiming(unsigned int frame_rate, unsigned int color_depth, unsigned int color_type) {
    int timing_type = -1;
    //HOST_PLAFTORM_TYPE platformType = ScalerHdmiTxGetH5DPlatformType();

    DebugMessageHDMITx("[HDMITX] ===%s=== frame_rate: %d, color_depth: %d, color_type: %d\n", __FUNCTION__, frame_rate, color_depth, color_type);
//#define ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH
#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
    //if(platformType == HOST_PLATFORM_LG_O18){
        // [KTASKWBS-13388] fix FRL rate on 10G mode for O18 fast timing switch
        switch(frame_rate) {
            case 600:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G;
            break;
            case 599:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_10G;
            break;
            case 300:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_10G;
            break;
            case 299:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_10G;
            break;
            case 239:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_10G;
            break;
            case 240:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_10G;
            break;
            case 250:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_10G;
            break;
            case 479:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP47_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_10G;
            break;
            case 480:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP48_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_10G;
            break;
            case 500:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP50_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_10G;
            break;
            default:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G;
            break;
        }
//    }else{

#else
        switch(frame_rate) {
            case 600:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G;
            break;
            case 599:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_8G;
            break;
            case 300:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_6G4L : TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_6G4L;
            break;
            case 299:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_6G4L : TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_6G4L;
            break;
            case 239:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_6G4L : TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_6G4L;
            break;
            case 240:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_6G4L : TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_6G4L;
            break;
            case 250:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_6G4L : TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_6G4L;
            break;
            case 479:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP47_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_8G;
            break;
            case 480:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP48_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_8G;
            break;
            case 500:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP50_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_8G;
            break;
            default:
                timing_type = (color_depth == COLOR_DEPTH_10_BIT) ? TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G : TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G;
            break;
        }
//  }
#endif // #ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify

//#ifdef _MARK2_FIXME_H5X_SCALER_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
    if(HDMITX_TIMING_TYPE != -1 && timing_type != HDMITX_TIMING_TYPE) {
        setHDMITX_Timing_Ready(TX_TIMING_NEED_CHANGE);
        isSettingType = true;
    }
    else{
        setHDMITX_Timing_Ready(TX_TIMING_SETTING_DONE);
    }
//#endif

    HDMITX_TIMING_TYPE = timing_type;

#if 1 //#if(_HDMI21_STREAM_OFF_WITHOUT_LINKTRAINING == _ON) // [TEST][KTASKWBS-13388] support fast timing change flow
    if(isSettingType && ScalerHdmiTxGetStreamOffWithoutLinkTrainingFlag() && ScalerHdmiTxGetFrlNewModeEnable()){
        unsigned int bPixelFmtChange = (PIXEL_CLK|PIXEL_DEPTH|PIXEL_COLOR);
        UINT8 bSupportFastTimingSwitch = ((timing_type >= TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G) && (timing_type <= TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G)? 1: 0);
        if(last_frlType && bSupportFastTimingSwitch && (last_frlType == tx_output_timing_table[timing_type].frl_type)){
            ScalerHdmiTxSetFastTimingSwitchEn(_ENABLE);
            if(last_pixelClk == tx_cea_timing_table[timing_type].pixelFreq)
                bPixelFmtChange &= ~PIXEL_CLK;
            if(last_colorFmt == tx_output_timing_table[timing_type].color_type)
                bPixelFmtChange &= ~PIXEL_COLOR;
            if(last_colorDepth == tx_output_timing_table[timing_type].color_depth)
                bPixelFmtChange &= ~PIXEL_DEPTH;
        }else{
            ScalerHdmiTxSetFastTimingSwitchEn(_DISABLE);
        }
        ScalerHdmiTxSetPixelFormatChange(bPixelFmtChange);
    }
    last_frlType = tx_output_timing_table[timing_type].frl_type;
    last_pixelClk = tx_cea_timing_table[timing_type].pixelFreq;
    last_colorFmt = tx_output_timing_table[timing_type].color_type;
    last_colorDepth = tx_output_timing_table[timing_type].color_depth;
#endif

    DebugMessageHDMITx("[HDMITX] ===%s=== timing_type: %d\n", __FUNCTION__, timing_type);
}

void setHDMITX_Input_Timing_Config(int timing_type) {
    unsigned int width = 0, height = 0;
    unsigned int color_type = 0, frl_type = 0, std_type=0;
    unsigned int color_depth = 0, frame_rate = 0;
    hdmi_tx_output_timing_type *tx_output_timing;
    unsigned char dscEn;

    if((timing_type < 0)|| (timing_type >= TX_TIMING_NUM)){
        DebugMessageHDMITx("[HDMITX][ERROR] set input timing_type[%d] OVER RANGE@%s\n", timing_type, __FUNCTION__);
        return;
    }

#if 0 // TEST -- assign HDMITX bootup default output timing
if(HDMITX_Get_HdmiTxStreamReady() == 0)
    timing_type = TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G;
#endif
    NoteMessageHDMITx("[HDMITX] %s, set input timing, timing_type: %d\n", __FUNCTION__, timing_type);

    tx_output_timing = &tx_output_timing_table[timing_type];
    width = tx_output_timing->width;
    height = tx_output_timing->height;
    frame_rate = tx_output_timing->frame_rate;
    color_type = tx_output_timing->color_type;
    color_depth = tx_output_timing->color_depth;
    frl_type = tx_output_timing->frl_type;
    std_type = tx_output_timing->std_type;
    dscEn = (timing_type >= TX_TIMING_HDMI21_DSCE_TIMING_START? 1: 0);

    HDMITX_Input_timing_config(width, height, frame_rate, color_depth, color_type, timing_type, frl_type, std_type, dscEn);

}

void showTimingList(void) {
    int i = 0;

    shell_printf (1, "===================================================\r\n");
    shell_printf (1, "The current timing is : %d %s\r\n", HDMITX_TIMING_TYPE, ((HDMITX_TIMING_TYPE >= 0) && (HDMITX_TIMING_TYPE < TX_TIMING_NUM)? tx_output_timing_table[HDMITX_TIMING_TYPE].timingName: "EMPTY"));
    shell_printf (1, "===================================================\r\n");

    shell_printf (1, "Please select the timing for hdmi tx output:\r\n");

    for(i = 0; i < TX_TIMING_NUM; i++) {
        if(tx_output_timing_table[i].enable)
            shell_printf (1, "[%d] %s\r\n", i, tx_output_timing_table[i].timingName);
    }

    shell_printf (1, "===================================================\r\n");
    shell_printf (1, "The current timing is : %d %s\r\n", HDMITX_TIMING_TYPE, ((HDMITX_TIMING_TYPE >= 0) && (HDMITX_TIMING_TYPE < TX_TIMING_NUM)? tx_output_timing_table[HDMITX_TIMING_TYPE].timingName: "EMPTY"));
    shell_printf (1, "===================================================\r\n");

    shell_printf (1, "=> ");
}

extern void ScalerHdmiTx0_H5xLaneSrcSel_config(BYTE value);
extern void ScalerHdmiTx0_H5xLanePnSwap_config(BYTE value);
extern void ScalerHdmiTxSetH5DPlatformType(unsigned char type);
void setPtgStatus(unsigned char ptgEn, unsigned char ptgtoggle)
{
    DBG_EN_PTG = ptgEn;
    DBG_EN_TOGGLE_PTG = ptgtoggle;

#if 1 //#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    // main DTG forcebg disable
    rtd_maskl(PPOVERLAY_Main_Display_Control_RSV_reg, ~PPOVERLAY_Main_Display_Control_RSV_m_force_bg_mask, PPOVERLAY_Main_Display_Control_RSV_m_force_bg(0));
    rtd_maskl(PPOVERLAY_Double_Buffer_CTRL_reg, ~PPOVERLAY_Double_Buffer_CTRL_dmainreg_dbuf_set_mask, PPOVERLAY_Double_Buffer_CTRL_dmainreg_dbuf_set(1));
    // main UZU ptg enable
    rtd_maskl(SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_reg, ~SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_patgen_tog_sel_mask,
                                    SCALEUP_D_UZU_PATGEN_FRAME_TOGGLE_patgen_tog_sel(ptgtoggle)); // PTG toggle B[26:25]: 0: no toggle, 1: 0/1 toggle, 2: 0/1/2 toggle, 3: 0/1/2/3 toggle
#else
    // set PTG enable
    rtd_maskl(TXSOCTG_TXSOC_Display_Timing_CTRL1_reg, ~TXSOCTG_TXSOC_Display_Timing_CTRL1_force_bg_mask,
                                    TXSOCTG_TXSOC_Display_Timing_CTRL1_force_bg(!ptgEn && !ptgtoggle));

    IoReg_Mask32(TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_reg, ~TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_tx_soc_tg_db_rdy_mask, TXSOCTG_TX_SOC_TG_DOUBLE_BUFFER_CTRL_tx_soc_tg_db_rdy(1));
    // set PTG toggle enable
    rtd_maskl(TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE_reg, ~TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE_patgen_tog_sel_mask,
                                    TXSOC_SCALEUP_TXSOC_UZU_PATGEN_FRAME_TOGGLE_patgen_tog_sel(ptgtoggle)); // PTG toggle B[26:25]: 0: no toggle, 1: 0/1 toggle, 2: 0/1/2 toggle, 3: 0/1/2/3 toggle
#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting

    return;
}


void setHdmitxConfig(unsigned int param1, unsigned int param2)
{
    HdmiTxConfigPara1 *para1 = (HdmiTxConfigPara1 *)&param1;
    HdmiTxConfigPara2 *para2 = (HdmiTxConfigPara2 *)&param2;

    NoteMessageHDMITx("[HDMITX] Platform[%d], PortNum=%d, LaneSrc/Swap=%x/%x, VRR FR/Mode/Delay/24=%d/%d/%d/%d\n",
        para2->ucPlatformType, para1->uclimitPortNum, para2->ucLaneSrcCfg, para2->ucTxPnSwapCfg,
        para2->vrr_frame_rate, para2->vrr_mode, para2->vrr_loop_delay, para2->vrr_24hz);

    NoteMessageHDMITx("[HDMITX] CheckEdie/BypassLT=%d/%d, ptg En/Toggle=%d/%d, hdcp22 En=%d, hdcp2 RC Off=%d, DscMode=%d\n",
        para1->bCheckEdidInfo, para1->bBypassLinkTraining, para1->bEnablePtg, para1->bTogglePtg, para1->bEnableHdcp22, para1->bDisableHdcp22Rc, para1->ucDscMode);

    // config H5D platform type: 0: General, 1: LG_O18, 2: LG_O20
    ScalerHdmiTxSetH5DPlatformType(para2->ucPlatformType);

    // Config HDMITX output connect to HDMIRX port number constrain (0~4): 0: no limit, x: port[x]
    ScalerHdmiTxSetHdmiRxPortLimit(para1->uclimitPortNum);

    // check EDID info for TX output enable
    ScalerHdmiTxCheckEdidForTxOut(para1->bCheckEdidInfo);

    // force bypass HPD/EDID/LinkTraining control flow
    ScalerHdmiTxSetBypassLinkTrainingEn(para1->bBypassLinkTraining);

    // config lane src sel
    ScalerHdmiTx0_H5xLaneSrcSel_config(para2->ucLaneSrcCfg);

    // config lane PN swap
    ScalerHdmiTx0_H5xLanePnSwap_config(para2->ucTxPnSwapCfg);

    // set ptg
    setPtgStatus(para1->bEnablePtg, para1->bTogglePtg);

    //set hdcp22 enable
    ScalerHdmiTxSetHDCP22Enable(para1->bEnableHdcp22);

    //set DSC mode
    // DSC mode: 0: disable, 1: default, 2: 12G (bbp=8.125), 3: 12G (bbp=9.9375), 4: 12G (bbp=12.000), 5: 12G (bbp=15.000)
    if((para1->ucDscMode <= 5) && (para1->ucDscMode != ScalerHdmiTxGetDscMode())){
        unsigned char bDscEn = (para1->ucDscMode? _ENABLE: _DISABLE);
        ScalerHdmiTxSetDscMode(para1->ucDscMode);
        if(_FORCE_DSC_ENABLE != bDscEn){
            _FORCE_DSC_ENABLE = bDscEn;
            NoteMessageHDMITx("[HDMITX] _FORCE_DSC_ENABLE=%d\n", _FORCE_DSC_ENABLE);
        }
    }

    // VRR: set output frame rate
    if(para2->vrr_frame_rate)
        Scaler_FastTimingSwitch_VRR_4k2k_setting_by_framerate(para2->vrr_frame_rate * 10);

    // VRR loop mode & delay
    vrrLoopMode = para2->vrr_mode;
    vrrLoopDelay = para2->vrr_loop_delay;
    vrr24Hz = para2->vrr_24hz;

    return;
}


void setHdmitxCmd(char *cmd_buf)
{

    // HDMITX shell command
    if(strncmp(cmd_buf, "tx_run=", 7) == 0){
        unsigned int runMode;
        sscanf (&cmd_buf[7],"%d", &runMode);
        NoteMessageHDMITx("[HDMITX] tx_run=%d\n", runMode);
        ScalerHdmiTxSetTxRunFlag(runMode);
    }
    else if(strncmp(cmd_buf, "tx_bypass=", 10) == 0){
        unsigned int bypassMode;
        sscanf (&cmd_buf[10],"%d", &bypassMode);
        NoteMessageHDMITx("[HDMITX] tx_bypass=%d\n", bypassMode);
        ScalerHdmiTxSetBypassLinkTrainingEn(bypassMode);
    }
    else if(strncmp(cmd_buf, "tx_dump=", 8) == 0){
        unsigned int dumpMode;
        sscanf (&cmd_buf[8],"%d", &dumpMode);
        NoteMessageHDMITx("[HDMITX] tx_dump=%d\n", dumpMode);
        ScalerHdmiTxSetTxDump(dumpMode);
    }
    else if(strncmp(cmd_buf, "tx_reset", 8) == 0){
        NoteMessageHDMITx("[HDMITX] tx_reset ...\n");
        ScalerHdmiTx_reset_FRL();
        NoteMessageHDMITx("[HDMITX] tx_reset Done!!\n");
    }
    else if(strncmp(cmd_buf, "tx_pkt=", 7) == 0){
        unsigned int pktType;
        sscanf (&cmd_buf[7],"%d", &pktType);
        NoteMessageHDMITx("[HDMITX] tx_pkt=%d\n", pktType);
        ScalerHdmiTxSetTxPkt(pktType);
    }
    else if (strncmp(cmd_buf, "tx_timing=", 10) == 0)
    {
        unsigned int timeMode;
        //NoteMessageHDMITx("[Dolby] Time Mode[%s]\n", &cmd_buf[10]);
        sscanf (&cmd_buf[10],"%d", &timeMode);
        NoteMessageHDMITx("[HDMITX] tx_timing=%d\n", timeMode);
        setInputTimingType(timeMode, 0);
    }else if (strncmp(cmd_buf, "tx_timeList", 11) == 0)
    {
        NoteMessageHDMITx("[HDMITX] tx_timeList!\n");
        showTimingList();
    }else if (strncmp(cmd_buf, "tx_ptg", 6) == 0)
    {
        unsigned int picWidth = Scaler_HDMITX_DispGetInputInfo(SLR_INPUT_DISP_WID);
        NoteMessageHDMITx("[HDMITX] tx_ptg Width=[%d]!\n", picWidth);
        setHdmitxPatternGen(picWidth);
    }else if(strncmp(cmd_buf, "tx_mute=", 8) == 0){
        unsigned int muteEn;
        sscanf (&cmd_buf[8],"%d", &muteEn);
        NoteMessageHDMITx("[HDMITX] tx_mute=%d\n", muteEn);
        ScalerHdmiTxSetMute(muteEn);
    }else if(strncmp(cmd_buf, "pcr_track=", 10) == 0){
        unsigned int track_en;
        sscanf (&cmd_buf[10],"%d", &track_en);
        NoteMessageHDMITx("[HDMITX] pcr_tracking=%d\n", track_en);
        modestate_set_fll_tracking_pcr_vs_flag(track_en);
    }else if(strncmp(cmd_buf, "audio_ch=", 9) == 0){
        unsigned int audio_ch;
        sscanf (&cmd_buf[9],"%d", &audio_ch);
        NoteMessageHDMITx("[HDMITX] audio_ch=%d\n", audio_ch);
        setHdmitxAudioChanel(audio_ch);
    }else if(strncmp(cmd_buf, "audio_type=", 11) == 0){
        unsigned int audio_type;
        sscanf (&cmd_buf[11],"%d", &audio_type);
        NoteMessageHDMITx("[HDMITX] audio_type=%d\n", audio_type);
        setHdmitxAudioType(audio_type);
    }else if(strncmp(cmd_buf, "audio_mute=", 11) == 0){
        unsigned int audio_mute;
        sscanf (&cmd_buf[11],"%d", &audio_mute);
        NoteMessageHDMITx("[HDMITX] audio_mute=%d\n", audio_mute);
        setHdmitxAudioMute(audio_mute);
    }else if(strncmp(cmd_buf, "audio_freq=", 11) == 0){
        unsigned int audio_freq;
        sscanf (&cmd_buf[11],"%d", &audio_freq);
        NoteMessageHDMITx("[HDMITX] audio_freq=%d\n", audio_freq);
        setHdmitxAudioFreq(audio_freq);
    }else if(strncmp(cmd_buf, "tx_vtem=", 8) == 0){
        unsigned int vtem_en;
        sscanf (&cmd_buf[8],"%d", &vtem_en);
        NoteMessageHDMITx("[HDMITX] VTEM_en=%d\n", vtem_en);
        ScalerHdmiTxSetVTEMpacektEn(vtem_en);
    }else if(strncmp(cmd_buf, "tx_vrr=", 7) == 0){
        unsigned int vrr_en;
        sscanf (&cmd_buf[7],"%d", &vrr_en);
        NoteMessageHDMITx("[HDMITX] VRR_en=%d\n", vrr_en);
        ScalerHdmiTxSetVRRFlagEn(vrr_en);
    }

    return;
}


void setInputTimingType(int index, unsigned int param2) {
    //unsigned int waitCnt = 100;
    if((index < 0)||(index >= TX_TIMING_NUM)){
        ErrorMessageHDMITx("[HDMITX] INDEX[%d/%d] OVER RANGE\n", index, TX_TIMING_NUM);
        return;
    }

    NoteMessageHDMITx("[HDMITX] Input Timing %d->%d(%d)\n", HDMITX_TIMING_TYPE, index, tx_output_timing_table[index].enable);

    if(!tx_output_timing_table[index].enable) {
        shell_printf (1, "select the wrong timing, please select again\n");
        return ;
    }
    else {
        if(_FORCE_DSC_ENABLE == 1){
            switch(index){
              case TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G:
                index = TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_6G4L;
                break;
              case TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G:
                index = TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_6G4L;
                break;
              case TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_8G:
                index = TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_8BIT_6G4L;
                break;
              case TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G:
                index = TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_10BIT_6G4L;
                break;
              default:
                break;
            }
        }

        HDMITX_TIMING_TYPE = index;

#if 1 //#if(_HDMI21_STREAM_OFF_WITHOUT_LINKTRAINING == _ON) // [TEST][KTASKWBS-13388] support fast timing change flow
    if(ScalerHdmiTxGetStreamOffWithoutLinkTrainingFlag() && ScalerHdmiTxGetFrlNewModeEnable()){
        unsigned int bPixelFmtChange=(PIXEL_CLK|PIXEL_DEPTH|PIXEL_COLOR);
        UINT8 bSupportFastTimingSwitch = ((index >= TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G) && (index <= TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G)? 1: 0);
        if(last_frlType && bSupportFastTimingSwitch  && (last_frlType == tx_output_timing_table[index].frl_type)){
            ScalerHdmiTxSetFastTimingSwitchEn(_ENABLE);
            if(last_pixelClk == tx_cea_timing_table[index].pixelFreq)
                bPixelFmtChange &= ~PIXEL_CLK;
            if(last_colorFmt == tx_output_timing_table[index].color_type)
                bPixelFmtChange &= ~PIXEL_COLOR;
            if(last_colorDepth == tx_output_timing_table[index].color_depth)
                bPixelFmtChange &= ~PIXEL_DEPTH;
        }else{
            ScalerHdmiTxSetFastTimingSwitchEn(_DISABLE);
        }
        ScalerHdmiTxSetPixelFormatChange(bPixelFmtChange);
    }
    last_frlType = tx_output_timing_table[index].frl_type;
    last_pixelClk = tx_cea_timing_table[index].pixelFreq;
    last_colorFmt = tx_output_timing_table[index].color_type;
    last_colorDepth = tx_output_timing_table[index].color_depth;
#endif

    }

    return;
}




// [VRR] Just adjust v-total & vsync period to avoid RX measure error during VRR_EN mode
void Scaler_FastTimingSwitch_VRR_4k2k_setting_by_framerate(int frame_rate)
{
//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    #define TXSOC_PIXEL_CLK_4K2K 594000000
    #define TXSOC_DH_TOTAL_4K2K 4400
    ppoverlay_fix_last_line_pcr_ctrl_RBUS ppoverlay_fix_last_line_pcr_ctrl_reg;
    ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
    ppoverlay_uzudtg_dv_total_RBUS ppoverlay_uzudtg_dv_total_reg;
    // pixel clock = H-total * V-total * frame rate
    // V-total = 594000000 / H-total(4400) / frame rate
    unsigned int dv_total;
    unsigned int usVBlankHalf;
    unsigned int curDvTotal, curVsyncPeriod;
    int index = HDMITX_TIMING_TYPE;
    unsigned int vEnd = PPOVERLAY_DV_DEN_Start_End_get_dv_den_end(IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg));
    unsigned int curLineCnt;
    ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;
    static UINT32 last_applyStc=0;
    UINT32 stc, stcDiff, count;

    if((index < TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G) || (index > TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G)){
        ErrorMessageHDMITx("[HDMITX] INVALID Current Video Index[%d]\n", index);
        return;
    }

    if((vrr24Hz? (frame_rate < 239): ((frame_rate < 479)))|| (frame_rate > 600)){
        ErrorMessageHDMITx("[HDMITX] frame rate %d over range\n", frame_rate);
        return;
    }

    // wait front-porch
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);


    dv_total = (TXSOC_PIXEL_CLK_4K2K/ TXSOC_DH_TOTAL_4K2K) * 10 / frame_rate;
    usVBlankHalf = (dv_total - tx_output_timing_table[index].height) / 2;;

    ppoverlay_dv_total_reg.regValue = rtd_inl(PPOVERLAY_DV_total_reg);
    ppoverlay_uzudtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    ppoverlay_fix_last_line_pcr_ctrl_reg.regValue = rtd_inl(PPOVERLAY_fix_last_line_pcr_ctrl_reg);

    curDvTotal = ppoverlay_uzudtg_dv_total_reg.uzudtg_dv_total+1;
    curVsyncPeriod = ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period;

    // target dv total
    ppoverlay_dv_total_reg.dv_total = dv_total -1;
    rtd_outl(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);

    ppoverlay_uzudtg_dv_total_reg.uzudtg_dv_total = dv_total -1;
    rtd_outl(PPOVERLAY_uzudtg_DV_TOTAL_reg, ppoverlay_uzudtg_dv_total_reg.regValue);

    // vsync tracking setting
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period_update = 1;
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period = ((27000000*10)/frame_rate);
    rtd_outl(PPOVERLAY_fix_last_line_pcr_ctrl_reg, ppoverlay_fix_last_line_pcr_ctrl_reg.regValue);

    // V Blanking
    rtd_outl(HDMITX21_MAC_HDMI21_IN_CONV_8_reg, usVBlankHalf >> 8);
    rtd_outl(HDMITX21_MAC_HDMI21_IN_CONV_9_reg, usVBlankHalf);

    fw_scaler_dtg_double_buffer_apply();

    count = 0x3fffff;
    ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    while((ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set) && --count){
        ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    }
    if(count==0)
        ErrorMessageHDMITx("[HDMI_TX] double buffer timeout !!!@%s\n", __FUNCTION__);

    curLineCnt = PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg));
    stc = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
    stcDiff = stc - last_applyStc;
    if((curLineCnt < vEnd)|| ((vrrLoopDelay == 0) && (stcDiff > ((curVsyncPeriod/300)*3/2))))
        ErrorMessageHDMITx("[HDMITX] Index[%d] Setting Done@V-end/LC=%d/%d, StcDIff=%d/%d@DB=%d!!\n",
            index, vEnd, curLineCnt, stcDiff, curVsyncPeriod/300, ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en);

    last_applyStc = stc;
    InfoMessageHDMITx("[HDMITX][2] Index[%d] vsPeriod=%d->%d, DvTotal=%d->%d, VBlankHalf=%d->%d\n", index,
        curVsyncPeriod, ((27000000*10)/frame_rate), curDvTotal, dv_total,
        ((rtd_inl(HDMITX21_MAC_HDMI21_IN_CONV_8_reg) << 8)| rtd_inl(HDMITX21_MAC_HDMI21_IN_CONV_9_reg)), usVBlankHalf);

//#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting
    return;
}


#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
// loopMoe 0: jump mode, 1: sequence mode
#define LOOP_MODE_NUM 2
#define LOOP_INDEX_NUM 40
int vrrLoopTable[LOOP_MODE_NUM][LOOP_INDEX_NUM]=
    {{600,500,480,479,600,500,480,479,600,479,500,480,600,479,480,500,600,479,500,480,600,479,600,480,500,600,500,480,479,600,500,480,479,480,500,600,479,500,480,600},
      {600,590,580,570,560,550,540,530,520,510,500,490,480,490,500,510,520,530,540,550,560,570,580,590,600,590,580,570,560,550,540,530,520,510,500,490,480,490,500,600}};

int vrrLoopTable_24hz[LOOP_MODE_NUM][LOOP_INDEX_NUM]=
     {{600,500,480,300,250,240,600,500,480,300,250,240,250,300,480,500,600,240,250,300,480,500,600,500,480,300,250,240,250,300,480,500,600,240,500,300,600,250,480,240},
      {600,590,580,560,540,520,500,480,460,440,420,400,380,360,340,320,300,280,260,250,240,250,260,280,300,320,340,360,380,400,420,440,460,480,500,520,540,560,580,600}};

void Scaler_FastTimingSwitch_VRR_4k2k_loopMode_handler(void)
{
    static UINT8 last_loopMode=0;
    static UINT8 loopDelayCnt=0;
    static UINT8 loopIndex=0;

    // VRR loop mode OFF
    if(vrrLoopMode|| last_loopMode){
        if(vrrLoopMode && (vrrLoopMode <= LOOP_MODE_NUM)){ // VRR jump mode
            if(loopDelayCnt >= vrrLoopDelay){
               int frame_rate;
               if(vrr24Hz)
                frame_rate = vrrLoopTable_24hz[vrrLoopMode-1][loopIndex % LOOP_INDEX_NUM];
               else
                frame_rate = vrrLoopTable[vrrLoopMode-1][loopIndex % LOOP_INDEX_NUM];

               Scaler_FastTimingSwitch_VRR_4k2k_setting_by_framerate(frame_rate);
               InfoMessageHDMITx("[HDMITX] VRR LOOP Mode/24/FR[%d/%d/%d], Index/DelayCnt=%d/%d(/%d)\n", vrrLoopMode, vrr24Hz, frame_rate, loopIndex, loopDelayCnt, vrrLoopDelay);
               loopDelayCnt = 0;
               loopIndex++;
            }else{
               loopDelayCnt++;
            }
        }else{ // VRR loop mode disable
            InfoMessageHDMITx("[HDMITX] VRR LOOP OFF(%d), Index/DelayCnt=%d/%d(/%d)\n", vrrLoopMode, loopIndex, loopDelayCnt, vrrLoopDelay);
            loopIndex = 0;
            loopDelayCnt = 0;
        }
        last_loopMode = vrrLoopMode;
    }

    return;
}
#endif // #ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify


// [VRR] Just adjust v-total & vsync period to avoid RX measure error during VRR_EN mode
void Scaler_FastTimingSwitch_VRR_4k2k_setting(int index)
{
//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    #define TXSOC_PIXEL_CLK_4K2K 594000000
    #define TXSOC_DH_TOTAL_4K2K 4400
    ppoverlay_fix_last_line_pcr_ctrl_RBUS ppoverlay_fix_last_line_pcr_ctrl_reg;
    ppoverlay_dv_total_RBUS ppoverlay_dv_total_reg;
    ppoverlay_uzudtg_dv_total_RBUS ppoverlay_uzudtg_dv_total_reg;
    // pixel clock = H-total * V-total * frame rate
    // V-total = 594000000 / H-total(4400) / frame rate
    unsigned int frame_rate = tx_output_timing_table[index].frame_rate;
    unsigned int dv_total = (TXSOC_PIXEL_CLK_4K2K/ TXSOC_DH_TOTAL_4K2K) * 10 / frame_rate;
    unsigned int usVBlankHalf = (dv_total - tx_output_timing_table[index].height) / 2;;
    ppoverlay_dv_total_reg.regValue = rtd_inl(PPOVERLAY_DV_total_reg);
    ppoverlay_uzudtg_dv_total_reg.regValue = rtd_inl(PPOVERLAY_uzudtg_DV_TOTAL_reg);
    ppoverlay_fix_last_line_pcr_ctrl_reg.regValue = rtd_inl(PPOVERLAY_fix_last_line_pcr_ctrl_reg);

    InfoMessageHDMITx("[HDMITX] Index[%d] vsPeriod=%d->%d, DvTotal=%d->%d, VBlankHalf=%d->%d\n", index,
        ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period, ((27000000*10)/frame_rate), ppoverlay_uzudtg_dv_total_reg.uzudtg_dv_total+1, dv_total,
        ((rtd_inl(HDMITX21_MAC_HDMI21_IN_CONV_8_reg) << 8)| rtd_inl(HDMITX21_MAC_HDMI21_IN_CONV_9_reg)), usVBlankHalf);

    // target dv total
    ppoverlay_dv_total_reg.dv_total = dv_total -1;
    rtd_outl(PPOVERLAY_DV_total_reg, ppoverlay_dv_total_reg.regValue);

    ppoverlay_uzudtg_dv_total_reg.uzudtg_dv_total = dv_total -1;
    rtd_outl(PPOVERLAY_uzudtg_DV_TOTAL_reg, ppoverlay_uzudtg_dv_total_reg.regValue);

    // vsync tracking setting
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period_update = 1;
    ppoverlay_fix_last_line_pcr_ctrl_reg.pcr_vs_free_period = ((27000000*10)/frame_rate);
    rtd_outl(PPOVERLAY_fix_last_line_pcr_ctrl_reg, ppoverlay_fix_last_line_pcr_ctrl_reg.regValue);

    // V Blanking
    rtd_outl(HDMITX21_MAC_HDMI21_IN_CONV_8_reg, usVBlankHalf >> 8);
    rtd_outl(HDMITX21_MAC_HDMI21_IN_CONV_9_reg, usVBlankHalf);
//#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting

    return;
}


void Scaler_FastTimingSwitch_config(int index)
{
//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    unsigned int vEnd = PPOVERLAY_DV_DEN_Start_End_get_dv_den_end(IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg));
    unsigned int curLineCnt, count;
    ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_reg;

    // wait front-porch
    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);

    switch(index){
// ===========================================================
// ========= VRR frame rate change timing: 4k2kp47/48/50/60 ============
// ===========================================================
#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
        /* HDMI2.1 4K2Kp47 444 */
        case TX_TIMING_HDMI21_4K2KP47_YUV444_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP47_YUV444_10BIT_10G:

        /* HDMI2.1 4K2Kp48 444 */
        case TX_TIMING_HDMI21_4K2KP48_YUV444_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP48_YUV444_10BIT_10G:

        /* HDMI2.1 4K2Kp50 444 */
        case TX_TIMING_HDMI21_4K2KP50_YUV444_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP50_YUV444_10BIT_10G:

        /* HDMI2.1 4K2Kp60 444 */
        case TX_TIMING_HDMI21_4K2KP60_RGB_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_RGB_10BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV444_10BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV444_12BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_12G:

        /* HDMI2.1 4K2Kp47 420 */
        case TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_10G:

        /* HDMI2.1 4K2Kp48 420 */
        case TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_10G:

        /* HDMI2.1 4K2Kp50 420 */
        case TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_10G:

        /* HDMI2.1 4K2Kp60 420 */
        case TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_6G4L:
        case TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G:
        case TX_TIMING_HDMI21_4K2KP60_YUV420_12BIT_6G4L:
            // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);
            break;

#else // #ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
        case TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_6G4L:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008cb); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x000899da); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002e); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_6G4L:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008cb); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00089544); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_6G4L:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x149f149f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00083d60); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000005); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x0000005a); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_6G4L:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x112f112f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x0006ddd0); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000001); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x000000ea); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;

        case TX_TIMING_HDMI21_4K2KP47_YUV444_8BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x000899da); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002e); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP48_YUV444_8BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00089544); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP50_YUV444_8BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x149f149f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00083d60); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000005); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x0000005a); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x112f112f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x0006ddd0); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            rtd_outl(0xb8021f00, 0x00000000); // <== TXSOC_DS444_txsoc_ds444_ctrl0_reg B[31:30]: ds444_mode_sel, B[28:16]: vs_lncnt, B[13:0]: hs_pxcnt

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000001); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x000000ea); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;

        case TX_TIMING_HDMI21_4K2KP47_YUV444_10BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x000899da); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002e); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP48_YUV444_10BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x157b157b); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008cb); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00089544); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000006); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x00000036); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP50_YUV444_10BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x149f149f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x00083d60); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000005); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x0000005a); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
        case TX_TIMING_HDMI21_4K2KP60_YUV444_10BIT_10G:
    #if 1 // [VRR] Just adjust v-total size to avoid RX measure error in VRR_EN mode
            Scaler_FastTimingSwitch_VRR_4k2k_setting(index);

    #else // [VRR] HDMI standard timing table
            // TXSOC setting
            rtd_outl(0xb8021808, 0x112f112f); // <== TXSOCTG_TXSOC_DH_Total_Last_Line_Length_reg B[29:16]: dh_total, B[13:0]: dh_total_last_line
            rtd_outl(0xb8021810, 0x000008c9); // <== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total
            rtd_outl(0xb80218c4, 0x0006ddd0); // <== TXSOCTG_TXSOC_PCR_cnt_reg B[31]: pcr_count_en, B[30:29]: pcr_count_vs_clk_sel, B[28]: count_vs_period_update, B[27:0]: count_vs_period

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9434, 0x0000002d); // <== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0

            rtd_outl(0xb80b9594, 0x00000001); // <== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8
            rtd_outl(0xb80b9598, 0x000000ea); // <== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
    #endif
            break;
#endif // #else // #ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
// ===========================================================================
// === Bridge H/V active/total size change timing: 4k2kp120/4k4kp60/4k8kp30/4192x4320p60 ======
// ===========================================================================
        case TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G: // [48] HDMI2.1 4K2K 120p YUV444 8bit@8G, v-start=82, v-end=2242, v-total=2249
            // TXSOC setting
            rtd_outl(0xb8021810,0x000008c9); //<== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total(2249)
            rtd_outl(0xb8021814,0x01801080); //<== TXSOCTG_TXSOC_DH_DEN_Start_End_reg B[29:16]: dh_den_sta(384), B[13:0]: dh_den_end(4224)
            rtd_outl(0xb8021818,0x005208c2); //<== TXSOCTG_TXOSC_DV_DEN_Start_End_reg B[29:16]: dv_den_sta(82), B[13:0]: dv_den_end(2242)
            rtd_outl(0xb8021824,0x01801080); //<== TXSOCTG_TXSOC_Active_H_Start_End_reg B[29:16]: dh_act_sta(384), B[13:0]: dh_act_end(4224)
            rtd_outl(0xb8021828,0x005208c2); //<== TXSOCTG_TXSOC_Active_V_Start_End_reg B[29:16]: dv_act_sta(82), B[13:0]: dv_act_end(2242)
            rtd_outl(0xb80218c4,0x00036ee8); //<== TXSOCTG_TXSOC_PCR_cnt_reg B[27:0]: count_vs_period(120hz)

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9420,0x00000008); //<== HDMITX21_MAC_HDMI21_IN_CONV_4_reg B[5:0]: v_height_13_8 (2160)
            rtd_outl(0xb80b9424,0x00000070); //<== HDMITX21_MAC_HDMI21_IN_CONV_5_reg B[7:0]: v_height_7_0
            rtd_outl(0xb80b9434,0x0000002d); //<== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0 (45)

            rtd_outl(0xb80b958c,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_3_reg B[2:0]: video_pre_keepout_start_1st_10_8 (304)
            rtd_outl(0xb80b9590,0x0000003a); //<== HDMITX21_MAC_HDMI21_SCHEDULER_4_reg B[7:0]: video_pre_keepout_start_1st_7_0
            rtd_outl(0xb80b9594,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8 (490)
            rtd_outl(0xb80b9598,0x000000ea); //<== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
        break;
        case TX_TIMING_HDMI21_3840x4320P60_YUV444_8BIT_8G: // [49] HDMI2.1 3840x4320 60p YUV444 8bit@8G, v-start=82, v-end=4402, v-total=4499
            // TXSOC setting
            rtd_outl(0xb8021810,0x00001193); //<x> TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total(4499)
            rtd_outl(0xb8021814,0x01801080); //<== TXSOCTG_TXSOC_DH_DEN_Start_End_reg B[29:16]: dh_den_sta(384), B[13:0]: dh_den_end(4224)
            rtd_outl(0xb8021818,0x00521132); //<x> TXSOCTG_TXOSC_DV_DEN_Start_End_reg B[29:16]: dv_den_sta(82), B[13:0]: dv_den_end(4402)
            rtd_outl(0xb8021824,0x01801080); //<== TXSOCTG_TXSOC_Active_H_Start_End_reg B[29:16]: dh_act_sta(384), B[13:0]: dh_act_end(4224)
            rtd_outl(0xb8021828,0x00521132); //<x> TXSOCTG_TXSOC_Active_V_Start_End_reg B[29:16]: dv_act_sta(82), B[13:0]: dv_act_end(4402)
            rtd_outl(0xb80218c4,0x0006ddd0); //<x> TXSOCTG_TXSOC_PCR_cnt_reg B[27:0]: count_vs_period (60hz)

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9420,0x00000010); //<x> HDMITX21_MAC_HDMI21_IN_CONV_4_reg B[5:0]: v_height_13_8 (4320)
            rtd_outl(0xb80b9424,0x000000e0); //<x> HDMITX21_MAC_HDMI21_IN_CONV_5_reg B[7:0]: v_height_7_0
            rtd_outl(0xb80b9434,0x0000005a); //<x> HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0 (90)

            rtd_outl(0xb80b958c,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_3_reg B[2:0]: video_pre_keepout_start_1st_10_8 (314)
            rtd_outl(0xb80b9590,0x0000003a); //<== HDMITX21_MAC_HDMI21_SCHEDULER_4_reg B[7:0]: video_pre_keepout_start_1st_7_0
            rtd_outl(0xb80b9594,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8 (490)
            rtd_outl(0xb80b9598,0x000000ea); //<== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0

        break;
        case TX_TIMING_HDMI21_3840x8640P30_YUV444_8BIT_8G:  // [50] HDMI2.1 3840x8640 30p YUV444 8bit@8G, v-start=82, v-end=8722, v-total=8999
            // TXSOC setting
            rtd_outl(0xb8021810,0x00002327); //<X> TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total (8999)
            rtd_outl(0xb8021814,0x01801080); //<== TXSOCTG_TXSOC_DH_DEN_Start_End_reg B[29:16]: dh_den_sta(384), B[13:0]: dh_den_end (4224)
            rtd_outl(0xb8021818,0x00522212); //<X> TXSOCTG_TXOSC_DV_DEN_Start_End_reg B[29:16]: dv_den_sta(82), B[13:0]: dv_den_end(8722)
            rtd_outl(0xb8021824,0x01801080); //<== TXSOCTG_TXSOC_Active_H_Start_End_reg B[29:16]: dh_act_sta(384), B[13:0]: dh_act_end(4224)
            rtd_outl(0xb8021828,0x00520212); //<*> TXSOCTG_TXSOC_Active_V_Start_End_reg B[29:16]: dv_act_sta(82), B[13:0]: dv_act_end(** 8722 overflow ->530 **)
            rtd_outl(0xb80218c4,0x000dbba0); //<X> TXSOCTG_TXSOC_PCR_cnt_reg  B[27:0]: count_vs_period (30Hz)

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9420,0x00000021); //<X> HDMITX21_MAC_HDMI21_IN_CONV_4_reg B[5:0]: v_height_13_8 (8640)
            rtd_outl(0xb80b9424,0x000000c0); //<X> HDMITX21_MAC_HDMI21_IN_CONV_5_reg B[7:0]: v_height_7_0
            rtd_outl(0xb80b9434,0x000000b4); //<X> HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0 (180)

            rtd_outl(0xb80b958c,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_3_reg B[2:0]: video_pre_keepout_start_1st_10_8 (314)
            rtd_outl(0xb80b9590,0x0000003a); //<== HDMITX21_MAC_HDMI21_SCHEDULER_4_reg B[7:0]: video_pre_keepout_start_1st_7_0
            rtd_outl(0xb80b9594,0x00000001); //<== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8 (490)
            rtd_outl(0xb80b9598,0x000000ea); //<== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0

        break;
        case TX_TIMING_HDMI21_4192x4320P60_YUV444_8BIT_8G:  // [51] HDMI2.1 4192x4320 60p YUV444 8bit@8G, v-start=82, v-end=4402, v-total=4499
            // TXSOC setting
            rtd_outl(0xb8021810, 0x00001193); //<== TXSOCTG_TXSOC_DV_total_reg B[13:0]: total_dv_total (4499)
            rtd_outl(0xb8021814, 0x00a81108); //<== TXSOCTG_TXSOC_DH_DEN_Start_End_reg B[29:16]: dh_den_sta(168), B[13:0]: dh_den_end(4360)
            rtd_outl(0xb8021818, 0x00521132); //<== TXSOCTG_TXOSC_DV_DEN_Start_End_reg B[29:16]: dv_den_sta(82), B[13:0]: dv_den_end(4402)
            rtd_outl(0xb8021824, 0x00a81108); //<== TXSOCTG_TXSOC_Active_H_Start_End_reg B[29:16]: dh_act_sta(168), B[13:0]: dh_act_end(4360)
            rtd_outl(0xb8021828, 0x00521132); //<== TXSOCTG_TXSOC_Active_V_Start_End_reg B[29:16]: dv_act_sta(82), B[13:0]: dv_act_end(4402)
            rtd_outl(0xb80218c4, 0x0006ddd0); //<== TXSOCTG_TXSOC_PCR_cnt_reg  B[27:0]: count_vs_period (60Hz)

            // HDMI21 TX MAC setting
            rtd_outl(0xb80b9420, 0x00000010); //<== HDMITX21_MAC_HDMI21_IN_CONV_4_reg B[5:0]: v_height_13_8 (4320)
            rtd_outl(0xb80b9424, 0x000000e0); //<== HDMITX21_MAC_HDMI21_IN_CONV_5_reg B[7:0]: v_height_7_0
            rtd_outl(0xb80b9434, 0x0000005a); //<== HDMITX21_MAC_HDMI21_IN_CONV_9_reg B[7:0]: v_blank_half_7_0 (90)

            rtd_outl(0xb80b958c, 0x00000000); //<== HDMITX21_MAC_HDMI21_SCHEDULER_3_reg B[2:0]: video_pre_keepout_start_1st_10_8 (98)
            rtd_outl(0xb80b9590, 0x00000062); //<== HDMITX21_MAC_HDMI21_SCHEDULER_4_reg B[7:0]: video_pre_keepout_start_1st_7_0
            rtd_outl(0xb80b9594, 0x00000000); //<== HDMITX21_MAC_HDMI21_SCHEDULER_5_reg B[4:0]: video_pre_keepout_start_12_8 (138)
            rtd_outl(0xb80b9598, 0x0000008a); //<== HDMITX21_MAC_HDMI21_SCHEDULER_6_reg B[7:0]: video_pre_keepout_start_7_0
        break;
        default:
            ErrorMessageHDMITx("[HDMITX] INVALID INDEX[%d]\n", index);
            break;
    }

    fw_scaler_dtg_double_buffer_apply();

    count = 0x3fffff;
    ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    while((ppoverlay_double_buffer_ctrl_reg.dreg_dbuf_set) && --count){
        ppoverlay_double_buffer_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    }
    if(count==0)
        ErrorMessageHDMITx("[HDMI_TX] double buffer timeout !!!@%s\n", __FUNCTION__);

    curLineCnt = PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg));
    NoteMessageHDMITx("[HDMITX] Index[%d] Setting Done@V-end/LC=%d/%d@DB=%d!!\n", index, vEnd, curLineCnt, ppoverlay_double_buffer_ctrl_reg.dmainreg_dbuf_en);
//#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting
    return;
}


extern void HDMITX_Set_FfeTable(HDMI_TX_FFE_TABLE_LIST tableId);
extern int ScalerHdmiTxPhy_CheckFfeSwingTableCodeCorrect(void);
//#ifdef CONFIG_ENABLE_TXSOC_PATH
extern void rpcVoIvSrcSel(int source);
//#endif
extern long long GetSTCClockWithOffset(void);

static int HDMITX_state_handler(void *data)
{
    #define _LINK_TRAINING_TIMEOUT 50 // 10*50=500ms
    unsigned char curStreamReady=0, last_txStreamReady=0;
    int timing_type=-1;
    unsigned int waitCnt, timeCfgStc1=0, timeCfgStc2=0; // HDMITX start send colorbar after system boot up(wait for a while) to avoid HDMI link training fail due to system is too busy
    unsigned int waitTxReadyCnt=0;
#ifdef ENABLE_ISR_TASK_RUN_TIME_PROFILING // Profiling ISR/Task Run Time and Run Period
    extern unsigned int IoReg_Read32 (unsigned int addr);
    extern unsigned int isrRunTime_stc[ISR_IDX_NUM];
    extern unsigned int isrRunTime_cnt[ISR_IDX_NUM];
    unsigned int stc1;
#endif
#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
    unsigned char bForceSlrReRun=0;
#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
    DS_HDMITX_CRC_TYPE pre_hdmitx_crc[HDMITX_CRC_LIST_NUM]={{{0,0,0}},{{0,0,0}},{{0,0,0}},{{0,0,0}},{{0,0,0}}};
    DS_HDMITX_CRC_TYPE hdmitx_crc[HDMITX_CRC_LIST_NUM];
    unsigned char bNewFrame=1;
    unsigned int lineCnt=0, last_lineCnt=0x1fff;
    unsigned int loopCnt[2]={0,0};
    unsigned char bTxRun[2]={0,0};
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP

#if 0 // delay HDMITX start output (sec)
    waitCnt = 1;
    while(waitCnt--){
        DebugMessageHDMITx("[HDMITX] Wait Cnt=%d...\n", waitCnt);
        ROSTimeDly(1000);
    }
#endif

    ErrorMessageHDMITx("[HDMITX] Handler start...@%s\n", __FUNCTION__);

    // Get HDMITX I2C BUSID
    pcb_mgr_get_enum_info_byname("HDMI_TX_I2C", &HDMI_TX_I2C_BUSID);
    HDMI_TX_I2C_BUSID = 5;
    ErrorMessageHDMITx("[HDMITX] HDMI_TX_I2C_BUSID=%llu...@%s\n",HDMI_TX_I2C_BUSID, __FUNCTION__);

    // set chip version
    //ScalerHdmiTxSetChipVersion(get_ic_version());

    // Set HDMITX input path
    //HDMITX_Set_DscSrc(DSC_SRC_TXSOC);
    HDMITX_Set_DscSrc(DSC_SRC_DISPD);

    // DTG FLL tracking PCR VS
    modestate_set_fll_tracking_pcr_vs_flag(_DISABLE);

    // check FFE/SWING table size
    if(ScalerHdmiTxPhy_CheckFfeSwingTableCodeCorrect() != 1){
        ErrorMessageHDMITx("\n\n[HDMITX] *** CHECK FFE/SWING TABLE FAIL!!@%s ***\n\n\n", __FUNCTION__);
        return 0;
    }

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
    // force bypass HPD/EDID/LinkTraining control flow
    ScalerHdmiTxSetBypassLinkTrainingEn(0);
#endif

    // board FFE table select
    HDMITX_Set_FfeTable(HDMI21_TX_FFE_TABLE_LIST_TV001);
    //HDMITX_Set_FfeTable(HDMI21_TX_FFE_TABLE_LIST_TV006);

//#ifdef _MARK2_FIXME_H5X_TXSOC_FLOW // [MARK2] FIX-ME -- replace TXSOC setting into main DTG setting flow
    // TXSOC YUV2RGB table Init
    Scaler_color_set_HDMITX_RGB2YUV();
//#endif // #if 0 // [MARK2] FIX-ME -- replace TXSOC setting

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
    // UZU CRC init
    rtd_outl(SCALEUP_D_UZU_CRC_CTRL_reg, SCALEUP_D_UZU_CRC_CTRL_start(1)|SCALEUP_D_UZU_CRC_CTRL_conti(1));
    // DSCE CRC init
    rtd_maskl(DSCE_DSC_ENCODER_CRC_reg, ~DSCE_DSC_ENCODER_CRC_crc_start_mask, DSCE_DSC_ENCODER_CRC_crc_start(1));
    // MISC CRC init
    rtd_outl(HDMITX_MISC_CRC_CTRL_reg, HDMITX_MISC_CRC_CTRL_crc_res_sel(0)|HDMITX_MISC_CRC_CTRL_crc_conti(1)|HDMITX_MISC_CRC_CTRL_crc_start(1));
    // HDMI20 CRC init (default in TMDS channel[0])
    rtd_outl(HDMITX20_MAC0_HDMI_CRC_0_reg, HDMITX20_MAC0_HDMI_CRC_0_crc_chsel(0));
    // HDMI21 CRC init (default in VESA mode)
    rtd_outl(HDMITX21_MAC_HDMI21_CRC_0_reg, HDMITX21_MAC_HDMI21_CRC_0_crc_en(1)|HDMITX21_MAC_HDMI21_CRC_0_crc_src_sel(0));
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify

    // wait VSC init done
    waitCnt = 0;
//#ifdef _MARK2_FIXME_H5X_SCALER_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
    while((getVscInitdoneFlag() == 0) || (ScalerHdmiTxGetTxRunFlag() == 0)
#ifdef _MARK2_ZEBU_BRING_UP_LOCAL_RUN // [MARK2] FIX-ME -- only for local verify
        && (kernel_finished() == 0)
#endif
        )
    {
        ROSTimeDly(200);
        if((waitCnt++ % 10) == 0){
            ErrorMessageHDMITx("[HDMITX] Wait VSC Init[%d]@vsc/run=[%d|%d]...\n", waitCnt, getVscInitdoneFlag() , ScalerHdmiTxGetTxRunFlag());
        }
    }
//#endif

    ErrorMessageHDMITx("[HDMITX] Wait VSC Init Done[%d]@%d\n", waitCnt, IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));

    // HDMITX default panel parameter init
    ScalerHdmiTxSetPanelParameter_init();

    setHDMITX_Timing_Ready(TX_TIMING_INIT_STATE);

#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
    ScalerHdmiTxSetFrlNewModeEnable(ScalerHdmiTxGetChipVersion() >= VERSION_C);
#endif


    // state check & process
    while(1){

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
        lineCnt = PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg));
        bTxRun[0]= HDMITX20_ON_REGION_TXFIFO_CTRL0_get_tx_en(rtd_inl(HDMITX20_ON_REGION_TXFIFO_CTRL0_reg));
        bTxRun[1]= !HDMITX21_MAC_HDMI21_FRL_19_get_gap_en(rtd_inl(HDMITX21_MAC_HDMI21_FRL_19_reg));
  #if 1 // [REMOVE-ME] FOR DEBUG only
        if(((bTxRun[0] == 0) && (bTxRun[1] == 0)) || (lineCnt < last_lineCnt)){
            NoteMessageHDMITx("[CRC] Time=%d/%d@%d, FRL/Rdy/20Run/21Run=%d/%d/%d/%d\n", timing_type, HDMITX_TIMING_TYPE, lineCnt,
                            ScalerHdmiTxGetTargetFrlRate(), HDMITX_Get_HdmiTxStreamReady(), bTxRun[0], bTxRun[1]);
        }else if(lineCnt - last_lineCnt > 500){
            NoteMessageHDMITx("[DBG] Time=%d/%d@%d\n", timing_type, HDMITX_TIMING_TYPE, lineCnt);
        }
  #endif // #if 1 // [REMOVE-ME] FOR DEBUG only
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify

        // check new TX output timing
        if((timing_type == -1)|| ((HDMITX_TIMING_TYPE != -1) && (timing_type != HDMITX_TIMING_TYPE))){
            if((timing_type == -1)|| curStreamReady || (++waitTxReadyCnt > _LINK_TRAINING_TIMEOUT)){
// HDMI20 2k1k60
            #if (0)  // HDMI20 2k1kp60 YUV444 8bit
                tx_timing_type defaultTiming = TX_TIMING_HDMI20_1080P60_YUV444_8BIT;
// HDMI20 4k2k60
            #elif (1) // HDMI20 4k2kp60 YUV444 8bit
                tx_timing_type defaultTiming = TX_TIMING_HDMI20_4K2KP60_YUV444_8BIT;//;TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_12G
            #elif (0) // HDMI20 4k2kp60 RGB 8bit
                tx_timing_type defaultTiming = TX_TIMING_HDMI20_4K2KP60_RGB_8BIT;
// HDMI21 2k1k60
            #elif (0) // HDMI21 1080p60 YUV444 8bit@6G4L
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_1080P60_YUV444_8BIT_6G4L;
            #elif (0) // HDMI21 1080p60 RGB 8bit@6G4L
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_1080P60_RGB_8BIT_6G4L;
// HDMI21 4k2k60
            #elif (0) // HDMI21 4k2kp60 RGB 8bit@10G
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_4K2KP60_RGB_8BIT_10G;
            #elif (0) // HDMI21 4k2kp60 YUV444 8bit@10G
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G;
                //tx_timing_type defaultTiming = TX_TIMING_HDMI21_4K2KP60_YUV444_12BIT_10G;
// HDMI21 4k2k120
            #elif (0) // HDMI21 4k2kp120 YUV444 8bit@8G
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G;
// STB6500 4k2k60
            #elif (0) // HDMI20 STB6500 4k2kp60 YUV444 8bit
                tx_timing_type defaultTiming = TX_TIMING_HDMI20_STB6500_4K2KP60_YUV444_8BIT;
// HDMI21 8k4kp24/30
            #elif (0) // HDMI2.1 8K4K 24p YUV420 8bit@6G4L
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_6G4L;
            #elif (0) // HDMI2.1 8K4K 30p YUV420 8bit@6G4L
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_6G4L;
// HDMI21 8k4k60
            #elif (0) // HDMI2.1 8K4K 60p YUV420 8bit@10G
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G;
            #elif (0) // HDMI2.1 8K4K 59p YUV420 8bit@10G
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_10G;
// default HMDI21 4k2kp60 444
            #else // HDMI21 4k2kp60 YUV444 8bit@6G4L
                tx_timing_type defaultTiming = TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L;
            #endif
                NoteMessageHDMITx("[HDMITX] timing change %d->%d, 90K: %X\n", timing_type, HDMITX_TIMING_TYPE, IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg));
                timing_type = ((HDMITX_TIMING_TYPE >= 0) && (HDMITX_TIMING_TYPE < TX_TIMING_NUM)? HDMITX_TIMING_TYPE: defaultTiming);
                // [Bridge][TEST] fast change TXSOC/TX HW setting
                if(ScalerHdmiTxGetFastTimingSwitchEn() && (ScalerHdmiTxGetPixelFormatChange() == 0) && (last_frameRate == 600)){
                    NoteMessageHDMITx("[HDMITX] Fast Timing Change...\n");
                    Scaler_FastTimingSwitch_config(timing_type);
                    NoteMessageHDMITx("[HDMITX] Timing Change Done!!\n");
                }else{
                    HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
#ifndef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] FIX-ME -- remove me after VODMA clock is enabled
                    IoReg_Mask32(VODMA_VODMA_CLKGEN_reg, ~(_BIT0 | _BIT3), 0); //disable gating
#endif
                    // reset D-clock source to DPLL beforce TX timing change
                    ScalerHdmiTx_Set_DtgClkSource(timing_type, DCLK_SRC_DPLL);
                    udelay(150);

//#ifdef CONFIG_ENABLE_TXSOC_PATH
                    ScalerHdmiTxPhy_SetFllTracking(_DISABLE);
                    rpcVoIvSrcSel(VSGEN_IV_SRC_SEL_PIXELCLOCK);
//#endif
//#ifdef _MARK2_FIXME_H5X_SCALER_FLOW // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
                    setHDMITX_Timing_Ready(TX_TIMING_FREERUN_CHANGE_DONE);
    #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
                    if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
                        ErrorMessageHDMITx("[HDMITX] Force Scaler Re-Run[1]\n");
                        bForceSlrReRun = 1;
                    }
    #endif
//#endif

                    timeCfgStc1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
                    NoteMessageHDMITx("[HDMITX] timing cfg, 90K: %X\n", timeCfgStc1);
                    setHDMITX_Input_Timing_Config(timing_type);

                    // panel hdmitxPanelParameter init
                    ScalerHdmiTxSetPanelParameter_config(timing_type);

                    last_frameRate = tx_output_timing_table[timing_type].frame_rate;
                }
                isSettingType = false;
                waitTxReadyCnt = 0;
            }else{
                InfoMessageHDMITx("[HDMITX][%d] wait TX ready...(%d->%d)\n", waitTxReadyCnt, timing_type, HDMITX_TIMING_TYPE);
            }
        }

#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
        if(vrrLoopMode == 0)
#endif
        ROSTimeDly(10);
#ifdef ENABLE_ISR_TASK_RUN_TIME_PROFILING // Profiling ISR/Task Run Time and Run Period
        stc1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
#endif

        Scaler_TxStateHandler();

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP  // [Mark2][FIX-ME] force re-start scaler flow to update scaler d-domain setting
        if(bForceSlrReRun){
            Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);//Let main path to search state. In order to request VO
            DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;//trigger run scaler
            DbgSclrFlgTkr.Main_Scaler_Stop_flag = FALSE;//Let scaler can be run
            vsc_force_rerun_main_scaler = TRUE;
            setHDMITX_Timing_Ready(TX_TIMING_WAIT_SETTING_DONE);
            ErrorMessageHDMITx("[HDMITX] timing_type=%d, Force Scaler Re-Run[2]\n", timing_type);
            bForceSlrReRun = 0;
        }
#endif

        // show HDMITX mode change statue
        curStreamReady = HDMITX_Get_HdmiTxStreamReady();
        if((curStreamReady != last_txStreamReady)|| (HDMITX_Get_ReNewIndex_in() != HDMITX_Get_ReNewIndex_out())){
            timeCfgStc2 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
            ErrorMessageHDMITx("[HDMITX] Stream ready=%d->%d, 90K: %X(%d ms, LT/PixelCh=%d/%x)\n", last_txStreamReady, curStreamReady, timeCfgStc2, (timeCfgStc2 - timeCfgStc1)/90, !ScalerHdmiTxGetFastTimingSwitchEn(), ScalerHdmiTxGetPixelFormatChange());
            ErrorMessageHDMITx("[HDMITX] Scaler ReNew Index IN=%d, OUT=%d\n", HDMITX_Get_ReNewIndex_in(), HDMITX_Get_ReNewIndex_out());
            if(curStreamReady) {
                HDMITX_DTG_Wait_Den_Stop_Done((unsigned char*)__FUNCTION__, __LINE__);
				//when d-domain is not setting the clk source should not change to TXDPLL
                // set D-domain clock source
                ScalerHdmiTx_Set_DtgClkSource(timing_type, DCLK_SRC_HDMITX_DPLL); // 0: from dpll, 1: from hdmitxpll
                udelay(150);

                // VO iv_src_sel: 2:HDMIDMAVS(dispd_vo_ds2_vsync=>memcdtg), a:DISPD_S1(dispd_vo_ds1_vsync=>uzudtg)
                if(curStreamReady != last_txStreamReady){
                    rpcVoIvSrcSel(VSGEN_IV_SRC_SEL_DISPD_S1);
                    // [MARK2] FIX-ME -- update mark2 scaler/hdmitx control flow
                    setHDMITX_Timing_Ready(TX_TIMING_SETTING_DONE);
                }

                // reset dispD FIFO status when input timing is changed
                if(1)//if(HDMITX_Get_DscSrc() == DSC_SRC_DISPD)
                {
                    // [Mark2][IC][FIX-ME] BYPASS MEMC In IC BRING UP stage
                    #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP
                    Scaler_MEMC_outMux(_ENABLE,_DISABLE);	//db on, mux off
                    #endif

                    ScalerHdmiTx0_H5xDsc_inputSrcConfig(DSC_SRC_DISPD);
                }

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
                // update CRC input
                rtd_maskl(HDMITX21_MAC_HDMI21_CRC_0_reg, ~HDMITX21_MAC_HDMI21_CRC_0_crc_src_sel_mask,
                                HDMITX21_MAC_HDMI21_CRC_0_crc_src_sel(timing_type >= TX_TIMING_HDMI21_DSCE_TIMING_START? 1: 0));
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
            }

            last_txStreamReady = curStreamReady ;
            HDMITX_Set_ReNewIndex_out(HDMITX_Get_ReNewIndex_in());
        }

#ifdef ENABLE_FIX_FRL_RATE_FOR_VRR_FAST_TIMING_SWAITCH // for fast timing switch & VRR function verify
        // VRR loop mode handler
        Scaler_FastTimingSwitch_VRR_4k2k_loopMode_handler();
#endif

#ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify
#ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
        loopCnt[0]++;
        if(lineCnt < last_lineCnt){
            bNewFrame = 1;
            loopCnt[1]++;
        }
        last_lineCnt = lineCnt;
        // check CRC after stream ready
        if(curStreamReady && bNewFrame){
            //static unsigned int stc_lastDump=0, loopCnt=0;
            //unsigned int stc = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
            unsigned char hdmiMode = (tx_output_timing_table[timing_type].frl_type? HDMITX_CRC_HDMI21TX: HDMITX_CRC_HDMI20TX);

            bNewFrame = 0;

            // UZU CRC
            hdmitx_crc[HDMITX_CRC_UZU].crc_ch[0] = SCALEUP_D_UZU_CRC_Result_get_uzu_crc(rtd_inl(SCALEUP_D_UZU_CRC_Result_reg));

            // DSCE CRC
            hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[0] = DSCE_DSC_CRC_0_M_get_crc_0_15_8(rtd_inl(DSCE_DSC_CRC_0_M_reg))|DSCE_DSC_CRC_0_L_get_crc_0_7_0(rtd_inl(DSCE_DSC_CRC_0_L_reg));
            hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[1] = DSCE_DSC_CRC_1_M_get_crc_1_15_8(rtd_inl(DSCE_DSC_CRC_1_M_reg))|DSCE_DSC_CRC_1_L_get_crc_1_7_0(rtd_inl(DSCE_DSC_CRC_1_L_reg));;
            hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[2] = DSCE_DSC_CRC_2_M_get_crc_2_15_8(rtd_inl(DSCE_DSC_CRC_2_M_reg))|DSCE_DSC_CRC_2_L_get_crc_2_7_0(rtd_inl(DSCE_DSC_CRC_2_L_reg));;

            // MISC CRC
            hdmitx_crc[HDMITX_CRC_MISC].crc_ch[0] = HDMITX_MISC_CRC_RESULT_get_crc_result(rtd_inl(HDMITX_MISC_CRC_RESULT_reg));

            // HDMI21 CRC
            if(hdmiMode == HDMITX_CRC_HDMI21TX){
                int ucByteShift, ucChNum;
                for(ucByteShift=0; ucByteShift < 2; ucByteShift++) // ucByteShift=0: B[7:0], ucByteShift=1:B [11:8]
                    for(ucChNum=0; ucChNum < HDMITX_CRC_CH_NUM; ucChNum++){ // ucChNum=0: ch0, ucChNum=1: ch1, ucChNum=2: ch2
                        if(ucByteShift==0)
                            hdmitx_crc[HDMITX_CRC_HDMI21TX].crc_ch[ucChNum] = HDMITX21_MAC_HDMI21_CRC_1_get_crcob2(rtd_inl(HDMITX21_MAC_HDMI21_CRC_1_reg)) ;
                        else
                            hdmitx_crc[HDMITX_CRC_HDMI21TX].crc_ch[ucChNum] |= (HDMITX21_MAC_HDMI21_CRC_1_get_crcob2(rtd_inl(HDMITX21_MAC_HDMI21_CRC_1_reg)) << 8);
                    }

                // CRC reset to reset CRC read sequence
                rtd_clearbits(HDMITX21_MAC_HDMI21_CRC_0_reg, HDMITX21_MAC_HDMI21_CRC_0_crc_en_mask);
                rtd_setbits(HDMITX21_MAC_HDMI21_CRC_0_reg, HDMITX21_MAC_HDMI21_CRC_0_crc_en_mask);
            }else{ // HDMI20 CRC
                int ucChNum;
                for(ucChNum=0; ucChNum<HDMITX_CRC_CH_NUM; ucChNum++){
                    rtd_maskl(HDMITX20_MAC0_HDMI_CRC_0_reg, ~HDMITX20_MAC0_HDMI_CRC_0_crc_chsel_mask, HDMITX20_MAC0_HDMI_CRC_0_crc_chsel(ucChNum));
                    hdmitx_crc[HDMITX_CRC_HDMI20TX].crc_ch[ucChNum] =
                        ((HDMITX20_MAC0_HDMI_CRC_1_get_tmds_crc_31_24(rtd_inl(HDMITX20_MAC0_HDMI_CRC_1_reg)) << 24)
                        |(HDMITX20_MAC0_HDMI_CRC_2_get_tmds_crc_23_16(rtd_inl(HDMITX20_MAC0_HDMI_CRC_2_reg)) << 16)
                        |(HDMITX20_MAC0_HDMI_CRC_3_get_tmds_crc_15_8(rtd_inl(HDMITX20_MAC0_HDMI_CRC_3_reg)) << 8)
                        |HDMITX20_MAC0_HDMI_CRC_4_get_tmds_crc_7_0(rtd_inl(HDMITX20_MAC0_HDMI_CRC_4_reg)));
                }
            }

#if 0 // dump TX driver state
            //NoteMessageHDMITx("[DBG] stc/lastStc@loopCnt=%x/%x@%d@%d\n", stc, stc_lastDump, loopCnt, __LINE__);
            NoteMessageHDMITx("[DBG][CRC] loopCnt[%d]@%d\n", loopCnt[1], lineCnt);
#endif
            //if(((unsigned int)(stc - stc_lastDump) > 90000)|| ((++loopCnt % 5) == 0))
            {
                // dump current CRC value
                int ucChNum, lineCnt;
                lineCnt = PPOVERLAY_new_meas0_linecnt_real_get_line_cnt_rt(IoReg_Read32(PPOVERLAY_new_meas0_linecnt_real_reg));
                if(timing_type >= TX_TIMING_HDMI21_DSCE_TIMING_START)
                    NoteMessageHDMITx("[HDMITX][%d][%d@%d] CRC DSCE=%8x/%8x/%8x\n", timing_type, loopCnt[1], lineCnt, hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[0], hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[1], hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[2]);
                NoteMessageHDMITx("[HDMITX][%d][%d@%d] CRC.UZU/MISC=%8x/%8x\n", timing_type, loopCnt[1], lineCnt, hdmitx_crc[HDMITX_CRC_UZU].crc_ch[0], hdmitx_crc[HDMITX_CRC_MISC].crc_ch[0]);
                NoteMessageHDMITx("[HDMITX][%d][%d@%d] CRC.TX2%c=%8x/%8x/%8x\n", timing_type, loopCnt[1], lineCnt, (hdmiMode==HDMITX_CRC_HDMI21TX? '1':'0'), hdmitx_crc[hdmiMode].crc_ch[0], hdmitx_crc[hdmiMode].crc_ch[1], hdmitx_crc[hdmiMode].crc_ch[2]);

                // save current CRC value
                for(ucChNum=0; ucChNum < HDMITX_CRC_CH_NUM; ucChNum++){
                    pre_hdmitx_crc[HDMITX_CRC_UZU].crc_ch[ucChNum] = hdmitx_crc[HDMITX_CRC_UZU].crc_ch[ucChNum];
                    pre_hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[ucChNum] = hdmitx_crc[HDMITX_CRC_DSCE].crc_ch[ucChNum];
                    pre_hdmitx_crc[HDMITX_CRC_MISC].crc_ch[ucChNum] = hdmitx_crc[HDMITX_CRC_MISC].crc_ch[ucChNum];
                    pre_hdmitx_crc[HDMITX_CRC_HDMI20TX].crc_ch[ucChNum] = hdmitx_crc[HDMITX_CRC_HDMI20TX].crc_ch[ucChNum];
                    pre_hdmitx_crc[HDMITX_CRC_HDMI21TX].crc_ch[ucChNum] = hdmitx_crc[HDMITX_CRC_HDMI21TX].crc_ch[ucChNum];
                }
                //stc_lastDump = stc;
            }
        }
#endif // #ifdef _MARK2_ZEBU_BRING_UP_CHECK_CRC // [MARK2] FIX-ME -- check DSCE/HDMITX CRC
#endif // #ifdef CONFIG_ENABLE_MARK2_ZEBU_BRING_UP // [MARK2] for Mark2 Zebu platform verify

#ifdef ENABLE_ISR_TASK_RUN_TIME_PROFILING // Profiling ISR/Task Run Time and Run Period
        isrRunTime_cnt[ISR_IDX_TASK_HDMITX]++;
        isrRunTime_stc[ISR_IDX_TASK_HDMITX] += (IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg) - stc1);
#endif
    }

    DebugMessageHDMITx("[HDMITX] Done!!%s\n", __FUNCTION__);
    return 0;
}



int HDMITX_event_handler(void *data)
{

    DebugMessageHDMITx("[HDMITX] Handler start...@%s\n", __FUNCTION__);

    // event check & process
    while(1){
        //
        ROSTimeDly(100);
    }

    DebugMessageHDMITx("[HDMITX] Done!!@%s\n", __FUNCTION__);
    return 0;
}


static void create_hdmitx_tsk(void)
{
	int err;
	if (hdmitx_tsk_running_flag == FALSE) {
		p_hdmitx_tsk = kthread_create(HDMITX_state_handler, NULL, "hdmitx_tsk");

	    if (p_hdmitx_tsk) {
			wake_up_process(p_hdmitx_tsk);
			hdmitx_tsk_running_flag = TRUE;
	    } else {
	    	err = PTR_ERR(p_hdmitx_tsk);
	    	ErrorMessageHDMITx("[HDMITX] Unable to start create_hdmitx_tsk (err_id = %d)./n", err);
	    }
	}
}

static void delete_hdmitx_tsk(void)
{
	int ret;
	if (hdmitx_tsk_running_flag) {
 		ret = kthread_stop(p_hdmitx_tsk);
 		if (!ret) {
 			p_hdmitx_tsk = NULL;
 			hdmitx_tsk_running_flag = FALSE;
			ErrorMessageHDMITx("[HDMITX] hdmitx_scaler_tsk thread stopped\n");
 		}
	}
}


void hdmitx_sys_clock_reset(void)
{

   ErrorMessageHDMITx("[HDMITX] %s\n", __FUNCTION__);
   NoteMessageHDMITx("[HDMITX][Now] CRT RST/CLK=%x/%x\n", rtd_inl(SYS_REG_SYS_SRST4_reg), rtd_inl(SYS_REG_SYS_CLKEN4_reg));

    rtd_outl(SYS_REG_SYS_SRST4_reg, (UINT32)(SYS_REG_SYS_SRST4_rstn_dsce_mask|SYS_REG_SYS_SRST4_rstn_hdmitx_mask| SYS_REG_SYS_SRST4_rstn_hdmitx_phy_mask| SYS_REG_SYS_SRST4_rstn_hdmitx_off_mask));//Reset bit of hdmi dsce/hdmi hdmitx
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, (UINT32)(SYS_REG_SYS_CLKEN4_clken_dsce_mask|SYS_REG_SYS_CLKEN4_clken_hdmitx_mask|SYS_REG_SYS_CLKEN4_write_data_mask));//clk bit of hdmi dsce/hdmitx
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, (UINT32)(SYS_REG_SYS_CLKEN4_clken_dsce_mask|SYS_REG_SYS_CLKEN4_clken_hdmitx_mask));//clk bit of hdmi dsce/hdmitx
    rtd_outl(SYS_REG_SYS_SRST4_reg, (UINT32)(SYS_REG_SYS_SRST4_rstn_dsce_mask|SYS_REG_SYS_SRST4_rstn_hdmitx_mask| SYS_REG_SYS_SRST4_rstn_hdmitx_phy_mask| SYS_REG_SYS_SRST4_rstn_hdmitx_off_mask|SYS_REG_SYS_SRST4_write_data_mask));//Reset bit of hdmi dsce/hdmi hdmitx
    rtd_outl(SYS_REG_SYS_CLKEN4_reg, (UINT32)(SYS_REG_SYS_CLKEN4_clken_dsce_mask|SYS_REG_SYS_CLKEN4_clken_hdmitx_mask|SYS_REG_SYS_CLKEN4_write_data_mask));//clk bit of hdmi dsce/hdmitx

    NoteMessageHDMITx("[HDMITX][New] CRT RST/CLK=%x/%x\n", rtd_inl(SYS_REG_SYS_SRST4_reg), rtd_inl(SYS_REG_SYS_CLKEN4_reg));

    // [RL6583-3452] fix HDMITX 2.0 6G Guard Bang Disparity
    rtd_maskl(HDMITX20_MAC1_VFIFO_8_reg, ~HDMITX20_MAC1_VFIFO_8_tmds_disp_cnt_sel_mask, HDMITX20_MAC1_VFIFO_8_tmds_disp_cnt_sel_mask);

    ErrorMessageHDMITx("[HDMITX] %s Done\n", __FUNCTION__);

    return;
}


void hdmitx_sys_clock_disable(void)
{
   ErrorMessageHDMITx("[HDMITX] %s\n", __FUNCTION__);
   NoteMessageHDMITx("[HDMITX][Now] CRT RST/CLK=%x/%x\n", rtd_inl(SYS_REG_SYS_SRST4_reg), rtd_inl(SYS_REG_SYS_CLKEN4_reg));

    rtd_outl(SYS_REG_SYS_CLKEN4_reg, (UINT32)(SYS_REG_SYS_CLKEN4_clken_dsce_mask|SYS_REG_SYS_CLKEN4_clken_hdmitx_mask));//clk bit of hdmi dsce/hdmitx
    rtd_outl(SYS_REG_SYS_SRST4_reg, (UINT32)(SYS_REG_SYS_SRST4_rstn_dsce_mask|SYS_REG_SYS_SRST4_rstn_hdmitx_mask));//Reset bit of hdmi dsce/hdmi hdmitx

    NoteMessageHDMITx("[HDMITX][New] CRT RST/CLK=%x/%x\n", rtd_inl(SYS_REG_SYS_SRST4_reg), rtd_inl(SYS_REG_SYS_CLKEN4_reg));
    ErrorMessageHDMITx("[HDMITX] %s Done\n", __FUNCTION__);

    return;
}

void setHdmitxAudioChanel(unsigned char ch_num)
{
	bHdmiTxAudio_ch = ch_num;
	return;
}
unsigned char getHdmitxAudioChanel(void)
{
	return bHdmiTxAudio_ch;
}
void setHdmitxAudioType(unsigned char audio_type)
{
	bHdmiTxAudio_type = audio_type;
	return;
}
unsigned char getHdmitxAudioType(void)
{
	return bHdmiTxAudio_type;
}
void setHdmitxAudioMute(unsigned char audio_mute)
{
	bHdmiTxAudio_mute = audio_mute;
	return;
}
unsigned char getHdmitxAudioMute(void)
{
	return bHdmiTxAudio_mute;
}
void setHdmitxAudioFreq(unsigned int audio_freq)
{
	HdmiTxAudio_freq = audio_freq;
	return;
}
unsigned int getHdmitxAudioFreq(void)
{
	return HdmiTxAudio_freq;
}


int hdmitx_init_module(void)
{
    hdmitx_sys_clock_reset();
    create_hdmitx_tsk();
    return 0;
}


void __exit hdmitx_cleanup_module(void)
{
    delete_hdmitx_tsk();
    hdmitx_sys_clock_disable();
    return;
}

module_init(hdmitx_init_module) ;
module_exit(hdmitx_cleanup_module) ;
