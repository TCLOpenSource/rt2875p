#ifndef __HDMITX_LIB_H__
#define __HDMITX_LIB_H__


#define _LPCM_ASP                                           0x00
#define _HBR_ASP                                            0x01

typedef enum
{
    DSC_SRC_TXSOC=0,
    DSC_SRC_DISPD,
}EnumHDMITXDscSrc;



typedef enum _HDMI_TX_FFE_TABLE_LIST{
    HDMI21_TX_FFE_TABLE_LIST_TV001=0,
    HDMI21_TX_FFE_TABLE_LIST_TV006,
    HDMI21_TX_FFE_TABLE_LIST_NUM,
}HDMI_TX_FFE_TABLE_LIST;

typedef enum _EnumOutputPort{
    _P0_OUTPUT_PORT=0,
    _P1_OUTPUT_PORT,
    _P2_OUTPUT_PORT,
    _P3_OUTPUT_PORT,
    _P4_OUTPUT_PORT
}EnumOutputPort;

typedef enum _EnumAudioFreqType{
    _AUDIO_FREQ_NO_AUDIO=0,
    _AUDIO_FREQ_32K,
    _AUDIO_FREQ_64K,
    _AUDIO_FREQ_128K,
    _AUDIO_FREQ_44_1K,
    _AUDIO_FREQ_88_2K,
    _AUDIO_FREQ_176_4K,
    _AUDIO_FREQ_48K,
    _AUDIO_FREQ_96K,
    _AUDIO_FREQ_192K,
    _AUDIO_FREQ_256K,
    _AUDIO_FREQ_352_8K,
    _AUDIO_FREQ_384K,
    _AUDIO_FREQ_512K,
    _AUDIO_FREQ_705_6K,
    _AUDIO_FREQ_768K
}EnumAudioFreqType;

EnumHDMITXDscSrc HDMITX_Get_DscSrc(void);
void HDMITX_Set_DscSrc(EnumHDMITXDscSrc srcType);
void HDMITX_Set_MuteEn(unsigned char enable);
unsigned char HDMITX_Get_MuteEn(void);
unsigned int HDMITX_Get_Output_Width(void);
unsigned int HDMITX_Get_Output_Height(void);
unsigned int HDMITX_Get_Output_VFreq(void);
unsigned int HDMITX_Get_Output_ColorSpace(void);
unsigned int HDMITX_Get_Output_ColorDepth(void);
unsigned char HDMITX_Get_HdmiTxStreamReady(void);
unsigned char ScalerStreamAudioGetChannelCount(EnumOutputPort ucOutputPort);
unsigned char ScalerStreamAudioGetCodingType(EnumOutputPort ucOutputPort);
unsigned char ScalerStreamAudioGetSamplingFrequency(EnumOutputPort ucOutputPort);
unsigned char ScalerStreamGetAudioMuteFlag(EnumOutputPort ucOutputPort);

void HDMITX_Set_ReNewIndex_in(unsigned int index);
void HDMITX_Set_ReNewIndex_out(unsigned int index);
unsigned char HDMITX_Get_ReNewIndex_in(void);
unsigned char HDMITX_Get_ReNewIndex_out(void);

#if 1 //#ifndef _MARK2_FIXME_H5X_ROS_API // [MARK2] FIX-ME -- replace ROS API into Linux API
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "HDMITX"

#define ErrorMessageHDMITx(format, args...)  rtd_pr_hdmitx_err(format, ## args)
#define NoteMessageHDMITx(format, args...)  rtd_pr_hdmitx_notice(format, ## args)
#define InfoMessageHDMITx(format, args...)  rtd_pr_hdmitx_info(format, ## args)
#define DebugMessageHDMITx(format, args...)  rtd_pr_hdmitx_debug(format, ## args)
#else
#define ErrorMessageHDMITx(fmt, ...) ROS_Printf_Module(LOGLEVEL_ERR, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__)
#define NoteMessageHDMITx(fmt, ...) ROS_Printf_Module(LOGLEVEL_NOTICE, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__)
#define InfoMessageHDMITx(fmt, ...) ROS_Printf_Module(LOGLEVEL_INFO, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__)
#define DebugMessageHDMITx(fmt, ...)  ROS_Printf_Module(LOGLEVEL_DEBUG, RTD_LOG_MODULE_SCPU1_SCALER, fmt, ##__VA_ARGS__) // (x,y...) //
#endif

#endif
