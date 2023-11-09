#ifndef _RHAL_AUDIO__
#define _RHAL_AUDIO__

#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/delay.h>

#include "rtkdemux_export.h"

#include "hresult.h"
#include "audio_flow.h"
#include <linux/slab.h>
#include <linux/kthread.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtkaudio.h>

#include "audio_rpc.h"
#include "audio_inc.h"
#include "kadp_audio.h"

#include "rtk_kdriver/rtkaudio_debug.h"
#include "audio_base.h"

#include <hal_audio.h>


#define  PRINT_BT_DELAY_PATH "/tmp/printpcmcapdelay.cfg" // print bt delay
#define  DUMP_PCM_CAP_PATH  "/tmp/dumpAudioPcmToFile.cfg" // dump Cap data 2 file
#define  ENABLE_DUMMY_CAP_PATH  "/tmp/dummypcmcap.cfg" // simulate BT enable

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"


/************************************************************************
 *  Config Definitions
 ************************************************************************/
//#define PCM_FILE_DUMP

typedef enum DTS_PARSER_SYNC_TYPE {
    DTS_PARSER_SYNC_TYPE_STANDALONE_CORE = 0,
    DTS_PARSER_SYNC_TYPE_STANDALONE_EXSS,
    DTS_PARSER_SYNC_TYPE_CORE_PLUS_EXSS,
    DTS_PARSER_SYNC_TYPE_STANDALONE_METADATA,
    DTS_PARSER_SYNC_TYPE_FTOC
} DTS_PARSER_SYNC_TYPE;

SINT32 g_16bitsize = 0;
SINT32 g_ringsize = 0;
UINT8 *g_dumpBuffer = NULL;
SINT64 g_prevDumpWriteTime = 0L;
SINT32 g_haveStartCap = 0;

/* use for avoid pop noise when adjust volume */
#define AVOID_USELESS_RPC

/* for user can change sound out device while running */
#define SNDOUT_DEV_PLUGNPLAY

/* check SE input while disconnect if SNDOUT device's input is SE, for SE->X issue */
#define SNDOUT_HARD_CONNECTION

// auto mute decoder when no output
#define DEC_AUTO_MUTE


// auto mute decoder when no output
#define AMIXER_AUTO_MUTE


/* use hybrid volume control (positive @dec and negative @PP) for AD gain */
#define SUPPORT_AD_POSITIVE_GAIN

#define ENUM_DTS_PARAM_MBHL_PROCESS_DISCARD_I32  ENUM_DTS_PARAM_MBHL_FIRST_PARAM + 34
#define ENUM_DTS_PARAM_TBHDX_PROCESS_DISCARD_I32 ENUM_DTS_PARAM_TBHDX_FIRST_PARAM + 7

extern bool ARC_offon;
extern bool eARC_offon;
extern uint32_t rtkaudio_alloc_uncached(int size, unsigned char** uncached_addr_info);
extern int rtkaudio_free(unsigned int phy_addr);


extern int KADP_SDEC_DisConnect(DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T con_dest);
extern int KADP_SDEC_Connect(DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T con_dest, DEMUX_STRUCT_CONNECT_RESULT_T *pBufInfo);
extern int KADP_SDEC_PrivateInfo (DEMUX_CHANNEL_T ch, RSDEC_ENUM_TYPE_DEST_T dest, DEMUX_PRIVATEINFO__T infoId, ULONG *pInfoData, ULONG infoSize);

/* End of Config Definitions
 ************************************************************************/

//=========================================================
// ALL module
extern int gUSER_CONFIG;
static DEFINE_SEMAPHORE(Aud_hdmiFmtNtfy_sem);
static DEFINE_SEMAPHORE(fmtNtfy_sem);

extern int khal_debug;
#define KHAL_DEBUG_SHOW_DIGITALINFO_ 1<<0 //define debug flag

typedef enum {
    ENUM_DTS_PARAM_TBHDX_APP_FIRST_PARAM = ENUM_DTS_PARAM_TBHDX_FIRST_PARAM + 0x1000,
    ENUM_DTS_PARAM_TBHDX_APP_SPKSIZE_I32,           /**< TruBass HDx Speaker Custom Size. Type: int. Range: 40 ~ 600. Default: 80. */
    ENUM_DTS_PARAM_TBHDX_APP_HPRATIO_F32,           /**< TruBass HDx HP ratio. Type: dtsFloat32. Range: 0 ~ 1.0. Default: 0.5f. */
    ENUM_DTS_PARAM_TBHDX_APP_EXTBASS_F32,           /**< TruBass HDX ExtBass. Type: dtsFloat32. Range: 0 ~ 1.0. Default: 0.8f. */
    ENUM_DTS_PARAM_TBHDX_APP_LAST_PARAM
} TBHDX_APPT_TYPE;

typedef enum {
    RHAL_NOT_INIT = 0xff,
    RHAL_FULL,
    RHAL_LITE,
} RHAL_TYPE;
static RHAL_TYPE rhal_type = RHAL_NOT_INIT;

BOOLEAN IsAudioInitial(void)
{
    if(rhal_type == RHAL_NOT_INIT)
    {
        ERROR("[AUDH] audio not yet initialmodule \n");
        return FALSE;
    }
    return TRUE;
}
BOOLEAN IsRHALFullMode(void)
{
    if(rhal_type != RHAL_FULL)
    {
        ERROR("[AUDH] audio not initial in Full support mode rhal_type %d\n", rhal_type);
        return FALSE;
    }
    return TRUE;
}
#define LITE_HAL_INIT_OK IsAudioInitial
#define AUDIO_HAL_CHECK_INITIAL_OK IsRHALFullMode


DTV_STATUS_T RAL_AUDIO_FinalizeModule(void);

#define HAL_MAX_OUTPUT (10)
#define HAL_MAX_RESOURCE_NUM (HAL_AUDIO_RESOURCE_MAX) /* max resource in hal_audio.h */

#define AUD_ADEC_MAX (HAL_AUDIO_ADEC_MAX + 1)
#define AUD_AMIX_MAX (HAL_AUDIO_MIXER_MAX + 1)

#define AUD_MAX_CHANNEL (8)
#define AUD_MAX_DELAY (1000) /* ms */
/*

 open <->  connect    <->      run <-> pause (pause_and_stopflow)
 ^          ^                   ^
 |          |                   |

 close <-> disconnect  <->     stop <-> pause

*/
typedef  enum
{
    HAL_AUDIO_RESOURCE_CLOSE            = 0,
    HAL_AUDIO_RESOURCE_OPEN             = 1,
    HAL_AUDIO_RESOURCE_CONNECT          = 2,
    HAL_AUDIO_RESOURCE_DISCONNECT       = 3,
    HAL_AUDIO_RESOURCE_STOP             = 4,
    HAL_AUDIO_RESOURCE_PAUSE            = 5,
    HAL_AUDIO_RESOURCE_RUN              = 6,
    HAL_AUDIO_RESOURCE_PAUSE_AND_STOP   = 7,
    HAL_AUDIO_RESOURCE_STATUS_UNKNOWN   = 8,
} HAL_AUDIO_RESOURCE_STATUS;

typedef struct {
    ENUM_AUDIO_INFO_TYPE infoType;
    int iptSrc;
    int sndOut;
    int arcMode;
    int latency;
} AUDIO_LATENCY_INFO;

typedef enum {
    SOURCE_UNKNOWN = 0,
    SOURCE_DTV,
    SOURCE_GST_HALF_TUNNEL,
    SOURCE_GST_FULL_TUNNEL,
    SOURCE_GST_NONE_TUNNEL,
    SOURCE_HDMI,
    SOURCE_ATV,
    SOURCE_ADC,
    SOURCE_OFFLOAD,
    SOURCE_ATSC30,
    SOURCE_GST_DIRECT_MEDIA,
    SOURCE_MAX,
} AUDIO_SOURCE;

/**
 * @brief input source type for audio latency time
 */
typedef enum
{
    DELAY_INPUT_SRC_UNKNOWN = 0, // DEFAULT
    DELAY_INPUT_SRC_TP      = 1,
    DELAY_INPUT_SRC_ADC     = 2,
    DELAY_INPUT_SRC_AAD     = 3,
    DELAY_INPUT_SRC_HDMI    = 4,
    DELAY_INPUT_SRC_MEDIA   = 5,
    DELAY_INPUT_SRC_MAX     = 6,
} HAL_AUDIO_DELAY_INPUT_SRC;

// All resource
const CHAR *ResourceName[] = {
    "SDEC0",
    "SDEC1",
    "ATP0",
    "ATP1",
    "ADC",
    "HDMI",
    "AAD",
    "SYSTEM",
    "ADEC0",
    "ADEC1",
    "AENC0",
    "AENC1",
    "SE",
    "SPK",
    "SPDIF",
    "SB_SPDIF",
    "SB_PCM",
    "SB_CANVAS",
    "HP",
    "SCART",
    "AMIXER",
    "MIXER1",
    "MIXER2",
    "MIXER3",
    "MIXER4",
    "MIXER5",
    "MIXER6",
    "MIXER7",
    "SPDIF_ES",
    "HDMI0",
    "HDMI1",
    "HDMI2",
    "HDMI3",
    "SWITCH",
    "DP",
    "MAX",
    "unknown",// unknown
};

const CHAR* ResourceStatusSrting[] = {
    "CLOSE",
    "OPEN",
    "CONNECT",
    "DISCONNECT",
    "STOP",
    "PAUSE",
    "RUN",
    "UNKNOWN",// unknown
};

typedef struct
{
    CHAR name[12];
    HAL_AUDIO_RESOURCE_STATUS connectStatus[HAL_MAX_OUTPUT];
    HAL_AUDIO_RESOURCE_T inputConnectResourceId[HAL_MAX_OUTPUT]; //HAL_AUDIO_RESOURCE_T
    HAL_AUDIO_RESOURCE_T outputConnectResourceID[HAL_MAX_OUTPUT];//HAL_AUDIO_RESOURCE_T
    SINT32 numOptconnect;
    SINT32 numIptconnect;
    SINT32 maxVaildIptNum; // output moduel used

} HAL_AUDIO_MODULE_STATUS;

static const CHAR* GetResourceString(HAL_AUDIO_RESOURCE_T resId);
static const CHAR* GetResourceStatusString(SINT32  statusId);


#define AUDIO_HAL_CHECK_PLAY_NOTAVAILABLE(res, inputID) \
    ((res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_CONNECT) && \
     (res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_STOP))

#define AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(res, inputID) \
    ((res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_RUN) && \
     (res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_PAUSE))

#define AUDIO_HAL_CHECK_PAUSE_NOTAVAILABLE(res, inputID) \
    (res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_RUN)

#define AUDIO_HAL_CHECK_PAUSE_AND_STOP_NOTAVAILABLE(res, inputID) \
    (res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_RUN)


#define AUDIO_HAL_CHECK_RESUME_NOTAVAILABLE(res, inputID) \
    (res.connectStatus[inputID] != HAL_AUDIO_RESOURCE_PAUSE)

#define AUDIO_HAL_CHECK_ISATRUNSTATE(res, inputID) \
    ((res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_RUN) || \
    (res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_PAUSE))

#define AUDIO_HAL_CHECK_ISATSTOPSTATE(res, inputID) \
    ((res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_CONNECT) || \
    (res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_STOP))

#define AUDIO_HAL_CHECK_ISATPAUSESTATE(res, inputID) \
    ((res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_PAUSE))

#define AUDIO_HAL_CHECK_ISATPAUSEANDSTOPSTATE(res, inputID) \
    ((res.connectStatus[inputID] == HAL_AUDIO_RESOURCE_PAUSE_AND_STOP))

// Flow
typedef struct
{
    UINT8 IsAINExist;
    SINT32 AinConnectDecIndex;
    UINT8 IsSubAINExist;
    SINT32 subAinConnectDecIndex;
    UINT8 IsDEC0Exist;
    UINT8 IsDEC1Exist;
    UINT8 IsScartOutBypas;
    SINT32 ScartConnectDecIndex;
    SINT32 mainDecIndex;
    UINT8 IsDTV0SourceRead; // ATP
    UINT8 IsDTV1SourceRead;
    UINT8 IsMainPPAOExist;
    UINT8 IsSubPPAOExist;
} HAL_AUDIO_FLOW_STATUS;

static void UpdateADECStatus(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect);
static BOOLEAN AddAndConnectDecAinToFlow(FlowManager* pFlowManager, HAL_AUDIO_ADEC_INDEX_T adecIndex, Base* pAin, HAL_AUDIO_FLOW_STATUS* pFlowStatus);
static BOOLEAN AddAndConnectPPAOToFlow(FlowManager* pFlowManager, HAL_AUDIO_ADEC_INDEX_T adecIndex, Base* pPPAO, HAL_AUDIO_FLOW_STATUS* pFlowStatus);
//===============================================================================


//=========================================================
//  DEC
typedef struct
{
    UINT32 decMute;
    UINT32 decDelay;
    UINT32 decVol[AUD_MAX_CHANNEL];
} ADSP_DEC_INFO;

typedef struct
{
    BOOLEAN spk_mute;
    BOOLEAN spdif_mute;
    BOOLEAN spdifes_mute;
    BOOLEAN hp_mute;
    BOOLEAN scart_mute;
} ADSP_SNDOUT_INFO;

typedef struct
{
    SINT32 spdifouttype;
    SINT32 trickPauseState;
    BOOLEAN trickPauseEnable;
    BOOLEAN spdifESMute;
    BOOLEAN decInMute;
    BOOLEAN userSetRun; // internal will auto stop flow, use this flag to store user setting.
    HAL_AUDIO_VOLUME_T decInVolume;
    HAL_AUDIO_VOLUME_T decOutVolume[AUD_MAX_CHANNEL];
    HAL_AUDIO_DOLBY_DRC_MODE_T drcMode;
    HAL_AUDIO_DOWNMIX_MODE_T downmixMode;
    BOOLEAN decAtvMute;
} HAL_AUDIO_DEC_MODULE_STATUS;

HAL_AUDIO_DEC_MODULE_STATUS Aud_decstatus[AUD_ADEC_MAX];

ADSP_SNDOUT_INFO adsp_sndout_info;

SINT32 Aud_curStartDecodingTime = 0;
SINT32 Aud_mainDecIndex     = 0;
SINT32 Aud_prevMainDecIndex = -1;
SINT32 Aud_descriptionMode  = 0;
SINT32 Aud_subDecEnable     = 0;
BOOLEAN m_IsSetMainDecOptByMW = FALSE;
BOOLEAN m_IsHbbTV = FALSE;
BOOLEAN m_bForceStopUpload = FALSE;
BOOLEAN m_bPcm_cb_reset = FALSE;
BOOLEAN m_bcheckDummyInfo = FALSE;
BOOLEAN m_bTimeConsuming = FALSE;
BOOLEAN m_bresetCounting = FALSE;
SINT32 m_bTimeConsumingType = 0;
SINT32 m_nDumpBTInterval = 1;//90K pts unit, default is 1 (21.333ms)
SINT32 m_nDumpBTLogType = 0;//0:simple type, 1:full type
BOOLEAN m_bMonitorBTTimer = FALSE;//check 2 ms timer delayed: 2ms->xx ms
SINT32 m_nMonitorBTTimer_restrict = 10;//check 2 ms timer delayed: 2ms->xx ms
static BOOLEAN g_isAMIXERRunning = FALSE;

//static unsigned long* dbxTableNonCacheAddr = NULL;
static unsigned long dbxTablePhyAddr = 0;
static UINT8* dbxTableAddr = NULL;

//=========================================================
// Ain
#define MAIN_AIN_ADC_PIN (9)
typedef struct
{
    SINT32 ainPinStatus[MAIN_AIN_ADC_PIN];
} HAL_AUDIO_AIN_MODULE_STATUS;

HAL_AUDIO_MODULE_STATUS ResourceStatus[HAL_MAX_RESOURCE_NUM];
HAL_AUDIO_FLOW_STATUS MainFlowStatus;
HAL_AUDIO_FLOW_STATUS SubFlowStatus;
HAL_AUDIO_AIN_MODULE_STATUS AinStatus;
ADSP_DEC_INFO AudioDecInfo[AUD_ADEC_MAX];

HAL_AUDIO_ADEC_INFO_T Aud_Adec_info[AUD_ADEC_MAX];
UINT32 _AudioSPDIFMode = ENABLE_DOWNMIX;
UINT32 _AudioARCMode = ENABLE_DOWNMIX;
UINT32 _ARC_Enable = FALSE;
UINT32 _EARC_Enable = FALSE;

static UINT32 Sndout_Devices = 0;

#define HAL_DEC_MAX_OUTPUT_NUM (7) // dec maybe connect with spdif, spdif_ES, HP, SE(speak), scart, SB_PCM(BT), SB_CANVAS

#define AUDIO_CHECK_ADC_PIN_OPEN_NOTAVAILABLE(status)       ((status != HAL_AUDIO_RESOURCE_CLOSE) && ( status != HAL_AUDIO_RESOURCE_OPEN))
#define AUDIO_CHECK_ADC_PIN_CLOSE_NOTAVAILABLE(status)      ((status != HAL_AUDIO_RESOURCE_OPEN) && (status != HAL_AUDIO_RESOURCE_DISCONNECT) && (status != HAL_AUDIO_RESOURCE_OPEN))
#define AUDIO_CHECK_ADC_PIN_CONNECT_NOTAVAILABLE(status)    ((status != HAL_AUDIO_RESOURCE_OPEN) && (status != HAL_AUDIO_RESOURCE_DISCONNECT))
#define AUDIO_CHECK_ADC_PIN_DISCONNECT_NOTAVAILABLE(status) (status != HAL_AUDIO_RESOURCE_CONNECT)

//=========================================================
//  SE
// for SE connect to both deocder and AMIX
#define SE_INPUT_ADEC0_INDEX (0)
#define SE_INPUT_ADEC1_INDEX (1)
#define SE_INPUT_AMIX_INDEX (2)
#define MAX_SE_INPUT_NUM (3)

SINT32 g_printBTDelay = 0;
SINT32 g_OriDataPTS = 0;
UINT32 g_BTDelayPrevPrintTime = 0;

typedef void (*pfnCopyMem)(SINT32*, UINT8*, SINT32);
typedef void (*pfnCopyMem_LRToIntlv)(SINT32*, SINT32*, UINT8*, SINT32);

#define FAST_PCMCAP
UINT32 cap_period[100] = {0};
SINT64 cap_prev_time = 0;
SINT64 cap_curr_time = 0;

typedef struct
{
    HAL_AUDIO_RESOURCE_STATUS SEPinStatus;
} HAL_AUDIO_SE_MODULE_STATUS;

//=========================================================
//  AMIX
// AMIX no open close api

#define AUDIO_HAL_CHECK_AMIX_PLAY_NOTAVAILABLE(res) \
    ((res.connectStatus[0] != HAL_AUDIO_RESOURCE_CONNECT) && \
     (res.connectStatus[0] != HAL_AUDIO_RESOURCE_STOP))

#define AUDIO_HAL_CHECK_AMIX_STOP_NOTAVAILABLE(res) \
    ((res.connectStatus[0] != HAL_AUDIO_RESOURCE_RUN) && \
     (res.connectStatus[0] != HAL_AUDIO_RESOURCE_PAUSE))

#define AUDIO_HAL_CHECK_AMIX_PAUSE_NOTAVAILABLE(res) \
    (res.connectStatus[0] != HAL_AUDIO_RESOURCE_RUN)

#define AUDIO_HAL_CHECK_AMIX_RESUME_NOTAVAILABLE(res) \
    (res.connectStatus[0] != HAL_AUDIO_RESOURCE_PAUSE)

void ShowFlow(HAL_AUDIO_RESOURCE_T adecResId, HAL_AUDIO_RESOURCE_STATUS newStatus, SINT32 forcePrint);

//=========================================================

struct AUDIO_QUICK_SHOW_INIT_TABLE
{
    int quick_show_status;
    int ain_inst;
    int ain_out_rh;
    int ain_out_buf;
    int dec_inst;
    int dec_out_rh[8];
    int dec_out_buf[8];
    int dec_icq_rh;
    int dec_icq_buf;
    int ao_inst;
};
typedef struct AUDIO_QUICK_SHOW_INIT_TABLE AUDIO_QUICK_SHOW_INIT_TABLE;

//RESAMPLE_COEF_INFO AUD_22To48Info;
//RESAMPLE_COEF_INFO AUD_11To48Info;

//=========================================================


//--- spdif out channel status setting ---//
// copy from cppaout.h
#define HAL_SPDIF_CONSUMER_USE          0x0
#define HAL_SPDIF_PROFESSIONAL_USE      0x1

#define HAL_SPDIF_COPYRIGHT_NEVER       0x0
#define HAL_SPDIF_COPYRIGHT_FREE        0x1

#define HAL_SPDIF_CATEGORY_GENERAL      0x00
#define HAL_SPDIF_CATEGORY_EUROPE       0x0C
#define HAL_SPDIF_CATEGORY_USA          0x64
#define HAL_SPDIF_CATEGORY_JAPAN        0x04

#define HAL_SPDIF_CATEGORY_L_BIT_IS_1   0x80
#define HAL_SPDIF_CATEGORY_L_BIT_IS_0   0x00

#define HAL_SPDIF_WORD_LENGTH_NONE      0x0
#define HAL_SPDIF_WORD_LENGTH_16        0x2
#define HAL_SPDIF_WORD_LENGTH_18        0x4
#define HAL_SPDIF_WORD_LENGTH_19        0x8
#define HAL_SPDIF_WORD_LENGTH_20_0      0xA
#define HAL_SPDIF_WORD_LENGTH_17        0xC
#define HAL_SPDIF_WORD_LENGTH_20_1      0x3
#define HAL_SPDIF_WORD_LENGTH_22        0x5
#define HAL_SPDIF_WORD_LENGTH_23        0x9
#define HAL_SPDIF_WORD_LENGTH_24        0xB
#define HAL_SPDIF_WORD_LENGTH_21        0xD
//======================================================
typedef struct
{
    struct task_struct *rtkaudio_hdmifmtntfy_tsk;
    pfnHdmiFmtNotify          pfnCallBack;
    HAL_AUDIO_ADEC_INDEX_T    adecIdx;
} RAL_AUDIO_HDMI_FMT_NTFY_T;
static RAL_AUDIO_HDMI_FMT_NTFY_T* Aud_hdmiFmtNtfy = NULL;


#define DATASIZE_PER_COUNT (256*4)

typedef struct {
    SINT32     datasize;
    SINT64 pts;
} RAL_AUDIO_BT_PTS_T;

static HAL_AUDIO_COMMON_INFO_T g_AudioStatusInfo;  //initial ???

typedef struct HAL_AUDIO_SIF_INFO
{
    HAL_AUDIO_SIF_INPUT_T               sifSource;                  /* Currnet SIF Source Input Status */
    HAL_AUDIO_SIF_TYPE_T                curSifType;                 /* Currnet SIF Type Status */
    BOOLEAN                             bHighDevOnOff;              /* Currnet High DEV ON/OFF Status */
    HAL_AUDIO_SIF_SOUNDSYSTEM_T         curSifBand;                 /* Currnet SIF Sound Band(Sound System) */
    HAL_AUDIO_SIF_STANDARD_T            curSifStandard;             /* Currnet SIF Sound Standard */
    HAL_AUDIO_SIF_EXISTENCE_INFO_T      curSifIsA2;                 /* Currnet SIF A2 Exist Status */
    HAL_AUDIO_SIF_EXISTENCE_INFO_T      curSifIsNicam;              /* Currnet SIF NICAM Exist Status */
    HAL_AUDIO_SIF_MODE_SET_T            curSifModeSet;              /* Currnet SIF Sound Mode Set Status */
    HAL_AUDIO_SIF_MODE_GET_T            curSifModeGet;              /* Currnet SIF Sound Mode Get Status */
} HAL_AUDIO_SIF_INFO_T;

static HAL_AUDIO_SIF_INFO_T g_AudioSIFInfo = {HAL_AUDIO_SIF_INPUT_INTERNAL, HAL_AUDIO_SIF_TYPE_NONE, FALSE, HAL_AUDIO_SIF_SYSTEM_UNKNOWN, HAL_AUDIO_SIF_MODE_DETECT,
                                              HAL_AUDIO_SIF_DETECTING_EXSISTANCE,HAL_AUDIO_SIF_DETECTING_EXSISTANCE, HAL_AUDIO_SIF_SET_PAL_UNKNOWN,HAL_AUDIO_SIF_GET_PAL_UNKNOWN};

HAL_AUDIO_VOLUME_T g_mixer_gain[AUD_AMIX_MAX] = {{.mainVol = 0x7F, .fineVol = 0x0}};
HAL_AUDIO_VOLUME_T g_mixer_out_gain = {.mainVol = 0x7F, .fineVol = 0x0}; // only one ?


BOOLEAN g_mixer_user_mute[AUD_AMIX_MAX] = {FALSE};
BOOLEAN g_mixer_curr_mute[AUD_AMIX_MAX] = {FALSE};

#define DB2MIXGAIN_TABLE_SIZE (52)
const UINT32 dB2mixgain_table[DB2MIXGAIN_TABLE_SIZE] = {
    0x7FFFFFFF , 0x721482BF , 0x65AC8C2E , 0x5A9DF7AA , //  0  ~ -3dB
    0x50C335D3 , 0x47FACCF0 , 0x3FFFFFFF , 0x392CED8D , // -4  ~ -7dB
    0x32F52CFE , 0x2D6A866F , 0x287A26C4 , 0x241346F5 , // -8  ~ -11dB
    0x2026F30F , 0x1CA7D767 , 0x198A1357 , 0x16C310E3 , // -12 ~ -15dB
    0x144960C5 , 0x12149A5F , 0x101D3F2D , 0xE5CA14C  , // -16 ~ -19dB
    0xCCCCCCC  , 0xB687379  , 0xA2ADAD1  , 0x90FCBF7  , // -20 ~ -23dB
    0x8138561  , 0x732AE17  , 0x66A4A52  , 0x5B7B15A  , // -24 ~ -27dB
    0x518847F  , 0x48AA70B  , 0x40C3713  , 0x39B8718  , // -28 ~ -31dB
    0x337184E  , 0x2DD958A  , 0x28DCEBB  , 0x246B4E3  , // -32 ~ -35dB
    0x207567A  , 0x1CEDC3C  , 0x19C8651  , 0x16FA9BA  , // -36 ~ -39dB
    0x147AE14  , 0x1240B8C  , 0x1044914  , 0xE7FACB   , // -40 ~ -43dB
    0xCEC089   , 0xB8449B   , 0xA43AA1   , 0x925E89   , // -44 ~ -47dB
    0x8273A6   , 0x7443E7   , 0x679F1B   , 0x5C5A4F     // -48 ~ -51dB
};
#define ADEC_DSP_MIX_GAIN_MUTE (0x00000000)
#define ADEC_DSP_MIX_GAIN_0DB  (0x7FFFFFFF)
HAL_AUDIO_VOLUME_T currMixADVol = {.mainVol = 0x7F, .fineVol = 0x0};
HAL_AUDIO_VOLUME_T currDecADVol = {.mainVol = 0x7F, .fineVol = 0x0};

static Base* Aud_MainAin = NULL;
static Base* Aud_SubAin = NULL;
static Base* Aud_dec[AUD_ADEC_MAX] = {NULL};
static Base* Aud_ppAout = NULL;
static Base* Aud_subPPAout = NULL;
static Base* Aud_DTV[AUD_ADEC_MAX] = {NULL};

static FlowManager* Aud_flow0; // ADEC0 use
static FlowManager* Aud_flow1; // ADEC1 use

#if 0
static AUDIO_PROCESS* AudioProcess = NULL;

static AUDIO_PROCESS* AudioManagerProcess = NULL;
#endif

static CHAR AUD_StringBuffer[128];

const CHAR *SRCTypeName[] = {
    "UNKNOWN",     /* = 0  */
    "PCM",         /* = 1  */
    "AC3",         /* = 2  */
    "EAC3",        /* = 3  */
    "MPEG",        /* = 4  */
    "AAC",         /* = 5  */
    "HEAAC",       /* = 6  */
    "DRA",         /* = 7  */
    "MP3",         /* = 8  */
    "DTS",         /* = 9  */
    "SIF",         /* = 10 */
    "SIF_BTSC",    /* = 11 */
    "SIF_A2",      /* = 12 */
    "DEFAULT",     /* = 13 */
    "NONE",        /* = 14 */
    "DTS_HD_MA",   /* = 15 */
    "DTS_EXPRESS", /* = 16 */
    "DTS_CD",      /* = 17 */
    "EAC3_ATMOS",  /* = 18 */
    "AC4",         /* = 19 */
    "AC4_ATMOS",   /* = 20 */
    "MPEG_H",      /* = 21 */
    "MAT",         /* = 22 */
    "MAT_ATMOS",   /* = 23 */
    "OPUS",        /* = 24 */
};

/***********************************************************************************
 * Static resource checking function
 **********************************************************************************/
static const CHAR* GetResourceString(HAL_AUDIO_RESOURCE_T resId)
{
    if ((resId >= 0) && (resId <= HAL_AUDIO_RESOURCE_MAX))
        return ResourceName[resId];

    return ResourceName[HAL_AUDIO_RESOURCE_MAX];
}

static const CHAR* GetResourceStatusString(SINT32 statusId)
{
    SINT32 maxid = (sizeof(ResourceStatusSrting)/sizeof(ResourceStatusSrting[0]));
    if((statusId >= 0) && (statusId < maxid))
        return ResourceStatusSrting[statusId];
    else
        return ResourceStatusSrting[maxid -1];
}

static const CHAR* GetSRCTypeName(HAL_AUDIO_SRC_TYPE_T SRCType)
{
    if(SRCType < 0 || SRCType > HAL_AUDIO_SRC_TYPE_OPUS)
        return SRCTypeName[0];
    else
        return SRCTypeName[SRCType];
}

static inline HAL_AUDIO_ADEC_INDEX_T res2adec(HAL_AUDIO_RESOURCE_T res_id)
{
    return (HAL_AUDIO_ADEC_INDEX_T)(res_id - HAL_AUDIO_RESOURCE_ADEC0);
}

static inline HAL_AUDIO_RESOURCE_T adec2res(HAL_AUDIO_ADEC_INDEX_T dec_id)
{
    return (HAL_AUDIO_RESOURCE_T)(dec_id + HAL_AUDIO_RESOURCE_ADEC0);
}

static inline HAL_AUDIO_MIXER_INDEX_T res2amixer(HAL_AUDIO_RESOURCE_T res_id)
{
    return (HAL_AUDIO_MIXER_INDEX_T)(res_id - HAL_AUDIO_RESOURCE_MIXER0);
}

static inline HAL_AUDIO_RESOURCE_T amixer2res(HAL_AUDIO_MIXER_INDEX_T amixer_id)
{
    return (HAL_AUDIO_RESOURCE_T)(amixer_id + HAL_AUDIO_RESOURCE_MIXER0);
}


static inline BOOLEAN IsAMIXSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if((res_id <= HAL_AUDIO_RESOURCE_MIXER7) && (res_id >= HAL_AUDIO_RESOURCE_MIXER0))
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsADCSource(HAL_AUDIO_RESOURCE_T res_id)
{
    return (res_id == HAL_AUDIO_RESOURCE_ADC);
}

static inline BOOLEAN IsHDMISource(HAL_AUDIO_RESOURCE_T res_id)
{
    if (res_id == HAL_AUDIO_RESOURCE_HDMI)
        return TRUE;
    else if ((res_id >= HAL_AUDIO_RESOURCE_HDMI0) && (res_id <= HAL_AUDIO_RESOURCE_SWITCH))
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsDPSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if (res_id == HAL_AUDIO_RESOURCE_DP)
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsATVSource(HAL_AUDIO_RESOURCE_T res_id)
{
    return (res_id == HAL_AUDIO_RESOURCE_AAD);
}

static inline BOOLEAN IsSystemSource(HAL_AUDIO_RESOURCE_T res_id)
{
    return (res_id == HAL_AUDIO_RESOURCE_SYSTEM);
}

static inline BOOLEAN IsDTVSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if(res_id == HAL_AUDIO_RESOURCE_ATP0 || res_id == HAL_AUDIO_RESOURCE_ATP1)
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsAinSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if(IsADCSource(res_id))
        return TRUE;
    else if(IsHDMISource(res_id))
        return TRUE;
    else if(IsATVSource(res_id))
        return TRUE;
    else if(IsDPSource(res_id))
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsADECSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if(res_id == HAL_AUDIO_RESOURCE_ADEC0 || res_id == HAL_AUDIO_RESOURCE_ADEC1)
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsSESource(HAL_AUDIO_RESOURCE_T res_id)
{
    return (res_id == HAL_AUDIO_RESOURCE_SE);
}

static inline BOOLEAN IsBTOutputSource(HAL_AUDIO_RESOURCE_T res_id)
{
    if (res_id == HAL_AUDIO_RESOURCE_OUT_SB_PCM)
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsAOutSource(HAL_AUDIO_RESOURCE_T res_id)
{
    BOOLEAN reval;
    switch (res_id)
    {
        case HAL_AUDIO_RESOURCE_SE:
        case HAL_AUDIO_RESOURCE_OUT_SPK:
        case HAL_AUDIO_RESOURCE_OUT_SPDIF:
        case HAL_AUDIO_RESOURCE_OUT_SB_SPDIF:
        case HAL_AUDIO_RESOURCE_OUT_SB_PCM:
        case HAL_AUDIO_RESOURCE_OUT_SB_CANVAS:
        case HAL_AUDIO_RESOURCE_OUT_HP:
        case HAL_AUDIO_RESOURCE_OUT_SCART:
        case HAL_AUDIO_RESOURCE_OUT_SPDIF_ES:
            reval = TRUE;
            break;
        default:
            reval = FALSE;
            break;
    }
    return reval;
}

static inline BOOLEAN IsValidSEOpts(HAL_AUDIO_RESOURCE_T res_id)
{
    if((res_id == HAL_AUDIO_RESOURCE_OUT_SPK) ||
       (res_id == HAL_AUDIO_RESOURCE_OUT_SPDIF))
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN IsValidADECIpts(HAL_AUDIO_RESOURCE_T res_id)
{
    if(IsAinSource(res_id))
        return TRUE;
    else if(IsDTVSource(res_id))
        return TRUE;
    else if(IsSystemSource(res_id))
        return TRUE;
    else
        return FALSE;
}

static inline BOOLEAN RangeCheck(UINT32 target, UINT32 min, UINT32 max)
{
    return (target >= min && target < max);
}
/*********** End of Resource related static functions *****************************/

/**********************************************************************************
 * Static variables access functions
 **********************************************************************************/
static inline UINT32 GetDecInMute(UINT32 index)
{
    return Aud_decstatus[index].decInMute;
}

static inline UINT32 GetDecESMute(UINT32 index)
{
    return Aud_decstatus[index].spdifESMute;
}

static inline HAL_AUDIO_VOLUME_T GetDecInVolume(UINT32 index)
{
    return Aud_decstatus[index].decInVolume;
}

static inline HAL_AUDIO_VOLUME_T GetDecOutVolume(UINT32 index, UINT32 ch)
{
    return Aud_decstatus[index].decOutVolume[ch];
}

static inline UINT32 GetTrickState(UINT32 index)
{
    return Aud_decstatus[index].trickPauseState;
}

static inline void SetDecInMute(UINT32 index, UINT32 bMute)
{
    Aud_decstatus[index].decInMute = bMute;
}

static inline void SetDecInVolume(UINT32 index, HAL_AUDIO_VOLUME_T vol)
{
    Aud_decstatus[index].decInVolume = vol;
}

static inline void SetDecOutVolume(UINT32 index, UINT32 ch, HAL_AUDIO_VOLUME_T vol)
{
    Aud_decstatus[index].decOutVolume[ch] = vol;
}

static inline void SetDecESMute(UINT32 index, UINT32 bMute)
{
    Aud_decstatus[index].spdifESMute = bMute;
}

static inline void SetSPDIFOutType(UINT32 index, UINT32 type)
{
    Aud_decstatus[index].spdifouttype = type;
}

static inline void SetTrickState(UINT32 index, UINT32 state)
{
    Aud_decstatus[index].trickPauseState = state;
}

/*
static inline void SetTrickPause(UINT32 index, UINT32 bPause)
{
    Aud_decstatus[index].trickPauseEnable = bPause;
}
*/

static inline void SetDecUserState(UINT32 index, UINT32 state)
{
    Aud_decstatus[index].userSetRun = state;
}
static inline UINT32 GeDectUserState(UINT32 adecIndex)
{
    return Aud_decstatus[adecIndex].userSetRun;
}

static inline UINT32 GetCurrentFormat(UINT32 adecIndex)
{
    return Aud_Adec_info[adecIndex].curAdecFormat;
}


static inline void SetDecDrcMode(UINT32 adecIndex, HAL_AUDIO_DOLBY_DRC_MODE_T mode)
{
    Aud_decstatus[adecIndex].drcMode = mode;
}

static inline HAL_AUDIO_DOLBY_DRC_MODE_T GetDecDrcMode(UINT32 adecIndex)
{
    return Aud_decstatus[adecIndex].drcMode;
}


static inline void SetDecDownMixMode(UINT32 adecIndex, HAL_AUDIO_DOWNMIX_MODE_T mode)
{
    Aud_decstatus[adecIndex].downmixMode = mode;
}

static inline HAL_AUDIO_DOWNMIX_MODE_T GetDecDownMixMode(UINT32 adecIndex)
{
    return Aud_decstatus[adecIndex].downmixMode;
}

static inline UINT32 GetAmixerUserMute(HAL_AUDIO_MIXER_INDEX_T index)
{
    //assert(index < AUD_AMIX_MAX);//rhal do
    return g_mixer_user_mute[index];
}

static inline void SetAmixerUserMute(HAL_AUDIO_MIXER_INDEX_T index, UINT32 bMute)
{
    //assert(index < AUD_AMIX_MAX);//rhal do
    g_mixer_user_mute[index] = bMute;
}



/*********** End of Static variables access functions ****************************/


/**********************************************************************************
 * HAL Volume and ADSP Gain related functions
 **********************************************************************************/
SINT32 Volume_Compare(HAL_AUDIO_VOLUME_T v1, HAL_AUDIO_VOLUME_T v2)
{
    if((v1.mainVol == v2.mainVol) && (v1.fineVol == v2.fineVol))
        return TRUE;
    else
        return FALSE;
}

HAL_AUDIO_VOLUME_T Volume_Add(HAL_AUDIO_VOLUME_T v1, HAL_AUDIO_VOLUME_T v2)
{
    HAL_AUDIO_VOLUME_T result;
    SINT16 mainVol, fineVol;

    mainVol = (SINT16)(v1.mainVol - 127) + (SINT16)(v2.mainVol - 127);
    fineVol = v1.fineVol + v2.fineVol;
    //fix coverity 306, Event uninit_use: Using uninitialized value "result.fineVol"
    //if(result.fineVol >= 16)
    if(fineVol >= 16)
    {
        mainVol += 1;
        fineVol -= 16;
    }

    if(mainVol < -127)
    {
        //AUDIO_VERBOSE("[HAL_AUD][WARNING] (%d dB) is under support range\n", mainVol);
        result.mainVol = 0x0;
        result.fineVol = 0x0;
    }
    else if(mainVol > 127)
    {
        //AUDIO_VERBOSE("[HAL_AUD][WARNING] (%d dB) is over support range\n", mainVol);
        result.mainVol = 0xFF;
        result.fineVol = 0x0;
    }
    else
    {
        result.mainVol = (UINT8)mainVol + 0x7F;
        result.fineVol = (UINT8)fineVol;
    }
    return result;
}

SINT32 Volume_to_DSPGain(HAL_AUDIO_VOLUME_T volume)
{
    SINT32 dsp_gain = ENUM_AUDIO_DVOL_K0p0DB;
    if(volume.fineVol%4 != 0)
    {
        //INFO("[HAL_AUD][WARNING] fineVol(%d) not support, approximate it\n", volume.fineVol);
    }

    /* SW dsp_gain scale: 0.25 dB
     * mainVol scale: 1 dB
     * fineVol scale: 0.0625 dB
     */
    dsp_gain += ((volume.mainVol - 127)*4);
    dsp_gain += (volume.fineVol/4);

    if(dsp_gain > ENUM_AUDIO_DVOL_K30p0DB)
    {
        //AUDIO_VERBOSE("[HAL_AUD][WARNING] volume(%d) over support range, set to +30dB\n",dsp_gain);
        dsp_gain = ENUM_AUDIO_DVOL_K30p0DB;
    }
    else if(dsp_gain < ENUM_AUDIO_DVOL_KMINUS72p0DB)
    {
        //AUDIO_VERBOSE("[HAL_AUD][WARNING] volume(%d) under support range, set to -72dB\n",dsp_gain);
        dsp_gain = ENUM_AUDIO_DVOL_KMINUS72p0DB;
    }

    return dsp_gain;
}

UINT32 Volume_to_MixerGain(HAL_AUDIO_VOLUME_T volume)
{
    SINT32 Adsp_gainume_dB = (SINT32)volume.mainVol-0x7F;
    if(Adsp_gainume_dB > 0)
    {
        return ADEC_DSP_MIX_GAIN_0DB;
    }
    else if(Adsp_gainume_dB <= -DB2MIXGAIN_TABLE_SIZE) /* under -51dB */
    {
        return ADEC_DSP_MIX_GAIN_MUTE;
    }
    else
    {
        return dB2mixgain_table[(-Adsp_gainume_dB)];
    }
}

// inputConnect -> current Connect
static void CleanConnectInputSourceAndStatus(HAL_AUDIO_MODULE_STATUS resourceStatus[HAL_MAX_RESOURCE_NUM],
                                             HAL_AUDIO_RESOURCE_T currentConnect,
                                             HAL_AUDIO_RESOURCE_T inputConnect)
{
    SINT32 i, index;

    // current  module 's input pin
    index = resourceStatus[currentConnect].numIptconnect;

    if(index <=  0)
    {
        AUDIO_FATAL("[AUDH-FATAL] Inputs of %s = (%d)\n", GetResourceString(currentConnect), index);
        return;
    }

    for( i = 0; i < resourceStatus[currentConnect].numIptconnect; i++)
    {
        if(resourceStatus[currentConnect].inputConnectResourceId[i] == inputConnect)
        {
            break;
        }
    }

    //assert(i < resourceStatus[currentConnect].numIptconnect);// no find connect //rhal do
    for(; i < (resourceStatus[currentConnect].numIptconnect -1); i++)
    {
        resourceStatus[currentConnect].inputConnectResourceId[i] = resourceStatus[currentConnect].inputConnectResourceId[i+1];
        resourceStatus[currentConnect].connectStatus[i] = resourceStatus[currentConnect].connectStatus[i+1];
    }

    if(resourceStatus[currentConnect].numIptconnect <= 0)
    {
        ERROR( "[AUDH] %s  %d %d \n", __func__, currentConnect, inputConnect);
        //assert(resourceStatus[currentConnect].numIptconnect > 0);//rhal do
    }

    resourceStatus[currentConnect].inputConnectResourceId[index-1] = HAL_AUDIO_RESOURCE_NO_CONNECTION;
    resourceStatus[currentConnect].connectStatus[index-1] = HAL_AUDIO_RESOURCE_DISCONNECT;
    resourceStatus[currentConnect].numIptconnect--;

    if(HAL_AUDIO_RESOURCE_NO_CONNECTION == inputConnect) // AMXI no connectinput
        return;

    // input module 's output pin
    index = resourceStatus[inputConnect].numOptconnect;

    if(index <=  0)
    {
        AUDIO_FATAL("[AUDH-FATAL] Outputs of %s = (%d)\n", GetResourceString(inputConnect), index);
        AUDIO_FATAL("[AUDH][%s] %s -> %s\n", __func__,
                GetResourceString(inputConnect), GetResourceString(currentConnect));
        //assert(resourceStatus[inputConnect].numOptconnect > 0);//rhal do
        return ;//  input module has no output pin
    }

    for( i = 0; i < resourceStatus[inputConnect].numOptconnect; i++)
    {
        if(resourceStatus[inputConnect].outputConnectResourceID[i] == currentConnect)
        {
            break;
        }
    }

    //assert(i <  resourceStatus[inputConnect].numOptconnect);// no find connect //rhal do
    for(; i < (resourceStatus[inputConnect].numOptconnect -1); i++)
    {
        resourceStatus[inputConnect].outputConnectResourceID[i] = resourceStatus[inputConnect].outputConnectResourceID[i+1];
    }

    resourceStatus[inputConnect].outputConnectResourceID[index -1] = HAL_AUDIO_RESOURCE_NO_CONNECTION;
    resourceStatus[inputConnect].numOptconnect--;
}

static SINT32 GetConnectInputSourceIndex(HAL_AUDIO_MODULE_STATUS resourceStatus, HAL_AUDIO_RESOURCE_T searchId)
{
    SINT32 i;

    // check if reconnect
    for(i = 0; i< resourceStatus.numIptconnect; i++)
    {
        if(resourceStatus.inputConnectResourceId[i] == searchId) // at connect status
        {
            return i;
        }
    }

    return -1;
}

static BOOLEAN inline SetResourceOpen(HAL_AUDIO_RESOURCE_T id)
{
    SINT32 i = 0;
    //assert(id < HAL_AUDIO_RESOURCE_MAX);//rhal do

    // check all status is correct
    for(i = 0; i < HAL_MAX_OUTPUT; i++)
    {
        if((ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_CLOSE)&&
            (ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_OPEN))
        {
            ERROR("[AUDH] open %s failed status is at %s \n",
                    GetResourceString(id), GetResourceStatusString(ResourceStatus[id].connectStatus[0]));
            return FALSE;
        }
    }

    for(i = 0; i < HAL_MAX_OUTPUT; i++)
    {
        ResourceStatus[id].connectStatus[i] = HAL_AUDIO_RESOURCE_OPEN;
    }

    return TRUE;
}

static BOOLEAN inline SetResourceDisconnect(HAL_AUDIO_RESOURCE_T id, HAL_AUDIO_RESOURCE_T inputConnect)
{
    SINT32 i;
    //assert(id < HAL_AUDIO_RESOURCE_SWITCH);//rhal do

    if((i = GetConnectInputSourceIndex(ResourceStatus[id], inputConnect)) == -1)
    {
        ERROR("(%s)has no pin connected to %s \n", GetResourceString(id),  GetResourceString(inputConnect));

        return FALSE;
    }

    if((ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_CONNECT) &&
       (ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_STOP))
    {
        ERROR("[AUDH] %s disconnect pin id %s failed status is at %s \n",GetResourceString(id),
                    GetResourceString(ResourceStatus[id].inputConnectResourceId[i]), GetResourceStatusString((SINT32)ResourceStatus[id].connectStatus[i]));
        //fixed WOSQRTK-1793
        //return FALSE;
    }

    if(IsAOutSource(id))
    {

    }
    else
    {
        //assert(ResourceStatus[id].maxVaildIptNum == 1);//rhal do
        //assert(i == 0);//rhal do
    }

    CleanConnectInputSourceAndStatus(ResourceStatus, id, inputConnect);

    return TRUE;
}

static void SetConnectSourceAndStatus(HAL_AUDIO_MODULE_STATUS resourceStatus[HAL_MAX_RESOURCE_NUM],
                                      HAL_AUDIO_RESOURCE_T currentConnect,
                                      HAL_AUDIO_RESOURCE_T inputConnect)
{
    SINT32 i, index;

    // set current module's input id
    index = resourceStatus[currentConnect].numIptconnect;
    //assert(index <  resourceStatus[currentConnect].maxVaildIptNum); // check size //rhal do

    // check if has been connected
    i = GetConnectInputSourceIndex(resourceStatus[currentConnect], inputConnect);
    //assert(i == -1);//rhal do
    resourceStatus[currentConnect].inputConnectResourceId[index] = inputConnect;
    resourceStatus[currentConnect].connectStatus[index]          = HAL_AUDIO_RESOURCE_CONNECT;
    resourceStatus[currentConnect].numIptconnect++;

    // set input module's output Id

    if(HAL_AUDIO_RESOURCE_NO_CONNECTION == inputConnect) // AMXI no connectinput
        return;

    index = resourceStatus[inputConnect].numOptconnect;

    if(index >= HAL_DEC_MAX_OUTPUT_NUM)
    {
        AUDIO_FATAL("[AUDH-FATAL] Outputs of %s = (%d)\n", GetResourceString(inputConnect), index);
        //assert(index < HAL_DEC_MAX_OUTPUT_NUM);//rhal do
        return;
    }
    resourceStatus[inputConnect].outputConnectResourceID[index] = currentConnect;
    resourceStatus[inputConnect].numOptconnect++;

    return;
}

static void GetConnectOutputSource(HAL_AUDIO_RESOURCE_T resID,
                                   HAL_AUDIO_RESOURCE_T* outputConnectResId,
                                   SINT32 numOutputConnectArray,
                                   SINT32* totalOutputConnectResource)
{
    SINT32 minArraySize;
    SINT32 i;

    *totalOutputConnectResource = ResourceStatus[resID].numOptconnect;

    if(numOutputConnectArray <= *totalOutputConnectResource)
        minArraySize = numOutputConnectArray;
    else
        minArraySize = *totalOutputConnectResource;

    for(i = 0; i < minArraySize; i++)
    {
        outputConnectResId[i] = ResourceStatus[resID].outputConnectResourceID[i];
    }
    return;
}

AUDIO_DEC_TYPE Convert2AudioDecTypefrmHal(HAL_AUDIO_SRC_TYPE_T fmt)
{
    AUDIO_DEC_TYPE dec_type = AUDIO_UNKNOWN_TYPE;

    switch(fmt)
    {
           case HAL_AUDIO_SRC_TYPE_PCM:
               dec_type = AUDIO_LPCM_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_AC3:
               dec_type = AUDIO_AC3_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_EAC3:
           case HAL_AUDIO_SRC_TYPE_EAC3_ATMOS:
                dec_type = AUDIO_DDP_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_MPEG:
                dec_type = AUDIO_MPEG_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_AAC:
           case HAL_AUDIO_SRC_TYPE_HEAAC:
                dec_type= AUDIO_AAC_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_DTS:
           case HAL_AUDIO_SRC_TYPE_DTS_CD:
           case HAL_AUDIO_SRC_TYPE_DTS_EXPRESS:
           case HAL_AUDIO_SRC_TYPE_DTS_HD_MA:
                dec_type = AUDIO_DTS_DECODER_TYPE;
               break;
           case HAL_AUDIO_SRC_TYPE_MAT:
           case HAL_AUDIO_SRC_TYPE_MAT_ATMOS:
                dec_type = AUDIO_MAT_DECODER_TYPE;
               break;

           default:
             break;
    }

    return dec_type;
}

AUDIO_DEC_TYPE Convert2AudioDecType(HAL_AUDIO_SRC_TYPE_T audioSrcType)
{
    AUDIO_DEC_TYPE reType = AUDIO_UNKNOWN_TYPE;
    switch(audioSrcType)
    {
        default :
        case HAL_AUDIO_SRC_TYPE_UNKNOWN :
        case HAL_AUDIO_SRC_TYPE_NONE:
            ERROR("[AUDH] unknow type %x\n", audioSrcType);
            break;

        case HAL_AUDIO_SRC_TYPE_PCM : // need other infomation
            ERROR("[AUDH] unknow type %x\n", audioSrcType);
            break;

        case HAL_AUDIO_SRC_TYPE_AC3 :
            reType = AUDIO_AC3_DECODER_TYPE;
            break;

        case HAL_AUDIO_SRC_TYPE_EAC3 :
        case HAL_AUDIO_SRC_TYPE_EAC3_ATMOS:
            reType = AUDIO_DDP_DECODER_TYPE;
            break;

        case HAL_AUDIO_SRC_TYPE_MP3 :
        case HAL_AUDIO_SRC_TYPE_MPEG :
            reType = AUDIO_MPEG_DECODER_TYPE;
            break;

        case HAL_AUDIO_SRC_TYPE_AAC :
        case HAL_AUDIO_SRC_TYPE_HEAAC :
            reType = AUDIO_AAC_DECODER_TYPE; // LATM , ADTS ?
            break;

        case HAL_AUDIO_SRC_TYPE_DRA :
            reType = AUDIO_DRA_DECODER_TYPE;
            break;

        case HAL_AUDIO_SRC_TYPE_MPEG_H:
            ERROR("[AUDH] not yet implemnet type %x\n", audioSrcType);
            break;

        case HAL_AUDIO_SRC_TYPE_AC4:
        case HAL_AUDIO_SRC_TYPE_AC4_ATMOS:
            reType = AUDIO_AC4_DECODER_TYPE;
            break;

        case HAL_AUDIO_SRC_TYPE_DTS :
            reType = AUDIO_DTS_DECODER_TYPE; // tv030 need to support DTS
            break;

        case HAL_AUDIO_SRC_TYPE_DTS_HD_MA :
            ERROR("[AUDH] not yet implemnet type %x\n", audioSrcType);
            break;

         case HAL_AUDIO_SRC_TYPE_DTS_EXPRESS :
            ERROR("[AUDH] not yet implemnet type %x\n", audioSrcType);
            break;

         case HAL_AUDIO_SRC_TYPE_DTS_CD :
            ERROR("[AUDH] not yet implemnet type %x\n", audioSrcType);
            break;
    }

    return reType;
}

void FillConnectedOutput(HAL_AUDIO_RESOURCE_T resId)
{
    HAL_AUDIO_RESOURCE_T decOptResourceId[5];
    SINT32 totalDecOutputConnectResource;
    CHAR buffer[16];

    GetConnectOutputSource(resId, decOptResourceId, 5, &totalDecOutputConnectResource);
    if(totalDecOutputConnectResource > 0)
    {
        SINT32 i;
        strncat(AUD_StringBuffer, "---(", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        for(i = 0; i < totalDecOutputConnectResource; i++)
        {
            snprintf(&buffer[0], sizeof(buffer)," %d.", i);
            strncat(AUD_StringBuffer, buffer, sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            strncat(AUD_StringBuffer, GetResourceString(decOptResourceId[i]), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

            if(IsSESource(decOptResourceId[i]))
            {
                HAL_AUDIO_RESOURCE_T seOptResourceId[5];
                SINT32 totalSeOutputConnectResource;
                GetConnectOutputSource(decOptResourceId[i], seOptResourceId, 5, &totalSeOutputConnectResource);

                if(totalSeOutputConnectResource >= 1)
                {

                    strncat(AUD_StringBuffer, "---", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                    strncat(AUD_StringBuffer, GetResourceString(seOptResourceId[0]), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                }
                else
                    strncat(AUD_StringBuffer, "---X", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

            }
            strncat(AUD_StringBuffer, ",", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        }
        if(totalDecOutputConnectResource > 0)
            strncat(AUD_StringBuffer, ")", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
    }
    else
    {
        strncat(AUD_StringBuffer, "---no output", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
    }
}

HAL_AUDIO_RESOURCE_T GetNonOutputModuleSingleInputResource(HAL_AUDIO_MODULE_STATUS resourceStatus)
{
    ////assert(resourceStatus.numIptconnect <= 1);//rhal do
    return resourceStatus.inputConnectResourceId[0];
}

void FillDecInput(HAL_AUDIO_RESOURCE_T adecId)
{
    HAL_AUDIO_RESOURCE_T decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[adecId]);

    if(decIptResId < HAL_MAX_RESOURCE_NUM)
    {
        // dec input

        // pipe2

        HAL_AUDIO_RESOURCE_T sdecResId = GetNonOutputModuleSingleInputResource(ResourceStatus[decIptResId]);

        // drow pipe 1
        if(sdecResId  < HAL_MAX_RESOURCE_NUM)
        {
            strncat(AUD_StringBuffer, GetResourceString(sdecResId), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            strncat(AUD_StringBuffer, "---", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        }

        strncat(AUD_StringBuffer, GetResourceString(decIptResId), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

        strncat(AUD_StringBuffer, "---", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

        // draw pipe 3
        strncat(AUD_StringBuffer, GetResourceString(adecId), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
    }
    else
    {
        strncat(AUD_StringBuffer, "no input---", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        strncat(AUD_StringBuffer, GetResourceString(adecId), sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
    }
}

void ShowFlow(HAL_AUDIO_RESOURCE_T adecResId, HAL_AUDIO_RESOURCE_STATUS newStatus, SINT32 forcePrint)
{
    SINT32 i;
    SINT32 mainDecResId;
    HAL_AUDIO_RESOURCE_T mixerOptResourceId[5];
    HAL_AUDIO_RESOURCE_T decId[2];
    SINT32 totalmixerOutputConnectResource;

    mainDecResId = adec2res((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex);

    // check input
    // Amixer connect data
    for(i = (SINT32)HAL_AUDIO_RESOURCE_MIXER0; i <= HAL_AUDIO_RESOURCE_MIXER7 ; i++)
    {
        GetConnectOutputSource(((HAL_AUDIO_RESOURCE_T) i), mixerOptResourceId, 5, &totalmixerOutputConnectResource);
        memset(AUD_StringBuffer, 0, sizeof(AUD_StringBuffer));
        snprintf(AUD_StringBuffer, sizeof(AUD_StringBuffer), "AMixer%d:: status (%s)", (i-HAL_AUDIO_RESOURCE_MIXER0), GetResourceStatusString((SINT32)(ResourceStatus[((HAL_AUDIO_RESOURCE_T) i)].connectStatus[0])));
        //if(totalmixerOutputConnectResource >= 1)
        {
            FillConnectedOutput((HAL_AUDIO_RESOURCE_T) i);
        }
       // else
        {
            //strcat(AUD_StringBuffer, "AMixer%d  no connect to output", i);
        }

        if(forcePrint)
            INFO("%s\n", AUD_StringBuffer);
        else
            INFO("%s\n", AUD_StringBuffer);
     }


    // dec 0
    decId[0] = HAL_AUDIO_RESOURCE_ADEC0;
    decId[1] = HAL_AUDIO_RESOURCE_ADEC1;
    for(i = 0; i < 2; i++)
    {
        SINT32 DecResId =  decId[i];
        memset(AUD_StringBuffer, 0, sizeof(AUD_StringBuffer));// clean
        if(mainDecResId == DecResId)
        {
              strncat(AUD_StringBuffer, "Main:: ", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        }
        else
        {
            if(mainDecResId != -1)
            {
                strncat(AUD_StringBuffer, " Sub:: ", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            }
            else
            {
                strncat(AUD_StringBuffer, "Unknown Main ID:: ", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            }
        }

        FillDecInput(decId[i]);

        if(ResourceStatus[decId[i]].connectStatus[0] == HAL_AUDIO_RESOURCE_RUN)
        {

            if(decId[i] == adecResId)
            {
                if(newStatus == HAL_AUDIO_RESOURCE_RUN)
                    strncat(AUD_StringBuffer, "(Run  to  Run)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_STOP)
                    strncat(AUD_StringBuffer, "(Run  to  Stop)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_PAUSE)
                    strncat(AUD_StringBuffer, "(Run  to  Pause)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else
                    strncat(AUD_StringBuffer, "(Running State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            }
            else
                strncat(AUD_StringBuffer, "(Running State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

        }
        else if(ResourceStatus[decId[i]].connectStatus[0] == HAL_AUDIO_RESOURCE_PAUSE)
        {

            if(decId[i] == adecResId)
            {
                if(newStatus == HAL_AUDIO_RESOURCE_RUN)
                    strncat(AUD_StringBuffer, "(Pause  to Run)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_STOP)
                    strncat(AUD_StringBuffer, "(Pause  to  Stop)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_PAUSE)
                    strncat(AUD_StringBuffer, "(Pause  to  Pause)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else
                    strncat(AUD_StringBuffer, "(Pause   State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            }
            else
                strncat(AUD_StringBuffer, "(Pause   State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        }
        else
        {
            if(decId[i] == adecResId)
            {
                if(newStatus == HAL_AUDIO_RESOURCE_RUN)
                    strncat(AUD_StringBuffer, "(Stop  to  Run)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_STOP)
                    strncat(AUD_StringBuffer, "(Stop  to  Stop)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else if(newStatus == HAL_AUDIO_RESOURCE_PAUSE)
                    strncat(AUD_StringBuffer, "(Stop  to  Pause)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
                else
                    strncat(AUD_StringBuffer, "(Stop    State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
            }
            else
                strncat(AUD_StringBuffer, "(Stop    State)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);

            if(GeDectUserState(i) == 1)
                strncat(AUD_StringBuffer, "(MiddleWare setting is Run)", sizeof(AUD_StringBuffer) - strlen(AUD_StringBuffer) - 1);
        }
        FillConnectedOutput(decId[i]);
       if(forcePrint)
            INFO("%s\n", AUD_StringBuffer);
        else
            INFO("%s\n", AUD_StringBuffer);
    }
}

static void GetRingBufferInfo(RINGBUFFER_HEADER* pRingBuffer, ULONG* pbase,
    ULONG* prp, ULONG* pwp , ULONG* psize
    )
{
    ULONG base, rp, wp, size;

    INFO("ring buffer %p  \n", pRingBuffer);
    if(pRingBuffer == NULL)
        return;


    rp   = IPC_ReadU32((BYTE*) &(pRingBuffer->readPtr[0]));
    base = IPC_ReadU32((BYTE*) &(pRingBuffer->beginAddr));
    wp   = IPC_ReadU32((BYTE*) &(pRingBuffer->writePtr));
    size  = IPC_ReadU32((BYTE*) &(pRingBuffer->size));
    INFO(" baddr %x rp %x wp %x size %x \n",  base, rp, wp, size);
    //assert(rp < (base) + (size));//rhal do
    //assert(wp < (base) + (size));//hhal do

    if(pbase)
        *pbase = base;

    if(pwp)
        *pwp = wp;

    if(prp)
        *prp = rp;

    if(psize)
        *psize = size;

}

DTV_STATUS_T AUDIO_Get_MediaSourceLatency(AUDIO_LATENCY_INFO *latency_info)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 ret = 0;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id  = AUDIO_CONFIG_CMD_GET_LATENCY;
    audioConfig.value[0] = latency_info->iptSrc;
    audioConfig.value[1] = latency_info->sndOut;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);
    if(ret != 0){
        ERROR("[AUDH][%s %d] fail\n",__func__,__LINE__);
        return NOT_OK;
    }

    latency_info->latency = audioConfig.value[2];

    return OK;
}

HAL_AUDIO_SRC_TYPE_T Convert2HalAudioDecType(AUDIO_DEC_TYPE audioSrcType, SLONG reserved)
{
    HAL_AUDIO_SRC_TYPE_T  src_type = HAL_AUDIO_SRC_TYPE_UNKNOWN;
    SINT32 version = 0;
    UINT32 strm_type;
    UINT32 nuframe_type;
    switch(audioSrcType)
    {
        default :
        case AUDIO_UNKNOWN_TYPE :
            break;
        case AUDIO_MPEG_DECODER_TYPE :
            src_type = HAL_AUDIO_SRC_TYPE_MPEG;
            break;
        case AUDIO_AC3_DECODER_TYPE :
            src_type = HAL_AUDIO_SRC_TYPE_AC3;
            break;
        case AUDIO_LPCM_DECODER_TYPE :
             src_type = HAL_AUDIO_SRC_TYPE_PCM;
            break;
        case AUDIO_DTS_DECODER_TYPE :
        case AUDIO_DTS_HD_DECODER_TYPE :
            // reserved[1]: 0x80=LBR, 0x100=XLL reference from dtshd_dec_api_common.h
            /*
            typedef enum DTS_DECINFO_STRMTYPE_MASK {               DTS Bitstream Type         OSD Display
                DTSXDECSTRMTYPE_UNKNOWN              = 0x00000000, DTS:X (P2)                 DTS:X
                DTSXDECSTRMTYPE_DTS_LEGACY           = 0x00000001, DTS Core                   DTS
                DTSXDECSTRMTYPE_DTS_ES_MATRIX        = 0x00000002, DTS-ES Matrix              DTS
                DTSXDECSTRMTYPE_DTS_ES_DISCRETE      = 0x00000004, DTS-ES Discrete            DTS
                DTSXDECSTRMTYPE_DTS_9624             = 0x00000008, DTS 96/24                  DTS
                DTSXDECSTRMTYPE_DTS_ES_8CH_DISCRETE  = 0x00000010, DTS-ES Discrete            DTS
                DTSXDECSTRMTYPE_DTS_HIRES            = 0x00000020, DTS-HD HiRes               DTS-HD
                DTSXDECSTRMTYPE_DTS_MA               = 0x00000040, DTS-HD Master Audio        DTS-HD
                DTSXDECSTRMTYPE_DTS_LBR              = 0x00000080, DTS Express                DTS-HD
                DTSXDECSTRMTYPE_DTS_LOSSLESS         = 0x00000100, DTS-HD Master Audio        DTS-HD
                DTSXDECSTRMTYPE_DTS_UHD              = 0x00000200, DTS:X (P1)                 DTS:X
                DTSXDECSTRMTYPE_DTS_UHD_MA           = 0x00000400, DTS:X Master Audio (P1)    DTS:X
                DTSXDECSTRMTYPE_DTS_UHD_GAME         = 0x00000800  DTS:X (P1)                 DTS:X
            } DTSXDECSTRMTYPE;
            */
            strm_type = reserved & 0x00ffffff;
            nuframe_type = (reserved >> 24) & 0xff;
            INFO("%s strm_type=%d, nuframe_type=%d\n", __func__, strm_type, nuframe_type);
            if(strm_type == 0x20 || strm_type == 0x80) {
                src_type = HAL_AUDIO_SRC_TYPE_DTS_EXPRESS;
            } else if(strm_type == 0x40 || strm_type == 0x100) {
                src_type = HAL_AUDIO_SRC_TYPE_DTS_HD_MA;
            } else if((strm_type == 0 && nuframe_type == DTS_PARSER_SYNC_TYPE_FTOC )
                || strm_type == 0x200 || strm_type == 0x400 || strm_type == 0x800) {
                src_type = HAL_AUDIO_SRC_TYPE_DTS_X;
            } else {
                src_type = HAL_AUDIO_SRC_TYPE_DTS;
            }
            break;
        case AUDIO_WMA_DECODER_TYPE :
            break;
        case AUDIO_AAC_DECODER_TYPE :
            //-- reserved[1] = ((VERSION<<8) & 0xFF00) | (FORMAT & 0x00FF)
            // bit [0:7]  for format  /* LOAS/LATM = 0x0, ADTS = 0x1 */
            // bit [8:15]  for version   /* AAC = 0x0, HE-AACv1 = 0x1, HE-AACv2 = 0x2 */
            version =  (reserved >> 8) & 0xFF ;
             if(version != 0)
                src_type = HAL_AUDIO_SRC_TYPE_HEAAC;
             else
                src_type = HAL_AUDIO_SRC_TYPE_AAC;
            break;
        case AUDIO_VORBIS_DECODER_TYPE :
            break;
        case AUDIO_DV_DECODER_TYPE :
            break;
        case AUDIO_DDP_DECODER_TYPE :
            version = reserved & 0x1;
            if(version)
                src_type = HAL_AUDIO_SRC_TYPE_EAC3_ATMOS;
            else
                src_type = HAL_AUDIO_SRC_TYPE_EAC3;
            break;
        case AUDIO_AC4_DECODER_TYPE:
            version = reserved & 0x1;
            if(version)
                src_type = HAL_AUDIO_SRC_TYPE_AC4_ATMOS;
            else
                src_type = HAL_AUDIO_SRC_TYPE_AC4;
            break;
        case AUDIO_MLP_DECODER_TYPE :
        case AUDIO_MAT_DECODER_TYPE:
            version = reserved & 0x1;
            if(version)
                src_type = HAL_AUDIO_SRC_TYPE_MAT_ATMOS;
            else
                src_type = HAL_AUDIO_SRC_TYPE_MAT;
            break;
        case AUDIO_WMA_PRO_DECODER_TYPE :
            break;
        case AUDIO_MP4AAC_DECODER_TYPE :
            break;
        case AUDIO_MP3_PRO_DECODER_TYPE :
            break;
        case AUDIO_MP4HEAAC_DECODER_TYPE :
            break;
        case AUDIO_RAW_AAC_DECODER_TYPE :
            break;
        case AUDIO_RA1_DECODER_TYPE :
            break;
        case AUDIO_RA2_DECODER_TYPE :
            break;
        case AUDIO_ATRAC3_DECODER_TYPE :
            break;
        case AUDIO_COOK_DECODER_TYPE :
            break;
        case AUDIO_LSD_DECODER_TYPE :
            break;
        case AUDIO_ADPCM_DECODER_TYPE :
            break;
        case AUDIO_FLAC_DECODER_TYPE :
            break;
        case AUDIO_ULAW_DECODER_TYPE :
            break;
        case AUDIO_ALAW_DECODER_TYPE :
            break;
        case AUDIO_ALAC_DECODER_TYPE :
            break;
        case AUDIO_DTS_HIGH_RESOLUTION_DECODER_TYPE :
            break;
        case AUDIO_DTS_LBR_DECODER_TYPE :
            break;
        case AUDIO_DTS_MASTER_AUDIO_DECODER_TYPE :
            break;
        case AUDIO_AMRNB_DECODER_TYPE :
            break;
        case AUDIO_MIDI_DECODER_TYPE :
            break;
        case AUDIO_APE_DECODER_TYPE :
            break;
        case AUDIO_AVS_DECODER_TYPE :
            break;
        case AUDIO_NELLYMOSER_DECODER_TYPE :
            break;
        case AUDIO_WMA_LOSSLESS_DECODER_TYPE :
            break;
        case AUDIO_UNCERTAIN_DECODER_TYPE :
            break;
        case AUDIO_UNCERTAIN_HDMV_DECODER_TYPE :
            break;
        case AUDIO_ILBC_DECODER_TYPE :
            break;
        case AUDIO_SILK_DECODER_TYPE :
            break;
        case AUDIO_AMRWB_DECODER_TYPE :
            break;
        case AUDIO_G729_DECODER_TYPE :
            break;
        case AUDIO_DRA_DECODER_TYPE :
             src_type = HAL_AUDIO_SRC_TYPE_DRA;
            break;
        case AUDIO_OPUS_DECODER_TYPE :
            break;
    }

   return src_type;
}

static BOOLEAN inline SetResourceConnect(HAL_AUDIO_RESOURCE_T id, HAL_AUDIO_RESOURCE_T inputConnect)
{
    SINT32 i;
    SINT32 useInputPinId;
    //assert(id < HAL_AUDIO_RESOURCE_MAX);//rhal do

    if(IsAOutSource(id)) //  connect according to pin
    {
        // check nerver been connected
        if((i = GetConnectInputSourceIndex(ResourceStatus[id], inputConnect)) != -1)
        {
            ERROR("%s pin %d has connected to %s\n",
                    GetResourceString(id), i, GetResourceString(inputConnect));
            return FALSE;
        }
    }
    else
    {
        // normal case only need to check  connectStatus[0]
        if((ResourceStatus[id].connectStatus[0] != HAL_AUDIO_RESOURCE_OPEN) &&
           (ResourceStatus[id].connectStatus[0] != HAL_AUDIO_RESOURCE_DISCONNECT))
        {
            ERROR("[AUDH] connect %s failed status is %d \n",
                    GetResourceString(id), ResourceStatus[id].connectStatus[0]);
            return FALSE;
        }

        //assert((ResourceStatus[id].maxVaildIptNum == 1) && (ResourceStatus[id].numIptconnect ==0));//rhal do
    }

    useInputPinId = ResourceStatus[id].numIptconnect;
    if(IsAOutSource(id))
    {
        //assert(useInputPinId <=  ResourceStatus[id].maxVaildIptNum);//rhal do
    }
    else
    {// normal case
        //assert(useInputPinId == 0);//rhal do
    }

    SetConnectSourceAndStatus(ResourceStatus, id, inputConnect);

    return TRUE;
}

static BOOLEAN inline SetResourceClose(HAL_AUDIO_RESOURCE_T id)
{
    SINT32 i, statusFailed;
    //assert(id < HAL_AUDIO_RESOURCE_MAX);//rhal do

    statusFailed = 0;
    // check all status is at right status
    for(i = 0; i < HAL_MAX_OUTPUT; i++)
    {
        if((ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_OPEN) &&
           (ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_DISCONNECT) &&
           (ResourceStatus[id].connectStatus[i] != HAL_AUDIO_RESOURCE_CLOSE))
        {
            ERROR("[AUDH] close %s failed input pin (%d) status is %s \n",
                    GetResourceString(id), i, GetResourceStatusString(ResourceStatus[id].connectStatus[i]));

            if(ResourceStatus[id].connectStatus[i] == HAL_AUDIO_RESOURCE_CONNECT)
            {
                ERROR("[AUDH] close %s failed input pin (%d) still connect to  %s \n",
                    GetResourceString(id), i, GetResourceString(ResourceStatus[id].inputConnectResourceId[i]));
            }

            statusFailed = 1;
        }
    }

    if(statusFailed )
        return FALSE;

    for(i = 0; i < HAL_MAX_OUTPUT; i++)
    {
        ResourceStatus[id].connectStatus[i] = HAL_AUDIO_RESOURCE_CLOSE;
    }

    return TRUE;
}

void DeleteDbxTvTable(void)
{
    if (dbxTableAddr != NULL) {
        rtkaudio_free(dbxTablePhyAddr);
        dbxTableAddr = NULL;
    }
}


#ifdef DUMP_HAL_DBG
void static OpenHALDumpFile(void)
{
    #define HAL_DUMP_PATH ("/tmp/audio_hal_dump.txt")
    g_hal_file = fopen(HAL_DUMP_PATH, "wt");
}

void static CloseHALDumpFile(void)
{
    if(g_hal_file != NULL)
        fclose(g_hal_file);
    g_hal_file = NULL;
}
#endif

void Init_ATV_Driver(HAL_AUDIO_SIF_TYPE_T eSifType)
{
    //KADP_AUDIO_HW_IOCT_Init();//only needed in rhal
    KADP_Audio_HwpAADSIFInit(NULL);//check initial value....
    if (rhal_type == RHAL_LITE)
        g_AudioSIFInfo.curSifType = HAL_AUDIO_SIF_DVB_CN_SELECT;
    else g_AudioSIFInfo.curSifType = eSifType;
    KADP_Audio_HwpCurSifType( (ATV_AUDIO_SIF_TYPE_T) eSifType);
    KADP_Audio_AtvSetDeviationMethod(ATV_DEV_CHANGE_BY_USER);
    KADP_Audio_AtvEnableAutoChangeSoundModeFlag(1);//enable
}

void DeInit_ATV_Driver(void)
{
    KADP_Audio_AtvInit(NULL);//check initial value....
    g_AudioSIFInfo.curSifType     = HAL_AUDIO_SIF_TYPE_NONE;
    KADP_Audio_HwpCurSifType( (ATV_AUDIO_SIF_TYPE_T) g_AudioSIFInfo.curSifType);
    KADP_Audio_HwpAADSIFFinalize();

	//KADP_AUDIO_HW_IOCT_Finalize();//rhal do
}

DTV_STATUS_T Init_Audio_Driver(void)
{
    //KADP_AUDIO_Init();//rhal do
    return OK;
}

void DeInit_Audio_Driver(void)
{
	DeleteDbxTvTable();
    //KADP_AUDIO_Finalize();//rhal do
}

/*********** End of HAL Volume to ADSP Gain related functions *********************/

//DTV_STATUS_T RAL_AUDIO_StopDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex);



#if 0 //rhal do
static void thread_sched_attr(SINT32 policy, struct sched_param *param)
{
    INFO("policy=%s, priority=%d\n",
                (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                (policy == SCHED_RR)    ? "SCHED_RR" :
                (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                "???",
                param->sched_priority);
}

static void show_audio_pcmcap_thread_sched_attr(void)
{
    SINT32 policy, s;
    struct sched_param param;

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    if (s != 0)
        return;

    thread_sched_attr(policy, &param);

    /*if(policy != SCHED_RR) {
        SINT32 max_prio = sched_get_priority_max(SCHED_RR);
        param.sched_priority = max_prio - 5;
        pthread_setschedparam(pthread_self(), (SINT32)SCHED_RR, &param);
    }*/
    //if(policy != SCHED_FIFO) {
        SINT32 max_prio = sched_get_priority_max(SCHED_FIFO);
        param.sched_priority = max_prio;
        pthread_setschedparam(pthread_self(), (SINT32)SCHED_FIFO, &param);
    //}
}
#endif

#ifdef PCM_FILE_DUMP

//FILE* file_dump_pcm = NULL; // ori in_ring data

//FILE* file_dump_pcm_16bit = NULL; // after invert


DTV_STATUS_T DummyCopy(UINT8 *pBuf, UINT16 length)
{
    //FIXME: only add return.
    return OK;
}

#if 0 //rhal do
void DumpAudioPcmCapToFile(void *data, SINT32 newReadSize)
{
    RAL_AUDIO_PCMCap_T *pcmCap = (RAL_AUDIO_PCMCap_T*)data;
    /* data buffer from pcm capture */
    UINT8 *bufL;
    UINT8 *bufBaseL, *bufLimitL;
    UINT32 bufSize;
    SINT32 readSize;

    //if ((access(DUMP_PCM_CAP_PATH, F_OK) == 0)) //rhal do
    {
    #if 0 //rhal do
        if(file_dump_pcm == NULL)
        {
            file_dump_pcm = fopen("/tmp/AudioPcmDumpFile_48k_32_mono.pcm", "wb");
            ERROR("%s:%d open file descriptor of dump pcm file\n",__func__,__LINE__);

            file_dump_pcm_16bit = fopen("/tmp/AudioPcmDumpFile_48k_16_2ch.pcm", "wb");
            ERROR("%s:%d open file descriptor of dump bit 16 pcm file\n",__func__,__LINE__);

            g_dumpBuffer = (UINT8*)malloc(AUDIO_AOUT_OUTPUT_BUF_SIZE);;
            //assert(file_dump_pcm != NULL);//rhal do
            //assert(file_dump_pcm_16bit != NULL);//rhal do
            //assert(g_dumpBuffer != NULL);//rhal do
        }
    #endif
        /* data buffer send to callback function */
        UINT8 *pData = g_dumpBuffer;
        Pin* pinL = pcmCap->ao_outPin;
        //memset(pData, 0, AUDIO_AOUT_OUTPUT_BUF_SIZE);

        pinL->GetBuffer(pinL, (UINT8**)&bufBaseL, (UINT32*)&bufSize);
        bufLimitL = (UINT8*)(bufBaseL+bufSize);
        bufL = (UINT8*)pinL->GetReadPtr(pinL, 0);

        if(newReadSize == -1)
            readSize = (SINT32)pinL->GetReadSize(pinL, 0);
        else
            readSize = newReadSize;
        //INFO("%s:%d dump audio pcm data to file, size=%d\n",__func__,__LINE__,readSize);

        if(pData != NULL)
        {
            if(bufL + readSize < bufLimitL)
            {
                memcpy(pData,bufL,readSize);
            }
            else
            {
                UINT32 part1Size = (bufLimitL - bufL);
                UINT32 part2Size = (readSize - part1Size);
                memcpy(pData,bufL, part1Size);
                memcpy(pData+part1Size,bufBaseL, part2Size);
            }
#if 0 //rhal do
            if(file_dump_pcm != NULL)
            {
                g_ringsize += readSize;
                fwrite(pData,1 , readSize, file_dump_pcm);
            }
#endif
        }

        //free(pData);

        g_prevDumpWriteTime = pli_getPTS();

    }
    else
    {
#if 0 //rhal do
        if(file_dump_pcm != NULL)
        {
            if(pli_getPTS() >= (g_prevDumpWriteTime + (15*90*1000))) // wait 15 sec to finsh dump
            {
                ERROR("%s: finish close file descriptor of dump pcm file\n",__func__);
                if(file_dump_pcm)
                {
                    fclose(file_dump_pcm);
                    g_ringsize = 0;
                }

                if(file_dump_pcm_16bit)
                {
                    fclose(file_dump_pcm_16bit);
                    g_16bitsize = 0;
                }

                if(g_dumpBuffer)
                    free(g_dumpBuffer);

                file_dump_pcm = NULL;
                file_dump_pcm_16bit = NULL;
            }
        }
#endif
    }
/*
    if ((access(ENABLE_DUMMY_CAP_PATH, F_OK) == 0))
    {
        if( g_haveStartCap == 0)
        {

            HAL_AUDIO_PCM_RegSendPCMCallback(HAL_AUDIO_PCM_I2S, DummyCopy);
            HAL_AUDIO_PCM_StartUpload(HAL_AUDIO_PCM_I2S);
            g_haveStartCap = 1;
        }
    }
    else
    {
         g_haveStartCap = 0;
    }
*/
}
#endif

void DumpAudioPcmCapToFile_16Bit(void *pData, SINT32 size)
{
    /* data buffer from pcm capture */
    //if ((access(DUMP_PCM_CAP_PATH, F_OK) == 0)) //rhal do
    {
#if 0 //rhal do
        if(file_dump_pcm_16bit != NULL)
        {
            g_16bitsize += size;
            fwrite(pData,1 , size, file_dump_pcm_16bit);
        }
#endif
    }
    else
    {

    }
}
#endif

static void InitialResourceStatus(void)
{
    SINT32 i,j;

    for(i =0; i < HAL_MAX_RESOURCE_NUM; i++)
    {
        memcpy(ResourceStatus[i].name, ResourceName[i], sizeof(ResourceStatus[i].name));
        for(j=0; j < HAL_MAX_OUTPUT; j++)
        {
            ResourceStatus[i].inputConnectResourceId[j] = HAL_AUDIO_RESOURCE_NO_CONNECTION;
            ResourceStatus[i].outputConnectResourceID[j] = HAL_AUDIO_RESOURCE_NO_CONNECTION;
            ResourceStatus[i].connectStatus[j] = HAL_AUDIO_RESOURCE_CLOSE;
        }

        ResourceStatus[i].numOptconnect = 0;
        ResourceStatus[i].numIptconnect = 0;

        if(IsAOutSource((HAL_AUDIO_RESOURCE_T)i))
            ResourceStatus[i].maxVaildIptNum = 10;//adec 0 , 1, amix0~7
        else
            ResourceStatus[i].maxVaildIptNum = 1; // normal module
    }

    Aud_mainDecIndex = 0;// no main dec now
    Aud_prevMainDecIndex = -1;
    Aud_descriptionMode = 0;

    memset(&MainFlowStatus, 0, sizeof(MainFlowStatus));
    memset(&SubFlowStatus, 0, sizeof(SubFlowStatus));

    memset(&g_AudioStatusInfo,0, sizeof(g_AudioStatusInfo));
    memset(&Aud_Adec_info[0], 0,  sizeof(HAL_AUDIO_ADEC_INFO_T));
    memset(&Aud_Adec_info[1], 0,  sizeof(HAL_AUDIO_ADEC_INFO_T));


    for(i =0; i < AUD_ADEC_MAX; i++)
    {
        SetDecDrcMode(i, HAL_AUDIO_DOLBY_RF_MODE);// initial
        SetDecDownMixMode(i, HAL_AUDIO_LORO_MODE);
    }

}

static void InitialADCStatus(void)
{
    SINT32 i;
    for(i = 0; i < MAIN_AIN_ADC_PIN; i++)
    {
        AinStatus.ainPinStatus[i] = HAL_AUDIO_RESOURCE_CLOSE;
    }
}

static void InitialDecStatus(void)
{
    SINT32 i;
    SINT32 j;
    HAL_AUDIO_VOLUME_T default_volume = {.mainVol = 0x7F, .fineVol = 0x0};
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        SetSPDIFOutType(i, HAL_AUDIO_SPDIF_PCM);
        //SetTrickPause(i, FALSE);
        SetTrickState(i, HAL_AUDIO_TRICK_NONE);

        SetDecInMute(i, FALSE);
        SetDecESMute(i, FALSE);
        SetDecInVolume(i, default_volume);
        for(j = 0; j < AUD_MAX_CHANNEL; j++)
            SetDecOutVolume(i, j, default_volume);
    }
}

static UINT32 ADSP_TSK_GetStarted(void)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_GET_TASK_STARTED;
    return rtkaudio_send_audio_config(&audioConfig);
}

static DTV_STATUS_T CreateAINFilter(void)
{
    Aud_MainAin = new_AIN();
    if(Aud_MainAin == NULL)
    {
        ERROR("create main AIN failed\n");
        return NOT_OK;
    }
    Aud_SubAin = new_AIN();
    if(Aud_SubAin == NULL)
    {
        ERROR("create sub AIN failed\n");
        return NOT_OK;
    }
    return OK;
}

static DTV_STATUS_T CreateAINFilterQuickShow(QuickShowCreateParam param)
{
    Aud_MainAin = new_AIN_QuickShow(param);
    if(Aud_MainAin == NULL)
    {
        ERROR("create main AIN failed\n");
        return NOT_OK;
    }
    return OK;
}

static void DeleteAINFilter(void)
{
    INFO("[AUDH] %s \n", __func__);

    if(Aud_SubAin != NULL)
    {
        Aud_SubAin->Stop(Aud_SubAin);
        Aud_SubAin->Delete(Aud_SubAin);
        Aud_SubAin = NULL;
    }

    if(Aud_MainAin != NULL)
    {
        Aud_MainAin->Stop(Aud_MainAin);
        Aud_MainAin->Delete(Aud_MainAin);
        Aud_MainAin = NULL;
    }
}

static DTV_STATUS_T CreateFlowManagerFilter(void)
{
    if((Aud_flow0 = new_flow()) == NULL)
    {
        ERROR("create audio flow DEC0 failed\n");
        return NOT_OK;
    }
    if((Aud_flow1 = new_flow()) == NULL)
    {
        ERROR("create audio flow DEC1 failed\n");
        return NOT_OK;
    }

    return OK;
}

static void DeleteFlowManagerFilter(void)
{
    INFO("[AUDH] %s \n", __func__);

    if(Aud_flow0 != NULL)
    {
        Aud_flow0->Stop(Aud_flow0);
        Aud_flow0->Delete(Aud_flow0);
        Aud_flow0 = NULL;
    }
    if(Aud_flow1 != NULL)
    {
        Aud_flow1->Stop(Aud_flow1);
        Aud_flow1->Delete(Aud_flow1);
        Aud_flow1 = NULL;
    }
}

static DTV_STATUS_T CreatePPAOFilter(void)
{
    if(Aud_ppAout == NULL)
    {
        Aud_ppAout = new_PPAO(PPAO_FULL);
        if(Aud_ppAout == NULL)
        {
            ERROR("create audio ppao failed\n");
            return NOT_OK;
        }
    }

    if(Aud_subPPAout == NULL)
    {
        Aud_subPPAout = new_PPAO(PPAO_FULL);
        if(Aud_subPPAout == NULL)
        {
            ERROR("create audio sub ppao failed\n");
            return NOT_OK;
        }
    }
    return OK;
}

static DTV_STATUS_T CreatePPAOFilterQuickShow(QuickShowCreateParam param)
{
    if(Aud_ppAout == NULL)
    {
        Aud_ppAout = new_PPAO_QuickShow(PPAO_FULL, param);
        if(Aud_ppAout == NULL)
        {
            ERROR("create audio ppao failed\n");
            return NOT_OK;
        }
    }

    if(Aud_subPPAout == NULL)
    {
        Aud_subPPAout = new_PPAO_QuickShow(PPAO_FULL, param);
        if(Aud_subPPAout == NULL)
        {
            ERROR("create audio sub ppao failed\n");
            return NOT_OK;
        }
    }
    return OK;
}

static DTV_STATUS_T DeletePPAOFilter(void)
{
    INFO("[AUDH] %s \n", __func__);

    if(Aud_subPPAout != NULL)
    {
        Aud_subPPAout->Stop(Aud_subPPAout);
        Aud_subPPAout->Delete(Aud_subPPAout);
        Aud_subPPAout = NULL;
    }

    if(Aud_ppAout != NULL)
    {
        Aud_ppAout->Stop(Aud_ppAout);
        Aud_ppAout->Delete(Aud_ppAout);
        Aud_ppAout = NULL;
    }

    return OK;
}

static DTV_STATUS_T CreateDecFilter(void)
{
    UINT32 i;
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        if(Aud_dec[i] == NULL)
        {
            Aud_dec[i] = new_DEC(i);
            if(Aud_dec[i] == NULL)
            {
                ERROR("create audio dec %d  failed\n", i);
                return NOT_OK;
            }
        }

        Aud_DTV[i] = new_DtvCom();

        if(Aud_DTV[i] == NULL)
        {
            ERROR("create audio dtv dec %d failed\n", i);
            return NOT_OK;
        }
    }

    return OK;
}

static DTV_STATUS_T CreateDecFilterQuickShow(QuickShowCreateParam param)
{
    UINT32 i;
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        if(Aud_dec[i] == NULL)
        {
            if(i == 0)
                Aud_dec[i] = new_DEC_QuickShow(param);
            else
                Aud_dec[i] = new_DEC(i);

            if(Aud_dec[i] == NULL)
            {
                ERROR("create audio dec %d  failed\n", i);
                return NOT_OK;
            }
        }

        Aud_DTV[i] = new_DtvCom();

        if(Aud_DTV[i] == NULL)
        {
            ERROR("create audio dtv dec %d failed\n", i);
            return NOT_OK;
        }
    }

    return OK;
}

static void DeleteDecFilter(void)
{
    UINT32 i;
    INFO("[AUDH] %s \n", __func__);

    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        if(Aud_dec[i] != NULL)
        {
            Aud_dec[i]->Stop(Aud_dec[i]);
            Aud_dec[i]->Delete(Aud_dec[i]);
            Aud_dec[i] = NULL;
        }

        if(Aud_DTV[i])
        {
            Aud_DTV[i]->Stop(Aud_DTV[i]);
            Aud_DTV[i]->Delete(Aud_DTV[i]);
        }
        Aud_DTV[i] = NULL;

    }
}

static DTV_STATUS_T CreateHdmiFmtNtfy(void)
{
    if(Aud_hdmiFmtNtfy == NULL)
    {
        //Aud_hdmiFmtNtfy = (RAL_AUDIO_HDMI_FMT_NTFY_T*)malloc(sizeof(RAL_AUDIO_HDMI_FMT_NTFY_T));
        Aud_hdmiFmtNtfy = (RAL_AUDIO_HDMI_FMT_NTFY_T*)kmalloc(sizeof(RAL_AUDIO_HDMI_FMT_NTFY_T), GFP_KERNEL);
        if(Aud_hdmiFmtNtfy == NULL)
            return NOT_OK;

        memset(Aud_hdmiFmtNtfy, 0, sizeof(RAL_AUDIO_HDMI_FMT_NTFY_T));
        Aud_hdmiFmtNtfy->adecIdx = HAL_AUDIO_ADEC0;
        Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk = NULL;
        Aud_hdmiFmtNtfy->pfnCallBack = NULL;
        up(&Aud_hdmiFmtNtfy_sem);
    }

    return OK;
}

static void DeleteHdmiFmtNtfy(void)
{
    if(Aud_hdmiFmtNtfy != NULL)
    {
#if 0 //rhal do
      if(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk != NULL)
      {

          if(Aud_hdmiFmtNtfy->pThread->IsRun(Aud_hdmiFmtNtfy->pThread))
          Aud_hdmiFmtNtfy->pThread->Exit(Aud_hdmiFmtNtfy->pThread, TRUE);
      }
      if(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk != NULL)
          Aud_hdmiFmtNtfy->pThread->Delete(Aud_hdmiFmtNtfy->pThread);

      free(Aud_hdmiFmtNtfy);
#else
        if (Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk) {
            int ret;
            ret = kthread_stop(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk);
            if (!ret)
                ERROR("HdmiFmtNtfy task stopped\n");
        }
#endif
      Aud_hdmiFmtNtfy = NULL;
    }
}

SINT32 GetCurrentADCConnectPin(void)
{
    SINT32 revalue = -1;
    SINT32 i;

    for(i = 0; i < MAIN_AIN_ADC_PIN; i++)
    {
        INFO("[AUDH] ADC pin %d is at  %s state \n", i, GetResourceStatusString(AinStatus.ainPinStatus[i]));
        if(AinStatus.ainPinStatus[i] == HAL_AUDIO_RESOURCE_CONNECT)
        {
            if(revalue != -1)
            {
                ERROR("[AUDH] Error ADC too much connect %d %d \n", revalue, i);
                return -1;
            }
            revalue =  i;
        }
    }

    return revalue;
}

SINT32 GetCurrentHDMIConnectPin(void)
{
    SINT32 revalue = -1;
    SINT32 i;

    if(ResourceStatus[HAL_AUDIO_RESOURCE_HDMI].connectStatus[0] == HAL_AUDIO_RESOURCE_CONNECT)
    {
        revalue =  HAL_AUDIO_RESOURCE_HDMI;
    }

    for(i = HAL_AUDIO_RESOURCE_HDMI0; i <= HAL_AUDIO_RESOURCE_SWITCH; i++)
    {
        if(ResourceStatus[i].connectStatus[0] == HAL_AUDIO_RESOURCE_CONNECT)
        {
            if(revalue != -1)
            {
                ERROR("[AUDH] Error HDMI too much connect %d %d \n", revalue, i);
                return -1;
            }
            revalue =  i;
        }
    }

    if(revalue == -1)
        INFO("[AUDH] no  HDMI connect\n");
    return revalue;
}

SINT32 GetCurrentDPConnectPin(void)
{
    SINT32 revalue = -1;

    if (ResourceStatus[HAL_AUDIO_RESOURCE_DP].connectStatus[0] == HAL_AUDIO_RESOURCE_CONNECT) {
        revalue = HAL_AUDIO_RESOURCE_DP;
    }

    if (revalue == -1) {
        INFO("[AUDH] no display port connect\n");
    }
    return revalue;
}

static void GetInputConnectSource(HAL_AUDIO_RESOURCE_T resID,
                                  HAL_AUDIO_RESOURCE_T *inputIDArr,
                                  SINT32 array_length,
                                  SINT32 *num_of_inputs)
{
    SINT32 minArraySize, i;

    *num_of_inputs = ResourceStatus[resID].numIptconnect;

    if(array_length <= *num_of_inputs)
        minArraySize = array_length;
    else
        minArraySize = *num_of_inputs;

    for(i = 0; i < minArraySize; i++)
    {
        inputIDArr[i] = ResourceStatus[resID].inputConnectResourceId[i];
    }
    return;
}

BOOLEAN IsHbbTV(HAL_AUDIO_ADEC_INDEX_T newAdecIndex, HAL_AUDIO_ADEC_INDEX_T oldAdecIndex)
{
    HAL_AUDIO_RESOURCE_T newAdecResourceId = adec2res(newAdecIndex);
    HAL_AUDIO_RESOURCE_T oldAdecResourceId = adec2res(oldAdecIndex);
    HAL_AUDIO_RESOURCE_T decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[oldAdecResourceId]);// dec input source
    // 1 new main dec has no input
    if(ResourceStatus[newAdecResourceId].numIptconnect > 0)
    {
        ERROR("new Adec=ADEC%d, numIptconnect=%d. \n", newAdecIndex, ResourceStatus[newAdecResourceId].numIptconnect);
        return FALSE;
    }
    // 2 new sub dec is DTV
    if(IsDTVSource(decIptResId) == FALSE)
    {
        ERROR("old Adec=ADEC%d is not DTV. \n", oldAdecIndex);
        return FALSE;
    }
    // 3 new sub dec have output source connected
    if(ResourceStatus[oldAdecResourceId].numOptconnect == 0)
    {
        ERROR("old Adec=ADEC%d, numOptconnect=%d. \n", oldAdecIndex, ResourceStatus[oldAdecResourceId].numOptconnect);
        return FALSE;
    }
    pr_debug("Is HbbTV case. \n");
    return TRUE;
}

BOOLEAN IsResourceRunningByProcess(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_MODULE_STATUS resourceStatus)
{
    // check resource status
    if((resourceStatus.connectStatus[0] == HAL_AUDIO_RESOURCE_RUN)||
        resourceStatus.connectStatus[0] == HAL_AUDIO_RESOURCE_PAUSE)
    {
        // check if flow has module or not
        Base* pFlowBaseObj;
        if(adecIndex == HAL_AUDIO_ADEC0)
        {
            pFlowBaseObj = Aud_flow0->pBaseObj;
        }
        else
        {
            pFlowBaseObj = Aud_flow1->pBaseObj;
        }

        if(list_empty(&pFlowBaseObj->flowList))
        {
            INFO("[%s] ADEC%d is running but no module found in FlowManager", __func__, adecIndex);
            return FALSE;
        }
        else
            return TRUE;
    }

    return FALSE;
}

/*! Check a Speaker Connect Status to mute Speaker Output.
 */
BOOLEAN ADEC_SPK_CheckConnect(void)
{
#ifdef SNDOUT_DEV_PLUGNPLAY
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0, i;

    /* check SPK input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SPK, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    /* if input is SE, check SE input status */
    for(i = 0; i < num_of_inputs; i++) {
        if(IsSESource(inputIDArr[i])) {
            GetInputConnectSource(HAL_AUDIO_RESOURCE_SE, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);
            break;
        }
    }
    return (num_of_inputs > 0);
#else
    return TRUE;
#endif
}

/*! Check a SPDIF Connect Status to mute SPDIF Output.
 */
BOOLEAN ADEC_SPDIF_CheckConnect(void)
{
#ifdef SNDOUT_DEV_PLUGNPLAY
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0, i;

    /* check SPDIF input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SPDIF, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    /* if input is SE, check SE input status */
    for(i = 0; i < num_of_inputs; i++) {
        if(IsSESource(inputIDArr[i])) {
            GetInputConnectSource(HAL_AUDIO_RESOURCE_SE, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);
            break;
        }
    }
    if (num_of_inputs > 0) // has connected
    {
        return TRUE;
    }

    /* check SPDIF SB input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SB_SPDIF, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    /* if input is SE, check SE input status */
    for(i = 0; i < num_of_inputs; i++) {
        if(IsSESource(inputIDArr[i])) {
            GetInputConnectSource(HAL_AUDIO_RESOURCE_SE, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);
            break;
        }
    }
    return (num_of_inputs > 0);


#else
    return TRUE;
#endif
}

/*! Check a HP Connect Status to mute HP Output.
 */
BOOLEAN ADEC_HP_CheckConnect(void)
{
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0;

    /* check HP input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_HP, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    return (num_of_inputs > 0);
}

/*! Check a SB_SPDIF Connect Status
 */
BOOLEAN ADEC_SB_SPDIF_CheckConnect(void)
{
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0;

    /* check SB_SPDIF input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SB_SPDIF, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    return (num_of_inputs > 0);
}


/*! Check a SPDIF_ES Connect Status to mute SPDIF_ES Output.
 */
BOOLEAN ADEC_SPDIF_ES_CheckConnect(void)
{
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0, i;

    /* check SPDIF_ES input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SPDIF_ES, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    if (num_of_inputs > 0) // has connected
    {
        return TRUE;
    }

    /* check SPDIF SB input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SB_SPDIF, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    /* if input is SE, check SE input status */
    for(i = 0; i < num_of_inputs; i++) {
        if(IsSESource(inputIDArr[i])) {
            GetInputConnectSource(HAL_AUDIO_RESOURCE_SE, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);
            break;
        }
    }

    return (num_of_inputs > 0);
}

BOOLEAN ADEC_SPDIF_SB_CheckConnect(void)
{
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0;

    /* check SPDIF_SB input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SB_SPDIF, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    return (num_of_inputs > 0);
}

BOOLEAN ADEC_SB_PCM_CheckConnect(HAL_AUDIO_RESOURCE_T *adecIndex)
{
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0, i;

    /* check SB_PCM input status */
    GetInputConnectSource(HAL_AUDIO_RESOURCE_OUT_SB_PCM, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

    if (num_of_inputs > 0) // has connected
    {
        for(i = 0; i < num_of_inputs; i++)
        {
            if((inputIDArr[i] == HAL_AUDIO_RESOURCE_ADEC0) || (inputIDArr[i] == HAL_AUDIO_RESOURCE_ADEC1))
            {
               *adecIndex = inputIDArr[i];
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*! Check ADEC Connect Status to auto-mute DecoderInput
 */
BOOLEAN ADEC_CheckConnect(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
#ifdef DEC_AUTO_MUTE
    HAL_AUDIO_RESOURCE_T res_id = adec2res(adecIndex);
    HAL_AUDIO_RESOURCE_T outputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_outputs = 0;

    GetConnectOutputSource(res_id, outputIDArr, HAL_MAX_OUTPUT, &num_of_outputs);

    return (num_of_outputs > 0);
#else
    return TRUE;
#endif
}

BOOLEAN AMIXER_CheckConnect(HAL_AUDIO_RESOURCE_T amixIndex)
{


#ifdef AMIXER_AUTO_MUTE

    HAL_AUDIO_RESOURCE_T outputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_outputs = 0;

    GetConnectOutputSource(amixIndex, outputIDArr, HAL_MAX_OUTPUT, &num_of_outputs);

    return (num_of_outputs > 0);
#else
    return TRUE;
#endif
}


/*! Get ADEC volume setting by DecInVol, DecOutVol, DecAdsp_gain
 */
void ADEC_Calculate_DSPGain(UINT32 adecIndex, SINT32 dsp_gain[AUD_MAX_CHANNEL])
{
    SINT32 i;
    HAL_AUDIO_VOLUME_T tempVol = GetDecInVolume(adecIndex);

#ifdef SUPPORT_AD_POSITIVE_GAIN
    /* Apply DecAdsp_gain for sub decoder */
    if(Aud_descriptionMode & (adecIndex != (UINT32)Aud_mainDecIndex))
    {
        tempVol = Volume_Add(tempVol, currDecADVol);
        DEBUG("[AUDH] AD dec in vol 0x%x dec out vol[0] 0x%x  ad vol 0x%x  \n", ((HAL_AUDIO_VOLUME_T)(GetDecInVolume(adecIndex))).mainVol, ((HAL_AUDIO_VOLUME_T)(GetDecOutVolume(adecIndex, 0))).mainVol, currDecADVol.mainVol);
     }
    else
    {
        DEBUG("[AUDH] Main dec in vol 0x%x dec out vol[0] 0x%x\n",((HAL_AUDIO_VOLUME_T)(GetDecInVolume(adecIndex))).mainVol, ((HAL_AUDIO_VOLUME_T)(GetDecOutVolume(adecIndex, 0))).mainVol);
    }
#endif

    for(i = 0; i < AUD_MAX_CHANNEL; i++)
    {
        dsp_gain[i] = Volume_to_DSPGain(Volume_Add(GetDecOutVolume(adecIndex, i), tempVol));
    }
    return;
}

typedef enum {
    SNDOUT_NO_OUTPUT  = 0x0000,
    SNDOUT_SPK        = 0x0001,
    SNDOUT_OPTIC      = 0x0002,
    SNDOUT_OPTIC_LG   = 0x0004,
    SNDOUT_BLUETOOTH  = 0x0008,
    SNDOUT_HP         = 0x0010,
    SNDOUT_ARC        = 0x0020,
    SNDOUT_WISA       = 0x0040,
    SNDOUT_SE_BT      = 0x0080,
    SNDOUT_MAX_OUTPUT = 0x0100,
} SNDOUT_DEVICE;

/****************************************************************************************
 * Static Audio DSP communication functions
 ****************************************************************************************/

static long SendRPC_AudioConfig(AUDIO_CONFIG_COMMAND_RTKAUDIO *audioConfig)
{
    UINT32 ret;
    ret = rtkaudio_send_audio_config(audioConfig);
    if(ret != S_OK){
        ERROR("%s ret!=S_OK, %x\n",__func__,ret);
    }
    return ret;
}

static KADP_STATUS_T ADSP_SNDOut_AddDevice(HAL_AUDIO_SNDOUT_T opt_id)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 sndout_id;
    if(opt_id == HAL_AUDIO_SPDIF) {
        sndout_id = SNDOUT_OPTIC | SNDOUT_ARC;
    } else if (opt_id == HAL_AUDIO_HP) {
        sndout_id = SNDOUT_HP;
    } else if (opt_id == HAL_AUDIO_SPK) {
        sndout_id = SNDOUT_SPK;
    } else {
        return KADP_NOT_OK;
    }
    if(Sndout_Devices & sndout_id) {
        DEBUG("output type %d already open\n",opt_id);
        return KADP_OK;
    }
    Sndout_Devices |= sndout_id;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SNDOUT_DEVICE;
    audioConfig.value[0] = Sndout_Devices;
    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SNDOut_RemoveDevice(HAL_AUDIO_SNDOUT_T opt_id)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 sndout_id;
    if(opt_id == HAL_AUDIO_SPDIF) {
        sndout_id = SNDOUT_OPTIC | SNDOUT_ARC;
    } else if (opt_id == HAL_AUDIO_HP) {
        sndout_id = SNDOUT_HP;
    } else if (opt_id == HAL_AUDIO_SPK) {
        sndout_id = SNDOUT_SPK;
    } else {
        return KADP_NOT_OK;
    }
    if((Sndout_Devices & sndout_id) == 0)
    {
        DEBUG("output type %d already close\n",opt_id);
        return KADP_OK;
    }
    Sndout_Devices &= (~sndout_id);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SNDOUT_DEVICE;
    audioConfig.value[0] = Sndout_Devices;
    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static KADP_STATUS_T ADSP_DEC_SetDelay(UINT32 index, UINT32 delay)
{
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    Base* pPPAO;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    if(delay > AUD_MAX_DELAY)
    {
        ERROR("Set (ADEC%d) delay %d ms, exceed max delayTime\n", index, delay);
        return KADP_NOT_OK;
    }

    if(index == (UINT32)Aud_mainDecIndex)
        pPPAO = Aud_ppAout;
    else
        pPPAO = Aud_subPPAout;

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_DELAY;
    audioConfig.value[0] = pPPAO->GetAgentID(pPPAO);
    audioConfig.value[1] = ENUM_DEVICE_DECODER;
    audioConfig.value[2] = pPPAO->GetInPinID(pPPAO);
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < AUD_MAX_CHANNEL; i++)
        audioConfig.value[4+i] = delay;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;

    AudioDecInfo[index].decDelay = delay;
    return KADP_OK;
}

static KADP_STATUS_T ADSP_DEC_SetVolume(UINT32 index, SINT32 ch_vol[AUD_MAX_CHANNEL])
{
    UINT32 i, update = FALSE;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = Aud_dec[index]->GetAgentID(Aud_dec[index]);
    //audioConfig.value[1] = ENUM_DEVICE_DECODER;
    if(index == 0)
        audioConfig.value[1] = ENUM_DEVICE_DECODER0;
    else
        audioConfig.value[1] = ENUM_DEVICE_DECODER1;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < AUD_MAX_CHANNEL; i++)
    {
        if(ch_vol[i] != (SINT32)AudioDecInfo[index].decVol[i])
            update = TRUE;
        audioConfig.value[4+i] = ch_vol[i];
        AudioDecInfo[index].decVol[i] = ch_vol[i];
        DEBUG("adec %d channel %d = %x  \n", index, i, ch_vol[i]);

     }
#ifdef AVOID_USELESS_RPC
    if(update == FALSE) return KADP_OK;
#endif
    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;

    return KADP_OK;
}

static KADP_STATUS_T ADSP_DEC_GetMute(UINT32 index)
{
    return AudioDecInfo[index].decMute;
}

static KADP_STATUS_T ADSP_DEC_SetMute(UINT32 index, BOOLEAN bConnected, BOOLEAN bMute)
{
    UINT32 i;
    BOOLEAN bAutoMute;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    bAutoMute = (bConnected)? FALSE : TRUE;

    INFO("Set (ADEC%d) %s [AutoMute(%s)|UserMute(%s)]\n", index,
              (bAutoMute|bMute)? "MUTE":"UN-MUTE", bAutoMute? "ON":"OFF", bMute? "ON":"OFF");

#ifdef AVOID_USELESS_RPC
    if((bAutoMute|bMute) == ADSP_DEC_GetMute(index))
    {
        INFO("Skip this time mute\n");

        return KADP_OK;
    }
#endif

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = Aud_dec[index]->GetAgentID(Aud_dec[index]);
    //audioConfig.value[1] = ENUM_DEVICE_DECODER;
    if(index == 0)
        audioConfig.value[1] = ENUM_DEVICE_DECODER0;
    else
        audioConfig.value[1] = ENUM_DEVICE_DECODER1;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
        audioConfig.value[4+i] = (bAutoMute|bMute);

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;

    AudioDecInfo[index].decMute = (bAutoMute|bMute);
    return KADP_OK;
}

static KADP_STATUS_T ADSP_DEC_SetHdmiFmt(UINT32 index, HAL_AUDIO_SRC_TYPE_T fmt)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    if(fmt > HAL_AUDIO_SRC_TYPE_MAT_ATMOS)
    {
        INFO("invalid dec format\n");
        return KADP_NOT_OK;
    }

    INFO("ADSP_DEC_SetHdmiFmt=%d=\n", Convert2AudioDecTypefrmHal(fmt));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_NTFY_HAL_HDMI_FMT;
    audioConfig.value[0] = Aud_dec[index]->GetAgentID(Aud_dec[index]);
    audioConfig.value[1] = (index == 0) ? ENUM_DEVICE_DECODER0 : ENUM_DEVICE_DECODER1;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xff;
    audioConfig.value[4] = (u_int)Convert2AudioDecTypefrmHal(fmt);
    audioConfig.value[6] = 1 << 7; // hdmi mode

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static BOOLEAN ADSP_AMIXER_GetMute(UINT32 mixerIndex)
{
    //assert(mixerIndex < AUD_AMIX_MAX);//rhal do
    return  g_mixer_curr_mute[mixerIndex] ;
}

static KADP_STATUS_T ADSP_AMIXER_SetMute(HAL_AUDIO_MIXER_INDEX_T mixerIndex, BOOLEAN bConnected, BOOLEAN bMute)
{
    UINT32 i;
    BOOLEAN bAutoMute;
    //assert(mixerIndex < AUD_AMIX_MAX);//rhal do
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    bAutoMute = (bConnected)? FALSE : TRUE;
#ifdef AVOID_USELESS_RPC
    if((bAutoMute|bMute) == ADSP_AMIXER_GetMute(mixerIndex))
    {

        //INFO("Check (AMIXER%d) [AutoMute(%s)|UserMute(%s)] == fw %s\n", mixerIndex,
             //bAutoMute? "ON":"OFF", bMute? "ON":"OFF", ADSP_AMIXER_GetMute(mixerIndex)? "ON":"OFF" );
        return KADP_OK;
    }
#endif
    INFO("Set (AMIXER%d) %s [AutoMute(%s)|UserMute(%s)]\n", mixerIndex,
            (bAutoMute|bMute)? "MUTE":"UN-MUTE", bAutoMute? "ON":"OFF", bMute? "ON":"OFF");

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_FLASH_PIN;
    audioConfig.value[2] = (UINT32)mixerIndex;
    audioConfig.value[3] = 0xFF;

   for(i = 0; i < 8; i++)
   {
       audioConfig.value[4+i] = (bAutoMute|bMute);
   }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;

//   INFO("after send rpc usr mute %d %d \n", g_mixer_curr_mute[mixerIndex], ADSP_AMIXER_GetMute(mixerIndex));

    g_mixer_curr_mute[mixerIndex]= (bAutoMute|bMute);
    return KADP_OK;
}


static KADP_STATUS_T ADSP_DEC_SetADMode(BOOLEAN bOnOff, UINT32 mainIndex, UINT32 subStreamId)
{
    DUAL_DEC_INFO decADMode;
    UINT32 info_type = INFO_AUDIO_MIX_INFO;
    UINT32 i;

    decADMode.bEnable     = bOnOff;
    decADMode.subStreamId = subStreamId;
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        decADMode.subDecoderMode = (bOnOff && i != mainIndex)? DEC_IS_SUB : DEC_IS_MAIN;
        Aud_dec[i]->PrivateInfo(Aud_dec[i], info_type, (BYTE*)&decADMode, sizeof(decADMode));
    }
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SetRawMode(int mode)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SPDIF;
    audioConfig.value[0] = audioConfig.value[1] = mode;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = TRUE;
    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] arc auto aac failed\n", __func__, __LINE__);
        return KADP_NOT_OK;
    }
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SetDTSSupport(int mode)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_DTS_SPDIF_RAW;
    audioConfig.value[0] = mode;
    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] arc auto aac failed\n", __func__, __LINE__);
        return KADP_NOT_OK;
    }
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SetDTSHDSupport(int mode)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_DTS_HDMI_RAW;
    audioConfig.value[0] = mode;
    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] arc auto aac failed\n", __func__, __LINE__);
        return KADP_NOT_OK;
    }
    return KADP_OK;
}

void Update_RawMode_by_connection(void)
{
    if(ADEC_SB_SPDIF_CheckConnect() == TRUE) {
        ADSP_SetRawMode(ENABLE_DOWNMIX);
    } else if(_ARC_Enable || _EARC_Enable) {
        ADSP_SetRawMode(_AudioARCMode);
    } else {
        ADSP_SetRawMode(_AudioSPDIFMode);
    }
    return;
}

static BOOLEAN GetCurrSNDOutMute(UINT32 dev_id)
{
    switch(dev_id)
    {
        case ENUM_DEVICE_SPEAKER:
            return adsp_sndout_info.spk_mute;
        case ENUM_DEVICE_SPDIF:
            return adsp_sndout_info.spdif_mute;
        case ENUM_DEVICE_SPDIF_ES:
            return adsp_sndout_info.spdifes_mute;
        case ENUM_DEVICE_HEADPHONE:
            return adsp_sndout_info.hp_mute;
        case ENUM_DEVICE_SCART:
            return adsp_sndout_info.scart_mute;
        default:
            return FALSE;
    }
}

static void SetCurrSNDOutMute(UINT32 dev_id, BOOLEAN mute)
{
    switch(dev_id)
    {
        case ENUM_DEVICE_SPEAKER:
            adsp_sndout_info.spk_mute = mute; break;
        case ENUM_DEVICE_SPDIF:
            adsp_sndout_info.spdif_mute = mute; break;
        case ENUM_DEVICE_SPDIF_ES:
            adsp_sndout_info.spdifes_mute = mute; break;
        case ENUM_DEVICE_HEADPHONE:
            adsp_sndout_info.hp_mute = mute; break;
        case ENUM_DEVICE_SCART:
            adsp_sndout_info.scart_mute = mute; break;
        default:
            return;
    }
    return;
}

#define AUDIOONLY_HANG_PATCH
static KADP_STATUS_T ADSP_SNDOut_SetMute(UINT32 dev_id,  BOOLEAN bMute)
{
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    CHAR dev_name[ENUM_DEVICE_MAX][8] = {
        "NONE", "ENC", "DEC", "SPDIF", "SPK", "HP", "SCART", "PCMCap", "Mixer", "Flash", "SPDIFES", "DEC0", "DEC1"};

    INFO("Set (%s) %s \n", dev_name[dev_id],(bMute)? "MUTE":"UN-MUTE" );
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = dev_id;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
    {
        audioConfig.value[4+i] = (bMute);
    }
    SetCurrSNDOutMute(dev_id, (bMute));
    ERROR("%s ADSP_SNDOut_SetMute dev_id:%d,%d ---\n", __func__, dev_id, bMute);
    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static KADP_STATUS_T SNDOut_SetMute(UINT32 dev_id, BOOLEAN bConnected, BOOLEAN bMute)
{
    BOOLEAN bAutoMute;
    CHAR dev_name[ENUM_DEVICE_MAX][8] = {
        "NONE", "ENC", "DEC", "SPDIF", "SPK", "HP", "SCART", "PCMCap", "Mixer", "Flash", "SPDIFES"};

    bAutoMute = (bConnected)? FALSE : TRUE;

    INFO("Set (%s) %s [AutoMute(%s)|UserMute(%s)]\n", dev_name[dev_id],
            (bAutoMute|bMute)? "MUTE":"UN-MUTE", bAutoMute? "ON":"OFF", bMute? "ON":"OFF");
#ifdef AUDIOONLY_HANG_PATCH
#ifdef AVOID_USELESS_RPC
    if((bAutoMute|bMute) == GetCurrSNDOutMute(dev_id))
    {

        INFO("Sip (%s) [AutoMute|UserMute] (%x) == GetCurrSNDOutMute(%x ) \n", dev_name[dev_id],
                (bAutoMute|bMute), GetCurrSNDOutMute(dev_id) );
        return KADP_OK;
    }
#endif
#endif
    return ADSP_SNDOut_SetMute(dev_id, (bAutoMute|bMute));
}


static KADP_STATUS_T ADSP_SNDOut_SetDelay(UINT32 dev_id, UINT32 delay)
{
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    CHAR dev_name[ENUM_DEVICE_MAX][8] = {
        "NONE", "ENC", "DEC", "SPDIF", "SPK", "HP", "SCART", "PCMCap", "Mixer", "Flash"};

    if(delay > AUD_MAX_DELAY)
    {
        ERROR("Set (%s) delay %d ms, exceed max delayTime\n", dev_name[dev_id], delay);
        return KADP_NOT_OK;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_DELAY;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = dev_id;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0x03;
    for(i = 0; i < 8; i++)
    {
        audioConfig.value[4+i] = delay;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SNDOut_SetVolume(UINT32 dev_id, UINT32 dsp_gain)
{
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = dev_id;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;
}

static KADP_STATUS_T ADSP_SNDOut_SetFineVolume(UINT32 dev_id, UINT32 dsp_gain, UINT32 dsp_fine)
{
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = dev_id;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    audioConfig.value[12] = dsp_fine%4;  // 0~3

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;
    return KADP_OK;

}

/*! Set PP mixer mode and apply mixer gain to un-focus pin
 */
static KADP_STATUS_T ADSP_PPMix_ConfigMixer(BOOLEAN bOnOff, UINT32 mixer_gain)
{
    DUAL_DEC_MIXING audioMixing;
    UINT32 info_type = INFO_AUDIO_START_MIXING;
    audioMixing.mode   = bOnOff;

    DEBUG("[AUDH] %s  PPMix = %d mixer_gain = 0x%x \n", __func__, bOnOff, mixer_gain);

    audioMixing.volume = mixer_gain;
    Aud_subPPAout->PrivateInfo(Aud_subPPAout, info_type, (BYTE*)&audioMixing, sizeof(audioMixing));

    /* Keep main pin (focus pin) been mixed with 0dB */
    audioMixing.volume = ADEC_DSP_MIX_GAIN_0DB;
    Aud_ppAout->PrivateInfo(Aud_ppAout, info_type, (BYTE*)&audioMixing, sizeof(audioMixing));

    return KADP_OK;
}
/***************** End of Audio DSP communication functions *****************************/

static void setQuickShowParam(QuickShowCreateParam *param, AUDIO_QUICK_SHOW_PARAM p, UINT32 *nonCachedAddr)
{
    param->ain_instanceID  = p.value[1];
    param->dec_instanceID  = p.value[2];
    param->aout_instanceID = p.value[3];
    param->ain_rh_phyAddr  = p.value[4];
    param->icq_rh_phyAddr  = p.value[5];
    param->dec_rh_phyAddr  = p.value[6];
    param->ain_buf_size    = p.value[7];
    param->icq_buf_size    = p.value[8];
    param->dec_buf_size    = p.value[9];

    param->ain_nonCachedAddr = nonCachedAddr;
    param->icq_nonCachedAddr = param->ain_nonCachedAddr + (param->icq_rh_phyAddr - param->ain_rh_phyAddr);
    param->dec_nonCachedAddr = param->icq_nonCachedAddr + (param->dec_rh_phyAddr - param->icq_rh_phyAddr);

    INFO("[QuickShow][%s] ain_rh_phyAddr:%x, icq_rh_phyAddr:%x, dec_rh_phyAddr:%x", __func__, param->ain_rh_phyAddr, param->icq_rh_phyAddr, param->dec_rh_phyAddr);
    INFO("[QuickShow][%s] ain_buf_size:%x, icq_buf_size:%x, dec_buf_size:%x", __func__, param->ain_buf_size, param->icq_buf_size, param->dec_buf_size);
    INFO("[QuickShow][%s] ain_nonCachedAddr:%x, icq_nonCachedAddr:%x, dec_nonCachedAddr:%x", __func__, param->ain_nonCachedAddr, param->icq_nonCachedAddr, param->dec_nonCachedAddr);
};

DTV_STATUS_T Init_Flow_Control_QuickShow(RTKAUDIO_QS_TYPE *qs_type)
{
    AUDIO_QUICK_SHOW_PARAM p;
    QuickShowCreateParam param;
    UINT32 *nonCachedAddr;
    UINT32 carvedout_size, carvedout_start_addr;
    SINT32 i;
    *qs_type = rtkaudio_quickshow_init_table(&p);

    carvedout_start_addr = p.value[4];
    carvedout_size = p.value[0] - p.value[4];
    nonCachedAddr = (UINT32 *)phys_to_virt(ntohl(carvedout_start_addr));
    INFO("[QuickShow][%s] nonCachedAddr:%x, carvedout_size:%x\n", __func__, nonCachedAddr, carvedout_size);

    setQuickShowParam(&param, p, nonCachedAddr);

    if(CreateAINFilterQuickShow(param) != OK)
    {
        ERROR("[AUDH] initial AIN Filter Failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreateFlowManagerFilter() != OK)
    {
        ERROR("create FlowManager failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreatePPAOFilterQuickShow(param) != OK)
    {
        ERROR("create PPAO failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

#if 0
    if(CreatePCMCapture(((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout)) != OK)
    {
        ERROR("create PCMCap failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }
#endif

    if(CreateDecFilterQuickShow(param) != OK)
    {
        ERROR("create DEC filter failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreateHdmiFmtNtfy() != OK)
    {
        ERROR("create Hdmi Fmt Ntfy failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    Aud_subDecEnable = 0;

    InitialResourceStatus();
    InitialADCStatus();
    InitialDecStatus();

    SetResourceOpen(HAL_AUDIO_RESOURCE_SE);
    SetResourceOpen(HAL_AUDIO_RESOURCE_OUT_SPK);
    ADSP_SNDOut_AddDevice(HAL_AUDIO_SPK);
    if(SetResourceConnect(HAL_AUDIO_RESOURCE_OUT_SPK, HAL_AUDIO_RESOURCE_SE) != TRUE)
    {
        ERROR("[AUDH] SetResourceConnect (%d,%d) failed\n",HAL_AUDIO_RESOURCE_OUT_SPK,HAL_AUDIO_RESOURCE_SE);
        return NOT_OK;
    }

    for(i = 0; i <AUD_AMIX_MAX; i++)
    {
        g_mixer_gain[i].mainVol = 0x7F;
        g_mixer_gain[i].fineVol = 0x0;
    }

    return OK;
}

DTV_STATUS_T Init_Lite_Control(void)
{
    if(Aud_ppAout == NULL)
    {
        Aud_ppAout = new_PPAO(AO_ONLY);
        if(Aud_ppAout == NULL)
        {
            ERROR("create audio ppao failed\n");
            return NOT_OK;
        }
    }
    return OK;
}

DTV_STATUS_T DeInit_Lite_Control(void)
{
    if(Aud_ppAout != NULL)
    {
        Aud_ppAout->Stop(Aud_ppAout);
        Aud_ppAout->Delete(Aud_ppAout);
        Aud_ppAout = NULL;
    }
    return OK;
}

//---------------------------------------------  RAL Function Start  --------------------------------------------------
DTV_STATUS_T RAL_AUDIO_SetAudioDescriptionMode(SINT32 mainIndex, BOOLEAN bOnOff)
{
#ifdef SUPPORT_AD_POSITIVE_GAIN
    SINT32 i, ch_vol[AUD_MAX_CHANNEL];
#endif
    INFO("%s Enter.\n", __func__);
    /* Set Dual-Decode mode */
    ADSP_DEC_SetADMode(bOnOff, mainIndex, 0);/* current HAL API doesn't support substream ID */

    /* Set Mixer mode */
    ADSP_PPMix_ConfigMixer(bOnOff, Volume_to_MixerGain(currMixADVol));
    Aud_descriptionMode = bOnOff;

#ifdef SUPPORT_AD_POSITIVE_GAIN
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        ADEC_Calculate_DSPGain(i, ch_vol);

        {
            INFO("[AUDH]%s:Do setting decoder_%d ch_vol[0]= %d\n", __func__, i, ch_vol[0]);
            ADSP_DEC_SetVolume(i, ch_vol);
        }
    }
#endif
    return OK;
}

DTV_STATUS_T RAL_AUDIO_AMIX_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));
    if(!IsAMIXSource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != AMIX\n", GetResourceString(currentConnect));
        return NOT_OK;
    }

    SetResourceOpen(currentConnect);

    if(SetResourceConnect(currentConnect, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RAL_AUDIO_AMIX_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!IsAMIXSource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != AMIX\n", GetResourceString(currentConnect));
        return NOT_OK;
    }
    if(SetResourceDisconnect(currentConnect, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    if(SetResourceClose(currentConnect) != TRUE)
    {
        ERROR("[AUDH][Error]%s:%d SetResourceClose = %d\n", __func__,__LINE__,currentConnect);
        return NOT_OK;
    }
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_GetDecodingType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T *pAudioType)
{
    AUDIO_RPC_DEC_FORMAT_INFO dec_fomat;
    UINT32 retStatus;
    HAL_AUDIO_RESOURCE_T curResourceId;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(pAudioType == NULL)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curResourceId = adec2res(adecIndex);

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0))
    {
        DEBUG("[AUDH] %s play check failed %d \n",
                __func__, ResourceStatus[curResourceId].connectStatus[0]);
        return NOT_OK;
    }

    retStatus = ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->GetAudioFormatInfo(Aud_dec[adecIndex], &dec_fomat);

    if (retStatus != S_OK)
    {
        ( *pAudioType= Aud_Adec_info[adecIndex].curAdecFormat); //WOSQRTK-3050 , return default setting
        INFO("[AUDH] get return fail return %s\n",  GetSRCTypeName(*pAudioType));
        return OK;
    }

    *pAudioType = Convert2HalAudioDecType(dec_fomat.type, dec_fomat.reserved[1]);

    if (dec_fomat.type == AUDIO_UNKNOWN_TYPE)
    {
        (*pAudioType = Aud_Adec_info[adecIndex].curAdecFormat); //WOSQRTK-3050 , return default setting
        INFO("[AUDH] get return format is unknown return %s\n ", GetSRCTypeName(*pAudioType) );
        return OK;
    }
    return OK;  //OK or NOT_OK
}

static DTV_STATUS_T RAL_AUDIO_ResumeDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T audioType, HAL_AUDIO_MODULE_STATUS* ResourceStatus, SINT32 ResourceStatusSize)
{
    // no condition check, condition check is at RAL_AUDIO_StartDecoding
    HAL_AUDIO_RESOURCE_T curResourceId;
    INFO("[AUDH] %s ADEC%d type %s\n", __func__, adecIndex, GetSRCTypeName(audioType));

    if ((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curResourceId = adec2res(adecIndex);

    if(!AUDIO_HAL_CHECK_ISATPAUSESTATE(ResourceStatus[curResourceId], 0))
    {

        ERROR("[AUDH] %s status is at %s \n", GetResourceString(curResourceId),
                GetResourceStatusString(ResourceStatus[curResourceId].connectStatus[0]));
        return NOT_OK;
    }

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_RUN, 0);

    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        Aud_flow0->Run(Aud_flow0);
    }
    else
    {
        Aud_flow1->Run(Aud_flow1);
    }

    ResourceStatus[adec2res(adecIndex)].connectStatus[0] = HAL_AUDIO_RESOURCE_RUN;

    return OK;
}

DTV_STATUS_T RAL_AUDIO_PauseDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_MODULE_STATUS* ResourceStatus, SINT32 ResourceStatusSize)
{
    HAL_AUDIO_RESOURCE_T curResourceId;

    INFO("[AUDH] %s ADEC%d\n", __func__, adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("Pause  Decoding flow DEC0 (main) info::\n");

    else
        INFO("Pause  Decoding flow DEC0 (sub):: \n");

    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);


    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("Pause  Decoding flow DEC1 (main) info::\n");

    else
        INFO("Pause  Decoding flow DEC1 (sub):: \n");

    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_PAUSE, 0); // HAL_AUDIO_RESOURCE_NO_CONNECTION for show current state

    // open ADEC at run state
    curResourceId = adec2res(adecIndex);

    if(AUDIO_HAL_CHECK_ISATPAUSESTATE(ResourceStatus[curResourceId], 0))
    {
        return OK;
    }

    if(!AUDIO_HAL_CHECK_ISATRUNSTATE(ResourceStatus[curResourceId], 0))
    {
      ERROR("[AUDH] %s status is at %s \n", GetResourceString(curResourceId),
              GetResourceStatusString(ResourceStatus[curResourceId].connectStatus[0]));
      return NOT_OK;
    }

    // check at run sate
    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        Aud_flow0->Pause(Aud_flow0);
    }
    else
    {
        Aud_flow1->Pause(Aud_flow1);
    }
    ResourceStatus[curResourceId].connectStatus[0] = HAL_AUDIO_RESOURCE_PAUSE;
    return OK;
}

DTV_STATUS_T RAL_AUDIO_GetESInfo(int adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo, UINT16 *sampleRate)
{
    #define AAC_CHANNEL_DUALMONO (MODE_11)
    #define AAC_CHANNEL_MONO (MODE_10)
    #define AAC_CHANNEL_STEREO (MODE_20)
    #define AAC_CHANNEL_MULTI (MODE_32)

    AUDIO_RPC_DEC_FORMAT_INFO dec_fomat;
    UINT32 ret = 0;
    *sampleRate = 48000;

    memset(pAudioESInfo, 0, sizeof(HAL_AUDIO_ES_INFO_T));

    if(adecIndex == HAL_AUDIO_ADEC0 || adecIndex == HAL_AUDIO_ADEC1) {
        ret = ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->GetAudioFormatInfo(Aud_dec[adecIndex], &dec_fomat);
    } else {
        void *data_cma = NULL;//uncached vir addr
        unsigned long data_cma_phy = 0;//physical addr
        void *vir_addr = NULL;//cached vir addr
        AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

        vir_addr = get_rpc_mem(&data_cma);
        data_cma_phy = (unsigned long)virt_to_phys(vir_addr);

        if (data_cma == NULL) {
            ERROR("[AUDH][%s %d] RTKAudio_Malloc data size=%d fail\n",__func__,__LINE__,sizeof(AUDIO_RPC_DEC_FORMAT_INFO));
            return NOT_OK;
        }

        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id = AUDIO_CONFIG_CMD_MEDIA_FORMAT;
        audioConfig.value[0] = data_cma_phy;
        audioConfig.value[1] = sizeof(AUDIO_RPC_DEC_FORMAT_INFO);

        ret = KADP_AUDIO_AudioConfig(&audioConfig);

        if(ret != 0){
            put_rpc_mem(vir_addr);
            ERROR("[AUDH][%s %d] fail\n",__func__,__LINE__);
            return NOT_OK;
        }
        ret = S_OK;
        memcpy(&dec_fomat, data_cma, sizeof(AUDIO_RPC_DEC_FORMAT_INFO));
        put_rpc_mem(vir_addr);
    }

    if (ret == E_FAIL) {
        dec_fomat.type = AUDIO_UNKNOWN_TYPE;
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_UNKNOWN;
        INFO("[AUDH][WARNING] GetAudioFormatInfo fail, return unknown %x, ch %x \n", dec_fomat.type, dec_fomat.nChannels );
        return OK;
    }

    if(dec_fomat.type == AUDIO_UNKNOWN_TYPE && dec_fomat.nChannels != 0)
    {
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_UNKNOWN;
        return OK;
    }

    pAudioESInfo->adecFormat =  Convert2HalAudioDecType(dec_fomat.type, dec_fomat.reserved[1]);
    *sampleRate = dec_fomat.nSamplesPerSec;

    if(dec_fomat.nChannels == 0)
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_UNKNOWN;
    else if(dec_fomat.nChannels == 1)
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_MONO;
    else if(dec_fomat.nChannels == 2)
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_STEREO;
    else if(dec_fomat.nChannels >  2)
        pAudioESInfo->audioMode = HAL_AUDIO_MODE_MULTI;

    if(dec_fomat.type == AUDIO_MPEG_DECODER_TYPE)
    {
#if 0
        // bit [0:7]  for mpegN
        SINT32 mpegN;
        mpegN =  (dec_fomat.reserved[0] & 0xFF);
#endif
        // bit [8:15]  for layerN
        SINT32 layerN, stereoMode;
        layerN = (dec_fomat.reserved[0] & 0xFF00) >> 8;
        stereoMode = (dec_fomat.reserved[0] & 0xFF0000) >> 16;

        if (layerN == 3)
        {
            //pAudioESInfo->adecFormat = HAL_AUDIO_SRC_TYPE_MP3; // keep mpeg
        }

        pAudioESInfo->mpegESInfo.bitRate = (dec_fomat.nAvgBytesPerSec) / 4000; // from UTAudioDebug.cpp
        pAudioESInfo->mpegESInfo.sampleRate = (dec_fomat.nSamplesPerSec / 1000);
        pAudioESInfo->mpegESInfo.layer = layerN;
        pAudioESInfo->mpegESInfo.channelNum = dec_fomat.nChannels;

        // bit [16:23] for mode (0x0:stereo,0x1:joint stereo,0x2:dual,0x3:mono)

        if(stereoMode == 1)
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_JOINT_STEREO;
        else if(stereoMode == 2)
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_DUAL_MONO;
        else if(stereoMode == 3)
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_MONO;
        else // 0
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_STEREO;
    }
    else if((dec_fomat.type == AUDIO_AC3_DECODER_TYPE) || (dec_fomat.type == AUDIO_DDP_DECODER_TYPE))
    {
        SINT32 acmode;
        pAudioESInfo->ac3ESInfo.bitRate = (dec_fomat.nAvgBytesPerSec) / 4000; // from UTAudioDebug.cpp
        pAudioESInfo->ac3ESInfo.sampleRate =  (dec_fomat.nSamplesPerSec / 1000);
        pAudioESInfo->ac3ESInfo.channelNum = dec_fomat.nChannels;
        if (dec_fomat.type == AUDIO_DDP_DECODER_TYPE)
            pAudioESInfo->ac3ESInfo.EAC3 = 1;
        else
            pAudioESInfo->ac3ESInfo.EAC3 = 0;

        // reserved[0] has extra info:
        // bit [0:7]  for lfeon
        // bit [8:15]  for acmod
        //SINT32 acmode = (dec_fomat.reserved[0] & 0xFF) >> 8;
        //fix coverity:220,Event result_independent_of_operands:
        acmode = (dec_fomat.reserved[0] & 0xFF00) >> 8;

        if(acmode == 0) // 1+1
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_DUAL_MONO;
        }
        else if(acmode == 1)
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_MONO;
            if(dec_fomat.nChannels != 1)
            {
                pAudioESInfo->ac3ESInfo.channelNum = 1;
            }
        }
        else if(acmode == 2)
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_STEREO;
            if(dec_fomat.nChannels != 2)
            {
                pAudioESInfo->ac3ESInfo.channelNum = 2;
            }
        }
        else
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_MULTI;
        }
    }
    else if(dec_fomat.type == AUDIO_AAC_DECODER_TYPE)
    {
        //version;              /* AAC = 0x0, HE-AACv1 = 0x1, HE-AACv2 = 0x2 */
        //transmissionformat;   /* LOAS/LATM = 0x0, ADTS = 0x1*/
        SINT32 channel_mode, version, format;
        channel_mode = (dec_fomat.reserved[0] & 0xFF00) >> 8;
        version = (dec_fomat.reserved[1] & 0xFF00) >> 8;
        format = (dec_fomat.reserved[1] & 0xFF);

        if(channel_mode == AAC_CHANNEL_DUALMONO) // mode11
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_DUAL_MONO;
            if(dec_fomat.nChannels != 2)
            {
                dec_fomat.nChannels = 2;
            }
        }
        else  if(channel_mode == AAC_CHANNEL_MONO) // mode10
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_MONO;
            if(dec_fomat.nChannels != 1)
            {
                dec_fomat.nChannels = 1;
            }
        }
        else if(channel_mode == AAC_CHANNEL_STEREO) // mode120
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_STEREO;
            if(dec_fomat.nChannels != 2)
            {
                dec_fomat.nChannels = 2;
            }
        }
        else
        {
            pAudioESInfo->audioMode = HAL_AUDIO_MODE_MULTI;
        }

        if(version == 0)
            pAudioESInfo->heAAcEsInfo.version  = 0; // AAC
        else if(version == 1)
            pAudioESInfo->heAAcEsInfo.version  = 1; // he AAC v1
        else
            pAudioESInfo->heAAcEsInfo.version  = 2; // he AAC v2

        if(format  == 0)
            pAudioESInfo->heAAcEsInfo.transmissionformat   = 0;
        else if(format  == 1)
            pAudioESInfo->heAAcEsInfo.transmissionformat   = 1;
        else
        {
            ERROR("[AUDH]  unknown format %d\n", format);
            pAudioESInfo->heAAcEsInfo.transmissionformat   = 0;
        }
        pAudioESInfo->heAAcEsInfo.channelNum = dec_fomat.nChannels;
    }
    else if(dec_fomat.type == AUDIO_RAW_AAC_DECODER_TYPE)
    {
        ERROR("[AUDH]  raw aac format\n");
        return NOT_OK;
    }
    else if(dec_fomat.type == AUDIO_DRA_DECODER_TYPE)
    {
        // reserved[0] has extra info:
        // bit [0:15]  for lfeon
        // bit [16:31]  for main channel number
        SLONG tmp = dec_fomat.reserved[0];
        SINT32 lfeon = (SINT32)(tmp & 0x0000FFFF);
        SINT32 main_channel = (SINT32)((tmp>>16) & 0x0000FFFF);
        SINT32 *ptr;
        if(dec_fomat.nChannels != (main_channel + lfeon))
        {
            ERROR("[AUDH][ERROR]:  dec_fomat.nChannels = %d, main_channel+lfeon = %d\n", dec_fomat.nChannels,(main_channel + lfeon));
            return NOT_OK;
        }

        ptr = (SINT32*)(&pAudioESInfo->audioMode);
        if(lfeon == 1)
        {
            if(main_channel == 2)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_2_1_FL_FR_LFE;
            else if(main_channel == 3)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_3_1_FL_FR_RC_LFE;
            else if(main_channel == 4)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_4_1_FL_FR_RL_RR_LFE;
            else if(main_channel == 5)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_5_1_FL_FR_FC_RL_RR_LFE;
            else
            {
                ERROR("[AUDH] Invalid DRA feedback info: lfeon=%d, main_channel=%d\n", (SINT32)lfeon, (SINT32)main_channel);
                return NOT_OK;
            }
        }
        else
        {
            if(main_channel == 1)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_MONO;
            else if(main_channel == 2)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_STEREO;
            else if(main_channel == 3)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_3_0_FL_FR_RC;
            else if(main_channel == 4)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_4_0_FL_FR_RL_RR;
            else if(main_channel == 5)
              *ptr = (SINT32)HAL_AUDIO_CH_MODE_5_0_FL_FR_FC_RL_RR;
            else
            {
                ERROR("[AUDH] Invalid DRA feedback info: lfeon=%d, main_channel=%d\n", (SINT32)lfeon, (SINT32)main_channel);
                return NOT_OK;
            }
        }
    }

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T DeInit_Flow_Control(void)
{
	SINT32 i;
    // auto connect (KTASKWBS-469)
    INFO("%s Enter.\n", __func__);
    for(i = (SINT32)HAL_AUDIO_RESOURCE_MIXER0; i <= (SINT32)HAL_AUDIO_RESOURCE_MIXER7; i++)
        RAL_AUDIO_AMIX_Disconnect( (HAL_AUDIO_RESOURCE_T)i, (HAL_AUDIO_RESOURCE_T)i);

    DeleteFlowManagerFilter();
    DeleteAINFilter();
    DeletePPAOFilter();
    DeleteDecFilter();
    DeleteHdmiFmtNtfy();
    return OK;
}

DTV_STATUS_T RAL_AUDIO_FinalizeModule(void)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUD] %s \n", __func__);

#ifdef DUMP_HAL_DBG
    CloseHALDumpFile();
#endif

    if(rhal_type == RHAL_LITE) {
        DeInit_Lite_Control();
    } else {
        DeInit_Flow_Control();
    }
    DeInit_Audio_Driver();
    DeInit_ATV_Driver();

    rhal_type = RHAL_NOT_INIT;
    INFO("[AUDH] %s finish\n", __func__);

    return OK;  //or NOT_OK
}

//---------------------------------------------  RAL Function End  ----------------------------------------------
DTV_STATUS_T Init_Flow_Control(void)
{
    int Aud_wait_cnt = 10;
    SINT32 i;
    INFO("%s Enter.\n", __func__);


    while(ADSP_TSK_GetStarted() != ST_AUD_TSK_STRTD && Aud_wait_cnt > 0)
    {
        Aud_wait_cnt--;
        AUDIO_INFO("FW Task not init finished, count down %d\n", Aud_wait_cnt);
        msleep(20);
    }

    if(CreateAINFilter() != OK)
    {
        ERROR("[AUDH] initial AIN Filter Failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreateFlowManagerFilter() != OK)
    {
        ERROR("create FlowManager failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreatePPAOFilter() != OK)
    {
        ERROR("create PPAO failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreateDecFilter() != OK)
    {
        ERROR("create DEC filter failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    if(CreateHdmiFmtNtfy() != OK)
    {
        ERROR("create Hdmi Fmt Ntfy failed\n");
        RAL_AUDIO_FinalizeModule();
        return NOT_OK;
    }

    Aud_subDecEnable = 0;

    InitialResourceStatus();
    InitialADCStatus();
    InitialDecStatus();

    SetResourceOpen(HAL_AUDIO_RESOURCE_SE);
    SetResourceOpen(HAL_AUDIO_RESOURCE_OUT_SPK);
    ADSP_SNDOut_AddDevice(HAL_AUDIO_SPK);
    if(SetResourceConnect(HAL_AUDIO_RESOURCE_OUT_SPK, HAL_AUDIO_RESOURCE_SE) != TRUE)
    {
        ERROR("[AUDH] SetResourceConnect (%d,%d) failed\n",HAL_AUDIO_RESOURCE_OUT_SPK,HAL_AUDIO_RESOURCE_SE);
        return NOT_OK;
    }

    for(i = 0; i <AUD_AMIX_MAX; i++)
    {
        g_mixer_gain[i].mainVol = 0x7F;
        g_mixer_gain[i].fineVol = 0x0;
    }

    return OK;
}

DTV_STATUS_T RAL_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_T eSifType)
{
    SINT64 timerstamp[2] = {0};
    INFO("%s Enter.\n", __func__);
    INFO("[AUD] %s %d \n", __func__, eSifType);
    if(rhal_type != RHAL_NOT_INIT)
    {
        ERROR("[AUDH] %s retry\n", __func__);
        return OK;
    }

    timerstamp[0] = pli_getPTS();

    //rhal_rpcdev_init();////only needed in rhal
    //Init_ATV_Driver(eSifType);//Do not run here, move to rhal's RHAL_AUDIO_InitializeModule through ioctl

    if(Init_Audio_Driver() != OK)
    {
        return NOT_OK;
    }

    if(eSifType == HAL_AUDIO_LITE_INIT)
    {
        if(Init_Lite_Control() != OK)
            return NOT_OK;
        rhal_type = RHAL_LITE;
    }
    else
    {
        SINT32 ch_vol[AUD_MAX_CHANNEL];
        SINT32 i;

        if(Init_Flow_Control() != OK) {
            return NOT_OK;
        }

        rhal_type = RHAL_FULL;
        // initial state is at disconnected so need to mute it.
        //ADSP_SNDOut_SetMute(ENUM_DEVICE_SPEAKER,  TRUE);
        //ADSP_SNDOut_SetMute(ENUM_DEVICE_HEADPHONE,  TRUE);
        //ADSP_SNDOut_SetMute(ENUM_DEVICE_SCART,  TRUE);
        //ADSP_SNDOut_SetMute(ENUM_DEVICE_SPDIF,  TRUE);
        //ADSP_SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES,  TRUE);

        // auto connect (KTASKWBS-469)
        for(i = (SINT32)HAL_AUDIO_RESOURCE_MIXER0; i <= (SINT32)HAL_AUDIO_RESOURCE_MIXER7; i++)
            RAL_AUDIO_AMIX_Connect( (HAL_AUDIO_RESOURCE_T)i, (HAL_AUDIO_RESOURCE_T)i);

        // WOSQRTK-3390
#ifdef AMIXER_AUTO_MUTE
        for(i = HAL_AUDIO_MIXER0; i < AUD_AMIX_MAX; i++)
        {
            ADSP_AMIXER_SetMute((HAL_AUDIO_MIXER_INDEX_T)i, FALSE, FALSE);// default no connect, so  mute
        }
#endif

        for(i = 0; i < AUD_MAX_CHANNEL; i++)
            ch_vol[i] = ENUM_AUDIO_DVOL_K0p0DB;
        for(i = 0; i < AUD_ADEC_MAX; i++)
        {
            ADSP_DEC_SetVolume(i, ch_vol);
        }
    }

#ifdef DUMP_HAL_DBG
    OpenHALDumpFile();
#endif
    timerstamp[1] = pli_getPTS();
    INFO("[AUD] %s start:%lld\n", __func__, timerstamp[0]);
    INFO("[AUD] %s   end:%lld\n", __func__, timerstamp[1]);

    return OK;// or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_SetDolbyDRCMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOLBY_DRC_MODE_T drcMode)
{
    HAL_AUDIO_RESOURCE_T curDecResourceId;
    HAL_AUDIO_RESOURCE_T outputResId[HAL_DEC_MAX_OUTPUT_NUM];
    SINT32 num_of_outputs=0;
    SINT32 isConnect2OputputModule = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("[AUDH] %s ADEC%d mode %d\n", __func__, adecIndex, drcMode);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curDecResourceId = adec2res(adecIndex);
    /* Check Output source and Error handling */
    GetConnectOutputSource(curDecResourceId, outputResId, HAL_DEC_MAX_OUTPUT_NUM, &num_of_outputs);

    if((num_of_outputs < HAL_DEC_MAX_OUTPUT_NUM) && (num_of_outputs > 0))
    {
        SINT32 i;
        for(i = 0; i < num_of_outputs; i++)
        {
            if(IsAOutSource(outputResId[i]))
            {
                isConnect2OputputModule = 1;
                break;
            }
        }
    }

    if(isConnect2OputputModule == 1)
    {
        if(HAL_AUDIO_DOLBY_LINE_MODE == drcMode)
        {
            /*SetAudioDolbyCompressMode(COMP_LINEOUT);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_COMP;
            audioConfig.value[0] = COMP_LINEOUT;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set compressmode failed\n", __func__, __LINE__);
                return NOT_OK;
            }

            /*SetAudioDolbyScaleHILO(COMP_SCALE_FULL, COMP_SCALE_FULL);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_SCALE;
            audioConfig.value[0] = COMP_SCALE_FULL;
            audioConfig.value[1] = COMP_SCALE_FULL;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set scalehilo failed\n", __func__, __LINE__);
                return NOT_OK;
            }
        }
        else if(HAL_AUDIO_DOLBY_RF_MODE == drcMode)
        {
            /*SetAudioDolbyCompressMode(COMP_RF);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_COMP;
            audioConfig.value[0] = COMP_RF;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set compressmode failed\n", __func__, __LINE__);
                return NOT_OK;
            }

            /*SetAudioDolbyScaleHILO(COMP_SCALE_FULL, COMP_SCALE_FULL);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_SCALE;
            audioConfig.value[0] = COMP_SCALE_FULL;
            audioConfig.value[1] = COMP_SCALE_FULL;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set scalehilo failed\n", __func__, __LINE__);
                return NOT_OK;
            }
        }
        else if(HAL_AUDIO_DOLBY_DRC_OFF == drcMode)
        {
            /*SetAudioDolbyCompressMode(COMP_LINEOUT);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_COMP;
            audioConfig.value[0] = COMP_LINEOUT;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set compressmode failed\n", __func__, __LINE__);
                return NOT_OK;
            }

            /*SetAudioDolbyScaleHILO(COMP_SCALE_NONE, COMP_SCALE_NONE);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_SCALE;
            audioConfig.value[0] = COMP_SCALE_NONE;
            audioConfig.value[1] = COMP_SCALE_NONE;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set scalehilo failed\n", __func__, __LINE__);
                return NOT_OK;
            }
        }
        else
        {
            ERROR("[AUDH] unknow drc type %d \n", drcMode);
            return NOT_OK;
        }
    }

    SetDecDrcMode((SINT32)adecIndex, drcMode);// initial

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_SetDownMixMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOWNMIX_MODE_T downmixMode)
{
    HAL_AUDIO_RESOURCE_T curDecResourceId;
    HAL_AUDIO_RESOURCE_T outputResId[HAL_DEC_MAX_OUTPUT_NUM];
    SINT32 num_of_outputs=0;
    SINT32 isConnect2OputputModule = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("[AUDH] %s ADEC%d mode %d\n", __func__, adecIndex, downmixMode);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curDecResourceId = adec2res(adecIndex);
    /* Check Output source and Error handling */
    GetConnectOutputSource(curDecResourceId, outputResId, HAL_DEC_MAX_OUTPUT_NUM, &num_of_outputs);

    if((num_of_outputs < HAL_DEC_MAX_OUTPUT_NUM) && (num_of_outputs > 0))
    {
    	SINT32 i;
        for(i = 0; i < num_of_outputs; i++)
        {
            if(IsAOutSource(outputResId[i]))
            {
                isConnect2OputputModule = 1;
                break;
            }
        }
    }

    if(isConnect2OputputModule == 1)
    {
        if(HAL_AUDIO_LORO_MODE == downmixMode)
        {
            /*SetAudioDownmixMode(MODE_STEREO, LFE_OFF);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_DOWNMIXMODE;
            audioConfig.value[0] = MODE_STEREO;
            audioConfig.value[1] = LFE_OFF;
            audioConfig.value[2] = 0x00002379;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set dd donwmix failed\n", __func__, __LINE__);
                return NOT_OK;
            }
        }
        else if(HAL_AUDIO_LTRT_MODE == downmixMode)
        {
            /*SetAudioDownmixMode(MODE_DOLBY_SURROUND, LFE_OFF);*/
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_DD_DOWNMIXMODE;
            audioConfig.value[0] = MODE_DOLBY_SURROUND;
            audioConfig.value[1] = LFE_OFF;
            audioConfig.value[2] = 0x00002379;

            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] set dd donwmix failed\n", __func__, __LINE__);
                return NOT_OK;
            }
        }
        else
        {
            ERROR("[AUDH] unknow downmix type %d \n", downmixMode);
            return NOT_OK;
        }
    }

    SetDecDownMixMode((SINT32)adecIndex, downmixMode);

    return OK;  //OK or NOT_OK
}

/* HAL_AUDIO functions */
DTV_STATUS_T RHAL_AUDIO_SetMS12Version(HAL_AUDIO_MS12_VERSION_T eMS12Version)
{
    INFO("%s Enter.\n", __func__);
    return OK;// or NOT_OK
}

DTV_STATUS_T RHAL_QS_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_T eSifType)
{
    SINT64 timerstamp[2] = {0};
    SINT32 i;
    SINT32 ch_vol[AUD_MAX_CHANNEL];
    HAL_AUDIO_FLOW_STATUS* pCurFlowStatus;
    HAL_AUDIO_RESOURCE_T curDecResourceId, decIptResId;
    RTKAUDIO_QS_TYPE qs_type = RTKAUDIO_QS_HDMI;

    INFO("[AUD] %s %d \n", __func__, eSifType);
    if(rhal_type != RHAL_NOT_INIT)
    {
        ERROR("[AUDH] %s retry\n", __func__);
        return OK;
    }

    timerstamp[0] = pli_getPTS();

    //rhal_rpcdev_init();//only needed in rhal
    //Init_ATV_Driver(eSifType);//Do not run here, move to rhal's RHAL_QS_AUDIO_InitializeModule through ioctl

    if(Init_Audio_Driver() != OK)
    {
        return NOT_OK;
    }

    if(Init_Flow_Control_QuickShow(&qs_type) != OK)
        return NOT_OK;
    rhal_type = RHAL_FULL;

    // auto connect (KTASKWBS-469)
    for(i = (SINT32)HAL_AUDIO_RESOURCE_MIXER0; i <= (SINT32)HAL_AUDIO_RESOURCE_MIXER7; i++)
        RAL_AUDIO_AMIX_Connect( (HAL_AUDIO_RESOURCE_T)i, (HAL_AUDIO_RESOURCE_T)i);

    // WOSQRTK-3390
#ifdef AMIXER_AUTO_MUTE
    for(i = HAL_AUDIO_MIXER0; i < AUD_AMIX_MAX; i++)
    {
        ADSP_AMIXER_SetMute((HAL_AUDIO_MIXER_INDEX_T)i, FALSE, FALSE);// default no connect, so  mute
    }
#endif

    for(i = 0; i < AUD_MAX_CHANNEL; i++)
        ch_vol[i] = ENUM_AUDIO_DVOL_K0p0DB;
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        ADSP_DEC_SetVolume(i, ch_vol);
    }

    /* Init status for QS flow */
    if (qs_type == RTKAUDIO_QS_DP) {
        SetResourceOpen(HAL_AUDIO_RESOURCE_DP);
        SetResourceOpen(HAL_AUDIO_RESOURCE_ADEC0);
        SetResourceOpen(HAL_AUDIO_RESOURCE_OUT_HP);
        SetResourceConnect(HAL_AUDIO_RESOURCE_DP, HAL_AUDIO_RESOURCE_NO_CONNECTION);
        SetResourceConnect(HAL_AUDIO_RESOURCE_ADEC0, HAL_AUDIO_RESOURCE_DP);
        SetResourceConnect(HAL_AUDIO_RESOURCE_SE, HAL_AUDIO_RESOURCE_ADEC0);
        SetResourceConnect(HAL_AUDIO_RESOURCE_OUT_HP, HAL_AUDIO_RESOURCE_ADEC0);
        UpdateADECStatus(HAL_AUDIO_RESOURCE_ADEC0, HAL_AUDIO_RESOURCE_DP);
        SetDecInMute(0, FALSE);

        AUDIO_INFO("[AUD] %s QS resource setting is DP\n", __FUNCTION__);
    } else {
        SetResourceOpen(HAL_AUDIO_RESOURCE_HDMI0);
        SetResourceOpen(HAL_AUDIO_RESOURCE_ADEC0);
        SetResourceOpen(HAL_AUDIO_RESOURCE_OUT_HP);
        SetResourceConnect(HAL_AUDIO_RESOURCE_HDMI0, HAL_AUDIO_RESOURCE_NO_CONNECTION);
        SetResourceConnect(HAL_AUDIO_RESOURCE_ADEC0, HAL_AUDIO_RESOURCE_HDMI0);
        SetResourceConnect(HAL_AUDIO_RESOURCE_SE, HAL_AUDIO_RESOURCE_ADEC0);
        SetResourceConnect(HAL_AUDIO_RESOURCE_OUT_HP, HAL_AUDIO_RESOURCE_ADEC0);
        UpdateADECStatus(HAL_AUDIO_RESOURCE_ADEC0, HAL_AUDIO_RESOURCE_HDMI0);
        SetDecInMute(0, FALSE);

        AUDIO_INFO("[AUD] %s QS resource setting is HDMI\n", __FUNCTION__);
    }


    pCurFlowStatus = &MainFlowStatus;
    Aud_mainDecIndex = 0;
    // analog flow
    AddAndConnectDecAinToFlow(Aud_flow0, (HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex, Aud_MainAin, pCurFlowStatus);
    /* Connect to PPAO */
    AddAndConnectPPAOToFlow(Aud_flow0, (HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex, Aud_ppAout, pCurFlowStatus);

    // update adec info status
    Aud_Adec_info[Aud_mainDecIndex].prevAdecFormat = Aud_Adec_info[Aud_mainDecIndex].curAdecFormat;
    Aud_Adec_info[Aud_mainDecIndex].curAdecFormat  = HAL_AUDIO_SRC_TYPE_PCM;
    Aud_Adec_info[Aud_mainDecIndex].bAdecStart     = TRUE;
    /* update resource status */

    curDecResourceId = adec2res((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex);
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curDecResourceId]);// dec input source
    ResourceStatus[curDecResourceId].connectStatus[0] = HAL_AUDIO_RESOURCE_RUN;

    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);

    timerstamp[1] = pli_getPTS();
    INFO("[AUD] %s start:%lld\n", __func__, timerstamp[0]);
    INFO("[AUD] %s   end:%lld\n", __func__, timerstamp[1]);

    return OK;// or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_Init_ATV_Driver(HAL_AUDIO_SIF_TYPE_T eSifType)
{
    INFO("%s Enter.\n", __func__);
    Init_ATV_Driver(eSifType);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_InitializeModule(HAL_AUDIO_SIF_TYPE_T eSifType)
{
    INFO("%s Enter.\n", __func__);
    return RAL_AUDIO_InitializeModule(eSifType);// or NOT_OK
    //return OK;
}

DTV_STATUS_T RHAL_AUDIO_FinalizeModule(void)
{
    INFO("%s Enter.\n", __func__);
    return RAL_AUDIO_FinalizeModule();  //or NOT_OK
    //return OK;
}

/* Open, Close */
DTV_STATUS_T RHAL_AUDIO_TP_Open(HAL_AUDIO_TP_INDEX_T tpIndex)
{
    HAL_AUDIO_RESOURCE_T id = (HAL_AUDIO_RESOURCE_T)(HAL_AUDIO_RESOURCE_ATP0 + tpIndex);
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, tpIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(tpIndex > HAL_AUDIO_TP_MAX)
    {
        ERROR("error tp port %d \n", tpIndex);
        return NOT_OK;
    }

    if(SetResourceOpen(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_TP_Close(HAL_AUDIO_TP_INDEX_T tpIndex)
{
    HAL_AUDIO_RESOURCE_T id = (HAL_AUDIO_RESOURCE_T)(HAL_AUDIO_RESOURCE_ATP0 + tpIndex);
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, tpIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(tpIndex > HAL_AUDIO_TP_MAX)
    {
        ERROR("error tp port %d \n", tpIndex);
        return NOT_OK;
    }

    if(SetResourceClose(id) != TRUE)
    {
        return NOT_OK;
    }

    RAL_AUDIO_SetAudioDescriptionMode(Aud_mainDecIndex, FALSE);

    return OK;
}

// 10/30
DTV_STATUS_T RHAL_AUDIO_ADC_Open(HAL_AUDIO_BBADC_SRC portNum)
{
    HAL_AUDIO_RESOURCE_T id = HAL_AUDIO_RESOURCE_ADC;
    // check open again
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, portNum);

    if(portNum >= (AUDIO_BBADC_SRC_AIO2_PORT_NUM + 1))
        return NOT_OK;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(AUDIO_CHECK_ADC_PIN_OPEN_NOTAVAILABLE(AinStatus.ainPinStatus[portNum]))
    {
        ERROR("[AUDH] Error ADC pin %d  is still at %s state \n",portNum, GetResourceStatusString(AinStatus.ainPinStatus[portNum]));
        return NOT_OK;
    }

    if(SetResourceOpen(id) != TRUE)
    {
        return NOT_OK;
    }

    AinStatus.ainPinStatus[portNum] = HAL_AUDIO_RESOURCE_OPEN;

    return OK;
}

// 10/30
DTV_STATUS_T RHAL_AUDIO_ADC_Close(HAL_AUDIO_BBADC_SRC portNum)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, portNum);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(portNum >= (AUDIO_BBADC_SRC_AIO2_PORT_NUM + 1))
        return NOT_OK;

    if(AUDIO_CHECK_ADC_PIN_CLOSE_NOTAVAILABLE(AinStatus.ainPinStatus[portNum]))
    {
        ERROR("[AUDH] Error ADC pin %d  is still at %s state \n",portNum, GetResourceStatusString(AinStatus.ainPinStatus[portNum]));
        return NOT_OK;
    }

    if(SetResourceClose(HAL_AUDIO_RESOURCE_ADC) != TRUE)
    {
        return NOT_OK;
    }

    AinStatus.ainPinStatus[portNum] = HAL_AUDIO_RESOURCE_CLOSE;

    return OK; //or NOT_OK
}

//10/30
DTV_STATUS_T RHAL_AUDIO_HDMI_Open(void)
{
	HAL_AUDIO_RESOURCE_T id = HAL_AUDIO_RESOURCE_HDMI;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port  \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceOpen(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;
}

//10/30
DTV_STATUS_T RHAL_AUDIO_HDMI_Close(void)
{
    HAL_AUDIO_RESOURCE_T id = HAL_AUDIO_RESOURCE_HDMI;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceClose(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK
}

//10/30
DTV_STATUS_T RHAL_AUDIO_DP_Open(void)
{
    HAL_AUDIO_RESOURCE_T id = HAL_AUDIO_RESOURCE_DP;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port  \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceOpen(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;
}

//10/30
DTV_STATUS_T RHAL_AUDIO_DP_Close(void)
{
    HAL_AUDIO_RESOURCE_T id = HAL_AUDIO_RESOURCE_DP;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceClose(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK
}

//10/30
DTV_STATUS_T RHAL_AUDIO_HDMI_OpenPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex)
{
    HAL_AUDIO_RESOURCE_T id = (HAL_AUDIO_RESOURCE_T)(HAL_AUDIO_RESOURCE_HDMI0 + hdmiIndex);
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, hdmiIndex);

    if(hdmiIndex > HAL_AUDIO_HDMI_MAX)
    {
        ERROR("error hdmi port \n");
        return NOT_OK;
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceOpen(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;
}

//10/30
DTV_STATUS_T RHAL_AUDIO_HDMI_ClosePort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex)
{
    HAL_AUDIO_RESOURCE_T id = (HAL_AUDIO_RESOURCE_T) (HAL_AUDIO_RESOURCE_HDMI0 + hdmiIndex);
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, hdmiIndex);

    if(hdmiIndex > HAL_AUDIO_HDMI_MAX)
    {
        ERROR("error hdmi port \n");
        return NOT_OK;
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceClose(id) != TRUE)
    {
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK

}

//10/30
DTV_STATUS_T RHAL_AUDIO_AAD_Open(void)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceOpen(HAL_AUDIO_RESOURCE_AAD) != TRUE)
    {
        return NOT_OK;
    }
    //KADP_Audio_AtvEnterAtvSource();// need to modify later
    return OK;  //OK or NOT_OK
}

//10/30
DTV_STATUS_T RHAL_AUDIO_AAD_Close(void)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port \n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceClose(HAL_AUDIO_RESOURCE_AAD) != TRUE)
    {
        return NOT_OK;
    }

    //KADP_Audio_AtvCleanTVSourceData();

    return NOT_OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SYSTEM_Open(void)
{
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if(SetResourceOpen(HAL_AUDIO_RESOURCE_SYSTEM) != TRUE)
    {
        return NOT_OK;
    }
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SYSTEM_Close(void)
{
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if(SetResourceClose(HAL_AUDIO_RESOURCE_SYSTEM) != TRUE)
    {
        return NOT_OK;
    }
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_ADEC_Open(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] ADEC index error  %d \n", adecIndex);
        return NOT_OK;
    }
    if(SetResourceOpen(adec2res(adecIndex)) != TRUE)
    {
        return NOT_OK;
    }
    SetSPDIFOutType(adecIndex, HAL_AUDIO_SPDIF_PCM);
    return OK;// or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_ADEC_Close(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] ADEC index error  %d \n", adecIndex);
        return NOT_OK;
    }
    if(SetResourceClose(adec2res(adecIndex)) != TRUE)
    {
        return NOT_OK;
    }
    return OK;// or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_AENC_Open(HAL_AUDIO_AENC_INDEX_T aencIndex)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_AENC_Close(HAL_AUDIO_AENC_INDEX_T aencIndex)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SE_Open(void)
{
    INFO("%s Enter.\n", __func__);
    return OK;//  or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SE_Close(void)
{
    INFO("%s Enter.\n", __func__);
    /* SE should have never been closed  */
    return OK;
}

static SINT32 ConvertSNDOUTIndexToResourceId(HAL_AUDIO_SNDOUT_T soundOutType)
{
    SINT32 id;
    switch(soundOutType)
    {
        case HAL_AUDIO_NO_OUTPUT:
        default :
            id = -1;
            ERROR("unknow sndout id %d\n", soundOutType);
            break;
        case HAL_AUDIO_SPK:
            id = HAL_AUDIO_RESOURCE_OUT_SPK;
            break;
        case HAL_AUDIO_SPDIF:
            id = HAL_AUDIO_RESOURCE_OUT_SPDIF;
            break;
        case HAL_AUDIO_SB_SPDIF:
            id = HAL_AUDIO_RESOURCE_OUT_SB_SPDIF;
            break;
        case HAL_AUDIO_SB_PCM:
            id = HAL_AUDIO_RESOURCE_OUT_SB_PCM;
            break;
        case HAL_AUDIO_SB_CANVAS:
            id = HAL_AUDIO_RESOURCE_OUT_SB_CANVAS;
            break;
        case HAL_AUDIO_HP:
            id = HAL_AUDIO_RESOURCE_OUT_HP;
            break;
        case HAL_AUDIO_SCART:
            id = HAL_AUDIO_RESOURCE_OUT_SCART;
            break;
        case HAL_AUDIO_SPDIF_ES:
            id = HAL_AUDIO_RESOURCE_OUT_SPDIF_ES;
            break;
    }

    return id;
}

DTV_STATUS_T RHAL_AUDIO_SNDOUT_Open(HAL_AUDIO_SNDOUT_T soundOutType)
{
    SINT32 id;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = ConvertSNDOUTIndexToResourceId(soundOutType);
    INFO("[AUDH] %s %s \n", __func__, GetResourceString((HAL_AUDIO_RESOURCE_T) (id)));

    if(id >= 0)
    {
        /* SPK already open at init */
        if((HAL_AUDIO_RESOURCE_T)id == HAL_AUDIO_RESOURCE_OUT_SPK)
        {
            INFO("SPK already open at init\n");
            return OK;
        }
        else if(SetResourceOpen((HAL_AUDIO_RESOURCE_T)id) != TRUE)
        {
            return NOT_OK;
        }
        ADSP_SNDOut_AddDevice(soundOutType);
        return OK;  //OK or NOT_OK
    }
    else
    {
        ERROR("[AUDH] unknow sndout type %d \n", soundOutType);
        return NOT_OK;
    }
    return NOT_OK;
}

//10/30
DTV_STATUS_T RHAL_AUDIO_SNDOUT_Close(HAL_AUDIO_SNDOUT_T soundOutType)
{
    SINT32 id = (SINT32)ConvertSNDOUTIndexToResourceId(soundOutType);
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    INFO("[AUDH] %s %s \n", __func__, GetResourceString((HAL_AUDIO_RESOURCE_T)id));

    if(id >= 0)
    {
        /* SPK should have never been closed */
        if((HAL_AUDIO_RESOURCE_T)id == HAL_AUDIO_RESOURCE_OUT_SPK)
        {
            return OK;
        }
        if(SetResourceClose((HAL_AUDIO_RESOURCE_T)id) != TRUE)
        {
            return NOT_OK;
        }
        ADSP_SNDOut_RemoveDevice(soundOutType);
        return OK;  //OK or NOT_OK
    }
    else
    {
        return NOT_OK;
    }
}

void PrintRingBuffer(RINGBUFFER_HEADER* pRingBuffer, ULONG phyaddress)
{
    ULONG pbase, prp, pwp, size;

    INFO("ring buffer %p  \n", pRingBuffer);
    if(pRingBuffer == NULL)
        return;

    prp   = IPC_ReadU32((BYTE*) &(pRingBuffer->readPtr[0]));
    pbase = IPC_ReadU32((BYTE*) &(pRingBuffer->beginAddr));
    pwp   = IPC_ReadU32((BYTE*) &(pRingBuffer->writePtr));
    size  = IPC_ReadU32((BYTE*) &(pRingBuffer->size));
    INFO("phy %x -> baddr %x rp %x wp %x size %x \n", phyaddress, pbase, prp, pwp, size);
}



/* RTWTV-219
it is audio description path, example
ADTU0(1) - SDEC0(1) - ATP1(1) - ADEC0(1) - SE - SPK
ADTU0(1) - SDEC0(1) - ATP0(1) - ADEC1(1) - SE - SPK
*/

/* Connect & Disconnect */
DTV_STATUS_T RHAL_AUDIO_TP_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    HAL_AUDIO_RESOURCE_T outputId;
    SINT32 totalOutputConnectResource;
    DEMUX_CHANNEL_T sdec_channel;
    RSDEC_ENUM_TYPE_DEST_T sdec_dest_type;
    DEMUX_STRUCT_CONNECT_RESULT_T AddressInfo;
    SINT32 atpindex;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d %d \n", __func__, currentConnect, inputConnect);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((currentConnect != HAL_AUDIO_RESOURCE_ATP0) && (currentConnect != HAL_AUDIO_RESOURCE_ATP1))
    {
        ERROR("[AUDH] error connect index %d \n",currentConnect);
        return NOT_OK;
    }

    if((inputConnect != HAL_AUDIO_RESOURCE_SDEC0) && (inputConnect != HAL_AUDIO_RESOURCE_SDEC1))
    {
        ERROR("[AUDH] error connect input index %d \n",inputConnect);
        return NOT_OK;
    }

    GetConnectOutputSource(inputConnect, &outputId, 1, &totalOutputConnectResource);// tp's iutput is sdec

    if(totalOutputConnectResource > 1)// sdec's output
    {
        ERROR("[AUDH] sdec output connect too much %d != 1\n", totalOutputConnectResource);
        return NOT_OK;
    }

    if(SetResourceConnect(currentConnect, inputConnect) != TRUE)
        return NOT_OK;

    GetConnectOutputSource(currentConnect, &outputId, 1, &totalOutputConnectResource);// tp's output is dec

    if(IsADECSource(outputId))// tp's output is dec
    {
        SINT32 adecIndex = res2adec(outputId);
        Aud_Adec_info[adecIndex].prevTPInputPort = Aud_Adec_info[adecIndex].curTPInputPort;
        Aud_Adec_info[adecIndex].curTPInputPort  = inputConnect;
    }

    if(inputConnect == HAL_AUDIO_RESOURCE_SDEC0)
        sdec_channel = DEMUX_CH_A;
    else
        sdec_channel = DEMUX_CH_B;

    if(currentConnect == HAL_AUDIO_RESOURCE_ATP0)
    {
        sdec_dest_type = RSDEC_ENUM_TYPE_DEST_ADEC0;
    }
    else
    {
        sdec_dest_type = RSDEC_ENUM_TYPE_DEST_ADEC1;
    }

    if(KADP_SDEC_Connect(sdec_channel, sdec_dest_type, &AddressInfo) != OK)
    {
        ERROR("[AUDH] sdec connect failed\n");
        return NOT_OK;
    }

    atpindex = currentConnect - HAL_AUDIO_RESOURCE_ATP0;

    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetSDECInfo(Aud_DTV[atpindex], (UINT32)sdec_channel, (UINT32)sdec_dest_type);

    INFO("[AUDH] IBand add %x  size %x \n", AddressInfo.ibRingHeader, AddressInfo.ibHeaderSize);

    INFO("[AUDH] BS add %x  size %x \n", AddressInfo.bsRingHeader, AddressInfo.bsHeaderSize);

    INFO("[AUDH] Ref add %x  size %x \n", AddressInfo.refClock, AddressInfo.refClockHeaderSize);

    if(AddressInfo.refClockHeaderSize != sizeof(REFCLOCK)) // sdec's reference min size is 16 align
    {
        //INFO("error ref clock size %x %x \n", AddressInfo.refClockHeaderSize, sizeof(REFCLOCK));
        //return NOT_OK;
    }

    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetICQRingBufPhyAddress(Aud_DTV[atpindex], AddressInfo.ibRingHeader, (void*)__va(AddressInfo.ibRingHeader), 0, 0);
    //PrintRingBuffer((RINGBUFFER_HEADER*)nonCacheAddress, (ULONG) AddressInfo.ibRingHeader);

    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetBSRingBufPhyAddress(Aud_DTV[atpindex], AddressInfo.bsRingHeader, (void*)__va(AddressInfo.bsRingHeader), 0, 0);
    //PrintRingBuffer((RINGBUFFER_HEADER*)nonCacheAddress, (ULONG) AddressInfo.bsRingHeader);

    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetRefClockPhyAddress(Aud_DTV[atpindex], AddressInfo.refClock, (void*)__va(AddressInfo.refClock), 0, 0);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_TP_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    //KADP_STATUS_T reValue;
    SINT32 atpindex;
    //UINT32 mapAddress;
    //UINT32 mapSize;
    HAL_AUDIO_RESOURCE_T id = currentConnect;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d %d \n", __func__, currentConnect, inputConnect);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((currentConnect != HAL_AUDIO_RESOURCE_ATP0) && (currentConnect != HAL_AUDIO_RESOURCE_ATP1))
    {
        ERROR("[AUDH] error connect index %d \n",currentConnect);
        return NOT_OK;
    }

    if((inputConnect != HAL_AUDIO_RESOURCE_SDEC0) && (inputConnect != HAL_AUDIO_RESOURCE_SDEC1))
    {
        ERROR("[AUDH] error disconnect input index %d \n",inputConnect);
        return NOT_OK;
    }

    if(SetResourceDisconnect(id, inputConnect) != TRUE)
        return NOT_OK;

    atpindex = currentConnect - HAL_AUDIO_RESOURCE_ATP0;

    // clean
    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetICQRingBufPhyAddress(Aud_DTV[atpindex],0, 0, 0, 0);
    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetBSRingBufPhyAddress(Aud_DTV[atpindex], 0, 0, 0,0);
    ((DtvCom*)Aud_DTV[atpindex]->pDerivedObj)->SetRefClockPhyAddress(Aud_DTV[atpindex], 0, 0, 0, 0);

    if(IsDTVSource(currentConnect))// tp's output is dec
    {
        DEMUX_CHANNEL_T sdec_channel;
        RSDEC_ENUM_TYPE_DEST_T sdec_dest;

        if(inputConnect == HAL_AUDIO_RESOURCE_SDEC0)
            sdec_channel = DEMUX_CH_A;
        else
            sdec_channel = DEMUX_CH_B;

        if(currentConnect == HAL_AUDIO_RESOURCE_ATP0)
        {
            sdec_dest = RSDEC_ENUM_TYPE_DEST_ADEC0;
        }
        else
        {
            sdec_dest = RSDEC_ENUM_TYPE_DEST_ADEC1;
        }

        KADP_SDEC_DisConnect(sdec_channel, sdec_dest);
    }
    return OK;
}

//10/30
DTV_STATUS_T RHAL_AUDIO_ADC_Connect(HAL_AUDIO_BBADC_SRC portNum)
{
    SINT32 i;
    HAL_AUDIO_RESOURCE_T optResourceId[2];
    SINT32 totalOutputConnectResource;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d \n", __func__, portNum);

    //INFO("[AUDH] force to change pin id %d -> 1   \n", portNum);
    //portNum = 1;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(portNum >= (AUDIO_BBADC_SRC_AIO2_PORT_NUM + 1))
        return NOT_OK;

    // check if other pins are also at connect status
    for(i = 0; i < MAIN_AIN_ADC_PIN; i++)
    {
        if(AinStatus.ainPinStatus[i] == HAL_AUDIO_RESOURCE_CONNECT)
        {
            ERROR("[AUDH] Error ADC too much connect %d %d \n",i, AinStatus.ainPinStatus[i]);
            return NOT_OK;
        }
    }

    // check adc is at correct status
    if( AUDIO_CHECK_ADC_PIN_CONNECT_NOTAVAILABLE(AinStatus.ainPinStatus[portNum]))
    {
         ERROR("[AUDH] Error ADC pin %d  is still at %s state \n",portNum, GetResourceStatusString(AinStatus.ainPinStatus[portNum]));
         return NOT_OK;
    }

#if 1 /* move check to decoder module*/
    GetConnectOutputSource(HAL_AUDIO_RESOURCE_ADC, optResourceId, 2, &totalOutputConnectResource);
    if(totalOutputConnectResource > 2)
    {
        ERROR("[AUDH] ADC output connect error %d \n", totalOutputConnectResource);
        return NOT_OK;
    }
#endif

    if(SetResourceConnect(HAL_AUDIO_RESOURCE_ADC, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    for(i = 0; i < totalOutputConnectResource; i++)
    {
        if(IsADECSource(optResourceId[i]))
        {
            HAL_AUDIO_ADEC_INDEX_T adecIndex = res2adec(optResourceId[i]);

            Aud_Adec_info[adecIndex].prevADCPortNum = Aud_Adec_info[adecIndex].curADCPortNum;
            Aud_Adec_info[adecIndex].curADCPortNum  = portNum;
        }
    }

    AinStatus.ainPinStatus[portNum] = HAL_AUDIO_RESOURCE_CONNECT;

    return OK;// or NOT_OK
}

//10/30
DTV_STATUS_T RHAL_AUDIO_ADC_Disconnect(HAL_AUDIO_BBADC_SRC portNum)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s port %d\n", __func__,  portNum);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;


    if(portNum >= (AUDIO_BBADC_SRC_AIO2_PORT_NUM + 1))
        return NOT_OK;

    if(AUDIO_CHECK_ADC_PIN_DISCONNECT_NOTAVAILABLE(AinStatus.ainPinStatus[portNum]))
    {
        ERROR("[AUDH] Error ADC pin %d  is still at %s state \n",portNum, GetResourceStatusString(AinStatus.ainPinStatus[portNum]));
        return NOT_OK;
    }

    if(SetResourceDisconnect(HAL_AUDIO_RESOURCE_ADC, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    AinStatus.ainPinStatus[portNum] = HAL_AUDIO_RESOURCE_DISCONNECT;

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_HDMI_Connect(void)
{
    SINT32 i;
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = HAL_AUDIO_RESOURCE_HDMI;

    for(i = HAL_AUDIO_RESOURCE_HDMI0; i <= HAL_AUDIO_RESOURCE_SWITCH; i++)
    {
        if(ResourceStatus[i].connectStatus[0] == HAL_AUDIO_RESOURCE_CONNECT)
        {
            ERROR("[AUDH] Error HDMI too much connect %d %d \n",
                          i, ResourceStatus[i].connectStatus[0]);
            return NOT_OK;
        }
    }

    // do not need to update hdmi port of adecstatus api

    if(SetResourceConnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_HDMI_Disconnect(void)
{
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = HAL_AUDIO_RESOURCE_HDMI;

    if(SetResourceDisconnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_DP_Connect(void)
{
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = HAL_AUDIO_RESOURCE_DP;

    if(SetResourceConnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_DP_Disconnect(void)
{
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = HAL_AUDIO_RESOURCE_DP;

    if(SetResourceDisconnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_HDMI_ConnectPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex)
{
    SINT32 i;
    HAL_AUDIO_RESOURCE_T optResourceId[2];
    SINT32 totalOutputConnectResource;
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s %d \n", __func__, hdmiIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(hdmiIndex > HAL_AUDIO_HDMI_MAX)
    {
        ERROR("error hdmi port\n");
        return NOT_OK;
    }
    id = (HAL_AUDIO_RESOURCE_T)(HAL_AUDIO_RESOURCE_HDMI0 + hdmiIndex);

    if(GetCurrentHDMIConnectPin() != -1)
    {
        ERROR("error too many hdmi connect \n");
        return NOT_OK;
    }

    GetConnectOutputSource(id, optResourceId, 2, &totalOutputConnectResource);
    if(totalOutputConnectResource > 2)
    {
        ERROR("[AUDH] hdmiport output connect error %d \n", totalOutputConnectResource);
        return NOT_OK;
    }

    for(i = 0; i < totalOutputConnectResource; i++)
    {
        if(IsADECSource(optResourceId[i]))
        {
            HAL_AUDIO_ADEC_INDEX_T adecIndex = res2adec(optResourceId[i]);

            Aud_Adec_info[adecIndex].prevHDMIPortNum = Aud_Adec_info[adecIndex].curHDMIPortNum;
            Aud_Adec_info[adecIndex].curHDMIPortNum  = hdmiIndex;
        }
    }

    if(SetResourceConnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_HDMI_DisconnectPort(HAL_AUDIO_HDMI_INDEX_T hdmiIndex)
{
    HAL_AUDIO_RESOURCE_T id;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s %d \n", __func__ , hdmiIndex);

    if(hdmiIndex > HAL_AUDIO_HDMI_MAX)
    {
        ERROR("error hdmi port \n");
        return NOT_OK;
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    id = (HAL_AUDIO_RESOURCE_T)((UINT32)HAL_AUDIO_RESOURCE_HDMI0 + (UINT32)hdmiIndex);

    if(SetResourceDisconnect(id, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    return OK;
}


DTV_STATUS_T RHAL_AUDIO_AAD_Connect(void)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s \n", __func__);

    if (rhal_type == RHAL_LITE) //for sina
        KADP_Audio_AtvEnterAtvSource();// need to modify later
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceConnect(HAL_AUDIO_RESOURCE_AAD, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;

    if (rhal_type != RHAL_LITE)
        KADP_Audio_AtvEnterAtvSource();// need to modify later
    //g_AudioSIFInfo.sifSource      = HAL_AUDIO_SIF_INPUT_INTERNAL; //No need to reset when AAD Connect
    //g_AudioSIFInfo.curSifType     = HAL_AUDIO_SIF_TYPE_NONE; //clear on Finalize...
    g_AudioSIFInfo.bHighDevOnOff  = FALSE;
    g_AudioSIFInfo.curSifBand     = HAL_AUDIO_SIF_SYSTEM_UNKNOWN;
    g_AudioSIFInfo.curSifStandard = HAL_AUDIO_SIF_MODE_DETECT;
    g_AudioSIFInfo.curSifIsA2     = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
    g_AudioSIFInfo.curSifIsNicam  = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
    g_AudioSIFInfo.curSifModeSet  = HAL_AUDIO_SIF_SET_PAL_UNKNOWN;
    g_AudioSIFInfo.curSifModeGet  = HAL_AUDIO_SIF_GET_PAL_UNKNOWN;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AAD_Disconnect(void)
{
    INFO("%s Enter.\n", __func__);

    // To make sure ATV thread cannot invoke RAL_ATVSetDecoderXMute callback and cause deadlock by //AUDIO_FUNC_CALL()
    KADP_Audio_AtvPauseTvStdDetection(true);
    INFO("[AUDH] %s port\n", __func__);

    if (rhal_type == RHAL_LITE)
        KADP_Audio_AtvCleanTVSourceData();
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
    {
        // release sem
        KADP_Audio_AtvPauseTvStdDetection(false);
        return NOT_OK;
    }

    if(SetResourceDisconnect(HAL_AUDIO_RESOURCE_AAD, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
    {
        // release sem
        KADP_Audio_AtvPauseTvStdDetection(false);
        return NOT_OK;
    }

    if (rhal_type != RHAL_LITE)
        KADP_Audio_AtvCleanTVSourceData();

    // release sem
    KADP_Audio_AtvPauseTvStdDetection(false);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SYSTEM_Connect(void)
{
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if(SetResourceConnect(HAL_AUDIO_RESOURCE_SYSTEM, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;
    return OK;// or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SYSTEM_Disconnect(void)
{
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if(SetResourceDisconnect(HAL_AUDIO_RESOURCE_SYSTEM, HAL_AUDIO_RESOURCE_NO_CONNECTION) != TRUE)
        return NOT_OK;
    return OK;// or NOT_OK
}

static void UpdateADECStatus(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    HAL_AUDIO_ADEC_INDEX_T adecIndex = res2adec(currentConnect);
    HAL_AUDIO_IN_PORT_T tempport;

    if(IsDTVSource(inputConnect))
    {
        if((ResourceStatus[inputConnect].connectStatus[0] != HAL_AUDIO_RESOURCE_CONNECT))  // check ATP is at connect status
        {

        }
        else
        {
            // at connect status
            HAL_AUDIO_RESOURCE_T atpInputSource;
            atpInputSource = GetNonOutputModuleSingleInputResource(ResourceStatus[inputConnect]);
            if((atpInputSource == HAL_AUDIO_RESOURCE_SDEC0) || (atpInputSource == HAL_AUDIO_RESOURCE_SDEC1))
            {
                Aud_Adec_info[adecIndex].prevTPInputPort = Aud_Adec_info[adecIndex].curTPInputPort;
                Aud_Adec_info[adecIndex].curTPInputPort  = atpInputSource;
            }
        }

        tempport = HAL_AUDIO_IN_PORT_TP;
    }
    else if(IsATVSource(inputConnect))
    {
        tempport = HAL_AUDIO_IN_PORT_SIF;
    }
    else if(IsADCSource(inputConnect))
    {
        if((ResourceStatus[inputConnect].connectStatus[0] != HAL_AUDIO_RESOURCE_CONNECT))// check at connect status
        {
            //  adc connect will update the port status
        }
        else
        {   // ADC is at  connected status
            Aud_Adec_info[adecIndex].prevADCPortNum = Aud_Adec_info[adecIndex].curADCPortNum;
            Aud_Adec_info[adecIndex].curADCPortNum  = GetCurrentADCConnectPin();
        }

        tempport = HAL_AUDIO_IN_PORT_ADC;
    }
    else if(IsSystemSource(inputConnect))
    {
        tempport = HAL_AUDIO_IN_PORT_SYSTEM;
    }
    else if(IsDPSource(inputConnect))
    {
        tempport = HAL_AUDIO_IN_PORT_DP;
    }
    else if(IsHDMISource(inputConnect))
    {
        tempport = HAL_AUDIO_IN_PORT_HDMI;

        if((ResourceStatus[inputConnect].connectStatus[0] != HAL_AUDIO_RESOURCE_CONNECT))// check at connect status
        {
            //  hdmi connect will update the port status
        }
        else
        {   // ADC is at  connected status
            if(inputConnect >= HAL_AUDIO_RESOURCE_HDMI0)
            {
            	SINT32 hdmiconnectsource;
                Aud_Adec_info[adecIndex].prevHDMIPortNum = Aud_Adec_info[adecIndex].curHDMIPortNum;
                hdmiconnectsource = GetCurrentHDMIConnectPin();
                if(hdmiconnectsource == (SINT32)HAL_AUDIO_RESOURCE_HDMI)
                {
                    INFO("[AUDH] unknow hdmi port \n");
                    Aud_Adec_info[adecIndex].curHDMIPortNum = HAL_AUDIO_HDMI0;
                }
                else
                    Aud_Adec_info[adecIndex].curHDMIPortNum = (HAL_AUDIO_HDMI_INDEX_T)(hdmiconnectsource - HAL_AUDIO_RESOURCE_HDMI0);
            }
        }
    }
    else
    {
        ERROR("[AUDH] error ipt index %d \n", inputConnect);
       //assert(0);
        return;
    }
    Aud_Adec_info[adecIndex].prevAdecInputPort = Aud_Adec_info[adecIndex].curAdecInputPort;
    Aud_Adec_info[adecIndex].curAdecInputPort  = tempport;
}

DTV_STATUS_T RHAL_AUDIO_ADEC_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!IsADECSource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != ADEC\n", GetResourceString(currentConnect));
        return NOT_OK;
    }

    if(IsValidADECIpts(inputConnect) != TRUE)
    {
        ERROR("[AUDH][Error] Invalid input %s\n", GetResourceString(inputConnect));
        return NOT_OK;
    }

    if(SetResourceConnect(currentConnect, inputConnect) != TRUE)
        return NOT_OK;

    UpdateADECStatus(currentConnect, inputConnect);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_ADEC_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);

    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!IsADECSource(currentConnect))
        return NOT_OK;

    if(SetResourceDisconnect(currentConnect, inputConnect) != TRUE)
        return NOT_OK;

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_AMIX_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    //  do not care inputConnect paremeter
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!IsAMIXSource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != AMIX\n", GetResourceString(currentConnect));
        return NOT_OK;
    }
    return OK; // always connect
}


DTV_STATUS_T RHAL_AUDIO_AMIX_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    //  do not care inputConnect paremeter
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!IsAMIXSource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != AMIX\n", GetResourceString(currentConnect));
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK // always connect
}


DTV_STATUS_T RHAL_AUDIO_AENC_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_AENC_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK
}


// se has 2 input , can not use setresourceconnect
DTV_STATUS_T RAL_AUDIO_SE_Connect(HAL_AUDIO_RESOURCE_T inputConnect)
{
    BOOLEAN bReturn = FALSE;
    INFO("[AUDH] %s ipt %s \n", __func__, GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    bReturn  = SetResourceConnect(HAL_AUDIO_RESOURCE_SE, inputConnect);
    if(bReturn == FALSE)
    {
        ERROR("[AUDH]se SetResourceConnect %s failed\n", GetResourceString(inputConnect));
        return NOT_OK;
    }

    SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);

    if(IsADECSource(inputConnect))
    {
    	HAL_AUDIO_ADEC_INDEX_T adecIndex;
        RAL_AUDIO_SetDolbyDRCMode( res2adec(inputConnect), GetDecDrcMode((SINT32) res2adec(inputConnect) )); // revert ori DRC setting after multi-view switch
        RAL_AUDIO_SetDownMixMode( res2adec(inputConnect),  GetDecDownMixMode((SINT32) res2adec(inputConnect) )); // revert ori DownMix setting after multi-view switch

        adecIndex = res2adec(inputConnect);
        ADSP_DEC_SetMute(adecIndex, ADEC_CheckConnect(adecIndex), GetDecInMute(adecIndex));// change connect mute status
    }

    if(IsAMIXSource(inputConnect) == TRUE)
    {
        HAL_AUDIO_MIXER_INDEX_T amixerIndex= res2amixer(inputConnect);
        ADSP_AMIXER_SetMute(amixerIndex, AMIXER_CheckConnect(inputConnect), GetAmixerUserMute(amixerIndex));
    }

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SE_Connect(HAL_AUDIO_RESOURCE_T inputConnect)
{
    DTV_STATUS_T status;
    INFO("%s Enter.\n", __func__);

    status =  RAL_AUDIO_SE_Connect(inputConnect);  //OK or NOT_OK
    if(status != OK)
        return status;

   return status;
}

// se has 2 input , can not use setresourcedisconnect

DTV_STATUS_T RHAL_AUDIO_SE_Disconnect(HAL_AUDIO_RESOURCE_T inputConnect)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ipt %s \n", __func__, GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(SetResourceDisconnect(HAL_AUDIO_RESOURCE_SE, inputConnect) != TRUE)
        return NOT_OK;

    if(IsADECSource(inputConnect))
    {
        HAL_AUDIO_ADEC_INDEX_T adecIndex = res2adec(inputConnect);
        ADSP_DEC_SetMute(adecIndex, ADEC_CheckConnect(adecIndex), GetDecInMute(adecIndex));// disconnect auto mute
    }

    if(IsAMIXSource(inputConnect) == TRUE)
    {
       HAL_AUDIO_MIXER_INDEX_T amixerIndex= res2amixer(inputConnect);
       ADSP_AMIXER_SetMute(amixerIndex, AMIXER_CheckConnect(inputConnect), GetAmixerUserMute(amixerIndex));
    }

    SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_SNDOUT_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    // check current resource is valid
    if((IsAOutSource(currentConnect) != TRUE) || IsSESource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != SNDOUT device\n", GetResourceString(currentConnect));
        return NOT_OK;
    }

    // check input resource is valid
    if(IsADECSource(inputConnect) || IsAMIXSource(inputConnect))
    {
        // valid input resource
    }
    else if(IsSESource(inputConnect) && IsValidSEOpts(currentConnect))
    {
        /* SPK & SE  should have been connected at init */
        if(currentConnect == HAL_AUDIO_RESOURCE_OUT_SPK)
        {
            return OK;
        }
        // valid input resource
    }
    else
    {
        INFO("[AUDH] Error::  %s want to connect %s \n",
                    GetResourceString(currentConnect), GetResourceString(inputConnect));
        return NOT_OK;
    }

    if(SetResourceConnect(currentConnect, inputConnect) != TRUE)
        return NOT_OK;


    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

#ifdef SNDOUT_DEV_PLUGNPLAY
    switch(currentConnect)
    {
        case HAL_AUDIO_RESOURCE_OUT_SCART:
        {
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SCART_ON;
            audioConfig.value[0] = 1;
            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] scart on failed\n", __func__, __LINE__);
                return NOT_OK;
            }
            break;
        }
        case HAL_AUDIO_RESOURCE_OUT_SPK:
            //SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_SB_SPDIF:
            SNDOut_SetMute(ENUM_DEVICE_SPDIF, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            Update_RawMode_by_connection();
            break;
        case HAL_AUDIO_RESOURCE_OUT_SPDIF:
            SNDOut_SetMute(ENUM_DEVICE_SPDIF, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_HP:
            SNDOut_SetMute(ENUM_DEVICE_HEADPHONE, ADEC_HP_CheckConnect(), g_AudioStatusInfo.curHPMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_SPDIF_ES:
            //SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_ES_CheckConnect(), GetDecESMute(Aud_mainDecIndex));
            break;
        default:
            break;
    }
#else
    if(currentConnect == HAL_AUDIO_RESOURCE_OUT_SCART)
    {
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SCART_ON;
        audioConfig.value[0] = 1;
        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] scart on failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }
#endif

    if(IsADECSource(inputConnect))
    {
    	HAL_AUDIO_ADEC_INDEX_T adecIndex;
        RAL_AUDIO_SetDolbyDRCMode(res2adec(inputConnect), GetDecDrcMode((SINT32) res2adec(inputConnect) )); // revert ori DRC setting after multi-view switch
        RAL_AUDIO_SetDownMixMode( res2adec(inputConnect),  GetDecDownMixMode((SINT32) res2adec(inputConnect) )); // revert ori DownMix setting after multi-view switch

        adecIndex = res2adec(inputConnect);
        ADSP_DEC_SetMute(adecIndex, ADEC_CheckConnect(adecIndex), GetDecInMute(adecIndex));// connect , check if need to unmute
    }

    if(IsAMIXSource(inputConnect) == TRUE)
    {
        HAL_AUDIO_MIXER_INDEX_T amixerIndex= res2amixer(inputConnect);
        ADSP_AMIXER_SetMute(amixerIndex, AMIXER_CheckConnect(inputConnect), GetAmixerUserMute(amixerIndex));
    }

    return OK;  //OK or NOT_OK
}


DTV_STATUS_T RHAL_AUDIO_SNDOUT_Connect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    DTV_STATUS_T status;
    INFO("%s Enter.\n", __func__);

    status =  RAL_AUDIO_SNDOUT_Connect(currentConnect, inputConnect);  //OK or NOT_OK
    if(status != OK)
        return status;

    return status;
}


DTV_STATUS_T RHAL_AUDIO_SNDOUT_Disconnect(HAL_AUDIO_RESOURCE_T currentConnect, HAL_AUDIO_RESOURCE_T inputConnect)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s cur %s ipt %s\n", __func__,
                GetResourceString(currentConnect), GetResourceString(inputConnect));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    // check current resource is valid
    if((IsAOutSource(currentConnect) != TRUE) || IsSESource(currentConnect))
    {
        ERROR("[AUDH][Error] %s != SNDOUT device\n", GetResourceString(currentConnect));
        return NOT_OK;
    }

    switch(currentConnect)
    {
        case HAL_AUDIO_RESOURCE_OUT_SPK:
            /* SPK & SE should have never been disconnected */
            if(IsSESource(inputConnect))
            {
                return OK;
            }
            break;
        case HAL_AUDIO_RESOURCE_OUT_SPDIF:
            /* check SE input status before disconnect */
            if(IsSESource(inputConnect) && ADEC_SPDIF_CheckConnect() == TRUE)
            {
                INFO("[AUDH] SE inputs > 0, Keep %s - %s connected\n",
                        GetResourceString(inputConnect), GetResourceString(currentConnect));
                return OK;
            }
            break;
        default:
            break;
    }

    if(SetResourceDisconnect(currentConnect, inputConnect) != TRUE)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

#ifdef SNDOUT_DEV_PLUGNPLAY
    switch(currentConnect)
    {
        case HAL_AUDIO_RESOURCE_OUT_SCART:
        {
            memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
            audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SCART_ON;
            audioConfig.value[0] = 0;
            if (SendRPC_AudioConfig(&audioConfig) != S_OK)
            {
                ERROR("[%s,%d] scart on failed\n", __func__, __LINE__);
                return NOT_OK;
            }
            break;
        }
        case HAL_AUDIO_RESOURCE_OUT_SPK:
            //SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_SPDIF:
            SNDOut_SetMute(ENUM_DEVICE_SPDIF, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_CheckConnect(), g_AudioStatusInfo.curSPDIFMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_SB_SPDIF:
            Update_RawMode_by_connection();
            break;
        case HAL_AUDIO_RESOURCE_OUT_HP:
            SNDOut_SetMute(ENUM_DEVICE_HEADPHONE, ADEC_HP_CheckConnect(), g_AudioStatusInfo.curHPMuteStatus);
            break;
        case HAL_AUDIO_RESOURCE_OUT_SPDIF_ES:
            //SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_ES_CheckConnect(), GetDecESMute(Aud_mainDecIndex));
            break;
        default:
            break;
    }
#else
    if(currentConnect == HAL_AUDIO_RESOURCE_OUT_SCART)
    {
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SCART_ON;
        audioConfig.value[0] = 0;
        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] scart on failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }
#endif

    if(IsADECSource(inputConnect))
    {
        HAL_AUDIO_ADEC_INDEX_T adecIndex = res2adec(inputConnect);
        ADSP_DEC_SetMute(adecIndex, ADEC_CheckConnect(adecIndex), GetDecInMute(adecIndex));// disconnect , auto mute
    }

    if(IsAMIXSource(inputConnect) == TRUE)
    {
        HAL_AUDIO_MIXER_INDEX_T amixerIndex= res2amixer(inputConnect);
        ADSP_AMIXER_SetMute(amixerIndex, AMIXER_CheckConnect(inputConnect), GetAmixerUserMute(amixerIndex));
    }

    return OK;  //OK or NOT_OK
}


static BOOLEAN AddAndConnectDecAinToFlow(FlowManager* pFlowManager, HAL_AUDIO_ADEC_INDEX_T adecIndex, Base* pAin, HAL_AUDIO_FLOW_STATUS* pFlowStatus)
{
    if(pFlowManager == NULL)
        return FALSE;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return FALSE;

    if(pAin == NULL)
        return FALSE;

    if(pFlowStatus == NULL)
        return FALSE;

    if(adecIndex == Aud_mainDecIndex)
    {
        pFlowStatus->IsAINExist = TRUE;
    }
    else
    {
        pFlowStatus->IsSubAINExist = TRUE;
    }

    if (adecIndex == HAL_AUDIO_ADEC0)
    {
        pFlowStatus->IsDEC0Exist = TRUE;
    }

    if (adecIndex == HAL_AUDIO_ADEC1)
    {
        pFlowStatus->IsDEC1Exist = TRUE;
    }

    pFlowManager->Connect(pFlowManager, pAin, Aud_dec[adecIndex]);

    pFlowStatus->AinConnectDecIndex = adecIndex;

    return TRUE;
}

static BOOLEAN AddAndConnectPPAOToFlow(FlowManager* pFlowManager, HAL_AUDIO_ADEC_INDEX_T adecIndex, Base* pPPAO, HAL_AUDIO_FLOW_STATUS* pFlowStatus)
{
    if(pFlowManager == NULL)
        return FALSE;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return FALSE;

    if (pPPAO == NULL)
        return FALSE;

    if(pFlowStatus == NULL)
        return FALSE;

    if(pPPAO == Aud_ppAout)
        pFlowStatus->IsMainPPAOExist = TRUE;
    else
    {
        INFO("set sub pp\n");
        pFlowStatus->IsSubPPAOExist = TRUE;
    }

    pFlowManager->Connect(pFlowManager, Aud_dec[adecIndex], pPPAO);

    return TRUE;
}

static BOOLEAN AddAndConnectDTVSourceFilterDecToFlow(FlowManager* pFlowManager, HAL_AUDIO_ADEC_INDEX_T adecIndex, Base* pAud_DTV, HAL_AUDIO_FLOW_STATUS* pFlowStatus)
{
    HAL_AUDIO_RESOURCE_T curResourceId, decIptResId;

    if(pFlowManager == NULL)
        return FALSE;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return FALSE;

    if(pFlowStatus == NULL)
        return FALSE;

   //assert(adecIndex < 2);
    curResourceId = adec2res(adecIndex);

    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source

    if(decIptResId == HAL_AUDIO_RESOURCE_ATP0)
        pFlowStatus->IsDTV0SourceRead = TRUE;
    else if(decIptResId == HAL_AUDIO_RESOURCE_ATP1)
        pFlowStatus->IsDTV1SourceRead = TRUE;
    else
    {
        AUDIO_FATAL("[AUDH-FATAL] unknow atp resource id %d \n", decIptResId);
       //assert(0);
    }

    if (adecIndex == HAL_AUDIO_ADEC0)
    {
        pFlowStatus->IsDEC0Exist = TRUE;
    }

    if (adecIndex == HAL_AUDIO_ADEC1)
    {
        pFlowStatus->IsDEC1Exist = TRUE;
    }

    return TRUE;
}


static BOOLEAN SwitchADCFocus(Base* pAin)
{
    SINT32 adcInputPin;
    if((adcInputPin = GetCurrentADCConnectPin()) >= 0)
    {
        AUDIO_IPT_SRC audioInput;
        memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

        audioInput.focus[0] = AUDIO_IPT_SRC_BBADC;
        audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;

        if(adcInputPin == AUDIO_BBADC_SRC_AIO_PORT_NUM)
            audioInput.mux_in = AUDIO_BBADC_SRC_AIO1;
        else if(adcInputPin == AUDIO_BBADC_SRC_AIN1_PORT_NUM)
            audioInput.mux_in = AUDIO_BBADC_SRC_AIN1;
        else if(adcInputPin == AUDIO_BBADC_SRC_AIN2_PORT_NUM)
            audioInput.mux_in = AUDIO_BBADC_SRC_AIN2;
        else if(adcInputPin == AUDIO_BBADC_SRC_AIN3_PORT_NUM)
            audioInput.mux_in = AUDIO_BBADC_SRC_AIN3;
        else
        {
            audioInput.mux_in = AUDIO_BBADC_SRC_MUTE_ALL;
            INFO("ain mut in set mute %x \n",  audioInput.mux_in);
        }

        //audioInput.mux_in = AUDIO_BBADC_SRC_AIN1;
        INFO("Force let adc input pin is ain1 for demo\n");
        INFO("ADC connect pin %d  set fw ain mut in index = %x \n",  adcInputPin, audioInput.mux_in);

        pAin->SwitchFocus(pAin, &audioInput);
        return TRUE;
    }
    else
    {
        ERROR("find adc pin failed\n");
        return FALSE;
    }
}

static BOOLEAN SwitchHDMIFocus(Base* pAin)
{
    SINT32 hdmiInputPin;
    if((hdmiInputPin = GetCurrentHDMIConnectPin()) >= 0)
    {
        AUDIO_IPT_SRC audioInput;
        memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

        audioInput.focus[0] = AUDIO_IPT_SRC_SPDIF;
        audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.mux_in = AUDIO_SPDIFI_SRC_HDMI;
        pAin->SwitchFocus(pAin, &audioInput);
        return TRUE;
    }
    else
    {
        ERROR("find hdmi pin failed\n");
        return FALSE;
    }
}

static BOOLEAN SwitchDPFocus(Base* pAin)
{
    if (GetCurrentDPConnectPin() >= 0) {
        AUDIO_IPT_SRC audioInput;
        memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

        audioInput.focus[0] = AUDIO_IPT_SRC_SPDIF;
        audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;
        audioInput.mux_in = AUDIO_SPDIFI_SRC_DISPLAY_PORT;
        pAin->SwitchFocus(pAin, &audioInput);
        return TRUE;
    }
    else
    {
        ERROR("find display port pin failed\n");
        return FALSE;
    }
}

static BOOLEAN SwitchATVFocus(Base* pAin)
{
    AUDIO_IPT_SRC audioInput;
    memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

    audioInput.focus[0] = AUDIO_IPT_SRC_ATV;
    audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
    audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
    audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;
//  audioInput.mux_in = AIO_SPDIFI_HDMI; // do not care
    pAin->SwitchFocus(pAin, &audioInput);

    return TRUE;
}

BOOLEAN RemoveFilter(HAL_AUDIO_FLOW_STATUS* prevStatus, FlowManager* pFlowManager)
{
    if(prevStatus->IsAINExist)
    {
        INFO("remove main ain\n");
        pFlowManager->Remove(pFlowManager, Aud_MainAin);
    }
    prevStatus->IsAINExist =0;

    if(prevStatus->IsSubAINExist)
    {
        INFO("remove sub ain\n");
        pFlowManager->Remove(pFlowManager, Aud_SubAin);
    }
    prevStatus->IsSubAINExist = 0;

    if(prevStatus->IsDTV0SourceRead)
    {
        /* Reset external reference clock */
        pFlowManager->SetExtRefClock(pFlowManager, 0);
    }
    prevStatus->IsDTV0SourceRead = 0;

    if(prevStatus->IsDTV1SourceRead)
    {
        /* Reset external reference clock */
        pFlowManager->SetExtRefClock(pFlowManager, 0);
    }
    prevStatus->IsDTV1SourceRead = 0;

    if(prevStatus->IsDEC0Exist)
    {
        pFlowManager->Remove(pFlowManager, Aud_dec[0]);
    }
    prevStatus->IsDEC0Exist = 0;

    if(prevStatus->IsDEC1Exist)
    {
        pFlowManager->Remove(pFlowManager, Aud_dec[1]);
    }
    prevStatus->IsDEC1Exist = 0;

    if(prevStatus->IsMainPPAOExist)
    {
        pFlowManager->Remove(pFlowManager, Aud_ppAout);
    }
    prevStatus->IsMainPPAOExist = 0;

    if(prevStatus->IsSubPPAOExist)
    {
        pFlowManager->Remove(pFlowManager, Aud_subPPAout);
    }
    prevStatus->IsSubPPAOExist = 0;

    pFlowManager->ShowCurrentExitModule(pFlowManager, 0);


    return TRUE;
}

BOOLEAN check_LiveTV_SB_PCM(HAL_AUDIO_RESOURCE_T *source)
{
    HAL_AUDIO_RESOURCE_T adec = HAL_AUDIO_RESOURCE_NO_CONNECTION;
    HAL_AUDIO_RESOURCE_T inputIDArr[HAL_MAX_OUTPUT];
    SINT32 num_of_inputs = 0;
    SINT32 i;

    *source = HAL_AUDIO_RESOURCE_NO_CONNECTION;

    for(i=HAL_AUDIO_RESOURCE_ADEC0; i<=HAL_AUDIO_RESOURCE_ADEC1; i++)
    {
        num_of_inputs = 0;
        GetInputConnectSource((HAL_AUDIO_RESOURCE_T)i, inputIDArr, HAL_MAX_OUTPUT, &num_of_inputs);

        if (num_of_inputs > 0) // has connected
        {
        	SINT32 j;
            for(j=0; j<num_of_inputs; j++)
            {
                if( (inputIDArr[j] == HAL_AUDIO_RESOURCE_ATP0) ||
                    (inputIDArr[j] == HAL_AUDIO_RESOURCE_ATP1) ||
                    (inputIDArr[j] == HAL_AUDIO_RESOURCE_ADC) ||
                    (inputIDArr[j] == HAL_AUDIO_RESOURCE_HDMI) ||
                    (inputIDArr[j] == HAL_AUDIO_RESOURCE_AAD) ||
                    (inputIDArr[j] == HAL_AUDIO_RESOURCE_SYSTEM))
                {
                    if(ADEC_SB_PCM_CheckConnect(&adec))
                    {
                        if((adec) == (HAL_AUDIO_RESOURCE_T)i)
                        {
                            *source = inputIDArr[j];
                            ERROR("[AUDH] %s ---- %d-%d-BT ----\n", __func__,(SINT32)(*source), (SINT32)adec);
                            return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

BOOL IsConnectToSoundOutModule(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_RESOURCE_T curResourceId;
    HAL_AUDIO_RESOURCE_T outputResId[HAL_DEC_MAX_OUTPUT_NUM];

    SINT32 num_of_outputs;
    SINT32 i;

    curResourceId = adec2res(adecIndex);

    /* Check Output source and Error handling */
    GetConnectOutputSource(curResourceId, outputResId, HAL_DEC_MAX_OUTPUT_NUM, &num_of_outputs);

    if(num_of_outputs > HAL_DEC_MAX_OUTPUT_NUM)
    {
        ERROR("[AUDH][Error] decode output connect error %d\n", num_of_outputs);
    }

    for(i = 0; i < num_of_outputs; i++)
    {
        if(IsAOutSource(outputResId[i]))
        {
            INFO("[AUDH] ADEC_%d is connected to %s\n",adecIndex, GetResourceString(outputResId[i]));

            return 1;
        }
    }
    return 0;
}

DTV_STATUS_T RAL_AUDIO_StopDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_FLOW_STATUS* pCurFlowStatus;
    FlowManager* pFlow;
    HAL_AUDIO_RESOURCE_T curResourceId;
    HAL_AUDIO_RESOURCE_T decIptResId;
    INFO("[AUDH] %s ADEC%d\n", __func__, adecIndex);

    if(GetTrickState(adecIndex) == HAL_AUDIO_TRICK_PAUSE)
    {
          INFO("Stop change to Pause API \n");
          return RAL_AUDIO_PauseDecoding(adecIndex, ResourceStatus, sizeof(ResourceStatus));
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("before Stop Decoding flow DEC0 (main) info::\n");
    else
        INFO("before Stop Decoding flow DEC0 (sub):: \n");
    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("before Stop Decoding flow DEC1 (main) info::\n");
    else
        INFO("before Stop Decoding flow DEC1 (sub):: \n");
    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_STOP, 0); // HAL_AUDIO_RESOURCE_NO_CONNECTION for show current state
    curResourceId = adec2res(adecIndex);

    if(AUDIO_HAL_CHECK_ISATSTOPSTATE(ResourceStatus[curResourceId], 0))
    {
        INFO("[AUDH] ADEC %d is at stop state\n", adecIndex);
        return OK;
    }

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0)) // decoder only has one input
    {
        ERROR("[AUDH] %s still at %s status\n", GetResourceString(curResourceId),
                GetResourceStatusString(ResourceStatus[curResourceId].connectStatus[0]));
        return NOT_OK;
    }

    ResourceStatus[curResourceId].connectStatus[0] = HAL_AUDIO_RESOURCE_STOP;

    // update adec status info
    Aud_Adec_info[adecIndex].bAdecStart = FALSE;
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source

    if (adecIndex == Aud_mainDecIndex)
    {
    }
    else
    {
        // sub
        if(IsAinSource(decIptResId)) // analog source
        {
            ((PPAO*)Aud_ppAout->pDerivedObj)->SetSubChannel(Aud_ppAout, 0);
        }
    }

    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        pCurFlowStatus = &MainFlowStatus;
        pFlow = Aud_flow0;
    }
    else
    {
        pCurFlowStatus = &SubFlowStatus;
        pFlow = Aud_flow1;
    }

    if(adecIndex == Aud_mainDecIndex)
    {
        HAL_AUDIO_RESOURCE_T decIptResId;
        // main
        pFlow->Stop(pFlow);

        //Aud_flow0->Disconnect(Aud_flow0, Aud_dec[adecIndex]);
        decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source

        // main
        if(IsATVSource(decIptResId))
        {
            KADP_Audio_AtvSetAudioInHandle(0); // remove AIN filter
            g_AudioSIFInfo.curSifStandard = HAL_AUDIO_SIF_MODE_DETECT;
            KADP_Audio_AtvStopDecoding();
        }

        if(IsAinSource(decIptResId))
        {
            AUDIO_IPT_SRC audioInput;
            memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

            audioInput.focus[0] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;
            Aud_MainAin->SwitchFocus(Aud_MainAin, &audioInput); // unfocus
        }
    }
    else
    {
        // turn off sub channel
        HAL_AUDIO_RESOURCE_T decOptResourceId[HAL_DEC_MAX_OUTPUT_NUM];
        SINT32 totalOutputConnectResource;
        SINT32 i;

        if(Aud_subDecEnable == 1)
        {
            pFlow->Stop(pFlow);
        }

        if(IsATVSource(decIptResId))
        {
            KADP_Audio_AtvSetSubAudioInHandle(0); // remove Sub AIN filter
            g_AudioSIFInfo.curSifStandard = HAL_AUDIO_SIF_MODE_DETECT; // Frank need to check this
            KADP_Audio_AtvStopDecoding();
        }

        if(IsAinSource(decIptResId))
        {
            AUDIO_IPT_SRC audioInput;
            memset(&audioInput, 0, sizeof(AUDIO_IPT_SRC));

            audioInput.focus[0] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[1] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[2] = AUDIO_IPT_SRC_UNKNOWN;
            audioInput.focus[3] = AUDIO_IPT_SRC_UNKNOWN;
            Aud_SubAin->SwitchFocus(Aud_SubAin, &audioInput); // unfocus
        }

        GetConnectOutputSource(curResourceId, decOptResourceId, ( sizeof(decOptResourceId) / sizeof(decOptResourceId[0])), &totalOutputConnectResource);
        if(totalOutputConnectResource > (SINT32)(sizeof(decOptResourceId) / sizeof(decOptResourceId[0])))
        {
            ERROR("[AUDH] decode output connect number error %d \n", totalOutputConnectResource);
            return NOT_OK;
        }

        // turn off sub channel
        for(i = 0; i < totalOutputConnectResource; i++)
        {
            if(decOptResourceId[i] == HAL_AUDIO_RESOURCE_OUT_SCART)
            {
                ((PPAO*)Aud_subPPAout->pDerivedObj)->SetSubChannel(Aud_subPPAout, 0);
            }
        }
    }

    RemoveFilter(pCurFlowStatus, pFlow);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("after Stop Decoding flow DEC0 (main) info::\n");
    else
        INFO("after Stop Decoding flow DEC0 (sub):: \n");
    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("after Stop Decoding flow DEC1 (main) info::\n");
    else
        INFO("after Stop Decoding flow DEC1 (sub):: \n");
    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    return OK;  //OK or NOT_OK
}

static DTV_STATUS_T RAL_AUDIO_StartDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T audioType, SINT32 force2reStart)
{
    HAL_AUDIO_RESOURCE_T curDecResourceId, decIptResId;
    HAL_AUDIO_FLOW_STATUS* pCurFlowStatus;
    Base* pAin;
    Base* pPPAo;
    FlowManager* pFlow;
    SINT32 i;
    HAL_AUDIO_RESOURCE_T outputResId[HAL_DEC_MAX_OUTPUT_NUM];
    SINT32 num_of_outputs;
    SINT32 output_scart_resID = -1;

    INFO("[AUDH] %s ADEC%d type %s\n", __func__, adecIndex, GetSRCTypeName(audioType));

    curDecResourceId = adec2res(adecIndex);

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if(audioType > HAL_AUDIO_SRC_TYPE_OPUS)
    {
        ERROR("[AUDH] unknow dec type %d\n", audioType);
        return NOT_OK;
    }

    if(GetTrickState(adecIndex) == HAL_AUDIO_TRICK_NORMAL_PLAY && AUDIO_HAL_CHECK_ISATPAUSESTATE(ResourceStatus[curDecResourceId], 0))
    {
        INFO("start change to resume API \n");
        return RAL_AUDIO_ResumeDecoding(adecIndex, audioType, ResourceStatus, sizeof(ResourceStatus));
    }

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_RUN, 0);

    // open ADEC ready check
    if(AUDIO_HAL_CHECK_ISATRUNSTATE(ResourceStatus[curDecResourceId], 0))
    {
        if((force2reStart == 0) && audioType == GetCurrentFormat(adecIndex))
        {
            INFO("[AUDH] ADEC %d is at run state and format %d is same \n", adecIndex, audioType);
            return OK;
        }
        else
        {
            INFO("[AUDH] ADEC %d is at run state and format change(%d->%d) , need auto stop \n", adecIndex, GetCurrentFormat(adecIndex), audioType);
            RAL_AUDIO_StopDecoding(adecIndex);
        }
    }

    if(AUDIO_HAL_CHECK_PLAY_NOTAVAILABLE(ResourceStatus[curDecResourceId], 0))
    {
        ERROR("[AUDH] %s status is at %s \n", GetResourceString(curDecResourceId),
                GetResourceStatusString(ResourceStatus[curDecResourceId].connectStatus[0]));
        return NOT_OK;
    }

    if((m_IsSetMainDecOptByMW == FALSE)||((Aud_mainDecIndex != HAL_AUDIO_ADEC0)&&(Aud_mainDecIndex != HAL_AUDIO_ADEC1)))
    {
        if(IsConnectToSoundOutModule(adecIndex))
        {
            Aud_mainDecIndex = adecIndex;
            INFO("[AUDH] %s:%d MainDecOutput is not set. Now, set auto, adecIndex=%d,Aud_mainDecIndex=%d\n",__func__,__LINE__, adecIndex, Aud_mainDecIndex);
        }
    }

    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        pFlow = Aud_flow0;
        pCurFlowStatus = &MainFlowStatus;
        pPPAo = Aud_ppAout;
    }
    else
    {
        pFlow = Aud_flow1;
        pCurFlowStatus = &SubFlowStatus;
        pPPAo = Aud_subPPAout;
    }

    if(adecIndex == Aud_mainDecIndex)
    {
        pAin = Aud_MainAin;
        pPPAo = Aud_ppAout;
//        pFlow = Aud_flow0;

    }
    else
    {
        pAin = Aud_SubAin;
        pPPAo = Aud_subPPAout;
//        pFlow = Aud_flow1;

    }

    if(pFlow == NULL || pPPAo == NULL /*|| pAin == NULL*/ || pCurFlowStatus == NULL)
    {
        ERROR("Flow usage is NULL\n");
        goto error;
    }

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("before Start Decoding flow DEC0 (main) info::\n");
    else
        INFO("before Start Decoding flow DEC0 (sub):: \n");
    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("before Start Decoding flow DEC1 (main) :: \n");
    else
        INFO("before Start Decoding flow DEC1 (sub) :: \n");
    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    pFlow->Stop(pFlow);
    RemoveFilter(pCurFlowStatus, pFlow);

    /* Check Input source and Error handling */
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curDecResourceId]);// dec input source

    if(IsAinSource(decIptResId) || IsDTVSource(decIptResId) || IsSystemSource(decIptResId))
    {
        INFO("dec input is %s\n", GetResourceString(decIptResId));
    }
    else
    {
        ERROR("[AUDH][Error] Invalid dec input src %s\n", GetResourceString(decIptResId));
        goto error;
    }
    /* End of Input source checking */

    /* Check Output source and Error handling */
    GetConnectOutputSource(curDecResourceId, outputResId, HAL_DEC_MAX_OUTPUT_NUM, &num_of_outputs);
    if(num_of_outputs > HAL_DEC_MAX_OUTPUT_NUM)
    {
        ERROR("[AUDH][Error] decode output connect error %d\n", num_of_outputs);
        goto error;
    }

    for(i = 0; i < num_of_outputs; i++)
    {
        if(outputResId[i] == HAL_AUDIO_RESOURCE_OUT_SCART)
            output_scart_resID = outputResId[i];
    }

    if(adecIndex == Aud_mainDecIndex)
    {
        if((output_scart_resID != -1) && IsAinSource(decIptResId))
        {
            ERROR("[AUDH][Error] Scart can't connect to main ADEC%d at %s source\n",
                    Aud_mainDecIndex, GetResourceString(decIptResId));
            goto error;
        }
    }
    /* End of Output source checking */

    /* Connect to Input Source */
    if(IsAinSource(decIptResId) == TRUE)
    {
        // analog flow
        AddAndConnectDecAinToFlow(pFlow, adecIndex, pAin, pCurFlowStatus);

        if(IsADCSource(decIptResId) && SwitchADCFocus(pAin) != TRUE)
        {
            goto error;
        }
        else if(IsHDMISource(decIptResId))
        {
            if(SwitchHDMIFocus(pAin) != TRUE)
                goto error;

            ADSP_DEC_SetHdmiFmt(adecIndex, audioType);
        }
        else if(IsDPSource(decIptResId))
        {
            if(SwitchDPFocus(pAin) != TRUE)
                goto error;

            ADSP_DEC_SetHdmiFmt(adecIndex, audioType);
        }
        else if(IsATVSource(decIptResId))
        {
            SwitchATVFocus(pAin);

            if (adecIndex == Aud_mainDecIndex)
                KADP_Audio_AtvSetAudioInHandle(pAin->GetAgentID(pAin));
            else
                KADP_Audio_AtvSetSubAudioInHandle(pAin->GetAgentID(pAin));// sub ain

            g_AudioSIFInfo.curSifStandard = HAL_AUDIO_SIF_MODE_DETECT;
            KADP_Audio_AtvStartDecoding();
        }
    }
    else if(IsDTVSource(decIptResId))
    {
        SINT32 atpindex = decIptResId - HAL_AUDIO_RESOURCE_ATP0;
        DEC* pDEC = (DEC*)Aud_dec[atpindex]->pDerivedObj;
        DtvCom* pDtvCom = (DtvCom*)Aud_DTV[atpindex]->pDerivedObj;
        INFO("DTV flow \n");

        AddAndConnectDTVSourceFilterDecToFlow(pFlow, adecIndex, Aud_DTV[atpindex], pCurFlowStatus);

        pFlow->SetExtRefClock(pFlow, pDtvCom->GetRefClockPhyAddress(Aud_DTV[atpindex]));
        pDEC->InitBSRingBuf(Aud_dec[adecIndex], pDtvCom->GetBSRingBufPhyAddress(Aud_DTV[atpindex]));
        pDEC->InitICQRingBuf(Aud_dec[adecIndex], pDtvCom->GetICQRingBufPhyAddress(Aud_DTV[atpindex]));
    }

    /* Connect to Output source */
    {
        /* Connect to PPAO */
        AddAndConnectPPAOToFlow(pFlow, adecIndex, pPPAo, pCurFlowStatus);

        if (adecIndex == Aud_mainDecIndex)
        {
            INFO("[AUDH] focus \n");
            pPPAo->SwitchFocus(pPPAo, NULL);
            //wait AO finish swithing focus
            msleep(20);
        }
        else if (m_IsHbbTV)
        {
            INFO("[AUDH] focus for HbbTV \n");
            pPPAo->SwitchFocus(pPPAo, NULL);
            m_IsHbbTV = FALSE;
            //wait AO finish swithing focus
            msleep(100);
            SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_HEADPHONE, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SCART, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SPDIF, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
            SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPK_CheckConnect(), g_AudioStatusInfo.curSPKMuteStatus);
        }
        else if((output_scart_resID != -1) && IsAinSource(decIptResId))
        {
             INFO("[AUDH] not focus \n");
            // analog pip case via scart out
            ((PPAO*)pPPAo->pDerivedObj)->SetSubChannel(pPPAo, 1);
        }
    }

    // update adec info status
    Aud_Adec_info[adecIndex].prevAdecFormat = Aud_Adec_info[adecIndex].curAdecFormat;
    Aud_Adec_info[adecIndex].curAdecFormat  = audioType;
    Aud_Adec_info[adecIndex].bAdecStart     = TRUE;

    if(adecIndex != Aud_mainDecIndex) Aud_subDecEnable = 1;

    /* update resource status */
    ResourceStatus[curDecResourceId].connectStatus[0] = HAL_AUDIO_RESOURCE_RUN;

    SetTrickState(adecIndex, HAL_AUDIO_TRICK_NONE);

    pFlow->Flush(pFlow);
    pFlow->Run(pFlow);

    if(IsDTVSource(decIptResId))
    {
        SINT32 newfmt[9] = {0};
        Base* pDtv = Aud_DTV[decIptResId - HAL_AUDIO_RESOURCE_ATP0];
        DEMUX_CHANNEL_T demux_ch = (DEMUX_CHANNEL_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECChannel(pDtv);
        RSDEC_ENUM_TYPE_DEST_T sdec_dest = (RSDEC_ENUM_TYPE_DEST_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECDest(pDtv);
        // AUDIO_DEC_NEW_FORMAT:: audio type + private info[8]
        newfmt[0] = Convert2AudioDecType(audioType);
        if(newfmt[0] == AUDIO_UNKNOWN_TYPE)
            return NOT_OK;

        //newfmt[1] = (newfmt[0] == AUDIO_AAC_DECODER_TYPE)? 1 : 0; // aac latm
        newfmt[7] = 1 << 1; // DTV mode
        KADP_SDEC_PrivateInfo(demux_ch, sdec_dest, DEMUX_PRIVATEINFO_AUDIO_FORMAT_, (ULONG*)newfmt, sizeof(newfmt));
    }

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
      INFO("after Start Decoding flow DEC0 (main) info::\n");

    else
      INFO("after Start Decoding flow DEC0 (sub):: \n");

    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);


    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
      INFO("after Start Decoding flow DEC1 (main) info::\n");

    else
      INFO("after Start Decoding flow DEC1 (sub):: \n");


    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    //ShowFlow();
    return OK;// or NOT_OK

error:

    return NOT_OK;
}

DTV_STATUS_T RHAL_AUDIO_StopDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    DTV_STATUS_T status;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ADEC%d\n", __func__, adecIndex);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    status = RAL_AUDIO_StopDecoding(adecIndex);
    SetDecUserState(adecIndex, 0);
    return status;
}



/* Start & Stop */
// add PlayBack Peter
// add check se not connect to main
DTV_STATUS_T RHAL_AUDIO_StartDecoding(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T audioType)
{
    DTV_STATUS_T status;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ADEC%d type %d\n", __func__, adecIndex, audioType);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    status =  RAL_AUDIO_StartDecoding(adecIndex, audioType, 0);
    SetDecUserState(adecIndex, TRUE);
    return status ;
}

// add swap Peter
DTV_STATUS_T RAL_AUDIO_SetMainDecoderOutput(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_RESOURCE_T id;
    SINT32 isNeedAutoStart = 0;
    SINT32 isNeedSubAutoStart = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("[AUDH] %s to ADEC%d\n", __func__, adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if ((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if((SINT32)adecIndex != Aud_mainDecIndex)
    {
        HAL_AUDIO_RESOURCE_T id;
        // change main flow

        // stop main
        if((Aud_mainDecIndex != -1))
        {
            HAL_AUDIO_RESOURCE_T id = adec2res((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex);
            // The dec might be started by other process, os check here. We'll do auto re-start if it is own by this process
            if(IsResourceRunningByProcess((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex,ResourceStatus[id]))
            {
                if(ResourceStatus[id].connectStatus[0] == HAL_AUDIO_RESOURCE_RUN)
                    isNeedSubAutoStart = 1; // no sub dec type , so need to check sub decoder is at run state
                if(IsHbbTV((HAL_AUDIO_ADEC_INDEX_T)adecIndex, (HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex))
                {
                    // mute SPK before focus change to avoid pop sound
                    ADSP_SNDOut_SetMute(ENUM_DEVICE_SPEAKER, TRUE);
                    ADSP_SNDOut_SetMute(ENUM_DEVICE_HEADPHONE, TRUE);
                    ADSP_SNDOut_SetMute(ENUM_DEVICE_SCART, TRUE);
                    ADSP_SNDOut_SetMute(ENUM_DEVICE_SPDIF, TRUE);
                    ADSP_SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, TRUE);
                }
                RAL_AUDIO_StopDecoding((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex);
            }
        }

        // stop current sub
        id = adec2res(adecIndex);

        if(ResourceStatus[id].connectStatus[0] == HAL_AUDIO_RESOURCE_RUN)
        {
            isNeedAutoStart = 1;
            RAL_AUDIO_StopDecoding((HAL_AUDIO_ADEC_INDEX_T)adecIndex);
        }
    }

    id = adec2res(adecIndex);

    Aud_prevMainDecIndex = Aud_mainDecIndex;
    Aud_mainDecIndex     = adecIndex;

#if 1 /* Inform ADSP about the dec port for SPDIF output */
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SPDIF_OUTPUT_SWITCH;
    audioConfig.value[0] = Aud_mainDecIndex;
    if(SendRPC_AudioConfig(&audioConfig) != S_OK) {
        ERROR("[%s,%d] SPDIF out switch failed\n", __func__, __LINE__);
        return NOT_OK;
    }
#endif

    SetSPDIFOutType(adecIndex, HAL_AUDIO_SPDIF_AUTO);

    // need to add auto deceoding
    if(isNeedAutoStart == 1)
    {
        if(AUDIO_HAL_CHECK_PLAY_NOTAVAILABLE(ResourceStatus[id], 0))
        {
            ERROR("[AUDH] dec failed status != stop %d \n", ResourceStatus[id].connectStatus[0]);
            return  NOT_OK;
        }
        RAL_AUDIO_StartDecoding((HAL_AUDIO_ADEC_INDEX_T)adecIndex, Aud_Adec_info[adecIndex].curAdecFormat, 0);
    }
    // need to add auto deceoding
    if(isNeedSubAutoStart == 1)
    {
        // If HbbTV, switch focus
        if(IsHbbTV((HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex, (HAL_AUDIO_ADEC_INDEX_T)Aud_prevMainDecIndex))
        {
            m_IsHbbTV = TRUE;
        }
        RAL_AUDIO_StartDecoding((HAL_AUDIO_ADEC_INDEX_T)Aud_prevMainDecIndex, Aud_Adec_info[Aud_prevMainDecIndex].curAdecFormat, 0);
    }
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetMainDecoderOutput(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);

    m_IsSetMainDecOptByMW = TRUE;

    return RAL_AUDIO_SetMainDecoderOutput(adecIndex);  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetMainAudioOutput(HAL_AUDIO_INDEX_T audioIndex)
{
    HAL_AUDIO_ADEC_INDEX_T adecIndex = HAL_AUDIO_ADEC0;
    INFO("%s Enter.\n", __func__);

    m_IsSetMainDecOptByMW = TRUE;

    if (audioIndex > HAL_AUDIO_INDEX_MAX)
    {
        ERROR("[AUDH] Audio Index out of range!!!\n");
        return NOT_OK;
    }

    if((audioIndex != HAL_AUDIO_INDEX0) && (audioIndex != HAL_AUDIO_INDEX1))
    {
        AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
        INFO("[AUDH] %s to audioIndex(%d), force SPDIF/ARC output PCM!!!\n", __func__,audioIndex);
        #if 1 /* Inform ADSP about the dec port for SPDIF/ARC output */
        audioConfig.msg_id = AUDIO_CONFIG_CMD_SPDIF_OUTPUT_SWITCH;
        audioConfig.value[0] = audioIndex;
        if(SendRPC_AudioConfig(&audioConfig) != S_OK) {
            ERROR("[%s,%d] SPDIF out switch failed\n", __func__, __LINE__);
            return NOT_OK;
        }
        #endif
        return OK;
    }

    adecIndex = (audioIndex == HAL_AUDIO_INDEX0) ? HAL_AUDIO_ADEC0 : HAL_AUDIO_ADEC1;

    return RAL_AUDIO_SetMainDecoderOutput(adecIndex);  //OK or NOT_OK
}

/* HDMI */
DTV_STATUS_T RAL_AUDIO_GetDecodingType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T *pAudioType);

HAL_AUDIO_HDMI_TYPE_T ConvertSRCType2HDMIMode(HAL_AUDIO_SRC_TYPE_T src_type)
{
    HAL_AUDIO_HDMI_TYPE_T HDMIMode;
    switch(src_type)
    {
    case HAL_AUDIO_SRC_TYPE_PCM:
        HDMIMode = HAL_AUDIO_HDMI_PCM;
        break;
    case HAL_AUDIO_SRC_TYPE_AC3:
        HDMIMode = HAL_AUDIO_HDMI_AC3;
        break;
    case HAL_AUDIO_SRC_TYPE_MPEG:
        HDMIMode = HAL_AUDIO_HDMI_MPEG;
        break;
    case HAL_AUDIO_SRC_TYPE_AAC:
        HDMIMode = HAL_AUDIO_HDMI_AAC;
        break;
    case HAL_AUDIO_SRC_TYPE_DTS:
        HDMIMode = HAL_AUDIO_HDMI_DTS;
        break;
    case HAL_AUDIO_SRC_TYPE_DTS_HD_MA:
        HDMIMode = HAL_AUDIO_HDMI_DTS_HD_MA;
        break;
    case HAL_AUDIO_SRC_TYPE_DTS_EXPRESS:
        HDMIMode = HAL_AUDIO_HDMI_DTS_EXPRESS;
        break;
    case HAL_AUDIO_SRC_TYPE_DTS_CD:
        HDMIMode = HAL_AUDIO_HDMI_DTS_CD;
        break;
    case HAL_AUDIO_SRC_TYPE_EAC3:
        HDMIMode = HAL_AUDIO_HDMI_EAC3;
        break;
    default:
        HDMIMode = HAL_AUDIO_HDMI_DEFAULT;
        break;
    }
    return HDMIMode;
}

DTV_STATUS_T RAL_AUDIO_HDMI_GetAudioMode(HAL_AUDIO_HDMI_TYPE_T *pHDMIMode)
{
    HAL_AUDIO_SRC_TYPE_T code_type;
    HAL_AUDIO_RESOURCE_T upstreamResourceId0, upstreamResourceId1;
    HAL_AUDIO_ADEC_INDEX_T adecIndex = HAL_AUDIO_ADEC_MAX;
    DTV_STATUS_T Ret = NOT_OK;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(pHDMIMode == NULL)
        return NOT_OK;

    /* finding the decoder which is used by checking the upstream source is HDMI or not. */
    upstreamResourceId0 = GetNonOutputModuleSingleInputResource(ResourceStatus[HAL_AUDIO_RESOURCE_ADEC0]);
    upstreamResourceId1 = GetNonOutputModuleSingleInputResource(ResourceStatus[HAL_AUDIO_RESOURCE_ADEC1]);

    if (IsHDMISource(upstreamResourceId0) && IsHDMISource(upstreamResourceId1)) {
        ERROR("[AUDH] %s:%d Two decoders are both connected to HDMI.\n", __func__,__LINE__);
        return NOT_PERMITTED;
    } else if (IsDPSource(upstreamResourceId0) && IsDPSource(upstreamResourceId1)) {
        ERROR("[AUDH] %s:%d Two decoders are both connected to DP.\n", __func__,__LINE__);
        return NOT_PERMITTED;
    } else if(IsHDMISource(upstreamResourceId0) || IsDPSource(upstreamResourceId0)) {
        adecIndex = HAL_AUDIO_ADEC0;
    } else if(IsHDMISource(upstreamResourceId1) || IsDPSource(upstreamResourceId1)) {
        adecIndex = HAL_AUDIO_ADEC1;
    } else {
        *pHDMIMode = HAL_AUDIO_HDMI_NO_AUDIO;
        return OK;
    }

    if (AUDIO_HAL_CHECK_ISATSTOPSTATE(ResourceStatus[adec2res(adecIndex)], 0)) {
        *pHDMIMode = HAL_AUDIO_HDMI_NO_AUDIO;
        return OK;
    } else {
        BOOLEAN isESExist = FALSE;

        isESExist = ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->IsESExist(Aud_dec[adecIndex]);
        if (isESExist == 0) {
            *pHDMIMode = HAL_AUDIO_HDMI_NO_AUDIO;
            return OK;
        }

        Ret = RAL_AUDIO_GetDecodingType(adecIndex, &code_type);

        if(Ret == OK) {
            if (code_type == HAL_AUDIO_SRC_TYPE_UNKNOWN) {
                *pHDMIMode = HAL_AUDIO_HDMI_NO_AUDIO;
                return OK;
            } else {
                *pHDMIMode = ConvertSRCType2HDMIMode(code_type);
                return OK;
            }
        } else {
            ERROR("[AUDH] %s Fail to get the decoding type!!!\n", __func__);
            *pHDMIMode = HAL_AUDIO_HDMI_NO_AUDIO;
            return NOT_OK;
        }
    }
}

DTV_STATUS_T RHAL_AUDIO_HDMI_GetAudioMode(HAL_AUDIO_HDMI_TYPE_T *pHDMIMode)
{
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    return RAL_AUDIO_HDMI_GetAudioMode(pHDMIMode); //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_HDMI_SetAudioReturnChannel(BOOLEAN bOnOff)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s %d  \n", __func__, bOnOff);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    if(bOnOff) {
        /*SetAudioHDMIARCMode(TRUE, 1<<AUDIO_HDMI_CODING_TYPE_DDP, FALSE);*/
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id    = AUDIO_CONFIG_CMD_HDMI_ARC;
        audioConfig.value[0] = TRUE;
        audioConfig.value[1] = 1<<AUDIO_HDMI_CODING_TYPE_DDP;
        audioConfig.value[2] = FALSE;

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] hdmi arc on failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    } else {
        /*SetAudioHDMIARCMode(FALSE, FALSE, FALSE);*/
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id    = AUDIO_CONFIG_CMD_HDMI_ARC;
        audioConfig.value[0] = FALSE;
        audioConfig.value[1] = FALSE;
        audioConfig.value[2] = FALSE;

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] hdmi arc off failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }
    _ARC_Enable = bOnOff;
    ARC_offon = bOnOff;

    Update_RawMode_by_connection();
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_HDMI_SetEnhancedAudioReturnChannel(BOOLEAN bOnOff)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s %d  \n", __func__, bOnOff);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    if(bOnOff) {
        /*SetAudioHDMIARCMode(TRUE, 1<<AUDIO_HDMI_CODING_TYPE_DDP, FALSE);*/
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id    = AUDIO_CONFIG_CMD_HDMI_EARC;
        audioConfig.value[0] = TRUE;
        audioConfig.value[1] = 1<<AUDIO_HDMI_CODING_TYPE_DDP;
        audioConfig.value[2] = FALSE;

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] hdmi earc on failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    } else {
        /*SetAudioHDMIARCMode(FALSE, FALSE, FALSE);*/
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id    = AUDIO_CONFIG_CMD_HDMI_EARC;
        audioConfig.value[0] = FALSE;
        audioConfig.value[1] = FALSE;
        audioConfig.value[2] = FALSE;

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] hdmi earc off failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }
    _EARC_Enable = bOnOff;
    eARC_offon = bOnOff;

    Update_RawMode_by_connection();
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_HDMI_GetCopyInfo(HAL_AUDIO_SPDIF_COPYRIGHT_T *pCopyInfo)
{
	UINT8 copyright = 0;
    UINT8 category_code = 0;
    KADP_AO_SPDIF_CHANNEL_STATUS_BASIC channel_status;
    AUDIO_VERBOSE("[AUDH] %s %p  \n", __func__, pCopyInfo);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;
    if(pCopyInfo == NULL)
    {
        ERROR("[AUDH] address error \n");
        return NOT_OK;
    }

    memset(&channel_status, 0, sizeof(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC));
    if (KADP_AUDIO_GetAudioSpdifChannelStatus(&channel_status, AUDIO_IN))
    {
        ERROR("[%s,%d] get spdif channel status failed\n", __func__, __LINE__);
        return NOT_OK;
    }
    copyright = channel_status.copyright;
    category_code = channel_status.category_code;

    if(copyright == HAL_SPDIF_COPYRIGHT_FREE)
    {
        *pCopyInfo  = HAL_AUDIO_SPDIF_COPY_FREE; /* cp-bit : 1, L-bit : 0 */
    }
    else if( (category_code & HAL_SPDIF_CATEGORY_L_BIT_IS_1) == HAL_SPDIF_CATEGORY_L_BIT_IS_1)
    {
        *pCopyInfo = HAL_AUDIO_SPDIF_COPY_NEVER;
    }
    else
    {
        *pCopyInfo = HAL_AUDIO_SPDIF_COPY_ONCE; /* cp-bit : 0, L-bit : 0 */
    }

    *pCopyInfo = HAL_AUDIO_SPDIF_COPY_NEVER;
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_HDMI_GetCopyInfo(HAL_AUDIO_SPDIF_COPYRIGHT_T *pCopyInfo)
{
    INFO("%s Enter.\n", __func__);
    AUDIO_VERBOSE("[AUDH] %s %p  \n", __func__, pCopyInfo);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;
    if(pCopyInfo == NULL)
    {
        ERROR("[AUDH] address error == NULL \n");
        return NOT_OK;
    }

    return RAL_AUDIO_HDMI_GetCopyInfo(pCopyInfo);
}

DTV_STATUS_T RHAL_AUDIO_SetDolbyDRCMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOLBY_DRC_MODE_T drcMode)
{
    INFO("%s Enter.\n", __func__);
    return RAL_AUDIO_SetDolbyDRCMode(adecIndex, drcMode);
}

DTV_STATUS_T RHAL_AUDIO_SetDownMixMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DOWNMIX_MODE_T downmixMode)
{
    INFO("%s Enter.\n", __func__);
    return RAL_AUDIO_SetDownMixMode(adecIndex, downmixMode) ;
}

DTV_STATUS_T RHAL_AUDIO_GetDecodingType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_SRC_TYPE_T *pAudioType)
{
    INFO("%s Enter.\n", __func__);// decrease regular print

    return RAL_AUDIO_GetDecodingType(adecIndex, pAudioType);
}

static SINT32 ConvetChannelStatusSampleRate(UINT32 sampleRateIndex)
{
    SINT32 SR_table[] = {44100, 0, 48000, 32000, 22050, 0,
                       24000, 0, 88200, 0, 96000, 0, 176400, 0, 192000 };

    //fix coverity:272, Event overrun-local: Overrunning array
    //if(sampleRateIndex > (sizeof(SR_table)/ sizeof(SR_table[0])))
    if(sampleRateIndex >= (sizeof(SR_table)/ sizeof(SR_table[0])))
    {
        ERROR("[AUDH] Error sample rate index %d \n", sampleRateIndex);
        return 0;
    }

    return (SR_table[sampleRateIndex]);
}

BOOLEAN RAL_AUDIO_TP_IsESExist(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_RESOURCE_T decIptResId;
    HAL_AUDIO_RESOURCE_T curResourceId = adec2res(adecIndex);
    BOOLEAN isESExist = FALSE;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return FALSE;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return FALSE;

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0)) // at run state,  decoder can only have one input
    {
        ERROR("[AUDH] %s is not at run state %d\n",
                GetResourceString(curResourceId), ResourceStatus[curResourceId].connectStatus[0]); // decrease regular print
        return FALSE;
    }

    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source
    if(IsDTVSource(decIptResId) != TRUE)
    {
        ERROR("[AUDH] %s is not at DTV mode (input = %s)\n",
                GetResourceString(curResourceId), GetResourceString(decIptResId)); // decrease regular print
        return FALSE;
    }

    isESExist = ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->IsESExist(Aud_dec[adecIndex]);
    INFO("[AUDH] %s from ADEC%d = %s\n", __func__, adecIndex, isESExist? "TRUE":"FALSE");// decrease regular print
    return isESExist;
}

DTV_STATUS_T RHAL_AUDIO_GetAdecStatus(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ADEC_INFO_T *pAudioAdecInfo)
{
    HAL_AUDIO_RESOURCE_T curResourceId;
    HAL_AUDIO_RESOURCE_T decIptResId;

    INFO("%s Enter.\n", __func__);// decrease regular print
    INFO("[AUDH] %s ADEC%d %p \n", __func__, adecIndex, pAudioAdecInfo);//decrease regular print

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if(pAudioAdecInfo == NULL)
        return NOT_OK;

    curResourceId = adec2res(adecIndex);
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source

    if(IsDTVSource(decIptResId) == TRUE)
        Aud_Adec_info[adecIndex].bAdecESExist = RAL_AUDIO_TP_IsESExist(adecIndex);
    else
        Aud_Adec_info[adecIndex].bAdecESExist = FALSE;

    if(IsHDMISource(decIptResId) || IsDPSource(decIptResId))
    {
        KADP_AO_SPDIF_CHANNEL_STATUS_BASIC channel_status;
        HAL_AUDIO_HDMI_TYPE_T audiomode;
        RAL_AUDIO_HDMI_GetAudioMode(&audiomode);

        /*GetAudioSpdifChannelStatus(&channel_status, AUDIO_IN);*/
        memset(&channel_status, 0, sizeof(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC));
        if (KADP_AUDIO_GetAudioSpdifChannelStatus(&channel_status, AUDIO_IN))
        {
            ERROR("[%s,%d] get spdif channel status failed\n", __func__, __LINE__);
            return NOT_OK;
        }

        Aud_Adec_info[adecIndex].curHdmiAudioType = audiomode;
        Aud_Adec_info[adecIndex].curHdmiSamplingFreq = (HAL_AUDIO_SAMPLING_FREQ_T)ConvetChannelStatusSampleRate(channel_status.sampling_freq);
        RAL_AUDIO_HDMI_GetCopyInfo(&(Aud_Adec_info[adecIndex].curHdmiCopyInfo));
    }
    else
    {
        Aud_Adec_info[adecIndex].curHdmiAudioType = HAL_AUDIO_HDMI_DEFAULT;
        Aud_Adec_info[adecIndex].curHdmiSamplingFreq = HAL_AUDIO_SAMPLING_FREQ_NONE;
        Aud_Adec_info[adecIndex].curHdmiCopyInfo = HAL_AUDIO_SPDIF_COPY_FREE;
    }

    if(IsATVSource(decIptResId))
    {
        Aud_Adec_info[adecIndex].sifSource      = g_AudioSIFInfo.sifSource;
        Aud_Adec_info[adecIndex].curSifType     = g_AudioSIFInfo.curSifType;
        Aud_Adec_info[adecIndex].bHighDevOnOff  = g_AudioSIFInfo.bHighDevOnOff;
        Aud_Adec_info[adecIndex].curSifBand     = g_AudioSIFInfo.curSifBand;
        Aud_Adec_info[adecIndex].curSifStandard = g_AudioSIFInfo.curSifStandard;
        Aud_Adec_info[adecIndex].curSifIsA2     = g_AudioSIFInfo.curSifIsA2;
        Aud_Adec_info[adecIndex].curSifIsNicam  = g_AudioSIFInfo.curSifIsNicam;
        Aud_Adec_info[adecIndex].curSifModeSet  = g_AudioSIFInfo.curSifModeSet;
        Aud_Adec_info[adecIndex].curSifModeGet  = g_AudioSIFInfo.curSifModeGet;
    }
    else
    {
        Aud_Adec_info[adecIndex].sifSource      = HAL_AUDIO_SIF_INPUT_INTERNAL;
        Aud_Adec_info[adecIndex].curSifType     = HAL_AUDIO_SIF_TYPE_NONE;
        Aud_Adec_info[adecIndex].bHighDevOnOff  = FALSE;
        Aud_Adec_info[adecIndex].curSifBand     = HAL_AUDIO_SIF_SYSTEM_UNKNOWN;
        Aud_Adec_info[adecIndex].curSifStandard = HAL_AUDIO_SIF_MODE_DETECT;
        Aud_Adec_info[adecIndex].curSifIsA2     = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
        Aud_Adec_info[adecIndex].curSifIsNicam  = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
        Aud_Adec_info[adecIndex].curSifModeSet  = HAL_AUDIO_SIF_SET_PAL_UNKNOWN;
        Aud_Adec_info[adecIndex].curSifModeGet  = HAL_AUDIO_SIF_GET_PAL_UNKNOWN;
    }

    memcpy(pAudioAdecInfo, &Aud_Adec_info[adecIndex], sizeof(HAL_AUDIO_ADEC_INFO_T));
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetDualMonoOutMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DUALMONO_MODE_T outputMode)
{
    Base *pDec;
    const CHAR modeName[4][4] = {"LR","LL","RR","MIX"};
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    INFO("[AUDH] %s ADEC%d to %s mode\n",__func__, adecIndex, modeName[outputMode]);

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] error dec index %d\n", adecIndex);
        return NOT_OK;
    }

    if(outputMode > HAL_AUDIO_DUALMONO_MODE_MIX)
        return NOT_OK;

    // It should be independent and should work even though calling the function before starting decode
    // Related to WOSQRTK-3606
    /*
    if(Aud_Adec_info[adecIndex].bAdecStart != TRUE)
    {
        ERROR("[AUDH] ADEC%d is not started\n", adecIndex);
        return NOT_OK;
    }
    */

    pDec = Aud_dec[adecIndex];

    if(outputMode == HAL_AUDIO_DUALMONO_MODE_LR)
        ((DEC*)pDec->pDerivedObj)->SetChannelSwap(pDec, AUDIO_CHANNEL_OUT_SWAP_STEREO);
    else if(outputMode == HAL_AUDIO_DUALMONO_MODE_LL)
        ((DEC*)pDec->pDerivedObj)->SetChannelSwap(pDec, AUDIO_CHANNEL_OUT_SWAP_L_TO_R);
    else if(outputMode == HAL_AUDIO_DUALMONO_MODE_RR)
        ((DEC*)pDec->pDerivedObj)->SetChannelSwap(pDec, AUDIO_CHANNEL_OUT_SWAP_R_TO_L);
    else  //HAL_AUDIO_DUALMONO_MODE_MIX
        ((DEC*)pDec->pDerivedObj)->SetChannelSwap(pDec, AUDIO_CHANNEL_OUT_SWAP_LR_MIXED);

    Aud_Adec_info[adecIndex].curAdecDualmonoMode = outputMode;
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_TP_GetESInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo)
{
    #define AAC_CHANNEL_DUALMONO (MODE_11)
    #define AAC_CHANNEL_MONO (MODE_10)
    #define AAC_CHANNEL_STEREO (MODE_20)
    #define AAC_CHANNEL_MULTI (MODE_32)

    HAL_AUDIO_RESOURCE_T decIptResId;
    HAL_AUDIO_RESOURCE_T curResourceId = adec2res(adecIndex);
    UINT16 sampleRate;

    INFO("[AUDH] %s from %s\n", __func__, GetResourceString(curResourceId));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) {
        INFO("[AUDH] %s from %s fail not init\n", __func__, GetResourceString(curResourceId));
        return NOT_OK;
    }

    if(pAudioESInfo == NULL) {
        INFO("[AUDH] %s from %s fail pAudioESInfo null\n", __func__, GetResourceString(curResourceId));
        return NOT_OK;
    }

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1)) {
        INFO("[AUDH] %s from %s fail adecIndex=%d not in range\n", __func__, GetResourceString(curResourceId), adecIndex);
        return NOT_OK;
    }


    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0)) //decoder can only have one input
    {
        INFO("[AUDH] %s is not at run state %d\n",
                GetResourceString(curResourceId), ResourceStatus[curResourceId].connectStatus[0]);
        return NOT_OK;
    }

    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source
    if(IsDTVSource(decIptResId) != TRUE)
    {
        INFO("[AUDH] %s is not at DTV mode (input = %s)\n",
                GetResourceString(curResourceId), GetResourceString(decIptResId));
        return NOT_OK;
    }

    RAL_AUDIO_GetESInfo(adecIndex, pAudioESInfo, &sampleRate);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_TP_GetESInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo)
{
    INFO("%s Enter.\n", __func__); // decrease regular print
    return RAL_AUDIO_TP_GetESInfo(adecIndex, pAudioESInfo);    //OK or NOT_OK
}

BOOLEAN RHAL_AUDIO_TP_IsESExist(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);// decrease regular print
    return RAL_AUDIO_TP_IsESExist(adecIndex);  //TRUE
}

DTV_STATUS_T RHAL_AUDIO_TP_GetAudioPTS(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 *pPts)
{
    SINT64 pts;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(pPts == NULL)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] error dec index %d \n",  adecIndex);
        return NOT_OK;
    }

    if(Aud_Adec_info[adecIndex].bAdecStart != TRUE)
    {
        ERROR("[AUDH] ADEC%d is not running\n", adecIndex);
        return NOT_OK;
    }

    ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->GetCurrentPTS(Aud_dec[adecIndex], &pts);
    *pPts = (UINT32) pts;

    INFO("[AUDH] %s ADEC%d PTS: %x\n", __func__, adecIndex, *pPts);
    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_TP_GetAudioPTS64(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT64 *pPts)
{
    SINT64 pts;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(pPts == NULL)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] error dec index %d \n",  adecIndex);
        return NOT_OK;
    }

    if(Aud_Adec_info[adecIndex].bAdecStart != TRUE)
    {
        ERROR("[AUDH] ADEC%d is not running\n", adecIndex);
        return NOT_OK;
    }

    ((DEC*)Aud_dec[adecIndex]->pDerivedObj)->GetCurrentPTS(Aud_dec[adecIndex], &pts);
    *pPts = (UINT64) pts;

    INFO("[AUDH] %s ADEC%d PTS: %lld\n", __func__, adecIndex, *pPts);

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_TP_SetAudioDescriptionMain(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff)
{
    SINT32 id;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ADEC%d type %d \n", __func__, adecIndex, bOnOff);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] error dec index %d \n",  adecIndex);
        return NOT_OK;
    }

    id = adec2res(adecIndex);

    if(AUDIO_HAL_CHECK_PLAY_NOTAVAILABLE(ResourceStatus[id], 0))  // // decoder can only have one input
    {
        ERROR("[AUDH] dec description failed status (%d)!= stop\n",
                      ResourceStatus[id].connectStatus[0]);
        return NOT_OK;
    }

    RAL_AUDIO_SetAudioDescriptionMode(Aud_mainDecIndex, bOnOff);

    return OK;  //OK or NOT_OK
}

SLONG Convert2SpeedRate(HAL_AUDIO_TRICK_MODE_T eTrickMode)
{
    SLONG rate = 256;
    switch(eTrickMode)
    {
        default :
        case HAL_AUDIO_TRICK_NONE :
        case HAL_AUDIO_TRICK_PAUSE :
        case HAL_AUDIO_TRICK_NORMAL_PLAY :
            rate = 256;
            break;

        case HAL_AUDIO_TRICK_SLOW_MOTION_0P25X :
            rate = 256 >> 2;
            break;
        case HAL_AUDIO_TRICK_SLOW_MOTION_0P50X :
            rate = 256 >> 1;
            break;
        case HAL_AUDIO_TRICK_SLOW_MOTION_0P80X  :
            rate = 204; // 256*0.8
            break;
        case HAL_AUDIO_TRICK_FAST_FORWARD_1P20X :
            rate = 307; // 256*1.2
            break;
        case HAL_AUDIO_TRICK_FAST_FORWARD_1P50X  :
            rate = 384;
            break;
        case HAL_AUDIO_TRICK_FAST_FORWARD_2P00X  :
            rate = 256 << 1;
            break;


    }

    return rate;
}

DTV_STATUS_T RHAL_AUDIO_SetTrickMode(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_TRICK_MODE_T eTrickMode)
{
    FlowManager* pflow = NULL;
    HAL_AUDIO_RESOURCE_T curResourceId;
    HAL_AUDIO_RESOURCE_T decIptResId;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ADEC%d mode %d\n", __func__, adecIndex, eTrickMode);

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
    {
        ERROR("[AUDH] ADEC index error  %d \n", adecIndex);
        return NOT_OK;
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(eTrickMode > HAL_AUDIO_TRICK_FAST_FORWARD_2P00X)
    {
        ERROR("[AUDH] trick mode  error %d \n", eTrickMode);
        return NOT_OK;
    }

    curResourceId = adec2res(adecIndex);
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source

    if((eTrickMode == HAL_AUDIO_TRICK_PAUSE) && IsDTVSource(decIptResId))
    {
        //SetTrickPause(adecIndex, TRUE);
        Base* pDtv = Aud_DTV[decIptResId - HAL_AUDIO_RESOURCE_ATP0];
        DEMUX_CHANNEL_T demux_ch = (DEMUX_CHANNEL_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECChannel(pDtv);

        RSDEC_ENUM_TYPE_DEST_T sdec_dest = (RSDEC_ENUM_TYPE_DEST_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECDest(pDtv);
        KADP_SDEC_PrivateInfo(demux_ch, sdec_dest, DEMUX_PRIVATEINFO_AUDIO_PAUSE_, 0, 0);
    }
    else
    {
        //SetTrickPause(adecIndex, FALSE);
    }

    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        pflow = Aud_flow0;
    }
    else
    {
        pflow = Aud_flow1;
    }

    SetTrickState(adecIndex, eTrickMode);

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_TP_GetBufferStatus(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 *pMaxSize, UINT32 *pFreeSize)
{
    HAL_AUDIO_RESOURCE_T curResourceId, decIptResId, atpIptResId;
    ULONG base , rp , wp, size;
    DTV_STATUS_T retValue = NOT_OK;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    // check is dtv flow
    curResourceId = adec2res(adecIndex);
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source
    //INFO("[AUDH]  dec ipt %s \n", GetResourceString( decIptResId));
    if(IsDTVSource(decIptResId))
    { // DTV case

        //
        atpIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[decIptResId]);// dec input source

        //INFO("[AUDH]  tp ipt %s \n", GetResourceString( atpIptResId));

        if((atpIptResId == HAL_AUDIO_RESOURCE_SDEC0) || (atpIptResId == HAL_AUDIO_RESOURCE_SDEC1))
        {
           SINT32 atpindex = decIptResId - HAL_AUDIO_RESOURCE_ATP0;

            RINGBUFFER_HEADER* pRingBufferHeader;
            //DEC* pDEC = (DEC*)Aud_dec[atpindex]->pDerivedObj;

            DtvCom* pDtvCom = (DtvCom*)Aud_DTV[atpindex]->pDerivedObj;
            pRingBufferHeader = (RINGBUFFER_HEADER*)(pDtvCom->GetBSRingBufVirAddress(Aud_DTV[atpindex]));

            GetRingBufferInfo(pRingBufferHeader, &base, &rp, &wp, &size);

            if(wp  >= rp )
            {
                if(pFreeSize)
                    *pFreeSize = size - (wp - rp) -4;// for 4 byte aligm
            }
            else
            {
                if(pFreeSize)
                    *pFreeSize = (rp - wp) - 4 ; // for 4 byte aligm
            }

            if(pMaxSize)
                *pMaxSize = size;

            retValue =  OK;
        }
    }
    return retValue;
}

/* Volume, Mute & Delay */
DTV_STATUS_T RHAL_AUDIO_SetDecoderInputGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T volume)
{
    SINT32 ch_vol[AUD_MAX_CHANNEL];
    INFO("%s Enter.\n", __func__);

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;

      INFO("[AUDH] %s ADEC%d = main %d fine %d \n", __func__,
            adecIndex, volume.mainVol, volume.fineVol);

#ifdef AVOID_USELESS_RPC
    if (Volume_Compare(volume, GetDecInVolume(adecIndex)))
    {
        INFO("[AUDH] skip ADEC%d  volume %d ==  GetDecInVolume %d \n",
            adecIndex, volume.mainVol, (((HAL_AUDIO_VOLUME_T)GetDecInVolume(adecIndex)).mainVol));
            return OK;
    }
#endif
    SetDecInVolume(adecIndex, volume);

    {
        ADEC_Calculate_DSPGain(adecIndex, ch_vol);
        if(ADSP_DEC_SetVolume(adecIndex, ch_vol) != KADP_OK) return NOT_OK;
    }

    return OK; //OK or NOT_OK
}


DTV_STATUS_T RHAL_AUDIO_GetDecoderInputGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T *volume)
{
    INFO("%s Enter.\n", __func__);

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(volume == NULL) return NOT_OK;

    *volume = GetDecInVolume(adecIndex);

    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetMixerInputGain(HAL_AUDIO_MIXER_INDEX_T mixerIndex, HAL_AUDIO_VOLUME_T *volume)
{
    INFO("%s Enter.\n", __func__);
    if(!RangeCheck(mixerIndex, 0, AUD_AMIX_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(volume == NULL) return NOT_OK;

    *volume = g_mixer_gain[mixerIndex];

    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetDecoderInputMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN *bOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(bOnOff == NULL) return NOT_OK;

    *bOnOff = (BOOLEAN)GetDecInMute(adecIndex);

    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetMixerInputMute(HAL_AUDIO_MIXER_INDEX_T mixerIndex, BOOLEAN *bOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(!RangeCheck(mixerIndex, 0, AUD_AMIX_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(bOnOff == NULL) return NOT_OK;

    *bOnOff = (BOOLEAN)GetAmixerUserMute(mixerIndex);

    return OK; //OK or NOT_OK
}


DTV_STATUS_T RHAL_AUDIO_SetGlobalDecoderGain(HAL_AUDIO_VOLUME_T volume)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 ch_id;
    INFO("%s Enter.\n", __func__);
    /*SetAudioDecMultiChannelVolume(AUDIO_CH_ID_ALL, (AUDIO_DVOL_LEVEL)Volume_to_DSPGain(volume));*/
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    ch_id = KADP_AUDIO_CH_ID_ALL;
    audioConfig.msg_id = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_VOLUME;

    audioConfig.value[0] = ch_id;
    audioConfig.value[1] = Volume_to_DSPGain(volume);

    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] ao dec multi ch pb failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetDecoderDelayTime(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 delayTime)
{
    INFO("%s Enter.\n", __func__);
    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;

    if(ADSP_DEC_SetDelay(adecIndex, delayTime) != KADP_OK) return NOT_OK;

    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetMixerInputGain(HAL_AUDIO_MIXER_INDEX_T mixerIndex, HAL_AUDIO_VOLUME_T volume)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
	UINT32 i;
	SINT32 dsp_gain;

    INFO("%s Enter.\n", __func__);
    if(!RangeCheck(mixerIndex, 0, AUD_AMIX_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    INFO("[AUDH] %s AMIXER%d = main %d fine %d \n", __func__,
            mixerIndex, volume.mainVol, volume.fineVol);

    dsp_gain = Volume_to_DSPGain(Volume_Add(volume, g_mixer_out_gain));

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_FLASH_PIN;
    audioConfig.value[2] = (UINT32)mixerIndex;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < AUD_MAX_CHANNEL; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    g_mixer_gain[mixerIndex] = volume;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSPKOutVolume(HAL_AUDIO_VOLUME_T volume)//PB
{
    SINT32 dsp_gain;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    INFO("[AUDH] %s volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);

#ifdef AVOID_USELESS_RPC
    if (Volume_Compare(volume, g_AudioStatusInfo.curSPKOutVolume))
    {
        INFO("[AUDH] %s Skip volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);
        return OK;
    }
#endif
    dsp_gain = Volume_to_DSPGain(volume);
    if(ADSP_SNDOut_SetFineVolume(ENUM_DEVICE_SPEAKER, dsp_gain, volume.fineVol) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curSPKOutVolume.mainVol = volume.mainVol;
    g_AudioStatusInfo.curSPKOutVolume.fineVol = volume.fineVol;

    return OK;
}

DTV_STATUS_T SetSPDIFOutVolume(HAL_AUDIO_VOLUME_T volume)
{
    SINT32 dsp_gain;
    INFO("[AUDH] %s volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);

#ifdef AVOID_USELESS_RPC
    if (Volume_Compare(volume, g_AudioStatusInfo.curSPDIFOutVolume))
    {
        INFO("[AUDH] %s Skip volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);
        return OK;
    }
#endif
    dsp_gain = Volume_to_DSPGain(volume);
    if(ADSP_SNDOut_SetFineVolume(ENUM_DEVICE_SPDIF, dsp_gain, volume.fineVol) != KADP_OK) {
        ERROR("ADSP_SNDOut_SetFineVolume dsp_gain=%d,volume.fineVol=%d\n", dsp_gain, volume.fineVol);
        return NOT_OK;
    }

    g_AudioStatusInfo.curSPDIFOutVolume.mainVol = volume.mainVol;
    g_AudioStatusInfo.curSPDIFOutVolume.fineVol = volume.fineVol;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutVolume(HAL_AUDIO_VOLUME_T volume)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    return SetSPDIFOutVolume(volume);
}

int AudioSW_SPDIF_Volume(int dbGain_index)
{
    HAL_AUDIO_VOLUME_T volume;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    /*!
      UINT8	mainVol;	// 1 dB step, -127 ~ +30 dB.
      UINT8	fineVol;  	// 1/16 dB step, 0dB ~ 15/16dB
      */
    if (dbGain_index >= 0) {
        volume.mainVol = dbGain_index / 8 + 127;
        volume.fineVol = (dbGain_index % 8) * 2;
    } else if (dbGain_index % 8 == 0) {
        volume.mainVol = dbGain_index / 8 + 127;
        volume.fineVol = 0;
    } else {
        volume.mainVol = dbGain_index / 8 + 127 - 1;
        volume.fineVol = 16 + ((dbGain_index % 8) * 2);
    }
    SetSPDIFOutVolume(volume);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetHPOutVolume(HAL_AUDIO_VOLUME_T volume, BOOLEAN bForced)
{
    SINT32 dsp_gain;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    INFO("[AUDH] %s volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);

#ifdef AVOID_USELESS_RPC
    if (Volume_Compare(volume, g_AudioStatusInfo.curHPOutVolume))
    {
        INFO("[AUDH] %s Skip volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);
        return OK;
    }
#endif

    dsp_gain = Volume_to_DSPGain(volume);
    if(ADSP_SNDOut_SetFineVolume(ENUM_DEVICE_HEADPHONE, dsp_gain, volume.fineVol) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curHPOutVolume.mainVol = volume.mainVol;
    g_AudioStatusInfo.curHPOutVolume.fineVol = volume.fineVol;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetSCARTOutVolume(HAL_AUDIO_VOLUME_T volume, BOOLEAN bForced)
{
    SINT32 dsp_gain;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    INFO("[AUDH] %s volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);

#ifdef AVOID_USELESS_RPC
    if (Volume_Compare(volume, g_AudioStatusInfo.curSCARTOutVolume))
    {
        INFO("[AUDH] %s Skip volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);
        return OK;
    }
#endif

    dsp_gain = Volume_to_DSPGain(volume);
    if(ADSP_SNDOut_SetVolume(ENUM_DEVICE_SCART, dsp_gain) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curSCARTOutVolume.mainVol = volume.mainVol;
    g_AudioStatusInfo.curSCARTOutVolume.fineVol = volume.fineVol;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAudioDescriptionVolume(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_VOLUME_T volume)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    SINT32 i, ch_vol[AUD_MAX_CHANNEL];
    HAL_AUDIO_VOLUME_T volume_0dB = {.mainVol=0x7F, .fineVol=0x0};
    int balance_adjustment;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s volume = main %d fine %d \n", __func__, volume.mainVol, volume.fineVol);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;

    balance_adjustment = (int)(volume.mainVol - volume_0dB.mainVol);

    if(balance_adjustment < -32) {
        balance_adjustment = -32;
    } else if(balance_adjustment > 32) {
        balance_adjustment = 32;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_USER_BALANCE_ADJUSTMENT;
    audioConfig.value[0] = (u_int)balance_adjustment;

    /* UI send main volume 55, 97 ~ 159, remapping */
    if (volume.mainVol > 127) {
        volume.mainVol = 127 + (volume.mainVol -127)/2;
    }

    volume.mainVol = volume.mainVol - 6;

    if (volume.mainVol < 55) {
        volume.mainVol = 55;
    }

    currMixADVol = volume_0dB;
    currDecADVol = volume;
    INFO("[AUDH] %s  currMixADVol = 0x%x currDecADVol = 0x%x \n", __func__, currMixADVol.mainVol, currDecADVol.mainVol);

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    if(Aud_descriptionMode == FALSE)
    {
        ERROR("[AUDH][%s] Audio Descriptioin mode is OFF, return NOT_OK\n", __func__);
        return NOT_OK;
    }
    ADSP_PPMix_ConfigMixer(Aud_descriptionMode, Volume_to_MixerGain(currMixADVol));

#ifdef SUPPORT_AD_POSITIVE_GAIN
    for(i = 0; i < AUD_ADEC_MAX; i++)
    {
        ADEC_Calculate_DSPGain(i, ch_vol);
        ADSP_DEC_SetVolume(i, ch_vol);
    }
#endif

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetDecoderInputMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s ADEC%d onoff=%d\n", __func__, adecIndex, bOnOff);

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;

    if(GetTrickState(adecIndex) == HAL_AUDIO_TRICK_PAUSE)
    {
        INFO("Skip mute at pause state \n");// pause play will have cutting sound
        return OK;
    }

    if(ADSP_DEC_SetMute(adecIndex, ADEC_CheckConnect(adecIndex), bOnOff) != KADP_OK)
        return NOT_OK;

    SetDecInMute(adecIndex, (UINT32)bOnOff);

    if(!bOnOff)
    {
        UINT32 value = 0;
        KADP_AUDIO_GetAMixerRunningStatus(&value);
        if((value & 0xFF) != 0)
            g_isAMIXERRunning = TRUE;
        else
            g_isAMIXERRunning = FALSE;

        if(g_isAMIXERRunning)
        {
            INFO("[AUDH] %s AMIXER is running.\n", __func__);
        }
        else
        {
            INFO("[AUDH] %s AMIXER is not running.\n", __func__);
        }
    }

    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetDecoderInputESMute(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetMixerInputMute(HAL_AUDIO_MIXER_INDEX_T mixerIndex, BOOLEAN bOnOff)
{
    HAL_AUDIO_RESOURCE_T resID;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s AMIXER%d onoff=%d \n", __func__, mixerIndex, bOnOff);
    if(!RangeCheck(mixerIndex, 0, AUD_AMIX_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;

/*
    UINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_FLASH_PIN;
    audioConfig.value[2] = (UINT32)mixerIndex;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
    {
        audioConfig.value[4+i] = bOnOff;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return KADP_NOT_OK;

    g_mixer_user_mute[mixerIndex]=  bOnOff;
*/

    SetAmixerUserMute(mixerIndex, (UINT32)bOnOff);

    resID = amixer2res(mixerIndex);
    ADSP_AMIXER_SetMute(mixerIndex, AMIXER_CheckConnect(resID), GetAmixerUserMute(mixerIndex));


    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSPKOutMute(BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s   onoff=%d curr %d \n", __func__, bOnOff, g_AudioStatusInfo.curSPKMuteStatus);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    //if(g_AudioStatusInfo.curSPKMuteStatus == bOnOff) return OK;

    if(SNDOut_SetMute(ENUM_DEVICE_SPEAKER, ADEC_SPK_CheckConnect(), bOnOff) != KADP_OK) {
        ERROR("%s SNDOut_SetMute fail\n", __func__);
        return NOT_OK;
    }

    g_AudioStatusInfo.curSPKMuteStatus = bOnOff;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutMute(BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s   onoff=%d \n", __func__, bOnOff);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    //if(g_AudioStatusInfo.curSPDIFMuteStatus == bOnOff) return OK;

    if(SNDOut_SetMute(ENUM_DEVICE_SPDIF, ADEC_SPDIF_CheckConnect(), bOnOff) != KADP_OK)
        return NOT_OK;
    if(SNDOut_SetMute(ENUM_DEVICE_SPDIF_ES, ADEC_SPDIF_CheckConnect(), bOnOff) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curSPDIFMuteStatus = bOnOff;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetHPOutMute(BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s   onoff=%d \n", __func__, bOnOff);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    //if(g_AudioStatusInfo.curHPMuteStatus == bOnOff) return OK;

    if(SNDOut_SetMute(ENUM_DEVICE_HEADPHONE, ADEC_HP_CheckConnect(), bOnOff) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curHPMuteStatus = bOnOff;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSCARTOutMute(BOOLEAN bOnOff)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 i;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s   onoff=%d \n", __func__, bOnOff);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    //if(g_AudioStatusInfo.curSCARTMuteStatus == bOnOff) return OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_SCART;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xFF;
    for(i = 0; i < 8; i++)
    {
        //audioConfig.value[4+i] = (bOnOff==1)? TRUE:FALSE; //bOnOff 1: On, 2: Off
        audioConfig.value[4+i] = bOnOff;                    //bOnOff 1: On, 0: Off
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    g_AudioStatusInfo.curSCARTMuteStatus = bOnOff;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetSPKOutMuteStatus(BOOLEAN *pOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(pOnOff == NULL) return NOT_OK;

    *pOnOff = g_AudioStatusInfo.curSPKMuteStatus;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetSPDIFOutMuteStatus(BOOLEAN *pOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(pOnOff == NULL) return NOT_OK;

    *pOnOff = g_AudioStatusInfo.curSPDIFMuteStatus;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetHPOutMuteStatus(BOOLEAN *pOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(pOnOff == NULL) return NOT_OK;

    *pOnOff = g_AudioStatusInfo.curHPMuteStatus;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetSCARTOutMuteStatus(BOOLEAN *pOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(pOnOff == NULL) return NOT_OK;

    *pOnOff = g_AudioStatusInfo.curSCARTMuteStatus;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSPKOutDelayTime(UINT32 delayTime, BOOLEAN bForced)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(ADSP_SNDOut_SetDelay(ENUM_DEVICE_SPEAKER, delayTime) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curSPKOutDelay = delayTime;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSPDIFOutDelayTime(UINT32 delayTime, BOOLEAN bForced)
{
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL ret;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    /* Set SPDIF (pcm) delay */
    if(ADSP_SNDOut_SetDelay(ENUM_DEVICE_SPDIF, delayTime) != KADP_OK)
        return NOT_OK;

    /* Set SPDIF_ES delay */
    /*SetAudioDelayTime(AUDIO_DELAY_RAW, TRUE, delayTime);*/

    memset(&parameter, 0, sizeof(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS));
    memset(&ret, 0, sizeof(AUDIO_RPC_PRIVATEINFO_RETURNVAL));

    if(delayTime < 0)
    {
        delayTime = 0;
    }

    /* RAW delay */
    parameter.privateInfo[0] = KADP_AUDIO_DELAY_RAW;
    parameter.privateInfo[1] = TRUE;
    parameter.privateInfo[2] = delayTime; // no limitaion

    if (KADP_AUDIO_PrivateInfo(&parameter, &ret) != KADP_OK) {
        ERROR("[%s,%d] set es delay failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    g_AudioStatusInfo.curSPDIFOutDelay = delayTime;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetHPOutDelayTime(UINT32 delayTime, BOOLEAN bForced)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(ADSP_SNDOut_SetDelay(ENUM_DEVICE_HEADPHONE, delayTime) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curHPOutDelay = delayTime;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SetSCARTOutDelayTime(UINT32 delayTime, BOOLEAN bForced)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(ADSP_SNDOut_SetDelay(ENUM_DEVICE_SCART, delayTime) != KADP_OK)
        return NOT_OK;

    g_AudioStatusInfo.curSCARTOutDelay = delayTime;
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RAL_AUDIO_GetStatusInfo(HAL_AUDIO_COMMON_INFO_T *pAudioStatusInfo)
{
    KADP_AO_SPDIF_CHANNEL_STATUS_BASIC sc;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(pAudioStatusInfo == NULL) return NOT_OK;

    if(g_AudioStatusInfo.curAudioSpdifMode == HAL_AUDIO_SPDIF_PCM)
        g_AudioStatusInfo.bCurAudioSpdifOutPCM  = TRUE;
    else
        g_AudioStatusInfo.bCurAudioSpdifOutPCM  = FALSE;

    /*GetAudioSpdifChannelStatus(&sc, AUDIO_OUT);*/
    memset(&sc, 0, sizeof(KADP_AO_SPDIF_CHANNEL_STATUS_BASIC));
    if (KADP_AUDIO_GetAudioSpdifChannelStatus(&sc, AUDIO_OUT))
    {
        ERROR("[%s,%d] get spdif channel status failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    if(sc.data_type == 0) // pcm
        g_AudioStatusInfo.bAudioSpdifOutPCM  = TRUE;
    else
        g_AudioStatusInfo.bCurAudioSpdifOutPCM  = FALSE;

    memcpy(pAudioStatusInfo, &g_AudioStatusInfo, sizeof(HAL_AUDIO_COMMON_INFO_T));
    return OK; //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_GetStatusInfo(HAL_AUDIO_COMMON_INFO_T *pAudioStatusInfo)
{
    INFO("%s Enter.\n", __func__);
    return RAL_AUDIO_GetStatusInfo(pAudioStatusInfo); //OK or NOT_OK
}

static inline HAL_AUDIO_DELAY_INPUT_SRC res2DlyIpt(HAL_AUDIO_RESOURCE_T res_id)
{
   HAL_AUDIO_DELAY_INPUT_SRC delay_input_src = DELAY_INPUT_SRC_UNKNOWN;

   switch(res_id)
   {
       case HAL_AUDIO_RESOURCE_ATP0:
       case HAL_AUDIO_RESOURCE_ATP1:
           delay_input_src = DELAY_INPUT_SRC_TP;
           break;
       case HAL_AUDIO_RESOURCE_ADC:
           delay_input_src = DELAY_INPUT_SRC_ADC;
           break;
       case HAL_AUDIO_RESOURCE_AAD:
           delay_input_src = DELAY_INPUT_SRC_AAD;
           break;
       case HAL_AUDIO_RESOURCE_HDMI:
       case HAL_AUDIO_RESOURCE_HDMI0:
       case HAL_AUDIO_RESOURCE_HDMI1:
       case HAL_AUDIO_RESOURCE_HDMI2:
       case HAL_AUDIO_RESOURCE_HDMI3:
           delay_input_src = DELAY_INPUT_SRC_HDMI;
           break;
       default:
           break;
   }

   return delay_input_src;
}

static inline AUDIO_SOURCE ipt2audiosrc(HAL_AUDIO_DELAY_INPUT_SRC input_src)
{
    AUDIO_SOURCE ipt_src = SOURCE_UNKNOWN;

    switch(input_src) {
        case DELAY_INPUT_SRC_HDMI:
            ipt_src = SOURCE_HDMI;
            break;
        case DELAY_INPUT_SRC_TP:
            ipt_src = SOURCE_DTV;
            break;
        case DELAY_INPUT_SRC_MEDIA:
            ipt_src = SOURCE_GST_HALF_TUNNEL;
            break;
        case DELAY_INPUT_SRC_AAD:
            ipt_src = SOURCE_ATV;
            break;
        case DELAY_INPUT_SRC_ADC:
            ipt_src = SOURCE_ADC;
            break;
        default:
            ipt_src = SOURCE_UNKNOWN;
            ERROR("[AUDH][%s,%d] ipt_src %d not support !!!\n", __func__, __LINE__, input_src);
            break;
    }
    return ipt_src;
}

static inline SNDOUT_DEVICE soundOutType2sndoutdevice(HAL_AUDIO_SNDOUT_T soundOutType)
{
    SNDOUT_DEVICE sndOut;

    switch(soundOutType) {
        case HAL_AUDIO_SPK:
            sndOut = SNDOUT_SPK;
            break;
        case HAL_AUDIO_SPDIF:
            sndOut = SNDOUT_ARC;
            break;
        case HAL_AUDIO_BT:
            sndOut = SNDOUT_BLUETOOTH;
            break;
        case HAL_AUDIO_SB_SPDIF:
        case HAL_AUDIO_SB_PCM:
        case HAL_AUDIO_SB_CANVAS:
        case HAL_AUDIO_HP:
        case HAL_AUDIO_SCART:
        case HAL_AUDIO_SPDIF_ES:
            sndOut = SNDOUT_SPK;
            break;
        default:
            sndOut = SNDOUT_NO_OUTPUT;
            ERROR("[AUDH][%s,%d] soundOutType %d not support !!!\n", __func__, __LINE__, soundOutType);
            break;
    }
    return sndOut;
}

DTV_STATUS_T RHAL_AUDIO_SNDOUT_GetLatency(HAL_AUDIO_SNDOUT_T soundOutType, int *latency_ms)
{
    HAL_AUDIO_ES_INFO_T AudioESInfo;
    AUDIO_LATENCY_INFO latency_info;
    UINT16 sampleRate = 0;
    HAL_AUDIO_DELAY_INPUT_SRC input_src;
    HAL_AUDIO_RESOURCE_T decIptResId;
    HAL_AUDIO_ADEC_INDEX_T input_port = (HAL_AUDIO_ADEC_INDEX_T)Aud_mainDecIndex;
    HAL_AUDIO_RESOURCE_T curResourceId = adec2res(input_port);
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL ret;

    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(latency_ms == NULL) return NOT_OK;

    memset(&parameter, 0, sizeof(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS));
    memset(&ret, 0, sizeof(AUDIO_RPC_PRIVATEINFO_RETURNVAL));

    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curResourceId]);// dec input source
    input_src = res2DlyIpt(decIptResId);
    latency_info.iptSrc = ipt2audiosrc(input_src);
    latency_info.sndOut = soundOutType2sndoutdevice(soundOutType);

    if (latency_info.sndOut == SNDOUT_NO_OUTPUT) {
        ERROR("[AUDH][%s,%d] soundOutType %d not support\n", __func__, __LINE__, soundOutType);
        return NOT_OK;
    }

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0) /*&& adecIndex == Aud_mainDecIndex*/) {
        DTV_STATUS_T result = AUDIO_Get_MediaSourceLatency(&latency_info);
        if (result != OK) {
            ERROR("[AUDH][%s,%d] Get Latency failed\n", __func__, __LINE__);
            return NOT_OK;
        }
        *latency_ms = latency_info.latency;
        INFO("[AUDH] latency %d,in:%d,out:%d is not in run state\n",*latency_ms, latency_info.iptSrc, latency_info.sndOut);

        return OK;
    } else {
        parameter.instanceID = Aud_dec[input_port]->GetAgentID(Aud_dec[input_port]);
        parameter.type = ENUM_PRIVATEINFO_AUDIO_GET_SNDOUT_LATENCY;
        parameter.privateInfo[0] = latency_info.iptSrc;
        parameter.privateInfo[1] = latency_info.sndOut;

        if (KADP_AUDIO_PrivateInfo(&parameter, &ret) != KADP_OK) {
            ERROR("[AUDH][%s,%d] Get Latency failed\n", __func__, __LINE__);
            return NOT_OK;
        }
        *latency_ms = ret.privateInfo[0];
        INFO("[AUDH] latency %d,in:%d,out:%d\n",*latency_ms, latency_info.iptSrc, latency_info.sndOut);
    }

    memset(&AudioESInfo, 0, sizeof(HAL_AUDIO_ES_INFO_T));
    RAL_AUDIO_GetESInfo(Aud_mainDecIndex, &AudioESInfo, &sampleRate);
    if (AudioESInfo.adecFormat == HAL_AUDIO_SRC_TYPE_UNKNOWN || AudioESInfo.audioMode == HAL_AUDIO_MODE_UNKNOWN || sampleRate == 0) {
        INFO("[AUDH] dec not ready (ch=%d, fs=%d), use predict value",AudioESInfo.audioMode, sampleRate);
    }

    return OK;
}

/* SPDIF(Sound Bar) */
DTV_STATUS_T RHAL_AUDIO_SPDIF_SetOutputType(HAL_AUDIO_SPDIF_MODE_T eSPDIFMode, BOOLEAN bForced)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  %d\n", __func__, eSPDIFMode);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    switch(eSPDIFMode)
    {
    case HAL_AUDIO_SPDIF_AUTO:
    case HAL_AUDIO_SPDIF_AUTO_NODTS:
        _AudioSPDIFMode = NON_PCM_OUT_EN_AUTO; break;
    case HAL_AUDIO_SPDIF_AUTO_AAC:
    case HAL_AUDIO_SPDIF_AUTO_AAC_NODTS:
        _AudioSPDIFMode = NON_PCM_OUT_EN_AUTO_AAC; break;
    case HAL_AUDIO_SPDIF_FORCE_AC3:
    case HAL_AUDIO_SPDIF_FORCE_AC3_DTS:
        _AudioSPDIFMode = NON_PCM_OUT_EN_AUTO_FORCED_AC3; break;
    case HAL_AUDIO_SPDIF_BYPASS:
    case HAL_AUDIO_SPDIF_BYPASS_NODTS:
        _AudioSPDIFMode = NON_PCM_OUT_EN_AUTO_BYPASS; break;
    case HAL_AUDIO_SPDIF_BYPASS_AAC:
    case HAL_AUDIO_SPDIF_BYPASS_AAC_NODTS:
        _AudioSPDIFMode = NON_PCM_OUT_EN_AUTO_BYPASS_AAC; break;
    case HAL_AUDIO_SPDIF_PCM:
        _AudioSPDIFMode = ENABLE_DOWNMIX;
        break;
    default:
        ERROR("[AUDH] error type %d \n", eSPDIFMode);
        return NOT_OK;
    }

    //control DTS bypass
    switch(eSPDIFMode)
    {
    case HAL_AUDIO_SPDIF_PCM:
    case HAL_AUDIO_SPDIF_FORCE_AC3:
    case HAL_AUDIO_SPDIF_BYPASS_NODTS:
    case HAL_AUDIO_SPDIF_AUTO_NODTS:
    case HAL_AUDIO_SPDIF_BYPASS_AAC_NODTS:
    case HAL_AUDIO_SPDIF_AUTO_AAC_NODTS:
        //muted DTS so amp will not show DTS logo.
        ADSP_SetDTSSupport(FALSE);
        break;
    default:
        //turn on DTS bypass so amp show DTS logo.
        ADSP_SetDTSSupport(TRUE);
        break;
    }

    Update_RawMode_by_connection();
    g_AudioStatusInfo.curAudioSpdifMode = eSPDIFMode;

    return OK;  //OK or NOT_OK
}

// for TB24 ARC DD+ output 2016/06/4 by
DTV_STATUS_T RHAL_AUDIO_ARC_SetOutputType(HAL_AUDIO_ARC_MODE_T eARCMode, BOOLEAN bForced)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s %d\n", __func__, eARCMode);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    switch(eARCMode)
    {
    case HAL_AUDIO_ARC_AUTO:
    case HAL_AUDIO_ARC_AUTO_NODTS:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO; break;
    case HAL_AUDIO_ARC_AUTO_AAC:
    case HAL_AUDIO_ARC_AUTO_AAC_NODTS:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_AAC; break;
    case HAL_AUDIO_ARC_AUTO_EAC3:
    case HAL_AUDIO_ARC_AUTO_EAC3_NODTS:
    case HAL_AUDIO_ARC_AUTO_EAC3_DTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_DDP; break;
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC:
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC_NODTS:
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC_DTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_DDP_AAC; break;
    case HAL_AUDIO_ARC_AUTO_MAT:
    case HAL_AUDIO_ARC_AUTO_MAT_NODTS:
    case HAL_AUDIO_ARC_AUTO_MAT_NODTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_MAT; break;
    case HAL_AUDIO_ARC_AUTO_MAT_AAC:
    case HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTS:
    case HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_MAT_AAC; break;
    case HAL_AUDIO_ARC_FORCE_AC3:
    case HAL_AUDIO_ARC_FORCE_AC3_DTS:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_FORCED_AC3; break;
    case HAL_AUDIO_ARC_FORCE_EAC3:
    case HAL_AUDIO_ARC_FORCE_EAC3_DTS:
    case HAL_AUDIO_ARC_FORCE_EAC3_DTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_FORCED_DDP; break;
    case HAL_AUDIO_ARC_BYPASS_NODTS:
    case HAL_AUDIO_ARC_BYPASS:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS; break;
    case HAL_AUDIO_ARC_BYPASS_EAC3_NODTS:
    case HAL_AUDIO_ARC_BYPASS_EAC3:
    case HAL_AUDIO_ARC_BYPASS_EAC3_DTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS_DDP; break;
    case HAL_AUDIO_ARC_FORCE_MAT:
    case HAL_AUDIO_ARC_FORCE_MAT_DTS:
    case HAL_AUDIO_ARC_FORCE_MAT_NODTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_FORCED_MAT; break;
    case HAL_AUDIO_ARC_BYPASS_MAT_NODTS:
    case HAL_AUDIO_ARC_BYPASS_MAT:
    case HAL_AUDIO_ARC_BYPASS_MAT_NODTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS_MAT; break;
    case HAL_AUDIO_ARC_BYPASS_AAC_NODTS:
    case HAL_AUDIO_ARC_BYPASS_AAC:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS_AAC; break;
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC_NODTS:
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC:
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC_DTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS_DDP_AAC; break;
    case HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTS:
    case HAL_AUDIO_ARC_BYPASS_MAT_AAC:
    case HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTSHD:
        _AudioARCMode = NON_PCM_OUT_EN_AUTO_BYPASS_MAT_AAC; break;
    case HAL_AUDIO_ARC_PCM:
        _AudioARCMode = ENABLE_DOWNMIX; break;
    default:
        ERROR("[AUDH] error type %d \n", eARCMode);
        return NOT_OK;
    }
    //control DTS bypass
    switch(eARCMode)
    {
    case HAL_AUDIO_ARC_FORCE_AC3:
    case HAL_AUDIO_ARC_FORCE_EAC3:
    case HAL_AUDIO_ARC_FORCE_MAT:
    case HAL_AUDIO_ARC_BYPASS_NODTS:
    case HAL_AUDIO_ARC_BYPASS_EAC3_NODTS:
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC_NODTS:
    case HAL_AUDIO_ARC_BYPASS_MAT_NODTS:
    case HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTS:
    case HAL_AUDIO_ARC_BYPASS_AAC_NODTS:
    case HAL_AUDIO_ARC_AUTO_NODTS:
    case HAL_AUDIO_ARC_AUTO_EAC3_NODTS:
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC_NODTS:
    case HAL_AUDIO_ARC_AUTO_MAT_NODTS:
    case HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTS:
    case HAL_AUDIO_ARC_AUTO_AAC_NODTS:
        //muted DTS so amp will not show DTS logo.
        ADSP_SetDTSSupport(FALSE);
        ADSP_SetDTSHDSupport(FALSE);
        break;
    case HAL_AUDIO_ARC_AUTO_EAC3_DTSHD:
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC_DTSHD:
    case HAL_AUDIO_ARC_FORCE_EAC3_DTSHD:
    case HAL_AUDIO_ARC_BYPASS_EAC3_DTSHD:
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC_DTSHD:
        ADSP_SetDTSSupport(TRUE);
        ADSP_SetDTSHDSupport(TRUE);
        break;
    case HAL_AUDIO_ARC_FORCE_MAT_NODTSHD:
    case HAL_AUDIO_ARC_BYPASS_MAT_NODTSHD:
    case HAL_AUDIO_ARC_BYPASS_MAT_AAC_NODTSHD:
    case HAL_AUDIO_ARC_AUTO_MAT_NODTSHD:
    case HAL_AUDIO_ARC_AUTO_MAT_AAC_NODTSHD:
    case HAL_AUDIO_ARC_AUTO_EAC3:
    case HAL_AUDIO_ARC_AUTO_EAC3_AAC:
    case HAL_AUDIO_ARC_BYPASS_EAC3:
    case HAL_AUDIO_ARC_BYPASS_EAC3_AAC:
        //muted DTSHD so amp will not show DTS HD logo.
        ADSP_SetDTSSupport(TRUE);
        ADSP_SetDTSHDSupport(FALSE);
        break;
    default:
        //turn on DTS bypass so amp show DTS logo.
        ADSP_SetDTSSupport(TRUE);
        ADSP_SetDTSHDSupport(TRUE);
        break;
    }

    Update_RawMode_by_connection();
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_ATMOS_EncodeOnOff(BOOLEAN bOnOff)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_EAC3_ATMOS_ENCODE_ONOFF;
    audioConfig.value[0] = (u_int)bOnOff;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SPDIF_SetCopyInfo(HAL_AUDIO_SPDIF_COPYRIGHT_T copyInfo)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  %d   \n", __func__, copyInfo);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    if(copyInfo > HAL_AUDIO_SPDIF_COPY_NEVER)
    {
        ERROR("[AUDH] error type %d \n", HAL_AUDIO_SPDIF_COPY_NEVER);
        return NOT_OK;
    }

    g_AudioStatusInfo.curSpdifCopyInfo = copyInfo;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SPDIF_CS_INFO;
    audioConfig.value[0] = TRUE;
    audioConfig.value[1] = HAL_SPDIF_CONSUMER_USE;
    audioConfig.value[5] = HAL_SPDIF_WORD_LENGTH_16; // 16 bit
    audioConfig.value[3] = g_AudioStatusInfo.curSpdifCategoryCode;
    if(g_AudioStatusInfo.curSpdifCopyInfo == HAL_AUDIO_SPDIF_COPY_FREE)
    {
        audioConfig.value[2] = HAL_SPDIF_COPYRIGHT_FREE;
        audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_0;
    }
    else
    {
        audioConfig.value[2] = HAL_SPDIF_COPYRIGHT_NEVER;
        if(g_AudioStatusInfo.curSpdifCopyInfo == HAL_AUDIO_SPDIF_COPY_ONCE)
        {
            audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_0;
        }
        else
        {
            //RTWTV-247 nerver is the same with  no more
            audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_1;  //HAL_AUDIO_SPDIF_COPY_NEVER
        }
    }

    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] set channel status failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_SPDIF_SetCategoryCode(UINT8 categoryCode)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  %d   \n", __func__, categoryCode);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;
    categoryCode = (categoryCode & 0x7F);// category occupy 7 bits7

    g_AudioStatusInfo.curSpdifCategoryCode = categoryCode;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_SPDIF_CS_INFO;
    audioConfig.value[0] = TRUE;
    audioConfig.value[1] = HAL_SPDIF_CONSUMER_USE; // CONSUMER
    audioConfig.value[5] = HAL_SPDIF_WORD_LENGTH_16; // 16 bit
    audioConfig.value[3] = g_AudioStatusInfo.curSpdifCategoryCode;
    if(g_AudioStatusInfo.curSpdifCopyInfo == HAL_AUDIO_SPDIF_COPY_FREE)
    {
        audioConfig.value[2] = HAL_SPDIF_COPYRIGHT_FREE;
        audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_1;
    }
    else
    {
        audioConfig.value[2] = HAL_SPDIF_COPYRIGHT_NEVER;
        if(g_AudioStatusInfo.curSpdifCopyInfo == HAL_AUDIO_SPDIF_COPY_ONCE)
        {
            audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_0;
        }
        else
        {
            audioConfig.value[4] = HAL_SPDIF_CATEGORY_L_BIT_IS_1;  //HAL_AUDIO_SPDIF_COPY_NEVER
        }
    }

    if (SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        ERROR("[%s,%d] set channel status failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    return OK;  //OK or NOT_OK
}

/* AAD */
DTV_STATUS_T RHAL_AUDIO_SIF_SetInputSource(HAL_AUDIO_SIF_INPUT_T sifSource)
{
    //HAL enum format to RTK format
    SIF_INPUT_SOURCE  rtk_sif_input_source;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    switch(sifSource)
    {
        case HAL_AUDIO_SIF_INPUT_EXTERNAL:
            rtk_sif_input_source = SIF_FROM_SIF_ADC;
            break;
        case HAL_AUDIO_SIF_INPUT_INTERNAL:
            rtk_sif_input_source = SIF_FROM_IFDEMOD;
            break;
        default:
            ERROR("[%s] sif input %d\n",__func__,sifSource);
            return API_INVALID_PARAMS;
    }
    INFO("[%s] sif input %d %d\n",__func__,sifSource, rtk_sif_input_source);

    KADP_Audio_HwpSetSIFDataSource(rtk_sif_input_source);
    g_AudioSIFInfo.sifSource  = sifSource;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_DetectSoundSystemStd(HAL_AUDIO_SIF_SOUNDSYSTEM_T setSoundSystem, BOOLEAN bManualMode,
                                                            HAL_AUDIO_SIF_SOUNDSYSTEM_T *pDetectSoundSystem, HAL_AUDIO_SIF_STANDARD_T *pDetectSoundStd,
                                                            UINT32 *pSignalQuality)
{
	ATV_SOUND_STD_MAIN_SYSTEM atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
	ATV_SOUND_STD_MAIN_SYSTEM HWDetectSoundSystem = ATV_SOUND_UNKNOWN_SYSTEM;
	ATV_SOUND_STD HWDetectSoundStd = ATV_SOUND_STD_UNKNOWN;
	UINT32 pToneSNR = 0;
	SIF_INPUT_SOURCE  rtk_sif_input_source;

	INFO("%s Enter.\n", __func__);

	if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

	//set band internal/external
	switch(g_AudioSIFInfo.sifSource)
	{
		case HAL_AUDIO_SIF_INPUT_EXTERNAL:
			rtk_sif_input_source = SIF_FROM_SIF_ADC;
			break;
		case HAL_AUDIO_SIF_INPUT_INTERNAL:
			rtk_sif_input_source = SIF_FROM_IFDEMOD;
			break;
		default:
			ERROR("[%s] sif input %d\n",__func__, g_AudioSIFInfo.sifSource);
			return API_INVALID_PARAMS;
	}

	KADP_Audio_AtvSetScanStdFlag(true);
	//detect sound system
	switch(setSoundSystem)
	{
		case  HAL_AUDIO_SIF_SYSTEM_BG:
			atv_sound_std_main_system = ATV_SOUND_BG_SYSTEM;
			break;
		case  HAL_AUDIO_SIF_SYSTEM_I:
			atv_sound_std_main_system = ATV_SOUND_I_SYSTEM;
			break;
		case  HAL_AUDIO_SIF_SYSTEM_DK:
			KADP_Audio_AtvSetMtsPriority(ATV_MTS_PRIO_DK);
			atv_sound_std_main_system = ATV_SOUND_DK_SYSTEM;
			break;
		case  HAL_AUDIO_SIF_SYSTEM_L:
			KADP_Audio_AtvSetMtsPriority(ATV_MTS_PRIO_L);
			atv_sound_std_main_system = ATV_SOUND_L_SYSTEM;
			break;
		case  HAL_AUDIO_SIF_SYSTEM_MN:
			KADP_Audio_AtvSetMtsPriority(ATV_MTS_PRIO_BTSC);
			atv_sound_std_main_system = ATV_SOUND_MN_SYSTEM;
			break;
		case  HAL_AUDIO_SIF_SYSTEM_UNKNOWN:
			atv_sound_std_main_system = ATV_SOUND_AUTO_SYSTEM;
			break;
		default:
			ERROR("[%s] not in case1... %d\n",__func__,setSoundSystem);
			return API_INVALID_PARAMS;
	}

	KADP_Audio_HwpSetAtvAudioBand(rtk_sif_input_source, atv_sound_std_main_system);
	KADP_Audio_HwpSIFGetMainToneSNR(atv_sound_std_main_system, &HWDetectSoundSystem, &HWDetectSoundStd, &pToneSNR);
	switch (HWDetectSoundStd)
	{
		case ATV_SOUND_STD_MN_MONO:
			*pDetectSoundStd = HAL_AUDIO_SIF_MN_FM;
			break;
		case ATV_SOUND_STD_BTSC:
			*pDetectSoundStd = HAL_AUDIO_SIF_MN_BTSC;
			break;
		case ATV_SOUND_STD_A2_M:
			*pDetectSoundStd = HAL_AUDIO_SIF_MN_A2;
			break;
		case ATV_SOUND_STD_EIAJ:
			*pDetectSoundStd = HAL_AUDIO_SIF_MN_EIAJ;
			break;
		case ATV_SOUND_STD_BG_MONO:
			*pDetectSoundStd = HAL_AUDIO_SIF_BG_FM;
			break;
		case ATV_SOUND_STD_A2_BG:
			*pDetectSoundStd = HAL_AUDIO_SIF_BG_A2;
			break;
		case ATV_SOUND_STD_NICAM_BG:
			*pDetectSoundStd = HAL_AUDIO_SIF_BG_NICAM;
			break;
		case ATV_SOUND_STD_DK_MONO:
			*pDetectSoundStd = HAL_AUDIO_SIF_DK_FM;
			break;
		case ATV_SOUND_STD_A2_DK1:
			*pDetectSoundStd = HAL_AUDIO_SIF_DK1_A2;
			break;
		case ATV_SOUND_STD_A2_DK2:
			*pDetectSoundStd = HAL_AUDIO_SIF_DK2_A2;
			break;
		case ATV_SOUND_STD_A2_DK3:
			*pDetectSoundStd = HAL_AUDIO_SIF_DK3_A2;
			break;
		case ATV_SOUND_STD_NICAM_DK:
			*pDetectSoundStd = HAL_AUDIO_SIF_DK_NICAM;
			break;
		case ATV_SOUND_STD_AM_MONO:
			*pDetectSoundStd = HAL_AUDIO_SIF_L_AM;
			break;
		case ATV_SOUND_STD_NICAM_L:
			*pDetectSoundStd = HAL_AUDIO_SIF_L_NICAM;
			break;
		case ATV_SOUND_STD_FM_MONO_NO_I:
			*pDetectSoundStd = HAL_AUDIO_SIF_I_FM;
			break;
		case ATV_SOUND_STD_NICAM_I:
			*pDetectSoundStd = HAL_AUDIO_SIF_I_NICAM;
			break;
		case ATV_SOUND_STD_UNKNOWN:
		default:
			ERROR("[%s][%d] not in case... %d\n",__func__, __LINE__, HWDetectSoundStd);
			*pDetectSoundStd = HAL_AUDIO_SIF_UNKNOWN;
			break;
	}

	if(bManualMode == FALSE)
	{
		switch(HWDetectSoundSystem) //auto mode reuten HWDetectSoundSystem
		{
			case  ATV_SOUND_BG_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_BG;
				break;
			case  ATV_SOUND_I_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_I;
				break;
			case  ATV_SOUND_DK_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_DK;
				break;
			case  ATV_SOUND_L_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_L;
				break;
			case  ATV_SOUND_MN_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_MN;
				break;
			case  ATV_SOUND_UNKNOWN_SYSTEM:
				*pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_UNKNOWN;
				break;
			default:
				ERROR("[%s] not in case... %d\n",__func__,HWDetectSoundSystem);
				return API_INVALID_PARAMS;
		}
	}
	else
	{
		*pDetectSoundSystem = setSoundSystem;//  manual mode setSoundSystem and *pDetectSoundSystem must be the same
	}

	*pSignalQuality = (UINT32)pToneSNR;
	return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_NoSignalMute(BOOLEAN Enable)
{
    int ret = 0;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if (Enable) ret = KADP_Audio_AtvConfigNoSignalNeed2Mute(true);
    else ret = KADP_Audio_AtvConfigNoSignalNeed2Mute(false);

    return (DTV_STATUS_T)ret;
}

DTV_STATUS_T RHAL_AUDIO_SIF_AutoChangeSoundMode(BOOLEAN Enable)
{
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    KADP_Audio_AtvEnableAutoChangeSoundModeFlag(Enable);

    return 0;
}
DTV_STATUS_T RHAL_AUDIO_SIF_AutoChangeSoundStd(BOOLEAN Enable)
{
    int ret = 0;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if (Enable) ret = KADP_Audio_AtvEnableAutoChangeStdFlag(true);
    else ret = KADP_Audio_AtvEnableAutoChangeStdFlag(false);

    return (DTV_STATUS_T)ret;
}

DTV_STATUS_T RHAL_AUDIO_SIF_FirstTimePlayMono(BOOLEAN Enable)
{
    int ret = 0;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if (Enable) ret = KADP_Audio_AtvForceFirstTimeMonoSoundFlag(true);
    else ret = KADP_Audio_AtvForceFirstTimeMonoSoundFlag(false);

    return (DTV_STATUS_T)ret;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetFwPriority(UINT16 Priority)
{
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(KADP_Audio_AtvSetFwPriority(Priority) < 0)
    {
        return NOT_OK;
    }

    return OK;
}

HAL_AUDIO_SIF_STANDARD_T Audio_SoundStd_KadpToHal(ATV_SOUND_STD KadpSoundStd)
{
	switch(KadpSoundStd)
	{
		case ATV_SOUND_STD_MN_MONO:
			return HAL_AUDIO_SIF_MN_FM;
		case ATV_SOUND_STD_BTSC:
			return HAL_AUDIO_SIF_MN_BTSC;
		case ATV_SOUND_STD_A2_M:
			return HAL_AUDIO_SIF_MN_A2;
		case ATV_SOUND_STD_EIAJ:
			return HAL_AUDIO_SIF_MN_EIAJ;
		case ATV_SOUND_STD_BG_MONO:
			return HAL_AUDIO_SIF_BG_FM;
		case ATV_SOUND_STD_A2_BG:
			return HAL_AUDIO_SIF_BG_A2;
		case ATV_SOUND_STD_NICAM_BG:
			return HAL_AUDIO_SIF_BG_NICAM;
		case ATV_SOUND_STD_DK_MONO:
			return HAL_AUDIO_SIF_DK_FM;
		case ATV_SOUND_STD_A2_DK1:
			return HAL_AUDIO_SIF_DK1_A2;
		case ATV_SOUND_STD_A2_DK2:
			return HAL_AUDIO_SIF_DK2_A2;
		case ATV_SOUND_STD_A2_DK3:
			return HAL_AUDIO_SIF_DK3_A2;
		case ATV_SOUND_STD_NICAM_DK:
			return HAL_AUDIO_SIF_DK_NICAM;
		case ATV_SOUND_STD_AM_MONO:
			return HAL_AUDIO_SIF_L_AM;
		case ATV_SOUND_STD_NICAM_L:
			return HAL_AUDIO_SIF_L_NICAM;
		case ATV_SOUND_STD_FM_MONO_NO_I:
			return HAL_AUDIO_SIF_I_FM;
		case ATV_SOUND_STD_NICAM_I:
			return HAL_AUDIO_SIF_I_NICAM;
		case ATV_SOUND_STD_UNKNOWN:
		default:
			ERROR("[%s][%s][%d] not in case...\n", __FILE__, __func__, __LINE__);
			return HAL_AUDIO_SIF_UNKNOWN;
	}
}

DTV_STATUS_T RHAL_Audio_SIF_SET_MTS_PRIORITY(HAL_SIF_MTS_PRIORITY_T Priority)
{
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(KADP_Audio_AtvSetMtsPriority((ATV_MTS_PRIORITY)Priority) < 0)
    {
        ERROR("[AudioHw_ERR][%s][%s][%d]\n", __FILE__, __func__, __LINE__);
        return NOT_OK;
    }

    return OK;
}

#if 0 //Seems no need
DTV_STATUS_T RHAL_Audio_ScanFmRadioMode(HAL_AUDIO_SIF_STANDARD_T *SoundSTD, HAL_AUDIO_SIF_CARRIER_INFO_T *SIF_CARRIER_INFO)
{
	ATV_SOUND_STD DetectSoundStd = ATV_SOUND_STD_UNKNOWN;
	ATV_Carrier_INFO Carrier_info;
    memset(&Carrier_info, 0, sizeof(ATV_Carrier_INFO));

	INFO("%s Enter.\n", __func__);

	if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

	if(KADP_Audio_AtvScanSoundStd_FMRadio(&DetectSoundStd, &Carrier_info) < 0)
	{
		ERROR("[AudioHw_ERR][%s][%s][%d]\n", __FILE__, __func__, __LINE__);
		return NOT_OK;
	}

	*SoundSTD = Audio_SoundStd_KadpToHal(DetectSoundStd);
	SIF_CARRIER_INFO->CARRIER_SHIFT_VALUE = (UINT8)Carrier_info.Carrier_shif_value;
	SIF_CARRIER_INFO->CARRIER_DEVIATION = (UINT8)Carrier_info.Carrier_deviation;
	return OK;
}
#endif

DTV_STATUS_T RHAL_Audio_ATV_VERIFY_SET_PATH(void)
{
    INFO("%s Enter.\n", __func__);

    if (KADP_Audio_ATV_VERIFY_SET_PATH() != KADP_OK)
    {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_Audio_SetFmRadioMode(AUDIO_ATV_FM_RADIO_MODE Enable)
{
    AUDIO_ATV_FM_RADIO_MODE Mode;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if (Enable == HAL_AUDIO_ATV_FM_RADIO_SCAN)
        Mode = AUDIO_ATV_FM_RADIO_SCAN;
    else if (Enable == HAL_AUDIO_ATV_FM_RADIO_PLAY)
        Mode = AUDIO_ATV_FM_RADIO_PLAY;
    else Mode = AUDIO_ATV_FM_RADIO_STOP;

    if(KADP_Audio_SetFmRadioMode(Mode) < 0)
    {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetSoundInfo(HAL_ATV_SOUND_INFO_T *SoundInfo)
{
    ATV_SOUND_INFO p_sound_info;
    INFO("%s Enter.\n", __func__);
    memset(&p_sound_info, 0, sizeof(ATV_SOUND_INFO));

    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;
    if(KADP_Audio_AtvGetSoundStd(&p_sound_info) < 0)
    {
        return NOT_OK;
    }

    SoundInfo->isNicamSystem = p_sound_info.isNicamSystem;
    SoundInfo->isPlayDigital = p_sound_info.isPlayDigital;
    SoundInfo->current_select = (HAL_ATV_SOUND_SELECT)p_sound_info.current_select;
    SoundInfo->sound_std = (HAL_AUDIO_SIF_STANDARD_T)p_sound_info.sound_std;
    SoundInfo->ana_soundmode = (HAL_ATV_SOUND_MODE)p_sound_info.ana_soundmode;
    SoundInfo->dig_soundmode = (HAL_ATV_SOUND_MODE)p_sound_info.dig_soundmode;
    SoundInfo->std_type = (HAL_ATV_MAIN_STD_TYPE_T)p_sound_info.std_type;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetHighDevMode(BOOLEAN bOnOff)
{
    SINT32 fm_down_6dB = 0;
    SINT32 am_down_6dB = 0;
    SINT32 am_wider_bw = 0;
    A2_BW_SEL_T  deviation_bw;
    A2_BW_SEL_T  deviation_bw_sub;

    INFO("%s Enter.\n", __func__);

    deviation_bw_sub = BANDWIDTH_HDV0_120KHZ;

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;
    if(bOnOff > 1)
    {
        ERROR("[%s] HighDevMode %d\n",__func__,bOnOff);
        return INVALID_PARAMS;
    }
    INFO("HighDev enable %d Stype %d\n", bOnOff, (SINT32)g_AudioSIFInfo.curSifType);

    if (bOnOff == TRUE)
    {
        KADP_Audio_AtvSetDevOnOff(TRUE);
        if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_KOREA_A2_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_ATSC_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV0_240KHZ;    //200Khz
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_AJJA_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV1_740KHZ;    //540Khz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_IN_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV1_740KHZ;    //540Khz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_CN_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV1_480KHZ;    //384Khz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_ID_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV1_480KHZ;    //384Khz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if ( g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_BR_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_US_SELECT)
            deviation_bw = BANDWIDTH_HDV0_370KHZ;
        else
            deviation_bw = BANDWIDTH_HDV0_120KHZ;    //100Khz
    }
    else
    {
        KADP_Audio_AtvSetDevOnOff(FALSE);
        if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_KOREA_A2_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_ATSC_SELECT)
            deviation_bw = BANDWIDTH_HDV0_240KHZ;    //200Khz
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_AJJA_SELECT)
        {
            deviation_bw     = BANDWIDTH_HDV0_370KHZ;    //100kHz
            //deviation_bw_sub = BANDWIDTH_HDV0_370KHZ;    //100kHz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_IN_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV0_370KHZ;    //100kHz
            //deviation_bw_sub = BANDWIDTH_HDV0_370KHZ;    //100kHz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_CN_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV0_370KHZ;    //100kHz
            //deviation_bw_sub = BANDWIDTH_HDV0_370KHZ;    //100kHz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if (g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_DVB_ID_SELECT)
        {
            deviation_bw = BANDWIDTH_HDV0_370KHZ;    //100kHz
            //deviation_bw_sub = BANDWIDTH_HDV0_370KHZ;    //100kHz
            fm_down_6dB = 1;
            am_down_6dB = 0;
            am_wider_bw = 0;
        }
        else if ( g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_BR_SELECT || g_AudioSIFInfo.curSifType == HAL_AUDIO_SIF_BTSC_US_SELECT)
            deviation_bw = BANDWIDTH_HDV0_370KHZ;

        else
            deviation_bw = BANDWIDTH_HDV0_120KHZ;  //50Khz
    }
    g_AudioSIFInfo.bHighDevOnOff = bOnOff;
    KADP_Audio_AtvSetDevBandWidth(deviation_bw, deviation_bw_sub);
    KADP_Audio_AtvSetFMoutDownGain(fm_down_6dB);
    //Audio_AtvSetAMoutDownGain(am_down_6dB);
    //Audio_AtvSetAMWideBW(am_wider_bw);
    return OK;

}

DTV_STATUS_T RHAL_AUDIO_SIF_SetBandSetup(HAL_AUDIO_SIF_TYPE_T eSifType, HAL_AUDIO_SIF_SOUNDSYSTEM_T sifBand)
{
    ATV_SOUND_STD_MAIN_SYSTEM atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;

    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    INFO("[%s] SIF set band %d\n",__func__,(SINT32)sifBand);

    //if(eSifType == HAL_AUDIO_SIF_DVB_SELECT ||
    // eSifType == HAL_AUDIO_SIF_DVB_ID_SELECT ||
    // eSifType == HAL_AUDIO_SIF_DVB_IN_SELECT ||
    // eSifType == HAL_AUDIO_SIF_DVB_CN_SELECT ||
    // eSifType == HAL_AUDIO_SIF_DVB_AJJA_SELECT
    // )
    //{
    KADP_Audio_HwpSetChannelChange();
    KADP_Audio_HwpSetBandDelay();
    //}
    if(sifBand < HAL_AUDIO_SIF_SYSTEM_BG || sifBand > HAL_AUDIO_SIF_SYSTEM_MN)
    {
        ERROR("[%s] error sifBand %d\n",__func__,(SINT32)sifBand);
        return API_INVALID_PARAMS;
    }
    else
    {
          SIF_INPUT_SOURCE  rtk_sif_input_source;

          switch(g_AudioSIFInfo.sifSource)
        {
              case HAL_AUDIO_SIF_INPUT_EXTERNAL:
                  rtk_sif_input_source = SIF_FROM_SIF_ADC;
                  break;
              case HAL_AUDIO_SIF_INPUT_INTERNAL:
                  rtk_sif_input_source = SIF_FROM_IFDEMOD;
                  break;
              default:
                  ERROR("[%s] sif input %d\n",__func__,(SINT32)g_AudioSIFInfo.sifSource);
                  return API_INVALID_PARAMS;
        }
        switch(sifBand)
        {
            case  HAL_AUDIO_SIF_SYSTEM_BG:
                atv_sound_std_main_system = ATV_SOUND_BG_SYSTEM;
                break;
            case  HAL_AUDIO_SIF_SYSTEM_I:
                atv_sound_std_main_system = ATV_SOUND_I_SYSTEM;
                break;
            case  HAL_AUDIO_SIF_SYSTEM_DK:
                atv_sound_std_main_system = ATV_SOUND_DK_SYSTEM;
                break;
            case  HAL_AUDIO_SIF_SYSTEM_L:
                atv_sound_std_main_system = ATV_SOUND_L_SYSTEM;
                break;
            case  HAL_AUDIO_SIF_SYSTEM_MN:
                atv_sound_std_main_system = ATV_SOUND_MN_SYSTEM;
                break;
            case  HAL_AUDIO_SIF_SYSTEM_UNKNOWN:
                atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
                break;
            default:
                ERROR("[%s] not in case1... %d\n",__func__,(SINT32)sifBand);
                return API_INVALID_PARAMS;
        }

        switch (eSifType)
        {
            case  HAL_AUDIO_SIF_ATSC_SELECT:    //A2
            case  HAL_AUDIO_SIF_KOREA_A2_SELECT:
                break;
            case  HAL_AUDIO_SIF_BTSC_SELECT:
            case  HAL_AUDIO_SIF_BTSC_BR_SELECT:
            case  HAL_AUDIO_SIF_BTSC_US_SELECT:
                break;
            case  HAL_AUDIO_SIF_DVB_SELECT:
            case  HAL_AUDIO_SIF_DVB_ID_SELECT:
            case  HAL_AUDIO_SIF_DVB_IN_SELECT:
            case  HAL_AUDIO_SIF_DVB_CN_SELECT:
            case  HAL_AUDIO_SIF_DVB_AJJA_SELECT:
                INFO("DVB ASD func\n");
                //atv_sound_std_main_system = ATV_SOUND_AUTO_SYSTEM;
                break;

            case  HAL_AUDIO_SIF_TYPE_NONE:
            case  HAL_AUDIO_SIF_TYPE_MAX:
            default:
                ERROR("[%s] not in case2... %d\n",__func__,(SINT32)eSifType);
                return API_INVALID_PARAMS;

        }
        if(KADP_Audio_HwpSetAtvAudioBand(rtk_sif_input_source, atv_sound_std_main_system)== -1)
        {
            ERROR("[%s] KADP_Audio_HwpSetAtvAudioBand not success\n",__func__);
        }
        g_AudioSIFInfo.curSifBand= sifBand;
        g_AudioSIFInfo.curSifType = eSifType;
        KADP_Audio_HwpCurSifType( (ATV_AUDIO_SIF_TYPE_T) eSifType);
        return OK;
    }
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetModeSetup(HAL_AUDIO_SIF_STANDARD_T sifStandard)
{
    ATV_SOUND_STD sound_std = ATV_SOUND_STD_UNKNOWN;
    ATV_SOUND_STD_MAIN_SYSTEM main_system = ATV_SOUND_UNKNOWN_SYSTEM;

    // To make sure ATV thread cannot invoke RAL_ATVSetDecoderXMute callback and cause deadlock by //AUDIO_FUNC_CALL()
    KADP_Audio_AtvPauseTvStdDetection(true);
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
    {
        KADP_Audio_AtvPauseTvStdDetection(false);
        return NOT_OK;
    }

    switch(sifStandard)
    {
      //BG
        case  HAL_AUDIO_SIF_BG_NICAM:
            sound_std = ATV_SOUND_STD_NICAM_BG;
            break;
        case  HAL_AUDIO_SIF_BG_FM:
            sound_std = ATV_SOUND_STD_BG_MONO;
            break;
        case  HAL_AUDIO_SIF_BG_A2:
            sound_std = ATV_SOUND_STD_A2_BG;
            break;
            //I
        case  HAL_AUDIO_SIF_I_NICAM:
            sound_std = ATV_SOUND_STD_NICAM_I;
            break;
        case  HAL_AUDIO_SIF_I_FM:
            sound_std = ATV_SOUND_STD_FM_MONO_NO_I;
            break;
            //DK
        case  HAL_AUDIO_SIF_DK_NICAM:
            sound_std = ATV_SOUND_STD_NICAM_DK;
            break;
        case  HAL_AUDIO_SIF_DK_FM:
            sound_std = ATV_SOUND_STD_DK_MONO;
            break;
        case  HAL_AUDIO_SIF_DK1_A2:
            sound_std = ATV_SOUND_STD_A2_DK1;
            break;
        case  HAL_AUDIO_SIF_DK2_A2:
            sound_std = ATV_SOUND_STD_A2_DK2;
            break;
        case  HAL_AUDIO_SIF_DK3_A2:
            sound_std = ATV_SOUND_STD_A2_DK3;
            break;
            //L
        case  HAL_AUDIO_SIF_L_NICAM:
            sound_std = ATV_SOUND_STD_NICAM_L;
            break;
        case  HAL_AUDIO_SIF_L_AM:
            sound_std = ATV_SOUND_STD_AM_MONO;
            break;
            //MN
        case  HAL_AUDIO_SIF_MN_A2:
            sound_std = ATV_SOUND_STD_A2_M;
            break;
        case  HAL_AUDIO_SIF_MN_BTSC:
            sound_std = ATV_SOUND_STD_BTSC;
            break;
        case  HAL_AUDIO_SIF_MN_EIAJ:
            ERROR("HAL_AUDIO_SIF_MN_EIAJ not support\n");
            //Release sem
            KADP_Audio_AtvPauseTvStdDetection(false);
            return API_INVALID_PARAMS;
        default:
            //Release sem
            KADP_Audio_AtvPauseTvStdDetection(false);
            return API_INVALID_PARAMS;
    }

    //INFO("[%s] HAL %d Drv 0x%x\n",__func__,sifStandard, sound_std);
    KADP_Audio_AtvSetSoundStd( main_system, sound_std);
    g_AudioSIFInfo.curSifStandard = sifStandard;

    //Release sem
    KADP_Audio_AtvPauseTvStdDetection(false);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetUserAnalogMode(HAL_AUDIO_SIF_MODE_SET_T sifAudioMode)
{
    SINT32 force_analogmode =0;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    switch(sifAudioMode)
    {
        //A2
        case  HAL_AUDIO_SIF_SET_PAL_MONO:
            KADP_Audio_AtvSetA2SoundSelect(0, 0);
            KADP_Audio_AtvSetNICAMSoundSelect(0, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_STEREO:
            KADP_Audio_AtvSetA2SoundSelect(1, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_DUALI:
            KADP_Audio_AtvSetA2SoundSelect(0, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_DUALII:
            KADP_Audio_AtvSetA2SoundSelect(0, 1);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_DUALI_II:
            KADP_Audio_AtvSetA2SoundSelect(0, 2);
            break;
            //NICAM
        case  HAL_AUDIO_SIF_SET_PAL_NICAM_MONO:
            KADP_Audio_AtvSetNICAMSoundSelect(1, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_NICAM_STEREO:
            KADP_Audio_AtvSetNICAMSoundSelect(1, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_NICAM_DUALI:
            KADP_Audio_AtvSetNICAMSoundSelect(1, 0);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_NICAM_DUALII:
            KADP_Audio_AtvSetNICAMSoundSelect(1, 1);
            break;
        case  HAL_AUDIO_SIF_SET_PAL_NICAM_DUALI_II:
            KADP_Audio_AtvSetNICAMSoundSelect(1, 2);
            break;
            //A2
        case  HAL_AUDIO_SIF_SET_NTSC_A2_MONO:
            KADP_Audio_AtvSetA2SoundSelect(0, 0);
            break;
        case  HAL_AUDIO_SIF_SET_NTSC_A2_SAP:
            KADP_Audio_AtvSetA2SoundSelect(0, 1);//Lang B
            break;
        case  HAL_AUDIO_SIF_SET_NTSC_A2_STEREO:
            KADP_Audio_AtvSetA2SoundSelect(1, 0);
            break;
            //BTSC
        case  HAL_AUDIO_SIF_SET_NTSC_BTSC_MONO:
            KADP_Audio_AtvSetBTSCSoundSelect(0,0);
            break;
        case  HAL_AUDIO_SIF_SET_NTSC_BTSC_STEREO:
            KADP_Audio_AtvSetBTSCSoundSelect(1,0);
            break;
        case  HAL_AUDIO_SIF_SET_NTSC_BTSC_SAP_MONO:
            //assume SAP_1
            KADP_Audio_AtvSetBTSCSoundSelect(1,1);//
            break;
        case  HAL_AUDIO_SIF_SET_NTSC_BTSC_SAP_STEREO:
            //assume SAP_2
            KADP_Audio_AtvSetBTSCSoundSelect(1,1);
            break;

        case HAL_AUDIO_SIF_SET_PAL_MONO_FORCED:
            KADP_Audio_AtvSetA2SoundSelect(0, 0);
            KADP_Audio_AtvSetNICAMSoundSelect(0, 0);
            force_analogmode = 1;
            break;
        case HAL_AUDIO_SIF_SET_PAL_STEREO_FORCED:
            KADP_Audio_AtvSetA2SoundSelect(0, 0);
            break;
        case HAL_AUDIO_SIF_SET_PAL_NICAM_MONO_FORCED:
            KADP_Audio_AtvSetNICAMSoundSelect(0, 0);
            force_analogmode = 1;
            break;
        case HAL_AUDIO_SIF_SET_PAL_NICAM_STEREO_FORCED:
            KADP_Audio_AtvSetNICAMSoundSelect(0, 0);
            force_analogmode = 1;
            break;
        case HAL_AUDIO_SIF_SET_PAL_NICAM_DUAL_FORCED:
            KADP_Audio_AtvSetNICAMSoundSelect(0, 0);
            force_analogmode = 1;
            break;
            //-------------
        case HAL_AUDIO_SIF_SET_PAL_UNKNOWN:
        case HAL_AUDIO_SIF_SET_NTSC_A2_UNKNOWN:
        case HAL_AUDIO_SIF_SET_NTSC_BTSC_UNKNOWN:
        default:

            ERROR("[%s]Not Ready case or expected value %d\n",__func__,sifAudioMode);
            return API_INVALID_PARAMS;
    }
    INFO("[%s] Sound Sel %d\n",__func__,sifAudioMode);
    g_AudioSIFInfo.curSifModeSet = sifAudioMode;
    if(force_analogmode == 1)
    {
        KADP_Audio_AtvForceSoundSel(1);
    }
    else
    {
        KADP_Audio_AtvForceSoundSel(0);
    }
    return OK;
}

//UINT16 value has two value. Bit[7:0] is threshold high and low reg.
DTV_STATUS_T RHAL_AUDIO_SIF_SetA2ThresholdLevel(UINT16 thrLevel)
{
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    if(thrLevel > 100)
    {
        ERROR("[%s]A2 TH Not expected value %d\n",__func__,thrLevel);
        return API_INVALID_PARAMS;
    }

    KADP_Audio_HwpSIFSetA2StereoDualTH( (UINT32)thrLevel);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetNicamThresholdLevel(UINT16 thrLevel)
{
    UINT8  hi_th;
    UINT8  lo_th;

    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    //Bit[7:4] High threshold
    //Bit[3:0] Low threshold
    hi_th  = (thrLevel&0x00F0)>>4;
    lo_th  = thrLevel&0x000F;
    if(hi_th > 15)
    {
        ERROR("[%s]hi_th Not expected value %d\n",__func__,hi_th);
        return API_INVALID_PARAMS;
    }
    if(lo_th > 15)
    {
        ERROR("[%s]lo_th Not expected value %d\n",__func__,lo_th);
        return API_INVALID_PARAMS;
    }
    KADP_Audio_HwpSIFSetNicamTH(hi_th,lo_th);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetBandDetect(HAL_AUDIO_SIF_SOUNDSYSTEM_T soundSystem, UINT32 *pBandStrength)
{
    ATV_SOUND_STD_MAIN_SYSTEM atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD_MAIN_SYSTEM HWDetectSoundSystem = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD HWDetectSoundStd = ATV_SOUND_STD_UNKNOWN;
    UINT32 pToneSNR = 0;

    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    switch(soundSystem)
    {
        case  HAL_AUDIO_SIF_SYSTEM_BG:
            atv_sound_std_main_system = ATV_SOUND_BG_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_I:
            atv_sound_std_main_system = ATV_SOUND_I_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_DK:
            atv_sound_std_main_system = ATV_SOUND_DK_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_L:
            atv_sound_std_main_system = ATV_SOUND_L_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_MN:
            atv_sound_std_main_system = ATV_SOUND_MN_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_UNKNOWN:
            atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
            break;
        default:
            ERROR("[%s] not in case1... %d\n",__func__,soundSystem);
            return API_INVALID_PARAMS;
    }
    KADP_Audio_HwpSIFGetMainToneSNR(atv_sound_std_main_system, &HWDetectSoundSystem, &HWDetectSoundStd, &pToneSNR);

    *pBandStrength = (UINT32)pToneSNR;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_DetectSoundSystem(HAL_AUDIO_SIF_SOUNDSYSTEM_T setSoundSystem, BOOLEAN bManualMode,
                            HAL_AUDIO_SIF_SOUNDSYSTEM_T *pDetectSoundSystem, UINT32 *pSignalQuality)
{
    ATV_SOUND_STD_MAIN_SYSTEM atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD_MAIN_SYSTEM HWDetectSoundSystem = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD HWDetectSoundStd = ATV_SOUND_STD_UNKNOWN;
    UINT32 pToneSNR = 0;
    SIF_INPUT_SOURCE rtk_sif_input_source;

    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
     return NOT_OK;
    //set band internal/external
    switch(g_AudioSIFInfo.sifSource)
    {
        case HAL_AUDIO_SIF_INPUT_EXTERNAL:
            rtk_sif_input_source = SIF_FROM_SIF_ADC;
            break;
        case HAL_AUDIO_SIF_INPUT_INTERNAL:
            rtk_sif_input_source = SIF_FROM_IFDEMOD;
            break;
        default:
            ERROR("[%s] sif input %d\n",__func__, g_AudioSIFInfo.sifSource);
            return API_INVALID_PARAMS;
    }

    //detect sound system
    switch(setSoundSystem)
    {
        case  HAL_AUDIO_SIF_SYSTEM_BG:
            atv_sound_std_main_system = ATV_SOUND_BG_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_I:
            atv_sound_std_main_system = ATV_SOUND_I_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_DK:
            atv_sound_std_main_system = ATV_SOUND_DK_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_L:
            atv_sound_std_main_system = ATV_SOUND_L_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_MN:
            atv_sound_std_main_system = ATV_SOUND_MN_SYSTEM;
            break;
        case  HAL_AUDIO_SIF_SYSTEM_UNKNOWN:
            atv_sound_std_main_system = ATV_SOUND_AUTO_SYSTEM;
            break;
        default:
            ERROR("[%s] not in case1... %d\n",__func__,setSoundSystem);
            return API_INVALID_PARAMS;

    }
    //Audio_HwpSIFDetectedSoundSystem(&atv_sound_std_main_system);
    KADP_Audio_HwpSetAtvAudioBand(rtk_sif_input_source, atv_sound_std_main_system);
    KADP_Audio_HwpSIFGetMainToneSNR(atv_sound_std_main_system, &HWDetectSoundSystem, &HWDetectSoundStd, &pToneSNR);

    if(bManualMode == TRUE)
    {
        *pDetectSoundSystem = setSoundSystem;//  manual mode setSoundSystem and *pDetectSoundSystem must be the same
    }
    else
    {
        switch(HWDetectSoundSystem) //auto mode reuten HWDetectSoundSystem
        {
            case  ATV_SOUND_BG_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_BG;
                break;
            case  ATV_SOUND_I_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_I;
                break;
            case  ATV_SOUND_DK_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_DK;
                break;
            case  ATV_SOUND_L_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_L;
            break;
            case  ATV_SOUND_MN_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_MN;
            break;
            case  ATV_SOUND_UNKNOWN_SYSTEM:
                *pDetectSoundSystem = HAL_AUDIO_SIF_SYSTEM_UNKNOWN;
            break;
            default:
                ERROR("[%s] not in case2... %d\n",__func__,HWDetectSoundSystem);
                return API_INVALID_PARAMS;
        }
    }

    *pSignalQuality = (UINT32)pToneSNR;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_CheckNicamDigital(HAL_AUDIO_SIF_EXISTENCE_INFO_T *pIsNicamDetect)
{
    SINT32 nicam_flag;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_HwpSIFCheckNicamDigital(&nicam_flag);
    if(nicam_flag == 1)//exist
    {
        *pIsNicamDetect = HAL_AUDIO_SIF_PRESENT;
        g_AudioSIFInfo.curSifIsNicam = HAL_AUDIO_SIF_PRESENT;
    }
    else if(nicam_flag == 2)// not exist
    {
        *pIsNicamDetect = HAL_AUDIO_SIF_ABSENT;
        g_AudioSIFInfo.curSifIsNicam = HAL_AUDIO_SIF_ABSENT;
    }
    else
    {
        g_AudioSIFInfo.curSifIsNicam = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
        return NOT_OK;
    }
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_CheckAvailableSystem(HAL_AUDIO_SIF_AVAILE_STANDARD_T standard,
                                            HAL_AUDIO_SIF_EXISTENCE_INFO_T *pAvailability)
{
    ATV_SOUND_STD  sound_std = ATV_SOUND_STD_UNKNOWN;
    INFO("%s Enter.\n", __func__);// decrease print log
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_HwpSIFDetectedSoundStandard(&sound_std);

    *pAvailability = HAL_AUDIO_SIF_ABSENT;
    if(standard == HAL_AUDIO_SIF_NICAM)
    {
        if(sound_std == ATV_SOUND_STD_NICAM_BG || sound_std == ATV_SOUND_STD_NICAM_DK || sound_std == ATV_SOUND_STD_NICAM_I || sound_std == ATV_SOUND_STD_NICAM_L)
        {
            *pAvailability = HAL_AUDIO_SIF_PRESENT;
        }
    }
    else if(standard == HAL_AUDIO_SIF_A2)
    {
        if(sound_std == ATV_SOUND_STD_A2_M || sound_std == ATV_SOUND_STD_A2_BG || sound_std == ATV_SOUND_STD_A2_DK1 || sound_std == ATV_SOUND_STD_A2_DK2 || sound_std == ATV_SOUND_STD_A2_DK3)
        {
            *pAvailability = HAL_AUDIO_SIF_PRESENT;
        }
    }
    else if(standard == HAL_AUDIO_SIF_FM)
    {
        if(sound_std == ATV_SOUND_STD_AM_MONO ||
           sound_std == ATV_SOUND_STD_BG_MONO ||
           sound_std == ATV_SOUND_STD_DK_MONO ||
           sound_std == ATV_SOUND_STD_FM_MONO_NO_I ||
           sound_std == ATV_SOUND_STD_BTSC ||
           sound_std == ATV_SOUND_STD_MN_MONO)
        {
            //check BTSC: BTSC belong to FM???
            *pAvailability = HAL_AUDIO_SIF_PRESENT;
        }
    }

    else if(standard == HAL_AUDIO_SIF_DETECTING_AVALIBILITY)
    {
        ERROR("[%s] error para %d",__func__,standard);
        return API_INVALID_PARAMS;
    }
    else
    {
        ERROR("[%s] error para %d",__func__,standard);
        return API_INVALID_PARAMS;
    }

    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_SIF_CheckA2DK(HAL_AUDIO_SIF_STANDARD_T standard,
                                    HAL_AUDIO_SIF_EXISTENCE_INFO_T *pAvailability)
{
    ATV_SOUND_STD sound_std = ATV_SOUND_STD_UNKNOWN;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_HwpSIFDetectedSoundStandard(&sound_std);


    g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_DETECTING_EXSISTANCE;
    switch(standard)
    {
        case  HAL_AUDIO_SIF_DK1_A2:
            if(sound_std == ATV_SOUND_STD_A2_DK1)
            {
                *pAvailability = HAL_AUDIO_SIF_PRESENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_PRESENT;
            }
            else
            {
                *pAvailability = HAL_AUDIO_SIF_ABSENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_ABSENT;
            }
            break;
        case  HAL_AUDIO_SIF_DK2_A2:
            if(sound_std == ATV_SOUND_STD_A2_DK2)
            {
                *pAvailability = HAL_AUDIO_SIF_PRESENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_PRESENT;
            }
            else
            {
                *pAvailability = HAL_AUDIO_SIF_ABSENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_ABSENT;
            }
            break;
        case  HAL_AUDIO_SIF_DK3_A2:
            if(sound_std == ATV_SOUND_STD_A2_DK3)
            {
                *pAvailability = HAL_AUDIO_SIF_PRESENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_PRESENT;
            }
            else
            {
                *pAvailability = HAL_AUDIO_SIF_ABSENT;
                g_AudioSIFInfo.curSifIsA2 = HAL_AUDIO_SIF_ABSENT;
            }
            break;
        case  HAL_AUDIO_SIF_BG_NICAM:
        case  HAL_AUDIO_SIF_BG_FM:
        case  HAL_AUDIO_SIF_BG_A2:
        case  HAL_AUDIO_SIF_I_NICAM:
        case  HAL_AUDIO_SIF_I_FM:
        case  HAL_AUDIO_SIF_DK_NICAM:
        case  HAL_AUDIO_SIF_DK_FM:
        case  HAL_AUDIO_SIF_L_NICAM:
        case  HAL_AUDIO_SIF_L_AM:
        case  HAL_AUDIO_SIF_MN_A2:
        case  HAL_AUDIO_SIF_MN_BTSC:
        case  HAL_AUDIO_SIF_MN_EIAJ:
            ERROR("[%s] error para %d",__func__,standard);
            return API_INVALID_PARAMS;
        default:
            ERROR("[%s] error para %d",__func__,standard);
            return API_INVALID_PARAMS;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetA2StereoLevel(UINT16 *pLevel)
{
    UINT32  getA2Th = 0;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_HwpSIFGetA2StereoDualTH( &getA2Th);
    *pLevel = (UINT16)getA2Th;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetNicamThresholdLevel(UINT16 *pLevel)
{
    UINT8  hi_th = 0;
    UINT8  lo_th = 0;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_HwpSIFGetNicamTH( &hi_th, &lo_th);
    //Bit[7:4] High threshold
    //Bit[3:0] Low threshold
    *pLevel  = (hi_th<<4) | (lo_th);
    INFO("[%s] level %x hi %x lo %x\n",__func__, *pLevel, hi_th,lo_th);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetNicamStable(UINT16 *IsStable)
{
	SINT32 nicam_stable = 0;
	INFO("%s Enter.\n", __func__);
	if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
		return NOT_OK;

	KADP_Audio_AtvGetNicamSignalStable(&nicam_stable);
	*IsStable = nicam_stable;
	INFO("[%s] Nicam Stable = %d\n", __func__, *IsStable);
	return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetCurAnalogMode(HAL_AUDIO_SIF_MODE_GET_T *pSifAudioMode)
{
    ATV_SOUND_INFO sound_info;
    UINT8 isNTSC = 0;
    SINT32 nicam_stable = 0;
    //INFO("%s Enter.\n", __func__);

    memset(&sound_info, 0, sizeof(ATV_SOUND_INFO));
    //INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    KADP_Audio_AtvGetSoundStd(&sound_info);
    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_UNKNOWN;//need add for PAL NTSC BTSC..........
    switch(sound_info.std_type)
    {
        case ATV_MAIN_STD_MONO:
            if( g_AudioSIFInfo.curSifType & (HAL_AUDIO_SIF_ATSC_SELECT | HAL_AUDIO_SIF_KOREA_A2_SELECT) )
                *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_A2_MONO;
            if( g_AudioSIFInfo.curSifType & (HAL_AUDIO_SIF_BTSC_SELECT | HAL_AUDIO_SIF_BTSC_BR_SELECT | HAL_AUDIO_SIF_BTSC_US_SELECT) )
                *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_BTSC_MONO;
            if( g_AudioSIFInfo.curSifType & (HAL_AUDIO_SIF_DVB_SELECT | HAL_AUDIO_SIF_DVB_ID_SELECT | HAL_AUDIO_SIF_DVB_IN_SELECT | HAL_AUDIO_SIF_DVB_CN_SELECT | HAL_AUDIO_SIF_DVB_AJJA_SELECT /*| HAL_AUDIO_SIF_DVB_AU_SELECT*/) )
                *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_MONO;
            else
            *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_MONO;
            break;
        case ATV_MAIN_STD_NICAM:
            if(sound_info.dig_soundmode  == ATV_SOUND_MODE_MONO)
            {
            	KADP_Audio_AtvGetNicamSignalStable(&nicam_stable);
                if( 1 ==  nicam_stable)
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_NICAM_MONO;
                }
                else
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_MONO;
                }
            }
            else if(sound_info.dig_soundmode  == ATV_SOUND_MODE_STEREO)
            {
                *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_NICAM_STEREO;
            }
            else if(sound_info.dig_soundmode  == ATV_SOUND_MODE_DUAL)
            {
                *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_NICAM_DUAL;
            }
            break;
        case ATV_MAIN_STD_A2:
            if((sound_info.sound_std == ATV_SOUND_STD_MN_MONO || sound_info.sound_std == ATV_SOUND_STD_BTSC || sound_info.sound_std == ATV_SOUND_STD_A2_M))
            {
                isNTSC = 1;
            }
            if(sound_info.ana_soundmode == ATV_SOUND_MODE_STEREO)
            {
                if(isNTSC == 1)
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_A2_STEREO;
                }
                else
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_STEREO;
                }

            }
            else if(sound_info.ana_soundmode == ATV_SOUND_MODE_DUAL)
            {
                if(isNTSC == 1)
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_A2_SAP;
                }
                else
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_DUAL;
                }
            }
            else if(sound_info.ana_soundmode == ATV_SOUND_MODE_MONO)
            {
                if(isNTSC == 1)
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_A2_MONO;
                }
                else
                {
                    *pSifAudioMode = HAL_AUDIO_SIF_GET_PAL_MONO;
                }
            }
            break;
        case ATV_MAIN_STD_BTSC:
            if(sound_info.ana_soundmode == ATV_SOUND_MODE_MONO)
            {
                *pSifAudioMode =  HAL_AUDIO_SIF_GET_NTSC_BTSC_MONO;
            }
            else if(sound_info.ana_soundmode == ATV_SOUND_MODE_STEREO)
            {
                *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_BTSC_STEREO;
            }
            else if(sound_info.ana_soundmode == ATV_SOUND_MODE_SAP_MONO)
            {
                *pSifAudioMode = HAL_AUDIO_SIF_GET_NTSC_BTSC_SAP_MONO;
            }
            else if(sound_info.ana_soundmode == ATV_SOUND_MODE_SAP_STEREO)
            {
                *pSifAudioMode =  HAL_AUDIO_SIF_GET_NTSC_BTSC_SAP_STEREO;
            }

            break;
        case ATV_MAIN_STD_UNKNOW:
        default:
            break;
    }
    g_AudioSIFInfo.curSifModeGet = *pSifAudioMode;
    if(sound_info.std_type == ATV_MAIN_STD_NICAM)
    {
        AUDIO_VERBOSE("[%s] Hal_mode %d Ni_drv_mod %d\n", __func__, (SINT32)*pSifAudioMode, (SINT32)sound_info.dig_soundmode);
    }
    else
    {
        AUDIO_VERBOSE("[%s] std_type %d Hal_mode %d drv_mod %d\n", __func__, sound_info.std_type, (INT32)*pSifAudioMode, (INT32)sound_info.ana_soundmode);
    }

    return OK;
}

BOOLEAN RHAL_AUDIO_SIF_IsSIFExist(void)
{
    ATV_SOUND_STD_MAIN_SYSTEM atv_sound_std_main_system = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD_MAIN_SYSTEM HWDetectSoundSystem = ATV_SOUND_UNKNOWN_SYSTEM;
    ATV_SOUND_STD HWDetectSoundStd = ATV_SOUND_STD_UNKNOWN;
    UINT32 ToneSNR = 0;
    UINT32 passSNR = 0x1500;//dB

    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    for(atv_sound_std_main_system=ATV_SOUND_DK_SYSTEM; atv_sound_std_main_system<=ATV_SOUND_L_SYSTEM;)
    {
        KADP_Audio_HwpSIFGetMainToneSNR(atv_sound_std_main_system, &HWDetectSoundSystem, &HWDetectSoundStd, &ToneSNR);
        atv_sound_std_main_system =  (ATV_SOUND_STD_MAIN_SYSTEM)((SINT16)atv_sound_std_main_system+1);
        if(ToneSNR > passSNR)
        {
            INFO("[%s] tone %d pass %d\n", __func__, ToneSNR, passSNR);
            return TRUE;
        }

    }
    INFO("[%s] tone %d pass %d\n", __func__, ToneSNR, passSNR);
    return FALSE;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetAudioEQMode(BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    INFO("[%s] This function does not use now.\n", __func__);
    return OK;    //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_AENC_Start(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_ENCODING_FORMAT_T audioType)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AENC_Stop(HAL_AUDIO_AENC_INDEX_T aencIndex)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AENC_RegCallback(HAL_AUDIO_AENC_INDEX_T aencIndex, pfnAENCDataHandling pfnCallBack)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_AENC_SetInfo(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_INFO_T info)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AENC_GetInfo(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_AENC_INFO_T *pInfo)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_AENC_SetVolume(HAL_AUDIO_AENC_INDEX_T aencIndex, HAL_AUDIO_VOLUME_T volume)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_AENC_CopyData(HAL_AUDIO_AENC_INDEX_T aencIndex, UINT8 *pDest, UINT8 *pBufAddr, UINT32 datasize, UINT8 *pRStart, UINT8 *pREnd)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_AENC_ReleaseData(HAL_AUDIO_AENC_INDEX_T aencIndex, UINT8 *pBufAddr, UINT32 datasize)
{
    INFO("%s Enter.\n", __func__);
    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_PCM_SetVolume(HAL_AUDIO_PCM_INPUT_T apcmIndex, HAL_AUDIO_VOLUME_T volume)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    SINT32 dsp_gain;
    UINT32 i;

    INFO("%s Enter.\n", __func__);
    AUDIO_INFO("[AUDH] %s capture %d %x \n", __FUNCTION__, apcmIndex, volume.mainVol);

    if(!RangeCheck(apcmIndex, 0, HAL_AUDIO_PCM_INPUT_MAX)) return NOT_OK;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if((apcmIndex != HAL_AUDIO_PCM_I2S) && (apcmIndex != HAL_AUDIO_PCM_SB_PCM))
    {
        AUDIO_ERROR("[AUDH] error capture source %x \n",  apcmIndex);
        return NOT_OK;
    }


    dsp_gain = Volume_to_DSPGain(volume);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_PCM_CAPTURE;
    audioConfig.value[2] = FALSE;
    audioConfig.value[3] = 0x03;
    for(i = 0; i < 2; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_PCM_SetVolume_ByPin(HAL_AUDIO_PCM_INPUT_T apcmIndex, HAL_AUDIO_VOLUME_T volume, HAL_AUDIO_BT_OUT_PIN pinID)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    SINT32 dsp_gain;
    UINT32 i;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s capture %d %x pin %d \n", __func__, apcmIndex, volume.mainVol, pinID);

    if(!RangeCheck(apcmIndex, 0, HAL_AUDIO_PCM_INPUT_MAX)) return NOT_OK;
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if((apcmIndex != HAL_AUDIO_PCM_I2S) && (apcmIndex != HAL_AUDIO_PCM_SB_PCM))
    {
        ERROR("[AUDH] error capture source %x \n",  apcmIndex);
        return NOT_OK;
    }

    /* check hal define is the same with common */
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT  == (u_int)PCM_OUT_RTK);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT1 == (u_int)PCM_OUT1);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT2 == (u_int)PCM_OUT2);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT3 == (u_int)PCM_OUT3);

    if((pinID != HAL_AUDIO_BT_PCM_OUT) && (pinID != HAL_AUDIO_BT_PCM_OUT1) && (pinID != HAL_AUDIO_BT_PCM_OUT2) && (pinID != HAL_AUDIO_BT_PCM_OUT3))
    {
        ERROR("%s  error pinID  %d != %d or %d or %d or %d \n", __func__, pinID, PCM_OUT_RTK, PCM_OUT1, PCM_OUT2, PCM_OUT3);
        return NOT_OK;
    }

    dsp_gain = Volume_to_DSPGain(volume);
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_PCM_CAPTURE;
    audioConfig.value[2] = (int)pinID;
    audioConfig.value[3] = 0x03;
    for(i = 0; i < 2; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    if (SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_PCM_SetMute_ByPin(BOOLEAN bOnOff, HAL_AUDIO_BT_OUT_PIN pinID)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 i;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s capture %s pin %d \n", __func__, bOnOff ? "Mute" : "Unmute", pinID);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    /* check hal define is the same with common */
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT  == (u_int)PCM_OUT_RTK);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT1 == (u_int)PCM_OUT1);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT2 == (u_int)PCM_OUT2);
   //assert((u_int)HAL_AUDIO_BT_PCM_OUT3 == (u_int)PCM_OUT3);

    if((pinID != HAL_AUDIO_BT_PCM_OUT) && (pinID != HAL_AUDIO_BT_PCM_OUT1) && (pinID != HAL_AUDIO_BT_PCM_OUT2) && (pinID != HAL_AUDIO_BT_PCM_OUT3))
    {
        ERROR("%s  error pinID  %d != %d or %d or %d or %d \n", __func__, pinID, PCM_OUT_RTK, PCM_OUT1, PCM_OUT2, PCM_OUT3);
        return NOT_OK;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_MUTE;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_PCM_CAPTURE;
    audioConfig.value[2] = (int)pinID;
    audioConfig.value[3] = 0x03;
    for(i = 0; i < 2; i++)
    {
        audioConfig.value[4+i] = bOnOff;
    }

    if (SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    return OK;  //OK or NOT_OK.
}

DTV_STATUS_T RHAL_AUDIO_SetSPKOutput(UINT8 i2sNumber, HAL_AUDIO_SAMPLING_FREQ_T samplingFreq)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

void ConvertHALVolume2Decimal(HAL_AUDIO_VOLUME_T halVol, SINT32* mainVolume, SINT32* fineVolume)
{
    SINT32 mainVol, fineVol;

    mainVol= (SINT32)halVol.mainVol - (SINT32)0x7F;
    fineVol = (SINT32)halVol.fineVol * 625;// 1/16 = 0.0625
    if( (mainVol < 0) && ( fineVol != 0 ))
    {
       mainVol = mainVol + 1;
       fineVol = 10000 - (fineVol);
    }
    *mainVolume = mainVol;
    *fineVolume = fineVol;
}

void RAL_AUDIO_ShowHALDebugInfo(void)
{
    #define GETMUTESTRING(mute) (mute)? "MUTE":"UN-MUTE"

    SINT32 i;
    HAL_AUDIO_ADEC_INDEX_T k;
    SINT32 mainVolume, fineVolume;

    KADP_Audio_PrintSemBacktrace();

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("flow DEC0 (main)info:: \n");
    else
        INFO("flow DEC0 (sub)info:: \n");
    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 1);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("flow DEC1 (main)info:: \n");
    else
        INFO("flow DEC1 (sub)info:: \n");

    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 1);

    ShowFlow(HAL_AUDIO_RESOURCE_SDEC0, HAL_AUDIO_RESOURCE_STATUS_UNKNOWN, 1); // HAL_AUDIO_RESOURCE_SDEC0 is for showing all information

    for(i = 0; i <= HAL_AUDIO_ADEC_MAX; i++)
    {
        k = (HAL_AUDIO_ADEC_INDEX_T)i;
        ConvertHALVolume2Decimal(GetDecInVolume(k), &mainVolume, &fineVolume);
        INFO("\ndec %d input gain main 0x%x fine 0x%x (%d.%d db)\n", k, (((HAL_AUDIO_VOLUME_T)GetDecInVolume(k)).mainVol) , (((HAL_AUDIO_VOLUME_T)GetDecInVolume(k)).fineVol), mainVolume, fineVolume);
        ConvertHALVolume2Decimal(GetDecOutVolume(k, 0), &mainVolume, &fineVolume);
        INFO("dec %d output gain L main 0x%x fine 0x%x (%d.%d db) \n", k, (((HAL_AUDIO_VOLUME_T)GetDecOutVolume(k, 0)).mainVol) , (((HAL_AUDIO_VOLUME_T)GetDecOutVolume(k, 0)).fineVol), mainVolume, fineVolume);
        ConvertHALVolume2Decimal(GetDecOutVolume(k, 1), &mainVolume, &fineVolume);
        INFO("dec %d output gain R main 0x%x fine 0x%x (%d.%d db) \n", k, (((HAL_AUDIO_VOLUME_T)GetDecOutVolume(k, 1)).mainVol) , (((HAL_AUDIO_VOLUME_T)GetDecOutVolume(k, 1)).fineVol),  mainVolume, fineVolume);
        INFO("dec %d input %s connect %s atv %s fw %s \n",  k, GETMUTESTRING(Aud_decstatus[i].decInMute), GETMUTESTRING(!ADEC_CheckConnect(k)), GETMUTESTRING(Aud_decstatus[i].decAtvMute), GETMUTESTRING(AudioDecInfo[i].decMute)  );
        INFO("dec %d user ES %s connect %s fw %s \n", k, GETMUTESTRING(GetDecESMute(i)), GETMUTESTRING(ADEC_SPDIF_ES_CheckConnect()), GETMUTESTRING(GetCurrSNDOutMute(ENUM_DEVICE_SPDIF_ES)) );
    }

    ConvertHALVolume2Decimal( g_AudioStatusInfo.curSPKOutVolume, &mainVolume, &fineVolume);
    INFO("\nspk  volume main 0x%x  fine 0x%x (%d.%d db) delay 0x%x ms \n", g_AudioStatusInfo.curSPKOutVolume.mainVol, g_AudioStatusInfo.curSPKOutVolume.fineVol, mainVolume,  fineVolume, g_AudioStatusInfo.curSPKOutDelay);
    INFO("spk  user %s  connect %s fw %s\n", GETMUTESTRING(g_AudioStatusInfo.curSPKMuteStatus), GETMUTESTRING(!ADEC_SPK_CheckConnect()),  GETMUTESTRING(GetCurrSNDOutMute(ENUM_DEVICE_SPEAKER)));

    ConvertHALVolume2Decimal( g_AudioStatusInfo.curSPDIFOutVolume, &mainVolume, &fineVolume);
    INFO("\nspdif  volume main 0x%x fine 0x%x (%d.%d db) delay 0x%x \n", g_AudioStatusInfo.curSPDIFOutVolume.mainVol,  g_AudioStatusInfo.curSPDIFOutVolume.fineVol, mainVolume,  fineVolume, g_AudioStatusInfo.curSPDIFOutDelay);
    INFO("spdif  user  %s  connect %s fw %s  \n", GETMUTESTRING(g_AudioStatusInfo.curSPDIFMuteStatus),  GETMUTESTRING(!ADEC_SPDIF_CheckConnect()),  GETMUTESTRING(GetCurrSNDOutMute(ENUM_DEVICE_SPDIF)));

    ConvertHALVolume2Decimal( g_AudioStatusInfo.curHPOutVolume, &mainVolume, &fineVolume);
    INFO("\nhp  volume main 0x%x  fine 0x%x (%d.%d db) delay 0x%x ms\n", g_AudioStatusInfo.curHPOutVolume.mainVol, g_AudioStatusInfo.curHPOutVolume.fineVol,  mainVolume,  fineVolume, g_AudioStatusInfo.curHPOutDelay);
    INFO("hp  user  %s  connect %s fw %s \n", GETMUTESTRING(g_AudioStatusInfo.curHPMuteStatus),  GETMUTESTRING(!ADEC_HP_CheckConnect()),  GETMUTESTRING(GetCurrSNDOutMute(ENUM_DEVICE_HEADPHONE)));

    ConvertHALVolume2Decimal( g_AudioStatusInfo.curSCARTOutVolume, &mainVolume, &fineVolume);
    INFO("\nscart  volume main 0x%x  fine 0x%x (%d.%d db) delay 0x%x ms  %s \n", g_AudioStatusInfo.curSCARTOutVolume.mainVol, g_AudioStatusInfo.curSCARTOutVolume.fineVol,  mainVolume,  fineVolume, g_AudioStatusInfo.curSCARTOutDelay, GETMUTESTRING(g_AudioStatusInfo.curSCARTMuteStatus));

    for(i = 0; i <  AUD_AMIX_MAX; i++)
    {
        HAL_AUDIO_RESOURCE_T resID;
        ConvertHALVolume2Decimal(g_mixer_gain[i], &mainVolume, &fineVolume);
        INFO("\nAmixer %d input gain main 0x%x fine 0x%x (%d.%d db)\n", i, (g_mixer_gain[i].mainVol) , (g_mixer_gain[i].fineVol), mainVolume, fineVolume);
        ConvertHALVolume2Decimal(g_mixer_out_gain, &mainVolume, &fineVolume);
        INFO("Amixer %d output gain main 0x%x fine 0x%x (%d.%d db) \n", i, (g_mixer_out_gain.mainVol) , (g_mixer_out_gain.fineVol), mainVolume, fineVolume);
        resID = amixer2res((HAL_AUDIO_MIXER_INDEX_T)i);
        INFO("Amixer %d  user %s, connect %s  fw %s\n",  i, GETMUTESTRING(GetAmixerUserMute((HAL_AUDIO_MIXER_INDEX_T)i)), GETMUTESTRING(!AMIXER_CheckConnect(resID)),  GETMUTESTRING(ADSP_AMIXER_GetMute(i)) );
    }
}

DTV_STATUS_T RHAL_AUDIO_PauseAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    //to do:
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  adec index %d  \n", __func__,adecIndex);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    return RAL_AUDIO_PauseDecoding(adecIndex, ResourceStatus, sizeof(ResourceStatus));
}


/// for system request ,only stop flow directly  no need to delete it.
// pair of HAL_AUDIO_ResumeAndRunAdec
// pvr use
DTV_STATUS_T RHAL_AUDIO_PauseAndStopFlowAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_RESOURCE_T curResourceId;
    //to do:
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  adec index %d  \n", __func__,adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
           return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curResourceId = adec2res(adecIndex);

    if(Aud_mainDecIndex == HAL_AUDIO_ADEC0)
        INFO("PauseAndStopFlow   DEC0 (main) info::\n");
    else
        INFO("PauseAndStopFlow   DEC0 (sub):: \n");

    Aud_flow0->ShowCurrentExitModule(Aud_flow0, 0);


    if(Aud_mainDecIndex == HAL_AUDIO_ADEC1)
        INFO("PauseAndStopFlow    DEC1 (main) info::\n");
    else
        INFO("PauseAndStopFlow    DEC1 (sub):: \n");

    Aud_flow1->ShowCurrentExitModule(Aud_flow1, 0);

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_PAUSE, 0); // HAL_AUDIO_RESOURCE_NO_CONNECTION for show current state

    // open ADEC at run state
    if(AUDIO_HAL_CHECK_ISATPAUSEANDSTOPSTATE(ResourceStatus[curResourceId], 0))
    {
        return OK;
    }

    // check if onlyStopflow available
    if(AUDIO_HAL_CHECK_PAUSE_AND_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0))
    {
        ERROR("[AUDH] %s status is at %s \n", GetResourceString(curResourceId),
                GetResourceStatusString(ResourceStatus[curResourceId].connectStatus[0]));
        return NOT_OK;
    }

    // only stop and no delete flow
    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        Aud_flow0->Stop(Aud_flow0);
    }
    else
    {
        Aud_flow1->Stop(Aud_flow1);
    }
    ResourceStatus[curResourceId].connectStatus[0] = HAL_AUDIO_RESOURCE_PAUSE_AND_STOP;
    return OK;
}


DTV_STATUS_T RHAL_AUDIO_ResumeAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  adec index %d  \n", __func__,adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    return RAL_AUDIO_ResumeDecoding(adecIndex, HAL_AUDIO_SRC_TYPE_UNKNOWN, ResourceStatus, sizeof(ResourceStatus));// don't need to care type
}

// pair HAL_AUDIO_PauseAndStopFlowAdec
// pvr use
DTV_STATUS_T RHAL_AUDIO_ResumeAndRunAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    HAL_AUDIO_SRC_TYPE_T audioType = HAL_AUDIO_SRC_TYPE_UNKNOWN;
    HAL_AUDIO_RESOURCE_T curDecResourceId, decIptResId;
    HAL_AUDIO_RESOURCE_T curResourceId;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  adec index %d  \n", __func__,adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    // no condition check, condition check is at RAL_AUDIO_StartDecoding
    INFO("[AUDH] %s ADEC%d type %s\n", __func__, adecIndex, GetSRCTypeName(audioType));

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    curResourceId = adec2res(adecIndex);

    if(!AUDIO_HAL_CHECK_ISATPAUSEANDSTOPSTATE(ResourceStatus[curResourceId], 0))
    {
        ERROR("[AUDH] %s status is at %s \n", GetResourceString(curResourceId),
                GetResourceStatusString(ResourceStatus[curResourceId].connectStatus[0]));
        return NOT_OK;
    }

    ShowFlow(adec2res(adecIndex), HAL_AUDIO_RESOURCE_RUN, 0);

    audioType = Aud_Adec_info[adecIndex].curAdecFormat;

    if(adecIndex == HAL_AUDIO_ADEC0)
    {
        Aud_flow0->Flush(Aud_flow0);
        Aud_flow0->Run(Aud_flow0);
    }
    else
    {
        Aud_flow1->Flush(Aud_flow1);
        Aud_flow1->Run(Aud_flow1);
    }

    curDecResourceId = adec2res(adecIndex);
    decIptResId = GetNonOutputModuleSingleInputResource(ResourceStatus[curDecResourceId]);// dec input source
    if(IsDTVSource(decIptResId))
    {
        SINT32 newfmt[9] = {0};
        Base* pDtv = Aud_DTV[decIptResId - HAL_AUDIO_RESOURCE_ATP0];
        DEMUX_CHANNEL_T demux_ch = (DEMUX_CHANNEL_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECChannel(pDtv);
        RSDEC_ENUM_TYPE_DEST_T sdec_dest = (RSDEC_ENUM_TYPE_DEST_T)((DtvCom*)pDtv->pDerivedObj)->GetSDECDest(pDtv);
        // AUDIO_DEC_NEW_FORMAT:: audio type + private info[8]
        newfmt[0] = Convert2AudioDecType(audioType);
        if(newfmt[0] == AUDIO_UNKNOWN_TYPE)
            return NOT_OK;

        newfmt[1] = (newfmt[0] == AUDIO_AAC_DECODER_TYPE)? 1 : 0; // aac latm
        newfmt[7] = 1 << 1; // DTV mode
        KADP_SDEC_PrivateInfo(demux_ch, sdec_dest, DEMUX_PRIVATEINFO_AUDIO_FORMAT_, (ULONG*)newfmt, sizeof(newfmt));
    }
    ResourceStatus[adec2res(adecIndex)].connectStatus[0] = HAL_AUDIO_RESOURCE_RUN;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_FlushAdec(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    FlowManager* pFlow;
    INFO("%s Enter.\n", __func__);
    INFO("[AUDH] %s  adec index %d  \n", __func__,adecIndex);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX))
    {
        return NOT_OK;
    }

    if(adecIndex == HAL_AUDIO_ADEC0)
        pFlow = Aud_flow0; //m dec0 use flow 0
    else
        pFlow = Aud_flow1;

    pFlow->Flush(pFlow);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AndroidDolbyDynamicRange(UINT32 bMode)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AndroidHEAACDynamicRange(UINT32 bMode)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AndroidMPEGAudioLevel(UINT32 bValue)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AndroidHEAACAudioLevel(UINT32 bValue)
{
    INFO("%s Enter.\n", __func__);
    return OK;
}

void SNDOutFroceMute(AUDIO_CONFIG_DEVICE deviceID, BOOLEAN muteflag, AUDIO_DVOL_LEVEL dbValue)
{
     ADSP_SNDOut_SetMute(deviceID, muteflag);
     ADSP_SNDOut_SetVolume(deviceID, dbValue);
}

SINT64 searchMachedPts(SINT32 id, RAL_AUDIO_BT_PTS_T bt_pts[4])
{
    //ERROR("[PCMCap Debug]:: [%d, %d],[%d, %d],[%d, %d],[%d, %d]\n", bt_pts[id%4].datasize, (SINT32)bt_pts[id%4].pts,bt_pts[(id+3)%4].datasize, (SINT32)bt_pts[(id+3)%4].pts,bt_pts[(id+2)%4].datasize, (SINT32)bt_pts[(id+2)%4].pts, bt_pts[(id+1)%4].datasize, (SINT32)bt_pts[(id+1)%4].pts);
    if(bt_pts[id%4].datasize== 4096)
    {
        return bt_pts[id%4].pts;
    }
    else if((bt_pts[id%4].datasize + bt_pts[(id+3)%4].datasize) == 4096)
    {
        return bt_pts[id%4].pts;
    }
    else if((bt_pts[id%4].datasize + bt_pts[(id+3)%4].datasize + bt_pts[(id+2)%4].datasize) == 4096)
    {
        return bt_pts[id%4].pts;
    }
    else if((bt_pts[id%4].datasize + bt_pts[(id+3)%4].datasize + bt_pts[(id+2)%4].datasize + bt_pts[(id+1)%4].datasize) == 4096)
    {
        return bt_pts[id%4].pts;
    }
    else
    {
        if(bt_pts[id%4].datasize > 4096)
            return bt_pts[id%4].pts;
        else
            return 0;
    }
}

SINT64 searchMachedPts2(SINT32 id, RAL_AUDIO_BT_PTS_T bt_pts[4])
{
    //ERROR("[PCMCap Debug]:: [%d, %d],[%d, %d],[%d, %d],[%d, %d]\n", bt_pts[id%4].datasize, (SINT32)bt_pts[id%4].pts,bt_pts[(id+3)%4].datasize, (SINT32)bt_pts[(id+3)%4].pts,bt_pts[(id+2)%4].datasize, (SINT32)bt_pts[(id+2)%4].pts, bt_pts[(id+1)%4].datasize, (SINT32)bt_pts[(id+1)%4].pts);
    if(bt_pts[id%4].datasize== 4096)
    {
        return bt_pts[id%4].pts;
    }
    else
    {
        return 0;
    }
}

DTV_STATUS_T RHAL_AUDIO_SIF_GetSoundStandard(HAL_AUDIO_SIF_STANDARD_T *standard)
{
    UINT32 ret = 0;
    ATV_SOUND_INFO p_sound_info;
    INFO("%s Enter.\n", __func__);
    memset(&p_sound_info, 0, sizeof(ATV_SOUND_INFO));
    ret = KADP_Audio_AtvGetSoundStd(&p_sound_info);
    if(ret != 0)
        return NOT_OK;

    switch(p_sound_info.sound_std)
    {
            //MN
        case  ATV_SOUND_STD_MN_MONO:
            *standard = HAL_AUDIO_SIF_MN_FM;
            break;
        case  ATV_SOUND_STD_BTSC:
            *standard = HAL_AUDIO_SIF_MN_BTSC;
            break;
        case  ATV_SOUND_STD_A2_M:
            *standard = HAL_AUDIO_SIF_MN_A2;
            break;
        case  ATV_SOUND_STD_EIAJ:
            *standard = HAL_AUDIO_SIF_MN_EIAJ;
            break;
            //BG
        case  ATV_SOUND_STD_BG_MONO:
            *standard = HAL_AUDIO_SIF_BG_FM;
            break;
        case  ATV_SOUND_STD_A2_BG:
            *standard = HAL_AUDIO_SIF_BG_A2;
            break;
        case  ATV_SOUND_STD_NICAM_BG:
            *standard = HAL_AUDIO_SIF_BG_NICAM;
            break;
            //DK
        case  ATV_SOUND_STD_DK_MONO:
            *standard = HAL_AUDIO_SIF_DK_FM;
            break;
        case  ATV_SOUND_STD_A2_DK1:
            *standard = HAL_AUDIO_SIF_DK1_A2;
            break;
        case  ATV_SOUND_STD_A2_DK2:
            *standard = HAL_AUDIO_SIF_DK2_A2;
            break;
        case  ATV_SOUND_STD_A2_DK3:
            *standard = HAL_AUDIO_SIF_DK3_A2;
            break;
        case  ATV_SOUND_STD_NICAM_DK:
            *standard = HAL_AUDIO_SIF_DK_NICAM;
            break;
            //L
        case  ATV_SOUND_STD_AM_MONO:
            *standard = HAL_AUDIO_SIF_L_AM;
            break;
        case  ATV_SOUND_STD_NICAM_L:
            *standard = HAL_AUDIO_SIF_L_NICAM;
            break;
            //I
        case  ATV_SOUND_STD_FM_MONO_NO_I:
            *standard = HAL_AUDIO_SIF_I_FM;
            break;
        case  ATV_SOUND_STD_NICAM_I:
            *standard = HAL_AUDIO_SIF_I_NICAM;
            break;
        case  ATV_SOUND_STD_UNKNOWN:
            *standard = HAL_AUDIO_SIF_UNKNOWN;
            break;
        default:
            ERROR("[%s] not in case... %d\n",__func__, *standard);
            return API_INVALID_PARAMS;
    }

    return OK;

}

DTV_STATUS_T RHAL_AUDIO_SIF_DetectComplete(SINT32 *isOff)
{
    UINT32 ret = 0;
    INFO("%s Enter.\n", __func__);
    ret = KADP_Audio_AtvGetCurrentDetectUpdate(isOff);
    if(ret != 0)
        return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SIF_SetOverDeviation(HAL_SIF_OVER_DEVIATION_E overDeviation)
{
	UINT32 ret = 0;
    INFO("%s Enter.\n", __func__);
	switch(overDeviation)
	{
		case HAL_SIF_OVER_DEVIATION_50K:
			KADP_Audio_AtvSetDevOnOff(FALSE);
			KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV0_120KHZ, BANDWIDTH_HDV0_120KHZ);
			break;
		case HAL_SIF_OVER_DEVIATION_100K:
			KADP_Audio_AtvSetDevOnOff(FALSE);
			KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV0_240KHZ, BANDWIDTH_HDV0_120KHZ);
			break;
		case HAL_SIF_OVER_DEVIATION_150K:
			if (gUSER_CONFIG != AUDIO_USER_CONFIG_TV001)
			{
				KADP_Audio_AtvSetDevOnOff(FALSE);
				KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV0_370KHZ, BANDWIDTH_HDV0_120KHZ);
			}
			break;
		case HAL_SIF_OVER_DEVIATION_200K:
			KADP_Audio_AtvSetDevOnOff(TRUE);
			KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV1_240KHZ, BANDWIDTH_HDV0_120KHZ);
			break;
		case HAL_SIF_OVER_DEVIATION_384K:
			KADP_Audio_AtvSetDevOnOff(TRUE);
			KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV1_340KHZ, BANDWIDTH_HDV0_120KHZ);
			break;
		case HAL_SIF_OVER_DEVIATION_462K:
			if (gUSER_CONFIG != AUDIO_USER_CONFIG_TV001)
			{
				KADP_Audio_AtvSetDevOnOff(TRUE);
				KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV1_480KHZ, BANDWIDTH_HDV0_120KHZ);
			}
			break;
		case HAL_SIF_OVER_DEVIATION_540K:
			KADP_Audio_AtvSetDevOnOff(TRUE);
			KADP_Audio_AtvSetDevBandWidth(BANDWIDTH_HDV1_740KHZ, BANDWIDTH_HDV0_120KHZ);
			break;
		case HAL_SIF_OVER_DEVIATION_BUTT:
		default:
			break;
	}
	if(ret!= 0)
		return NOT_OK;
	return OK;
}

DTV_STATUS_T RHAL_AUDIO_GetDigitalInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_DIGITAL_INFO_T *pAudioDigitalInfo)
{
    HAL_AUDIO_RESOURCE_T curResourceId = adec2res(adecIndex);
    HAL_AUDIO_ES_INFO_T AudioESInfo;
    UINT16 sampleRate;

    if(khal_debug & KHAL_DEBUG_SHOW_DIGITALINFO_)
    {
        INFO("%s Enter.\n", __func__);
        INFO("[AUDH] %s from %s\n", __func__, GetResourceString(curResourceId));
    }

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;
    if(pAudioDigitalInfo == NULL)
        return NOT_OK;
    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    memset(&AudioESInfo, 0, sizeof(HAL_AUDIO_ES_INFO_T));

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0) && adecIndex == Aud_mainDecIndex) {
        RAL_AUDIO_GetESInfo(-1, &AudioESInfo, &sampleRate);
    } else {
        RAL_AUDIO_GetESInfo(adecIndex, &AudioESInfo, &sampleRate);
    }

    pAudioDigitalInfo->adecFormat = AudioESInfo.adecFormat;
    pAudioDigitalInfo->audioMode  = AudioESInfo.audioMode;
    pAudioDigitalInfo->sampleRate = sampleRate;
    pAudioDigitalInfo->EAC3       = AudioESInfo.ac3ESInfo.EAC3;

    if(khal_debug & KHAL_DEBUG_SHOW_DIGITALINFO_)
    {
        INFO("codec %d, mode %d, fr %d, eac3 %d\n", pAudioDigitalInfo->adecFormat, pAudioDigitalInfo->audioMode, pAudioDigitalInfo->sampleRate, pAudioDigitalInfo->EAC3);
    }
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_DBXTV_SET_TABLE(UINT32* table, UINT32 size)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 ret = 0;
    INFO("%s Enter.\n", __func__);

    if (LITE_HAL_INIT_OK() != TRUE){
    	ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
    	return NOT_OK;
    }

    if (dbxTableAddr == NULL) {
        #if 0
        dbxTableAddr = RTKAudio_Malloc(size, (BYTE **)&dbxTableNonCacheAddr, (UINT32*)&dbxTablePhyAddr);
        #else
        dbxTablePhyAddr = rtkaudio_alloc_uncached(size, &dbxTableAddr);
        if (dbxTableAddr == NULL) {
        	ERROR("[AUDH][%s %d] rtkaudio_alloc_uncached dbx table fail\n",__func__,__LINE__);
        	return NOT_OK;
        }
        #endif
    }

    memcpy(dbxTableAddr, table, size);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_DBX_TAB_CONFIG;
    audioConfig.value[0] = dbxTablePhyAddr;
    audioConfig.value[1] = size;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);

    if(ret != 0){
    	ERROR("[AUDH][%s %d] KADP_AUDIO_DBXTV_SetTable fail\n",__func__,__LINE__);
    	return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_DBXTV_SET_SOUND_EFFECT(HAL_AUDIO_DBXTV_PARAM_TYPE paramType, UINT8 mode)
{

    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 ret = 0;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE){
        ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
        return NOT_OK;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id  = AUDIO_CONFIG_CMD_DBX_CONFIG;
    audioConfig.value[0] = paramType;
    audioConfig.value[1] = mode;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);
    if(ret != 0){
        ERROR("[AUDH][error] [%s %d] KADP_AUDIO_DBXTV_SetSoundEffect fail\n",__func__,__LINE__);
        return NOT_OK;
    }
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_DBXTV_DEBUG_CMD(UINT32 cmd, UINT32* param, UINT32 paramSize)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    UINT32 ret = 0;
    INFO("%s Enter.\n", __func__);

    if(LITE_HAL_INIT_OK() != TRUE){
        ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
        return NOT_OK;
    }

    if (paramSize <= 0 || paramSize > (sizeof(audioConfig.value)-4)) { //audioConfig.value[1]~value[14]
        ERROR("[AUDH][%s %d] paramSize <= 0\n",__func__,__LINE__);
        return NOT_OK;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_DBX_DEBUG;
    audioConfig.value[0] = cmd;
    memcpy(&audioConfig.value[1], param, paramSize);

    ret = KADP_AUDIO_AudioConfig(&audioConfig);
    if(ret != 0){
        ERROR("[AUDH][error] [%s %d] KADP_AUDIO_AudioConfig fail\n",__func__,__LINE__);
        return NOT_OK;
    }

    memcpy(param, &audioConfig.value[1], paramSize);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SwpSetSRS_TrusurroundHD(BOOLEAN para, HAL_APP_TSXT_CFG* pTSXT)
{
    AUDIO_RPC_TSXT info;
    INFO("%s Enter.\n", __func__);
    if(NULL == pTSXT) return NOT_OK;
    if(pTSXT->ModeType < 0 || pTSXT->ModeType > 11) return NOT_OK;
    if(pTSXT->TruBassGain < 0 || pTSXT->TruBassGain > 100) return NOT_OK;
    if(pTSXT->TruBassSize < 0 || pTSXT->TruBassSize > 7) return NOT_OK;
    if(pTSXT->FocusGain < 0 || pTSXT->FocusGain > 100) return NOT_OK;
    if(pTSXT->InputGain < 0 || pTSXT->InputGain > 100) return NOT_OK;
    if(pTSXT->IsTSXTEnable != 0 && pTSXT->IsTSXTEnable !=1) return NOT_OK;
    if(pTSXT->IsHeadphoneMode != 0 && pTSXT->IsHeadphoneMode !=1) return NOT_OK;
    if(pTSXT->IsTruBassEnable != 0 && pTSXT->IsTruBassEnable !=1) return NOT_OK;
    if(pTSXT->IsFocusEnable != 0 && pTSXT->IsFocusEnable !=1) return NOT_OK;
    if(pTSXT->DefinitionGain < 0 || pTSXT->DefinitionGain > 100) return NOT_OK;
    if(pTSXT->IsDefEnable != 0 && pTSXT->IsDefEnable !=1) return NOT_OK;
    //#ifdef ENABLE_SRS_SURROUND // gene modify => default enable it

    info.instanceID = -1; // do not care
    info.pp_Tsxt_Enable = para;

    info.ModeType = (UINT8 ) (pTSXT->ModeType);
    info.TruBassGain = (UINT8 ) pTSXT->TruBassGain;
    info.TruBassSize =(UINT8 )  pTSXT->TruBassSize;
    info.FocusGain = (UINT8 ) pTSXT->FocusGain;
    info.InputGain = (UINT8 ) pTSXT->InputGain;
    info.IsTSXTEnable = pTSXT->IsTSXTEnable;
    info.IsHeadphoneMode = (UINT8 ) pTSXT->IsHeadphoneMode;
    info.IsTruBassEnable = (UINT8 ) pTSXT->IsTruBassEnable;
    info.IsFocusEnable = (UINT8 ) pTSXT->IsFocusEnable;
    info.DefinitionGain = (UINT8 ) pTSXT->DefinitionGain;
    info.IsDefEnable = (UINT8 ) pTSXT->IsDefEnable;

    if (KADP_AUDIO_SwpSetSRS_TrusurroundHD(&info) != KADP_OK)
    {
        ERROR("[%s,%d] failed\n", __func__, __LINE__);
        return NOT_OK;
    }

#if 0
	SWP_SET_INFO	swp_info;
	HAL_APP_TSXT_CFG	t_tsxt_cfg;

	/*
	 * In this version, TrusurroundHD is implemented by TrusurroundXT upgrade version.
	 */
	swp_info.on_off   = para;
	swp_info.type     = SWP_CMD_SET_SRS_TSXT;
	swp_info.sub_type = type_id;

	if(p_cfg_tshd == NULL) {
		t_tsxt_cfg.ModeType = 8;//TSXT_kPassiveMatrix;
		t_tsxt_cfg.InputGain = 100;// 85;
		t_tsxt_cfg.FocusGain = 100;//30;
		t_tsxt_cfg.TruBassGain = 100;//20;
		t_tsxt_cfg.TruBassSize = 3;//TSXT_kSpeakerLFResponse150Hz;// 100Hz
		t_tsxt_cfg.IsTSXTEnable = 1;
		t_tsxt_cfg.IsFocusEnable = 1;
		t_tsxt_cfg.IsTruBassEnable = 1;
		t_tsxt_cfg.IsHeadphoneMode = 0;
		t_tsxt_cfg.DefinitionGain = 40;
		t_tsxt_cfg.IsDefEnable = 1;
		t_tsxt_cfg.Volume_compensate = 0;

		swp_info.p_coef = (void*)&t_tsxt_cfg;
	}else {
		swp_info.p_coef = p_cfg_tshd;
	}

	ioctl(audio_fd, SWP_CMD_SET_SRS_TSXT, (void*)&swp_info);
#endif

	return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetInputVolume(HAL_AUDIO_INPUT_SOURCE_T eSource, SINT32 sVol)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    SINT32 volume = 0, ch = 0;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDIO_HAL] %s %d\n", __func__, sVol);

    if(HAL_AUDIO_MIC_IN == eSource)
    {
        /* SetAudioInVolume(TRUE, vol, ENUM_AI_PB_PATH); */
        volume = AIN_VOL_0DB + (sVol - 30);
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_AI_VOLUME;
        audioConfig.value[0] = TRUE;
        audioConfig.value[1] = ENUM_AI_PB_PATH;

        for(ch=0;ch<8;ch++)
        {
            audioConfig.value[2 + ch] = volume;
        }

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] set ai volume failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }
    else if(HAL_AUDIO_OTHER_IN == eSource)
    {
        /*SetAudioDecVolume((AUDIO_DVOL_LEVEL) volume);*/
        volume = ENUM_AUDIO_DVOL_K0p0DB + (sVol - 30);
        memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
        audioConfig.msg_id = AUDIO_CONFIG_CMD_AO_DEC_VOLUME;
        audioConfig.value[0] = volume;

        if (SendRPC_AudioConfig(&audioConfig) != S_OK)
        {
            ERROR("[%s,%d] set ai volume failed\n", __func__, __LINE__);
            return NOT_OK;
        }
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_setTrackMode(HAL_TRACK_MODE eMode)
{
    AUDIO_AO_CHANNEL_OUT_SWAP mode;

    INFO("%s Enter.\n", __func__);
    INFO("[AUDIO_HAL] %s eMode %d \n", __func__,eMode);

    switch (eMode)
    {
        case  HAL_AUDIO_AO_CHANNEL_OUT_STEREO:
            mode = AUDIO_AO_CHANNEL_OUT_STEREO;
            break;
        case HAL_AUDIO_AO_CHANNEL_OUT_LR_MIXED:
            mode = AUDIO_AO_CHANNEL_OUT_LR_MIXED;
            break;
        case HAL_AUDIO_AO_CHANNEL_OUT_LR_SWAP:
            mode = AUDIO_AO_CHANNEL_OUT_LR_SWAP;
            break;
        case HAL_AUDIO_AO_CHANNEL_OUT_R_TO_L:
            mode = AUDIO_AO_CHANNEL_OUT_R_TO_L;
            break;
        case HAL_AUDIO_AO_CHANNEL_OUT_L_TO_R:
            mode = AUDIO_AO_CHANNEL_OUT_L_TO_R;
            break;
        default:
            break;
    }

    if(KADP_AUDIO_SetAudioOptChannelSwap(mode) != KADP_OK)
    {
        return NOT_OK;
    }

    return OK;
}

AUDIO_CONFIG_CMD_MSG ConvertConfigType(HAL_AUDIO_CONFIG_CMD_MSG msgID)
{
    AUDIO_CONFIG_CMD_MSG type;

    switch (msgID)
    {
        case HAL_AUDIO_CONFIG_CMD_SPEAKER:
            type = AUDIO_CONFIG_CMD_SPEAKER;
            break;
        case HAL_AUDIO_CONFIG_CMD_AGC:
            type = AUDIO_CONFIG_CMD_AGC;
            break;
        case HAL_AUDIO_CONFIG_CMD_SPDIF:
            type = AUDIO_CONFIG_CMD_SPDIF;
            break;
        case HAL_AUDIO_CONFIG_CMD_VOLUME:
            type = AUDIO_CONFIG_CMD_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_MUTE:
            type = AUDIO_CONFIG_CMD_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_MIC:
            type = AUDIO_CONFIG_CMD_AO_MIC;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_AUX:
            type = AUDIO_CONFIG_CMD_AO_AUX;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_ECHO_REVERB:
            type = AUDIO_CONFIG_CMD_AO_ECHO_REVERB;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_MIC1_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_MIC1_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_MIC2_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_MIC2_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_DEC_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_AUX_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_AUX_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_DAC_SWITCH:
            type = AUDIO_CONFIG_CMD_DAC_SWITCH;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_DUALMONO:
            type = AUDIO_CONFIG_CMD_DD_DUALMONO;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_SCALE:
            type = AUDIO_CONFIG_CMD_DD_SCALE;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_DOWNMIXMODE:
            type = AUDIO_CONFIG_CMD_DD_DOWNMIXMODE;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_COMP:
            type = AUDIO_CONFIG_CMD_DD_COMP;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_LFE:
            type = AUDIO_CONFIG_CMD_DD_LFE;
            break;
        case HAL_AUDIO_CONFIG_CMD_DD_STEREOMIX:
            type = AUDIO_CONFIG_CMD_DD_STEREOMIX;
            break;
        case HAL_AUDIO_CONFIG_CMD_DIGITAL_OUT:
            type = AUDIO_CONFIG_CMD_DIGITAL_OUT;
            break;
        case HAL_AUDIO_CONFIG_CMD_EXCLUSIVE:
            type = AUDIO_CONFIG_CMD_EXCLUSIVE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AC3_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_AC3_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_AC3_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_AC3_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_DTS_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_DTS_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_DTS_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_DTS_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_MPG_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_MPG_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_MPG_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_MPG_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_AAC_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_AAC_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_AAC_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_AAC_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_MLP_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_MLP_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_DDP_HDMI_RAW:
            type = AUDIO_CONFIG_CMD_DDP_HDMI_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_HDMI_CHANNEL_OUT:
            type = AUDIO_CONFIG_CMD_HDMI_CHANNEL_OUT;
            break;
        case HAL_AUDIO_CONFIG_CMD_FORCE_CHANNEL_CODEC:
            type = AUDIO_CONFIG_CMD_FORCE_CHANNEL_CODEC;
            break;
        case HAL_AUDIO_CONFIG_CMD_MLP_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_MLP_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_DDP_SPDIF_RAW:
            type = AUDIO_CONFIG_CMD_DDP_SPDIF_RAW;
            break;
        case HAL_AUDIO_CONFIG_CMD_MAX_OUTPUT_SAMPLERATE:
            type = AUDIO_CONFIG_CMD_MAX_OUTPUT_SAMPLERATE;
            break;
        case HAL_AUDIO_CONFIG_CMD_USB_DEVICE_SUPPORT_SAMPLERATE:
            type = AUDIO_CONFIG_CMD_USB_DEVICE_SUPPORT_SAMPLERATE;
            break;
        case HAL_AUDIO_CONFIG_CMD_KARAOKE_MODE:
            type = AUDIO_CONFIG_CMD_KARAOKE_MODE;
            break;
        case HAL_AUDIO_CONFIG_CMD_BRAZIL_LATM_AAC:
            type = AUDIO_CONFIG_CMD_BRAZIL_LATM_AAC;
            break;
        case HAL_AUDIO_CONFIG_CMD_DRC_PERCENT:
            type = AUDIO_CONFIG_CMD_DRC_PERCENT;
            break;
        case HAL_AUDIO_CONFIG_CMD_LICENSE_MODE:
            type = AUDIO_CONFIG_CMD_LICENSE_MODE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AUDIO_TYPE_PRIORITY:
            type = AUDIO_CONFIG_CMD_AUDIO_TYPE_PRIORITY;
            break;
        case HAL_AUDIO_CONFIG_CMD_SPDIF_OUTPUT_SWITCH:
            type = AUDIO_CONFIG_CMD_SPDIF_OUTPUT_SWITCH;
            break;
        case HAL_AUDIO_CONFIG_CMD_CUSTOMIZE_REQUEST:
            type = AUDIO_CONFIG_CMD_CUSTOMIZE_REQUEST;
            break;
        case HAL_AUDIO_CONFIG_CMD_DEC_PLAYBACK_RATE:
            type = AUDIO_CONFIG_CMD_DEC_PLAYBACK_RATE;
            break;
        case HAL_AUDIO_CONFIG_CMD_SAMPLERATE_TOLERANCE:
            type = AUDIO_CONFIG_CMD_SAMPLERATE_TOLERANCE;
            break;
        case HAL_AUDIO_CONFIG_CMD_KARAOKE_GENDER_CHANGE:
            type = AUDIO_CONFIG_CMD_KARAOKE_GENDER_CHANGE;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_DEC_DELAY_RP:
            type = AUDIO_CONFIG_CMD_SET_DEC_DELAY_RP;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_OUTPUT_CONFIG:
            type = AUDIO_CONFIG_CMD_AO_OUTPUT_CONFIG;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_AI_VOLUME:
            type = AUDIO_CONFIG_CMD_SET_AI_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MUTE:
            type = AUDIO_CONFIG_CMD_AO_DEC_MUTE;
            break;
        case HAL_AUDIO_CONFIG_TFAGC_DEBUG_PRINT:
            type = AUDIO_CONFIG_TFAGC_DEBUG_PRINT;
            break;
        case HAL_AUDIO_CONFIG_ASF_DEBUG_PRINT:
            type = AUDIO_CONFIG_ASF_DEBUG_PRINT;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_OUTPUT_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_OUTPUT_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_KARAOKE_MODE_FORTV_PROJECT:
            type = AUDIO_CONFIG_CMD_KARAOKE_MODE_FORTV_PROJECT;
            break;
        case HAL_AUDIO_CONFIG_CMD_AVOUT_FROM_HW:
            type = AUDIO_CONFIG_CMD_AVOUT_FROM_HW;
            break;
        case HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_AIO1:
            type = AUDIO_CONFIG_CMD_CONFIG_AIO_AIO1;
            break;
        case HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_AIO2:
            type = AUDIO_CONFIG_CMD_CONFIG_AIO_AIO2;
            break;
        case HAL_AUDIO_CONFIG_CMD_CONFIG_AIO_HEADPHONE:
            type = AUDIO_CONFIG_CMD_CONFIG_AIO_HEADPHONE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_MUTE:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_SPDIF_CS_INFO:
            type = AUDIO_CONFIG_CMD_SET_SPDIF_CS_INFO;
            break;
        case HAL_AUDIO_CONFIG_CMD_HDMI_ARC:
            type = AUDIO_CONFIG_CMD_HDMI_ARC;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_MUTE:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_PB_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_MUTE:
            type = AUDIO_CONFIG_CMD_AO_DEC_MULTI_CH_TS_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_FLASH_PB_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_FLASH_PB_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_FLASH_PB_MUTE:
            type = AUDIO_CONFIG_CMD_AO_FLASH_PB_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_FLASH_TS_VOLUME:
            type = AUDIO_CONFIG_CMD_AO_FLASH_TS_VOLUME;
            break;
        case HAL_AUDIO_CONFIG_CMD_AO_FLASH_TS_MUTE:
            type = AUDIO_CONFIG_CMD_AO_FLASH_TS_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AI_CEI_ENABLE:
            type = AUDIO_CONFIG_CMD_AI_CEI_ENABLE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AI_CEI_SETMODE:
            type = AUDIO_CONFIG_CMD_AI_CEI_SETMODE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AD_DESCRIPTOR:
            type = AUDIO_CONFIG_CMD_AD_DESCRIPTOR;
            break;
        case HAL_AUDIO_CONFIG_CMD_DEFAULT_DIALNORM:
            type = AUDIO_CONFIG_CMD_DEFAULT_DIALNORM;
            break;
        case HAL_AUDIO_CONFIG_CMD_AI_CEI_SET_PARAMETER:
            type = AUDIO_CONFIG_CMD_AI_CEI_SET_PARAMETER;
            break;
        case HAL_AUDIO_CONFIG_CMD_AI_CEI_GET_PARAMETER:
            type = AUDIO_CONFIG_CMD_AI_CEI_GET_PARAMETER;
            break;
        case HAL_AUDIO_CONFIG_CMD_AI_TRANSFER_MATRIX:
            type = AUDIO_CONFIG_CMD_AI_TRANSFER_MATRIX;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_VOLUME:
            type = AUDIO_CONFIG_CMD_SET_VOLUME;
            break;
        /*case 79:
            type = 79;
            break;*/
        case HAL_AUDIO_CONFIG_CMD_SET_MUTE:
            type = AUDIO_CONFIG_CMD_SET_MUTE;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_DELAY:
            type = AUDIO_CONFIG_CMD_SET_DELAY;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_SCART_ON:
            type = AUDIO_CONFIG_CMD_SET_SCART_ON;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_NO_DELAY_PARAM:
            type = AUDIO_CONFIG_CMD_SET_NO_DELAY_PARAM;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_SPDIF_CS_ALL:
            type = AUDIO_CONFIG_CMD_SET_SPDIF_CS_ALL;
            break;
        case HAL_AUDIO_CONFIG_CMD_ARC:
            type = AUDIO_CONFIG_CMD_ARC;
            break;
        case HAL_AUDIO_CONFIG_CMD_NTFY_HAL_HDMI_FMT:
            type = AUDIO_CONFIG_CMD_NTFY_HAL_HDMI_FMT;
            break;
        case HAL_AUDIO_CONFIG_CMD_AANDROID_DOLBY_DYN_RANGE:
            type = AUDIO_CONFIG_CMD_AANDROID_DOLBY_DYN_RANGE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AANDROID_HEAAC_DYN_RANGE:
            type = AUDIO_CONFIG_CMD_AANDROID_HEAAC_DYN_RANGE;
            break;
        case HAL_AUDIO_CONFIG_CMD_AANDROID_MPEG_AUD_LEVEL:
            type = AUDIO_CONFIG_CMD_AANDROID_MPEG_AUD_LEVEL;
            break;
        case HAL_AUDIO_CONFIG_CMD_AANDROID_HEAAC_AUD_LEVEL:
            type = AUDIO_CONFIG_CMD_AANDROID_HEAAC_AUD_LEVEL;
            break;
        case HAL_AUDIO_CONFIG_CMD_SET_PCM_INFO:
            type = AUDIO_CONFIG_CMD_SET_PCM_INFO;
            break;
        case HAL_AUDIO_CONFIG_CMD_GET_TASK_STARTED:
            type = AUDIO_CONFIG_CMD_GET_TASK_STARTED;
            break;
        case HAL_AUDIO_CONFIG_CMD_DBX_DEBUG:
            type = AUDIO_CONFIG_CMD_DBX_DEBUG;
            break;
        case HAL_AUDIO_CONFIG_CMD_DBX_CONFIG:
            type = AUDIO_CONFIG_CMD_DBX_CONFIG;
            break;
        case HAL_AUDIO_CONFIG_CMD_DBX_TAB_CONFIG:
            type = AUDIO_CONFIG_CMD_DBX_TAB_CONFIG;
            break;
        default:
            ERROR("[AUDH] unknown Config MsgID %d\n", msgID);
            type = (AUDIO_CONFIG_CMD_MSG) -1;
            break;
    }

    if (((u_int)type != (u_int)msgID) || (type == (AUDIO_CONFIG_CMD_MSG) -1))
    {
        ERROR("[AUDH] MsgID didn't match (%d,%d)\n", type, msgID);
        return (AUDIO_CONFIG_CMD_MSG) -1;
    }

    return type;
}

DTV_STATUS_T RHAL_AUDIO_SendConfig(HAL_AUDIO_CONFIG_COMMAND_T *audioConfig)
{
    SINT32 i;
    AUDIO_CONFIG_COMMAND_RTKAUDIO sendconfig;
    INFO("%s Enter.\n", __func__);

    if (audioConfig == NULL)
    {
        ERROR("[AUDH] %d Pointer is NULL\n", __LINE__);
        return NOT_OK;
    }

    sendconfig.msg_id = ConvertConfigType(audioConfig->msgID);

    if (sendconfig.msg_id == (AUDIO_CONFIG_CMD_MSG) -1)
    {
        ERROR("[AUDH] msdID didn't match\n");
        return NOT_OK;
    }

    for (i = 0; i < 15; i++)
    {
        sendconfig.value[i] = audioConfig->value[i];
    }

    if (SendRPC_AudioConfig(&sendconfig) != S_OK)
    {
        ERROR("[AUDH] %d AudioConfig is Failed\n", __LINE__);
        return NOT_OK;
    }

    return OK;
}

AUDIO_ENUM_PRIVAETINFO ConvertPrivateType(HAL_AUDIO_ENUM_PRIVAETINFO infotype)
{
    AUDIO_ENUM_PRIVAETINFO type;

    switch(infotype)
    {
        case HAL_PRIVATEINFO_AUDIO_FORMAT_PARSER_CAPABILITY:
            type = ENUM_PRIVATEINFO_AUDIO_FORMAT_PARSER_CAPABILITY;
            break;
        case HAL_PRIVATEINFO_AUDIO_DECODER_CAPABILITY:
            type = ENUM_PRIVATEINFO_AUDIO_DECODER_CAPABILITY;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONFIG_CMD_BS_INFO:
            type = ENUM_PRIVATEINFO_AUDIO_CONFIG_CMD_BS_INFO;
            break;
        case HAL_PRIVATEINFO_AUDIO_CHECK_LPCM_ENDIANESS:
            type = ENUM_PRIVATEINFO_AUDIO_CHECK_LPCM_ENDIANESS;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONFIG_CMD_AO_DELAY_INFO:
            type = ENUM_PRIVATEINFO_AUDIO_CONFIG_CMD_AO_DELAY_INFO;
            break;
        case HAL_PRIVATEINFO_AUDIO_AO_CHANNEL_VOLUME_LEVEL:
            type = ENUM_PRIVATEINFO_AUDIO_AO_CHANNEL_VOLUME_LEVEL;
            break;
        case HAL_PRIVATEINFO_AUDIO_GET_FLASH_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_GET_FLASH_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_RELEASE_FLASH_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_RELEASE_FLASH_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_GET_MUTE_N_VOLUME:
            type = ENUM_PRIVATEINFO_AUDIO_GET_MUTE_N_VOLUME;
            break;
        case HAL_PRIVATEINFO_AUDIO_AO_MONITOR_FULLNESS:
            type = ENUM_PRIVATEINFO_AUDIO_AO_MONITOR_FULLNESS;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONTROL_FLASH_VOLUME:
            type = ENUM_PRIVATEINFO_AUDIO_CONTROL_FLASH_VOLUME;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONTROL_DAC_SWITCH:
            type = ENUM_PRIVATEINFO_AUDIO_CONTROL_DAC_SWITCH;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONTROL_AO_GIVE_UP_SET_CLOCK:
            type = ENUM_PRIVATEINFO_AUDIO_CONTROL_AO_GIVE_UP_SET_CLOCK;
            break;
        case HAL_PRIVATEINFO_AUDIO_DEC_DELAY_RP:
            type = ENUM_PRIVATEINFO_AUDIO_DEC_DELAY_RP;
            break;
        case HAL_PRIVATEINFO_AUDIO_AI_SET_OUT_FMT:
            type = ENUM_PRIVATEINFO_AUDIO_AI_SET_OUT_FMT;
            break;
        case HAL_PRIVATEINFO_AUDIO_AI_SWITCH_FOCUS:
            type = ENUM_PRIVATEINFO_AUDIO_AI_SWITCH_FOCUS;
            break;
        case HAL_PRIVATEINFO_AUDIO_SUPPORT_SAMPLERATE:
            type = ENUM_PRIVATEINFO_AUDIO_SUPPORT_SAMPLERATE;
            break;
        case HAL_PRIVATEINFO_AUDIO_GET_AVAILABLE_AO_OUTPUT_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_GET_AVAILABLE_AO_OUTPUT_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_GET_PCM_IN_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_GET_PCM_IN_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_RELEASE_PCM_IN_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_RELEASE_PCM_IN_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_FIRMWARE_CAPABILITY:
            type = ENUM_PRIVATEINFO_AUDIO_FIRMWARE_CAPABILITY;
            break;
        case HAL_PRIVATEINFO_AUDIO_SRC_PROCESS_DONE:
            type = ENUM_PRIVATEINFO_AUDIO_SRC_PROCESS_DONE;
            break;
        case HAL_PRIVATEINFO_AUDIO_DEC_SRC_ENABLE:
            type = ENUM_PRIVATEINFO_AUDIO_DEC_SRC_ENABLE;
            break;
        case HAL_PRIVATEINFO_AUDIO_QUERY_FLASH_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_QUERY_FLASH_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_SET_FLASH_PIN:
            type = ENUM_PRIVATEINFO_AUDIO_SET_FLASH_PIN;
            break;
        case HAL_PRIVATEINFO_AUDIO_SET_SYSTEM_PROCESS_PID:
            type = ENUM_PRIVATEINFO_AUDIO_SET_SYSTEM_PROCESS_PID;
            break;
        case HAL_PRIVATEINFO_AUDIO_CONFIG_VIRTUALX_PARAM:
            type = ENUM_PRIVATEINFO_AUDIO_CONFIG_VIRTUALX_PARAM;
            break;
        case HAL_PRIVATEINFO_AUDIO_GET_CURR_AO_VOLUME:
            type = ENUM_PRIVATEINFO_AUDIO_GET_CURR_AO_VOLUME;
            break;
        default:
            ERROR("[AUDH] unknown private info type %d\n", infotype);
            type = (AUDIO_ENUM_PRIVAETINFO) -1;
            break;
    }

    if (((u_int)type != (u_int)infotype) || (type == (AUDIO_ENUM_PRIVAETINFO) -1))
    {
        ERROR("[AUDH] private type didn't match (%d,%d)\n", type, infotype);
        return (AUDIO_ENUM_PRIVAETINFO) -1;
    }

    return type;
}

DTV_STATUS_T RHAL_AUDIO_PrivateInfo(HAL_AUDIO_RPC_PRIVATEINFO_PARAMETERS_T *parameter, HAL_AUDIO_RPC_PRIVATEINFO_RETURNVAL_T *ret)
{
    SINT32 i;
    AUDIO_RPC_PRIVATEINFO_PARAMETERS rpc_parameter;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL rpc_ret;
    INFO("%s Enter.\n", __func__);
    if ((parameter == NULL) || (ret == NULL))
    {
        ERROR("[AUDH] %d Pointer is NULL\n", __LINE__);
        return NOT_OK;
    }

    rpc_parameter.instanceID = parameter->instanceID;
    rpc_parameter.type = ConvertPrivateType(parameter->type);

    if ((u_int)rpc_parameter.type == (u_int)(AUDIO_CONFIG_CMD_MSG) -1)
    {
        ERROR("[AUDH] private type didn't match\n");
        return NOT_OK;
    }

    for (i = 0; i < 16; i++) {
        rpc_parameter.privateInfo[i] = parameter->privateInfo[i];
    }

    if (KADP_AUDIO_PrivateInfo(&rpc_parameter, &rpc_ret) != KADP_OK)
    {
        ERROR("[AUDH] %d PrivateInfo is Failed\n", __LINE__);
        return NOT_OK;
    }

    ret->instanceID = rpc_ret.instanceID;
    for (i = 0; i < 16; i++) {
        ret->privateInfo[i] = rpc_ret.privateInfo[i];
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAudioSpectrumData(HAL_AUDIO_SPECTRUM_CFG_T *config)
{
    AUDIO_SPECTRUM_CFG spec_config;
    INFO("%s Enter.\n", __func__);
    if (config == NULL)
    {
        ERROR("[AUDH] %d Pointer is NULL\n", __LINE__);
        return NOT_OK;
    }

    spec_config.enable = config->enable;
    spec_config.bandnum = config->bandnum;
    spec_config.data_addr = config->data_addr;
    spec_config.upper_bandlimit_addr = config->upper_bandlimit_addr;

    if (KADP_AUDIO_SetAudioSpectrumData(&spec_config) != KADP_OK)
    {
        ERROR("[AUDH] %d AudioSpectrumData is Failed\n", __LINE__);
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetModelHash(char *pmodel_hash)
{
    AUDIO_DUMMY_DATA model_hash;
    INFO("%s Enter.\n", __func__);

    if (pmodel_hash == NULL)
    {
        ERROR("[AUDH] %d Pointer is NULL\n", __LINE__);
        return NOT_OK;
    }

    memcpy(&model_hash, pmodel_hash, 32);

    if (KADP_AUDIO_SetModelHash(model_hash) != KADP_OK)
    {
        ERROR("[AUDH] SetModelHash is Failed\n");
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAudioAuthorityKey(SLONG customer_key)
{
    INFO("%s Enter.\n", __func__);
    if (KADP_AUDIO_SetAudioAuthorityKey(customer_key) != KADP_OK)
    {
        ERROR("[AUDH] %d SetAudioAuthorityKey is Failed\n", __LINE__);
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAudioCapability(AUDIO_CAP cap)
{
    AUDIO_LICENSE_STATUS_CONFIG license_config;
    AUDIO_DUMMY_DATA model_hash;
    INFO("%s Enter.\n", __func__);

    license_config.HighWord = cap.capability;
    memcpy(license_config.LowWord, cap.auth_key, sizeof(cap.auth_key));
    memcpy(&model_hash, &cap.devhash, 32);

    if (KADP_AUDIO_SetAudioCapability(license_config) != KADP_OK)
    {
        ERROR("[AUDH] SetAudioCapability is Failed\n");
        return NOT_OK;
    }

    if (KADP_AUDIO_SetModelHash(model_hash) != KADP_OK)
    {
        ERROR("[AUDH] SetAudioCapability is Failed\n");
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAVSyncOffset(HAL_AUDIO_HDMI_OUT_VSDB_DATA_T *info)
{
    AUDIO_HDMI_OUT_VSDB_DATA data;
    INFO("%s Enter.\n", __func__);

    if (info == NULL)
    {
        ERROR("[AUDH] %d Pointer is NULL\n", __LINE__);
        return NOT_OK;
    }

    data.HDMI_VSDB_delay = info->HDMI_VSDB_delay;
    if (KADP_AUDIO_SetAVSyncOffset(&data) != KADP_OK)
    {
        ERROR("[AUDH] %d SetAVSyncOffset is Failed\n", __LINE__);
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetVolumeWithFade(HAL_AUDIO_VOLUME_T volume, UINT8 duration, UINT8 easing)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    SINT32 dsp_gain;
    UINT32 i;

    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    INFO("[AUDH] %s = main %d fine %d\n", __func__, volume.mainVol, volume.fineVol);

    dsp_gain = Volume_to_DSPGain(volume);
    INFO("[AUDH] Spk volume index = %d \n", dsp_gain);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_VOLUME;
    audioConfig.value[0] = ((PPAO*)Aud_ppAout->pDerivedObj)->GetAOAgentID(Aud_ppAout);
    audioConfig.value[1] = ENUM_DEVICE_SPEAKER;
    audioConfig.value[2] = 0;
    audioConfig.value[3] = 0xF0 | (easing & 0x0F);
    audioConfig.value[4] = duration;
    for(i = 1; i < 8; i++)
    {
        audioConfig.value[4+i] = dsp_gain;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        return NOT_OK;
    }

    return OK;
}


DTV_STATUS_T RHAL_AUDIO_SPDIF_SetEnable(BOOLEAN bOnOff)
{
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    if(TRUE == bOnOff)
        KADP_AUDIO_SetSpdifOutPinSrc(SPDIFO_SRC_FIFO);
    else
        KADP_AUDIO_SetSpdifOutPinSrc(SPDIFO_SRC_DISABLE);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetDTSDecoderParam(HAL_AUDIO_DTS_DEC_PARAM_TYPE paramType, UINT32 paramValue)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE) return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    switch(paramType)
    {
    case HAL_AUDIO_DTS_DEC_DRC:
        audioConfig.msg_id = AUDIO_CONFIG_CMD_DTS_DEC_PARAM;
        audioConfig.value[0] = AUDIO_DTS_DEC_DRC;
        audioConfig.value[1] = paramValue;
        break;
    case HAL_AUDIO_DTS_DEC_LFEMIX:
        audioConfig.msg_id = AUDIO_CONFIG_CMD_DTS_DEC_PARAM;
        audioConfig.value[0] = AUDIO_DTS_DEC_LFEMIX;
        audioConfig.value[1] = paramValue;
        break;
    default:
        return NOT_OK;
    }

    if(SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetTruVolume(BOOLEAN para, HAL_AUDIO_TRUVOLUME_MODE_T* pTruVolume)
{
    AUDIO_RPC_TRUVOLUME info;
    INFO("%s Enter.\n", __func__);

    if (pTruVolume == NULL) {
        return NOT_OK;
    }

    info.instanceID = -1; /* do not care */

    info.pp_TruVol_Enable = para;
    info.pp_TruVol_Info.enable = pTruVolume->enable;
    info.pp_TruVol_Info.InputGain = pTruVolume->InputGain;
    info.pp_TruVol_Info.OutputGain = pTruVolume->OutputGain;
    info.pp_TruVol_Info.BypassGain = pTruVolume->BypassGain;
    info.pp_TruVol_Info.mode = pTruVolume->mode;
    info.pp_TruVol_Info.MaxGain = pTruVolume->MaxGain;
    info.pp_TruVol_Info.NormalizerEnable = pTruVolume->NormalizerEnable;
    info.pp_TruVol_Info.BlockSize = pTruVolume->BlockSize;

    if (KADP_AUDIO_SetTruVolume(&info) != KADP_OK)
    {
        ERROR("[%s,%d] failed\n", __func__, __LINE__);
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_GetESInfo(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_ES_INFO_T *pAudioESInfo)
{
    HAL_AUDIO_RESOURCE_T curResourceId = adec2res(adecIndex);
    UINT16 sampleRate;
    //INFO("[AUDH] %s from %s\n", __func__, GetResourceString(curResourceId));

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(pAudioESInfo == NULL)
        return NOT_OK;

    if((adecIndex != HAL_AUDIO_ADEC0) && (adecIndex != HAL_AUDIO_ADEC1))
        return NOT_OK;

    if(AUDIO_HAL_CHECK_STOP_NOTAVAILABLE(ResourceStatus[curResourceId], 0)) //decoder can only have one input
    {
        INFO("[AUDH] %s is not at run state %d\n",
                GetResourceString(curResourceId), ResourceStatus[curResourceId].connectStatus[0]);
        return NOT_OK;
    }

    RAL_AUDIO_GetESInfo(adecIndex,pAudioESInfo,&sampleRate);

    return OK;  //OK or NOT_OK
}

void* dap_param_malloc(HAL_AUDIO_DAP_PARAM_TYPE paramType, int paramValueSize, AUDIO_RPC_PRIVATEINFO_PARAMETERS *parameter)
{
    BYTE *nonCachedAddr;
    UINT8 *p_return = NULL;
    UINT32 p_addr;

    memset(parameter, 0, sizeof(struct AUDIO_RPC_PRIVATEINFO_PARAMETERS));

    if(paramType == HAL_AUDIO_SET_DAP_ENABLE || paramType == HAL_AUDIO_GET_DAP_ENABLE){
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_ENABLE) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_ENABLE : ENUM_PRIVATEINFO_AUDIO_GET_DAP_ENABLE;
    } else if(paramType == HAL_AUDIO_SET_DAP_PARAM || paramType == HAL_AUDIO_GET_DAP_PARAM){
        if(paramValueSize != sizeof(HAL_DAP_PARAM)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_PARAM) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_PARAM : ENUM_PRIVATEINFO_AUDIO_GET_DAP_PARAM;
    } else if(paramType == HAL_AUDIO_SET_DAP_DIALOGUE_ENHANCER || paramType == HAL_AUDIO_GET_DAP_DIALOGUE_ENHANCER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_DIALOGUE_ENHANCER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_DIALOGUE_ENHANCER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_DIALOGUE_ENHANCER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_DIALOGUE_ENHANCER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_DIALOGUE_ENHANCER;
    } else if(paramType == HAL_AUDIO_SET_DAP_VOLUME_LEVELER || paramType == HAL_AUDIO_GET_DAP_VOLUME_LEVELER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_VOLUME_LEVELER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_VOLUME_LEVELER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_VOLUME_LEVELER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_VOLUME_LEVELER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_VOLUME_LEVELER;
    } else if(paramType == HAL_AUDIO_SET_DAP_INTELLIGENT_EQUALIZER || paramType == HAL_AUDIO_GET_DAP_INTELLIGENT_EQUALIZER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_INTELLIGENT_EQUALIZER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_INTELLIGENT_EQUALIZER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_INTELLIGENT_EQUALIZER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_INTELLIGENT_EQUALIZER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_INTELLIGENT_EQUALIZER;
    } else if(paramType == HAL_AUDIO_SET_DAP_SURROUND_DECODER || paramType == HAL_AUDIO_GET_DAP_SURROUND_DECODER) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_SURROUND_DECODER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_SURROUND_DECODER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_SURROUND_DECODER;
    } else if(paramType == HAL_AUDIO_SET_DAP_MEDIA_INTELLIGENCE || paramType == HAL_AUDIO_GET_DAP_MEDIA_INTELLIGENCE) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_MEDIA_INTELLIGENCE)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_MEDIA_INTELLIGENCE));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_MEDIA_INTELLIGENCE) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_MEDIA_INTELLIGENCE : ENUM_PRIVATEINFO_AUDIO_GET_DAP_MEDIA_INTELLIGENCE;
    } else if(paramType == HAL_AUDIO_SET_DAP_SURROUND_VIRTUALIZER || paramType == HAL_AUDIO_GET_DAP_SURROUND_VIRTUALIZER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_SURROUND_VIRTUALIZER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_SURROUND_VIRTUALIZER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_SURROUND_VIRTUALIZER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_SURROUND_VIRTUALIZER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_SURROUND_VIRTUALIZER;
    } else if(paramType == HAL_AUDIO_SET_DAP_GRAPHICAL_EQUALIZER || paramType == HAL_AUDIO_GET_DAP_GRAPHICAL_EQUALIZER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_GRAPHICAL_EQUALIZER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_GRAPHICAL_EQUALIZER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_GRAPHICAL_EQUALIZER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_GRAPHICAL_EQUALIZER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_GRAPHICAL_EQUALIZER;
    } else if(paramType == HAL_AUDIO_SET_DAP_AUDIO_OPTIMIZER || paramType == HAL_AUDIO_GET_DAP_AUDIO_OPTIMIZER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_AUDIO_OPTIMIZER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_AUDIO_OPTIMIZER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_AUDIO_OPTIMIZER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_AUDIO_OPTIMIZER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_AUDIO_OPTIMIZER;
    } else if(paramType == HAL_AUDIO_SET_DAP_AUDIO_REGULATOR || paramType == HAL_AUDIO_GET_DAP_AUDIO_REGULATOR) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_AUDIO_REGULATOR)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_AUDIO_REGULATOR));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_AUDIO_REGULATOR) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_AUDIO_REGULATOR : ENUM_PRIVATEINFO_AUDIO_GET_DAP_AUDIO_REGULATOR;
    } else if(paramType == HAL_AUDIO_SET_DAP_BASS_ENHANCER || paramType == HAL_AUDIO_GET_DAP_BASS_ENHANCER) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_BASS_ENHANCER)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_BASS_ENHANCER));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_BASS_ENHANCER) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_BASS_ENHANCER : ENUM_PRIVATEINFO_AUDIO_GET_DAP_BASS_ENHANCER;
    } else if(paramType == HAL_AUDIO_SET_DAP_VIRTUAL_BASS || paramType == HAL_AUDIO_GET_DAP_VIRTUAL_BASS) {
        if(paramValueSize != sizeof(HAL_DAP_PARAM_VIRTUAL_BASS)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(HAL_DAP_PARAM_VIRTUAL_BASS));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_VIRTUAL_BASS) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_VIRTUAL_BASS : ENUM_PRIVATEINFO_AUDIO_GET_DAP_VIRTUAL_BASS;
    } else if(paramType == HAL_AUDIO_SET_DAP_BASS_EXTRACTION || paramType == HAL_AUDIO_GET_DAP_BASS_EXTRACTION) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_BASS_EXTRACTION) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_BASS_EXTRACTION : ENUM_PRIVATEINFO_AUDIO_GET_DAP_BASS_EXTRACTION;
    } else if(paramType == HAL_AUDIO_SET_DAP_DMX_TYPE || paramType == HAL_AUDIO_GET_DAP_DMX_TYPE) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_DMX_TYPE) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_DMX_TYPE : ENUM_PRIVATEINFO_AUDIO_GET_DAP_DMX_TYPE;
    } else if(paramType == HAL_AUDIO_SET_DAP_VIRTUALIZER_ENABLE || paramType == HAL_AUDIO_GET_DAP_VIRTUALIZER_ENABLE) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_VIRTUALIZER_ENABLE) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_VIRTUALIZER_ENABLE : ENUM_PRIVATEINFO_AUDIO_GET_DAP_VIRTUALIZER_ENABLE;
    } else if(paramType == HAL_AUDIO_SET_DAP_HEADPHONE_REVERB || paramType == HAL_AUDIO_GET_DAP_HEADPHONE_REVERB) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_HEADPHONE_REVERB) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_HEADPHONE_REVERB : ENUM_PRIVATEINFO_AUDIO_GET_DAP_HEADPHONE_REVERB;
    } else if(paramType == HAL_AUDIO_SET_DAP_SPEAKER_START || paramType == HAL_AUDIO_GET_DAP_SPEAKER_START) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_SPEAKER_START) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_SPEAKER_START : ENUM_PRIVATEINFO_AUDIO_GET_DAP_SPEAKER_START;
    } else if(paramType == HAL_AUDIO_SET_DAP_DE_DUCKING || paramType == HAL_AUDIO_GET_DAP_DE_DUCKING) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_DE_DUCKING) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_DE_DUCKING : ENUM_PRIVATEINFO_AUDIO_GET_DAP_DE_DUCKING;
    } else if(paramType == HAL_AUDIO_SET_DAP_PREGAIN || paramType == HAL_AUDIO_GET_DAP_PREGAIN) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_PREGAIN) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_PREGAIN : ENUM_PRIVATEINFO_AUDIO_GET_DAP_PREGAIN;
    } else if(paramType == HAL_AUDIO_SET_DAP_POSTGAIN || paramType == HAL_AUDIO_GET_DAP_POSTGAIN) {
        if(paramValueSize != sizeof(int)){
            ERROR("[RHAL] DAP param size fail! type:%d(%x->%x) \n", paramType, paramValueSize, sizeof(int));
            return NULL;
        }

        parameter->type = (paramType == HAL_AUDIO_SET_DAP_POSTGAIN) ?
            ENUM_PRIVATEINFO_AUDIO_SET_DAP_POSTGAIN : ENUM_PRIVATEINFO_AUDIO_GET_DAP_POSTGAIN;
    } else {
        ERROR("[RHAL] error, DAP param unknow type:%d \n", paramType);
        return NULL;
    }

    p_addr = rtkaudio_alloc_uncached(paramValueSize, &p_return);
    nonCachedAddr = p_return;
    parameter->privateInfo[0] = p_addr;

    return p_return;
}

DTV_STATUS_T RHAL_AUDIO_SetDAPParam(HAL_AUDIO_DAP_PARAM_TYPE paramType, void *paramValue, int paramValueSize)
{
    /* Get Avalaible Outpin */
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL ret;
    void *p_dap_param = NULL;
    INFO("%s Enter.\n", __func__);
    if(paramValue == NULL){
        ERROR("[RHAL] Error, RHAL_AUDIO_SetDAPParam fail! \n");
        return NOT_OK;
    }

    p_dap_param = dap_param_malloc(paramType, paramValueSize, &parameter);
    if(p_dap_param == NULL){
        ERROR("[RHAL] Error, dap_rtkaudio_malloc fail! (set)\n");
        return NOT_OK;
    }

    memcpy(p_dap_param, paramValue, paramValueSize);
    KADP_AUDIO_PrivateInfo(&parameter, &ret);
    rtkaudio_free(parameter.privateInfo[0]);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_GetDAPParam(HAL_AUDIO_DAP_PARAM_TYPE paramType, void *paramValue, int paramValueSize)
{
    /* Get Avalaible Outpin */
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL ret;
    void *p_dap_param = NULL;
    INFO("%s Enter.\n", __func__);
    if(paramValue == NULL){
        ERROR("[RHAL] Error, RHAL_AUDIO_GetDAPParam fail! \n");
        return NOT_OK;
    }

    p_dap_param = dap_param_malloc(paramType, paramValueSize, &parameter);
    if(p_dap_param == NULL){
        ERROR("[RHAL] Error, dap_rtkaudio_malloc fail! (get)\n");
        return NOT_OK;
    }

    KADP_AUDIO_PrivateInfo(&parameter, &ret);
    memcpy(paramValue, p_dap_param, paramValueSize);
    rtkaudio_free(parameter.privateInfo[0]);

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetDAPTuningData(void *pData, int dataBytes)
{
    UINT32 ret = 0;
    UINT8 *data_cma = NULL;
    UINT32 data_cma_phy = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter. dataBytes=%d\n", __func__, dataBytes);

    if (LITE_HAL_INIT_OK() != TRUE){
    	ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
    	return NOT_OK;
    }
    if(pData == NULL || dataBytes == 0) {
        ERROR("[AUDH][%s] pData:%d, dataBytes:%d\n",__func__,pData, dataBytes);
        return NOT_OK;
    }

    #if 0
    data_cma = RTKAudio_Malloc(dataBytes, (BYTE **)&data_cma_vir, (UINT32*)&data_cma_phy);
    #else
    data_cma_phy = rtkaudio_alloc_uncached(dataBytes, &data_cma);
    #endif
    if (data_cma == NULL) {
    	ERROR("[AUDH][%s %d] RTKAudio_Malloc data size=%d fail\n",__func__,__LINE__,dataBytes);
    	return NOT_OK;
    }

    memcpy(data_cma, pData, dataBytes);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_DAP_TUNINGDATA;
    audioConfig.value[0] = data_cma_phy;
    audioConfig.value[1] = dataBytes;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);

    if(ret != 0){
        rtkaudio_free(data_cma_phy);
        ERROR("[AUDH][%s %d] fail\n",__func__,__LINE__);
        return NOT_OK;
    }
    rtkaudio_free(data_cma_phy);
    return OK;

}

DTV_STATUS_T RHAL_AUDIO_TruvolumeHD_SetParameter(HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T param_type, void* data)
{
    AUDIO_TRUVOLUMEHD_PARAM_INFO truvolumehd_param;
    KADP_STATUS_T ret;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(data == NULL) return NOT_OK;

    if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_ENABLE){
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32;
    }else if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_TARGET_LOUNDNESS){
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32;
    }else if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_PRESET){
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32;
    }
    truvolumehd_param.Value = *((int*)data);
    truvolumehd_param.ValueSize = sizeof(truvolumehd_param.Value);
    ret = KADP_AUDIO_SetTruvolumdHDParam(&truvolumehd_param);

    if (KADP_OK != ret)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_TruvolumeHD_GetParameter(HAL_AUDIO_TRUVOLUMEHD_PARAM_TYPE_T param_type, void* data)
{
    KADP_STATUS_T ret;
    AUDIO_TRUVOLUMEHD_PARAM_INFO truvolumehd_param;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(data == NULL) return NOT_OK;

    if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_ENABLE) {
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32;
    }else if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_TARGET_LOUNDNESS) {
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32;
    }if(param_type == HAL_AUDIO_TRUVOLUMEHD_PARAM_PRESET) {
        truvolumehd_param.ParameterType = (enum_dtsParamTruvolumeHDType)ENUM_DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32;
    }
    truvolumehd_param.Value = 0;
    truvolumehd_param.ValueSize = sizeof(truvolumehd_param.Value);
    ret = KADP_AUDIO_GetTruvolumdHDParam(&truvolumehd_param);
    *((int*)data) = truvolumehd_param.Value;

    if (KADP_OK != ret)
        return NOT_OK;

    return OK;
}

int RAL_AUDIO_Hdmi_Fmt_Ntfy_loop(void *data)
{
    RAL_AUDIO_HDMI_FMT_NTFY_T *fmtNtfy = (RAL_AUDIO_HDMI_FMT_NTFY_T*)data;
    //pli_setThreadName("RAL_AUDIO_Hdmi_Fmt_Ntfy_loop");

    HAL_AUDIO_SRC_TYPE_T probFmt, tmpFmt;
    HAL_AUDIO_SRC_TYPE_T curFmt;
    INFO("%s Enter.\n", __func__);
    curFmt = probFmt = tmpFmt = HAL_AUDIO_SRC_TYPE_UNKNOWN;

    //while((fmtNtfy->rtkaudio_hdmifmtntfy_tsk != NULL) && (fmtNtfy->rtkaudio_hdmifmtntfy_tsk->IsAskToExit(fmtNtfy->pThread) == FALSE)) //rhal do
    if(fmtNtfy->rtkaudio_hdmifmtntfy_tsk != NULL) {
        for (;;)
        {
            msleep(200);
            down(&fmtNtfy_sem);
            if(RAL_AUDIO_GetDecodingType(fmtNtfy->adecIdx, &probFmt) == OK)
        	{
        	  if(curFmt != probFmt)
        	  {
                      tmpFmt = probFmt;
        	      msleep(50);
                      if(RAL_AUDIO_GetDecodingType(fmtNtfy->adecIdx, &probFmt) == OK)
        	      {
        	         if(curFmt != probFmt && tmpFmt == probFmt)
        		 {
        		     curFmt = probFmt;
        		     if(fmtNtfy->pfnCallBack != NULL)
        		     {
        		         fmtNtfy->pfnCallBack(&curFmt);
        		     }
        		 }
        	      }
        	  }
        	}
    	up(&fmtNtfy_sem);
        }
    }
    return 0;
}

DTV_STATUS_T RHAL_AUDIO_HDMI_FMT_NTFY_Open(HAL_AUDIO_ADEC_INDEX_T adecIndex)
{
    INFO("%s Enter.\n", __func__);

    if(Aud_hdmiFmtNtfy == NULL)
        return NOT_OK;

    Aud_hdmiFmtNtfy->adecIdx = adecIndex;

    if(Aud_hdmiFmtNtfy->adecIdx == HAL_AUDIO_ADEC0 || Aud_hdmiFmtNtfy->adecIdx == HAL_AUDIO_ADEC1)
    {
#if 0 //rhal do
    	if(Aud_hdmiFmtNtfy->pThread == NULL)
    	{
    	    Aud_hdmiFmtNtfy->pThread = new_thread(RAL_AUDIO_Hdmi_Fmt_Ntfy_loop, Aud_hdmiFmtNtfy);
    	}
#else
	Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk = kthread_create(RAL_AUDIO_Hdmi_Fmt_Ntfy_loop, Aud_hdmiFmtNtfy,
			"rtkaudio_hdmifmtntfy_tsk");
	if (IS_ERR(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk)) {
		Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk = NULL;
		return S_FALSE;
	}
	wake_up_process(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk);
#endif
    }

    return OK;  //OK or NOT_OK
}

DTV_STATUS_T RHAL_AUDIO_HDMI_FMT_NTFY_Close(void)
{
    INFO("%s Enter.\n", __func__);

    if(Aud_hdmiFmtNtfy == NULL)
        return NOT_OK;

#if 0 //rhal do
    if(Aud_hdmiFmtNtfy->pThread && Aud_hdmiFmtNtfy->pThread->IsRun(Aud_hdmiFmtNtfy->pThread))
    {
        Aud_hdmiFmtNtfy->pThread->Exit(Aud_hdmiFmtNtfy->pThread, TRUE);
    }

    if(Aud_hdmiFmtNtfy->pThread != NULL)
    {
        Aud_hdmiFmtNtfy->pThread->Delete(Aud_hdmiFmtNtfy->pThread);
        Aud_hdmiFmtNtfy->pThread = NULL;
    }
#else
    if (Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk) {
        int ret;
        ret = kthread_stop(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk);
        if (!ret)
            ERROR("rtkaudio_hdmifmtntfy_tsks task stopped\n");
    }
#endif
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_HDMI_FMT_NTFY_Start(pfnHdmiFmtNotify pfnCallBack)
{
    INFO("%s Enter.\n", __func__);

    if(Aud_hdmiFmtNtfy == NULL)
        return NOT_OK;

    down(&Aud_hdmiFmtNtfy_sem);
    Aud_hdmiFmtNtfy->pfnCallBack = pfnCallBack;
    if(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk != NULL /*&& Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk->IsRun(Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk) == FALSE*/)
    {
#if 0 //rhal do
        Aud_hdmiFmtNtfy->rtkaudio_hdmifmtntfy_tsk->Run(Aud_hdmiFmtNtfy->pThread);
#endif
    }
    up(&Aud_hdmiFmtNtfy_sem);

    return OK;
}

int getVXEnumFrmRhalParamType(HAL_AUDIO_VX_PARAM_TYPE_T param_type)
{
    switch(param_type) {
        case HAL_AUDIO_VX_PARAM_VX_ENABLE:
            return ENUM_DTS_PARAM_VX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_VX_INPUT_MODE:
            return ENUM_DTS_PARAM_VX_INPUT_MODE_I32;
        case HAL_AUDIO_VX_PARAM_VX_OUTPUT_MODE:
            return ENUM_DTS_PARAM_VX_OUTPUT_MODE_I32;
        case HAL_AUDIO_VX_PARAM_VX_HEADROOM_GAIN:
            return ENUM_DTS_PARAM_VX_HEADROOM_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_VX_PROC_OUTPUT_GAIN:
            return ENUM_DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_VX_REFERENCE_LEVEL:
            return ENUM_DTS_PARAM_VX_REFERENCE_LEVEL_I32;
        case HAL_AUDIO_VX_PARAM_VX_GAIN_COMPENSATION:
            return ENUM_DTS_PARAM_VX_GAIN_COMPENSATION_I32;
        case HAL_AUDIO_VX_PARAM_VX_PROCESS_DISCARD:
            return ENUM_DTS_PARAM_VX_PROCESS_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_TSX_ENABLE:
            return ENUM_DTS_PARAM_TSX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE:
            return ENUM_DTS_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HEIGHT_UPMIX_ENABLE:
            return ENUM_DTS_PARAM_TSX_HEIGHT_UPMIX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_PHANTOM_CENTER_GAIN:
            return ENUM_DTS_PARAM_TSX_PHANTOM_CENTER_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TSX_CENTER_GAIN:
            return ENUM_DTS_PARAM_TSX_CENTER_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HORIZ_VIR_EFF_CTRL:
            return ENUM_DTS_PARAM_TSX_HORIZ_VIR_EFF_CTRL_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HEIGHTMIX_COEFF:
            return ENUM_DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HEIGHTOUTCH_GAIN:
            return ENUM_DTS_PARAM_TSX_HEIGHTOUTCH_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TSX_LFE_GAIN:
            return ENUM_DTS_PARAM_TSX_LFE_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TSX_LRMIX_RATIO2CTRGAIN:
            return ENUM_DTS_PARAM_TSX_LRMIX_RATIO2CTRGAIN_I32;
        case HAL_AUDIO_VX_PARAM_TSX_PRECOND_FRONT:
            return ENUM_DTS_PARAM_TSX_PRECOND_FRONT_I32;
        case HAL_AUDIO_VX_PARAM_TSX_PRECOND_SURND:
            return ENUM_DTS_PARAM_TSX_PRECOND_SURND_I32;
        case HAL_AUDIO_VX_PARAM_TSX_TOPSPK_LOC_CTRL:
            return ENUM_DTS_PARAM_TSX_TOPSPK_LOC_CTRL_I32;
        case HAL_AUDIO_VX_PARAM_TSX_FRONT_WIDE_ENABLE:
            return ENUM_DTS_PARAM_TSX_FRONT_WIDE_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HEIGHT_VIRTUALIZER_ENABLE:
            return ENUM_DTS_PARAM_TSX_HEIGHT_VIRTUALIZER_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_FRONT_SURROUND_ENABLE:
            return ENUM_DTS_PARAM_TSX_FRONT_SURROUND_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_HEIGHT_DISCARD:
            return ENUM_DTS_PARAM_TSX_HEIGHT_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_VX_DC_ENABLE:
            return ENUM_DTS_PARAM_VX_DC_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_VX_DC_CONTROL:
            return ENUM_DTS_PARAM_VX_DC_CONTROL_I32;
        case HAL_AUDIO_VX_PARAM_VX_DEF_ENABLE:
            return  ENUM_DTS_PARAM_VX_DEF_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_VX_DEF_CONTROL:
            return ENUM_DTS_PARAM_VX_DEF_CONTROL_I32;
        case HAL_AUDIO_VX_PARAM_VX_CS2TO3_ENABLE:
            return ENUM_DTS_PARAM_VX_CS2TO3_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TSX_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_TSX_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_TSX_CONTROLS_STU:
            return ENUM_DTS_PARAM_TSX_CONTROLS_STU;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_FRONT_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_MODE:
            return ENUM_DTS_PARAM_TBHDX_FRONT_MODE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_SPKSIZE:
            return ENUM_DTS_PARAM_TBHDX_FRONT_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_DYNAMICS:
            return ENUM_DTS_PARAM_TBHDX_FRONT_DYNAMICS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_HP_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_FRONT_HP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_HPORDER:
            return ENUM_DTS_PARAM_TBHDX_FRONT_HPORDER_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_BASSLVL:
            return ENUM_DTS_PARAM_TBHDX_FRONT_BASSLVL_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_EXTBASS:
            return ENUM_DTS_PARAM_TBHDX_FRONT_EXTBASS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_INPUT_GAIN:
            return ENUM_DTS_PARAM_TBHDX_FRONT_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_BYPASS_GAIN:
            return ENUM_DTS_PARAM_TBHDX_FRONT_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_FRONT_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_TBHDX_FRONT_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_REAR_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_MODE:
            return ENUM_DTS_PARAM_TBHDX_REAR_MODE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_SPKSIZE:
            return ENUM_DTS_PARAM_TBHDX_REAR_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_DYNAMICS:
            return ENUM_DTS_PARAM_TBHDX_REAR_DYNAMICS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_HP_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_REAR_HP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_HPORDER:
            return ENUM_DTS_PARAM_TBHDX_REAR_HPORDER_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_BASSLVL:
            return ENUM_DTS_PARAM_TBHDX_REAR_BASSLVL_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_EXTBASS:
            return ENUM_DTS_PARAM_TBHDX_REAR_EXTBASS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_INPUT_GAIN:
            return ENUM_DTS_PARAM_TBHDX_REAR_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_BYPASS_GAIN:
            return ENUM_DTS_PARAM_TBHDX_REAR_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_REAR_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_TBHDX_REAR_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_CENTER_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_SPKSIZE:
            return ENUM_DTS_PARAM_TBHDX_CENTER_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_DYNAMICS:
            return ENUM_DTS_PARAM_TBHDX_CENTER_DYNAMICS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_HP_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_CENTER_HP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_HPORDER:
            return ENUM_DTS_PARAM_TBHDX_CENTER_HPORDER_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_BASSLVL:
            return ENUM_DTS_PARAM_TBHDX_CENTER_BASSLVL_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_EXTBASS:
            return ENUM_DTS_PARAM_TBHDX_CENTER_EXTBASS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_INPUT_GAIN:
            return ENUM_DTS_PARAM_TBHDX_CENTER_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_BYPASS_GAIN:
            return ENUM_DTS_PARAM_TBHDX_CENTER_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CENTER_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_TBHDX_CENTER_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_SRRND_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_MODE:
            return ENUM_DTS_PARAM_TBHDX_SRRND_MODE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_SPKSIZE:
            return ENUM_DTS_PARAM_TBHDX_SRRND_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_DYNAMICS:
            return ENUM_DTS_PARAM_TBHDX_SRRND_DYNAMICS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_HP_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_SRRND_HP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_HPORDER:
            return ENUM_DTS_PARAM_TBHDX_SRRND_HPORDER_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_BASSLVL:
            return ENUM_DTS_PARAM_TBHDX_SRRND_BASSLVL_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_EXTBASS:
            return ENUM_DTS_PARAM_TBHDX_SRRND_EXTBASS_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_INPUT_GAIN:
            return ENUM_DTS_PARAM_TBHDX_SRRND_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_BYPASS_GAIN:
            return ENUM_DTS_PARAM_TBHDX_SRRND_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SRRND_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_TBHDX_SRRND_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_TBHDX_DELAY_MATCHING_GAIN:
            return ENUM_DTS_PARAM_TBHDX_DELAY_MATCHING_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_PROCESS_DISCARD:
            return ENUM_DTS_PARAM_TBHDX_PROCESS_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_DELAY:
            return ENUM_DTS_PARAM_TBHDX_DELAY_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_CONTROLS_STU:
            return ENUM_DTS_PARAM_TBHDX_CONTROLS_STU;
/*
        case HAL_AUDIO_VX_PARAM_TBHDX_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_MONO_MODE:
            return ENUM_DTS_PARAM_TBHDX_MONO_MODE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_SPKSIZE:
            return ENUM_DTS_PARAM_TBHDX_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_MAXGAIN:
            return ENUM_DTS_PARAM_TBHDX_MAXGAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_HP_ENABLE:
            return ENUM_DTS_PARAM_TBHDX_HP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_HPORDER:
            return ENUM_DTS_PARAM_TBHDX_HPORDER_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_TEMP_GAIN:
            return ENUM_DTS_PARAM_TBHDX_TEMP_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_PROCESS_DISCARD_I32:
            return ENUM_DTS_PARAM_TBHDX_PROCESS_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_APP_FIRST_PARAM:
            return ENUM_DTS_PARAM_TBHDX_APP_FIRST_PARAM;
        case HAL_AUDIO_VX_PARAM_TBHDX_APP_SPKSIZE_I32:
            return ENUM_DTS_PARAM_TBHDX_APP_SPKSIZE_I32;
        case HAL_AUDIO_VX_PARAM_TBHDX_APP_HPRATIO_F32:
            return ENUM_DTS_PARAM_TBHDX_APP_HPRATIO_F32;
        case HAL_AUDIO_VX_PARAM_TBHDX_APP_EXTBASS_F32:
            return ENUM_DTS_PARAM_TBHDX_APP_EXTBASS_F32;
        case HAL_AUDIO_VX_PARAM_TBHDX_APP_LAST_PARAM:
            return ENUM_DTS_PARAM_TBHDX_APP_LAST_PARAM;
*/
        case HAL_AUDIO_VX_PARAM_MBHL_ENABLE:
            return ENUM_DTS_PARAM_MBHL_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_IO_MODE:
            return ENUM_DTS_PARAM_MBHL_IO_MODE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_ALG_DELAY:
            return ENUM_DTS_PARAM_MBHL_ALG_DELAY_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_ALG_GAIN:
            return ENUM_DTS_PARAM_MBHL_ALG_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_BYPASS_GAIN:
            return ENUM_DTS_PARAM_MBHL_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_REFERENCE_LEVEL:
            return ENUM_DTS_PARAM_MBHL_REFERENCE_LEVEL_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_VOLUME:
            return ENUM_DTS_PARAM_MBHL_VOLUME_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_VOLUME_STEP:
            return ENUM_DTS_PARAM_MBHL_VOLUME_STEP_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_BALANCE_STEP:
            return ENUM_DTS_PARAM_MBHL_BALANCE_STEP_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_OUTPUT_GAIN:
            return ENUM_DTS_PARAM_MBHL_OUTPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_BOOST:
            return ENUM_DTS_PARAM_MBHL_BOOST_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_THRESHOLD:
            return ENUM_DTS_PARAM_MBHL_THRESHOLD_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_SLOW_OFFSET:
            return ENUM_DTS_PARAM_MBHL_SLOW_OFFSET_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_FAST_ATTACK:
            return ENUM_DTS_PARAM_MBHL_FAST_ATTACK_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_FAST_RELEASE:
            return ENUM_DTS_PARAM_MBHL_FAST_RELEASE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_SLOW_ATTACK:
            return ENUM_DTS_PARAM_MBHL_SLOW_ATTACK_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_SLOW_RELEASE:
            return ENUM_DTS_PARAM_MBHL_SLOW_RELEASE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_DELAY:
            return ENUM_DTS_PARAM_MBHL_DELAY_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_ENVELOPE_FREQUENCY:
            return ENUM_DTS_PARAM_MBHL_ENVELOPE_FREQUENCY_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_MODE:
            return ENUM_DTS_PARAM_MBHL_MODE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_CROSS_LOW:
            return ENUM_DTS_PARAM_MBHL_CROSS_LOW_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_CROSS_MID:
            return ENUM_DTS_PARAM_MBHL_CROSS_MID_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_ATTACK:
            return ENUM_DTS_PARAM_MBHL_COMP_ATTACK_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_RELEASE:
            return ENUM_DTS_PARAM_MBHL_COMP_LOW_RELEASE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_RATIO:
            return ENUM_DTS_PARAM_MBHL_COMP_LOW_RATIO_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_THRESH:
            return ENUM_DTS_PARAM_MBHL_COMP_LOW_THRESH_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_LOW_MAKEUP:
            return ENUM_DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_RELEASE:
            return ENUM_DTS_PARAM_MBHL_COMP_MID_RELEASE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_RATIO:
            return ENUM_DTS_PARAM_MBHL_COMP_MID_RATIO_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_THRESH:
            return ENUM_DTS_PARAM_MBHL_COMP_MID_THRESH_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_MID_MAKEUP:
            return ENUM_DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_RELEASE:
            return ENUM_DTS_PARAM_MBHL_COMP_HIGH_RELEASE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_RATIO:
            return ENUM_DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_THRESH:
            return ENUM_DTS_PARAM_MBHL_COMP_HIGH_THRESH_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_HIGH_MAKEUP:
            return ENUM_DTS_PARAM_MBHL_COMP_HIGH_MAKEUP_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_GAIN_REDUCTION:
            return ENUM_DTS_PARAM_MBHL_GAIN_REDUCTION_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COMP_GAIN_REDUCTION:
            return ENUM_DTS_PARAM_MBHL_COMP_GAIN_REDUCTION_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_PROCESS_DISCARD:
            return ENUM_DTS_PARAM_MBHL_PROCESS_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_AC_ENABLE:
            return ENUM_DTS_PARAM_MBHL_AC_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_AC_LEVEL:
            return ENUM_DTS_PARAM_MBHL_AC_LEVEL_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_CP_ENABLE:
            return ENUM_DTS_PARAM_MBHL_CP_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_CP_LEVEL:
            return ENUM_DTS_PARAM_MBHL_CP_LEVEL_I32;
        case HAL_AUDIO_VX_PARAM_MBHL_COEF_STU:
            return ENUM_DTS_PARAM_MBHL_COEF_STU;
        case HAL_AUDIO_VX_PARAM_MBHL_CONTROLS_STU:
            return ENUM_DTS_PARAM_MBHL_CONTROLS_STU;
        //case HAL_AUDIO_VX_PARAM_MBHL_APP_FRT_LOWCROSS_F32:
        //    return ENUM_DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32;
        //case HAL_AUDIO_VX_PARAM_MBHL_APP_FRT_MIDCROSS_F32:
        //    return ENUM_DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_ENABLE:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_IO_MODE:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_IO_MODE_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_PRESET:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_DISCARD:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_DISCARD_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_LATENCY_MODE:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_LATENCY_MODE_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_DELAY:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_DELAY_I32_1;
        case HAL_AUDIO_VX_PARAM_LOUDNESS_CONTROL_CONTROLS_STU:
            return ENUM_DTS_PARAM_LOUDNESS_CONTROL_CONTROLS_STU_1;
        case HAL_AUDIO_VX_PARAM_GEQ10B_ENABLE:
            return ENUM_DTS_PARAM_GEQ10B_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_CHANNEL_CTRL_MASK:
            return ENUM_DTS_PARAM_GEQ10B_CHANNEL_CTRL_MASK_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_INPUT_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND0_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND0_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND1_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND1_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND2_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND2_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND3_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND3_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND4_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND4_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND5_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND5_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND6_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND6_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND7_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND7_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND8_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND8_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_BAND9_GAIN:
            return ENUM_DTS_PARAM_GEQ10B_BAND9_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_DISCARD:
            return ENUM_DTS_PARAM_GEQ10B_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_GEQ10B_CONTROLS_STU:
            return ENUM_DTS_PARAM_GEQ10B_CONTROLS_STU;
        case HAL_AUDIO_VX_PARAM_AEQ_ENABLE:
            return ENUM_DTS_PARAM_AEQ_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_AEQ_CHANNEL_CTRL_MASK:
            return ENUM_DTS_PARAM_AEQ_CHANNEL_CTRL_MASK_I32;
        case HAL_AUDIO_VX_PARAM_AEQ_INPUT_GAIN:
            return ENUM_DTS_PARAM_AEQ_INPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_AEQ_OUTPUT_GAIN:
            return ENUM_DTS_PARAM_AEQ_OUTPUT_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_AEQ_BYPASS_GAIN:
            return ENUM_DTS_PARAM_AEQ_BYPASS_GAIN_I32;
        case HAL_AUDIO_VX_PARAM_AEQ_CONTROLS_STU:
            return ENUM_DTS_PARAM_AEQ_CONTROLS_STU;
        case HAL_AUDIO_VX_PARAM_AEQ_CUSTOMCOEFF_STU:
            return ENUM_DTS_PARAM_AEQ_CUSTOMCOEFF_STU;
        case HAL_AUDIO_VX_PARAM_AEQ_PROCESS_DISCARD:
            return ENUM_DTS_PARAM_AEQ_PROCESS_DISCARD_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_ENABLE:
            return ENUM_DTS_PARAM_MULTIRATE_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_FILTER_MODE:
            return ENUM_DTS_PARAM_MULTIRATE_FILTER_MODE_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_IN_MODE:
            return ENUM_DTS_PARAM_MULTIRATE_IN_MODE_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_OUT_MODE:
            return ENUM_DTS_PARAM_MULTIRATE_OUT_MODE_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_UPMIX_ENABLE:
            return ENUM_DTS_PARAM_MULTIRATE_UPMIX_ENABLE_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_DELAY_COMPENSATION:
            return ENUM_DTS_PARAM_MULTIRATE_DELAY_COMPENSATION_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_GAIN_COMPENSATION:
            return ENUM_DTS_PARAM_MULTIRATE_GAIN_COMPENSATION_I32;
        case HAL_AUDIO_VX_PARAM_MULTIRATE_CONTROLS_STU:
            return ENUM_DTS_PARAM_MULTIRATE_CONTROLS_STU;

        default:
            ERROR("Unknown ParamType %x\n", param_type);
            return -1;
    }
}

DTV_STATUS_T RHAL_AUDIO_VX_SetParameter(HAL_AUDIO_VX_PARAM_TYPE_T param_type, void* data)
{
    int vxEnum;
    KADP_STATUS_T ret = KADP_NOT_OK;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(data == NULL) return NOT_OK;
    if(param_type < 0) return NOT_OK;

    vxEnum = getVXEnumFrmRhalParamType(param_type);

    if (vxEnum >= ENUM_DTS_PARAM_VX_ENABLE_I32 && vxEnum <= ENUM_DTS_PARAM_VIRTUALXLIB1_LAST_PARAM) {
            AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO trusrndx_param;
            memset(&trusrndx_param, 0, sizeof(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO));

            trusrndx_param.ParameterType = (enum_dtsParamVirtualxlib1Type)vxEnum;
            trusrndx_param.Value[0] = *((int*)data);
            trusrndx_param.ValueSize = sizeof(trusrndx_param.Value[0]);
            ret = KADP_AUDIO_SetTruSrndXParam(&trusrndx_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_TBHDX_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_TBHDX_LAST_PARAM) {
            AUDIO_VIRTUALX_TBHDX_PARAM_INFO tbhdx_param;
            memset(&tbhdx_param, 0, sizeof(AUDIO_VIRTUALX_TBHDX_PARAM_INFO));

            tbhdx_param.ParameterType = (enum_dtsParamTBHDxType)vxEnum;
            tbhdx_param.Value[0] = *((int*)data);
            tbhdx_param.ValueSize = sizeof(tbhdx_param.Value[0]);
            ret = KADP_AUDIO_SetTbhdXParam(&tbhdx_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_MBHL_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_MBHL_LAST_PARAM) {
            AUDIO_VIRTUALX_MBHL_PARAM_INFO mbhl_param;
            memset(&mbhl_param, 0, sizeof(AUDIO_VIRTUALX_MBHL_PARAM_INFO));

            mbhl_param.ParameterType = (enum_dtsParamMbhlType)vxEnum;
            mbhl_param.Value[0] = *((int*)data);
            mbhl_param.ValueSize = sizeof(mbhl_param.Value[0]);
            ret = KADP_AUDIO_SetMbhlParam(&mbhl_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_LOUDNESS_CONTROL_FIRST_PARAM_1 && vxEnum <= ENUM_DTS_PARAM_LOUDNESS_CONTROL_LAST_PARAM_1) {
            AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO loudness_control_param;
            memset(&loudness_control_param, 0, sizeof(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO));

            loudness_control_param.ParameterType = (enum_dtsParamLoudnessControlType)vxEnum;
            loudness_control_param.Value[0] = *((int*)data);
            loudness_control_param.ValueSize = sizeof(loudness_control_param.Value[0]);
            ret = KADP_AUDIO_SetLoudnessControlParam(&loudness_control_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_GEQ10B_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_GEQ10B_LAST_PARAM) {
            AUDIO_VIRTUALX_Geq10b_PARAM_INFO geq_param;
            memset(&geq_param, 0, sizeof(AUDIO_VIRTUALX_Geq10b_PARAM_INFO));

            geq_param.ParameterType = (enum_dtsParamGeq10bType)vxEnum;
            geq_param.Value[0] = *((int*)data);
            geq_param.ValueSize = sizeof(geq_param.Value[0]);
            ret = KADP_AUDIO_SetGeqParam(&geq_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_AEQ_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_AEQ_LAST_PARAM) {
            AUDIO_VIRTUALX_Aeq_PARAM_INFO aeq_param;
            memset(&aeq_param, 0, sizeof(AUDIO_VIRTUALX_Aeq_PARAM_INFO));

            aeq_param.ParameterType = (enum_dtsParamAeqType)vxEnum;
            aeq_param.Value[0] = *((int*)data);
            aeq_param.ValueSize = sizeof(aeq_param.Value[0]);
            ret = KADP_AUDIO_SetAeqParam(&aeq_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_MULTIRATE_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_MULTIRATE_LAST_PARAM) {
            AUDIO_VIRTUALX_MultiRate_PARAM_INFO multi_rate_param;
            memset(&multi_rate_param, 0, sizeof(AUDIO_VIRTUALX_MultiRate_PARAM_INFO));

            multi_rate_param.ParameterType = (enum_dtsParamMultiRateType)vxEnum;
            multi_rate_param.Value[0] = *((int*)data);
            multi_rate_param.ValueSize = sizeof(multi_rate_param.Value[0]);
            ret = KADP_AUDIO_SetMultiRateParam(&multi_rate_param);
    }

    if (KADP_OK != ret)
        return NOT_OK;

    return OK;
}
DTV_STATUS_T RHAL_AUDIO_VX_GetParameter(HAL_AUDIO_VX_PARAM_TYPE_T param_type, void* data)
{
    int vxEnum;
    KADP_STATUS_T ret = KADP_NOT_OK;

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) return NOT_OK;
    if(data == NULL) return NOT_OK;
    if(param_type < 0) return NOT_OK;

    vxEnum = getVXEnumFrmRhalParamType(param_type);
    if (vxEnum >= ENUM_DTS_PARAM_VX_ENABLE_I32 && vxEnum <= ENUM_DTS_PARAM_VIRTUALXLIB1_LAST_PARAM) {
            AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO trusrndx_param;
            memset(&trusrndx_param, 0, sizeof(AUDIO_VIRTUALX_TRUSRNDX_PARAM_INFO));

            trusrndx_param.ParameterType = (enum_dtsParamVirtualxlib1Type)vxEnum;
            trusrndx_param.Value[0] = 0;
            trusrndx_param.ValueSize = sizeof(trusrndx_param.Value[0]);
            ret = KADP_AUDIO_GetTruSrndXParam(&trusrndx_param);
            *((int*)data) = trusrndx_param.Value[0];
    } else if(vxEnum >= ENUM_DTS_PARAM_TBHDX_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_TBHDX_LAST_PARAM) {
            AUDIO_VIRTUALX_TBHDX_PARAM_INFO tbhdx_param;
            memset(&tbhdx_param, 0, sizeof(AUDIO_VIRTUALX_TBHDX_PARAM_INFO));

            tbhdx_param.ParameterType =(enum_dtsParamTBHDxType)vxEnum;
            tbhdx_param.Value[0] = 0;
            tbhdx_param.ValueSize = sizeof(tbhdx_param.Value[0]);
            ret = KADP_AUDIO_GetTbhdXParam(&tbhdx_param);
            *((int*)data) = tbhdx_param.Value[0];
    } else if (vxEnum == ENUM_DTS_PARAM_MBHL_COMP_GAIN_REDUCTION_I32) {
            AUDIO_VIRTUALX_MBHL_PARAM_INFO mbhl_param;
            memset(&mbhl_param, 0, sizeof(AUDIO_VIRTUALX_MBHL_PARAM_INFO));

            mbhl_param.ParameterType = (enum_dtsParamMbhlType)vxEnum;
            mbhl_param.Value[0] = 0;
            mbhl_param.Value[1] = 0;
            mbhl_param.Value[2] = 0;
            mbhl_param.ValueSize = sizeof(mbhl_param.Value[0]) * 3;
            ret = KADP_AUDIO_GetMbhlParam(&mbhl_param);
            *((int*)data) = mbhl_param.Value[0];
            *(((int*)data) + 1) = mbhl_param.Value[1];
            *(((int*)data) + 2) = mbhl_param.Value[2];
    } else if (vxEnum >= ENUM_DTS_PARAM_MBHL_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_MBHL_LAST_PARAM) {
            AUDIO_VIRTUALX_MBHL_PARAM_INFO mbhl_param;
            memset(&mbhl_param, 0, sizeof(AUDIO_VIRTUALX_MBHL_PARAM_INFO));

            mbhl_param.ParameterType = (enum_dtsParamMbhlType)vxEnum;
            mbhl_param.Value[0] = 0;
            mbhl_param.ValueSize = sizeof(mbhl_param.Value[0]);
            ret = KADP_AUDIO_GetMbhlParam(&mbhl_param);
            *((int*)data) = mbhl_param.Value[0];
    } else if (vxEnum >= ENUM_DTS_PARAM_LOUDNESS_CONTROL_FIRST_PARAM_1 && vxEnum <= ENUM_DTS_PARAM_LOUDNESS_CONTROL_LAST_PARAM_1) {
            AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO loudness_control_param;
            memset(&loudness_control_param, 0, sizeof(AUDIO_VIRTUALX_Loudness_Control_PARAM_INFO));

            loudness_control_param.ParameterType = (enum_dtsParamLoudnessControlType)vxEnum;
            loudness_control_param.Value[0] = *((int*)data);
            loudness_control_param.ValueSize = sizeof(loudness_control_param.Value[0]);
            ret = KADP_AUDIO_GetLoudnessControlParam(&loudness_control_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_GEQ10B_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_GEQ10B_LAST_PARAM) {
            AUDIO_VIRTUALX_Geq10b_PARAM_INFO geq_param;
            memset(&geq_param, 0, sizeof(AUDIO_VIRTUALX_Geq10b_PARAM_INFO));

            geq_param.ParameterType = (enum_dtsParamGeq10bType)vxEnum;
            geq_param.Value[0] = *((int*)data);
            geq_param.ValueSize = sizeof(geq_param.Value[0]);
            ret = KADP_AUDIO_GetGeqParam(&geq_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_AEQ_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_AEQ_LAST_PARAM) {
            AUDIO_VIRTUALX_Aeq_PARAM_INFO aeq_param;
            memset(&aeq_param, 0, sizeof(AUDIO_VIRTUALX_Aeq_PARAM_INFO));

            aeq_param.ParameterType = (enum_dtsParamAeqType)vxEnum;
            aeq_param.Value[0] = *((int*)data);
            aeq_param.ValueSize = sizeof(aeq_param.Value[0]);
            ret = KADP_AUDIO_GetAeqParam(&aeq_param);
    } else if (vxEnum >= ENUM_DTS_PARAM_MULTIRATE_FIRST_PARAM && vxEnum <= ENUM_DTS_PARAM_MULTIRATE_LAST_PARAM) {
            AUDIO_VIRTUALX_MultiRate_PARAM_INFO multi_rate_param;
            memset(&multi_rate_param, 0, sizeof(AUDIO_VIRTUALX_MultiRate_PARAM_INFO));

            multi_rate_param.ParameterType = (enum_dtsParamMultiRateType)vxEnum;
            multi_rate_param.Value[0] = *((int*)data);
            multi_rate_param.ValueSize = sizeof(multi_rate_param.Value[0]);
            ret = KADP_AUDIO_GetMultiRateParam(&multi_rate_param);
    }

    if (KADP_OK != ret)
        return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAnalogAdjDB(HAL_AUDIO_AREA_TYPE_T area_type)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    /* check value */
    if(area_type < 0 || area_type >= HAL_AUDIO_AREA_MAX){
        ERROR("[%s] area_type out of range:%d \n", __func__, area_type);
        return NOT_OK;
    }

    audioConfig.msg_id = AUDIO_CONFIG_CMD_ANALOG_ADJ_DB;
    audioConfig.value[0] = area_type;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK)
    {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetDolbyOTTMode(BOOLEAN bIsOTTEnable, BOOLEAN bIsATMOSLockingEnable)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(LITE_HAL_INIT_OK() != TRUE)
        return NOT_OK;

    if(bIsOTTEnable == FALSE && bIsATMOSLockingEnable == TRUE)  //not enabled, invalid
        return NOT_OK;

    audioConfig.msg_id = AUDIO_CONFIG_CMD_SET_DOLBY_OTT_MODE;
    audioConfig.value[0] = bIsOTTEnable;             /* OTT mode */
    audioConfig.value[1] = bIsATMOSLockingEnable;    /* ATMOS Locking */

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

/* AC-4 decoder */
/* AC-4 Auto Presenetation Selection. Refer to "Selection using system-level preferences" of "Dolby MS12 Multistream Decoder Implementation integration manual" */
static int IsValidLang(HAL_AUDIO_LANG_CODE_TYPE_T enCodeType, UINT32 lang)
{
    UINT8 ch_1, ch_2, ch_3, ch_4;

    if (enCodeType != HAL_AUDIO_LANG_CODE_ISO639_1 &&
        enCodeType != HAL_AUDIO_LANG_CODE_ISO639_2)
        return FALSE;

    ch_1 = (lang & 0xFF000000) >> 24;
    ch_2 = (lang & 0x00FF0000) >> 16;
    ch_3 = (lang & 0x0000FF00) >> 8;
    ch_4 = (lang & 0x000000FF);

    if ((ch_1 < 0x61) || (ch_1 > 0x7a)) return FALSE;
    if ((ch_2 < 0x61) || (ch_2 > 0x7a)) return FALSE;
    if (enCodeType == HAL_AUDIO_LANG_CODE_ISO639_1 && ch_3 != 0) return FALSE;
    else if (enCodeType == HAL_AUDIO_LANG_CODE_ISO639_2 &&
            ((ch_3 < 0x61) || (ch_3 > 0x7a)))
        return FALSE;
    if (ch_4 != 0) return FALSE;

    return TRUE;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationFirstLanguage(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_LANG_CODE_TYPE_T enCodeType, UINT32 firstLang)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    if (!IsValidLang(enCodeType, firstLang))
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_FIRST_LANGUAGE;
    audioConfig.value[0] = (u_int)firstLang;
    audioConfig.value[1] = (u_int)enCodeType;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationSecondLanguage(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_LANG_CODE_TYPE_T enCodeType, UINT32 secondLang)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    if (!IsValidLang(enCodeType, secondLang))
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_SECOND_LANGUAGE;
    audioConfig.value[0] = (u_int)secondLang;
    audioConfig.value[1] = (u_int)enCodeType;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationADMixing(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bIsEnable)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_ADMIXING;
    audioConfig.value[0] = (u_int)bIsEnable;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationADType(HAL_AUDIO_ADEC_INDEX_T adecIndex, HAL_AUDIO_AC4_AD_TYPE_T enADType)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    if(enADType < HAL_AUDIO_AC4_AD_TYPE_NONE || enADType > HAL_AUDIO_AC4_AD_TYPE_VO)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_ADTYPE;
    audioConfig.value[0] = (u_int)enADType;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetAutoPresentationPrioritizeADType(HAL_AUDIO_ADEC_INDEX_T adecIndex, BOOLEAN bIsEnable)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_PRIORITIZE_ADTYPE;
    audioConfig.value[0] = (u_int)bIsEnable;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AC4_SetPresentationGroupIndex(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT32 pres_group_idx)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_PRESENTATION_GROUP_INDEX;
    audioConfig.value[0] = (u_int)pres_group_idx;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

/* AC-4 Dialogue Enhancement */
DTV_STATUS_T RHAL_AUDIO_AC4_SetDialogueEnhancementGain(HAL_AUDIO_ADEC_INDEX_T adecIndex, UINT8 dialEnhanceGain)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);

    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if(!RangeCheck(adecIndex, 0, AUD_ADEC_MAX)) return NOT_OK;

    if(dialEnhanceGain > 12)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));

    audioConfig.msg_id = AUDIO_CONFIG_CMD_AC4_DIALOGUE_ENHANCEMENT_GAIN;
    audioConfig.value[0] = (u_int)dialEnhanceGain;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SoundConfig_Data(UINT32* pData1, UINT32 size1, UINT32* pData2, UINT32 size2)
{
    UINT32 ret = 0;
    UINT8 *data1_cma = NULL, *data2_cma = NULL;
    UINT32 data1_cma_phy = 0, data2_cma_phy = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);

    if (LITE_HAL_INIT_OK() != TRUE){
        ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
        return NOT_OK;
    }
    if(pData1 == NULL || pData2 == NULL) {
        ERROR("[AUDH][%s] pData1:%p, pData2:%p\n",__func__,pData1,pData2);
        return NOT_OK;
    }

    #if 0
    data1_cma = RTKAudio_Malloc(size1, (BYTE **)&data1_cma_vir, (UINT32*)&data1_cma_phy);
    #else
    data1_cma_phy = rtkaudio_alloc_uncached(size1, &data1_cma);
    #endif
    if (data1_cma == NULL) {
        ERROR("[AUDH][%s %d] RTKAudio_Malloc data1 size=%d fail\n",__func__,__LINE__,size1);
        return NOT_OK;
    }
    #if 0
    data2_cma = RTKAudio_Malloc(size2, (BYTE **)&data2_cma_vir, (UINT32*)&data2_cma_phy);
    #else
    data2_cma_phy = rtkaudio_alloc_uncached(size2, &data2_cma);
    #endif
    if (data2_cma == NULL) {
        rtkaudio_free(data1_cma_phy);
        ERROR("[AUDH][%s %d] RTKAudio_Malloc data2 size=%d fail\n",__func__,__LINE__,size2);
        return NOT_OK;
    }

    memcpy(data1_cma, pData1, size1);
    memcpy(data2_cma, pData2, size2);

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_CUSTOMIZE_REQUEST;
    audioConfig.value[0] = ENUM_CUSTOMIZE_SOUND_CONFIG_DATA;
    audioConfig.value[1] = data1_cma_phy;
    audioConfig.value[2] = size1;
    audioConfig.value[3] = data2_cma_phy;
    audioConfig.value[4] = size2;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);

    if(ret != 0){
        rtkaudio_free(data1_cma_phy);
        rtkaudio_free(data2_cma_phy);
        ERROR("[AUDH][%s %d] fail\n",__func__,__LINE__);
        return NOT_OK;
    }
    rtkaudio_free(data1_cma_phy);
    rtkaudio_free(data2_cma_phy);
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SoundEffect_Level(UINT32 param1, UINT32 param2, UINT32 param3)
{
    UINT32 ret = 0;
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);

    if (LITE_HAL_INIT_OK() != TRUE){
        ERROR("[AUDH][%s %d] Audio is not initialized\n",__func__,__LINE__);
        return NOT_OK;
    }

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_CUSTOMIZE_REQUEST;
    audioConfig.value[0] = ENUM_CUSTOMIZE_SOUND_EFFECT_LEVEL;
    audioConfig.value[1] = param1;
    audioConfig.value[2] = param2;
    audioConfig.value[3] = param3;

    ret = KADP_AUDIO_AudioConfig(&audioConfig);

    if(ret != 0){
        ERROR("[AUDH][%s %d] fail\n",__func__,__LINE__);
        return NOT_OK;
    }
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetLowLatencyMode(BOOLEAN bOnOff)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_AO_LOW_LATENCY;
    audioConfig.value[0] = (u_int)bOnOff;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_SetAECReference_Source(HAL_AUDIO_AEC_SOURCE_T aec_source)
{
	AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;
    INFO("%s Enter.\n", __func__);
    if(AUDIO_HAL_CHECK_INITIAL_OK() != TRUE)
        return NOT_OK;

    if((aec_source >= HAL_AUDIO_AEC_SOURCE_MAX) || (aec_source <= 0)) {
        ERROR("[AUDH][%s] aec_source error:%d\n", __func__, aec_source);
        return NOT_OK;
    }
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_AEC_SOURCE;
    audioConfig.value[0] = (aec_source & HAL_AUDIO_AEC_I2S_EXTERNEL_LOOP) ? TRUE : FALSE;

    if(SendRPC_AudioConfig(&audioConfig) != S_OK) return NOT_OK;
    return OK;
}

DTV_STATUS_T RHAL_AUDIO_AISound_Enable(BOOLEAN bOnOff)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("%s Enter.\n", __func__);

    if (AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) {
        return NOT_OK;
    }
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_AISOUND_CONTROL;
    audioConfig.value[0] = bOnOff;

    if (SendRPC_AudioConfig(&audioConfig) != S_OK) {
        return NOT_OK;
    }

    return OK;
}

DTV_STATUS_T RHAL_AUDIO_BTMic_Enable(BOOLEAN bOnOff)
{
    AUDIO_CONFIG_COMMAND_RTKAUDIO audioConfig;

    INFO("%s Enter.\n", __func__);

    if (AUDIO_HAL_CHECK_INITIAL_OK() != TRUE) {
        return NOT_OK;
    }
    memset(&audioConfig, 0, sizeof(AUDIO_CONFIG_COMMAND_RTKAUDIO));
    audioConfig.msg_id = AUDIO_CONFIG_CMD_BTMIC_CONTROL;
    audioConfig.value[0] = bOnOff;

    if (SendRPC_AudioConfig(&audioConfig) != S_OK) {
        return NOT_OK;
    }

    return OK;
}

#endif /* _HAL_AUDIO_ */
