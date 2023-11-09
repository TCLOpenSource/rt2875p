#ifndef _HDMITX_H_
#define _HDMITX_H_

#include <scaler/scalerCommon.h>
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME_HDMITX "HDMITX"
#define shell_printf(level, format, args...) rtd_pr_hdmitx_emerg(format, ## args)
#define ROSTimeDly msleep

// [NOTE] add new timing table on tx_cea_timing_table[] && tx_output_timing_table[] when add a new HDMITX output timing
typedef enum{
    /* *********** HDMI21 ******************** */
    /* HDMI2.1 4K2Kp60 444 */
    TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_6G4L=0,

    /* HDMI2.1 8K4Kp60 420 */
    TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_8G,
    TX_TIMING_HDMI21_8K4KP60_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP60_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP60_YUV420_12BIT_12G,

    /* HDMI2.1 8K4Kp59 420 */
    TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_8G,
    TX_TIMING_HDMI21_8K4KP59_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP59_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP59_YUV420_12BIT_12G,

    /* HDMI2.1 8K4Kp23 420 */
    TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP23_YUV420_12BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP23_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP23_YUV420_10BIT_10G,

    /* HDMI2.1 8K4Kp24 420 */
    TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP24_YUV420_12BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP24_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP24_YUV420_10BIT_10G,

    /* HDMI2.1 8K4Kp25 420 */
    TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP25_YUV420_12BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP25_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP25_YUV420_10BIT_10G,

    /* HDMI2.1 8K4Kp30 420 */
    TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP30_YUV420_12BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP30_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP30_YUV420_10BIT_10G,

    /* HDMI2.1 8K4Kp29 420 */
    TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP29_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_8K4KP29_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP29_YUV420_12BIT_6G4L,

    /* HDMI2.1 8K4Kp30 444 */
    TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP30_YUV444_8BIT_12G,

    /* HDMI2.1 8K4Kp47 420 */
    TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_8G,
    TX_TIMING_HDMI21_8K4KP47_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP47_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP47_YUV420_12BIT_12G,

    /* HDMI2.1 8K4Kp48 420 */
    TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_8G,
    TX_TIMING_HDMI21_8K4KP48_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP48_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP48_YUV420_12BIT_12G,

    /* HDMI2.1 8K4Kp50 420 */
    TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_8G,
    TX_TIMING_HDMI21_8K4KP50_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_8K4KP50_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_8K4KP50_YUV420_12BIT_12G,

    /* HDMI2.1 4K2Kp120 444 */
    TX_TIMING_HDMI21_4K2KP120_YUV444_8BIT_8G,

    /* HDMI2.1 special 4k 444 input timing for bridge application */
    TX_TIMING_HDMI21_3840x4320P60_YUV444_8BIT_8G,
    TX_TIMING_HDMI21_3840x8640P30_YUV444_8BIT_8G,
    TX_TIMING_HDMI21_4192x4320P60_YUV444_8BIT_8G,

    /* HDMI2.1 4K2Kp47 444 */
    TX_TIMING_HDMI21_4K2KP47_YUV444_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP47_YUV444_10BIT_10G,

    /* HDMI2.1 4K2Kp48 444 */
    TX_TIMING_HDMI21_4K2KP48_YUV444_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP48_YUV444_10BIT_10G,

    /* HDMI2.1 4K2Kp50 444 */
    TX_TIMING_HDMI21_4K2KP50_YUV444_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP50_YUV444_10BIT_10G,

    /* HDMI2.1 4K2Kp60 444 */
    TX_TIMING_HDMI21_4K2KP60_RGB_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_RGB_10BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV444_10BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV444_12BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV444_8BIT_12G,

    /* HDMI2.1 4K2Kp47 420 */
    TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP47_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP47_YUV420_10BIT_10G,

    /* HDMI2.1 4K2Kp48 420 */
    TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP48_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP48_YUV420_10BIT_10G,

    /* HDMI2.1 4K2Kp50 420 */
    TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP50_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP50_YUV420_10BIT_10G,

    /* HDMI2.1 4K2Kp60 420 */
    TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_6G4L,
    TX_TIMING_HDMI21_4K2KP60_YUV420_8BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV420_10BIT_10G,
    TX_TIMING_HDMI21_4K2KP60_YUV420_12BIT_6G4L,

    /* HDMI2.1 2k1k@3G (3Lane) */
    TX_TIMING_HDMI21_640x480P59_RGB_8BIT_3G,
    TX_TIMING_HDMI21_480P59_YUV444_8BIT_3G,
    TX_TIMING_HDMI21_720P60_YUV444_8BIT_3G,
    TX_TIMING_HDMI21_1080P60_YUV444_8BIT_3G,

    /* HDMI2.1 2k1k@6G4L */
    TX_TIMING_HDMI21_640x480P59_RGB_8BIT_6G4L,
    TX_TIMING_HDMI21_480P59_YUV444_8BIT_6G4L,
    TX_TIMING_HDMI21_720P60_YUV444_8BIT_6G4L,
    TX_TIMING_HDMI21_1080P60_YUV444_8BIT_6G4L,
    TX_TIMING_HDMI21_1080P60_RGB_8BIT_6G4L,

    /* *********** HDMI20 ******************** */
    /* HDMI2.0 640x480p59 RGB */
    TX_TIMING_HDMI20_640X480P59_RGB_8BIT,

    /* HDMI2.0 720x480p59 YUV */
    TX_TIMING_HDMI20_720X480P59_YUV444_8BIT,

    /* HDMI2.0 720x576p50 YUV */
    TX_TIMING_HDMI20_720X576P50_YUV444_8BIT,

    /* HDMI2.0 720p50 444 */
    TX_TIMING_HDMI20_720P50_YUV444_8BIT,

    /* HDMI2.0 720p60 444 */
    TX_TIMING_HDMI20_720P60_YUV444_8BIT,

    /* HDMI2.0 1080p24 444 */
    TX_TIMING_HDMI20_1080P24_YUV444_8BIT,

    /* HDMI2.0 1080p60 444 */
    TX_TIMING_HDMI20_1080P47_YUV444_8BIT,
    TX_TIMING_HDMI20_1080P48_YUV444_8BIT,
    TX_TIMING_HDMI20_1080P50_YUV444_8BIT,
    TX_TIMING_HDMI20_1080P60_YUV444_8BIT,

    /* HDMI2.0 4k2kp30 444 */
    TX_TIMING_HDMI20_4K2KP30_YUV444_8BIT,

    /* HDMI2.0 4k2kp60 420 */
    TX_TIMING_HDMI20_4K2KP60_YUV420_8BIT,
    TX_TIMING_HDMI20_4K2KP60_YUV420_10BIT,
    TX_TIMING_HDMI20_4K2KP60_YUV420_12BIT,

    /* HDMI2.0 4k2kp60 444 */
    TX_TIMING_HDMI20_4K2KP47_YUV444_8BIT,
    TX_TIMING_HDMI20_4K2KP48_YUV444_8BIT,
    TX_TIMING_HDMI20_4K2KP50_YUV444_8BIT,
    TX_TIMING_HDMI20_4K2KP60_YUV444_8BIT,
    TX_TIMING_HDMI20_4K2KP60_RGB_8BIT,

    /* HDMI20 STB6500 timing */
    TX_TIMING_HDMI20_STB6500_720X576P50_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_720P50_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_720P60_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_1080P50_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_1080P60_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_4K2KP30_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_4K2KP50_YUV444_8BIT,
    TX_TIMING_HDMI20_STB6500_4K2KP60_YUV444_8BIT,

    /* *********** HDMI21 DSC timing ******************** */
    TX_TIMING_HDMI21_DSCE_TIMING_START,
    /* HDMI2.1 DSC 4k2kp60 444 */
    TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_8BIT_6G4L = TX_TIMING_HDMI21_DSCE_TIMING_START,
    TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_10BIT_6G4L,
    TX_TIMING_HDMI21_DSCE_3840_2160P60Hz_YUV444_12BIT_6G4L,

    /* HDMI2.1 DSC 4k2kp120 444 */
    TX_TIMING_HDMI21_DSCE_3840_2160P120Hz_YUV444_8BIT_6G4L,

    /* HDMI2.1 DSC 8k4kp30 444 */
    TX_TIMING_HDMI21_DSCE_7680_4320P30Hz_YUV444_8BIT_6G4L,

    /* HDMI2.1 DSC 8k4kp60 444 */
    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_6G4L,
    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_6G4L,
    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_6G4L,

    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_8BIT_12G,
    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_10BIT_12G,
    TX_TIMING_HDMI21_DSCE_7680_4320P60Hz_YUV444_12BIT_12G,

    /* HDMI2.1 DSC 8k4kp59 444 */
    TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_8BIT_6G4L,
    TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_10BIT_6G4L,
    TX_TIMING_HDMI21_DSCE_7680_4320P59Hz_YUV444_12BIT_6G4L,

    TX_TIMING_NUM,
} tx_timing_type;

//int HDMITX_state_handler(void *data);
//int HDMITX_event_handler(void *data);

typedef enum{
  DCLK_SRC_DPLL=0,
  DCLK_SRC_HDMITX_DPLL=1
}DCLK_SRC_TYPE;


typedef union
{
    unsigned int regValue;
    struct {
        unsigned int uclimitPortNum: 3; // o: no limit, x: only output to RX portNum[x]
        unsigned char bCheckEdidInfo: 1; // check MNT name & portNum
        unsigned char bBypassLinkTraining: 1; // force bypass HPD/EDID/LinkTraining control flow
        unsigned char bEnablePtg: 1; // enable show TXSOC PTG
        unsigned char bTogglePtg: 1; // toggle TXSOC PTG
        unsigned char bEnableHdcp22: 1; // Enable HDCP2.2
        unsigned char ucDscMode: 3; // DSC mode: 0: disable, 1: default, 2: 12G (bbp=8.125), 3: 12G (bbp=9.9375), 4: 12G (bbp=12.000), 5: 12G (bbp=15.000)
        unsigned char bDisableHdcp22Rc: 1; // Disable HDCP22 RC
        unsigned int reserved: 20;
    };
}HdmiTxConfigPara1;

typedef union
{
    unsigned int regValue;
    struct {
        unsigned int ucLaneSrcCfg: 8; // 0xe4: H5 Demo/LGE Dongle, 0xb1: LGE one board
        unsigned int ucTxPnSwapCfg: 4; // 0x0: default not swap, 0xf: all lane need swap
        unsigned int ucPlatformType:2; // 0: General, 1: LG_O18, 2: LG_O20
        unsigned int vrr_frame_rate:6; // VRR: TX output frame rate: 47~60
        unsigned int vrr_mode:2; // VRR Mode: 0: disable, 1: jump/2: sequence loop mode
        unsigned int vrr_loop_delay:7; // VRR loop delay: 0~127 vsync
        unsigned int vrr_24hz: 1; // VRR min frame rate support 24hz
        unsigned int reserved: 2;
    };
}HdmiTxConfigPara2;

#ifdef _MARK2_ZEBU_BRING_UP_LOCAL_RUN // [MARK2] FIX-ME -- only for local verify
extern bool kernel_finished(void);
#endif
unsigned char ScalerHdmiTxGetTxRunFlag(void);
extern unsigned char getVscInitdoneFlag(void);

void showTimingList(void);
void setHdmitxConfig(unsigned int param1, unsigned int param2);
void setInputTimingType(int index, unsigned int param2);
void setHdmitxTiming(unsigned int frame_rate, unsigned int color_depth, unsigned int color_type);
void setHdmitxPatternGen(unsigned int width);
TX_STATUS ScalerHdmiTx_Get_Timing_Status(void);
void ScalerHdmiTx_Set_DtgClkSource(int index, DCLK_SRC_TYPE type);
unsigned char HDMITX_DTG_Wait_Den_Stop_Done(unsigned char *func, unsigned int line);
unsigned char HDMITX_DTG_Wait_Den_Start(unsigned char *func, unsigned int line);
unsigned char HDMITX_DTG_Wait_vsync_start(unsigned char *func, unsigned int line);
void setHdmitxAudioChanel(unsigned char ch_num);
unsigned char getHdmitxAudioChanel(void);
void setHdmitxAudioType(unsigned char audio_type);
unsigned char getHdmitxAudioType(void);
void setHdmitxAudioMute(unsigned char audio_mute);
unsigned char getHdmitxAudioMute(void);
void setHdmitxAudioFreq(unsigned int audio_freq);
unsigned int getHdmitxAudioFreq(void);

#endif // #ifndef _HDMITX_H_
