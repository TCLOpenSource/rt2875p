#include "rtkdemux_export.h"
#include "audio_rpc.h"
#include <linux/string.h>
#include <rtk_kdriver/rtkaudio.h>

#include "kadp_audio.h"
#include <hal_common.h>
#include <hal_audio.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define IOCTL_RET_ERROR  (-1)
#define IOCTL_FD_NULL    (-2)
#define IOCTL_ADDR_NULL  (-3)
#define IOCTL_TYPE_ERROR (-4)

 /************************************************************************
 *  Static variables
 ************************************************************************/
static char Aud_trickPlayState[2] = {0};
int gUSER_CONFIG;

extern int IOCTL_GetBufferInfo(DEMUX_BUF_INFO_T *pInfo);
extern int IOCTL_SetPrivateInfo(DEMUX_PRIVATE_INFO_T *pInfo);
extern int IOCTLEX_ReleaseBuffer(DEMUX_BUF_INFO_T *pInfo);
extern int read_register_by_type(AUDIO_REGISTER_ACCESS_T *info);

typedef void (*Audio_AtvSetSubAudioInHandle_ptr)(long);
Audio_AtvSetSubAudioInHandle_ptr fpAudio_AtvSetSubAudioInHandle = NULL;
void rtkaudio_Audio_AtvSetSubAudioInHandle_ptr(void* fun_ptr)
{
    fpAudio_AtvSetSubAudioInHandle = (Audio_AtvSetSubAudioInHandle_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetSubAudioInHandle_ptr);
//////////////
typedef void (*Audio_AtvSetAudioInHandle_ptr)(long);
Audio_AtvSetAudioInHandle_ptr fpAudio_AtvSetAudioInHandle = NULL;
void rtkaudio_Audio_AtvSetAudioInHandle_ptr(void* fun_ptr)
{
    fpAudio_AtvSetAudioInHandle = (Audio_AtvSetAudioInHandle_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetAudioInHandle_ptr);
//////////////
typedef void (*Audio_AtvSetDeviationMethod_ptr)(ATV_DEV_METHOD);
Audio_AtvSetDeviationMethod_ptr fpAudio_AtvSetDeviationMethod = NULL;
void rtkaudio_Audio_AtvSetDeviationMethod_ptr(void* fun_ptr)
{
    fpAudio_AtvSetDeviationMethod = (Audio_AtvSetDeviationMethod_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetDeviationMethod_ptr);
//////////////
typedef void (*Audio_Hw_SetFmRadioMode_ptr)(AUDIO_ATV_FM_RADIO_MODE);
Audio_Hw_SetFmRadioMode_ptr fpAudio_Hw_SetFmRadioMode = NULL;
void rtkaudio_Audio_Hw_SetFmRadioMode_ptr(void* fun_ptr)
{
    fpAudio_Hw_SetFmRadioMode = (Audio_Hw_SetFmRadioMode_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_Hw_SetFmRadioMode_ptr);
//////////////
typedef int32_t (*Audio_SetAudioHWConfig_ptr)(int32_t);
Audio_SetAudioHWConfig_ptr fpAudio_SetAudioHWConfig = NULL;
void rtkaudio_Audio_SetAudioHWConfig_ptr(void* fun_ptr)
{
    fpAudio_SetAudioHWConfig = (Audio_SetAudioHWConfig_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_SetAudioHWConfig_ptr);
//////////////
typedef void (*Audio_AtvCleanTVSourceData_ptr)(void);
Audio_AtvCleanTVSourceData_ptr fpAudio_AtvCleanTVSourceData = NULL;
void rtkaudio_Audio_AtvCleanTVSourceData_ptr(void* fun_ptr)
{
    fpAudio_AtvCleanTVSourceData = (Audio_AtvCleanTVSourceData_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvCleanTVSourceData_ptr);
//////////////
typedef void (*Audio_AtvPauseTVStdDetection_ptr)(int);
Audio_AtvPauseTVStdDetection_ptr fpAudio_AtvPauseTVStdDetection = NULL;
void rtkaudio_Audio_AtvPauseTVStdDetection_ptr(void* fun_ptr)
{
    fpAudio_AtvPauseTVStdDetection = (Audio_AtvPauseTVStdDetection_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvPauseTVStdDetection_ptr);
//////////////
typedef void (*Audio_AtvStopDecoding_ptr)(void);
Audio_AtvStopDecoding_ptr fpAudio_AtvStopDecoding = NULL;
void rtkaudio_Audio_AtvStopDecoding_ptr(void* fun_ptr)
{
    fpAudio_AtvStopDecoding = (Audio_AtvStopDecoding_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvStopDecoding_ptr);
//////////////
typedef int32_t (*Audio_HwpAADSIFInit_ptr)(ATV_CFG *);
Audio_HwpAADSIFInit_ptr fpAudio_HwpAADSIFInit = NULL;
void rtkaudio_Audio_HwpAADSIFInit_ptr(void* fun_ptr)
{
    fpAudio_HwpAADSIFInit = (Audio_HwpAADSIFInit_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpAADSIFInit_ptr);
//////////////
typedef void (*Audio_HwpCurSifType_ptr)(ATV_AUDIO_SIF_TYPE_T);
Audio_HwpCurSifType_ptr fpAudio_HwpCurSifType = NULL;
void rtkaudio_Audio_HwpCurSifType_ptr(void* fun_ptr)
{
    fpAudio_HwpCurSifType = (Audio_HwpCurSifType_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpCurSifType_ptr);
//////////////
typedef void (*Audio_AtvEnableAutoChangeSoundModeFlag_ptr)(int);
Audio_AtvEnableAutoChangeSoundModeFlag_ptr fpAudio_AtvEnableAutoChangeSoundModeFlag = NULL;
void rtkaudio_Audio_AtvEnableAutoChangeSoundModeFlag_ptr(void* fun_ptr)
{
    fpAudio_AtvEnableAutoChangeSoundModeFlag = (Audio_AtvEnableAutoChangeSoundModeFlag_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvEnableAutoChangeSoundModeFlag_ptr);
//////////////
typedef int32_t (*Audio_AtvInit_ptr)(ATV_CFG *);
Audio_AtvInit_ptr fpAudio_AtvInit = NULL;
void rtkaudio_Audio_AtvInit_ptr(void* fun_ptr)
{
    fpAudio_AtvInit = (Audio_AtvInit_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvInit_ptr);
//////////////
typedef int32_t (*Audio_HwpAADSIFFinalize_ptr)(void);
Audio_HwpAADSIFFinalize_ptr fpAudio_HwpAADSIFFinalize = NULL;
void rtkaudio_Audio_HwpAADSIFFinalize_ptr(void* fun_ptr)
{
    fpAudio_HwpAADSIFFinalize = (Audio_HwpAADSIFFinalize_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpAADSIFFinalize_ptr);
//////////////
typedef void (*Audio_AtvEnterAtvSource_ptr)(void);
Audio_AtvEnterAtvSource_ptr fpAudio_AtvEnterAtvSource = NULL;
void rtkaudio_Audio_AtvEnterAtvSource_ptr(void* fun_ptr)
{
    fpAudio_AtvEnterAtvSource = (Audio_AtvEnterAtvSource_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvEnterAtvSource_ptr);
//////////////
typedef void (*Audio_AtvStartDecoding_ptr)(void);
Audio_AtvStartDecoding_ptr fpAudio_AtvStartDecoding = NULL;
void rtkaudio_Audio_AtvStartDecoding_ptr(void* fun_ptr)
{
    fpAudio_AtvStartDecoding = (Audio_AtvStartDecoding_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvStartDecoding_ptr);
//////////////
typedef void (*Audio_HwpSetSIFDataSource_ptr)(SIF_INPUT_SOURCE);
Audio_HwpSetSIFDataSource_ptr fpAudio_HwpSetSIFDataSource = NULL;
void rtkaudio_Audio_HwpSetSIFDataSource_ptr(void* fun_ptr)
{
    fpAudio_HwpSetSIFDataSource = (Audio_HwpSetSIFDataSource_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSetSIFDataSource_ptr);
//////////////
typedef void (*Audio_Hw_Set_Scan_Std_Flag_ptr)(bool);
Audio_Hw_Set_Scan_Std_Flag_ptr fpAudio_Hw_Set_Scan_Std_Flag = NULL;
void rtkaudio_Audio_Hw_Set_Scan_Std_Flag_ptr(void* fun_ptr)
{
    fpAudio_Hw_Set_Scan_Std_Flag = (Audio_Hw_Set_Scan_Std_Flag_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_Hw_Set_Scan_Std_Flag_ptr);
//////////////
typedef int32_t (*Audio_AtvSetMtsPriority_ptr)(ATV_MTS_PRIORITY);
Audio_AtvSetMtsPriority_ptr fpAudio_AtvSetMtsPriority = NULL;
void rtkaudio_Audio_AtvSetMtsPriority_ptr(void* fun_ptr)
{
    fpAudio_AtvSetMtsPriority = (Audio_AtvSetMtsPriority_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetMtsPriority_ptr);
//////////////
typedef int32_t (*Audio_HwpSetAtvAudioBand_ptr)(SIF_INPUT_SOURCE,ATV_SOUND_STD_MAIN_SYSTEM);
Audio_HwpSetAtvAudioBand_ptr fpAudio_HwpSetAtvAudioBand = NULL;
void rtkaudio_Audio_HwpSetAtvAudioBand_ptr(void* fun_ptr)
{
    fpAudio_HwpSetAtvAudioBand = (Audio_HwpSetAtvAudioBand_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSetAtvAudioBand_ptr);
//////////////
typedef void (*Audio_AtvConfigNoSignalNeed2Mute_ptr)(uint32_t);
Audio_AtvConfigNoSignalNeed2Mute_ptr fpAudio_AtvConfigNoSignalNeed2Mute = NULL;
void rtkaudio_Audio_AtvConfigNoSignalNeed2Mute_ptr(void* fun_ptr)
{
    fpAudio_AtvConfigNoSignalNeed2Mute = (Audio_AtvConfigNoSignalNeed2Mute_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvConfigNoSignalNeed2Mute_ptr);
//////////////
typedef void (*Audio_AtvEnableAutoChangeStdFlag_ptr)(int32_t);
Audio_AtvEnableAutoChangeStdFlag_ptr fpAudio_AtvEnableAutoChangeStdFlag = NULL;
void rtkaudio_Audio_AtvEnableAutoChangeStdFlag_ptr(void* fun_ptr)
{
    fpAudio_AtvEnableAutoChangeStdFlag = (Audio_AtvEnableAutoChangeStdFlag_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvEnableAutoChangeStdFlag_ptr);
//////////////
typedef void (*Audio_AtvForceFirstTimeMonoSoundFlag_ptr)(int);
Audio_AtvForceFirstTimeMonoSoundFlag_ptr fpAudio_AtvForceFirstTimeMonoSoundFlag = NULL;
void rtkaudio_Audio_AtvForceFirstTimeMonoSoundFlag_ptr(void* fun_ptr)
{
    fpAudio_AtvForceFirstTimeMonoSoundFlag = (Audio_AtvForceFirstTimeMonoSoundFlag_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvForceFirstTimeMonoSoundFlag_ptr);
//////////////
typedef void (*Audio_AtvSetFwPriority_ptr)(uint16_t);
Audio_AtvSetFwPriority_ptr fpAudio_AtvSetFwPriority = NULL;
void rtkaudio_Audio_AtvSetFwPriority_ptr(void* fun_ptr)
{
    fpAudio_AtvSetFwPriority = (Audio_AtvSetFwPriority_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetFwPriority_ptr);
//////////////
typedef void (*Audio_Auto_Verify_Set_Path_ptr)(void);
Audio_Auto_Verify_Set_Path_ptr fpAudio_Auto_Verify_Set_Path = NULL;
void rtkaudio_Audio_Auto_Verify_Set_Path_ptr(void* fun_ptr)
{
    fpAudio_Auto_Verify_Set_Path = (Audio_Auto_Verify_Set_Path_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_Auto_Verify_Set_Path_ptr);
//////////////
typedef int32_t (*Audio_AtvGetSoundStd_ptr)(ATV_SOUND_INFO *);
Audio_AtvGetSoundStd_ptr fpAudio_AtvGetSoundStd = NULL;
void rtkaudio_Audio_AtvGetSoundStd_ptr(void* fun_ptr)
{
    fpAudio_AtvGetSoundStd = (Audio_AtvGetSoundStd_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvGetSoundStd_ptr);
//////////////
typedef void (*Audio_AtvSetDevOnOff_ptr)(int);
Audio_AtvSetDevOnOff_ptr fpAudio_AtvSetDevOnOff = NULL;
void rtkaudio_Audio_AtvSetDevOnOff_ptr(void* fun_ptr)
{
    fpAudio_AtvSetDevOnOff = (Audio_AtvSetDevOnOff_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetDevOnOff_ptr);
//////////////
typedef void (*Audio_AtvSetDevBandWidth_ptr)(A2_BW_SEL_T,A2_BW_SEL_T);
Audio_AtvSetDevBandWidth_ptr fpAudio_AtvSetDevBandWidth = NULL;
void rtkaudio_Audio_AtvSetDevBandWidth_ptr(void* fun_ptr)
{
    fpAudio_AtvSetDevBandWidth = (Audio_AtvSetDevBandWidth_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetDevBandWidth_ptr);
////////////////////////////////////////////////////////////////////////////////////////
//////////////
typedef int (*Audio_AtvGetNicamSignalStable_ptr)(void);
Audio_AtvGetNicamSignalStable_ptr fpAudio_AtvGetNicamSignalStable = NULL;
void rtkaudio_Audio_AtvGetNicamSignalStable_ptr(void* fun_ptr)
{
    fpAudio_AtvGetNicamSignalStable = (Audio_AtvGetNicamSignalStable_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvGetNicamSignalStable_ptr);
//////////////
typedef void (*Audio_AtvForceSoundSel_ptr)(int);
Audio_AtvForceSoundSel_ptr fpAudio_AtvForceSoundSel = NULL;
void rtkaudio_Audio_AtvForceSoundSel_ptr(void* fun_ptr)
{
    fpAudio_AtvForceSoundSel = (Audio_AtvForceSoundSel_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvForceSoundSel_ptr);
//////////////
typedef void (*Audio_AtvSetBTSCSoundSelect_ptr)(uint32_t,uint32_t);
Audio_AtvSetBTSCSoundSelect_ptr fpAudio_AtvSetBTSCSoundSelect = NULL;
void rtkaudio_Audio_AtvSetBTSCSoundSelect_ptr(void* fun_ptr)
{
    fpAudio_AtvSetBTSCSoundSelect = (Audio_AtvSetBTSCSoundSelect_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetBTSCSoundSelect_ptr);
//////////////
typedef void (*Audio_AtvSetA2SoundSelect_ptr)(uint32_t,uint32_t);
Audio_AtvSetA2SoundSelect_ptr fpAudio_AtvSetA2SoundSelect = NULL;
void rtkaudio_Audio_AtvSetA2SoundSelect_ptr(void* fun_ptr)
{
    fpAudio_AtvSetA2SoundSelect = (Audio_AtvSetA2SoundSelect_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetA2SoundSelect_ptr);
//////////////
typedef void (*Audio_AtvSetNICAMSoundSelect_ptr)(uint32_t,uint32_t);
Audio_AtvSetNICAMSoundSelect_ptr fpAudio_AtvSetNICAMSoundSelect = NULL;
void rtkaudio_Audio_AtvSetNICAMSoundSelect_ptr(void* fun_ptr)
{
    fpAudio_AtvSetNICAMSoundSelect = (Audio_AtvSetNICAMSoundSelect_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetNICAMSoundSelect_ptr);
//////////////
typedef void (*Audio_AtvSetFMoutDownGain_ptr)(int);
Audio_AtvSetFMoutDownGain_ptr fpAudio_AtvSetFMoutDownGain = NULL;
void rtkaudio_Audio_AtvSetFMoutDownGain_ptr(void* fun_ptr)
{
    fpAudio_AtvSetFMoutDownGain = (Audio_AtvSetFMoutDownGain_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetFMoutDownGain_ptr);
//////////////
typedef void (*Audio_HwpSIFGetNicamTH_ptr)(uint8_t *,uint8_t *);
Audio_HwpSIFGetNicamTH_ptr fpAudio_HwpSIFGetNicamTH = NULL;
void rtkaudio_Audio_HwpSIFGetNicamTH_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFGetNicamTH = (Audio_HwpSIFGetNicamTH_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFGetNicamTH_ptr);
//////////////
typedef void (*Audio_HwpSIFGetA2StereoDualTH_ptr)(uint32_t *);
Audio_HwpSIFGetA2StereoDualTH_ptr fpAudio_HwpSIFGetA2StereoDualTH = NULL;
void rtkaudio_Audio_HwpSIFGetA2StereoDualTH_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFGetA2StereoDualTH = (Audio_HwpSIFGetA2StereoDualTH_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFGetA2StereoDualTH_ptr);
//////////////
typedef void (*Audio_HwpSIFDetectedSoundStandard_ptr)(ATV_SOUND_STD *);
Audio_HwpSIFDetectedSoundStandard_ptr fpAudio_HwpSIFDetectedSoundStandard = NULL;
void rtkaudio_Audio_HwpSIFDetectedSoundStandard_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFDetectedSoundStandard = (Audio_HwpSIFDetectedSoundStandard_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFDetectedSoundStandard_ptr);
//////////////
typedef void (*Audio_HwpSIFCheckNicamDigital_ptr)(int *);
Audio_HwpSIFCheckNicamDigital_ptr fpAudio_HwpSIFCheckNicamDigital = NULL;
void rtkaudio_Audio_HwpSIFCheckNicamDigital_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFCheckNicamDigital = (Audio_HwpSIFCheckNicamDigital_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFCheckNicamDigital_ptr);
//////////////
typedef void (*Audio_HwpSIFSetNicamTH_ptr)(uint8_t,uint8_t);
Audio_HwpSIFSetNicamTH_ptr fpAudio_HwpSIFSetNicamTH = NULL;
void rtkaudio_Audio_HwpSIFSetNicamTH_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFSetNicamTH = (Audio_HwpSIFSetNicamTH_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFSetNicamTH_ptr);
//////////////
typedef void (*Audio_HwpSIFSetA2StereoDualTH_ptr)(uint32_t);
Audio_HwpSIFSetA2StereoDualTH_ptr fpAudio_HwpSIFSetA2StereoDualTH = NULL;
void rtkaudio_Audio_HwpSIFSetA2StereoDualTH_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFSetA2StereoDualTH = (Audio_HwpSIFSetA2StereoDualTH_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFSetA2StereoDualTH_ptr);
//////////////
typedef void (*Audio_HwpSetBandDelay_ptr)(void);
Audio_HwpSetBandDelay_ptr fpAudio_HwpSetBandDelay = NULL;
void rtkaudio_Audio_HwpSetBandDelay_ptr(void* fun_ptr)
{
    fpAudio_HwpSetBandDelay = (Audio_HwpSetBandDelay_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSetBandDelay_ptr);
//////////////
typedef void (*Audio_HwpSetChannelChange_ptr)(void);
Audio_HwpSetChannelChange_ptr fpAudio_HwpSetChannelChange = NULL;
void rtkaudio_Audio_HwpSetChannelChange_ptr(void* fun_ptr)
{
    fpAudio_HwpSetChannelChange = (Audio_HwpSetChannelChange_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSetChannelChange_ptr);
//////////////
typedef int32_t (*Audio_AtvGetCurrentDetectUpdate_ptr)(int *);
Audio_AtvGetCurrentDetectUpdate_ptr fpAudio_AtvGetCurrentDetectUpdate = NULL;
void rtkaudio_Audio_AtvGetCurrentDetectUpdate_ptr(void* fun_ptr)
{
    fpAudio_AtvGetCurrentDetectUpdate = (Audio_AtvGetCurrentDetectUpdate_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvGetCurrentDetectUpdate_ptr);
//////////////
typedef int32_t (*Audio_HwpSIFGetMainToneSNR_ptr)(ATV_SOUND_STD_MAIN_SYSTEM ,
								   ATV_SOUND_STD_MAIN_SYSTEM *,
								   ATV_SOUND_STD *,
								   uint32_t *);
Audio_HwpSIFGetMainToneSNR_ptr fpAudio_HwpSIFGetMainToneSNR = NULL;
void rtkaudio_Audio_HwpSIFGetMainToneSNR_ptr(void* fun_ptr)
{
    fpAudio_HwpSIFGetMainToneSNR = (Audio_HwpSIFGetMainToneSNR_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_HwpSIFGetMainToneSNR_ptr);
//////////////
typedef int32_t (*Audio_AtvSetSoundStd_ptr)(ATV_SOUND_STD_MAIN_SYSTEM,ATV_SOUND_STD);
Audio_AtvSetSoundStd_ptr fpAudio_AtvSetSoundStd = NULL;
void rtkaudio_Audio_AtvSetSoundStd_ptr(void* fun_ptr)
{
    fpAudio_AtvSetSoundStd = (Audio_AtvSetSoundStd_ptr)fun_ptr;
}
EXPORT_SYMBOL(rtkaudio_Audio_AtvSetSoundStd_ptr);

char Err_Status[4][16] = {
    "DRIVER_ERROR",
    "FD_NULL",
    "ADDR_NULL",
    "TYPE_ERROR",
};

typedef struct
{
    char name[12];
    int connectStatus[10];
    int inputConnectResourceId[10]; //HAL_AUDIO_RESOURCE_T
    int outputConnectResourceID[10];//HAL_AUDIO_RESOURCE_T
    int numOptconnect;
    int numIptconnect;
    int maxVaildIptNum; // output moduel used

} AUDIO_MODULE_STATUS;

typedef struct
{
    unsigned char IsAINExist;
    int AinConnectDecIndex;
    unsigned char IsSubAINExist;
    int subAinConnectDecIndex;
    unsigned char IsDEC0Exist;
    unsigned char IsDEC1Exist;
    unsigned char IsDEC2Exist;
    unsigned char IsDEC3Exist;
    unsigned char IsEncExist;
    unsigned char IsScartOutBypas;
    int ScartConnectDecIndex;
    int mainDecIndex;
    unsigned char IsSystemOutput;
    unsigned char IsDTV0SourceRead; // ATP
    unsigned char IsDTV1SourceRead;
    unsigned char IsMainPPAOExist;
    unsigned char IsSubPPAOExist;
} AUDIO_FLOW_STATUS;

typedef struct
{
    int ainPinStatus[9];
} AUDIO_AIN_MODULE_STATUS;

typedef struct
{
    int SEPinStatus;
} AUDIO_SE_MODULE_STATUS;

typedef struct
{
    UINT32 decMute;
    UINT32 decDelay;
    UINT32 decVol[8];
} ADSP_DEC_INFO;

 /************************************************************************
 *  Function body
 ************************************************************************/

/* read from kernel driver with ioctl */
int rtkaudio_ioctl_get(int cntl_type, void *addr, int size)
{
    if(cntl_type >= MAX_CONTROL_NUM) {
        return IOCTL_TYPE_ERROR;
    }

    switch(cntl_type)
    {
    case VIRTUALX_CMD_INFO:
        return RTKAUDIO_RPC_TOAGENT_VX_PRIVATEINFO_SVC((virtualx_cmd_info*)addr);
        break;
    case MAIN_DECODER_STATUS:
        break;
    case ALSA_DEV_INFO:
        break;
    case GST_DEBUG_INFO:
        break;
    default:
        break;
    }
    return 0;
}

/* write to kernel driver with ioctl */
int rtkaudio_ioctl_set(int cntl_type, void *addr, int size)
{
    if(cntl_type >= MAX_CONTROL_NUM) return IOCTL_TYPE_ERROR;

    switch(cntl_type)
    {
    case VIRTUALX_CMD_INFO:
        return RTKAUDIO_RPC_TOAGENT_VX_PRIVATEINFO_SVC((virtualx_cmd_info*)addr);
    case MAIN_DECODER_STATUS:
        break;
    case ALSA_DEV_INFO:
        break;
    case GST_DEBUG_INFO:
        break;
    case SET_SWP_SRS_TRUSURROUNDHD:
        return RTKAUDIO_RPC_TOAGENT_SET_PP_TSXT_CONFIG_SVC((AUDIO_RPC_TSXT *)addr);
    default:
        break;
    }
    return 0;
}

KADP_STATUS_T  KADP_Audio_HwpSIFGetMainToneSNR(ATV_SOUND_STD_MAIN_SYSTEM main_sys,
											   ATV_SOUND_STD_MAIN_SYSTEM *p_hw_detected_main_sys,
											   ATV_SOUND_STD *p_hw_detected_sound_std,
											   uint32_t *pToneSNR)
{

	if(NULL == p_hw_detected_main_sys)
	{
		AUDIO_ERROR("\n%s(L%d):p_hw_detected_main_sys is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}
	if(NULL == pToneSNR)
	{
		AUDIO_ERROR("\n%s(L%d):pToneSNR is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}

    fpAudio_HwpSIFGetMainToneSNR(main_sys,    p_hw_detected_main_sys, p_hw_detected_sound_std, pToneSNR);
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetAVSyncOffset(AUDIO_HDMI_OUT_VSDB_DATA *info)
{
    unsigned long rpc_res;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AVSYNC_OFFSET_SVC(info);
    if(rpc_res != S_OK)
        return KADP_ERROR;

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetAudioSpectrumData(AUDIO_SPECTRUM_CFG *config)
{
    unsigned long rpc_res;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SEND_SPECTRUM_DATA_SVC(config);
    if(rpc_res != S_OK)
        return KADP_ERROR;

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetAudioOptChannelSwap(AUDIO_AO_CHANNEL_OUT_SWAP sel)
{
    AUDIO_RPC_AO_CHANNEL_OPT_SWAP cswap;
    unsigned long rpc_res;

    cswap.instanceID = -1;
    cswap.channelSwapIndex = sel;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_CHANNEL_OUT_SWAP_SVC(&cswap);
    if(rpc_res != S_OK)
        return KADP_ERROR;

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetAudioAuthorityKey(long customer_key)
{
    unsigned long rpc_res;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUTHORITY_KEY_SVC(&customer_key);
    if(rpc_res != S_OK)
        return KADP_ERROR;

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SwpSetSRS_TrusurroundHD(AUDIO_RPC_TSXT* pTSXT)
{
    int ret;
    if (pTSXT == NULL)
    {
        AUDIO_ERROR("[%s] pointer is NULL\n", __FUNCTION__);
        return KADP_ERROR;
    }

    ret = rtkaudio_ioctl_set(SET_SWP_SRS_TRUSURROUNDHD, (void *)pTSXT, (int)sizeof(AUDIO_RPC_TSXT));
    if(ret < 0) {
        AUDIO_ERROR("[%s] failed with rtkaudio_ioctl_set\n",__FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvGetCurrentDetectUpdate(int *isComplete)
{
	if(NULL == isComplete)
	{
		AUDIO_ERROR("\n%s(L%d):isComplete is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}

    if(fpAudio_AtvGetCurrentDetectUpdate(isComplete))
    {
        AUDIO_ERROR("[%s %d] %s - ioctl fail\n", __FILE__, __LINE__, __func__);
        return KADP_ERROR;
    }

    return KADP_OK;
}

void KADP_Audio_PrintSemBacktrace(void)
{
#ifdef ENABLE_AUDIO_BACKTRACE_DUMP
    int SIZE = 10240;
    int shm_fd = shm_open(SEM_HAL_AUDIO_BT, O_RDONLY, 0666);
    if(shm_fd >= 0) {
        char* shm_base = (char*)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (shm_base == MAP_FAILED) {
            KADP_AUDIO_DEBUG("mmap() failed: %s\n", strerror(errno));
        } else {
            KADP_AUDIO_DEBUG("%s", shm_base);
            munmap(shm_base, SIZE);
        }
        close(shm_fd);
    } else {
        KADP_AUDIO_DEBUG("======== Audio semaphore backtrace not exit (no lock) ======== \n");
    }
    KADP_AUDIO_DEBUG("========== Audio sem backtrace END ==========\n\n");
#endif
}

KADP_STATUS_T KADP_Audio_AtvGetNicamSignalStable(int *nicam_stable)
{
    int isStable = 0;

    if(NULL == nicam_stable)
    {
        AUDIO_ERROR("\n%s(L%d):nicam_stable is NULL!!!\n", __FUNCTION__, __LINE__);
        return KADP_ERROR;
    }

    isStable = fpAudio_AtvGetNicamSignalStable();

    *nicam_stable = isStable;
    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFGetNicamTH(uint8_t  *p_hi_th,  uint8_t *p_lo_th)
{

    //ATV_HWP_SIF_GET_NICAM_TH_T nicam_th;

    if(NULL == p_hi_th)
    {
        AUDIO_ERROR("\n%s(L%d):p_hi_th is NULL!!!\n", __FUNCTION__, __LINE__);
        return KADP_ERROR;
    }
    if(NULL == p_lo_th)
    {
        AUDIO_ERROR("\n%s(L%d):p_lo_th is NULL!!!\n", __FUNCTION__, __LINE__);
        return KADP_ERROR;
    }

    //nicam_th.hi_th = *p_hi_th;
    //nicam_th.lo_th = *p_lo_th;

    fpAudio_HwpSIFGetNicamTH(p_hi_th, p_lo_th);// get from Audio_HwpSIFGetNicamTH

    //*p_hi_th = nicam_th.hi_th;
    //*p_lo_th = nicam_th.lo_th;

    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFGetA2StereoDualTH(uint32_t *p_a2_th)
{
	if(NULL == p_a2_th)
	{
		AUDIO_ERROR("\n%s(L%d):p_a2_th is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}

    fpAudio_HwpSIFGetA2StereoDualTH(p_a2_th);

    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFDetectedSoundStandard(ATV_SOUND_STD *p_detected_sound_std)
{
	if(NULL == p_detected_sound_std)
	{
		AUDIO_ERROR("\n%s(L%d):p_detected_sound_std is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}

    fpAudio_HwpSIFDetectedSoundStandard(p_detected_sound_std);
    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFCheckNicamDigital(int *p_exist_flag)
{
	if(NULL == p_exist_flag)
	{
		AUDIO_ERROR("\n%s(L%d):p_exist_flag is NULL!!!\n", __FUNCTION__, __LINE__);
		return KADP_ERROR;
	}

    fpAudio_HwpSIFCheckNicamDigital(p_exist_flag);
    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFSetNicamTH(uint8_t  hi_th,  uint8_t lo_th)
{
	//ATV_HWP_SIF_SET_NICAM_TH_T nicam_th;
	//nicam_th.hi_th = hi_th;
	//nicam_th.lo_th = lo_th;

    fpAudio_HwpSIFSetNicamTH(hi_th, lo_th);
    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSIFSetA2StereoDualTH(uint32_t a2_th)
{
    fpAudio_HwpSIFSetA2StereoDualTH(a2_th);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvForceSoundSel(int flag)
{
    fpAudio_AtvForceSoundSel(flag);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetBTSCSoundSelect(uint32_t isPreferPlayStereo, uint32_t  isPreferPlaySap)
{
	//ATV_SET_BTSC_SOUND_SELECT_T btsc_sound_sel;
	//btsc_sound_sel.play_stereo = isPreferPlayStereo;
	//btsc_sound_sel.play_sap = isPreferPlaySap;

    fpAudio_AtvSetBTSCSoundSelect(isPreferPlayStereo, isPreferPlaySap);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetA2SoundSelect(uint32_t isPreferPlayStereo, uint32_t  preferDualLangIndex)
{
	//ATV_SET_A2_SOUND_SELECT_T a2_sound_sel;
	///a2_sound_sel.play_stereo = isPreferPlayStereo;
	///a2_sound_sel.dual_lang_index = preferDualLangIndex;

    fpAudio_AtvSetA2SoundSelect(isPreferPlayStereo, preferDualLangIndex);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetNICAMSoundSelect(uint32_t isPreferNicamDigital, uint32_t  preferDualLangIndex)
{
	//ATV_SET_NICAM_SOUND_SELECT_T nciam_sound_sel;
	//nciam_sound_sel.nicam_digital = isPreferNicamDigital;
	//nciam_sound_sel.dual_lang_index = preferDualLangIndex;

    fpAudio_AtvSetNICAMSoundSelect(isPreferNicamDigital, preferDualLangIndex);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetSoundStd(ATV_SOUND_STD_MAIN_SYSTEM main_system, ATV_SOUND_STD sound_std)
{
	//ATV_SET_SOUND_STD_T set_sound_std;
	//set_sound_std.main_system = main_system;
	//set_sound_std.sound_std = sound_std;

	if (fpAudio_AtvSetSoundStd(main_system, sound_std)) {
        AUDIO_ERROR("%s return fail\n", __FUNCTION__);
		return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpSetBandDelay(void)
{
    fpAudio_HwpSetBandDelay();
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpSetChannelChange(void)
{
    fpAudio_HwpSetChannelChange();
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetFMoutDownGain(int para)
{
    fpAudio_AtvSetFMoutDownGain(para);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetDevBandWidth(A2_BW_SEL_T bw_sel, A2_BW_SEL_T sub_bw_sel)
{
	//ATV_SET_DEV_BANDWIDTH_T dev_bandwidth;
	//dev_bandwidth.bw_sel = bw_sel;
	//dev_bandwidth.sub_bw_sel = sub_bw_sel;

    fpAudio_AtvSetDevBandWidth(bw_sel, sub_bw_sel);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetDevOnOff(int bOnOff)
{
    fpAudio_AtvSetDevOnOff(bOnOff);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvGetSoundStd(ATV_SOUND_INFO *p_sound_info)
{
    if (fpAudio_AtvGetSoundStd(p_sound_info)) {
        AUDIO_ERROR("%s return fail\n", __FUNCTION__);
        return KADP_ERROR;
    }
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_ATV_VERIFY_SET_PATH(void)
{
    fpAudio_Auto_Verify_Set_Path();
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetFwPriority(uint16_t Priority)
{
    fpAudio_AtvSetFwPriority(Priority);
	return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvForceFirstTimeMonoSoundFlag(int isEnable)
{
    fpAudio_AtvForceFirstTimeMonoSoundFlag(isEnable);
	return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvEnableAutoChangeStdFlag(int enable)
{
    fpAudio_AtvEnableAutoChangeStdFlag(enable);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvConfigNoSignalNeed2Mute(uint32_t isNeed2Mute)
{
    fpAudio_AtvConfigNoSignalNeed2Mute(isNeed2Mute);
	return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpSetAtvAudioBand(SIF_INPUT_SOURCE sif_source, ATV_SOUND_STD_MAIN_SYSTEM atv_main_system)
{
	//ATV_HWP_SET_ATV_AUDIO_BAND_T audio_band;
	//audio_band.sif_source= sif_source;
	//audio_band.atv_main_system= atv_main_system;

    if(fpAudio_HwpSetAtvAudioBand((SIF_INPUT_SOURCE)sif_source, (ATV_SOUND_STD_MAIN_SYSTEM)atv_main_system))
    {
        AUDIO_ERROR("[%s %d] %s - ioctl fail\n", __FILE__, __LINE__, __func__) ;
        return KADP_ERROR;
    }
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetMtsPriority(ATV_MTS_PRIORITY  mts_prio)
{

    //ATV_MTS_PRIORITY mts_priority;
    //mts_priority = mts_prio;

    fpAudio_AtvSetMtsPriority((ATV_MTS_PRIORITY)mts_prio);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetScanStdFlag(bool Enable)
{
    fpAudio_Hw_Set_Scan_Std_Flag(Enable);
    return KADP_OK;
}

KADP_STATUS_T  KADP_Audio_HwpSetSIFDataSource(SIF_INPUT_SOURCE kadp_sif_input_source)
{
    fpAudio_HwpSetSIFDataSource((SIF_INPUT_SOURCE)kadp_sif_input_source);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvStartDecoding(void)
{
    fpAudio_AtvStartDecoding();
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvEnterAtvSource(void)
{
    fpAudio_AtvEnterAtvSource();
    return KADP_OK;
}

int KADP_SDEC_DisConnect(DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T con_dest)
{
    DEMUX_BUF_INFO_T bufInfo;
    int ret = KADP_OK;
    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.ch = ch;
    bufInfo.dest = (con_dest == RSDEC_ENUM_TYPE_DEST_VDEC0) ? DEMUX_DEST_VDEC0 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_VDEC1) ? DEMUX_DEST_VDEC1 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ADEC0) ? DEMUX_DEST_ADEC0 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ADEC1) ? DEMUX_DEST_ADEC1 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_SUBTITLE) ? DEMUX_DEST_SUBTITLE :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_TELETEXT) ? DEMUX_DEST_TELETEXT :
	/*To consist with kadp_sdec_connect in ANDROIDTV, extend partial enum to fit SINA artitecture*/
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ISDB_CC) ? DEMUX_DEST_ISDB_CC :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ISDB_CC_SUPERIMPOSE) ? DEMUX_DEST_ISDB_CC_SUPERIMPOSE :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_STC0) ? DEMUX_DEST_STC0 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_STC1) ? DEMUX_DEST_STC1 :
	/* ----------------------------------------------------------------------------------------- */
					DEMUX_DEST_NONE)))))))));
    AUDIO_INFO("func %s, ch %d, dest %d\n", __func__, bufInfo.ch, bufInfo.dest);

//    ret = IOCTL_ReleaseBuffer(&bufInfo);
    ret = IOCTLEX_ReleaseBuffer(&bufInfo);

    if(ret < 0) {
        AUDIO_ERROR("%s return IOCTLEX_ReleaseBuffer fail \n", __FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;

}

int KADP_SDEC_Connect(DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T con_dest, DEMUX_STRUCT_CONNECT_RESULT_T *pBufInfo)
{
    DEMUX_BUF_INFO_T bufInfo;

    if(pBufInfo == 0)
    {
        AUDIO_ERROR("[%s %d] invalid argument\n", __FILE__, __LINE__) ;
        return -1;
    }

    memset(&bufInfo, 0, sizeof(bufInfo));
    bufInfo.ch = ch;
    bufInfo.dest = (con_dest == RSDEC_ENUM_TYPE_DEST_VDEC0) ? DEMUX_DEST_VDEC0 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_VDEC1) ? DEMUX_DEST_VDEC1 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ADEC0) ? DEMUX_DEST_ADEC0 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_ADEC1) ? DEMUX_DEST_ADEC1 :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_SUBTITLE) ? DEMUX_DEST_SUBTITLE :
                   ((con_dest == RSDEC_ENUM_TYPE_DEST_TELETEXT) ? DEMUX_DEST_TELETEXT : DEMUX_DEST_NONE)))));
    AUDIO_INFO("func %s, ch %d, dest %d\n", __func__, bufInfo.ch, bufInfo.dest);
    if(bufInfo.dest == DEMUX_DEST_NONE || IOCTL_GetBufferInfo(&bufInfo) < 0)
    {
        AUDIO_ERROR("[%s %d] %s - ioctl fail. dest = %d\n", __FILE__, __LINE__, __func__, bufInfo.dest) ;
        return KADP_ERROR;
    }

    pBufInfo->bsRingHeader = bufInfo.bsPhyAddr;
    pBufInfo->ibRingHeader = bufInfo.ibPhyAddr;
    pBufInfo->refClock     = bufInfo.refClockPhyAddr;
    pBufInfo->bsHeaderSize = bufInfo.bsHeaderSize;
    pBufInfo->ibHeaderSize = bufInfo.ibHeaderSize;
    pBufInfo->refClockHeaderSize = bufInfo.refClockHeaderSize;

//    if (con_dest == RSDEC_ENUM_TYPE_DEST_VDEC0) KADP_SDEC_SetESDump(ch, con_dest, 1, "/tmp/usbmounts/sda1/recordES.video") ;
//    if (con_dest == RSDEC_ENUM_TYPE_DEST_ADEC0) KADP_SDEC_SetESDump(ch, con_dest, 1, "/tmp/usbmounts/sda1/recordES.audio") ;
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpAADSIFFinalize(void)
{
    unsigned long rpc_res;
    rpc_res = fpAudio_HwpAADSIFFinalize();
    if(rpc_res != 0) {
        ERROR("%s return fail\n", __FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvInit(ATV_CFG *p_cfg)
{
    fpAudio_AtvInit(p_cfg);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvEnableAutoChangeSoundModeFlag(int isEnable)
{
    fpAudio_AtvEnableAutoChangeSoundModeFlag(isEnable);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpCurSifType(ATV_AUDIO_SIF_TYPE_T eSifType)
{
    fpAudio_HwpCurSifType(eSifType);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_HwpAADSIFInit(ATV_CFG *p_cfg)
{
    ATV_CFG atv_cfg = {0};

    if(p_cfg)
        atv_cfg = *p_cfg;

    fpAudio_HwpAADSIFInit(&atv_cfg);

    if(p_cfg)
        *p_cfg = (ATV_CFG)atv_cfg;

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_GetAMixerRunningStatus(UINT32* status)
{
    AUDIO_REGISTER_ACCESS_T info;

    info.reg_type = AMIXER_STATUS;
    info.reg_value = 0; /* no used */
    info.data = 0;      /* no used */

    *status = read_register_by_type(&info);

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetSpdifOutPinSrc(AUDIO_SPDIFO_SOURCE src)
{
    unsigned long rpc_res;
    AUDIO_RPC_SPDIFO_SOURCE aSpdifSrc;

    aSpdifSrc.instanceID = -1;// do not care
    aSpdifSrc.source = src;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_SPDIF_OUT_PIN_SRC_SVC(&aSpdifSrc);
    if(rpc_res != S_OK) {
        ERROR("%s return fail\n", __FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetTruVolume(AUDIO_RPC_TRUVOLUME* info)
{
    unsigned long rpc_res;

    if (info == NULL)
    {
        AUDIO_ERROR("[%s] pointer is NULL\n", __FUNCTION__);
        return KADP_ERROR;
    }

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_TRUVOLUME_SVC(info);
    if(rpc_res != S_OK) {
        ERROR("%s return fail\n", __FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;

}

KADP_STATUS_T KADP_AUDIO_SetModelHash(AUDIO_DUMMY_DATA model_hash)
{
    unsigned long rpc_res;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_MODEL_HASH_SVC(&model_hash);
    if(rpc_res != S_OK) {
        ERROR("%s return fail\n", __FUNCTION__);
        return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_AudioConfig(AUDIO_CONFIG_COMMAND_RTKAUDIO *audioConfig)
{
    rtkaudio_send_audio_config(audioConfig);
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetTruvolumdHDParam(AUDIO_TRUVOLUMEHD_PARAM_INFO *info)
{
    //not implement yet
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_GetTruvolumdHDParam(AUDIO_TRUVOLUMEHD_PARAM_INFO *info)
{
    //not implement yet
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetAudioCapability(AUDIO_LICENSE_STATUS_CONFIG license_config)
{
    unsigned long rpc_res;

    rpc_res = RTKAUDIO_RPC_TOAGENT_SET_AUDIO_CAPABILITY_SVC(&license_config);
    if(rpc_res != S_OK)
        return KADP_ERROR;

    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvStopDecoding(void)
{
    fpAudio_AtvStopDecoding();
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvPauseTvStdDetection(int tv_status)
{
    fpAudio_AtvPauseTVStdDetection(tv_status);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvCleanTVSourceData(void)
{
    fpAudio_AtvCleanTVSourceData();
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_HW_IOCT_Init(int32_t config)
{
    gUSER_CONFIG = config;
    fpAudio_SetAudioHWConfig(config);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_SetFmRadioMode(AUDIO_ATV_FM_RADIO_MODE Enable)
{
    fpAudio_Hw_SetFmRadioMode(Enable);
    return KADP_OK;
}

int KADP_SDEC_PrivateInfo (DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T dest, DEMUX_PRIVATEINFO__T infoId, ULONG *pInfoData, ULONG infoSize)
{
    int ret = 0;
    DEMUX_PRIVATE_INFO_T info ;

    if ((!pInfoData && infoSize > 0) || (infoSize > sizeof(ULONG)*9))
    {
        AUDIO_ERROR("func %s, line %d, InfoData(%p,%d) is fail!!\n", __func__, __LINE__, pInfoData, (UINT32)infoSize) ;
        return -1 ;
    }

    //KADP_CHECK_CH_PARAM_VALID(ch) ;

    memset(&info, 0, sizeof(info));
    info.ch   = ch;
    info.dest =  (dest == RSDEC_ENUM_TYPE_DEST_VDEC0) ? DEMUX_DEST_VDEC0 :
                ((dest == RSDEC_ENUM_TYPE_DEST_VDEC1) ? DEMUX_DEST_VDEC1 :
                ((dest == RSDEC_ENUM_TYPE_DEST_ADEC0) ? DEMUX_DEST_ADEC0 : DEMUX_DEST_ADEC1)) ;
    info.data[0] = (ULONG) infoId ;
    AUDIO_INFO("func %s, ch %d, dest %d, infoId %d\n", __func__, info.ch, info.dest, infoId);
    if (infoSize) memcpy(&info.data[1], pInfoData, infoSize) ;

    ret = IOCTL_SetPrivateInfo(&info);
    if (ret < 0)
        AUDIO_ERROR("IOCTL_SetPrivateInfo fail\n");

    return ret ;
}

KADP_STATUS_T KADP_Audio_AtvSetDeviationMethod(ATV_DEV_METHOD kadp_dev_method)
{
    fpAudio_AtvSetDeviationMethod(kadp_dev_method);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetAudioInHandle(long instanceID)
{
    fpAudio_AtvSetAudioInHandle(instanceID);
    return KADP_OK;
}

KADP_STATUS_T KADP_Audio_AtvSetSubAudioInHandle(long instanceID)
{
    fpAudio_AtvSetSubAudioInHandle(instanceID);
    return KADP_OK;
}

#define AUDIO_RESOURCE_ATP0 2
#define AUDIO_RESOURCE_ATP1 3

KADP_STATUS_T KADP_AUDIO_GetTrickState(UINT32 atp_index, char* ret)
{
    if(atp_index < AUDIO_RESOURCE_ATP0 || atp_index > AUDIO_RESOURCE_ATP1)
    {
        AUDIO_ERROR("[%s] atp_index[%d] is out of range.\n", __FUNCTION__, atp_index);
        return KADP_ERROR;
    }
    if (ret == NULL)
    {
        AUDIO_ERROR("[%s] ret is NULL\n", __FUNCTION__);
        return KADP_ERROR;
    }

    *ret = Aud_trickPlayState[atp_index-AUDIO_RESOURCE_ATP0];
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_ResetTrickState(UINT32 atp_index)
{
    if(atp_index < AUDIO_RESOURCE_ATP0 || atp_index > AUDIO_RESOURCE_ATP1)
    {
        AUDIO_DEBUG("[%s] atp_index[%d] is out of range.\n", __FUNCTION__, atp_index);
        return KADP_ERROR;
    }

    Aud_trickPlayState[atp_index-AUDIO_RESOURCE_ATP0] = 0;
    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_PrivateInfo(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS *parameter, AUDIO_RPC_PRIVATEINFO_RETURNVAL *ret)
{
    AUDIO_RPC_PRIVATEINFO_RETURNVAL result;
    UINT32 res;

    if ((ret == NULL) || (parameter == NULL))
    {
        AUDIO_ERROR("[%s] pointer is NULL\n", __FUNCTION__);
        return KADP_ERROR;
    }

    res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(parameter, &result);
    if(res != S_OK){
        ERROR("[%s:%d] RPC return != S_OK\n",__func__,__LINE__);
        return KADP_ERROR;
    }
    else
    {
        memcpy(ret, &result, sizeof(AUDIO_RPC_PRIVATEINFO_RETURNVAL));
        return KADP_OK;
    }
}

KADP_STATUS_T KADP_AUDIO_GetAudioSpdifChannelStatus(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC *cs, AUDIO_MODULE_TYPE type)
{
    UINT32 res;
    AUDIO_SPDIF_CS spdif_cs = {0};

    if (cs == NULL)
    {
        AUDIO_ERROR("[%s] pointer is NULL\n", __FUNCTION__);
        return KADP_ERROR;
    }

    if((type != AUDIO_OUT) && (type != AUDIO_IN))
    {
        AUDIO_ERROR("[%s,%d] error module type\n", __FUNCTION__, __LINE__);
        memset(cs, 0, sizeof(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC));
        return KADP_ERROR;
    }
    spdif_cs.module_type = type;

    res = RTKAUDIO_RPC_TOAGENT_GET_SPDIF_CS_SVC(&spdif_cs);
    if(res != S_OK) ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    if(res == S_OK)
    {
        cs->category_code          = spdif_cs.category_code;
        cs->professional           = spdif_cs.professional;
        cs->copyright              = spdif_cs.copyright;
        cs->channel_number         = spdif_cs.channel_number;
        cs->source_number          = spdif_cs.source_number;
        cs->sampling_freq          = spdif_cs.sampling_freq;
        cs->clock_accuracy         = spdif_cs.clock_accuracy;
        cs->word_length            = spdif_cs.word_length;
        cs->original_sampling_freq = spdif_cs.original_sampling_freq;
        cs->cgms_a                 = spdif_cs.cgms_a;
        cs->pre_emphasis           = spdif_cs.pre_emphasis;
        cs->data_type              = spdif_cs.data_type;
        cs->mode                   = spdif_cs.mode;
    } else {
        return KADP_ERROR;
    }

    return KADP_OK;
}

static KADP_STATUS_T KADP_AUDIO_SetVirtualXParam(virtualx_cmd_info *info)
{
    int ret = rtkaudio_ioctl_set(VIRTUALX_CMD_INFO, (void*)info, sizeof(*info));

    if(ret < 0) {
        AUDIO_ERROR("[%s] failed with %s\n",__FUNCTION__,Err_Status[-(ret+1)]);

        return KADP_ERROR;
    }

    return KADP_OK;
}

static KADP_STATUS_T KADP_AUDIO_GetVirtualXParam(virtualx_cmd_info *info)
{
    int ret = rtkaudio_ioctl_get(VIRTUALX_CMD_INFO, (void*)info, sizeof(*info));

    if(ret < 0) {
        AUDIO_ERROR("[%s] failed with %s\n",__FUNCTION__,Err_Status[-(ret+1)]);

        return KADP_ERROR;
    }

    return KADP_OK;
}

KADP_STATUS_T KADP_AUDIO_SetTruSrndXParam(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    //virx_cmd.index = index;
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_TRUSRNDX_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetTruSrndXParam(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    //virx_cmd.index = index;
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_TRUSRNDX_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_SetTbhdXParam(AUDIO_VIRTUALX_TBHDX_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_TBHDX_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetTbhdXParam(AUDIO_VIRTUALX_TBHDX_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_TBHDX_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_SetMbhlParam(AUDIO_VIRTUALX_MBHL_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_MBHL_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetMbhlParam(AUDIO_VIRTUALX_MBHL_PARAM_INFO *param)
{
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_MBHL_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK)
    {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;
    }
    return ret;
}

KADP_STATUS_T KADP_AUDIO_SetLoudnessControlParam(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_LOUDNESS_CONTROL_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK) {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;

    }

    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetLoudnessControlParam(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_LOUDNESS_CONTROL_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
   if (ret == KADP_OK) {
       if(0 == virx_cmd.result)
           memcpy(param, &virx_cmd.data, sizeof(*param));
       else
           ret = KADP_ERROR;
   }

   return ret;
}

KADP_STATUS_T KADP_AUDIO_SetGeqParam(AUDIO_VIRTUALX_Geq10b_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_GEQ_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK) {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;

    }

    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetGeqParam(AUDIO_VIRTUALX_Geq10b_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_GEQ_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
   if (ret == KADP_OK) {
       if(0 == virx_cmd.result)
           memcpy(param, &virx_cmd.data, sizeof(*param));
       else
           ret = KADP_ERROR;
   }

   return ret;
}

KADP_STATUS_T KADP_AUDIO_SetAeqParam(AUDIO_VIRTUALX_Aeq_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_AEQ_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK) {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;

    }

    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetAeqParam(AUDIO_VIRTUALX_Aeq_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_AEQ_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
   if (ret == KADP_OK) {
       if(0 == virx_cmd.result)
           memcpy(param, &virx_cmd.data, sizeof(*param));
       else
           ret = KADP_ERROR;
   }

   return ret;
}

KADP_STATUS_T KADP_AUDIO_SetMultiRateParam(AUDIO_VIRTUALX_MultiRate_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_MULTI_RATE_SETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_SetVirtualXParam(&virx_cmd);
    if (ret == KADP_OK) {
        if(0 == virx_cmd.result)
            memcpy(param, &virx_cmd.data, sizeof(*param));
        else
            ret = KADP_ERROR;

    }

    return ret;
}

KADP_STATUS_T KADP_AUDIO_GetMultiRateParam(AUDIO_VIRTUALX_MultiRate_PARAM_INFO *param){
    KADP_STATUS_T ret;

    virtualx_cmd_info virx_cmd;

    memset(&virx_cmd, 0, sizeof(virtualx_cmd_info));
    virx_cmd.type= ENUM_VIRTUALX_CMD_TYPE_MULTI_RATE_GETPARAM;
    virx_cmd.size= sizeof(*param);
    memcpy(&virx_cmd.data, param, sizeof(*param));

    ret = KADP_AUDIO_GetVirtualXParam(&virx_cmd);
   if (ret == KADP_OK) {
       if(0 == virx_cmd.result)
           memcpy(param, &virx_cmd.data, sizeof(*param));
       else
           ret = KADP_ERROR;
   }

   return ret;
}

