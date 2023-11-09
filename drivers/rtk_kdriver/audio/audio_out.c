/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yungjui.lee@realtek.com
 *
 *****************************************************************************/
#include "audio_base.h"
#include <linux/slab.h>
#include "audio_rpc.h"

/*#define CRASH_ON(exp) ({\
    if(exp)\
    {\
        printf("\n\n[%s:%d] CRASH_ON:%s\n\n\n", __FUNCTION__, __LINE__, #exp);\
        assert(0);\
    }\
})*/

static UINT32 m_MainPPAOInited = 0;
static UINT32 m_SubPPAOInited = 0;
static UINT32 m_focusPinID = 0;
static UINT32 m_pinCount = 0;

static UINT32 GetGeneralInstanceID(UINT32 instanceID, UINT32 pinID)
{
    return (instanceID&0xFFFFF000)|(pinID&0xFFF);
}
static UINT32 queryPPInPin(UINT32 instanceID)
{
    UINT32 curPinID, res;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL result;
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter={0};

    parameter.instanceID = instanceID;
    parameter.type = ENUM_PRIVATEINFO_AUDIO_GET_PCM_IN_PIN;
    res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &result);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    curPinID = result.privateInfo[0];

    m_pinCount++;
    DEBUG("[%s:%d] PinID=%d, total %d pin(s) in use\n", __FUNCTION__, __LINE__, curPinID, m_pinCount);
    return curPinID;
}

static void releasePPInPin(UINT32 instanceID, UINT32 curPinID)
{
    UINT32 res;
    AUDIO_RPC_PRIVATEINFO_RETURNVAL result;
    struct AUDIO_RPC_PRIVATEINFO_PARAMETERS parameter;

    parameter.instanceID = instanceID;
    parameter.type = ENUM_PRIVATEINFO_AUDIO_RELEASE_PCM_IN_PIN;
    parameter.privateInfo[0] = curPinID;
    res = RTKAUDIO_RPC_TOAGENT_PRIVATEINFO_SVC(&parameter, &result);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    m_pinCount--;
    DEBUG("[%s:%d] PinID=%d, total %d pin(s) in use\n", __FUNCTION__, __LINE__, curPinID, m_pinCount);
}

static UINT32 ConfigAO(Base* pObj)
{
    UINT32 res;

    AUDIO_CONFIG_DAC_I2S   dac_i2s_config;
    AUDIO_CONFIG_DAC_SPDIF dac_spdif_config;
    PPAO* pPPAOObj = (PPAO*)pObj->pDerivedObj;

    //config i2s
    dac_i2s_config.instanceID = pPPAOObj->GetAOAgentID(pObj);
    dac_i2s_config.dacConfig.audioGeneralConfig.interface_en = 1;
    dac_i2s_config.dacConfig.audioGeneralConfig.channel_out  = LEFT_CENTER_FRONT_CHANNEL_EN|RIGHT_CENTER_FRONT_CHANNEL_EN;
    dac_i2s_config.dacConfig.audioGeneralConfig.count_down_play_en  = 0;
    dac_i2s_config.dacConfig.audioGeneralConfig.count_down_play_cyc = 0;
    dac_i2s_config.dacConfig.sampleInfo.sampling_rate = 48000;
    dac_i2s_config.dacConfig.sampleInfo.PCM_bitnum    = 24;
    res = RTKAUDIO_RPC_TOAGENT_DAC_I2S_CONFIG_SVC(&dac_i2s_config);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    //config spdif
    dac_spdif_config.instanceID = pPPAOObj->GetAOAgentID(pObj);
    dac_spdif_config.spdifConfig.audioGeneralConfig.interface_en        = 1;
    dac_spdif_config.spdifConfig.audioGeneralConfig.channel_out         = SPDIF_LEFT_CHANNEL_EN|SPDIF_RIGHT_CHANNEL_EN;
    dac_spdif_config.spdifConfig.audioGeneralConfig.count_down_play_en  = 0;
    dac_spdif_config.spdifConfig.audioGeneralConfig.count_down_play_cyc = 0;
    dac_spdif_config.spdifConfig.sampleInfo.sampling_rate = 48000;
    dac_spdif_config.spdifConfig.sampleInfo.PCM_bitnum    = 24;
    dac_spdif_config.spdifConfig.out_cs_info.non_pcm_valid      = 0;
    dac_spdif_config.spdifConfig.out_cs_info.non_pcm_format     = 0;
    dac_spdif_config.spdifConfig.out_cs_info.audio_format       = 0;
    dac_spdif_config.spdifConfig.out_cs_info.spdif_consumer_use = 0;
    dac_spdif_config.spdifConfig.out_cs_info.copy_right         = 0;
    dac_spdif_config.spdifConfig.out_cs_info.pre_emphasis       = 0;
    dac_spdif_config.spdifConfig.out_cs_info.stereo_channel     = 0;
    res = RTKAUDIO_RPC_TOAGENT_DAC_SPDIF_CONFIG_SVC(&dac_spdif_config);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    return res;
}

static UINT32 SetPPMixer(Base* pBaseObj, UINT32 mode, UINT32 vol, UINT32 pinID)
{
    UINT32 res;
    AUDIO_RPC_PTS_MIXER_CONFIG rpc_pts_mixer;
    int i;

    rpc_pts_mixer.instanceID = pBaseObj->GetAgentID(pBaseObj);
    rpc_pts_mixer.config.pinID = pinID;
    rpc_pts_mixer.config.mixer_in_ena = 1;
    rpc_pts_mixer.config.mixer_in_change_PTS = 0;

    for(i = 0; i < AUDIO_MAX_CHNUM; i++) {
        rpc_pts_mixer.config.mixer_gain[i] = 0; // mute
    }

    if (vol > 0x7FFFFFFF)
        vol = 0x7FFFFFFF;
    /*
    if (vol < 0)
        vol = 0;
    */
    switch(mode)
    {
    default:
    case 0:     // mixer off
        DEBUG("mixer off\n");
        rpc_pts_mixer.config.mixer_in_ena = 0;
        for(i = 0; i < AUDIO_MAX_CHNUM; i++)
        {
            rpc_pts_mixer.config.mixer_gain[i] = 0;
        }
        break;

    case 1:     // mixer on, multi channel on
        DEBUG("mixer on, vol = %x\n", vol);
        rpc_pts_mixer.config.mixer_in_ena = 1;
        for(i = 0; i < AUDIO_MAX_CHNUM; i++)
        {
            rpc_pts_mixer.config.mixer_gain[i] = vol; // 0x7FFFFFFF;
        }
        break;

    case 2:     // mixer on, mute L channel, test only
        rpc_pts_mixer.config.mixer_in_ena = 1;
        rpc_pts_mixer.config.mixer_gain[0] = 0;
        rpc_pts_mixer.config.mixer_gain[1] = vol; // 0x7FFFFFFF;
        break;

    case 3:     // mixer on, mute R channel, test only
        rpc_pts_mixer.config.mixer_in_ena = 1;
        rpc_pts_mixer.config.mixer_gain[0] = vol; // 0x7FFFFFFF;
        rpc_pts_mixer.config.mixer_gain[1] = 0;
        break;
    }

    res = RTKAUDIO_RPC_TOAGENT_PTS_MIXER_CONFIG(&rpc_pts_mixer);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
    return res;

}

UINT32 GetAOAgentID(Base* pBaseObj)
{
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;
    return pPPAOObj->ao_instanceID;
}
UINT32 PPAO_SwitchFocus(Base* pBaseObj, void* data)
{
    UINT32 instanceID, res;
    AUDIO_RPC_FOCUS focus;
    PPAO* pPPAOObj = NULL;
    DEBUG("[%s]\n",__FUNCTION__);
    if(pBaseObj == NULL)
        return S_FALSE;
    pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    if(pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID) return S_FALSE;
    if(pBaseObj->GetInPinID(pBaseObj) == UNINIT_PINID) return S_FALSE;

    if(pPPAOObj == NULL)
        return S_FALSE;
    instanceID = GetGeneralInstanceID(pPPAOObj->GetAOAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));

    switch(pBaseObj->state)
    {
        case STATE_RUN:
            res = RTKAUDIO_RPC_TOAGENT_RUN_SVC(&instanceID);
            if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
            break;
        case STATE_PAUSE:
            res = RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(&instanceID);
            if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
            break;
        case STATE_STOP:
            res = RTKAUDIO_RPC_TOAGENT_STOP_SVC(&instanceID);
            if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
            break;
        default:
            break;
    }

    focus.instanceID = pBaseObj->GetAgentID(pBaseObj);
    focus.focusID    = pBaseObj->GetInPinID(pBaseObj);
    res = RTKAUDIO_RPC_TOAGENT_SWITCH_FOCUS_SVC(&focus);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
    m_focusPinID = pBaseObj->GetInPinID(pBaseObj);
    DEBUG("[%s] focusID to %d\n",__FUNCTION__, m_focusPinID);
    return res;
}

UINT32 PPAO_Run(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    if(pBaseObj->GetInPinID(pBaseObj) == m_focusPinID)
    {
        instanceID =  GetGeneralInstanceID(pPPAOObj->GetAOAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
        res = RTKAUDIO_RPC_TOAGENT_RUN_SVC(&instanceID);

        if(res != S_OK) {ERROR("[%s][pPPAOObj]RPC return != S_OK\n",__FUNCTION__); return res;}
        else DEBUG("[%s] [pPPAOObj]RPC return = S_OK\n",__FUNCTION__);
    }
    instanceID = GetGeneralInstanceID(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
    res = RTKAUDIO_RPC_TOAGENT_RUN_SVC(&instanceID);

    pBaseObj->state = STATE_RUN;
    if(res != S_OK) ERROR("[%s][pBaseObj]RPC return != S_OK\n",__FUNCTION__);
    else DEBUG("[%s] [pBaseObj]RPC return = S_OK\n",__FUNCTION__);
    return res;

}
UINT32 PPAO_Pause(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    if(pBaseObj->GetInPinID(pBaseObj) == m_focusPinID || pBaseObj->GetInPinID(pBaseObj) == (UINT32)FAKE_FOCUS_ID)
    {
        instanceID =  GetGeneralInstanceID(pPPAOObj->GetAOAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
        res = RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(&instanceID);

        if(res != S_OK) {ERROR("[%s][pPPAOObj]RPC return != S_OK\n",__FUNCTION__); return res;}
        else DEBUG("[%s] [pPPAOObj]RPC return = S_OK\n",__FUNCTION__);
    }
    instanceID = GetGeneralInstanceID(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
    res = RTKAUDIO_RPC_TOAGENT_PAUSE_SVC(&instanceID);

    pBaseObj->state = STATE_PAUSE;
    if(res != S_OK) ERROR("[%s][pBaseObj]RPC return != S_OK\n",__FUNCTION__);
    else DEBUG("[%s] [pBaseObj]RPC return = S_OK\n",__FUNCTION__);
    return res;

}

UINT32 PPAO_Stop(Base* pBaseObj)
{
    UINT32 res;
    UINT32 instanceID;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    if(pBaseObj->GetInPinID(pBaseObj) == m_focusPinID || pBaseObj->GetInPinID(pBaseObj) == (UINT32)FAKE_FOCUS_ID)
    {
        instanceID =  GetGeneralInstanceID(pPPAOObj->GetAOAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
        res = RTKAUDIO_RPC_TOAGENT_STOP_SVC(&instanceID);

        if(res != S_OK) {ERROR("[%s][pPPAOObj]RPC return != S_OK\n",__FUNCTION__); return res;}
        else DEBUG("[%s] [pPPAOObj]RPC return = S_OK\n",__FUNCTION__);
    }
    instanceID = GetGeneralInstanceID(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
    res = RTKAUDIO_RPC_TOAGENT_STOP_SVC(&instanceID);

    pBaseObj->state = STATE_STOP;
    if(res != S_OK) ERROR("[%s][pBaseObj]RPC return != S_OK\n",__FUNCTION__);
    else DEBUG("[%s] [pBaseObj]RPC return = S_OK\n",__FUNCTION__);
    return res;

}

UINT32 PPAO_SetRefClock(Base* pBaseObj, UINT32 refClockPhyAddr)
{
    UINT32 res;
    AUDIO_RPC_REFCLOCK rpc_refclock;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    rpc_refclock.instanceID  = pBaseObj->GetAgentID(pBaseObj);
    rpc_refclock.pRefClockID = pBaseObj->GetInPinID(pBaseObj);
    rpc_refclock.pRefClock   = refClockPhyAddr; // pass physical address to agent
    res = RTKAUDIO_RPC_TOAGENT_SETREFCLOCK_SVC(&rpc_refclock);
    if(res != S_OK) AUDIO_ERROR("[%s:%d][pBaseObj]RPC return != S_OK\n",__FUNCTION__,__LINE__);
    return res;

    rpc_refclock.instanceID  = pPPAOObj->GetAOAgentID(pBaseObj);
    res = RTKAUDIO_RPC_TOAGENT_SETREFCLOCK_SVC(&rpc_refclock);
    if(res != S_OK) AUDIO_ERROR("[%s:%d]RPC return != S_OK\n",__FUNCTION__,__LINE__);
    return res;

}
UINT32 PPAO_PrivateInfo(Base* pBaseObj, UINT32 infoId, UINT8* pData, UINT32 length)
{
    if(infoId == INFO_DELIVER_INFO && length == sizeof(DELIVERINFO))
    {
        //DEBUG("[%s] DeliverInfo\n",pBaseObj->name);
    }
    else if(infoId == INFO_EOS)
    {
        DEBUG("[%s] Audio Out Send out 1 EOS.\n",pBaseObj->name);
        if(pBaseObj->flowEventQ)
        {
            Queue* pQueue = (Queue*)pBaseObj->flowEventQ;
            pQueue->EnQueue(pQueue, (void*)EVENT_EOS);
            DEBUG("[%s] EnQueue EOS.\n",pBaseObj->name);
        }
    }
    else if(infoId == INFO_AUDIO_START_MIXING)
    {
        //assert(length == sizeof(DUAL_DEC_MIXING));
        DUAL_DEC_MIXING* pInfo = (DUAL_DEC_MIXING*)pData;

        /*DTV AD on: 2 DEC connect to 2 PPAO */
        SetPPMixer(pBaseObj, pInfo->mode,(UINT32)pInfo->volume, pBaseObj->GetInPinID(pBaseObj));
    }
    else
    {
        ERROR("[%s] UnHandled PrivateInfo id(%d)\n", pBaseObj->name, infoId);
        return S_FALSE;
    }

    return S_OK;
}
UINT32 PPAO_SetSeekingInfo(Base* pBaseObj, SINT32 Speed, SINT32 Skip)
{
    AUDIO_RPC_SEEK  seekInfo;
    UINT32          res;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    if (pBaseObj->GetAgentID(pBaseObj) == UNDEFINED_AGENT_ID)
    {
        res = S_OK;
    }
    else
    {
        seekInfo.skip = Skip;
        seekInfo.speed = Speed;
        seekInfo.instanceID = GetGeneralInstanceID(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
        res = RTKAUDIO_RPC_TOAGENT_SETSEEKING_SVC(&seekInfo);
        if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

        seekInfo.instanceID = GetGeneralInstanceID(pPPAOObj->GetAOAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
        res = RTKAUDIO_RPC_TOAGENT_SETSEEKING_SVC(&seekInfo);
        if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
    }

    return res;
}

UINT32 AO_Flush(Base* pBaseObj)
{
    UINT32 res;
    AUDIO_RPC_SENDIO sendIO;
    PPAO* pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    sendIO.pinID      = PCM_IN_RTK;
    sendIO.instanceID = pPPAOObj->GetAOAgentID(pBaseObj);

    res = RTKAUDIO_RPC_TOAGENT_FLUSH_SVC(&sendIO);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
    return res;
}

UINT32 SetSubChannel(Base* pBaseObj, UINT32 ena)
{
    UINT32 res;
    AUDIO_RPC_SUBCHANNEL rpc_sub;

    rpc_sub.instanceID = pBaseObj->GetAgentID(pBaseObj);
    if(ena) {
        rpc_sub.SubChannelPinID = pBaseObj->GetInPinID(pBaseObj);
    }else {
        rpc_sub.SubChannelPinID = -1;
    }

    DEBUG("[AP] instance=0x%08x, pin=%d\n\n", (unsigned int)rpc_sub.instanceID, (int)rpc_sub.SubChannelPinID);

    res = RTKAUDIO_RPC_TOAGENT_SUBCHANNEL_SVC(&rpc_sub);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);
    return res;
}


void delete_PPAO(Base* pBaseObj)
{
    UINT32 instanceID, res;
    PPAO* pPPAOObj = NULL;

    if(pBaseObj == NULL) return;
    pPPAOObj = (PPAO*)pBaseObj->pDerivedObj;

    PPAO_Stop(pBaseObj);
    pBaseObj->Stop = NULL;

    del_id_from_map(pBaseObj->GetAgentID(pBaseObj)   | pBaseObj->GetInPinID(pBaseObj));
    del_id_from_map(pPPAOObj->GetAOAgentID(pBaseObj) | pBaseObj->GetInPinID(pBaseObj));

    if(pPPAOObj->GetAOAgentID(pBaseObj) != UNDEFINED_AGENT_ID)
    {
        DestroyAgent(&pPPAOObj->ao_instanceID);
    }
    // init pp in pin (reset)
    if(pBaseObj->GetInPinID(pBaseObj) != UNINIT_PINID)
        releasePPInPin(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));

    instanceID = GetGeneralInstanceID(pBaseObj->GetAgentID(pBaseObj), pBaseObj->GetInPinID(pBaseObj));
    res = RTKAUDIO_RPC_TOAGENT_PP_INITPIN_SVC(&instanceID);
    if(res != S_OK) AUDIO_ERROR("[%s:%d] RPC return != S_OK\n",__FUNCTION__,__LINE__);

    pBaseObj->Remove(pBaseObj);

    if(m_MainPPAOInited && strcmp("MainPPAO",pBaseObj->name) == 0)
    {
        m_MainPPAOInited = 0;
    }
    if(m_SubPPAOInited && strcmp("SubPPAO",pBaseObj->name) == 0)
    {
        m_SubPPAOInited = 0;
    }

    kfree(pPPAOObj);
    delete_base(pBaseObj);
}

Base* new_PPAO_QuickShow(PPAO_MODE mode, QuickShowCreateParam param)
{
    /* object init */
    PPAO* pPPAOObj = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
    {
        return NULL;
    }
    pPPAOObj = (PPAO*)kmalloc(sizeof(PPAO), GFP_KERNEL);
    if(pPPAOObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pPPAOObj;

    /* setup member functions */
    pObj->Delete            = delete_PPAO;
    pObj->SwitchFocus       = PPAO_SwitchFocus;
    pObj->SetRefClock       = PPAO_SetRefClock;
    pObj->SetSeekingInfo    = PPAO_SetSeekingInfo;
    pObj->PrivateInfo       = PPAO_PrivateInfo;
    pObj->Run               = PPAO_Run;
    pObj->Pause             = PPAO_Pause;
    pObj->Stop              = PPAO_Stop;
    pPPAOObj->GetAOAgentID  = GetAOAgentID;
    pPPAOObj->SetSubChannel = SetSubChannel;
    pPPAOObj->AO_Flush      = AO_Flush;
    /* agent init */
    //pPPAOObj->ao_instanceID = CreateAgent(AUDIO_OUT);
    pPPAOObj->ao_instanceID = param.aout_instanceID;
    pObj->instanceID        = CreateAgent(AUDIO_PP_OUT);
    if(pPPAOObj->ao_instanceID == UNDEFINED_AGENT_ID || pObj->instanceID == UNDEFINED_AGENT_ID)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    ConfigAO(pObj);
    if(!m_MainPPAOInited)
    {
        char name[] = "MainPPAO";
        memcpy(&pObj->name, name, sizeof(name));
        m_MainPPAOInited = 1;
    }
    else if(!m_SubPPAOInited)
    {
        char name[] = "SubPPAO";
        memcpy(&pObj->name, name, sizeof(name));
        m_SubPPAOInited = 1;
    }
    else
    {
        return NULL;
    }
    if(mode == AO_ONLY)
    {
        pObj->inPinID = UNINIT_PINID;           // no in pin
    }
    else
    {
        pObj->inPinID = queryPPInPin(pObj->GetAgentID(pObj));
    }
    pObj->outPinID = AUDIO_PP_OUTPIN_ID;

    pObj->state = STATE_RUN; /*Avoid init InRingBuf & rpc as connect*/

    add_id_to_map(pObj, pObj->GetAgentID(pObj)       | pObj->GetInPinID(pObj));
    add_id_to_map(pObj, pPPAOObj->GetAOAgentID(pObj) | pObj->GetInPinID(pObj));
    return pObj;
}

Base* new_PPAO(PPAO_MODE mode)
{
    /* object init */
    PPAO* pPPAOObj = NULL;
    Base* pObj = new_base();
    if(pObj == NULL)
    {
        return NULL;
    }
    pPPAOObj = (PPAO*)kmalloc(sizeof(PPAO), GFP_KERNEL);
    if(pPPAOObj == NULL)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    pObj->pDerivedObj = pPPAOObj;

    /* setup member functions */
    pObj->Delete            = delete_PPAO;
    pObj->SwitchFocus       = PPAO_SwitchFocus;
    pObj->SetRefClock       = PPAO_SetRefClock;
    pObj->SetSeekingInfo    = PPAO_SetSeekingInfo;
    pObj->PrivateInfo       = PPAO_PrivateInfo;
    pObj->Run               = PPAO_Run;
    pObj->Pause             = PPAO_Pause;
    pObj->Stop              = PPAO_Stop;
    pPPAOObj->GetAOAgentID  = GetAOAgentID;
    pPPAOObj->SetSubChannel = SetSubChannel;
    pPPAOObj->AO_Flush      = AO_Flush;
    /* agent init */
    pPPAOObj->ao_instanceID = CreateAgent(AUDIO_OUT);
    pObj->instanceID        = CreateAgent(AUDIO_PP_OUT);
    if(pPPAOObj->ao_instanceID == UNDEFINED_AGENT_ID || pObj->instanceID == UNDEFINED_AGENT_ID)
    {
        pObj->Delete(pObj);
        return NULL;
    }
    ConfigAO(pObj);
    if(!m_MainPPAOInited)
    {
        char name[] = "MainPPAO";
        memcpy(&pObj->name, name, sizeof(name));
        m_MainPPAOInited = 1;
    }
    else if(!m_SubPPAOInited)
    {
        char name[] = "SubPPAO";
        memcpy(&pObj->name, name, sizeof(name));
        m_SubPPAOInited = 1;
    }
    else
    {
        return NULL;
    }
    if(mode == AO_ONLY)
    {
        pObj->inPinID = UNINIT_PINID;           // no in pin
    }
    else
    {
        pObj->inPinID = queryPPInPin(pObj->GetAgentID(pObj));
    }
    pObj->outPinID = AUDIO_PP_OUTPIN_ID;

    add_id_to_map(pObj, pObj->GetAgentID(pObj)       | pObj->GetInPinID(pObj));
    add_id_to_map(pObj, pPPAOObj->GetAOAgentID(pObj) | pObj->GetInPinID(pObj));
    return pObj;
}
