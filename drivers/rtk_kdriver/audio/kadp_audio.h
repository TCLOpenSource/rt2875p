#ifndef _KADP_AUDIO_H_
#define _KADP_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 *  Include files
 ************************************************************************/
#include "audio_inc.h"
#include <rtk_kdriver/rtkaudio.h>

#include "audio_hw_port.h"
#include "audio_hw_atv.h"
#include "audio_hw_aio.h"
#include "audio_hw_app.h"
#include "audio_hw_driver.h"
#include "audio_hw_ioctl.h"

/************************************************************************
 *  Definitions
 ************************************************************************/

#define AIN_VOL_0DB             0
#define AIN_VOL_STEP_MIN        1
#define AIN_VOL_STEP_1DB        2 /* 0.5dB per step */
#define AIN_MAX_VOL             ((30)*AIN_VOL_STEP_1DB)
#define AIN_MIN_VOL             (-70*AIN_VOL_STEP_1DB)

#ifndef KADP_STATUS_T
typedef enum
{
    KADP_OK                     =   0,
    KADP_ERROR                  =   -1,
    KADP_NOT_OK                 =   -1,
    KADP_PARAMETER_ERROR        =   -2,
    KADP_NOT_ENOUGH_RESOURCE    =   -3,
    KADP_NOT_SUPPORTED          =   -4,
    KADP_NOT_PERMITTED          =   -5,
    KADP_TIMEOUT                =   -6,
    KADP_NO_DATA_RECEIVED       =   -7,
    KADP_DN_BUF_OVERFLOW        =   -8,
    KADP_DLNA_NOT_CONNECTED     =   -9,
    KADP_UP_BUF_OVERFLOW        =   -10,
    KADP_FD_NOT_EXIST           =   -11,
} _KADP_STATUS_T;

#define KADP_STATUS_T _KADP_STATUS_T
#endif

KADP_STATUS_T AudioSW_EQ_BASS_SetEnable(ENUM_AUDIO_EQ_BASS_MODE mode, int eq_band, int OnOff);
KADP_STATUS_T AudioSW_EQ_BASS_SetParam(ENUM_AUDIO_EQ_BASS_MODE mode, int eq_band, ENUM_AUDIO_EQ_BASS_FILTER_TYPE type, double fc, double bw, double gain);
KADP_STATUS_T AudioSW_EQ_BASS_Set_AudioDataPath(ENUM_AUDIO_EQ_BASS_MODE mode, int OnOff);
KADP_STATUS_T KADP_AUDIO_EQ_BASS_SET_IOCTL(AUDIO_EQ_BASS_PARAM_INFO *param);

typedef enum {
    KADP_AUDIO_CH_ID_L    = (0x1<<0),
    KADP_AUDIO_CH_ID_R    = (0x1<<1),
    KADP_AUDIO_CH_ID_LS   = (0x1<<2),
    KADP_AUDIO_CH_ID_RS   = (0x1<<3),
    KADP_AUDIO_CH_ID_C    = (0x1<<4),
    KADP_AUDIO_CH_ID_SW   = (0x1<<5),
    KADP_AUDIO_CH_ID_LSS  = (0x1<<6),
    KADP_AUDIO_CH_ID_RSS  = (0x1<<7),
    KADP_AUDIO_CH_ID_ALL = 0xFF
} KADP_AUDIO_CH_ID;

typedef enum  {
    KADP_AUDIO_DELAY_RAW,
    KADP_AUDIO_DELAY_SPEAKER,
    KADP_AUDIO_DELAY_FIXVOL,
    KADP_AUDIO_DELAY_MAX
} KADP_AUDIO_DELAY_MODE;


typedef enum  {
	 KADP_AUDIO_SPDIFO_SRC_FIFO = 0x0,
	 KADP_AUDIO_SPDIFO_SRC_IN = 0x1,
	 KADP_AUDIO_SPDIFO_SRC_HDMI = 0x2,
	 KADP_AUDIO_SPDIFO_SRC_DISABLE = 0x3,
}KADP_AUDIO_SPDIFO_SOURCE;


typedef union {
	struct {
		unsigned char      volume: 7;
		unsigned char      bMute: 1;
	} OSD;
	UINT8  bits;
} KADP_AUDIO_SOUNDBAR_OSD_VOL;

typedef union {
	struct {
		unsigned char    woofer_level: 4;
		unsigned char    auto_vol: 4;
	} OSD;
	UINT8  bits;
} KADP_AUDIO_SOUNDBAR_WOOFER_VOL;

typedef struct
{
    unsigned int professional;
    unsigned int copyright;
    unsigned int category_code;
    unsigned int channel_number;
    unsigned int source_number;
    unsigned int sampling_freq;
    unsigned int clock_accuracy;
    unsigned int word_length;
    unsigned int original_sampling_freq;
    unsigned int cgms_a;
    unsigned int mode;
    unsigned int pre_emphasis;
    unsigned int data_type;
} KADP_AO_SPDIF_CHANNEL_STATUS_BASIC;

/************************************************************************
*  Function Declaration
 ************************************************************************/
KADP_STATUS_T KADP_AUDIO_Init(void);
KADP_STATUS_T KADP_AUDIO_Finalize(void);
//KADP_STATUS_T KADP_AUDIO_ConvertPhyAddressToVirtualAddress(unsigned long phyAddress, unsigned long* nonCacheAddress, unsigned long* vitualAddress, unsigned long bufferSize);
KADP_STATUS_T KADP_AUDIO_ConvertPhyAddressToVirtualAddress(unsigned long phyAddress, unsigned long* nonCacheAddress, unsigned long* vitualAddress, unsigned long bufferSize, unsigned long* pMapAddr, unsigned long* pMapSize);
KADP_STATUS_T KADP_AUDIO_ReleaseVirtualAddress(unsigned long mapAddress, unsigned long mapSize);

KADP_STATUS_T KADP_AUDIO_GetLRDigitalMuteStatus(BOOLEAN* l, BOOLEAN* r);
KADP_STATUS_T KADP_AUDIO_GetCSWDigitalMuteStatus(BOOLEAN* c, BOOLEAN* sw);
KADP_STATUS_T KADP_AUDIO_GetLRRRDigitalMuteStatus(BOOLEAN* lr, BOOLEAN* rr);
KADP_STATUS_T KADP_AUDIO_GetLSRSDigitalMuteStatus(BOOLEAN* ls, BOOLEAN* rs);

KADP_STATUS_T KADP_AUDIO_AudioConfig(AUDIO_CONFIG_COMMAND_RTKAUDIO *audioConfig);

KADP_STATUS_T KADP_AUDIO_GetAMixerRunningStatus(UINT32* status);

KADP_STATUS_T KADP_AUDIO_SetBassBack(AUDIO_RPC_TV_BASSBACK* info);
KADP_STATUS_T KADP_AUDIO_SetBassBackGain(AUDIO_RPC_TV_BASSBACK_CHANGE_BASS_GAIN *info);
KADP_STATUS_T KADP_AUDIO_SetBassBack_PCBU(AUDIO_RPC_PCBU_BASSBACK *info);

KADP_STATUS_T KADP_AUDIO_SwpSetSRS_TrusurroundHD(AUDIO_RPC_TSXT* pTSXT);

/* KADP for RPC */
KADP_STATUS_T KADP_AUDIO_PrivateInfo(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS *parameter, AUDIO_RPC_PRIVATEINFO_RETURNVAL *ret);
//KADP_STATUS_T KADP_AUDIO_SendConfig(struct AUDIO_CONFIG_COMMAND *audioConfig);
KADP_STATUS_T KADP_AUDIO_GetAudioSpdifChannelStatus(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC *cs, AUDIO_MODULE_TYPE type);
KADP_STATUS_T KADP_AUDIO_SetAudioOptChannelSwap(AUDIO_AO_CHANNEL_OUT_SWAP sel);
KADP_STATUS_T KADP_AUDIO_SetAudioSpectrumData(AUDIO_SPECTRUM_CFG *config);
KADP_STATUS_T KADP_AUDIO_SetAudioAuthorityKey(long customer_key);
KADP_STATUS_T KADP_AUDIO_SetAudioCapability(AUDIO_LICENSE_STATUS_CONFIG);
KADP_STATUS_T KADP_AUDIO_SetModelHash(AUDIO_DUMMY_DATA model_hash);
KADP_STATUS_T KADP_AUDIO_SetAVSyncOffset(AUDIO_HDMI_OUT_VSDB_DATA *info);
KADP_STATUS_T KADP_AUDIO_InitRingBufferHeader(AUDIO_RPC_RINGBUFFER_HEADER *header);
KADP_STATUS_T KADP_AUDIO_SetSpdifOutPinSrc(AUDIO_SPDIFO_SOURCE src);
KADP_STATUS_T KADP_AUDIO_SetTruVolume(AUDIO_RPC_TRUVOLUME* info);

KADP_STATUS_T KADP_AUDIO_SetTruSrndXParam(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetTruSrndXParam(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetTbhdXParam(AUDIO_VIRTUALX_TBHDX_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetTbhdXParam(AUDIO_VIRTUALX_TBHDX_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetMbhlParam(AUDIO_VIRTUALX_MBHL_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetMbhlParam(AUDIO_VIRTUALX_MBHL_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetLoudnessControlParam(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetLoudnessControlParam(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetGeqParam(AUDIO_VIRTUALX_Geq10b_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetGeqParam(AUDIO_VIRTUALX_Geq10b_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetAeqParam(AUDIO_VIRTUALX_Aeq_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetAeqParam(AUDIO_VIRTUALX_Aeq_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetMultiRateParam(AUDIO_VIRTUALX_MultiRate_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetMultiRateParam(AUDIO_VIRTUALX_MultiRate_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_SetTruvolumdHDParam(AUDIO_TRUVOLUMEHD_PARAM_INFO *param);
KADP_STATUS_T KADP_AUDIO_GetTruvolumdHDParam(AUDIO_TRUVOLUMEHD_PARAM_INFO *param);

//audio hw api
KADP_STATUS_T KADP_Audio_HwpAADSIFInit(ATV_CFG *p_cfg);
KADP_STATUS_T KADP_Audio_HwpCurSifType(ATV_AUDIO_SIF_TYPE_T eSifType);
KADP_STATUS_T KADP_Audio_AtvSetDeviationMethod(ATV_DEV_METHOD kadp_dev_method);
KADP_STATUS_T KADP_Audio_AtvEnableAutoChangeSoundModeFlag(int isEnable);
KADP_STATUS_T KADP_Audio_HwpAADSIFFinalize(void);
KADP_STATUS_T KADP_Audio_AtvInit(ATV_CFG *p_cfg);
KADP_STATUS_T KADP_Audio_HwpCurSifType(ATV_AUDIO_SIF_TYPE_T eSifType);
KADP_STATUS_T KADP_Audio_HwpAADSIFInit(ATV_CFG *p_cfg);
KADP_STATUS_T KADP_Audio_AtvGetCurrentDetectUpdate(int *isComplete);
void KADP_Audio_PrintSemBacktrace(void);
KADP_STATUS_T KADP_Audio_AtvGetNicamSignalStable(int *nicam_stable);
KADP_STATUS_T KADP_Audio_HwpSIFGetNicamTH(uint8_t  *p_hi_th,  uint8_t *p_lo_th);
KADP_STATUS_T KADP_Audio_HwpSIFGetA2StereoDualTH(uint32_t *p_a2_th);
KADP_STATUS_T KADP_Audio_HwpSIFDetectedSoundStandard(ATV_SOUND_STD *p_detected_sound_std);
KADP_STATUS_T KADP_Audio_HwpSIFCheckNicamDigital(int *p_exist_flag);
KADP_STATUS_T KADP_Audio_HwpSIFSetNicamTH(uint8_t  hi_th,  uint8_t lo_th);
KADP_STATUS_T KADP_Audio_HwpSIFSetA2StereoDualTH(uint32_t a2_th);
KADP_STATUS_T KADP_Audio_AtvForceSoundSel(int flag);
KADP_STATUS_T KADP_Audio_AtvSetBTSCSoundSelect(uint32_t isPreferPlayStereo, uint32_t  isPreferPlaySap);
KADP_STATUS_T KADP_Audio_AtvSetA2SoundSelect(uint32_t isPreferPlayStereo, uint32_t	preferDualLangIndex);
KADP_STATUS_T KADP_Audio_AtvSetNICAMSoundSelect(uint32_t isPreferNicamDigital, uint32_t  preferDualLangIndex);
KADP_STATUS_T KADP_Audio_AtvSetSoundStd(ATV_SOUND_STD_MAIN_SYSTEM main_system, ATV_SOUND_STD sound_std);
KADP_STATUS_T KADP_Audio_HwpSetBandDelay(void);
KADP_STATUS_T KADP_Audio_HwpSetChannelChange(void);
KADP_STATUS_T KADP_Audio_AtvSetFMoutDownGain(int para);
KADP_STATUS_T KADP_Audio_AtvSetDevBandWidth(A2_BW_SEL_T bw_sel, A2_BW_SEL_T sub_bw_sel);
KADP_STATUS_T KADP_Audio_AtvSetDevOnOff(int bOnOff);
KADP_STATUS_T KADP_Audio_AtvGetSoundStd(ATV_SOUND_INFO *p_sound_info);
KADP_STATUS_T KADP_Audio_ATV_VERIFY_SET_PATH(void);
KADP_STATUS_T KADP_Audio_AtvSetFwPriority(uint16_t Priority);
KADP_STATUS_T KADP_Audio_AtvForceFirstTimeMonoSoundFlag(int isEnable);
KADP_STATUS_T KADP_Audio_AtvEnableAutoChangeStdFlag(int enable);
KADP_STATUS_T KADP_Audio_AtvConfigNoSignalNeed2Mute(uint32_t isNeed2Mute);
KADP_STATUS_T KADP_Audio_HwpSetAtvAudioBand(SIF_INPUT_SOURCE sif_source, ATV_SOUND_STD_MAIN_SYSTEM atv_main_system);
KADP_STATUS_T KADP_Audio_AtvSetMtsPriority(ATV_MTS_PRIORITY  mts_prio);
KADP_STATUS_T KADP_Audio_AtvSetScanStdFlag(bool Enable);
KADP_STATUS_T KADP_Audio_HwpSetSIFDataSource(SIF_INPUT_SOURCE kadp_sif_input_source);
KADP_STATUS_T KADP_Audio_AtvStartDecoding(void);
KADP_STATUS_T KADP_Audio_AtvEnterAtvSource(void);
KADP_STATUS_T KADP_Audio_AtvPauseTvStdDetection(int tv_status);
KADP_STATUS_T KADP_Audio_AtvCleanTVSourceData(void);
KADP_STATUS_T KADP_AUDIO_HW_IOCT_Init(int32_t config);
KADP_STATUS_T KADP_Audio_SetFmRadioMode(AUDIO_ATV_FM_RADIO_MODE Enable);
KADP_STATUS_T KADP_Audio_AtvSetAudioInHandle(long instanceID);
KADP_STATUS_T KADP_Audio_AtvSetSubAudioInHandle(long instanceID);
KADP_STATUS_T KADP_Audio_AtvStopDecoding(void);
KADP_STATUS_T  KADP_Audio_HwpSIFGetMainToneSNR(ATV_SOUND_STD_MAIN_SYSTEM main_sys,
											   ATV_SOUND_STD_MAIN_SYSTEM *p_hw_detected_main_sys,
											   ATV_SOUND_STD *p_hw_detected_sound_std,
												   uint32_t *pToneSNR);
KADP_STATUS_T KADP_Audio_AtvScanSoundStd_FMRadio(ATV_SOUND_STD *p_sound_std, ATV_Carrier_INFO *p_ATV_Carrier_INFO);
#ifdef __cplusplus
}
#endif
#endif  /* _KADP_AUDIO_H_ */



