/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author danielwei@realtek.com
 *
 *****************************************************************************/

//#include <assert.h>

//#include <kadp_DTVTunerControl.h>

#include "khal_demod_common.h"
#include "rtk_tv_fe_types.h"

typedef struct
{
    KHAL_DEMOD_TRANS_SYSTEM_T    transSystem;
    KHAL_DEMOD_CHANNEL_BW_T      channelBW;
    TV_SYSTEM                   tvSystem;
} TV_SYSTEM_TRANSLATE;

typedef struct
{
    KHAL_DEMOD_TRANS_SYSTEM_T            transSystem;
    KHAL_DEMOD_AUDIO_SIF_SOUNDSYSTEM_T   SIFFormat;
    TV_SYSTEM                           tvSystem;
} ATV_SYSTEM_TRANSLATE;

typedef struct
{
        int lgeHierarchy;
        int RtkHierarchy;
}HIERARCHY_TRANSLATE;

typedef struct
{
    int lgeConstelCode;
    int RtkConstelCode;
}CONSTEL_CODE_TRANSLATE;


typedef struct
{
    int lgeCarrierMode;
    int RtkCarrierMode;
}CARRIER_MODE_TRANSLATE;

typedef struct
{
    int lgeGuardCode;
    int RtkGuardCode;
}GUARD_CODE_TRANSLATE;

typedef struct
{
    int lgeCodeRate;
    int RtkCodeRate;
}CODE_RATE_TRANSLATE;
//FOR DVBS

typedef struct 
{
    KHAL_DEMOD_TPS_CODERATE_T    hal_codeRate;
    int                         drv_code_rate;
}DVBS_MAP_CODE_RATE;

typedef struct 
{
    KHAL_DEMOD_TPS_CONSTELLATION_T   hal_constellation;
    int                             drv_constellation;
}DVBS_MAP_CONSTELLATION;

//FOR DTMB
typedef struct 
{
    KHAL_DEMOD_TPS_CARRIER_MODE_T    hal_carrier_mode;
    DTMB_OFDM_CARRIER_MODE          drv_carrier_mode;
}DTMB_MAP_CARRIER_MODE;

typedef struct 
{
    KHAL_DEMOD_TPS_GUARD_INTERVAL_T hal_guard_interval;
    DTMB_OFDM_PSEUDO_NOISE_E            drv_pseudo_noise;
    DTMB_OFDM_PSEUDO_NOISE_SEQUENCE_E   drv_pseudo_noise_seq;
}DTMB_MAP_GUARD_INTERVALE;

typedef struct 
{
    KHAL_DEMOD_TPS_CODERATE_T    hal_codeRate;
    DTMB_OFDM_FEC_RATE_E        drv_code_rate;
}DTMB_MAP_CODE_RATE;

typedef struct 
{
    KHAL_DEMOD_TPS_CONSTELLATION_T   hal_constellation;
    DTMB_OFDM_CONSTELLATION_E       drv_constellation;
}DTMB_MAP_CONSTELLATION;
//==================================================================================================

static TV_SYSTEM_TRANSLATE _mapTVSystem[] =
{
#ifdef DVBT_T2_AUTODETECT
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_6M, TV_SYS_AUTOSCAN_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_7M, TV_SYS_AUTOSCAN_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_8M, TV_SYS_AUTOSCAN_8M},

    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_6M, TV_SYS_AUTOSCAN_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_7M, TV_SYS_AUTOSCAN_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_8M, TV_SYS_AUTOSCAN_8M},
#else
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_6M, TV_SYS_DVBT_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_7M, TV_SYS_DVBT_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBT, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBT_8M},

    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_6M, TV_SYS_DVBT2_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_7M, TV_SYS_DVBT2_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBT2, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBT2_8M},
#endif

    {KHAL_DEMOD_TRANS_SYS_DVBC, KHAL_DEMOD_CH_BW_6M, TV_SYS_DVBC_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBC, KHAL_DEMOD_CH_BW_7M, TV_SYS_DVBC_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBC, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBC_8M},

    {KHAL_DEMOD_TRANS_SYS_DVBC2, KHAL_DEMOD_CH_BW_6M, TV_SYS_DVBC_6M},
    {KHAL_DEMOD_TRANS_SYS_DVBC2, KHAL_DEMOD_CH_BW_7M, TV_SYS_DVBC_7M},
    {KHAL_DEMOD_TRANS_SYS_DVBC2, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBC_8M},

    //{HAL_DEMOD_TRANS_SYS_DTMB, HAL_DEMOD_CH_BW_8M, TV_SYS_DTMB},
    {KHAL_DEMOD_TRANS_SYS_DTMB, KHAL_DEMOD_CH_BW_8M, TV_SYS_DTMB_8M},
    {KHAL_DEMOD_TRANS_SYS_DTMB, KHAL_DEMOD_CH_BW_6M, TV_SYS_DTMB_6M},

    {KHAL_DEMOD_TRANS_SYS_ISDBT, KHAL_DEMOD_CH_BW_6M, TV_SYS_ISDBT_6M},
    {KHAL_DEMOD_TRANS_SYS_ISDBT, KHAL_DEMOD_CH_BW_7M, TV_SYS_ISDBT_7M},
    {KHAL_DEMOD_TRANS_SYS_ISDBT, KHAL_DEMOD_CH_BW_8M, TV_SYS_ISDBT_8M},

    {KHAL_DEMOD_TRANS_SYS_VSB, KHAL_DEMOD_CH_BW_6M, TV_SYS_ATSC},
    {KHAL_DEMOD_TRANS_SYS_QAM, KHAL_DEMOD_CH_BW_6M, TV_SYS_OPENCABLE_256},

    {KHAL_DEMOD_TRANS_SYS_DVBS, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBS},
    {KHAL_DEMOD_TRANS_SYS_DVBS2, KHAL_DEMOD_CH_BW_8M, TV_SYS_DVBS2},
};

static ATV_SYSTEM_TRANSLATE _mapATVSystem[] =
{
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_BG, TV_SYS_BG_PAL},
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_I, TV_SYS_I_PAL},
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_DK, TV_SYS_DK_PAL},
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_MN, TV_SYS_M_PAL},
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_L, TV_SYS_L_SECAM},
    {KHAL_DEMOD_TRANS_SYS_PAL, KHAL_DEMOD_AUDIO_SIF_SYSTEM_LP, TV_SYS_LL_SECAM},
    {KHAL_DEMOD_TRANS_SYS_NTSC, KHAL_DEMOD_AUDIO_SIF_SYSTEM_MN, TV_SYS_M_NTSC},
};

static HIERARCHY_TRANSLATE transHierarchy[]=
{
    {KHAL_DEMOD_TPS_HIERA_NONE,  OFDM_ALPHA_0},
    {KHAL_DEMOD_TPS_HIERA_1,     OFDM_ALPHA_1},
    {KHAL_DEMOD_TPS_HIERA_2,     OFDM_ALPHA_2},
    {KHAL_DEMOD_TPS_HIERA_4,     OFDM_ALPHA_4},
};

static CONSTEL_CODE_TRANSLATE transConstelCode[]=
{
    {KHAL_DEMOD_TPS_CONST_DQPSK,     OFDM_CONST_DQPSK},
    {KHAL_DEMOD_TPS_CONST_QAM_4,     OFDM_CONST_4},
    {KHAL_DEMOD_TPS_CONST_QAM_16,    OFDM_CONST_16},
    {KHAL_DEMOD_TPS_CONST_QAM_32,    OFDM_CONST_32},
    {KHAL_DEMOD_TPS_CONST_QAM_64,    OFDM_CONST_64},
    {KHAL_DEMOD_TPS_CONST_QAM_128,   OFDM_CONST_128},
    {KHAL_DEMOD_TPS_CONST_QAM_256,   OFDM_CONST_256},
};

static CARRIER_MODE_TRANSLATE transCarrierMode[]=
{
    {KHAL_DEMOD_TPS_CARR_1K,OFDM_FFT_1K},
    {KHAL_DEMOD_TPS_CARR_2K,OFDM_FFT_2K},
    {KHAL_DEMOD_TPS_CARR_4K,OFDM_FFT_4K},
    {KHAL_DEMOD_TPS_CARR_8K,OFDM_FFT_8K},
    {KHAL_DEMOD_TPS_CARR_16K,OFDM_FFT_16K},
    {KHAL_DEMOD_TPS_CARR_32K,OFDM_FFT_32K},
};


static GUARD_CODE_TRANSLATE transGuardCode[]=
{
    {KHAL_DEMOD_TPS_GUARD_1_4,        OFDM_GI_1_4},
    {KHAL_DEMOD_TPS_GUARD_1_8,        OFDM_GI_1_8},
    {KHAL_DEMOD_TPS_GUARD_1_16,       OFDM_GI_1_16},
    {KHAL_DEMOD_TPS_GUARD_1_32,       OFDM_GI_1_32},
    {KHAL_DEMOD_TPS_GUARD_1_128,      OFDM_GI_1_128},
    {KHAL_DEMOD_TPS_GUARD_19_128,     OFDM_GI_19_128},
    {KHAL_DEMOD_TPS_GUARD_19_256,     OFDM_GI_19_256},
};

static CODE_RATE_TRANSLATE transCodeRate[]=
{
    {KHAL_DEMOD_TPS_CODE_1_2,OFDM_CODE_RATE_1_2},
    {KHAL_DEMOD_TPS_CODE_2_3,OFDM_CODE_RATE_2_3},
    {KHAL_DEMOD_TPS_CODE_3_4,OFDM_CODE_RATE_3_4},
    {KHAL_DEMOD_TPS_CODE_3_5,OFDM_CODE_RATE_3_5},
    {KHAL_DEMOD_TPS_CODE_4_5,OFDM_CODE_RATE_4_5},
    {KHAL_DEMOD_TPS_CODE_5_6,OFDM_CODE_RATE_5_6},
    {KHAL_DEMOD_TPS_CODE_7_8,OFDM_CODE_RATE_7_8},
};
//FOR DVBS

static DVBS_MAP_CODE_RATE dvbs_map_codeRate_table[] =
{
    {KHAL_DEMOD_TPS_CODE_1_2,  DVBS2_CR_1_2},
    {KHAL_DEMOD_TPS_CODE_2_3,  DVBS2_CR_2_3},
    {KHAL_DEMOD_TPS_CODE_3_4,  DVBS2_CR_3_4},
    {KHAL_DEMOD_TPS_CODE_5_6,  DVBS2_CR_5_6},
    {KHAL_DEMOD_TPS_CODE_7_8,  DVBS2_CR_7_8},
    {KHAL_DEMOD_TPS_CODE_3_5,  DVBS2_CR_3_5},
    {KHAL_DEMOD_TPS_CODE_4_5,  DVBS2_CR_4_5},
    {KHAL_DEMOD_TPS_CODE_8_9,  DVBS2_CR_8_9},
    {KHAL_DEMOD_TPS_CODE_9_10,  DVBS2_CR_9_10},
};

static DVBS_MAP_CONSTELLATION dvbs_map_constellation_table[] =
{
    {KHAL_DEMOD_TPS_CONST_QPSK,      DVBS2_MOD_QPSK},
    {KHAL_DEMOD_TPS_CONST_PSK_8,     DVBS2_MOD_8PSK},
    {KHAL_DEMOD_TPS_CONST_QAM_16,    DVBS2_MOD_16APSK},
    {KHAL_DEMOD_TPS_CONST_QAM_32,    DVBS2_MOD_32APSK},
};


//FOR DTMB
static DTMB_MAP_CARRIER_MODE dtmb_map_carrierMode_table[] =
{
    {KHAL_DEMOD_TPS_CARR_SC, DTMB_OFDM_CARRIER_SINGLE},
    {KHAL_DEMOD_TPS_CARR_MC, DTMB_OFDM_CARRIER_MULTI},
};

static DTMB_MAP_GUARD_INTERVALE dtmb_map_guard_intervale_table[] =
{
    {KHAL_DEMOD_TPS_GUARD_420_C, DTMB_OFDM_PN_420,   DTMB_OFDM_PN_SEQ_CONSTANT},
    {KHAL_DEMOD_TPS_GUARD_420_V, DTMB_OFDM_PN_420,   DTMB_OFDM_PN_SEQ_VARIABLE},
    {KHAL_DEMOD_TPS_GUARD_595,   DTMB_OFDM_PN_595,   DTMB_OFDM_PN_SEQ_CONSTANT},
    {KHAL_DEMOD_TPS_GUARD_595,   DTMB_OFDM_PN_595,   DTMB_OFDM_PN_SEQ_VARIABLE},
    {KHAL_DEMOD_TPS_GUARD_945_C, DTMB_OFDM_PN_945,   DTMB_OFDM_PN_SEQ_CONSTANT},
    {KHAL_DEMOD_TPS_GUARD_945_V, DTMB_OFDM_PN_945,   DTMB_OFDM_PN_SEQ_VARIABLE},
};

static DTMB_MAP_CODE_RATE dtmb_map_codeRate_table[] =
{
    {KHAL_DEMOD_TPS_CODE_2_5, DTMB_OFDM_FEC_4_OVER_10},
    {KHAL_DEMOD_TPS_CODE_3_5, DTMB_OFDM_FEC_6_OVER_10},
    {KHAL_DEMOD_TPS_CODE_4_5, DTMB_OFDM_FEC_8_OVER_10},
};

static DTMB_MAP_CONSTELLATION dtmb_map_constellation_table[] =
{
    {KHAL_DEMOD_TPS_CONST_QAM_4NR,   DTMB_OFDM_QAM_4_NR},
    {KHAL_DEMOD_TPS_CONST_QAM_4,     DTMB_OFDM_QAM_4},
    {KHAL_DEMOD_TPS_CONST_QAM_16,    DTMB_OFDM_QAM_16},
    {KHAL_DEMOD_TPS_CONST_QAM_32,    DTMB_OFDM_QAM_32},
    {KHAL_DEMOD_TPS_CONST_QAM_64,    DTMB_OFDM_QAM_64},
};
//==================================================================================================================
TV_SYSTEM trans_TV_SYSTEM(KHAL_DEMOD_TRANS_SYSTEM_T transSystem, KHAL_DEMOD_CHANNEL_BW_T channelBW)
{
    UINT32 i = 0 ;
    for ( i = 0 ; i < sizeof(_mapTVSystem)/sizeof(TV_SYSTEM_TRANSLATE) ; i++)
    {
        if (_mapTVSystem[i].transSystem == transSystem && _mapTVSystem[i].channelBW == channelBW)
        {
            return _mapTVSystem[i].tvSystem;
        }
    }
    return TV_SYS_UNKNOWN;
}

TV_SYSTEM trans_ATV_SYSTEM(KHAL_DEMOD_TRANS_SYSTEM_T transSystem, KHAL_DEMOD_AUDIO_SIF_SOUNDSYSTEM_T SIFFormat)
{
    UINT32 i = 0 ;
    for ( i = 0 ; i < sizeof(_mapATVSystem)/sizeof(ATV_SYSTEM_TRANSLATE) ; i++)
    {
        if (_mapATVSystem[i].transSystem == transSystem && _mapATVSystem[i].SIFFormat == SIFFormat)
        {
            return _mapATVSystem[i].tvSystem;
        }
    }
    return TV_SYS_UNKNOWN;
}

int trans_Hierarchy_Translate(int value, int blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(transHierarchy)/sizeof(HIERARCHY_TRANSLATE));i++)
    {
        if(blgeflag){
            if(transHierarchy[i].lgeHierarchy==value)
                return transHierarchy[i].RtkHierarchy;
        }else{
            if(transHierarchy[i].RtkHierarchy==value)
                return transHierarchy[i].lgeHierarchy;
        }
    }

    return blgeflag ? KHAL_DEMOD_TPS_HIERA_UNKNOWN: -1;
}

int trans_ConstelCode_Translate(int value, int blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(transConstelCode)/sizeof(CONSTEL_CODE_TRANSLATE));i++)
    {
        if(blgeflag){
           if(transConstelCode[i].lgeConstelCode==value)
                return transConstelCode[i].RtkConstelCode;
        }else{
            if(transConstelCode[i].RtkConstelCode==value)
                 return transConstelCode[i].lgeConstelCode;
        }
    }
    return blgeflag ? KHAL_DEMOD_TPS_CONST_UNKNOWN : -1;
}

int trans_CarrierMode_Translate(int value, int blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(transCarrierMode)/sizeof(CARRIER_MODE_TRANSLATE));i++)
    {
        if(blgeflag){
            if(transCarrierMode[i].lgeCarrierMode==value)
                return transCarrierMode[i].RtkCarrierMode;
        }else{
            if(transCarrierMode[i].RtkCarrierMode==value)
                return transCarrierMode[i].lgeCarrierMode;
        }
    }
    return blgeflag ? KHAL_DEMOD_TPS_CARR_UNKNOWN : -1;
}

int trans_GuardCode_Translate(int value, int blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(transGuardCode)/sizeof(GUARD_CODE_TRANSLATE));i++)
    {
        if(blgeflag){
            if(transGuardCode[i].lgeGuardCode==value)
                return transGuardCode[i].RtkGuardCode;
        }else{
            if(transGuardCode[i].RtkGuardCode==value)
                return transGuardCode[i].lgeGuardCode;
        }
    }
    return blgeflag ? KHAL_DEMOD_TPS_GUARD_UNKNOWN : -1;
}

int trans_CodeRate_Translate(int value, int blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(transCodeRate)/sizeof(CODE_RATE_TRANSLATE));i++)
    {
        if(blgeflag){
           if(transCodeRate[i].lgeCodeRate==value)
                return transCodeRate[i].RtkCodeRate;
        }else{
            if(transCodeRate[i].RtkCodeRate==value)
                 return transCodeRate[i].lgeCodeRate;
        }
    }
    return blgeflag ? KHAL_DEMOD_TPS_CODE_UNKNOWN : -1;
}
/*
    for DVBS
*/

int DVBS_trans_codeRate_Translate(int codeRate, BOOLEAN blgeflag)
{
    UINT32 i=0;  
    for(i=0;i<(sizeof(dvbs_map_codeRate_table)/sizeof(DVBS_MAP_CODE_RATE));i++)
    {
        if(blgeflag){
            if(dvbs_map_codeRate_table[i].hal_codeRate ==codeRate)
                return dvbs_map_codeRate_table[i].drv_code_rate;
        }else{
            if(dvbs_map_codeRate_table[i].drv_code_rate==codeRate)
                return dvbs_map_codeRate_table[i].hal_codeRate;
        }
    }
    return blgeflag ? -1 : KHAL_DEMOD_TPS_CODE_UNKNOWN;
}

int DVBS_trans_constellation_Translate(int constellation, BOOLEAN blgeflag)
{
    UINT32 i=0; 
    for(i=0;i<(sizeof(dvbs_map_constellation_table)/sizeof(DVBS_MAP_CODE_RATE));i++)
    {
        if(blgeflag){
            if(dvbs_map_constellation_table[i].hal_constellation ==constellation)
                return dvbs_map_constellation_table[i].drv_constellation;
        }else{
            if(dvbs_map_constellation_table[i].drv_constellation == constellation)
                return dvbs_map_constellation_table[i].hal_constellation;
        }
    }
    return blgeflag ? -1 : KHAL_DEMOD_TPS_CONST_UNKNOWN;
}   
/*
    for DTMB
*/
int DTMB_trans_carrierMode_Translate(int carrierMode, BOOLEAN blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(dtmb_map_carrierMode_table)/sizeof(DTMB_MAP_CARRIER_MODE));i++)
    {
        if(blgeflag){
            if(dtmb_map_carrierMode_table[i].hal_carrier_mode==carrierMode)
                return dtmb_map_carrierMode_table[i].drv_carrier_mode;
        }else{
            if(dtmb_map_carrierMode_table[i].drv_carrier_mode==carrierMode)
                return dtmb_map_carrierMode_table[i].hal_carrier_mode;
        }
    }
    return blgeflag ? DTMB_OFDM_CARRIER_INVALID : KHAL_DEMOD_TPS_CARR_UNKNOWN;
}

int DTMB_trans_guardInterval_Translate(int value1, int value2, BOOLEAN blgeflag)
{//only rtk to lg

    UINT32 i=0;
    for(i=0;i<(sizeof(dtmb_map_guard_intervale_table)/sizeof(DTMB_MAP_GUARD_INTERVALE));i++)
    {
        if (blgeflag)
        {
            if (dtmb_map_guard_intervale_table[i].hal_guard_interval == value1){
                if (value2)
                    return dtmb_map_guard_intervale_table[i].drv_pseudo_noise;
                else
                    return dtmb_map_guard_intervale_table[i].drv_pseudo_noise_seq;
            }
        }
        else{
            if((dtmb_map_guard_intervale_table[i].drv_pseudo_noise==value1) &&(dtmb_map_guard_intervale_table[i].drv_pseudo_noise_seq == value2))
                return dtmb_map_guard_intervale_table[i].hal_guard_interval;

        }
    }
    return blgeflag? (value2?DTMB_OFDM_PN_INVALID:DTMB_OFDM_PN_SEQ_INVALID):KHAL_DEMOD_TPS_GUARD_UNKNOWN;
}

int DTMB_trans_codeRate_Translate(int codeRate, BOOLEAN blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(dtmb_map_codeRate_table)/sizeof(DTMB_MAP_CODE_RATE));i++)
    {
        if(blgeflag){
            if(dtmb_map_codeRate_table[i].hal_codeRate ==codeRate)
                return dtmb_map_codeRate_table[i].drv_code_rate;
        }else{
            if(dtmb_map_codeRate_table[i].drv_code_rate==codeRate)
                return dtmb_map_codeRate_table[i].hal_codeRate;
        }
    }
    return blgeflag ? DTMB_OFDM_FEC_INVALID : KHAL_DEMOD_TPS_CODE_UNKNOWN;
}

int DTMB_trans_constellation_Translate(int constellation, BOOLEAN blgeflag)
{
    UINT32 i=0;
    for(i=0;i<(sizeof(dtmb_map_constellation_table)/sizeof(DTMB_MAP_CONSTELLATION));i++)
    {
        if(blgeflag){
            if(dtmb_map_constellation_table[i].hal_constellation ==constellation)
                return dtmb_map_constellation_table[i].drv_constellation;
        }else{
            if(dtmb_map_constellation_table[i].drv_constellation == constellation)
                return dtmb_map_constellation_table[i].hal_constellation;
        }
    }
    return blgeflag ? DTMB_OFDM_QAM_INVALID : KHAL_DEMOD_TPS_CONST_UNKNOWN;
}

