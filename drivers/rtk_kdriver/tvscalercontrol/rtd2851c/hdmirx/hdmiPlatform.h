#ifndef __HDMI_PLATFOM_H
#define __HDMI_PLATFOM_H
#ifndef BUILD_QUICK_SHOW
#include <mach/rtk_timer.h>
#include "generated/autoconf.h"
#endif
#include <tvscalercontrol/io/ioregdrv.h>


/**********************************************************************************************
*
*   Marco or Definitions
*
**********************************************************************************************/

//------------------------------------------
// Definitions of Bits
//------------------------------------------
#define _ZERO                       0x00
#define _BIT0                       0x01
#define _BIT1                       0x02
#define _BIT2                       0x04
#define _BIT3                       0x08
#define _BIT4                       0x10
#define _BIT5                       0x20
#define _BIT6                       0x40
#define _BIT7                       0x80
#define _BIT8                       0x0100
#define _BIT9                       0x0200
#define _BIT10                      0x0400
#define _BIT11                      0x0800
#define _BIT12                      0x1000
#define _BIT13                      0x2000
#define _BIT14                      0x4000
#define _BIT15                      0x8000
#define _BIT16                      0x10000
#define _BIT17                      0x20000
#define _BIT18                      0x40000
#define _BIT19                      0x80000
#define _BIT20                      0x100000
#define _BIT21                      0x200000
#define _BIT22                      0x400000
#define _BIT23                      0x800000
#define _BIT24                      0x1000000
#define _BIT25                      0x2000000
#define _BIT26                      0x4000000
#define _BIT27                      0x8000000
#define _BIT28                      0x10000000
#define _BIT29                      0x20000000
#define _BIT30                      0x40000000
#define _BIT31                      0x80000000

#ifndef UT_flag
#define hdmi_in(addr)                       rtd_inl(addr)
#define hdmi_out(addr, value)               rtd_outl(addr, value)
#define hdmi_mask(addr, andmask, ormask)    rtd_maskl(addr, andmask, ormask)
#ifndef BUILD_QUICK_SHOW
#define hdmi_get_system_time_ms()           rtk_timer_misc_90k_ms()
#else
#define hdmi_get_system_time_ms()           (hdmi_in(0xB801B6B8)/90)
#endif
#else
#define hdmi_in(addr)                       IoReg_Read32(addr)
#define hdmi_out(addr, value)               IoReg_Write32(addr, value)
#define hdmi_mask(addr, andmask, ormask)    IoReg_Mask32(addr, andmask, ormask)
#define hdmi_get_system_time_ms()           0
#endif // UT_flag

#define HDMI_RX_DBG_EN
#define TAG_NAME_HDMI "HDMI"

#define DEFAULT_HDMI_SINGLE_PIXEL_MODE    1
#define DEFAULT_HDMI_MULTI_PIXEL_MODE    2 // 2 or 4, depend on IC specification
#define ENABLE_FORCE_DRIVER_EDID_MODIFY    0 //Need to control by Android

#ifdef UT_flag
#undef CONFIG_SUPPORT_SCALER
#ifndef MAX
    #define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif

#endif
//------------------------------------------
// Scalar Function Wrapper
//------------------------------------------
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)

#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>


#define GET_MODE_CURR()             (Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_MODE_CURR))
#define GET_MODE_POLARITY()         (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_POLARITY))
#define GET_MODE_IVTOTAL()          (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVLEN))
#define GET_MODE_IHTOTAL()          (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHLEN))
#define GET_MODE_IVHEIGHT()         (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT))
#define GET_MODE_IHWIDTH()          (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHWID))
#define GET_MODE_IVSTA()            (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVSTARTPOS))
#define GET_MODE_IHSTA()            (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHSTARTPOS))
#define GET_MODE_IVFREQ()           (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVFREQ))
#define GET_MODE_IHFREQ()           (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHFREQ))
#define GET_MODE_IVCOUNT()          (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IVCOUNT))
#define GET_MODE_IHCOUNT()          (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_IHCOUNT))
#define GET_MODE_HDMI_DOLBY_VSIF()  (get_HDMI_Dolby_VSIF_mode())


#define SET_MODE_CURR(x)            (Scaler_DispSetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_MODE_CURR, (x)))
#define SET_MODE_POLARITY(x)        (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_POLARITY,  (x)))
#define SET_MODE_IVTOTAL(x)         (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVLEN, (x)))
#define SET_MODE_IHTOTAL(x)         (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHLEN, (x)))
#define SET_MODE_IVHEIGHT(x)        (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVHEIGHT, (x)))
#define SET_MODE_IHWIDTH(x)         (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHWID, (x)))
#define SET_MODE_IVSTA(x)           (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVSTARTPOS,(x)))
#define SET_MODE_IHSTA(x)           (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHSTARTPOS, (x)))
#define SET_MODE_IVFREQ(x)          (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVFREQ, (x)))
#define SET_MODE_IHFREQ(x)          (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHFREQ, (x)))
#define SET_MODE_IVCOUNT(x)         (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IVCOUNT, (x)))
#define SET_MODE_IHCOUNT(x)         (Scaler_ModeSet_Hdmi_ModeInfo(SLR_MODE_IHCOUNT, (x)))
#define SET_MODE_PREHDMI_DOLBY_VSIF(x) (set_PreHDMI_Dolby_VSIF_mode(x))
#define	SET_REPLY_ZERO_TIMING_FLAG(x,y) (Set_Reply_Zero_Timing_Flag(x,y))
#define SCALERDRV_WAIT_VS()         WaitFor_IVS1()

#define GET_DISPLAY_MODE_SRC(x)     Get_DisplayMode_Src(x)
#define GET_HDMI_SEARCH_MODE(x)     Hdmi_Search_Mode(x)

#include <tvscalercontrol/scalerdrv/scalerdisplay.h>

#define GET_FORCE_BG_STATUS()       drvif_scalerdisplay_get_force_bg_status()
#define SET_FORCE_BG_ENABLE(en)     mute_control(SLR_MAIN_DISPLAY, en)

#include <tvscalercontrol/panel/panelapi.h>
#define GET_PANEL_REFRESH_RATE()    Get_DISPLAY_REFRESH_RATE()
#else

#define GET_MODE_CURR()              0
#define GET_MODE_POLARITY()          0
#define GET_MODE_IVTOTAL()           0
#define GET_MODE_IHTOTAL()           0
#define GET_MODE_IVHEIGHT()          0
#define GET_MODE_IHWIDTH()           0
#define GET_MODE_IVSTA()             0
#define GET_MODE_IHSTA()             0
#define GET_MODE_IVFREQ()            0
#define GET_MODE_IHFREQ()            0
#define GET_MODE_IVCOUNT()           0
#define GET_MODE_IHCOUNT()           0
#define GET_MODE_HDMI_DOLBY_VSIF()   0


#define SET_MODE_CURR(x)            {}
#define SET_MODE_POLARITY(x)        {}
#define SET_MODE_IVTOTAL(x)         {}
#define SET_MODE_IHTOTAL(x)         {}
#define SET_MODE_IVHEIGHT(x)        {}
#define SET_MODE_IHWIDTH(x)         {}
#define SET_MODE_IVSTA(x)           {}
#define SET_MODE_IHSTA(x)           {}
#define SET_MODE_IVFREQ(x)          {}
#define SET_MODE_IHFREQ(x)          {}
#define SET_MODE_IVCOUNT(x)         {}
#define SET_MODE_IHCOUNT(x)         {}
#define SET_MODE_PREHDMI_DOLBY_VSIF(x) {}
#define	SET_REPLY_ZERO_TIMING_FLAG(x,y) {}
#define SCALERDRV_WAIT_VS()

#define GET_DISPLAY_MODE_SRC(x)     (VSC_INPUTSRC_HDMI)
#define GET_HDMI_SEARCH_MODE(x)     (0)
#define HDMI_AV_OUT_DISABLE_VSC_PRE_ACTION()	       {}

#define GET_FORCE_BG_STATUS()       (0)
#define SET_FORCE_BG_ENABLE(en)     {}

#define GET_PANEL_REFRESH_RATE()    0
#endif

#define SET_SCALER_POLARITY(x)      {}
#define SET_SCALER_IVTOTAL(x)       {}
#define SET_SCALER_IHTOTAL(x)       {}
#define SET_SCALER_IVHEIGHT(x)      {}
#define SET_SCALER_IVHEIGHT_PRE(x)  {}
#define SET_SCALER_IHWIDTH(x)       {}
#define SET_SCALER_IHWIDTH_PRE(x)   {}
#define SET_SCALER_IVSTA(x)         {}
#define SET_SCALER_IVSTA_PRE(x)     {}
#define SET_SCALER_IHSTA(x)         {}
#define SET_SCALER_IHSTA_PRE(x)     {}
#define SET_SCALER_IVFREQ(x)        {}
#define SET_SCALER_IHFREQ(x)        {}
#define SET_SCALER_IVCOUNT(x)       {}
#define SET_SCALER_IHCOUNT(x)       {}
#define SET_SCALER_ISINTERLACE(x)   {}
#define SET_SCALER_IS422(x)         {}
#define SET_SCALER_COLORSPACE(x)    {}
#define SET_SCALER_COLORDEPTH(x)    {}
#define SET_SCALER_COLORIMEYRY(x)   {}
#define SCALERDRV_FORCE_ONMS_FAIL() {}    //TBD, need add by scaler

//------------------------------------------
// HDR Function Wrapper
//------------------------------------------

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)

#define GET_HDMI_DOLBY_VSIF_MODE()      get_HDMI_Dolby_VSIF_mode()
#define SET_HDMI_DOLBY_VSIF_MODE(x)     set_HDMI_Dolby_VSIF_mode(x)

#define GET_PRE_HDMI_DOLBY_VSIF_MODE()  get_PreHDMI_Dolby_VSIF_mode()
#define SET_PRE_HDMI_DOLBY_VSIF_MODE(x) set_PreHDMI_Dolby_VSIF_mode(x)

#else

#define GET_HDMI_DOLBY_VSIF_MODE()      (0)//(DOLBY_HDMI_VSIF_DISABLE)
#define SET_HDMI_DOLBY_VSIF_MODE(x)     {}

#define GET_PRE_HDMI_DOLBY_VSIF_MODE()  (0)//(DOLBY_HDMI_VSIF_DISABLE)
#define SET_PRE_HDMI_DOLBY_VSIF_MODE(x) {}

#endif

//------------------------------------------
// Audio Function Wrapper
//------------------------------------------

// info audio FW when hdmi driver get ACR info
#ifdef CONFIG_RTK_KDRV_AUDIO
extern long rtkaudio_send_hdmi_fs(long hdmi_fs);
#define SET_AUDIO_ACR_FREQ(x)       rtkaudio_send_hdmi_fs(x)

// #include <rtk_kdriver/hal_inc/hal_audio.h> // ToDo: no RHAL_AUDIO
extern void RHAL_AUDIO_HDMI_GetAudioReturnChannel(bool *onoff);
extern void RHAL_AUDIO_HDMI_GetEnhancedAudioReturnChannel(bool *onoff);

#define GET_AUDIO_ARC_EN(x)    RHAL_AUDIO_HDMI_GetAudioReturnChannel(x)
#define GET_AUDIO_EARC_EN(x)    RHAL_AUDIO_HDMI_GetEnhancedAudioReturnChannel(x)
#else
#define SET_AUDIO_ACR_FREQ(x)
#define GET_AUDIO_ARC_EN(x)    {}
#define GET_AUDIO_EARC_EN(x)    {}
#endif // CONFIG_RTK_KDRV_AUDIO

#include <rbus/earc_reg.h> // for EARC connect state
#define GET_EARC_DISC_STATE()    EARC_eARC_CMDC_REQ_get_disc_state(hdmi_in(EARC_eARC_CMDC_REQ_reg))
/**********************************************************************************************
*
*   External Funtion Declarations
*
**********************************************************************************************/

// exported in scalerdrv/mode.c   : off line measure is a shared soure, so we need the semaphore to prevent race condition
extern struct semaphore* get_offline_semaphore(void);

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
extern void Hdmi_Av_Out_Disable_Vsc_Pre_Action(void);
#else
#define Hdmi_Av_Out_Disable_Vsc_Pre_Action()    0
#endif

#endif // __HDMI_PLATFOM_H

