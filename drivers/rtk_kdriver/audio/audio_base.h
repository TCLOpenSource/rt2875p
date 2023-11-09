/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/

#ifndef _RTK_AUDIO_
#define _RTK_AUDIO_
#include <linux/semaphore.h>
#include "audio_inc.h"
#include "audio_pin.h"
#include "audio_util.h"

// need to be multiple of 8, a transaction of DMA is 8 bytes
// AI -- ADEC (*2)
#define AUDIO_IN_OUTPUT_BUF_SIZE   (256*1024)
// ClipDec -- ADEC (*1)
#define AUDIO_MEMIN_OUTPUT_BUF_SIZE (768*1024)
// AMixer -- AO-FlashPin (*2)
#define AUDIO_FLASH_OUTPUT_BUF_SIZE (64*1024)
// ADEC -- PP (*8)
#define AUDIO_DEC_OUTPUT_BUF_SIZE   (64*1024)
// ENC -- (*1)
#define AUDIO_ENC_OUTPUT_BUF_SIZE   (32*1024)
// AO -- BT (*2)
#define AUDIO_AOUT_OUTPUT_BUF_SIZE  (32*1024)

#define AUDIO_MAX_CHNUM  8

typedef enum
{
    ENUM_AUDIO_DEC_EXT_BS,
    ENUM_AUDIO_DEC_CFG,
    ENUM_AUDIO_ERROR_STATUS,
    ENUM_AUDIO_RPC_PRIVATE_INFO,
} ENUM_AUDIO_INFO_TYPE;

// compression scale factors (examples)
#define COMP_SCALE_FULL ((UINT32)0x7fffffff)
#define COMP_SCALE_HALF ((UINT32)0x40000000)
#define COMP_SCALE_NONE ((UINT32)0x00000000)

// dual mono downmix mode
enum
{
    DUAL_MONO_MIX,
    DUAL_MONO_L,
    DUAL_MONO_R,
    DUAL_MONO_STEREO
};

// compression mode
enum
{
    COMP_CUSTOM_A,
    COMP_CUSTOM_D,
    COMP_LINEOUT,
    COMP_RF
};

// audio coding mode
enum
{
    MODE_11,
    MODE_10,
    MODE_20,
    MODE_30,
    MODE_21,
    MODE_31,
    MODE_22,
    MODE_32
};

enum
{
    LFE_OFF,
    LFE_ON,
    LFE_DUAL,
    LFE_MIX
};

enum
{
    SUBSTRM_ID_I0,
    SUBSTRM_ID_I1,
    SUBSTRM_ID_I2,
    SUBSTRM_ID_I3,
    SUBSTRM_ID_I4,
    SUBSTRM_ID_I5,
    SUBSTRM_ID_I6,
    SUBSTRM_ID_I7,
    SUBSTRM_ID_NUM,
};

enum AUDIO_CHANNEL_OUT_SWAP {
    AUDIO_CHANNEL_OUT_SWAP_STEREO = 0x0,
    AUDIO_CHANNEL_OUT_SWAP_L_TO_R = 0x1,
    AUDIO_CHANNEL_OUT_SWAP_R_TO_L = 0x2,
    AUDIO_CHANNEL_OUT_SWAP_LR_SWAP = 0x3,
    AUDIO_CHANNEL_OUT_SWAP_LR_MIXED = 0x4,
};

typedef struct
{
    ENUM_AUDIO_INFO_TYPE    infoType;
    UINT32                  err;
} AUDIO_INFO_ERR_STATUS;

typedef enum {

    /* sending PTS information (int64_t) */
    INFO_PTS,

    /* sending audio format information (AUDIO_FORMAT) */
    INFO_AUDIO_FORMAT,

    /* ask PP Aout start to do mixing. (DUAL_DEC_MIXING) */
    INFO_AUDIO_START_MIXING,

    /* (DUAL_DEC_INFO) */
    INFO_AUDIO_MIX_INFO,

    /* Deliver Info */
    INFO_DELIVER_INFO,

    /* EndOfStream */
    INFO_EOS,

    /* Errorstatus */
    INFO_ERROR_STATUS,

    /* DDP metadata */
    INFO_DDP_METADATA,

    /* AAC metadata */
    INFO_AAC_METADATA,

    /* DAP Selftest Params */
    INFO_DAP_PARAMS,

    /* DAP oamdi info */
    INFO_DAP_OAMDI,

    /* MIXER AD descriptor*/
    INFO_MIXERE_AD_DESCRIPTOR,

    /* MS12_DDP_MIXER metadata*/
    INFO_MS12_MIXER_CMD,

    /* MS12 IIDK USE_CASE Params*/
    INFO_MS12_IIDK_USE_CASE,

    /* MS12 IIDK INIT_PARAMS*/
    INFO_MS12_IIDK_INIT_PARAMS,

    /* MS12 IIDK RUNTIME_PARAMS*/
    INFO_MS12_IIDK_RUNTIME_PARAMS,

    /* AC4 metadata */
    INFO_AC4_METADATA,

    /* PCMR INIT_PARAMS */
    INFO_PCMR_RUNTIME_PARAMS,

} INFO_INDEX;

typedef enum
{
    DEC_IS_MAIN,
    DEC_IS_SUB,
} DUAL_DEC_MODE;

typedef struct
{
    bool bEnable;
    DUAL_DEC_MODE subDecoderMode;
    int subStreamId;
} DUAL_DEC_INFO;

typedef struct
{
    int mode;
    int volume;
} DUAL_DEC_MIXING;

typedef enum{
    RHALTYPE_NONE,
    RHALTYPE_PCM,               // PCM audio.(big endian)
    RHALTYPE_PCM_LITTLE_ENDIAN, // PCM audio.(little endian)
    RHALTYPE_MPEG1Packet,       // MPEG1 Audio packet.
    RHALTYPE_MPEG1ES,           // MPEG1 Audio Payload (Elementary Stream).
    RHALTYPE_MPEG2_AUDIO,       // MPEG-2 audio data
    RHALTYPE_DVD_LPCM_AUDIO,    // DVD audio data
    RHALTYPE_MLP_AUDIO,         // trueHD
    RHALTYPE_DOLBY_AC3,         // Dolby data
    RHALTYPE_DOLBY_AC3_SPDIF,   // Dolby AC3 over SPDIF.
    RHALTYPE_MPEG_AUDIO,        // general MPEG audio
    RHALTYPE_DTS,               // DTS audio
    RHALTYPE_DTS_HD,            // DTS-HD audio
    RHALTYPE_DTS_HD_EXT,        // DTS-HD extension sub stream
    RHALTYPE_DTS_HD_CORE,       // DTS_HD core sub stream
    RHALTYPE_DDP,               // Dolby Digital Plus audio
    RHALTYPE_SDDS,              // SDDS audio
    RHALTYPE_DV,                // DV audio
    RHALTYPE_AAC,               // AAC(Advanced Audio Coding)
    RHALTYPE_RAW_AAC,           // AAC without header
    RHALTYPE_OGG_AUDIO,         // OGG vorbis
    RHALTYPE_WMA,               // WMA audio
    RHALTYPE_WMAPRO,            // WMAPRO audio
    RHALTYPE_MP3,               // MP3 file
    RHALTYPE_MP4,               // MP4 aac file
    RHALTYPE_LATM_AAC,          // LATM/LOAS AAC file
    RHALTYPE_WAVE,              // WAVE audio
    RHALTYPE_AIFF,              // AIFF audio
    RHALTYPE_RTP,               // RTP
    RHALTYPE_APE,               // APE, pure audio
    RHALTYPE_RM,			    //
    RHALTYPE_RV,			    // real video
    RHALTYPE_RA_COOK,		    // RealAudio 8 Low Bit Rate(cook)
    RHALTYPE_RA_ATRC,		    // RealAudio 8 Hight Bit Rate(atrc)
    RHALTYPE_RA_RAAC,		    // AAC(raac)
    RHALTYPE_RA_SIPR,		    // RealAudio Voice(sipr)
    RHALTYPE_RA_LSD,		    // RealAudio Lossless
    RHALTYPE_ADPCM,             // ADPCM audio
    RHALTYPE_FLAC,			    //
    RHALTYPE_ULAW,			    //
    RHALTYPE_ALAW,			    //
    RHALTYPE_PCM_HDMV,          //
    RHALTYPE_HDMV_MLP_AUDIO,    // trueHD audio
    RHALTYPE_HDMV_DOLBY_AC3,    //
    RHALTYPE_HDMV_DDP,          //
    RHALTYPE_HDMV_DTS,          //
    RHALTYPE_HDMV_DTS_HD,       //
    RHALTYPE_AMRWB_AUDIO,       //
    RHALTYPE_AMRNB_AUDIO,       //
    RHALTYPE_SILK_AUDIO,        // Skype
    RHALTYPE_G729_AUDIO,        // Skype
    RHALTYPE_APE_AUDIO,         //
    RHALTYPE_OMADRMMP4,         // omadrm mp4
    RHALTYPE_OMADRMASF,         // omadrm asf
    RHALTYPE_SNDAOGG,           // snda raw ogg
    RHALTYPE_SNDASSOGG,         // snda standard ogg
    RHALTYPE_SNDAWAVE,          // snda WAVE audio
    RHALTYPE_DRA,               // DRA audio.
    RHALTYPE_OGG_OPUS,          //
    RHALTYPE_AMRWB_PLUS_AUDIO,  //
    RHALTYPE_DOLBY_AC4_AUDIO,   //
    RHALTYPE_AUDIO,             //
    RHALTYPE_DOLBY_MAT_AUDIO,   //

    RHALTYPE_MPEGH_AUDIO,       // MPEG-H

} ENUM_RHAL_CODEC_TYPE;

/* audio format information (INFO_AUDIO_FORMAT) */
typedef struct
{
    ENUM_RHAL_CODEC_TYPE type;     /* codec type */
    unsigned char   emphasis;      /* non-zero (true) means emphasis ON */
    unsigned char   mute;          /* non-zero (true) means mute ON */
    unsigned char   bitsPerSample;
    unsigned char   numberOfChannels;
    unsigned int    samplingRate;
    unsigned int    dynamicRange;  /* follow DVD-V spec, Page VI5-22 */
    int privateData[6];//add by yllin 2007.02.013, change size 2 to 6 by taro 2010.05.17
} AUDIO_FORMAT;

typedef enum{
    Enc_State_Stopped,
    Enc_State_Paused,
    Enc_State_Running,
} RTK_ENC_AUDIO_STATE;

typedef struct
{
    BYTE* pWritePointer;
    long  writeBufferSize;
} DELIVERINFO;

typedef struct
{
    UINT32 ain_instanceID;
    UINT32 dec_instanceID;
    UINT32 aout_instanceID;
    UINT32 ain_rh_phyAddr;
    UINT32 icq_rh_phyAddr;
    UINT32 dec_rh_phyAddr;
    UINT32 ain_buf_size;
    UINT32 icq_buf_size;
    UINT32 dec_buf_size;
    UINT32 *ain_nonCachedAddr;
    UINT32 *icq_nonCachedAddr;
    UINT32 *dec_nonCachedAddr;
} QuickShowCreateParam;

//typedef struct _AIN AIN;
typedef struct _AIN{
    Base* pBaseObj;
    AUDIO_BBADC_CONFIG  m_pBBADC_cfg;
    AUDIO_SPDIFI_CONFIG m_pSPDIF_cfg;
    AUDIO_I2SI_CONFIG   m_pI2S_cfg;
    AUDIO_RPC_IPT_SRC   m_pIptSrc;

    UINT32 (*SetATVClock)(Base*, SINT32);
    UINT32 (*SetSIFADCInit)(Base*, SINT32);
    UINT32 (*GetAudioFormatInfo)(Base*, AUDIO_RPC_AIN_FORMAT_INFO*);
}AIN;
Base* new_AIN(void);
Base* new_AIN_QuickShow(QuickShowCreateParam param);

typedef void (*pfnRecordCallBack)(UINT8 *pBuf, UINT32 length);

//typedef struct _MIC MIC;
typedef struct _MIC{
    Base* pBaseObj;
    UINT32 bStartRecord;
    SINT32 record_seconds;
    SINT32 record_fs;
    SINT32 record_bits;
    SINT32 record_ch;
    AUDIO_BBADC_CONFIG  m_pBBADC_cfg;
    AUDIO_I2SI_CONFIG   m_pI2S_cfg;
    AUDIO_RPC_IPT_SRC   m_pIptSrc;
    pfnRecordCallBack pRecordCallBack;
    Base* pDECObj;
    void *pDumpObj;
    UINT8* recordBuf;
    UINT32 recordBufOffset;
    SINT32 sampleInBuf;
    char filename[120];

    UINT32 (*SetRecordCallBack)(Base*,pfnRecordCallBack);
    UINT32 (*SetRecordStart)(Base*,UINT32);
    UINT32 (*SetRecordFormat)(Base*,SINT32,SINT32,SINT32);

}MIC;
Base *new_MIC(void);

//typedef struct _DEC DEC;
typedef struct _DEC{
    Base* pBaseObj;
    Pin*  pICQPin;
    Pin*  pDwnStrmQ;
    UINT8 m_ena_delayRp;
    SINT64 prev_PTS;
    SINT64 prev_2nd_PTS;
    SINT64 curr_PTS;
    struct semaphore* dwn_strm_sem;
    struct task_struct *adec_tsk;
    UINT32 rate;
    UINT32 channel;
    UINT32 chan_idx[AUDIO_MAX_CHNUM];
    int samples;
    char display_mode;
    char display_files;
    short display_period;
    UINT32 (*InitBSRingBuf)(Base*, UINT32);
    UINT32 (*InitICQRingBuf)(Base*, UINT32);
    UINT32 (*SetOutRingNumOfReadPtr)(Base*, UINT32);
    UINT32 (*GetAudioFormatInfo)(Base*, AUDIO_RPC_DEC_FORMAT_INFO*);
    UINT32 (*SetChannelSwap)(Base*, UINT32);
    UINT32 (*GetCurrentPTS)(Base*, SINT64*);
    UINT32 (*SetDecoderSRCMode)(Base*, SINT32, SINT32);
    UINT32 (*SetDelayRpMode)(Base*, UINT8);
    BOOLEAN (*IsESExist)(Base*);
}DEC;
Base* new_DEC(int idx);
Base* new_DEC_QuickShow(QuickShowCreateParam param);

//typedef struct _PPAO PPAO;
typedef struct _PPAO{
    Base* pBaseObj;
    UINT32 ao_instanceID;

    UINT32 (*GetAOAgentID)(Base*);
    UINT32 (*AO_Flush)(Base*);
    UINT32 (*SetSubChannel)(Base*, UINT32);
    UINT32 (*ConnectMultiInput)(Base*, Base*);
    UINT32 (*DisconnectMultiInput)(Base*);
}PPAO;
typedef enum {
    PPAO_FULL,
    AO_ONLY,
} PPAO_MODE;
Base* new_PPAO(PPAO_MODE);
Base* new_PPAO_QuickShow(PPAO_MODE, QuickShowCreateParam param);

typedef enum _PCM_FMT
{
    PCM_FMT_32_BE = 0,
    PCM_FMT_24_BE = 1,
    PCM_FMT_16_BE = 2,
    PCM_FMT_24_LE = 3,
    PCM_FMT_16_LE = 4,
    PCM_FMT_8     = 5,
    PCM_FMT_8_UNSIGNED = 6,
    PCM_FMT_INTERLEAVE_16_LE  = 0x84,
} PCM_FMT;

typedef struct _PCM_PARAM
{
    int64_t pts;
    int channel;
    int sampleRate;
    PCM_FMT format;
} PCM_PARAM;

typedef enum _SOUND_STAT
{
    FLASH_SND_ERR = 0,
    FLASH_SND_PLAYING,
    FLASH_SND_OVER,
} SOUND_STAT;


typedef void (*pfnFlashEOSCallBack)(SINT32 Data);

//typedef struct _DTV_COMMUNICATOR DtvCom;
typedef struct _DTV_COMMUNICATOR{
    Base* pBaseObj;
    UINT32 m_sdecChannel; // TV006 sdec using
    UINT32 m_sdecDest;    // TV006 sdec using
    bool   m_isEnableTracking;    // TV006 sdec using
    UINT32 BSRing_phy;
    void  *BSRing_vir;
    UINT32 ICQRing_phy;
    void  *ICQRing_vir;
    UINT32 RefClock_phy;
    void  *RefClock_vir;

    UINT32 BSRing_mapAddr;
    UINT32 BSRing_mapSz;
    UINT32 ICQRing_mapAddr;
    UINT32 ICQRing_mapSz;
    UINT32 RefClock_mapAddr;
    UINT32 RefClock_mapSz;


    UINT32 (*GetBSRingBufPhyAddress)(Base*);
    UINT32 (*GetICQRingBufPhyAddress)(Base*);
    UINT32 (*GetRefClockPhyAddress)(Base*);
    UINT32 (*SetBSRingBufPhyAddress)(Base*, UINT32, void*, UINT32, UINT32);
    UINT32 (*SetICQRingBufPhyAddress)(Base*, UINT32, void*, UINT32, UINT32);
    UINT32 (*SetRefClockPhyAddress)(Base*, UINT32, void*, UINT32, UINT32);
    UINT32 (*GetSDECChannel)(Base*);
    UINT32 (*GetSDECDest)(Base*);
    void   (*SetSDECInfo)(Base*, UINT32, UINT32);
    bool   (*SetAudioSyncMode)(Base*, AVSYNC_MODE);
    void* (*GetBSRingBufVirAddress)(Base*);
    void* (*GetICQRingBufVirAddress)(Base*);
    void* (*GetRefClockVirAddress)(Base*);
    bool   (*GetSDECTracking)(Base* pBaseObj);
    void   (*SetSDECTracking)(Base*, bool );
    void   (*GetBSRingBufMapInfo)(Base* , UINT32* , UINT32* );
    void   (*GetICQRingBufMapInfo)(Base* , UINT32* , UINT32* );
    void   (*GetRefClockMapInfo)(Base* , UINT32* , UINT32* );

}DtvCom;
Base* new_DtvCom(void);

#endif /*_RTK_AUDIO_*/
